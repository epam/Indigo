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

#include "base_cpp/exception.h"

#include "mmf_ptr.h"

namespace bingo
{
    template <typename T>
    class MMFList
    {
    private:
        struct _Link
        {
            MMFPtr<T> data_ptr;
            MMFPtr<_Link> next_link;
            MMFPtr<_Link> prev_link;

            _Link()
            {
                data_ptr = MMFPtr<T>(MMFAddress::null);
                next_link = MMFPtr<_Link>(MMFAddress::null);
                prev_link = MMFPtr<_Link>(MMFAddress::null);
            }
        };

    public:
        struct Iterator
        {
            MMFPtr<_Link> cur_link;

            Iterator() : cur_link(MMFAddress::null)
            {
            }
            Iterator(MMFPtr<_Link> link) : cur_link(link)
            {
            }
            Iterator(const Iterator& it) : cur_link(it.cur_link)
            {
            }

            T& operator*()
            {
                return cur_link->data_ptr.ref();
            }

            T* operator->()
            {
                return cur_link->data_ptr.ptr();
            }

            Iterator& operator=(const Iterator& it)
            {
                cur_link = it.cur_link;

                return *this;
            }

            bool operator==(const Iterator& it) const
            {
                if (cur_link.getAddress() == it.cur_link.getAddress())
                    return true;
                return false;
            }

            bool operator!=(const Iterator& it) const
            {
                return !(*this == it);
            }

            Iterator& operator++(int)
            {
                if (cur_link->next_link.getAddress() == MMFAddress::null)
                    throw indigo::Exception("MMFList::Iterator:operator++ There's no next link");

                cur_link = cur_link->next_link;

                return *this;
            }

            Iterator& operator--(int)
            {
                if (cur_link->prev_link.getAddress() == MMFAddress::null)
                    throw indigo::Exception("MMFList::Iterator:operator-- There's no previous link");

                cur_link = cur_link->prev_link;

                return *this;
            }
        };

        MMFList()
        {
            _size = 0;
            _begin_link.allocate();
            new (_begin_link.ptr()) _Link();
            _end_link = _begin_link;
        }

        bool empty() const
        {
            if (_end_link == _begin_link)
                return true;
            return false;
        }

        unsigned int size() const
        {
            return _size;
        }

        void insertBefore(Iterator pos, const MMFPtr<T> x)
        {
            MMFPtr<_Link> new_link;
            new_link.allocate();
            new (new_link.ptr()) _Link();

            new_link->data_ptr = x;

            new_link->next_link = pos.cur_link;
            new_link->prev_link = pos.cur_link->prev_link;

            if (pos.cur_link->prev_link.getAddress() != MMFAddress::null)
                pos.cur_link->prev_link->next_link = new_link;
            pos.cur_link->prev_link = new_link;

            if (pos.cur_link.getAddress() == _begin_link.getAddress())
                _begin_link = new_link;

            _size++;
        }

        void insertBefore(Iterator pos, const T& x)
        {
            MMFPtr<T> data;
            data.allocate();
            data.ref() = x;

            insertBefore(pos, data);
        }

        void erase(Iterator& pos)
        {
            if (pos.cur_link->prev_link != -1)
                pos.cur_link->prev_link->next_link = pos.cur_link->next_link;
            if (pos.cur_link->next_link != -1)
                pos.cur_link->next_link->prev_link = pos.cur_link->prev_link;

            if (pos.cur_link == _begin_link)
                _begin_link = pos.cur_link->next_link;

            if (pos.cur_link == _end_link)
                throw indigo::Exception("MMFList:erase End link can't be removed");

            _size--;
        }

        void pushBack(const T& x)
        {
            insertBefore(end(), x);
        }

        void pushBack(const MMFPtr<T> x)
        {
            insertBefore(end(), x);
        }

        Iterator begin() const
        {
            return Iterator(_begin_link);
        }

        Iterator top() const
        {
            Iterator end_it(_end_link);
            end_it--;
            return end_it;
        }

        Iterator end() const
        {
            return Iterator(_end_link);
        }

    private:
        MMFPtr<_Link> _begin_link;
        MMFPtr<_Link> _end_link;
        int _size;
    };
}
