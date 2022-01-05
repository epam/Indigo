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

#ifndef __obj_list_h__
#define __obj_list_h__

#include "base_cpp/list.h"

namespace indigo
{

    template <typename T>
    class ObjList
    {
    public:
        typedef typename List<T>::Elem Elem;

        ObjList()
        {
        }

        explicit ObjList(Pool<Elem>& pool) : _list(pool)
        {
        }

        ~ObjList()
        {
            clear();
        }

        int add()
        {
            int idx = _list.add();

            new (&_list[idx]) T();

            return idx;
        }

        template <typename A>
        int add(A& a)
        {
            int idx = _list.add();

            new (&_list[idx]) T(a);

            return idx;
        }

        int insertAfter(int existing)
        {
            int idx = _list.insertAfter(existing);

            new (&_list[idx]) T();

            return idx;
        }

        template <typename A>
        int insertAfter(int existing, A& a)
        {
            int idx = _list.insertAfter(existing);

            new (&_list[idx]) T(a);

            return idx;
        }

        int insertBefore(int existing)
        {
            int idx = _list.insertBefore(existing);

            new (&_list[idx]) T();

            return idx;
        }

        template <typename A>
        int insertBefore(int existing, A& a)
        {
            int idx = _list.insertBefore(existing);

            new (&_list[idx]) T(a);

            return idx;
        }

        void remove(int idx)
        {
            _list[idx].~T();
            _list.remove(idx);
        }

        int size() const
        {
            return _list.size();
        }
        int begin() const
        {
            return _list.begin();
        }
        int end() const
        {
            return _list.end();
        }
        int next(int idx) const
        {
            return _list.next(idx);
        }

        void clear()
        {
            while (_list.size() > 0)
                remove(_list.tail());
        }

        T& operator[](int index) const
        {
            return _list[index];
        }

        T& at(int index) const
        {
            return _list[index];
        }

    protected:
        List<T> _list;

    private:
        ObjList(const ObjList<T>&); // no implicit copy
    };

} // namespace indigo

#endif
