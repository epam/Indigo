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

#ifndef __array_h__
#define __array_h__

#include <cctype>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <utility>

#include "base_c/defs.h"
#include "base_cpp/exception.h"

namespace indigo
{
    DECL_EXCEPTION(ArrayError);

    template <typename T>
    class Array
    {
    public:
        DECL_TPL_ERROR(ArrayError);

        explicit Array() : _reserved(0), _length(0), _array(nullptr)
        {
        }

        Array(Array&& other) : _reserved(other._reserved), _length(other._length), _array(other._array)
        {
            other._array = nullptr;
            other._length = 0;
            other._reserved = 0;
        }

        ~Array()
        {
            if (_array != nullptr)
            {
                std::free(static_cast<void*>(_array));
                _array = nullptr;
                _length = 0;
                _reserved = 0;
            }
        }

        void clear()
        {
            _length = 0;
        }

        void reserve(int to_reserve)
        {
            if (to_reserve < 0)
                throw Error("to_reserve = %d", to_reserve);

            if (to_reserve > _reserved)
            {
                if (_length < 1)
                {
                    if (_array != nullptr)
                    {
                        std::free(static_cast<void*>(_array));
                        _array = nullptr;
                        _length = 0;
                        _reserved = 0;
                    }
                }

                T* oldptr = _array;

                _array = static_cast<T*>(std::realloc(static_cast<void*>(_array), sizeof(T) * to_reserve));
                if (_array == nullptr)
                {
                    _array = oldptr;
                    throw std::bad_alloc();
                }
                _reserved = to_reserve;
            }
        }

        void zerofill()
        {
            if (_length > 0)
                memset(_array, 0, _length * sizeof(T));
        }

        void fffill()
        {
            if (_length > 0)
                memset(_array, 0xFF, _length * sizeof(T));
        }

        void fill(const T& value)
        {
            for (int i = 0; i < size(); i++)
                _array[i] = value;
        }

        const T* ptr() const
        {
            return _array;
        }

        T* ptr()
        {
            return _array;
        }

        const T& operator[](int index) const
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return _array[index];
        }

        T& operator[](int index)
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return _array[index];
        }

        const T& at(int index) const
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return (*this)[index];
        }

        T& at(int index)
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return (*this)[index];
        }

        int size() const
        {
            return _length;
        }

        int sizeInBytes() const
        {
            return _length * sizeof(T);
        }

        void copy(const Array<T>& other)
        {
            copy(other._array, other._length);
        }

        void copy(const T* other, int count)
        {
            if (count > 0)
            {
                clear_resize(count);
                memcpy(_array, other, count * sizeof(T));
            }
            else
            {
                _length = 0;
            }
        }

        void concat(const Array<T>& other)
        {
            concat(other._array, other.size());
        }

        void concat(const T* other, int count)
        {
            if (count > 0)
            {
                int length = _length;
                resize(length + count);

                memcpy(_array + length, other, count * sizeof(T));
            }
        }

        int memcmp(const Array<T>& other) const
        {
            if (_length < other._length)
                return -1;
            if (_length > other._length)
                return -1;

            if (_length == 0)
                return 0;

            return ::memcmp(_array, other._array, _length * sizeof(T));
        }

        void remove(int idx, int span = 1)
        {
            if (idx < 0 || idx - _length - span + 1 >= 0)
                throw Error("remove(): invalid index %d with span %d (size=%d)", idx, span, _length);

            memmove(_array + idx, _array + idx + span, sizeof(T) * (_length - idx - span));
            _length -= span;
        }

        void remove_replace(int idx)
        {
            if (idx < 0 || idx >= _length)
                throw Error("remove_replace(): invalid index %d (size=%d)", idx, _length);

            if (idx < _length - 1)
                _array[idx] = _array[_length - 1];

            _length--;
        }

        int find(const T& value) const
        {
            return find(0, _length, value);
        }

        int find(int from, int to, const T& value) const
        {
            for (int i = from; i < to; i++)
                if (_array[i] == value)
                    return i;

            return -1;
        }

        int count(const T& value) const
        {
            return count(0, _length, value);
        }

        int count(int from, int to, const T& value) const
        {
            int cnt = 0;
            for (int i = from; i < to; i++)
                if (_array[i] == value)
                    cnt++;

            return cnt;
        }

        void swap(int idx1, int idx2)
        {
            if (idx1 < 0 || idx1 >= _length)
                throw Error("swap(): invalid index %d (size=%d)", idx1, _length);

            if (idx2 < 0 || idx2 >= _length)
                throw Error("swap(): invalid index %d (size=%d)", idx2, _length);

            if (idx1 == idx2)
                return;

            std::swap(_array[idx1], _array[idx2]);
        }

        void push(T elem)
        {
            resize(_length + 1);
            _array[_length - 1] = elem;
        }

        T& push()
        {
            resize(_length + 1);
            return _array[_length - 1];
        }

        T& pop()
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[--_length];
        }

        T& top()
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[_length - 1];
        }

        const T& top() const
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[_length - 1];
        }

        T& top(int offset)
        {
            if (_length - offset <= 0)
                throw Error("stack underflow");

            return _array[_length - 1 - offset];
        }

        const T& top(int offset) const
        {
            if (_length - offset <= 0)
                throw Error("stack underflow");

            return _array[_length - 1 - offset];
        }

        void resize(int newsize)
        {
            if (newsize > _reserved)
                reserve((newsize + 1) * 2);
            _length = newsize;
        }

        void expand(int newsize)
        {
            if (_length < newsize)
                resize(newsize);
        }

        void expandFill(int newsize, const T& value)
        {
            while (_length < newsize)
                push(value);
        }

        void clear_resize(int newsize)
        {
            if (_reserved < newsize)
            {
                _length = 0;
                reserve((newsize + 1) * 2);
            }
            _length = newsize;
        }

        void swap(Array<T>& other)
        {
            std::swap(_array, other._array);
            std::swap(_reserved, other._reserved);
            std::swap(_length, other._length);
        }

        T* begin()
        {
            return _array;
        }

        T* end()
        {
            return _array + _length;
        }

        // CMP_FUNCTOR has two arguments and returns sign of comparation
        template <typename CmpFunctor>
        void insertionSort(int start, int end, CmpFunctor cmp)
        {
            int i, j;
            char tmp[sizeof(T)]; // can't use T directly because it may have destructor

            for (i = start + 1; i <= end; i++)
            {
                j = i;
                while (j > start && cmp(_array[j - 1], _array[j]) > 0)
                {
                    T* a1 = _array + j - 1;
                    T* a2 = a1 + 1;
                    memcpy(&tmp, a1, sizeof(T));
                    memcpy(a1, a2, sizeof(T));
                    memcpy(a2, &tmp, sizeof(T));
                    j--;
                }
            }
        }

        // CMP_FUNCTOR has two arguments and returns sign of comparation
        template <typename CmpFunctor>
        void qsort(int start, int end, CmpFunctor cmp)
        {
            // Sort elements from start to end
            if (start >= end)
                return;
            if (end - start < 10)
                insertionSort(start, end, cmp);

            struct
            {
                T *lo, *hi;
            } stack[32], *sp;

            char tmp[sizeof(T)]; // can't use T directly because it may have destructor

            sp = stack;

            // push our initial values onto the stack
            sp->lo = _array + start;
            sp->hi = _array + end + 1;
            sp++;

            while (sp > stack)
            {
                // pop lo and hi off the stack
                sp--;
                T *high = sp->hi, *low = sp->lo;
                T* hi = high - 1;
                T* lo = low;
                T* pivot = low;

                while (1)
                {
                    while (lo < high && lo != pivot && cmp(*lo, *pivot) < 0)
                        lo++;

                    while (hi > low && (hi == pivot || cmp(*hi, *pivot) >= 0))
                        hi--;

                    if (lo < hi)
                    {
                        memcpy(&tmp, lo, sizeof(T));
                        memcpy(lo, hi, sizeof(T));
                        memcpy(hi, &tmp, sizeof(T));

                        if (lo == pivot)
                            pivot = hi;
                        else if (hi == pivot)
                            pivot = lo;

                        hi--;
                    }
                    else
                    {
                        hi++;

                        if (hi == high)
                            // done with this segment
                            break;

                        // push the larger segment onto the stack and continue
                        // sorting the smaller segment.
                        if ((hi - low) > (high - hi))
                        {
                            sp->lo = low;
                            sp->hi = hi;
                            sp++;

                            hi = high;
                            low = lo;
                        }
                        else
                        {
                            sp->hi = high;
                            sp->lo = hi;
                            sp++;

                            high = hi;
                            lo = low;
                        }

                        pivot = lo;
                        hi--;
                    }
                }
            }
        }

        template <typename T1, typename T2>
        void qsort(int start, int end, int (*cmp)(T1, T2, void*), void* context)
        {
            this->qsort(start, end, _CmpFunctorCaller<T1, T2>(cmp, context));
        }

        template <typename T1, typename T2>
        void qsort(int (*cmp)(T1, T2, void*), void* context)
        {
            this->qsort(0, _length - 1, cmp, context);
        }

        // Array<char>-specific
        void appendString(const char* str, bool keep_zero)
        {
            int len = (int)strlen(str);
            int initial_size = _length;

            if (initial_size > 0 && _array[initial_size - 1] == 0)
                initial_size--;

            resize(initial_size + len);
            memcpy(_array + initial_size, str, len);

            if (keep_zero)
                push(0);
        }

        void readString(const char* str, bool keep_zero)
        {
            clear();
            appendString(str, keep_zero);
        }

        void upper(const char* source)
        {
            clear();
            while (*source != 0)
                push(::toupper(*source++));
            push(0);
        }

        void lower(const char* source)
        {
            clear();
            while (*source != 0)
                push(::tolower(*source++));
            push(0);
        }

        void toupper()
        {
            for (int i = 0; i < _length; i++)
                _array[i] = ::toupper(_array[i]);
        }

        void tolower()
        {
            for (int i = 0; i < _length; i++)
                _array[i] = ::tolower(_array[i]);
        }

    protected:
        T* _array;

        int _reserved;
        int _length;

    private:
        Array(const Array&);                            // no implicit copy
        Array<int>& operator=(const Array<int>& right); // no copy constructor

        template <typename T1, typename T2>
        class _CmpFunctorCaller
        {
        public:
            _CmpFunctorCaller(int (*cmp)(T1, T2, void*), void* context) : _context(context), _cmp(cmp)
            {
            }

            int operator()(T1 arg1, T2 arg2) const
            {
                return _cmp(arg1, arg2, _context);
            }

        private:
            void* _context;
            int (*_cmp)(T1, T2, void*);
        };
    };

} // namespace indigo

#endif
