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

#ifndef __locale_guard__
#define __locale_guard__

#if defined(__linux__) || defined(__APPLE__)
#include <locale.h>
#if defined(__APPLE__)
#include <xlocale.h>
#endif // __APPLE__
#endif // __linux__ || __APPLE__

namespace indigo
{

    class LocaleGuard
    {
    public:
        LocaleGuard();
        ~LocaleGuard();

    protected:
#if defined(__linux__) || defined(__APPLE__)
        locale_t _locale;
        locale_t _baselocale;
#endif
    };

} // namespace indigo

#endif
