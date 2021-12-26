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

#ifndef __obj_pool_h__
#define __obj_pool_h__

#include "base_cpp/pool.h"

namespace indigo
{

    template <typename T>
    class ObjPool
    {
    public:
        ObjPool()
        {
        }

        ~ObjPool()
        {
            clear();
        }

        int add()
        {
            int idx = _pool.add();

            void* addr = &_pool[idx];

            new (addr) T();

            return idx;
        }

        template <typename A>
        int add(A& a)
        {
            int idx = _pool.add();

            void* addr = &_pool[idx];

            new (addr) T(a);

            return idx;
        }

        template <typename A, typename B>
        int add(A& a, B& b)
        {
            int idx = _pool.add();

            void* addr = &_pool[idx];

            new (addr) T(a, b);

            return idx;
        }

        void remove(int idx)
        {
            T& elem = _pool[idx];

            elem.~T();
            _pool.remove(idx);
        }

        int size() const
        {
            return _pool.size();
        }

        int begin() const
        {
            return _pool.begin();
        }

        int next(int i) const
        {
            return _pool.next(i);
        }

        int end() const
        {
            return _pool.end();
        }

        void clear()
        {
            for (int i = _pool.begin(); i != _pool.end(); i = _pool.next(i))
                _pool[i].~T();
            _pool.clear();
        }

        bool hasElement(int idx) const
        {
            return _pool.hasElement(idx);
        }

        const T& operator[](int index) const
        {
            return _pool[index];
        }

        T& operator[](int index)
        {
            return _pool[index];
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
        Pool<T> _pool;

    private:
        ObjPool(const ObjPool<T>&); // no implicit copy
    };

} // namespace indigo

#endif
