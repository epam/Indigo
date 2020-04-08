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

#ifndef __auto_ptr__
#define __auto_ptr__

#include "base_cpp/exception.h"

namespace indigo
{

    DECL_EXCEPTION(AutoPtrError);

    template <typename T> class AutoPtr
    {
    public:
        explicit AutoPtr(T* ptr = 0)
        {
            _ptr = ptr;
        }

        ~AutoPtr()
        {
            delete _ptr;
        }

        T* get() const
        {
            return _ptr;
        }

        T& ref() const
        {
            if (_ptr == 0)
                throw Error("no reference");

            return *_ptr;
        }

        T* operator->() const
        {
            if (_ptr == 0)
                throw Error("no reference");

            return _ptr;
        }

        T* release()
        {
            if (_ptr == 0)
                throw Error("nothing to release");

            T* ptr = _ptr;

            _ptr = 0;
            return ptr;
        }

        void reset(T* ptr)
        {
            if (ptr != _ptr)
            {
                delete _ptr;
                _ptr = ptr;
            }
        }

        void free()
        {
            reset(0);
        }

        AutoPtr<T>& operator=(T* ptr)
        {
            reset(ptr);
            return *this;
        }

        void create()
        {
            reset(new T());
        }

        template <typename A> void create(A& a)
        {
            reset(new T(a));
        }

        DECL_TPL_ERROR(AutoPtrError);

    protected:
        T* _ptr;

    private:
        AutoPtr(const AutoPtr&); // no implicit copy
        AutoPtr<T>& operator=(const AutoPtr<T>& other);
    };

} // namespace indigo

#endif
