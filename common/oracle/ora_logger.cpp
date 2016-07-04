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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "oracle/ora_logger.h"
#include "base_c/defs.h"

using namespace indigo;

int OracleLogger::dbgPrintfV (const char *format, va_list args)
{
   if (_file == NULL)
      return 0;

   int res = vfprintf(_file, format, args);

   fflush(_file);

   return res;
}

int OracleLogger::dbgPrintf (const char *format, ...)
{
   va_list args;
   int n;
  
   va_start(args, format);
   n = dbgPrintfV(format, args);
   va_end(args);

   return n;
}

OracleLogger::OracleLogger ()
{
   _file = NULL;
}

OracleLogger::~OracleLogger ()
{
   close();
}

void OracleLogger::close ()
{
   if (_file != NULL)
   {
      fclose(_file);
      _file = NULL;
   }
}

bool OracleLogger::init (const char *filename)
{
   char full_name[1024];
#ifdef _WIN32
   char *tmp_dir = getenv("TEMP");
   char path[1024];
   if (tmp_dir == NULL)
      strcpy(path, "C:\\");
   else
      snprintf(path, sizeof(path), "%s\\", tmp_dir);
#else
   char path[] = "/tmp/";
#endif
   close();

   snprintf(full_name, sizeof(full_name), "%s%s", path, filename);

   _file = fopen(full_name, "a+t");

   if (_file == NULL)
      return false;

   return true;
}

bool OracleLogger::isInited ()
{
   return _file != NULL;
}

bool OracleLogger::initIfClosed (const char *filename)
{
   if (isInited())
      return true;

   return init(filename);
}
