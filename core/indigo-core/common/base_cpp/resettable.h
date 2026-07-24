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

#ifndef __resettable_h__
#define __resettable_h__

#include <type_traits>

namespace indigo
{
    // Resettable — the interface an element type must implement to be stored in
    // PtrReusablePool<T> (milestone 19, RFC §4.4, task #3766).
    //
    // reset() must perform a NON-destructive reset: it returns the object to a
    // freshly-constructed logical state while keeping the object alive and its
    // internal buffers retained for reuse. It is invoked by the pool on remove
    // (reset-on-remove) and again when the freed slot is handed back out by
    // push(). It is the semantic equivalent of destroying and default-
    // constructing the object, minus the deallocation/reallocation.
    //
    // The method is a pure virtual so the language itself guarantees every
    // element type declares and defines it (a compile-time marker could not).
    class Resettable
    {
    public:
        virtual void reset() = 0;
        virtual ~Resettable() = default;
    };

    // Compile-time gate used by PtrReusablePool and its consumers to require
    // that T actually implements the Resettable interface. Primitive and
    // non-Resettable types (e.g. int) yield false and are rejected by the
    // pool's static_assert (carve-outs, RFC §0/§4.4).
    template <typename T>
    inline constexpr bool is_resettable_v = std::is_base_of<Resettable, T>::value;

} // namespace indigo

#endif
