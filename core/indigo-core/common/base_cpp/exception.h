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

#ifndef __exception_h__
#define __exception_h__

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <type_traits>

#include "base_c/defs.h"

namespace indigo
{

    class DLLEXPORT Exception : public std::exception
    {
        Exception() = delete;

    public:
        Exception(const Exception&) = default;
        ~Exception() = default;

        explicit Exception(const char* format, ...);

        const char* message() const noexcept
        {
            return _message;
        };
        const char* what() const noexcept override
        {
            return _message;
        };
        void appendMessage(const char* format, ...);

    protected:
        char _message[1024];
    };

#define DECL_EXCEPTION_BODY(ExceptionName, Parent)                                                                                                             \
    ExceptionName:                                                                                                                                             \
public                                                                                                                                                         \
    Parent                                                                                                                                                     \
    {                                                                                                                                                          \
        ExceptionName() = delete;                                                                                                                              \
                                                                                                                                                               \
    public:                                                                                                                                                    \
        explicit ExceptionName(const char* format, ...);                                                                                                       \
    }

#define DECL_EXCEPTION2(ExceptionName, Parent) class DLLEXPORT DECL_EXCEPTION_BODY(ExceptionName, Parent)

#define DECL_EXCEPTION(ExceptionName) DECL_EXCEPTION2(ExceptionName, indigo::Exception)

#define DECL_EXCEPTION_NO_EXP2(ExceptionName, Parent) class DECL_EXCEPTION_BODY(ExceptionName, Parent)

#define DECL_EXCEPTION_NO_EXP(ExceptionName) DECL_EXCEPTION_NO_EXP2(ExceptionName, indigo::Exception)

#define IMPL_EXCEPTION2(Namespace, ExceptionName, Parent, prefix)                                                                                              \
    Namespace::ExceptionName::ExceptionName(const char* format, ...) : Parent(prefix ": ")                                                                     \
    {                                                                                                                                                          \
        va_list args;                                                                                                                                          \
        va_start(args, format);                                                                                                                                \
        const size_t len = strlen(_message);                                                                                                                   \
        vsnprintf(_message + len, sizeof(_message) - len, format, args);                                                                                       \
        va_end(args);                                                                                                                                          \
    }

#define IMPL_EXCEPTION(Namespace, ExceptionName, prefix) IMPL_EXCEPTION2(Namespace, ExceptionName, indigo::Exception, prefix)

#define DECL_ERROR2(Parent) DECL_EXCEPTION2(Error, Parent)
#define DECL_ERROR DECL_ERROR2(indigo::Exception)
#define DECL_ERROR_NO_EXP DECL_EXCEPTION_NO_EXP(Error)

#define DECL_TPL_ERROR(CommonErrorName) typedef CommonErrorName Error

#define IMPL_ERROR2(Namespace, Parent, error_prefix) IMPL_EXCEPTION2(Namespace, Error, Parent, error_prefix)

#define IMPL_ERROR(Namespace, error_prefix) IMPL_ERROR2(Namespace, indigo::Exception, error_prefix)

#define DECL_TIMEOUT_EXCEPTION DECL_EXCEPTION(TimeoutException)
#define IMPL_TIMEOUT_EXCEPTION(Namespace, prefix) IMPL_EXCEPTION(Namespace, TimeoutException, prefix " timeout")

} // namespace indigo

#endif // __exception_h__
