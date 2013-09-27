/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#ifndef __bingo_error__
#define __bingo_error__

#include "base_cpp/exception.h"

using namespace indigo;

class BingoError : public Exception
{
public:
   explicit BingoError (const char *format, ...)
   {
      va_list args;

      va_start(args, format);
      _init("core", format, args);
      va_end(args);
   }
   BingoError (const BingoError &other) : Exception()
   {
       other._cloneTo(this);
   }
};

#endif
