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

#ifndef __ora_logger__
#define __ora_logger__

#include <stdarg.h>
#include <stdio.h>

namespace indigo
{

class OracleLogger
{
public:

   explicit OracleLogger ();
   virtual ~OracleLogger ();

   bool init (const char *filename);
   bool initIfClosed (const char *filename);
   void close ();

   bool isInited ();

   int dbgPrintf (const char *format, ...);

   int dbgPrintfV (const char *format, va_list args);

private:
   FILE *_file;
};

}

#endif
