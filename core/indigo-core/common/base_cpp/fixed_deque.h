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

#ifndef __fixed_deque_h__
#define __fixed_deque_h__

#include <vector>

#include "base_c/defs.h"
#include "base_cpp/exception.h"

namespace indigo
{
    DECL_EXCEPTION(FixedDequeError);

    template <typename T>
    class FixedDeque
    {
    public:
        DECL_TPL_ERROR(FixedDequeError);

        explicit FixedDeque(size_t capacity) : _buffer(capacity), _capacity(capacity), _head(0), _tail(0), _size(0)
        {
        }

        bool empty() const
        {
            return _size == 0;
        }
        bool full() const
        {
            return _size == _capacity;
        }
        size_t size() const
        {
            return _size;
        }
        size_t capacity() const
        {
            return _capacity;
        }

        void push_back(const T& value)
        {
            if (full())
                throw Error("Deque is full");
            _buffer[_tail] = value;
            _tail = (_tail + 1) % _capacity;
            ++_size;
        }

        void push_front(const T& value)
        {
            if (full())
                throw Error("Deque is full");
            _head = (_head + _capacity - 1) % _capacity;
            _buffer[_head] = value;
            ++_size;
        }

        void pop_back()
        {
            if (empty())
                throw Error("Deque is empty");
            _tail = (_tail + _capacity - 1) % _capacity;
            --_size;
        }

        void pop_front()
        {
            if (empty())
                throw Error("Deque is empty");
            _head = (_head + 1) % _capacity;
            --_size;
        }

        T& front()
        {
            if (empty())
                throw Error("Deque is empty");
            return _buffer[_head];
        }

        T& back()
        {
            if (empty())
                throw Error("Deque is empty");
            return _buffer[(_tail + _capacity - 1) % _capacity];
        }

        T& operator[](size_t idx)
        {
            if (idx >= _size)
                throw Error("Index out of range");
            return _buffer[(_head + idx) % _capacity];
        }

        const T& operator[](size_t idx) const
        {
            if (idx >= _size)
                throw Error("Index out of range");
            return _buffer[(_head + idx) % _capacity];
        }

    private:
        std::vector<T> _buffer;
        size_t _capacity;
        size_t _head;
        size_t _tail;
        size_t _size;
    };
} // namespace indigo

#endif
