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

#include <stdio.h>

#include "base_cpp/io_base.h"

#if defined(_WIN32) && !defined(__MINGW32__)
#include <fcntl.h>
#include <io.h>
#include <windows.h>
#endif

using namespace indigo;

FILE* indigo::openFile(Encoding filename_encoding, const char* filename, const char* mode)
{
    FILE* file = 0;
#if defined(_WIN32) && !defined(__MINGW32__)
    if (filename_encoding == ENCODING_UTF8)
    {
        wchar_t w_filename[1024];
        wchar_t w_mode[10];

        MultiByteToWideChar(CP_UTF8, 0, filename, -1, w_filename, 1024);
        MultiByteToWideChar(CP_UTF8, 0, mode, -1, w_mode, 10);

        file = _wfopen(w_filename, w_mode);
    }
#endif
    if (file == 0)
    {
        // For ASCII and linux
        file = fopen(filename, mode);
    }

    return file;
}

#if defined(_WIN32) && !defined(__MINGW32__)
CLocale CLocale::instance;

_locale_t indigo::getCLocale()
{
    return CLocale::instance.get();
}

CLocale::CLocale()
{
    _locale = _create_locale(LC_ALL, "C");
}

CLocale::~CLocale()
{
    _free_locale(_locale);
}

_locale_t CLocale::get()
{
    return _locale;
}

#endif
