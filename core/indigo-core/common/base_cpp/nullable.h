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

#ifndef __nullable__
#define __nullable__

#include <utility>

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

namespace indigo
{

    DECL_EXCEPTION(NullableError);

    template <typename T>
    class Nullable
    {
    public:
        Nullable() : _has_value(false)
        {
            variable_name.readString("<Undefined>", true);
        }

        Nullable(const Nullable<T>& other) : _has_value(false)
        {
            variable_name.copy(other.variable_name);
            if (other._has_value)
                set(other._value);
        }

        Nullable(Nullable<T>&& other) : _has_value(false)
        {
            variable_name.copy(other.variable_name);
            if (other._has_value)
            {
                _moveValue(_value, std::move(other._value));
                _has_value = true;
            }
        }

        Nullable<T>& operator=(const Nullable<T>& other)
        {
            if (this == &other)
                return *this;
            if (other._has_value)
                set(other._value);
            else
                reset();
            return *this;
        }

        Nullable<T>& operator=(Nullable<T>&& other)
        {
            if (this == &other)
                return *this;
            if (other._has_value)
            {
                _moveValue(_value, std::move(other._value));
                _has_value = true;
            }
            else
                reset();
            return *this;
        }

        T& value()
        {
            if (!_has_value)
                throw Error("\"%s\" variable was not set", variable_name.ptr());
            return _value;
        }

        const T& value() const
        {
            if (!_has_value)
                throw Error("\"%s\" variable was not set", variable_name.ptr());
            return _value;
        }

        template <typename U>
        T value_or(U&& default_value) const
        {
            return _has_value ? _value : static_cast<T>(std::forward<U>(default_value));
        }

        explicit operator bool() const
        {
            return _has_value;
        }

        bool operator==(const Nullable<T>& other) const
        {
            if (_has_value != other._has_value)
                return false;
            return !_has_value || _value == other._value;
        }

        bool operator!=(const Nullable<T>& other) const
        {
            return !(*this == other);
        }

        bool operator==(const T& value) const
        {
            return _has_value && _value == value;
        }

        bool operator!=(const T& value) const
        {
            return !(*this == value);
        }

        friend bool operator==(const T& value, const Nullable<T>& nullable)
        {
            return nullable == value;
        }

        friend bool operator!=(const T& value, const Nullable<T>& nullable)
        {
            return !(nullable == value);
        }

        Nullable<T>& operator=(const T& value)
        {
            set(value);
            return *this;
        }

        void set(const T& value)
        {
            _copyValue(_value, value);
            _has_value = true;
        }

        void reset()
        {
            _has_value = false;
        }

        bool has_value() const
        {
            return _has_value;
        }

        void setName(const char* name)
        {
            variable_name.readString(name, true);
        }

        DECL_TPL_ERROR(NullableError);

    private:
        template <typename U>
        static void _copyValue(U& dst, const U& src)
        {
            dst = src;
        }

        template <typename U>
        static void _copyValue(Array<U>& dst, const Array<U>& src)
        {
            dst.copy(src);
        }

        template <typename U>
        static void _moveValue(U& dst, U&& src)
        {
            dst = std::move(src);
        }

        template <typename U>
        static void _moveValue(Array<U>& dst, Array<U>&& src)
        {
            dst.swap(src);
        }

        T _value;
        bool _has_value;
        Array<char> variable_name;
    };

} // namespace indigo

#endif // __nullable__
