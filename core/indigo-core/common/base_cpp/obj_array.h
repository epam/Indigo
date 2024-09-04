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

#ifndef __objarray_h__
#define __objarray_h__

#include "base_cpp/array.h"

namespace indigo
{

    template <typename T>
    class ObjArray
    {
    public:
        explicit ObjArray()
        {
        }

        ~ObjArray()
        {
            while (size() > 0)
                pop();
        }

        const T& operator[](int index) const
        {
            return _array[index];
        }

        T& operator[](int index)
        {
            return _array[index];
        }

        const T& at(int index) const
        {
            return (*this)[index];
        }

        T& at(int index)
        {
            return (*this)[index];
        }

        int size(void) const
        {
            return _array.size();
        }

        T& push()
        {
            void* addr = &_array.push();

            new (addr) T();

            return _array.top();
        }

        template <typename A>
        T& push(A& a)
        {
            void* addr = &_array.push();

            new (addr) T(a);

            return _array.top();
        }

        template <typename A, typename B>
        T& push(A& a, B* b)
        {
            void* addr = &_array.push();

            new (addr) T(a, b);

            return _array.top();
        }

        template <typename A, typename B>
        T& push(A a, B& b)
        {
            void* addr = &_array.push();

            new (addr) T(a, b);

            return _array.top();
        }

        template <typename A, typename B, typename C>
        T& push(A& a, B& b, C& c)
        {
            void* addr = &_array.push();

            new (addr) T(a, b, c);

            return _array.top();
        }

        template <typename A, typename B, typename C>
        T& push(A* a, B b, C c)
        {
            void* addr = &_array.push();

            new (addr) T(a, b, c);

            return _array.top();
        }

        void clear()
        {
            while (size() > 0)
                pop();
        }

        void resize(int newSize)
        {
            while (newSize < size())
            {
                pop();
            }

            while (newSize > size())
            {
                push();
            }
        }

        void clear_resize(int newSize)
        {
            clear();
            resize(newSize);
        }
        void reserve(int size)
        {
            _array.reserve(size);
        }

        void expand(int newSize)
        {
            while (newSize > size())
            {
                push();
            }
        }

        void remove(int idx)
        {
            _array[idx].~T();
            _array.remove(idx);
        }

        T& top()
        {
            return _array.top();
        }

        const T& top() const
        {
            return _array.top();
        }

        void pop()
        {
            _array.top().~T();
            _array.pop();
        }

        template <typename T1, typename T2>
        void qsort(int (*cmp)(T1, T2, void*), void* context)
        {
            _array.qsort(cmp, context);
        }

        const T* ptr() const
        {
            return _array.ptr();
        }

    protected:
        Array<T> _array;

    private:
        ObjArray(const ObjArray&); // no implicit copy
    };

} // namespace indigo

#endif
