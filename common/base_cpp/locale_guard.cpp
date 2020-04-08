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

#include "base_cpp/locale_guard.h"

using namespace indigo;

LocaleGuard::LocaleGuard()
{
#if defined(__linux__) || defined(__APPLE__)
    _locale = newlocale(LC_NUMERIC_MASK, "C", NULL);
    if (_locale != NULL)
        _baselocale = uselocale(_locale);
    else
        _baselocale = NULL;
#endif
}

LocaleGuard::~LocaleGuard()
{
#if defined(__linux__) || defined(__APPLE__)
    if (_locale != NULL)
    {
        uselocale(_baselocale);
        freelocale(_locale);
    }
#endif
}
