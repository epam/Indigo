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

        const T& get() const
        {
            if (!_has_value)
                throw Error("\"%s\" variable was not set", variable_name.ptr());
            return _value;
        }

        operator const T&() const
        {
            return get();
        }

        Nullable<T>& operator=(const T& value)
        {
            set(value);
            return *this;
        }

        void set(const T& value)
        {
            _value = value;
            _has_value = true;
        }

        void reset()
        {
            _has_value = false;
        }

        bool hasValue() const
        {
            return _has_value;
        }

        void setName(const char* name)
        {
            variable_name.readString(name, true);
        }

        DECL_TPL_ERROR(NullableError);

    private:
        T _value;
        bool _has_value;
        Array<char> variable_name;
    };

} // namespace indigo

#endif // __nullable__
