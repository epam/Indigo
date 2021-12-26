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

#ifndef __reusable_obj_array__
#define __reusable_obj_array__

#include "base_cpp/array.h"

namespace indigo
{

    template <typename T>
    class ReusableObjArray
    {
    public:
        explicit ReusableObjArray()
        {
            _count = 0;
        }

        ~ReusableObjArray()
        {
            for (int i = 0; i < _array.size(); i++)
            {
                _array[i].~T();
            }
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
            return _count;
        }

        void resize(const int newSize)
        {
            if (newSize <= _count)
            {
                _count = newSize;
            }
            else
            {
                _array.reserve(newSize);
                while (_count < newSize)
                {
                    push();
                }
            }
        }

        T& push()
        {
            T* addr;
            if (_count == _array.size())
            {
                addr = &_array.push();
                new (addr) T();
            }
            else
            {
                addr = &_array[_count];
            }
            _count++;
            addr->clear();

            return *addr;
        }

        void pop()
        {
            _count--;
        }

        T& top()
        {
            return _array[_count - 1];
        }

        void clear()
        {
            _count = 0;
        }

        void reserve(int to_reserve)
        {
            _array.reserve(to_reserve);
        }

    protected:
        Array<T> _array;
        int _count;

    private:
        ReusableObjArray(const ReusableObjArray&); // no implicit copy
    };

} // namespace indigo

#endif
