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

#ifndef __obj_h__
#define __obj_h__

#include "base_cpp/exception.h"

namespace indigo
{

    DECL_EXCEPTION(ObjError);

    // Reusable storage for object
    template <typename T>
    class Obj
    {
    public:
        Obj() : _initialized(false)
        {
        }

        ~Obj()
        {
            free();
        }

        T* get() const
        {
            if (!_initialized)
                return 0;
            return _ptr();
        }

        T* operator->() const
        {
            if (!_initialized)
                throw Error("no object");

            return _ptr();
        }

        T& ref() const
        {
            if (!_initialized)
                throw Error("no object");

            return *_ptr();
        }

        T& create()
        {
            if (_initialized)
                throw Error("create(): already have object");

            new (_storage) T();
            _initialized = true;
            return *_ptr();
        }

        template <typename A>
        T& create(A& a)
        {
            if (_initialized)
                throw Error("create(): already have object");

            new (_storage) T(a);
            _initialized = true;
            return *_ptr();
        }

        template <typename A>
        T& recreate(A& a)
        {
            free();
            return create(a);
        }

        template <typename A>
        T& create(const A& a)
        {
            if (_initialized)
                throw Error("create(): already have object");

            new (_storage) T(a);
            _initialized = true;
            return *_ptr();
        }

        template <typename A, typename B>
        T& create(A& a, B& b)
        {
            if (_initialized)
                throw Error("create(): already have object");

            new (_storage) T(a, b);
            _initialized = true;
            return *_ptr();
        }

        template <typename A, typename B>
        T& create(A& a, B* b)
        {
            if (_initialized)
                throw Error("create(): already have object");

            new (_storage) T(a, b);
            _initialized = true;
            return *_ptr();
        }

        template <typename A, typename B, typename C>
        T& create(A& a, B& b, C* c)
        {
            if (_initialized)
                throw Error("create(): already have object");

            new (_storage) T(a, b, c);
            _initialized = true;
            return *_ptr();
        }

        template <typename A, typename B, typename C>
        T& create(A& a, B& b, C& c)
        {
            if (_initialized)
                throw Error("create(): already have object");

            new (_storage) T(a, b, c);
            _initialized = true;
            return *_ptr();
        }

        template <typename A, typename B, typename C>
        T& create(A& a, B* b, C& c)
        {
            if (_initialized)
                throw Error("create(): already have object");

            new (_storage) T(a, b, c);
            _initialized = true;
            return *_ptr();
        }

        void free()
        {
            if (_initialized)
            {
                _ptr()->~T();
                _initialized = false;
            }
        }

        inline void clear()
        {
            free();
        }

        DECL_TPL_ERROR(ObjError);

    protected:
        T* _ptr() const
        {
            return (T*)_storage;
        }

        char _storage[sizeof(T)];
        bool _initialized;

    private:
        Obj(const Obj&); // no implicit copy
    };

} // namespace indigo

#endif
