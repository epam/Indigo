/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef __io_base_h__
#define __io_base_h__

#include <stdio.h>

#ifdef _WIN32
#include <locale.h>
#endif

/*
When compiled on Linux with -D_FILE_OFFSET_BITS=64, the type off_t is compiled into
long long int
For 64-bit builds on Linux, -D_FILE_OFFSET_BITS=64 is always defined

Unfortunately, Windows defines its own off_t type as long, which makes it impossible
to use off_t across all platforms
With these directives, we use off_t as defined on Linux, and long long int on Windows
*/
#ifdef _WIN32
   typedef __int64 off_t_type;
#else
   typedef off_t off_t_type;
#endif

namespace indigo
{
   enum Encoding
   {
      ENCODING_ASCII = 1,
      ENCODING_UTF8 = 2
   };

   FILE *openFile( Encoding filename_encoding, const char *filename, const char *mode);

#if defined(_WIN32) && !defined(__MINGW32__)
   _locale_t getCLocale ();

   class CLocale
   {
   public:
      CLocale ();
      ~CLocale ();

      _locale_t get ();

      static CLocale instance;

   protected:
      _locale_t _locale;
   };
#endif
};

#endif
