/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __ptr_reusable_pool_h__
#define __ptr_reusable_pool_h__

#include <deque>
#include <map>
#include <memory>
#include <typeindex>
#include <vector>

#include "base_cpp/exception.h"
#include "base_cpp/reusable.h"

namespace indigo
{

    DECL_EXCEPTION(PtrReusablePoolError);
    /***************************************************************************
    * PtrReusablePool<T> — sparse owning pool with stable indices and stable
    * pointers, unifying the legacy ObjPool<T> and PtrPool<T>.
    *
    * Design:
    *  * Objects live in std::vector<std::unique_ptr<T>> (_slots): the pointers
    *   are heap-allocated and NEVER move, so references returned by get()/
    *    operator[] stay valid across push() (fixes the latent Graph/bingo UB
    *    where a T& was cached across an add). Only the vector of handles (the
    *    "spine") reallocates on growth — a nothrow move of pointers.
    *  * Freed slot indices are kept in a free list (_free). Reuse is LIFO
    *    (most-recently-freed first), matching the observable index-reuse order
    *    of the legacy Pool<T> it replaces, so consumer behavior is preserved.
    *  * Reuse, never destroy: remove(idx) does NOT destroy the object; it calls
    *    T::reuse() (non-destructive) and parks the slot in the free list. When
    *    push() reuses the slot it calls reuse() again on the already-built
    *    object. clear()/reuse() (whole-collection empty) moves every object to
    *    a RESERVE (retained, not destroyed) and empties the active spine so
    *    size()/end() become 0; a later grow pulls from the reserve before
    *    constructing. Objects are freed ONLY when the whole collection is
    *    destroyed (dtor) or purge() is called explicitly.
    *  * Exception-safe growth: the object is fully constructed via make_unique
    *    BEFORE any bookkeeping is mutated, so a throwing T constructor leaves
    *    _slots/_free/_live/_size consistent (fixes the legacy ObjPool bug where
    *    a slot was reserved before the placement-new).
    *  * The pool itself is Reusable, so pools can be nested: reuse() returns
    *    every slot to the free list.
    *  * Heterogeneous discipline (HETERO-POOL-DESIGN): for polymorphic element
    *    hierarchies (SGroup, MetaObject, BaseMolecule) freed slots are parked
    *    in PER-TYPE free lists (_free_by_type, keyed by Reusable::reuseTypeId()),
    *    because reuse() cannot change an object's dynamic type — a slot may be
    *    reused only by a request for the SAME type. The reuse-on-remove +
    *    reuse-on-reuse lifecycle is INHERITED from the homogeneous discipline;
    *    the disciplines differ only in where a slot is parked and how objects
    *    are created: add(key, factory) grows via a LAZY per-type factory,
    *    adopt() appends a caller-constructed (populated) object. The discipline
    *    is fixed by the first allocating call (push() = homogeneous,
    *    add(key, factory)/adopt() = heterogeneous); mixing the two on one pool
    *    instance throws. clear()/reuse() is non-destructive for BOTH disciplines
    *    (objects move to a per-discipline reserve, per-type for heterogeneous);
    *    only purge()/dtor free them.
    ****************************************************************************/

    template <class T>
    class PtrReusablePool : public Reusable
    {
        // The T-must-be-Reusable check lives in the allocating methods rather
        // than at class scope: is_reusable_v<T> (is_base_of) needs a COMPLETE T,
        // but the pool must be declarable with a forward-declared element type
        // (e.g. PtrReusablePool<BaseMolecule> in molecule_rgroups.h, which only
        // forward-declares BaseMolecule to avoid a circular include). By the time
        // push()/add()/adopt() are instantiated, T is complete at the call site.
        static void _assertReusable()
        {
            static_assert(is_reusable_v<T>, "T must implement the Reusable interface (inherit Reusable + define reuse())");
        }

    public:
        DECL_TPL_ERROR(PtrReusablePoolError);

        // Default initial spine reserve (power of two, RFC §4.2).
        static constexpr int DEFAULT_RESERVE = 2;

        PtrReusablePool()
        {
            _slots.reserve(DEFAULT_RESERVE);
            _live.reserve(DEFAULT_RESERVE);
        }

        ~PtrReusablePool() override = default;

        PtrReusablePool(const PtrReusablePool&) = delete;
        PtrReusablePool& operator=(const PtrReusablePool&) = delete;

        // Allocate a slot and return its index. Reuses a freed slot (calling
        // reuse() on the retained object) or grows the spine.
        int push()
        {
            _assertReusable();
            _requireDiscipline(Discipline::Homogeneous);
            if (!_free.empty())
            {
                int idx = _free.back(); // LIFO reuse
                _free.pop_back();
                // Mark the slot live and count it BEFORE reuse(), so that if a
                // (misbehaving) reuse() throws the slot stays in a consistent
                // live state rather than being lost from both the free list and
                // the live set. Mirrors the ordering already used in remove().
                _live[idx] = true;
                _size++;
                _slots[idx]->reuse();
                return idx;
            }

            // Grow by reusing an object retained across a prior clear() (no
            // construction), else construct a fresh one.
            if (!_reserve.empty())
            {
                _slots.push_back(std::move(_reserve.back()));
                _reserve.pop_back();
                _live.push_back(true);
                _size++;
                _slots.back()->reuse(); // reset AFTER bookkeeping (consistent if it throws)
                return static_cast<int>(_slots.size()) - 1;
            }
            // Construct the object first; if the ctor throws nothing below runs,
            // so the pool state is unchanged.
            std::unique_ptr<T> obj = std::make_unique<T>();
            _slots.push_back(std::move(obj));
            _live.push_back(true);
            _size++;
            return static_cast<int>(_slots.size()) - 1;
        }

        // Compatibility alias for legacy consumers migrating off the no-argument
        // ObjPool<T>::add() / PtrPool used only default construction at the site.
        int add()
        {
            return push();
        }

        // Heterogeneous allocation (construct-flow): reuse a freed slot of the
        // SAME dynamic type — reuse-on-reuse, the lifecycle inherited from the
        // homogeneous discipline — or grow by invoking the LAZY factory. The
        // factory runs only when no slot of `key` is available, so call sites
        // like add(key, [&]{ return mol.neu(); }) construct nothing on the
        // reuse path.
        template <typename F>
        int add(std::type_index key, F&& make)
        {
            _assertReusable();
            _requireDiscipline(Discipline::Heterogeneous);
            auto it = _free_by_type.find(key);
            if (it != _free_by_type.end() && !it->second.empty())
            {
                int idx = it->second.back(); // LIFO reuse within the type
                it->second.pop_back();
                // Same exception-safety ordering as push(): mark live BEFORE
                // reuse() so a throwing reuse() cannot lose the slot.
                _live[idx] = true;
                _size++;
                _slots[idx]->reuse();
                return idx;
            }

            // Grow by reusing a retained object of the SAME type (no factory
            // call), else invoke the lazy factory.
            auto rit = _reserve_by_type.find(key);
            if (rit != _reserve_by_type.end() && !rit->second.empty())
            {
                _slots.push_back(std::move(rit->second.back()));
                rit->second.pop_back();
                _live.push_back(true);
                _size++;
                _slots.back()->reuse();
                return static_cast<int>(_slots.size()) - 1;
            }

            std::unique_ptr<T> obj = make();
            if (!obj)
                throw Error("add(): type factory returned null");
            if (obj->reuseTypeId() != key)
                throw Error("add(): type factory produced an object of a different type than requested");
            _slots.push_back(std::move(obj));
            _live.push_back(true);
            _size++;
            return static_cast<int>(_slots.size()) - 1;
        }

        // Heterogeneous adoption (adopt-flow): take ownership of a caller-
        // constructed — typically already populated — object. Never reuses a
        // freed slot: the incoming object carries its own payload and the
        // retained occupant of a freed slot could not take it over
        // (HETERO-POOL-DESIGN §2.4). Freed adopted slots DO enter the per-type
        // free lists on remove(), so construct-flow add() of the same type can
        // reuse them later.
        int adopt(std::unique_ptr<T> obj)
        {
            _assertReusable();
            _requireDiscipline(Discipline::Heterogeneous);
            if (!obj)
                throw Error("adopt(): null object");
            _slots.push_back(std::move(obj));
            _live.push_back(true);
            _size++;
            return static_cast<int>(_slots.size()) - 1;
        }

        void remove(int idx)
        {
            checkUsed(idx);
            _slots[idx]->reuse(); // reuse-on-remove: object stays alive
            _live[idx] = false;
            _parkFreedSlot(idx);
            _size--;
        }

        // Empty the WHOLE collection, RETAINING every object for reuse (NEVER
        // destroying it here). Objects move to the reserve; the active spine
        // (_slots) is emptied, so size()/end() become 0 — reproducing the
        // legacy ObjPool<T>::clear()/PtrPool<T>::clear() OBSERVABLE contract
        // (backing emptied, allocation restarts at 0) that consumers depend on
        // (e.g. Graph::clear() must leave vertexEnd()==0, or vertexEnd()-sized
        // arrays like the reaction enumerator's forbidden_atoms desync).
        //
        // The distinction the design mandates: remove(element) and clear()
        // (whole-collection empty) REUSE — they never destroy. Destruction
        // happens only when the whole collection is destroyed (dtor) or purge()
        // is called. This is also the Reusable contract used when this pool is
        // itself a nested element: reuse() returns it to a fresh-constructed
        // (empty) logical state while keeping its objects available for reuse.
        void reuse() override
        {
            for (auto& slot : _slots)
                if (slot)
                    _reserveObject(std::move(slot));
            _slots.clear();
            _live.clear();
            _free.clear();
            _free_by_type.clear();
            _size = 0;
        }

        // Alias used by legacy consumers migrating off ObjPool/PtrPool::clear().
        // Non-destructive: retains objects for reuse (see reuse()).
        void clear()
        {
            reuse();
        }

        // Explicit memory release: DESTROY every object (active + reserved) and
        // release the spine. This is the only non-dtor path that frees objects.
        void purge()
        {
            _slots.clear();
            _slots.shrink_to_fit();
            _reserve.clear();
            _reserve.shrink_to_fit();
            _reserve_by_type.clear();
            _free.clear();
            _free.shrink_to_fit();
            _free_by_type.clear();
            _live.clear();
            _live.shrink_to_fit();
            _size = 0;
        }

        bool hasElement(int idx) const
        {
            return idx >= 0 && idx < static_cast<int>(_slots.size()) && _live[idx];
        }

        int size() const
        {
            return _size;
        }

        // Number of ACTIVE slots (live + freed-but-not-cleared) = the iteration
        // bound end(). After clear()/reuse() this is 0 (objects moved to the
        // reserve); it does NOT count reserved objects (see reserveSize()).
        int capacity() const
        {
            return static_cast<int>(_slots.size());
        }

        // Number of objects retained in the reserve for reuse (populated by
        // clear()/reuse()). Diagnostic/testing accessor.
        int reserveSize() const
        {
            int n = static_cast<int>(_reserve.size());
            for (const auto& kv : _reserve_by_type)
                n += static_cast<int>(kv.second.size());
            return n;
        }

        // Stable reference to the object in slot idx.
        T& operator[](int idx)
        {
#ifndef INDIGO_UNCHECKED_ACCESS
            checkUsed(idx);
#endif
            return *_slots[idx];
        }
        const T& operator[](int idx) const
        {
#ifndef INDIGO_UNCHECKED_ACCESS
            checkUsed(idx);
#endif
            return *_slots[idx];
        }

        T& at(int idx)
        {
            return operator[](idx);
        }
        const T& at(int idx) const
        {
            return operator[](idx);
        }

        int begin() const
        {
            int i = 0;
            for (; i < static_cast<int>(_slots.size()); i++)
                if (_live[i])
                    break;
            return i;
        }

        int next(int i) const
        {
            for (i++; i < static_cast<int>(_slots.size()); i++)
                if (_live[i])
                    break;
            return i;
        }

        int end() const
        {
            return static_cast<int>(_slots.size());
        }

    protected:
        void checkUsed(int idx) const
        {
            if (idx < 0 || idx >= static_cast<int>(_slots.size()) || !_live[idx])
                throw Error("access to unused element %d", idx);
        }

        // The free-list discipline is fixed by the first allocating call and
        // may not change afterwards: the two free structures are not
        // interchangeable, so silently mixing them would corrupt slot reuse.
        enum class Discipline
        {
            Unset,
            Homogeneous,  // push()/add(): single LIFO free list (_free)
            Heterogeneous // add(key, factory)/adopt(): per-type free lists (_free_by_type)
        };

        void _requireDiscipline(Discipline d)
        {
            if (_discipline == Discipline::Unset)
                _discipline = d;
            else if (_discipline != d)
                throw Error("homogeneous (push) and heterogeneous (add/adopt) APIs must not be mixed on one pool");
        }

        // Park a freed slot in the free structure of the active discipline.
        // Heterogeneous slots are bucketed by the object's dynamic type
        // (Reusable::reuseTypeId()) so only a same-type request can reuse them.
        void _parkFreedSlot(int idx)
        {
            if (_discipline == Discipline::Heterogeneous)
                _free_by_type[_slots[idx]->reuseTypeId()].push_back(idx);
            else
                _free.push_back(idx);
        }

        // Move a retained object into the reserve of the active discipline so a
        // later grow can reuse it instead of constructing. Objects go to the
        // reserve on clear()/reuse() (whole-collection empty) — they are NEVER
        // destroyed there, only when the whole collection itself is destroyed
        // (dtor) or purge() is called explicitly.
        void _reserveObject(std::unique_ptr<T> obj)
        {
            if (_discipline == Discipline::Heterogeneous)
                _reserve_by_type[obj->reuseTypeId()].push_back(std::move(obj));
            else
                _reserve.push_back(std::move(obj));
        }

        std::vector<std::unique_ptr<T>> _slots;                   // active owned objects; pointees heap-stable
        std::vector<bool> _live;                                  // per-slot occupancy
        std::deque<int> _free;                                    // homogeneous: freed active slots (LIFO via back)
        std::map<std::type_index, std::deque<int>> _free_by_type; // heterogeneous: freed active slots per dynamic type
        std::vector<std::unique_ptr<T>> _reserve;                 // homogeneous: objects retained across clear() for reuse
        std::map<std::type_index, std::vector<std::unique_ptr<T>>> _reserve_by_type; // heterogeneous: retained per type
        Discipline _discipline = Discipline::Unset;               // fixed by first allocating call
        int _size = 0;                                            // number of live slots
    };

} // namespace indigo

#endif
