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

#ifndef __reusable_h__
#define __reusable_h__

#include <type_traits>
#include <typeindex>

#include "base_c/defs.h"

namespace indigo
{
    // Reusable — the interface an element type must implement to be stored in
    // PtrReusablePool<T> (issue #3766).
    //
    // reuse() must perform a NON-destructive reset: it returns the object to a
    // freshly-constructed logical state while keeping the object alive and its
    // internal buffers retained for reuse. It is the semantic equivalent of
    // destroying and default-constructing the object, minus the deallocation
    // and reallocation. The pool calls it exactly once per retirement — when a
    // slot is freed by remove() or retired by reuse() — so an implementation
    // must leave the object usable, not merely consistent.
    //
    // The method is a pure virtual so the language itself guarantees every
    // element type declares and defines it (a compile-time marker could not).
    // DLLEXPORT so that dll-interface classes (e.g. Vertex) may derive from it
    // without MSVC C4275, mirroring how indigo::Exception is declared.
    class DLLEXPORT Reusable
    {
    public:
        virtual void reuse() = 0;

        // reuseTypeId() — the slot-type identity used by the heterogeneous mode
        // of PtrReusablePool (per-type free lists): a freed slot may be reused
        // only by a request for the SAME type, since reuse() cannot change an
        // object's dynamic type.
        //
        // The default implementation derives the identity from the dynamic
        // type: Reusable is polymorphic (virtual dtor), so typeid(*this) on it
        // yields the most-derived class. This puts every element type under
        // ONE contract with zero per-class boilerplate and no way to get out
        // of sync with the real type. Override only if a class genuinely needs
        // a custom identity (none is expected to).
        //
        // NOTE: identity must NEVER be based on mutable state (e.g. the
        // SGroup::sgroup_type field) — reuse() would corrupt the free-list key.
        virtual std::type_index reuseTypeId() const
        {
            return std::type_index(typeid(*this));
        }

        virtual ~Reusable() = default;
    };

    // Compile-time gate used by PtrReusablePool and its consumers to require
    // that T actually implements the Reusable interface. Primitive and
    // non-Reusable types (e.g. int) yield false and are rejected by the
    // pool's static_assert.
    template <typename T>
    inline constexpr bool is_reusable_v = std::is_base_of<Reusable, T>::value;

} // namespace indigo

#endif
