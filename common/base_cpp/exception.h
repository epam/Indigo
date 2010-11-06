/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __exception_h__
#define __exception_h__

#include <stdarg.h>
#include <string.h>

#include "base_c/defs.h"

namespace indigo {

class Exception
{
public:
   DLLEXPORT explicit Exception (const char *format, ...);
   DLLEXPORT virtual ~Exception ();

   DLLEXPORT int code ();
   DLLEXPORT const char * message ();

   DLLEXPORT virtual Exception * clone      ();
   DLLEXPORT virtual void        throwSelf  ();

   DLLEXPORT Exception (const Exception &);
protected:
   DLLEXPORT explicit Exception ();

   DLLEXPORT void _init (const char *format, va_list args);
   DLLEXPORT void _init (const char *prefix, const char *format, va_list args);

   DLLEXPORT void _cloneTo (Exception *dest) const;

   int  _code;
   char _message[1024];
private:
};

}

#define DEF_ERROR(error_prefix) \
   class Error : public Exception                     \
   {                                                  \
   public:                                            \
      explicit Error (const char *format, ...) :      \
      Exception()                                     \
      {                                               \
         va_list args;                                \
                                                      \
         va_start(args, format);                      \
         _init(error_prefix, format, args);           \
         va_end(args);                                \
      }                                               \
                                                      \
      virtual ~Error () {}                            \
      virtual Exception* clone ()                     \
      {                                               \
         Error *error = new Error("");                \
         _cloneTo(error);                             \
         return error;                                \
      }                                               \
      virtual void throwSelf ()                       \
      {                                               \
         throw *this;                                 \
      }                                               \
      Error (const Error &other) : Exception ()\
      {\
         other._cloneTo(this); \
      }\
   }

#endif // __exception_h__
