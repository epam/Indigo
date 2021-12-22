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

#ifndef __pool_h__
#define __pool_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

namespace indigo
{

    DECL_EXCEPTION(PoolError);

    template <typename T>
    class Pool
    {
    public:
        DECL_TPL_ERROR(PoolError);

        Pool() : _size(0), _first(-1)
        {
        }

        int add()
        {
            if (_first == -1)
            {
                _array.push();
                _next.push(-2);
                _size++;

                return _array.size() - 1;
            }

            int idx = _first;

            _first = _next[_first];

            if (_first == -2)
                throw Error("internal error: index %d is used in add()", idx);

            _next[idx] = -2;
            _size++;

            return idx;
        }

        int add(const T& item)
        {
            int idx = add();

            _array[idx] = item;
            return idx;
        }

        void remove(int idx)
        {
            if (_next[idx] != -2)
                throw Error("trying to remove unused element #%d", idx);

            _next[idx] = _first;
            _first = idx;

            _size--;
        }

        bool hasElement(int idx) const
        {
            return (_next[idx] == -2);
        }

        int size() const
        {
            return _size;
        }

        int begin() const
        {
            int i;

            for (i = 0; i < _next.size(); i++)
                if (_next[i] == -2)
                    break;

            return i;
        }

        int next(int i) const
        {
            for (i++; i < _next.size(); i++)
                if (_next[i] == -2)
                    break;

            return i;
        }

        int end() const
        {
            return _array.size();
        }

        void clear()
        {
            _array.clear();
            _next.clear();
            _size = 0;
            _first = -1;
        }

        const T& operator[](int index) const
        {
            if (_next[index] != -2)
                throw Error("access to unused element %d", index);
            return _array[index];
        }

        T& operator[](int index)
        {
            if (_next[index] != -2)
                throw Error("access to unused element %d", index);
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

    protected:
        Array<T> _array; // pool elements

        // _next[i] >= 0  => _array[i] is not used,
        //                    _next[i] contains the index of the next unused element
        // _next[i] == -1 => _array[i] is the last unused element
        // _next[i] == -2 => _array[i] is used
        Array<int> _next;

        int _size; // number of _array items used

        int _first; // index of the first unused element (-1 if all are used)

    private:
        Pool(const Pool<T>&); // no implicit copy
    };

} // namespace indigo

#endif
