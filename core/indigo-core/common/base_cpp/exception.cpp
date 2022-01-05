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

#include <string.h>

#include "base_cpp/exception.h"

using namespace indigo;

Exception::Exception(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    vsnprintf(_message, sizeof(_message), format, args);

    va_end(args);
}

void Exception::appendMessage(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    const size_t len = strlen(_message);
    vsnprintf(_message + len, sizeof(_message) - len, format, args);

    va_end(args);
}
