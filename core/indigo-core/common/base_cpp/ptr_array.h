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

#ifndef __ptr_array__
#define __ptr_array__

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "base_cpp/exception.h"

// PtrArray<T> — owning, heterogeneous, nullable-slot array of T pointers.
//
// Thread safety: NOT thread-safe. Callers must serialize access externally.
// The underlying `std::vector<std::unique_ptr<T>>` provides no synchronization.
//
// History: previously implemented as `Array<T*>` with manual `delete` in the
// destructor. As part of milestone-19 (issue #783) the internal storage was
// migrated to `std::vector<std::unique_ptr<T>>`. The public API was kept
// source-compatible with one exception: `operator[]` and `at()` now return
// `T*` by value rather than `T*&` (lvalue reference). Direct assignment via
// `arr[i] = ptr` must be replaced with `arr.set(i, ptr)` / `arr.reset(i, ptr)`.
//
// PR-1 (issue #3691) adds ownership-correct overloads and marks the raw-pointer
// overloads deprecated. Migration guide:
//   arr.add(new T(x))         →  arr.emplace(x)  or  arr.add(std::make_unique<T>(x))
//   arr.set(i, new T(x))      →  arr.set(i, std::make_unique<T>(x))
//   arr.reset(i, new T(x))    →  arr.reset(i, std::make_unique<T>(x))
//   T* p = arr.pop()          →  auto p = arr.pop()   (std::unique_ptr<T>)
// NEW in PR-1: arr.release(i) — transfers ownership of slot i out as unique_ptr.
//
// PR-2 (issue #3703) adds reserve()/qsort() as ObjArray-compatible aliases so
// ObjArray<T> client call sites migrate to PtrArray<T> mechanically.
//
// Ownership model: PtrArray owns the pointees. `add()`, `set()`, `reset(idx, p)`
// take ownership of the raw pointer / unique_ptr they receive. `release(idx)`,
// `pop()` transfer ownership out as `std::unique_ptr<T>`. Slots may legally be
// `nullptr` (holes); destruction, clear(), and shrink-resize skip null slots.
//
// Move/copy: nothrow-movable, non-copyable. Use `clone(const PtrArray&)` if a
// deep copy is genuinely needed (must be added explicitly per T).
//
// See also: tests in core/indigo-core/tests/tests/ptr_array.cpp — they pin
// the observable contract.

namespace indigo
{

    DECL_EXCEPTION(PtrArrayError);

    template <typename T>
    class PtrArray final
    {
    public:
        DECL_TPL_ERROR(PtrArrayError);

        PtrArray() = default;
        ~PtrArray() = default;

        PtrArray(PtrArray&&) noexcept = default;
        PtrArray& operator=(PtrArray&&) noexcept = default;

        PtrArray(const PtrArray&) = delete;
        PtrArray& operator=(const PtrArray&) = delete;

        // ----------------------------------------------------------------
        // add() — owning overload (preferred): takes ownership via unique_ptr.
        // Precondition: obj != nullptr. Use expand() if you need null slots.
        // Exception safety: if emplace_back throws (OOM), `obj` is still
        // owned by the caller's unique_ptr and will be destroyed cleanly.
        // ----------------------------------------------------------------
        T& add(std::unique_ptr<T> obj)
        {
            if (!obj)
                throw Error("add(): null unique_ptr is not allowed; use expand() for null slots");
            T* raw = obj.get();
            // emplace_back may throw (OOM) only before the move takes place
            // (vector growth path). unique_ptr's move constructor is noexcept,
            // so if we reach the move step the ownership transfer is atomic.
            // On throw, obj is still intact — its destructor cleans up.
            _items.emplace_back(std::move(obj));
            return *raw;
        }

        // add() — deprecated raw-pointer overload kept for source compatibility.
        // Prefer add(std::unique_ptr<T>) or emplace(args...) for new code.
        // Wraps obj into unique_ptr immediately, then delegates to the owning
        // overload — this closes the OOM-window where a thrown emplace_back
        // would leak obj (the historical `add(new T())` anti-pattern risk).
        [[deprecated("use add(std::unique_ptr<T>) or emplace(...)")]] T& add(T* obj)
        {
            return add(std::unique_ptr<T>(obj));
        }

        // emplace() — constructs T in-place and takes ownership.
        // Strong exception safety: make_unique may throw; the vector is not
        // modified in that case.
        template <class... Args>
        T& emplace(Args&&... args)
        {
            return add(std::make_unique<T>(std::forward<Args>(args)...));
        }

        // push() / push(args...) — ObjArray-compatible alias for emplace().
        // Provided to enable a drop-in source-compatible migration of
        // ObjArray<T> call sites to PtrArray<T> in milestone-19 Step 2:
        // existing `arr.push(a, b)` works unchanged, the object is now
        // heap-allocated and owned by PtrArray.
        template <class... Args>
        T& push(Args&&... args)
        {
            return emplace(std::forward<Args>(args)...);
        }

        // ----------------------------------------------------------------
        // pop() — transfers ownership of the last slot to the caller.
        // The slot is removed from the array. Throws if the array is empty.
        // [[nodiscard]]: discarding the return value would silently leak.
        // ----------------------------------------------------------------
        [[nodiscard]] std::unique_ptr<T> pop()
        {
            if (_items.empty())
                throw Error("stack underflow");
            std::unique_ptr<T> result = std::move(_items.back());
            _items.pop_back();
            return result;
        }

        // Raw pointer to the last slot (does not transfer ownership). Throws
        // if the array is empty — symmetric with pop().
        T* top()
        {
            if (_items.empty())
                throw Error("top(): array is empty");
            return _items.back().get();
        }

        // Grow up to `newsize`, filling new slots with nullptr. Smaller
        // `newsize` is a no-op (does NOT shrink) — matches the historical
        // contract used by some clients to pre-allocate sparse arrays.
        void expand(int newsize)
        {
            if (newsize > static_cast<int>(_items.size()))
                _items.resize(static_cast<std::size_t>(newsize));
        }

        // Destroys all owned objects; size becomes 0. Null slots are skipped
        // (delete-on-nullptr is well-defined). Safe on an already-empty
        // array.
        void clear() noexcept
        {
            _items.clear();
        }

        int size() const noexcept
        {
            return static_cast<int>(_items.size());
        }

        // Grow or shrink to `newsize`. On shrink, deletes the trailing
        // pointees (skipping nulls). On grow, fills new slots with nullptr.
        void resize(int newsize)
        {
            _items.resize(static_cast<std::size_t>(newsize));
        }

        void removeLast()
        {
            _items.pop_back();
        }

        // Delete the pointee at `idx` (skipping if null) and shift later
        // elements left, preserving order.
        void remove(int idx)
        {
            _check_bounds(idx, "remove");
            _items.erase(_items.begin() + idx);
        }

        // ----------------------------------------------------------------
        // set() — owning overload (preferred): places `obj` at slot `idx`.
        // The slot MUST be currently null (throws otherwise).
        // ----------------------------------------------------------------
        void set(int idx, std::unique_ptr<T> obj)
        {
            _check_bounds(idx, "set");
            if (_items[idx] != nullptr)
                throw Error("object #%d already set", idx);
            _items[idx] = std::move(obj);
        }

        // set() — deprecated raw-pointer overload kept for source compatibility.
        // Wraps obj into unique_ptr before delegating, so a throw from the
        // owning overload cleans up obj automatically.
        [[deprecated("use set(int, std::unique_ptr<T>)")]] void set(int idx, T* obj)
        {
            set(idx, std::unique_ptr<T>(obj));
        }

        // Delete the pointee at `idx` (idempotent on null) and null the slot.
        void reset(int idx)
        {
            _check_bounds(idx, "reset");
            _items[idx].reset();
        }

        // ----------------------------------------------------------------
        // reset() with replacement — owning overload (preferred): deletes
        // the pointee at `idx` and takes ownership of `obj`.
        // ----------------------------------------------------------------
        void reset(int idx, std::unique_ptr<T> obj)
        {
            _check_bounds(idx, "reset");
            _items[idx] = std::move(obj);
        }

        // reset() — deprecated raw-pointer overload kept for source compatibility.
        // Wraps obj into unique_ptr before delegating, so a throw from the
        // owning overload cleans up obj automatically.
        [[deprecated("use reset(int, std::unique_ptr<T>)")]] void reset(int idx, T* obj)
        {
            reset(idx, std::unique_ptr<T>(obj));
        }

        // ----------------------------------------------------------------
        // release() — transfers ownership of slot `idx` to the caller as a
        // unique_ptr. The slot becomes null but is NOT removed; size is
        // unchanged. Returns a null unique_ptr if the slot was already null.
        // [[nodiscard]]: discarding the return value would silently leak the
        // pointee (the slot is cleared unconditionally).
        // ----------------------------------------------------------------
        [[nodiscard]] std::unique_ptr<T> release(int idx)
        {
            _check_bounds(idx, "release");
            return std::move(_items[idx]);
        }

        const T* operator[](int idx) const
        {
            return _items[idx].get();
        }

        // NOTE: returns `T*` by value, not by `T*&` (lvalue reference), unlike
        // the historical implementation. Direct `arr[i] = newPtr` is no longer
        // valid — use `arr.set(i, newPtr)` or `arr.reset(i, newPtr)`.
        T* operator[](int idx)
        {
            return _items[idx].get();
        }

        const T* at(int idx) const
        {
            _check_bounds(idx, "at");
            return _items[idx].get();
        }

        T* at(int idx)
        {
            _check_bounds(idx, "at");
            return _items[idx].get();
        }

        // ----------------------------------------------------------------
        // reserve() — ObjArray-compatible alias for milestone-19 (#3703)
        // ObjArray<T>::reserve call sites. Pre-allocates capacity for the
        // unique_ptr slots; does not construct any pointees and does not
        // change size(). No-op for non-positive `to_reserve`.
        // ----------------------------------------------------------------
        void reserve(int to_reserve)
        {
            if (to_reserve > 0)
                _items.reserve(static_cast<std::size_t>(to_reserve));
        }

        // ----------------------------------------------------------------
        // qsort() — ObjArray-compatible sort for milestone-19 (#3703)
        // ObjArray<T>::qsort call sites. Sorts the owned pointees in place
        // using the same `int cmp(T&, T&, void*)` contract as the legacy
        // container (negative => first orders before second). The comparator
        // receives dereferenced pointees, so existing comparators that take
        // `T&`/`const T&` work unchanged. Preserves ownership (only the
        // unique_ptr slots are permuted). Null slots are not expected here —
        // ObjArray never produced them — and would dereference in `cmp`.
        // ----------------------------------------------------------------
        template <typename T1, typename T2>
        void qsort(int (*cmp)(T1, T2, void*), void* context)
        {
            std::sort(_items.begin(), _items.end(),
                      [cmp, context](const std::unique_ptr<T>& a, const std::unique_ptr<T>& b) { return cmp(*a, *b, context) < 0; });
        }

    private:
        std::vector<std::unique_ptr<T>> _items;

        void _check_bounds(int idx, const char* method) const
        {
            if (idx < 0 || idx >= static_cast<int>(_items.size()))
                throw Error("%s(): invalid index %d (size=%d)", method, idx, size());
        }
    };

} // namespace indigo

#endif
