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

#ifndef __io_base_h__
#define __io_base_h__

#include <stdio.h>

#ifdef _WIN32
#include <locale.h>
#endif

namespace indigo
{
    enum Encoding
    {
        ENCODING_ASCII = 1,
        ENCODING_UTF8 = 2
    };

    FILE* openFile(Encoding filename_encoding, const char* filename, const char* mode);

#if defined(_WIN32) && !defined(__MINGW32__)
    _locale_t getCLocale();

    class CLocale
    {
    public:
        CLocale();
        ~CLocale();

        _locale_t get();

        static CLocale instance;

    protected:
        _locale_t _locale;
    };
#endif
}; // namespace indigo

#endif
