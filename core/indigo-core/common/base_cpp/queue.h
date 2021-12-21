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

#ifndef __queue_h__
#define __queue_h__

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_cpp/exception.h"

namespace indigo
{

    DECL_EXCEPTION(QueueError);

    // Queue with fixed max length
    template <typename T>
    class Queue
    {
    public:
        DECL_TPL_ERROR(QueueError);

        explicit Queue(void)
        {
            _start = 0;
            _end = 0;
        }

        void setLength(int max_size)
        {
            _array.resize(max_size);
        }

        void clear(void)
        {
            _start = 0;
            _end = 0;
        }

        bool isEmpty(void)
        {
            return _start == _end;
        }

        T& push(const T& elem)
        {
            int idx = (_end + 1) % _array.size();
            if (idx == _start)
                throw Error("queue is full");
            int end = _end;
            _array[_end] = elem;
            _end = idx;
            return _array[end];
        }

        T& pop(void)
        {
            if (isEmpty())
                throw Error("queue is empty");
            int idx = _start;
            _start = (_start + 1) % _array.size();
            return _array[idx];
        }

    protected:
        Array<T> _array;
        int _start, _end;

    private:
        Queue(const Queue&); // no implicit copy
    };

} // namespace indigo

#endif // __queue_h__
