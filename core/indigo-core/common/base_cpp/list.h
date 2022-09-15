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

#ifndef __list_h__
#define __list_h__

#include "base_cpp/pool.h"

namespace indigo
{

    template <typename T>
    class List
    {
    public:
        struct Elem
        {
            int prev;
            int next;
            T item;
        };

        explicit List() : _pool(new Pool<Elem>), _size(0), _head(-1), _tail(-1), _own_pool(true)
        {
        }

        explicit List(Pool<Elem>& pool) : _pool(&pool), _size(0), _head(-1), _tail(-1), _own_pool(false)
        {
        }

        List(List<T>&& src) noexcept : _pool(src._pool), _size(src._size), _head(src._head), _tail(src._tail), _own_pool(src._own_pool)
        {
            src._size = 0;
            src._head = -1;
            src._tail = -1;
            // src still pointing to _pool, but ensure not owning pool
            src._own_pool = false;
        }

        ~List()
        {
            clear();
            if (_own_pool)
                delete _pool;
        }

        List<T>& operator=(List<T>&& src) noexcept
        {
            if (this != &src)
            {
                std::swap(_pool, src._pool);
                std::swap(_size, src._size);
                std::swap(_head, src._head);
                std::swap(_tail, src._tail);
                std::swap(_own_pool, src._own_pool);
            }
            return *this;
        }

        int add()
        {
            if (_size == 0)
            {
                _head = _pool->add();
                _tail = _head;

                Elem& elem = _pool->at(_head);

                elem.prev = -1;
                elem.next = -1;
            }
            else
            {
                int idx = _pool->add();
                Elem& elem = _pool->at(idx);

                _pool->at(_tail).next = idx;
                elem.prev = _tail;
                elem.next = -1;
                _tail = idx;
            }

            _size++;
            return _tail;
        }

        int add(const T& item)
        {
            int idx = add();

            _pool->at(idx).item = item;
            return idx;
        }

        int insertAfter(int existing)
        {
            _pool->at(existing); // will throw if the element does not exist

            int idx = _pool->add();
            Elem& ex = _pool->at(existing);
            Elem& elem = _pool->at(idx);

            elem.next = ex.next;
            elem.prev = existing;
            ex.next = idx;

            if (elem.next != -1)
                _pool->at(elem.next).prev = idx;

            if (_tail == existing)
                _tail = idx;

            _size++;
            return idx;
        }

        int insertBefore(int existing)
        {
            _pool->at(existing); // will throw if the element does not exist

            int idx = _pool->add();
            Elem& ex = _pool->at(existing);
            Elem& elem = _pool->at(idx);

            elem.prev = ex.prev;
            elem.next = existing;
            ex.prev = idx;

            if (elem.prev != -1)
                _pool->at(elem.prev).next = idx;

            if (_head == existing)
                _head = idx;

            _size++;
            return idx;
        }

        void remove(int idx)
        {
            Elem& elem = _pool->at(idx);

            if (elem.prev != -1)
                _pool->at(elem.prev).next = elem.next;
            else
                _head = elem.next;

            if (elem.next != -1)
                _pool->at(elem.next).prev = elem.prev;
            else
                _tail = elem.prev;

            _pool->remove(idx);
            _size--;
        }

        int size() const
        {
            return _size;
        }

        int begin() const
        {
            if (_head == -1)
                return _pool->end();

            return _head;
        }

        int end() const
        {
            return _pool->end();
        }

        int next(int idx) const
        {
            int res = _pool->at(idx).next;

            if (res == -1)
                return _pool->end();

            return res;
        }

        int prev(int idx) const
        {
            return _pool->at(idx).prev;
        }

        int tail() const
        {
            return _tail;
        }

        void clear()
        {
            if (_own_pool)
                _pool->clear();
            // or there may be other lists using the same _pool
            else
                while (_tail != -1)
                {
                    int iter = _tail;

                    _tail = _pool->at(iter).prev;
                    _pool->remove(iter);
                }

            _size = 0;
            _head = -1;
            _tail = -1;
        }

        T& operator[](int index) const
        {
            return _pool->at(index).item;
        }

        T& at(int index) const
        {
            return _pool->at(index).item;
        }

    protected:
        Pool<Elem>* _pool; // the element pool may be shared amongst lists
        int _size;
        int _head;
        int _tail;
        bool _own_pool;

    private:
        List(const List<T>&); // no implicit copy
        List<T>& operator=(const List<T>&);
    };

} // namespace indigo

#endif
