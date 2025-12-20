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

#pragma once

#include "mmf_address.h"
#include "mmf_allocator.h"

namespace bingo
{
    class MMFAllocator;

    template <typename T>
    class MMFPtr
    {
    public:
        MMFPtr() = default;

        explicit MMFPtr(MMFAddress addr) : _addr(addr)
        {
        }

        MMFPtr(const MMFPtr<T>& ptr) : _addr(ptr.getAddress())
        {
        }

        explicit MMFPtr(int file_id, ptrdiff_t offset) : _addr(file_id, offset)
        {
        }

        void allocate(int count = 1)
        {
            _addr = MMFAllocator::getAllocator().template allocate<T>(count);
        }

        T* ptr(MMFAllocator& allocator)
        {
            return static_cast<T*>(allocator.get(_addr.file_id, _addr.offset));
        }

        T* ptr()
        {
            return static_cast<T*>(MMFAllocator::getAllocator().get(_addr.file_id, _addr.offset));
        }

        const T* ptr() const
        {
            return static_cast<const T*>(MMFAllocator::getAllocator().get(_addr.file_id, _addr.offset));
        }

        T& ref()
        {
            return *ptr();
        }

        const T& ref() const
        {
            return *ptr();
        }

        T* operator->()
        {
            return ptr();
        }

        const T* operator->() const
        {
            return ptr();
        }

        MMFPtr<T> operator+(int off)
        {
            return MMFPtr<T>(_addr.file_id, _addr.offset + off * sizeof(T));
        }

        T& operator[](int idx)
        {
            return *(ptr() + idx);
        }

        bool isNull()
        {
            return (_addr.offset == (size_t)-1) && (_addr.file_id == (size_t)-1);
        }

        MMFAddress getAddress() const
        {
            return _addr;
        }

    private:
        MMFAddress _addr = MMFAddress::null;
    };
}
