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
