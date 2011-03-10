/****************************************************************************
 * Copyright (C) 2011 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#ifndef __locale_guard__
#define __locale_guard__

#if defined(__linux__) || defined(__APPLE__)
#include <locale.h>
#include <xlocale.h>
#endif

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

}

#endif
