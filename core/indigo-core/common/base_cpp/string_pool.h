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

#ifndef __string_pool_h__
#define __string_pool_h__

#include "base_cpp/auto_iter.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/pool.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class DLLEXPORT StringPool
    {
    public:
        DECL_ERROR;

        StringPool();
        ~StringPool();

        int add(const char* str);
        int add(Array<char>& str);
        int add(int size);
        void set(int idx, const char* str);
        void remove(int idx);
        int size() const;
        int begin() const;
        int end() const;
        int next(int i) const;
        void clear();

        char* at(int idx);
        const char* at(int idx) const;
        /*
         * Iterators
         */
        class PoolIter : public AutoIterator
        {
        public:
            PoolIter(StringPool& owner, int idx) : AutoIterator(idx), _owner(owner)
            {
            }
            PoolIter& operator++()
            {
                _idx = _owner.next(_idx);
                return *this;
            }

        private:
            StringPool& _owner;
        };
        class PoolAuto
        {
        public:
            PoolAuto(StringPool& owner) : _owner(owner)
            {
            }
            PoolIter begin()
            {
                return StringPool::PoolIter(_owner, _owner.begin());
            }
            PoolIter end()
            {
                return StringPool::PoolIter(_owner, _owner.end());
            }

        private:
            StringPool& _owner;
        };

        PoolAuto elements()
        {
            return PoolAuto(*this);
        }

    protected:
        int _add(const char* str, int size);

        Pool<int> _pool;
        ObjArray<Array<char>> _storage;

    private:
        StringPool(const StringPool&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
