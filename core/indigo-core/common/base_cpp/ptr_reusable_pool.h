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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <new>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include "base_cpp/exception.h"
#include "base_cpp/reusable.h"

namespace indigo
{

    DECL_EXCEPTION(PtrReusablePoolError);
    /***************************************************************************
    * PtrReusablePool<T> — sparse owning pool with stable indices and stable
    * object addresses (issue #3766).
    *
    * Contract:
    *  * Indices are handed out from 0 upwards; a freed slot is reused LIFO;
    *    reuse() restarts allocation at 0.
    *  * Reuse, never destroy: remove() and reuse() reset objects via T::reuse()
    *    and keep them. Objects are freed only by purge()/dtor, plus one bounded
    *    exception — the reserve holds at most MAX_RESERVE_PER_BUCKET objects per
    *    bucket and destroys the excess, so memory does not stay pinned at the
    *    all-time high-water mark.
    *  * A T& obtained from operator[] stays valid across later push() calls.
    *  * Growth is exception-safe: the object is fully constructed before any
    *    bookkeeping is mutated.
    *  * The pool is itself Reusable, so pools can nest.
    *
    * Two storage disciplines, one contract:
    *  * HOMOGENEOUS (push()): objects are constructed in place inside chunks
    *    (see Slabs). One allocation covers a whole chunk, neighbouring elements
    *    are contiguous, and construction is lazy — one constructor call per
    *    fresh push. reuse() retires the active range into an IMPLICIT reserve:
    *    the objects stay constructed in their chunks and the next growth
    *    re-activates them index by index with no construction.
    *  * HETEROGENEOUS (add(type_index, factory) / adopt()): polymorphic element
    *    hierarchies (SGroup, MetaObject, BaseMolecule) cannot share in-place
    *    slots of one static type, so objects live behind std::unique_ptr. Freed
    *    slots are parked in per-type free lists keyed by Reusable::reuseTypeId()
    *    because reuse() cannot change an object's dynamic type. add() grows
    *    through a lazy factory, so nothing is constructed on the reuse path;
    *    adopt() appends a caller-constructed, already populated object.
    *
    * The discipline is fixed by the first allocating call; mixing the two on one
    * instance throws.
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

        // A chunk spans roughly one memory page and holds a power-of-two number
        // of slots, capped so that a single chunk never grows unbounded for
        // small element types.
        static constexpr std::size_t CHUNK_TARGET_BYTES = 4096;
        static constexpr std::size_t CHUNK_MAX_SLOTS = 64;

        // ---- Slabs: the storage of the homogeneous discipline ---------------
        //
        // Why a hand-rolled store and not a standard container: the pooled
        // element types (Vertex, TGroup, Cycle, _AttachmentPoint, ...) are
        // neither movable nor copyable — each declares a destructor, which
        // suppresses the implicit move constructor, and holds Array/List/
        // PtrArray members that declare a move constructor, which deletes the
        // implicit copy constructor. Therefore:
        //  * std::vector<T> does not compile for them at all: growth requires
        //    MoveInsertable. The legacy ObjPool<T> worked around that by
        //    placement-new into Pool<T>'s Array<T> plus std::realloc, i.e. it
        //    relocated non-trivial objects byte-wise — undefined behaviour, and
        //    the reason references into the old pool could dangle.
        //  * std::deque<T> does compile (emplace_back needs no move) and gives
        //    the same reference stability, but its block size is unspecified:
        //    the MSVC STL uses one element per block once sizeof(T) exceeds 8,
        //    degenerating into one allocation per object, while libstdc++ uses
        //    512-byte blocks. A platform-dependent allocation profile is not
        //    acceptable here.
        //  * std::vector<std::unique_ptr<T>> also costs one allocation per
        //    object, which profiling identified as the dominant cost of the hot
        //    parse and substructure-match paths.
        // So: chunks of raw storage, one allocation per slotsPerChunk() objects,
        // lazy placement-new, explicit destruction. The ceremony this requires —
        // alignment, std::launder, destructor calls, chunk release — is confined
        // to this class; the pool itself only asks for addresses.
        class Slabs
        {
        public:
            // A constexpr FUNCTION (not a static data member) so sizeof(T) is
            // evaluated lazily — only where a homogeneous body is instantiated
            // and T is therefore complete.
            static constexpr std::size_t slotsPerChunk()
            {
                std::size_t slots = 1;
                while (slots < CHUNK_MAX_SLOTS && (slots * 2) * sizeof(T) <= CHUNK_TARGET_BYTES)
                    slots *= 2;
                return slots;
            }

            ~Slabs()
            {
                destroyFrom(0);
            }

            // Slots [0..constructed()) hold constructed objects.
            int constructed() const
            {
                return _constructed;
            }

            // Address of the constructed object in slot idx. std::launder is
            // required when forming a pointer to a placement-new'd object
            // through the raw-storage address.
            T* ptr(int idx) const
            {
                Chunk* chunk = _chunks[static_cast<std::size_t>(idx) / slotsPerChunk()].get();
                return std::launder(reinterpret_cast<T*>(chunk->raw) + static_cast<std::size_t>(idx) % slotsPerChunk());
            }

            // Append one object. A throwing constructor leaves the store
            // unchanged: the chunk may already be allocated, but that is spare
            // capacity, not an object.
            void construct(int idx)
            {
                assert(idx == _constructed); // slots are constructed in order
                const std::size_t chunk_idx = static_cast<std::size_t>(idx) / slotsPerChunk();
                if (chunk_idx >= _chunks.size())
                    // `new Chunk` default-initializes: the raw byte array stays
                    // uninitialized on purpose (make_unique would value-
                    // initialize it, i.e. memset a whole page).
                    _chunks.push_back(std::unique_ptr<Chunk>(new Chunk));
                Chunk& chunk = *_chunks[chunk_idx];
                new (chunk.raw + (static_cast<std::size_t>(idx) % slotsPerChunk()) * sizeof(T)) T();
                _constructed++;
            }

            // Destroy the objects in [keep..constructed()) and release the
            // chunks that become empty. destroyFrom(0) releases everything.
            void destroyFrom(int keep)
            {
                if (keep >= _constructed)
                    return;
                for (int i = _constructed - 1; i >= keep; i--)
                    ptr(i)->~T();
                _constructed = keep;
                const std::size_t needed_chunks = (static_cast<std::size_t>(keep) + slotsPerChunk() - 1) / slotsPerChunk();
                while (_chunks.size() > needed_chunks)
                    _chunks.pop_back();
            }

        private:
            // alignas(T) + unsigned char[] is the recommended raw-storage
            // pattern (std::aligned_storage is deprecated).
            struct Chunk
            {
                alignas(T) unsigned char raw[sizeof(T) * slotsPerChunk()];
            };

            std::vector<std::unique_ptr<Chunk>> _chunks; // chunks never move while in use
            int _constructed = 0;
        };

    public:
        DECL_TPL_ERROR(PtrReusablePoolError);

        // Initial spine reserve: a freshly constructed pool has room for a first
        // couple of elements. It is deliberately small, because most pools owned
        // by a molecule stay empty or hold single digits and every constructed
        // pool pays this reserve. It is NOT the growth base — growth starts from
        // the size the discipline actually needs (see _growLive/_growSpine), so
        // a small initial reserve costs no extra reallocations later.
        static constexpr int DEFAULT_RESERVE = 2;

        // Upper bound on retired objects kept per reuse bucket. The reserve
        // keeps up to this many objects for the load-clear-reload reuse win;
        // retiring more destroys the excess so memory stays bounded. Homogeneous
        // pools have one bucket; heterogeneous pools cap each per-type bucket
        // independently.
        static constexpr int MAX_RESERVE_PER_BUCKET = 64;

        PtrReusablePool()
        {
            // Pre-reserve the occupancy spine shared by both disciplines. The
            // discipline-specific storage is pre-reserved when the discipline is
            // fixed (_requireDiscipline): _slots for heterogeneous; homogeneous
            // needs no _slots at all (objects live in chunks).
            _live.reserve(DEFAULT_RESERVE);
        }

        ~PtrReusablePool() override
        {
            _slabs.destroyFrom(0);
        }

        // Rule of Five: the pool owns unique heap objects and stable indices, so
        // it is neither copyable nor movable — the identity of the container is
        // meaningful and must not be relocated. Declared explicitly for intent.
        PtrReusablePool(const PtrReusablePool&) = delete;
        PtrReusablePool& operator=(const PtrReusablePool&) = delete;
        PtrReusablePool(PtrReusablePool&&) = delete;
        PtrReusablePool& operator=(PtrReusablePool&&) = delete;

        // Allocate a slot and return its index: reuse a freed slot, re-activate
        // an object retained across a prior reuse(), or construct a fresh one.
        int push()
        {
            _assertReusable();
            _requireDiscipline(Discipline::Homogeneous);
            if (!_free.empty())
            {
                int idx = _free.back(); // LIFO
                _free.pop_back();
                // Single-reset contract: remove() already reset the object when
                // it parked the slot, and a parked slot cannot be reached
                // through checked access, so re-issuing it is pure bookkeeping.
                _live[idx] = true;
                _size++;
                return idx;
            }

            const int idx = static_cast<int>(_live.size());
            _ensureLiveCapacity(); // throws before anything is mutated

            // An object retained across a prior reuse() (implicit reserve): it
            // was reset on retirement, so no construction and no reset here.
            if (idx < _slabs.constructed())
            {
                _live.push_back(1); // nothrow after _ensureLiveCapacity()
                _size++;
                return idx;
            }

            _slabs.construct(idx);
            _live.push_back(1);
            _size++;
            return idx;
        }

        // Heterogeneous allocation: reuse a freed slot of the SAME dynamic type,
        // or grow. The factory runs only when no slot of `key` is available, so
        // call sites like add(key, [&]{ return mol.neu(); }) construct nothing
        // on the reuse path.
        template <typename F>
        int add(std::type_index key, F&& make)
        {
            _assertReusable();
            _requireDiscipline(Discipline::Heterogeneous);
            auto it = _free_by_type.find(key);
            if (it != _free_by_type.end() && !it->second.empty())
            {
                int idx = it->second.back(); // LIFO within the type
                it->second.pop_back();
                _live[idx] = true; // single-reset contract, as in push()
                _size++;
                return idx;
            }

            // Grow by reusing a retained object of the SAME type (no factory
            // call); reserved objects were already reset on retirement.
            auto rit = _reserve_by_type.find(key);
            if (rit != _reserve_by_type.end() && !rit->second.empty())
            {
                _ensureSpineCapacity();
                _slots.push_back(std::move(rit->second.back()));
                rit->second.pop_back();
                _live.push_back(1);
                _size++;
                return static_cast<int>(_slots.size()) - 1;
            }

            std::unique_ptr<T> obj = std::forward<F>(make)();
            if (!obj)
                throw Error("add(): type factory returned null");
            if (obj->reuseTypeId() != key)
                throw Error("add(): type factory produced an object of a different type than requested");
            _ensureSpineCapacity();
            _slots.push_back(std::move(obj));
            _live.push_back(1);
            _size++;
            return static_cast<int>(_slots.size()) - 1;
        }

        // Take ownership of a caller-constructed, typically already populated
        // object. Never reuses a freed slot: the incoming object carries its own
        // payload, which the retained occupant of a freed slot could not take
        // over. Freed adopted slots DO enter the per-type free lists on
        // remove(), so a later add() of the same type can reuse them.
        int adopt(std::unique_ptr<T> obj)
        {
            _assertReusable();
            // Validate BEFORE fixing the discipline: a rejected adopt(nullptr)
            // must not permanently lock a pool that has allocated nothing yet
            // (which would then reject a later push()).
            if (!obj)
                throw Error("adopt(): null object");
            _requireDiscipline(Discipline::Heterogeneous);
            _ensureSpineCapacity();
            _slots.push_back(std::move(obj));
            _live.push_back(1);
            _size++;
            return static_cast<int>(_slots.size()) - 1;
        }

        void remove(int idx)
        {
            checkUsed(idx);
            _objPtr(idx)->reuse(); // the object stays alive
            _live[idx] = false;
            _parkFreedSlot(idx);
            _size--;
        }

        // Empty the whole collection, RETAINING the objects for reuse. The
        // active spine is emptied, so size() and end() become 0 and allocation
        // restarts at 0 — consumers depend on that (e.g. Graph::clear() must
        // leave vertexEnd() == 0, or vertexEnd()-sized arrays desync).
        //
        // Homogeneous: retired objects stay constructed in their chunks and form
        // the implicit reserve. Heterogeneous: objects move into per-type
        // reserve buckets. In both cases the reserve is bounded by
        // MAX_RESERVE_PER_BUCKET and the excess is destroyed here — the only
        // destruction outside purge()/dtor.
        void reuse() override
        {
            if (_discipline == Discipline::Heterogeneous)
            {
                for (int i = 0; i < static_cast<int>(_slots.size()); i++)
                    if (_slots[i])
                        // Parked slots were already reset by remove().
                        _reserveObject(std::move(_slots[i]), /* needs_reset = */ _live[i] != 0);
                _slots.clear();
                _free_by_type.clear();
            }
            else
            {
                // Reset the retained objects on retirement so the reserve holds
                // reusable capacity buffers, not payloads. Parked slots were
                // already reset by remove(); objects beyond the reserve cap are
                // about to be destroyed and need no reset.
                const int end_active = static_cast<int>(_live.size());
                const int reset_upto = end_active < MAX_RESERVE_PER_BUCKET ? end_active : MAX_RESERVE_PER_BUCKET;
                for (int i = 0; i < reset_upto; i++)
                    if (_live[i])
                        _slabs.ptr(i)->reuse();
                _slabs.destroyFrom(MAX_RESERVE_PER_BUCKET);
                _free.clear();
            }
            _live.clear();
            _size = 0;
        }

        // Explicit memory release: DESTROY every object (active and reserved)
        // and release all storage.
        void purge()
        {
            _slabs.destroyFrom(0);
            _slots.clear();
            _slots.shrink_to_fit();
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
            return idx >= 0 && idx < static_cast<int>(_live.size()) && _live[idx];
        }

        int size() const
        {
            return _size;
        }

        // Diagnostics (tests): number of objects held in the reserve.
        int reserveSize() const
        {
            if (_discipline == Discipline::Heterogeneous)
            {
                int n = 0;
                for (const auto& kv : _reserve_by_type)
                    n += static_cast<int>(kv.second.size());
                return n;
            }
            return _slabs.constructed() - static_cast<int>(_live.size());
        }

        // Stable reference to the object in slot idx.
        T& operator[](int idx)
        {
#ifndef INDIGO_UNCHECKED_ACCESS
            checkUsed(idx);
#endif
            return *_objPtr(idx);
        }
        const T& operator[](int idx) const
        {
#ifndef INDIGO_UNCHECKED_ACCESS
            checkUsed(idx);
#endif
            return *_objPtr(idx);
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
            for (; i < static_cast<int>(_live.size()); i++)
                if (_live[i])
                    break;
            return i;
        }

        int next(int i) const
        {
            for (i++; i < static_cast<int>(_live.size()); i++)
                if (_live[i])
                    break;
            return i;
        }

        // Iteration bound: one past the last active slot. After reuse() this is
        // 0; it does not count reserved objects.
        int end() const
        {
            return static_cast<int>(_live.size());
        }

    protected:
        void checkUsed(int idx) const
        {
            if (idx < 0 || idx >= static_cast<int>(_live.size()) || !_live[idx])
                throw Error("access to unused element %d", idx);
        }

        // The free-list discipline is fixed by the first allocating call and
        // may not change afterwards: the two free structures are not
        // interchangeable, so silently mixing them would corrupt slot reuse.
        enum class Discipline
        {
            Unset,
            Homogeneous,  // push(): in-place chunks + single LIFO free list (_free)
            Heterogeneous // add(key, factory)/adopt(): unique_ptr slots + per-type free lists
        };

        void _requireDiscipline(Discipline d)
        {
            if (_discipline == Discipline::Unset)
            {
                _discipline = d;
                // Pre-reserve the spine this discipline actually uses: the
                // homogeneous one stores objects in chunks and never populates
                // _slots.
                if (d == Discipline::Heterogeneous)
                    _slots.reserve(DEFAULT_RESERVE);
            }
            else if (_discipline != d)
                throw Error("homogeneous (push) and heterogeneous (add/adopt) APIs must not be mixed on one pool");
        }

        // Object address for either discipline. The branch is perfectly
        // predictable (constant per pool instance).
        T* _objPtr(int idx) const
        {
            if (_discipline == Discipline::Heterogeneous)
                return _slots[idx].get();
            return _slabs.ptr(idx);
        }

        // ---- growth --------------------------------------------------------
        // reserve(n) allocates exactly n, so reserving size()+1 would degrade
        // growth to one reallocation per push; doubling keeps it logarithmic.
        // Hot/cold split: the almost-always-false capacity check stays small
        // enough to inline into the allocation paths, while the growth itself
        // lives in a separate method so it does not consume the callers' inline
        // budget (a fat inline body here measurably flipped unrelated inlining
        // decisions in the graph translation units).

        void _ensureLiveCapacity()
        {
            if (_live.size() + 1 > _live.capacity())
                _growLive(_live.size() + 1);
        }

        // Homogeneous spine. The growth base is a whole chunk, NOT
        // DEFAULT_RESERVE: a chunk covering slotsPerChunk() objects has already
        // been allocated for those slots, and the matching occupancy spine costs
        // one byte per slot. Reserving less would only buy extra reallocations
        // on the hottest path.
        void _growLive(std::size_t need)
        {
            const std::size_t base = Slabs::slotsPerChunk();
            std::size_t cap = _live.capacity() < base ? base : _live.capacity() * 2;
            while (cap < need)
                cap *= 2;
            _live.reserve(cap);
        }

        // Heterogeneous spine: keep _slots and _live growable in lockstep so the
        // paired push_backs after the check cannot reallocate, making the pair
        // effectively atomic. If a reserve() throws, nothing has been appended.
        void _ensureSpineCapacity()
        {
            const std::size_t need = _slots.size() + 1;
            if (need > _slots.capacity() || _live.capacity() < need)
                _growSpine(need);
        }

        void _growSpine(std::size_t need)
        {
            if (need > _slots.capacity())
            {
                std::size_t cap = _slots.capacity() < DEFAULT_RESERVE ? static_cast<std::size_t>(DEFAULT_RESERVE) : _slots.capacity() * 2;
                while (cap < need)
                    cap *= 2;
                _slots.reserve(cap);
            }
            if (_live.capacity() < need)
                _live.reserve(_slots.capacity()); // keep both spines in lockstep
        }

        // Park a freed slot in the free structure of the active discipline.
        // Heterogeneous slots are bucketed by the object's dynamic type so only
        // a same-type request can reuse them.
        void _parkFreedSlot(int idx)
        {
            if (_discipline == Discipline::Heterogeneous)
                _free_by_type[_slots[idx]->reuseTypeId()].push_back(idx);
            else
                _free.push_back(idx);
        }

        // Heterogeneous only: move a retained object into its per-type reserve
        // bucket so a later grow can reuse it instead of constructing. Objects
        // beyond the bucket cap are destroyed here (unique_ptr frees on return)
        // rather than pinning the high-water mark forever. needs_reset is false
        // for parked slots that remove() already reset.
        void _reserveObject(std::unique_ptr<T> obj, bool needs_reset)
        {
            std::vector<std::unique_ptr<T>>& bucket = _reserve_by_type[obj->reuseTypeId()];
            if (static_cast<int>(bucket.size()) >= MAX_RESERVE_PER_BUCKET)
                return;
            // The reserve must hold only the reusable buffers of a retired
            // object, not its payload, so grow-from-reserve hands out an
            // already-pristine object.
            if (needs_reset)
                obj->reuse();
            bucket.push_back(std::move(obj));
        }

        // ---- storage -------------------------------------------------------
        std::vector<std::unique_ptr<T>> _slots; // heterogeneous: active owned objects; pointees heap-stable
        std::vector<uint8_t> _live;             // per-slot occupancy for BOTH disciplines; its size() is the
                                                // active spine bound = end(). (uint8_t, not vector<bool>: the
                                                // bit proxy costs on every checkUsed()/begin()/next() access.)
        std::vector<int> _free;                 // homogeneous: freed active slots, LIFO stack via back()
        std::unordered_map<std::type_index, std::vector<int>> _free_by_type; // heterogeneous: freed slots per dynamic type
        Slabs _slabs;                                                        // homogeneous: in-place object storage. Slots
                                                                             // [_live.size().._slabs.constructed()) are the implicit reserve.
        std::unordered_map<std::type_index, std::vector<std::unique_ptr<T>>> _reserve_by_type; // heterogeneous: retained per type
        Discipline _discipline = Discipline::Unset;                                            // fixed by first allocating call
        int _size = 0;                                                                         // number of live slots
    };

} // namespace indigo

#endif
