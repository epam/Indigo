/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#include <stdio.h>

#include "base_cpp/io_base.h"

#ifdef _WIN32
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#endif

using namespace indigo;

FILE *indigo::openFile( Encoding filename_encoding, const char *filename, const char *mode )
{
   FILE *file = 0;
#ifdef _WIN32
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

#ifdef _WIN32
CLocale CLocale::instance;

_locale_t indigo::getCLocale ()
{
   return CLocale::instance.get();
}

CLocale::CLocale ()
{
   _locale = _create_locale(LC_ALL, "C");
}

CLocale::~CLocale ()
{
   _free_locale(_locale);
}

_locale_t CLocale::get ()
{
   return _locale;
}

#endif
