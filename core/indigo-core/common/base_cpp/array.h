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

#include <vector>
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
            clear_resize(0);
            concat(other);
        }

        void concat(const Array<T>& other)
        {
            for (int i = 0; i < other.size(); ++i)
                push_back(other[i]);
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
                return;g

            std::swap(_array[idx1], _array[idx2]);
        }

        T& push_back(const T& elem)
        {
            resize(_length + 1);
            new ((void*)&_array[_length - 1]) T(elem);
            return _array[_length - 1];
        }

        template <class... Args>
        T& emplace_back(Args&&... args);

        template <class... Args>
        T& replace(int idx, Args&&... args);

        void pop_back()
        {
            if (_length <= 0)
                throw Error("stack underflow");
            --_length;
            memset(_array + _length, 0, sizeof(T));
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
                push_back(value);
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

    template <typename T>
    template <class... Args>
    T& Array<T>::emplace_back(Args&&... args)
    {
        resize(_length + 1);
        new ((void*)&_array[_length - 1]) T(args...);
        return _array[_length - 1];
    }

    template <typename T>
    template <class... Args>
    T& Array<T>::replace(int idx, Args&&... args)
    {
        new (&_array[idx]) T(args...);
        return _array[_length - 1];
    }

    template <>
    class Array<bool>
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

                bool* oldptr = _array;

                _array = static_cast<bool*>(std::realloc(static_cast<void*>(_array), sizeof(bool) * to_reserve));
                if (_array == nullptr)
                {
                    _array = oldptr;
                    throw std::bad_alloc();
                }
                _reserved = to_reserve;
            }
        }

        void fill(const bool& value)
        {
            for (int i = 0; i < size(); i++)
                _array[i] = value;
        }

        const bool* ptr() const
        {
            return _array;
        }

        bool* ptr()
        {
            return _array;
        }

        const bool& operator[](int index) const
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return _array[index];
        }

        bool& operator[](int index)
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return _array[index];
        }

        const bool& at(int index) const
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return (*this)[index];
        }

        bool& at(int index)
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
            return _length * sizeof(bool);
        }

        void copy(const Array<bool>& other)
        {
            copy(other._array, other._length);
        }

        void copy(const bool* other, int count)
        {
            if (count > 0)
            {
                clear_resize(count);
                memcpy(_array, other, count * sizeof(bool));
            }
            else
            {
                _length = 0;
            }
        }

        void concat(const Array<bool>& other)
        {
            concat(other._array, other.size());
        }

        void concat(const bool* other, int count)
        {
            if (count > 0)
            {
                int length = _length;
                resize(length + count);

                memcpy(_array + length, other, count * sizeof(bool));
            }
        }

        int memcmp(const Array<bool>& other) const
        {
            if (_length < other._length)
                return -1;
            if (_length > other._length)
                return -1;

            if (_length == 0)
                return 0;

            return ::memcmp(_array, other._array, _length * sizeof(bool));
        }

        void remove(int idx, int span = 1)
        {
            if (idx < 0 || idx - _length - span + 1 >= 0)
                throw Error("remove(): invalid index %d with span %d (size=%d)", idx, span, _length);

            memmove(_array + idx, _array + idx + span, sizeof(bool) * (_length - idx - span));
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

        int find(const bool& value) const
        {
            return find(0, _length, value);
        }

        int find(int from, int to, const bool& value) const
        {
            for (int i = from; i < to; i++)
                if (_array[i] == value)
                    return i;

            return -1;
        }

        int count(const bool& value) const
        {
            return count(0, _length, value);
        }

        int count(int from, int to, const bool& value) const
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

        void push(bool elem)
        {
            resize(_length + 1);
            _array[_length - 1] = elem;
        }

        bool& push()
        {
            resize(_length + 1);
            return _array[_length - 1];
        }

        bool& pop()
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[--_length];
        }

        bool& top()
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[_length - 1];
        }

        const bool& top() const
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[_length - 1];
        }

        bool& top(int offset)
        {
            if (_length - offset <= 0)
                throw Error("stack underflow");

            return _array[_length - 1 - offset];
        }

        const bool& top(int offset) const
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

        void expandFill(int newsize, const bool& value)
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

        void swap(Array<bool>& other)
        {
            std::swap(_array, other._array);
            std::swap(_reserved, other._reserved);
            std::swap(_length, other._length);
        }

        bool* begin()
        {
            return _array;
        }

        bool* end()
        {
            return _array + _length;
        }

    protected:
        bool* _array;

        int _reserved;
        int _length;

    private:
        Array(const Array&);                            // no implicit copy
        Array<int>& operator=(const Array<int>& right); // no copy constructor
    };

    template <>
    class Array<char>
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

                char* oldptr = _array;

                _array = static_cast<char*>(std::realloc(static_cast<void*>(_array), sizeof(char) * to_reserve));
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
                memset(_array, 0, _length * sizeof(char));
        }

        void fffill()
        {
            if (_length > 0)
                memset(_array, 0xFF, _length * sizeof(char));
        }

        void fill(const char& value)
        {
            for (int i = 0; i < size(); i++)
                _array[i] = value;
        }

        const char* ptr() const
        {
            return _array;
        }

        char* ptr()
        {
            return _array;
        }

        const char& operator[](int index) const
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return _array[index];
        }

        char& operator[](int index)
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return _array[index];
        }

        const char& at(int index) const
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return (*this)[index];
        }

        char& at(int index)
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
            return _length * sizeof(char);
        }

        void copy(const Array<char>& other)
        {
            copy(other._array, other._length);
        }

        void copy(const char* other, int count)
        {
            if (count > 0)
            {
                clear_resize(count);
                memcpy(_array, other, count * sizeof(char));
            }
            else
            {
                _length = 0;
            }
        }

        void concat(const Array<char>& other)
        {
            concat(other._array, other.size());
        }

        void concat(const char* other, int count)
        {
            if (count > 0)
            {
                int length = _length;
                resize(length + count);

                memcpy(_array + length, other, count * sizeof(char));
            }
        }

        int memcmp(const Array<char>& other) const
        {
            if (_length < other._length)
                return -1;
            if (_length > other._length)
                return -1;

            if (_length == 0)
                return 0;

            return ::memcmp(_array, other._array, _length * sizeof(char));
        }

        void remove(int idx, int span = 1)
        {
            if (idx < 0 || idx - _length - span + 1 >= 0)
                throw Error("remove(): invalid index %d with span %d (size=%d)", idx, span, _length);

            memmove(_array + idx, _array + idx + span, sizeof(char) * (_length - idx - span));
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

        int find(const char& value) const
        {
            return find(0, _length, value);
        }

        int find(int from, int to, const char& value) const
        {
            for (int i = from; i < to; i++)
                if (_array[i] == value)
                    return i;

            return -1;
        }

        int count(const char& value) const
        {
            return count(0, _length, value);
        }

        int count(int from, int to, const char& value) const
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

        void push(char elem)
        {
            resize(_length + 1);
            _array[_length - 1] = elem;
        }

        char& push()
        {
            resize(_length + 1);
            return _array[_length - 1];
        }

        char& pop()
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[--_length];
        }

        char& top()
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[_length - 1];
        }

        const char& top() const
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[_length - 1];
        }

        char& top(int offset)
        {
            if (_length - offset <= 0)
                throw Error("stack underflow");

            return _array[_length - 1 - offset];
        }

        const char& top(int offset) const
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

        void expandFill(int newsize, const char& value)
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

        void swap(Array<char>& other)
        {
            std::swap(_array, other._array);
            std::swap(_reserved, other._reserved);
            std::swap(_length, other._length);
        }

        char* begin()
        {
            return _array;
        }

        char* end()
        {
            return _array + _length;
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
        char* _array;

        int _reserved;
        int _length;

    private:
        Array(const Array&);                            // no implicit copy
        Array<int>& operator=(const Array<int>& right); // no copy constructor
    };

    template <>
    class Array<int>
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

                int* oldptr = _array;

                _array = static_cast<int*>(std::realloc(static_cast<void*>(_array), sizeof(int) * to_reserve));
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
                memset(_array, 0, _length * sizeof(int));
        }

        void fffill()
        {
            if (_length > 0)
                memset(_array, 0xFF, _length * sizeof(int));
        }

        void fill(const int& value)
        {
            for (int i = 0; i < size(); i++)
                _array[i] = value;
        }

        const int* ptr() const
        {
            return _array;
        }

        int* ptr()
        {
            return _array;
        }

        const int& operator[](int index) const
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return _array[index];
        }

        int& operator[](int index)
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return _array[index];
        }

        const int& at(int index) const
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return (*this)[index];
        }

        int& at(int index)
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
            return _length * sizeof(int);
        }

        void copy(const Array<int>& other)
        {
            copy(other._array, other._length);
        }

        void copy(const int* other, int count)
        {
            if (count > 0)
            {
                clear_resize(count);
                memcpy(_array, other, count * sizeof(int));
            }
            else
            {
                _length = 0;
            }
        }

        void concat(const Array<int>& other)
        {
            concat(other._array, other.size());
        }

        void concat(const int* other, int count)
        {
            if (count > 0)
            {
                int length = _length;
                resize(length + count);

                memcpy(_array + length, other, count * sizeof(int));
            }
        }

        int memcmp(const Array<int>& other) const
        {
            if (_length < other._length)
                return -1;
            if (_length > other._length)
                return -1;

            if (_length == 0)
                return 0;

            return ::memcmp(_array, other._array, _length * sizeof(int));
        }

        void remove(int idx, int span = 1)
        {
            if (idx < 0 || idx - _length - span + 1 >= 0)
                throw Error("remove(): invalid index %d with span %d (size=%d)", idx, span, _length);

            memmove(_array + idx, _array + idx + span, sizeof(int) * (_length - idx - span));
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

        int find(const int& value) const
        {
            return find(0, _length, value);
        }

        int find(int from, int to, const int& value) const
        {
            for (int i = from; i < to; i++)
                if (_array[i] == value)
                    return i;

            return -1;
        }

        int count(const int& value) const
        {
            return count(0, _length, value);
        }

        int count(int from, int to, const int& value) const
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

        void push(int elem)
        {
            resize(_length + 1);
            _array[_length - 1] = elem;
        }

        int& push()
        {
            resize(_length + 1);
            return _array[_length - 1];
        }

        int& emplace_back()
        {
            return push();
        }

        int& replace(int idx)
        {
            _array[idx] = 0;
            return _array[idx];
        }

        int& pop()
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[--_length];
        }

        int& top()
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[_length - 1];
        }

        const int& top() const
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[_length - 1];
        }

        int& top(int offset)
        {
            if (_length - offset <= 0)
                throw Error("stack underflow");

            return _array[_length - 1 - offset];
        }

        const int& top(int offset) const
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

        void expandFill(int newsize, const int& value)
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

        void swap(Array<int>& other)
        {
            std::swap(_array, other._array);
            std::swap(_reserved, other._reserved);
            std::swap(_length, other._length);
        }

        int* begin()
        {
            return _array;
        }

        int* end()
        {
            return _array + _length;
        }

        // CMP_FUNCTOR has two arguments and returns sign of comparation
        template <typename CmpFunctor>
        void insertionSort(int start, int end, CmpFunctor cmp)
        {
            int i, j;
            char tmp[sizeof(int)]; // can't use T directly because it may have destructor

            for (i = start + 1; i <= end; i++)
            {
                j = i;
                while (j > start && cmp(_array[j - 1], _array[j]) > 0)
                {
                    int* a1 = _array + j - 1;
                    int* a2 = a1 + 1;
                    memcpy(&tmp, a1, sizeof(int));
                    memcpy(a1, a2, sizeof(int));
                    memcpy(a2, &tmp, sizeof(int));
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
                int *lo, *hi;
            } stack[32], *sp;

            char tmp[sizeof(int)]; // can't use T directly because it may have destructor

            sp = stack;

            // push our initial values onto the stack
            sp->lo = _array + start;
            sp->hi = _array + end + 1;
            sp++;

            while (sp > stack)
            {
                // pop lo and hi off the stack
                sp--;
                int *high = sp->hi, *low = sp->lo;
                int* hi = high - 1;
                int* lo = low;
                int* pivot = low;

                while (1)
                {
                    while (lo < high && lo != pivot && cmp(*lo, *pivot) < 0)
                        lo++;

                    while (hi > low && (hi == pivot || cmp(*hi, *pivot) >= 0))
                        hi--;

                    if (lo < hi)
                    {
                        memcpy(&tmp, lo, sizeof(int));
                        memcpy(lo, hi, sizeof(int));
                        memcpy(hi, &tmp, sizeof(int));

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

    protected:
        int* _array;

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

    template <>
    class Array<byte>
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

                byte* oldptr = _array;

                _array = static_cast<byte*>(std::realloc(static_cast<void*>(_array), sizeof(byte) * to_reserve));
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
                memset(_array, 0, _length * sizeof(byte));
        }

        void fffill()
        {
            if (_length > 0)
                memset(_array, 0xFF, _length * sizeof(byte));
        }

        void fill(const byte& value)
        {
            for (int i = 0; i < size(); i++)
                _array[i] = value;
        }

        const byte* ptr() const
        {
            return _array;
        }

        byte* ptr()
        {
            return _array;
        }

        const byte& operator[](int index) const
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return _array[index];
        }

        byte& operator[](int index)
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return _array[index];
        }

        const byte& at(int index) const
        {
            if (index < 0 || _length - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _length);

            return (*this)[index];
        }

        byte& at(int index)
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
            return _length * sizeof(byte);
        }

        void copy(const Array<byte>& other)
        {
            copy(other._array, other._length);
        }

        void copy(const byte* other, int count)
        {
            if (count > 0)
            {
                clear_resize(count);
                memcpy(_array, other, count * sizeof(byte));
            }
            else
            {
                _length = 0;
            }
        }

        void concat(const Array<byte>& other)
        {
            concat(other._array, other.size());
        }

        void concat(const byte* other, int count)
        {
            if (count > 0)
            {
                int length = _length;
                resize(length + count);
                memcpy(_array + length, other, count * sizeof(byte));
            }
        }

        void remove(int idx, int span = 1)
        {
            if (idx < 0 || idx - _length - span + 1 >= 0)
                throw Error("remove(): invalid index %d with span %d (size=%d)", idx, span, _length);

            memmove(_array + idx, _array + idx + span, sizeof(byte) * (_length - idx - span));
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

        int count(const byte& value) const
        {
            return count(0, _length, value);
        }

        int count(int from, int to, const byte& value) const
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

        void push(byte elem)
        {
            resize(_length + 1);
            _array[_length - 1] = elem;
        }

        byte& push()
        {
            resize(_length + 1);
            return _array[_length - 1];
        }

        void pop_back()
        {
            if (_length <= 0)
                throw Error("stack underflow");
            --_length;
            memset(_array + _length, 0, sizeof(byte));
        }

        byte& top()
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[_length - 1];
        }

        const byte& top() const
        {
            if (_length <= 0)
                throw Error("stack underflow");

            return _array[_length - 1];
        }

        byte& top(int offset)
        {
            if (_length - offset <= 0)
                throw Error("stack underflow");

            return _array[_length - 1 - offset];
        }

        const byte& top(int offset) const
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

        void expandFill(int newsize, const byte& value)
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

        void swap(Array<byte>& other)
        {
            std::swap(_array, other._array);
            std::swap(_reserved, other._reserved);
            std::swap(_length, other._length);
        }

        byte* begin()
        {
            return _array;
        }

        byte* end()
        {
            return _array + _length;
        }

    protected:
        byte* _array;

        int _reserved;
        int _length;

    private:
        void* _context;
    };
} // namespace indigo

// operators defined here for use with ObjArray<> and ObjPool<>
template <typename T>
void* operator new(size_t size, T* allocated_area)
{
    return allocated_area;
}

#endif
