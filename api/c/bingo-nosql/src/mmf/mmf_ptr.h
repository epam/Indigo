#pragma once

#include "mmf_address.h"
#include "mmf_allocator.h"

namespace bingo
{
    class MMFAllocator;

    template <typename T> class MMFPtr
    {
    public:
        MMFPtr() = default;

        explicit MMFPtr(MMFAddress addr) : _addr(addr)
        {
        }

        explicit MMFPtr(int file_id, ptrdiff_t offset) : _addr(file_id, offset)
        {
        }

        void allocate(int count = 1)
        {
            _addr = _allocator->template allocate<T>(count);
        }

        T* ptr()
        {
            return reinterpret_cast<T*>(_allocator->get(_addr.file_id, _addr.offset));
        }

        const T* ptr() const
        {
            return reinterpret_cast<const T*>(_allocator->get(_addr.file_id, _addr.offset));
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

        operator MMFAddress() const
        {
            return _addr;
        }

    private:
        MMFAddress _addr;

        // TODO: Currently it's always nullptr, we need to somehow initialize it
        MMFAllocator* _allocator = nullptr;
    };
}
