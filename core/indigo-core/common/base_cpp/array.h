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

#include "base_c/defs.h"
#include "base_cpp/exception.h"
#include <algorithm>
#include <stack>
#include <string>
#include <unordered_set>
#include <vector>
#include <array>

namespace indigo
{
    DECL_EXCEPTION(ArrayError);
    using IntPair =  std::array<int, 2>;
    using ArrayBool = std::vector<unsigned char>;
    class ArrayChar
    {
    public:
        DECL_TPL_ERROR(ArrayError);

        void clear()
        {
            _arr.clear();
        }

        void reserve(int to_reserve)
        {
            _arr.reserve(to_reserve);
        }

        const char* ptr() const
        {
            return _arr.data();
        }

        char* ptr()
        {
            return &_arr[0];
        }

        void copy(const ArrayChar& other)
        {
            copy(other._arr.data(), other.size());
        }

        void copy(const char* other, int count)
        {
            _arr.assign(other, count);
        }

        int size() const
        {
            return _arr.size();
        }

        int sizeInBytes() const
        {
            return _arr.size();
        }

        void push(char ch)
        {
            _arr.push_back(ch);
        }

        int memcmp(const ArrayChar other) const
        {
            if (_arr.size() < other.size())
                return -1;
            if (_arr.size() > other.size())
                return -1;

            if (_arr.size() == 0)
                return 0;

            return ::memcmp(_arr.data(), other._arr.data(), _arr.size());
        }

        const char& operator[](int index) const
        {
            if (index < 0 || _arr.size() - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _arr.size());
            return _arr[index];
        }

        char& operator[](int index)
        {
            if (index < 0 || _arr.size() - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _arr.size());
            return _arr[index];
        }

        void clear_resize(int newsize)
        {
            _arr.clear();
            _arr.resize(newsize, 0);
        }

        char pop()
        {
            char ch = _arr.back();
            _arr.pop_back();
            return ch;
        }

        void resize(int newsize)
        {
            _arr.resize(newsize);
        }

        void appendString(const char* str, bool keep_zero)
        {
            _arr.append(str);
            if (keep_zero)
                push(0);
        }

        void readString(const char* str, bool keep_zero)
        {
            clear();
            appendString(str, keep_zero);
        }

        void concat(const ArrayChar& other)
        {
            concat(other._arr.data(), other.size());
        }

        void concat(const char* other, int count)
        {
            _arr += std::string(other, count);
        }

        void zerofill()
        {
            if (_arr.size() > 0)
                memset(&_arr[0], 0, _arr.size());
        }

        int find(char value) const
        {
            return find(0, _arr.size(), value);
        }

        int find(int from, int to, char value) const
        {
            for (int i = from; i < to; i++)
                if (_arr[i] == value)
                    return i;
            return -1;
        }

        int count(int from, int to, char value) const
        {
            int cnt = 0;
            for (int i = from; i < to; i++)
                if (_arr[i] == value)
                    cnt++;
            return cnt;
        }

        int count(char value) const
        {
            return count(0, _arr.size(), value);
        }

        char& top()
        {
            return _arr.back();
        }

        const char& top() const
        {
            return _arr.back();
        }

        char& top(int offset)
        {
            if (_arr.size() - offset <= 0)
                throw Error("stack underflow");
            return _arr[_arr.size() - 1 - offset];
        }

        void remove(int idx, int span = 1)
        {
            if (idx < 0 || idx + span > _arr.size())
            {
                throw Error("remove(): invalid index %d with span %d (size=%d)", idx, span, _arr.size());
            }
            _arr.erase(idx, span);
        }

        void swap(ArrayChar& other)
        {
            _arr.swap(other._arr);
        }

        void upper(const char* source)
        {
            clear();
            while (*source != 0)
                push(::toupper(*source++));
            push(0);
        }

    private:
        std::string _arr;
    };

    template <typename T> class Array
    {
    public:
        DECL_TPL_ERROR(ArrayError);
        void clear()
        {
            _arr.clear();
        }

        void clear_resize(int newsize)
        {
            _arr.clear();
            _arr.resize(newsize);
        }

        void reserve(int to_reserve)
        {
            _arr.reserve(to_reserve);
        }

        const T* ptr() const
        {
            return _arr.data();
        }

        T* ptr()
        {
            return &_arr[0];
        }

        const T& operator[](int index) const
        {
            if (index < 0 || _arr.size() - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _arr.size());

            return _arr[index];
        }

        T& operator[](int index)
        {
            if (index < 0 || _arr.size() - index <= 0)
                throw Error("invalid index %d (size=%d)", index, _arr.size());

            return _arr[index];
        }

        const T& at(int index) const
        {
            return _arr[index];
        }

        T& at(int index)
        {
            return _arr[index];
        }

        int size() const
        {
            return _arr.size();
        }

        int sizeInBytes() const
        {
            return _arr.size() * sizeof(T);
        }

        const T& top() const
        {
            return _arr.back();
        }

        T& top()
        {
            return _arr.back();
        }

        void swap(Array<T>& other)
        {
            _arr.swap(other._arr);
        }

        void swap(int idx1, int idx2)
        {
            std::swap(_arr[idx1], _arr[idx2]);
        }

        void zerofill()
        {
            if (_arr.size())
                memset(&_arr[0], 0, _arr.size() * sizeof(T));
        }

        void fffill()
        {
            if (_arr.size())
                memset(&_arr[0], 0xFF, _arr.size() * sizeof(T));
        }

        void fill(const T& value)
        {
            std::fill(_arr.begin(), _arr.end(), value);
        }

        void resize(int newsize)
        {
            _arr.resize(newsize);
        }

        int find(const T& value) const // TODO:: remove it!!!
        {
            for (int i = 0; i < _arr.size(); i++)
                if (_arr[i] == value)
                    return i;
            return -1;
        }

        void push(T elem)
        {
            _arr.push_back(elem);
        }

        T& push()
        {
            _arr.emplace_back();
            return _arr.back();
        }

        T& pop()
        {
            T& res = _arr.back();
            _arr.pop_back();
            return res;
        }

        void copy(const Array<T>& other)
        {
            _arr = other._arr;
        }

        void copy(const T* other, int count)
        {
            _arr = std::vector<T>(other, other + count);
        }

        void concat(const Array<T>& other)
        {
            _arr.insert(_arr.end(), other._arr.begin(), other._arr.end());
        }

        void concat(const T* other, int count)
        {
            std::vector<T> tmp(other, other + count);
            _arr.insert(_arr.end(), tmp.begin(), tmp.end());
        }

        void expand(int newsize)
        {
            resize(newsize);
        }

        void expandFill(int newsize, const T& value)
        {
            while (_arr.size() < newsize)
                push(value);
        }

        void remove(int idx, int span = 1)
        {
            auto beg = _arr.begin();
            std::advance(beg, idx);
            auto end = beg;
            std::advance(end, span);
            _arr.erase(beg, end);
        }

        // TODO: implement qsort

        template <typename CmpFunctor> void qsort(int start, int end, CmpFunctor cmp)
        {
        }

        template <typename T1, typename T2> void qsort(int start, int end, int (*cmp)(T1, T2, void*), void* context)
        {
        }

        template <typename T1, typename T2> void qsort(int (*cmp)(T1, T2, void*), void* context)
        {
        }


    private:
        std::vector<T> _arr;
    };

    template <typename T> class ArrayDeprecated
    {
    public:
        DECL_TPL_ERROR(ArrayError);

        explicit ArrayDeprecated() : _reserved(0), _length(0), _array(nullptr)
        {
        }

        ~ArrayDeprecated()
        {
            if (_array != nullptr)
            {
                free(_array);
                _array = nullptr;
            }
        }

        void clear()
        {
            _length = 0;
        }

        void reserve(int to_reserve)
        {
            // Addtional check for unexpectedly large memory allocations (larger than 1 GB)
            if (to_reserve * sizeof(T) >= 1 << 30)
                throw Error("memory to reserve (%d x %d) is larger than the allowed threshold", to_reserve, (int)sizeof(T));

            if (to_reserve <= 0)
                throw Error("to_reserve = %d", to_reserve);

            if (to_reserve > _reserved)
            {
                if (_length < 1)
                {
                    if (_array != nullptr)
                    {
                        free(_array);
                        _array = nullptr;
                    }
                }

                T* oldptr = _array;

                _array = (T*)realloc(_array, sizeof(T) * to_reserve);
                if (_array == nullptr)
                {
                    _array = oldptr;
                    throw Error("reserve(): no memory");
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
            return (*this)[index];
        }

        T& at(int index)
        {
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

            T tmp;

            memcpy(&tmp, _array + idx1, sizeof(T));
            memcpy(_array + idx1, _array + idx2, sizeof(T));
            memcpy(_array + idx2, &tmp, sizeof(T));
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
            T* tmp_t;
            __swap(_array, other._array, tmp_t);
            int tmp_int;
            __swap(_reserved, other._reserved, tmp_int);
            __swap(_length, other._length, tmp_int);
        }

        // CMP_FUNCTOR has two arguments and returns sign of comparation
        template <typename CmpFunctor> void insertionSort(int start, int end, CmpFunctor cmp)
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
        template <typename CmpFunctor> void qsort(int start, int end, CmpFunctor cmp)
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

        template <typename T1, typename T2> void qsort(int start, int end, int (*cmp)(T1, T2, void*), void* context)
        {
            this->qsort(start, end, _CmpFunctorCaller<T1, T2>(cmp, context));
        }

        template <typename T1, typename T2> void qsort(int (*cmp)(T1, T2, void*), void* context)
        {
            this->qsort(0, _length - 1, cmp, context);
        }

    protected:
        T* _array;

        int _reserved;
        int _length;

    private:
        ArrayDeprecated(const ArrayDeprecated&);                  // no implicit copy
        ArrayDeprecated<int>& operator=(const ArrayDeprecated<int>& right); // no copy constructor

        template <typename T1, typename T2> class _CmpFunctorCaller
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

// operators defined here for use with ObjArray<> and ObjPool<>
template <typename T> void* operator new(size_t size, T* allocated_area)
{
    return allocated_area;
}

#endif
