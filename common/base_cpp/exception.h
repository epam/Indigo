/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

class DLLEXPORT Exception
{
public:
   explicit Exception (const char *format, ...);
   virtual ~Exception ();

   int code ();
   const char * message ();
   void appendMessage (const char *format, ...);

   virtual Exception * clone      ();
   virtual void        throwSelf  ();

   Exception (const Exception &);
protected:
   explicit Exception ();

   void _init (const char *format, va_list args);
   void _init (const char *prefix, const char *format, va_list args);

   void _cloneTo (Exception *dest) const;

   int  _code;
   char _message[1024];
private:
};

}

#define DEF_ERROR(error_prefix) \
   class Error : public indigo::Exception             \
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
