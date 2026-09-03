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

#include <cstdint>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base_cpp/exception.h"
#include "base_cpp/reusable.h"

namespace indigo
{

    DECL_EXCEPTION(PtrReusablePoolError);

    /***************************************************************************
     * Sparse owning pool of Reusable elements, addressed by stable int indices.
     *
     * Indices are handed out from 0 upwards and a freed one is recycled LIFO;
     * clear() empties the pool and restarts allocation at 0. A T& obtained from
     * operator[]/at() stays valid across later add() calls: the elements are
     * heap-allocated and never relocated. Iteration follows slot order, so the
     * order consumers observe (and save to files) is insertion order, never
     * hash order.
     *
     * Lifetime: remove() and clear() do not destroy an element, they reset it
     * through T::reuse() and keep it for a later add(). Elements are destroyed by
     * purge() and the destructor, and by clear() only for the part of a reuse
     * bucket beyond MAX_RESERVE_PER_BUCKET.
     *
     * The pool constructs every element itself; a caller never hands in a
     * ready-made object. Two families of add do that, and one instance must use
     * only one of them (mixing throws):
     *  - add(args...)/push(args...) for a single element type. The element is
     *    initialized with T::reuse(args...) on every path, so a fresh and a
     *    recycled slot are indistinguishable to the caller.
     *  - add_t(factory)/push_t(factory) for a polymorphic hierarchy, where the
     *    caller fills the returned element in place. A freed slot is recycled
     *    only by a request for the same dynamic type, because a reset cannot
     *    change the type of an object.
     *
     * Design — the pool separates its two roles the way an allocator does:
     *  - _slots is the address space. It resolves an index to an object in O(1)
     *    and defines end(). It cannot be a type-keyed container: pool indices
     *    are values that outlive any pool call — consumers store them in their
     *    own data (Edge ends, atom fields, C-API handles) and size dense arrays
     *    by end() — and elements are dereferenced far more often than allocated,
     *    while the dynamic type is known only at allocation, never at access.
     *  - Spares is the allocator satellite, consulted only when a slot is
     *    allocated or retired. The standard family has exactly one element
     *    type, so it keeps a single Spares and its allocation path never
     *    hashes; the custom family keys Spares by dynamic type, one lookup
     *    per add_t().
     *  - std::unique_ptr backing is the simplest storage that keeps references
     *    stable: the legacy pools relocated live objects with realloc, which is
     *    undefined behaviour for non-trivial types.
     *
     * The pool is itself Reusable, so pools can nest.
     ****************************************************************************/

    template <class T>
    class PtrReusablePool : public Reusable
    {
    public:
        DECL_TPL_ERROR(PtrReusablePoolError);

        // How to make an element of one dynamic type. The key is known without
        // calling `make`, which is what lets add_t() recycle a slot without
        // constructing anything. `context` is the argument passed to `make`, so a
        // Factory built by a static class method needs no allocation even when it
        // has to consult an existing object (sample.neu()).
        struct Factory
        {
            std::type_index key;
            std::unique_ptr<T> (*make)(void* context);
            void* context = nullptr;
        };

        // Room reserved by the constructor. Deliberately small: most pools stay
        // empty or hold single digits, and every constructed pool pays it.
        static constexpr int DEFAULT_RESERVE = 2;

        // Retired elements kept per reuse bucket; the excess is destroyed, so a
        // cleared pool does not pin the all-time high-water mark.
        static constexpr int MAX_RESERVE_PER_BUCKET = 64;

        PtrReusablePool()
        {
            // Checked here and not at class scope so that a pool can be declared
            // with a forward-declared element type; T is complete by the time an
            // owner is constructed.
            static_assert(is_reusable_v<T>, "T must implement the Reusable interface (inherit Reusable + define reuse())");
            _slots.reserve(DEFAULT_RESERVE);
            _live.reserve(DEFAULT_RESERVE);
        }

        ~PtrReusablePool() override = default;

        // Neither copyable nor movable: consumers hold indices and references into
        // a specific pool instance.
        PtrReusablePool(const PtrReusablePool&) = delete;
        PtrReusablePool& operator=(const PtrReusablePool&) = delete;
        PtrReusablePool(PtrReusablePool&&) = delete;
        PtrReusablePool& operator=(PtrReusablePool&&) = delete;

        // Allocates a slot and returns its index. The element is initialized with
        // T::reuse(args...) whether the slot is recycled or new.
        template <class... Args>
        int add(Args&&... args)
        {
            _requireFamily(Family::Standard);
            const int idx = _acquireStandardSlot();
            _slots[idx]->reuse(std::forward<Args>(args)...);
            return idx;
        }

        // add() that returns the element instead of its index.
        template <class... Args>
        T& push(Args&&... args)
        {
            return *_slots[add(std::forward<Args>(args)...)];
        }

        // Allocates a slot for an element of factory.key and returns its index.
        // factory.make() runs only when no slot of that type can be recycled.
        // With arguments the element is initialized by T::reuse(args...) on
        // every path, exactly like add(); without them the caller fills the
        // element in place.
        template <class... Args>
        int add_t(const Factory& factory, Args&&... args)
        {
            return add_t(
                factory.key, [&factory] { return factory.make(factory.context); }, std::forward<Args>(args)...);
        }

        // Overload for a maker that captures.
        template <class F, class... Args>
        int add_t(std::type_index key, F&& make, Args&&... args)
        {
            _requireFamily(Family::Custom);
            constexpr bool initializing = sizeof...(Args) > 0;
            int idx = -1;
            auto it = _spares_by_type.find(key); // the only type lookup on this path
            if (it != _spares_by_type.end())
            {
                Spares& spares = it->second;
                if (!spares.holes.empty())
                {
                    idx = spares.holes.back(); // LIFO within the type
                    spares.holes.pop_back();
                    _activate(idx);
                    if (!initializing)
                        _slots[idx]->reuse();
                }
                else if (!spares.detached.empty())
                {
                    std::unique_ptr<T> retained = std::move(spares.detached.back());
                    spares.detached.pop_back();
                    idx = _appendSlot(std::move(retained)); // reset when it was retired
                }
            }
            if (idx < 0)
                idx = _appendSlot(_make(key, std::forward<F>(make)));
            if constexpr (initializing)
                _slots[idx]->reuse(std::forward<Args>(args)...);
            return idx;
        }

        // add_t() that returns the element, so a caller can fill it in place:
        //     BaseMolecule& fragment = fragments.push_t(factory);
        template <class... Args>
        T& push_t(const Factory& factory, Args&&... args)
        {
            return *_slots[add_t(factory, std::forward<Args>(args)...)];
        }

        template <class F, class... Args>
        T& push_t(std::type_index key, F&& make, Args&&... args)
        {
            return *_slots[add_t(key, std::forward<F>(make), std::forward<Args>(args)...)];
        }

        void remove(int idx)
        {
            checkUsed(idx);
            _slots[idx]->reuse(); // the element stays alive
            _live[idx] = 0;
            _size--;
            _parkFreedSlot(idx);
        }

        // Empties the pool: allocation restarts at 0 and end() becomes 0, which
        // consumers rely on (Graph::clear() must leave vertexEnd() == 0, or
        // arrays sized by vertexEnd() desync).
        void clear()
        {
            reuse();
        }

        // Resetting a pool means emptying it, so a nested pool needs no separate
        // handling.
        void reuse() override
        {
            for (int i = 0; i < static_cast<int>(_slots.size()); i++)
                if (_slots[i])
                    // A slot freed by remove() was reset when it was parked; a
                    // live one is reset on its way to the reserve.
                    _retain(std::move(_slots[i]), /* needs_reset = */ _live[i] != 0);
            _slots.clear();
            _live.clear();
            _spares.holes.clear();
            for (auto& entry : _spares_by_type)
                entry.second.holes.clear();
            _size = 0;
        }

        // Destroys every element, active and retained, and releases the memory.
        void purge()
        {
            _slots.clear();
            _slots.shrink_to_fit();
            _live.clear();
            _live.shrink_to_fit();
            _spares.holes.clear();
            _spares.holes.shrink_to_fit();
            _spares.detached.clear();
            _spares.detached.shrink_to_fit();
            _spares_by_type.clear();
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

        // Elements currently held for reuse. Diagnostic accessor.
        int reserveSize() const
        {
            int n = static_cast<int>(_spares.detached.size());
            for (const auto& entry : _spares_by_type)
                n += static_cast<int>(entry.second.detached.size());
            return n;
        }

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

        // Index walk over the live slots: begin()/next() skip freed ones, end() is
        // one past the last slot and is NOT the number of live elements.
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

        // Range views over the live slots:
        //     for (auto& elem : pool.items())
        //     for (int idx : pool.indices())

        template <class PoolRef, class ElemRef>
        class ElementIterator
        {
        public:
            ElementIterator(PoolRef pool, int idx) : _pool(pool), _idx(idx)
            {
            }
            ElemRef operator*() const
            {
                return _pool->at(_idx);
            }
            ElementIterator& operator++()
            {
                _idx = _pool->next(_idx);
                return *this;
            }
            bool operator!=(const ElementIterator& other) const
            {
                return _idx != other._idx;
            }

        private:
            PoolRef _pool;
            int _idx;
        };

        class IndexIterator
        {
        public:
            IndexIterator(const PtrReusablePool* pool, int idx) : _pool(pool), _idx(idx)
            {
            }
            int operator*() const
            {
                return _idx;
            }
            IndexIterator& operator++()
            {
                _idx = _pool->next(_idx);
                return *this;
            }
            bool operator!=(const IndexIterator& other) const
            {
                return _idx != other._idx;
            }

        private:
            const PtrReusablePool* _pool;
            int _idx;
        };

        template <class PoolRef, class IterT>
        class Range
        {
        public:
            explicit Range(PoolRef pool) : _pool(pool)
            {
            }
            IterT begin() const
            {
                return IterT(_pool, _pool->begin());
            }
            IterT end() const
            {
                return IterT(_pool, _pool->end());
            }

        private:
            PoolRef _pool;
        };

        using ItemsRange = Range<PtrReusablePool*, ElementIterator<PtrReusablePool*, T&>>;
        using ConstItemsRange = Range<const PtrReusablePool*, ElementIterator<const PtrReusablePool*, const T&>>;
        using IndicesRange = Range<const PtrReusablePool*, IndexIterator>;

        ItemsRange items()
        {
            return ItemsRange(this);
        }
        ConstItemsRange items() const
        {
            return ConstItemsRange(this);
        }
        IndicesRange indices() const
        {
            return IndicesRange(this);
        }

    protected:
        void checkUsed(int idx) const
        {
            if (idx < 0 || idx >= static_cast<int>(_slots.size()) || !_live[idx])
                throw Error("access to unused element %d", idx);
        }

        // The spare structures of the two families are not interchangeable, so
        // silently mixing them would corrupt slot recycling.
        enum class Family
        {
            Unset,
            Standard, // add()/push()
            Custom    // add_t()/push_t()
        };

        void _requireFamily(Family f)
        {
            if (_family == Family::Unset)
                _family = f;
            else if (_family != f)
                throw Error("add()/push() and add_t()/push_t() must not be mixed on one pool");
        }

        // Recycles a freed slot, takes back an element retained across a clear(),
        // or constructs a fresh one. Does not initialize it: add() applies
        // T::reuse(args...) to whatever this returns.
        int _acquireStandardSlot()
        {
            if (!_spares.holes.empty())
            {
                const int idx = _spares.holes.back(); // LIFO
                _spares.holes.pop_back();
                _activate(idx);
                return idx;
            }
            if (!_spares.detached.empty())
            {
                std::unique_ptr<T> retained = std::move(_spares.detached.back());
                _spares.detached.pop_back();
                return _appendSlot(std::move(retained));
            }
            // Construct first: if the constructor throws, no bookkeeping has
            // been touched and the pool is unchanged.
            return _appendSlot(std::make_unique<T>());
        }

        template <class F>
        std::unique_ptr<T> _make(std::type_index key, F&& make)
        {
            std::unique_ptr<T> obj = std::forward<F>(make)();
            if (!obj)
                throw Error("add_t(): factory returned null");
            if (obj->reuseTypeId() != key)
                throw Error("add_t(): factory produced an object of a different type than requested");
            return obj;
        }

        // Appends a new slot. If the occupancy push_back throws, the slot is rolled
        // back, so the two vectors never desync.
        int _appendSlot(std::unique_ptr<T> obj)
        {
            _slots.push_back(std::move(obj));
            try
            {
                _live.push_back(1);
            }
            catch (...)
            {
                _slots.pop_back();
                throw;
            }
            _size++;
            return static_cast<int>(_slots.size()) - 1;
        }

        // Bookkeeping is completed before the element is initialized, so a throwing
        // T::reuse() leaves the slot live rather than lost from both the free list
        // and the live set.
        void _activate(int idx)
        {
            _live[idx] = 1;
            _size++;
        }

        void _parkFreedSlot(int idx)
        {
            if (_family == Family::Custom)
                _spares_by_type[_slots[idx]->reuseTypeId()].holes.push_back(idx);
            else
                _spares.holes.push_back(idx);
        }

        // Keeps a retired element for reuse, up to MAX_RESERVE_PER_BUCKET; beyond
        // that it is destroyed when this unique_ptr goes out of scope.
        void _retain(std::unique_ptr<T> obj, bool needs_reset)
        {
            std::vector<std::unique_ptr<T>>& bucket = (_family == Family::Custom) ? _spares_by_type[obj->reuseTypeId()].detached : _spares.detached;
            if (static_cast<int>(bucket.size()) >= MAX_RESERVE_PER_BUCKET)
                return;
            // The reserve keeps only the buffers of a retired element, not its
            // payload, so a later add() hands out a pristine one.
            if (needs_reset)
                obj->reuse();
            bucket.push_back(std::move(obj));
        }

        // Spare capacity of one element type: indices of slots freed by remove()
        // — their objects stay in _slots — plus elements detached by clear().
        // The standard family has a single element type, so it holds one Spares
        // directly and never touches a hash or typeid; the custom family keys
        // them by dynamic type, because a slot may be recycled only by a request
        // for the same type.
        struct Spares
        {
            std::vector<int> holes;                   // freed slot indices, LIFO stack via back()
            std::vector<std::unique_ptr<T>> detached; // elements retained across clear()
        };

        std::vector<std::unique_ptr<T>> _slots;                      // owned objects; the pointees never move
        std::vector<uint8_t> _live;                                  // per-slot occupancy (uint8_t, not vector<bool>:
                                                                     // the bit proxy costs on every access)
        Spares _spares;                                              // standard family
        std::unordered_map<std::type_index, Spares> _spares_by_type; // custom family
        Family _family = Family::Unset;                              // fixed by the first allocating call
        int _size = 0;                                               // number of live slots
    };

} // namespace indigo

#endif
