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

#ifndef __ptr_pool__
#define __ptr_pool__

#include "base_cpp/pool.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    DECL_EXCEPTION(PtrPoolError);

    template <typename T>
    class PtrPool
    {
    public:
        explicit PtrPool()
        {
        }

        virtual ~PtrPool()
        {
            clear();
        }

        DECL_TPL_ERROR(PtrPoolError);

        int add(T* obj)
        {
            return _ptrpool.add(obj);
        }

        void remove(int idx)
        {
            delete _ptrpool[idx];
            _ptrpool.remove(idx);
        }

        bool hasElement(int idx) const
        {
            return _ptrpool.hasElement(idx);
        }

        int size() const
        {
            return _ptrpool.size();
        }

        int begin() const
        {
            return _ptrpool.begin();
        }

        int end() const
        {
            return _ptrpool.end();
        }

        int next(int i) const
        {
            return _ptrpool.next(i);
        }

        void clear(void)
        {
            int i;

            for (i = _ptrpool.begin(); i != _ptrpool.end(); i = _ptrpool.next(i))
                delete _ptrpool[i];

            _ptrpool.clear();
        }

        const T* operator[](int index) const
        {
            return _ptrpool[index];
        }
        T*& operator[](int index)
        {
            return _ptrpool[index];
        }

        const T* at(int index) const
        {
            return _ptrpool[index];
        }
        T*& at(int index)
        {
            return _ptrpool[index];
        }

        const T& ref(int index) const
        {
            return *_ptrpool[index];
        }

        T& ref(int index)
        {
            return *_ptrpool[index];
        }

    protected:
        Pool<T*> _ptrpool;

    private:
        PtrPool(const PtrPool&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
