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

#ifndef __cyclic_array_h__
#define __cyclic_array_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

namespace indigo
{

    DECL_EXCEPTION(CyclicArrayError);
    // Cyclic array
    template <typename T>
    struct CyclicArray
    {
    public:
        DECL_TPL_ERROR(CyclicArrayError);
        explicit CyclicArray(void)
        {
            _offset = 0;
        }

        void setSize(int max_size)
        {
            _array.resize(max_size);
        }

        void zeroFill(void)
        {
            _array.zerofill();
        }

        bool isInBound(int index) const
        {
            return index >= _offset && index < _offset + _array.size();
        }

        const T& operator[](int index) const
        {
            int length = _array.size();
            if (length == 0)
                throw Error("Zero length");
            int offset = index % _array.size();
            return index >= 0 ? _array[offset] : _array[length + offset];
        }

        T& operator[](int index)
        {
            int length = _array.size();
            if (length == 0)
                throw Error("Zero length");
            int offset = index % _array.size();
            return index >= 0 ? _array[offset] : _array[length + offset];
        }

        void setOffset(int offset)
        {
            _offset = offset;
        }

    protected:
        Array<T> _array;
        int _offset;

    private:
        CyclicArray(const CyclicArray&); // no implicit copy
    };

} // namespace indigo

#endif // __cyclic_array_h__
