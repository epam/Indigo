#pragma once

#include "base_cpp/exception.h"

#include "mmf_ptr.h"

namespace bingo
{
    template <typename T>
    class MMFArray
    {
    public:
        MMFArray(int block_size = 10000) : _block_size(block_size), _size(0), _block_count(0)
        {
        }

        void resize(int new_size)
        {
            if (new_size > reserved())
            {
                int blocks_count = (_size + _block_size - 1) / _block_size;
                int new_blocks_count = (new_size + _block_size - 1) / _block_size;

                if (new_blocks_count > _max_block_count)
                    throw indigo::Exception("MMFArray: block count limit is exceeded");

                for (int i = blocks_count; i < new_blocks_count; i++)
                {
                    _blocks[i].allocate(_block_size);
                    for (int j = 0; j < _block_size; j++)
                        new ((_blocks[i] + j).ptr()) T();
                }
                _block_count = new_blocks_count;
            }

            _size = new_size;
        }

        T& at(int index)
        {
            if (index < 0 || index >= _size)
                throw indigo::Exception("MMFArray: incorrect idx %d (size=%d)", index, _size);

            return *(_blocks[index / _block_size].ptr() + index % _block_size);
        }

        const T& at(int index) const
        {
            if (index < 0 || index >= _size)
                throw indigo::Exception("MMFArray: incorrect idx %d (size=%d)", index, _size);

            const auto i1 = index / _block_size;
            const auto block = _blocks[i1];
            const auto ptr = block.ptr();
            const auto i2 = index % _block_size;
            const auto i3 = ptr + i2;
            return *i3;
        }

        T& operator[](int index)
        {
            return at(index);
        }

        const T& operator[](int index) const
        {
            return at(index);
        }

        T& top()
        {
            int index = _size - 1;

            return *(_blocks[index / _block_size].ptr() + index % _block_size);
        }

        size_t find(const T& elem)
        {
            for (size_t i = 0; i < size(); i++)
            {
                if (at(i) == elem)
                    return i;
            }

            return (size_t)-1;
        }

        T& push()
        {
            if (_size % _block_size == 0)
            {
                int blocks_count = (_size + _block_size - 1) / _block_size;

                _blocks[blocks_count].allocate(_block_size);
            }

            T* arr = _blocks[_size / _block_size].ptr();
            int idx_in_block = _size % _block_size;

            _size++;

            new (arr + idx_in_block) T();

            return arr[idx_in_block];
        }

        template <typename A>
        T& push(A& a)
        {
            if (_size % _block_size == 0)
            {
                int blocks_count = (_size + _block_size - 1) / _block_size;

                _blocks[blocks_count].allocate(_block_size);
            }

            T* arr = _blocks[_size / _block_size].ptr();
            int idx_in_block = _size % _block_size;

            _size++;

            new (arr + idx_in_block) T(a);

            return arr[idx_in_block];
        }

        void push(T elem)
        {
            T& new_elem = push();

            new_elem = elem;
        }

        int size() const
        {
            return _size;
        }

        int reserved() const
        {
            return _block_count * _block_size;
        }

    private:
        static const int _max_block_count = 40000;

        int _block_size;
        int _block_count;
        int _size;
        MMFPtr<T> _blocks[_max_block_count];
    };
}
