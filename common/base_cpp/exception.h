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

#define DECL_EXCEPTION(ExceptionName) \
   class DLLEXPORT ExceptionName : public indigo::Exception       \
   {                                                              \
   public:                                                        \
      explicit ExceptionName (const char *format, ...);           \
      virtual ~ExceptionName ();                                  \
      virtual Exception* clone ();                                \
      virtual void throwSelf ();                                  \
      ExceptionName (const ExceptionName &other);                 \
   }

#define IMPL_EXCEPTION(Namespace, ExceptionName, prefix) \
   Namespace::ExceptionName::ExceptionName (const char *format, ...) \
      : indigo::Exception()                                                            \
   {                                                                                   \
      va_list args;                                                                    \
                                                                                       \
      va_start(args, format);                                                          \
      _init(prefix, format, args);                                                     \
      va_end(args);                                                                    \
   }                                                                                   \
                                                                                       \
   Namespace::ExceptionName::~ExceptionName () {}                                      \
                                                                                       \
   indigo::Exception* Namespace::ExceptionName::clone ()                                       \
   {                                                                                   \
      ExceptionName *error = new ExceptionName("");                                    \
      _cloneTo(error);                                                                 \
      return error;                                                                    \
   }                                                                                   \
   void Namespace::ExceptionName::throwSelf ()                                         \
   {                                                                                   \
      throw *this;                                                                     \
   }                                                                                   \
   Namespace::ExceptionName::ExceptionName (const ExceptionName &other) : Exception () \
   {                                                                                   \
      other._cloneTo(this);                                                            \
   }                                                                                   \

#define DECL_ERROR DECL_EXCEPTION(Error)
#define DECL_TPL_ERROR(CommonErrorName) typedef CommonErrorName Error

#define IMPL_ERROR(Namespace, error_prefix) \
   IMPL_EXCEPTION(Namespace, Error, error_prefix)

// Deprecated

#define DEF_EXCEPTION(ExceptionName, prefix) \
   class ExceptionName : public indigo::Exception                 \
   {                                                              \
   public:                                                        \
      explicit ExceptionName (const char *format, ...) :          \
      Exception()                                                 \
      {                                                           \
         va_list args;                                            \
                                                                  \
         va_start(args, format);                                  \
         _init(prefix, format, args);                             \
         va_end(args);                                            \
      }                                                           \
                                                                  \
      virtual ~ExceptionName () {}                                \
      virtual Exception* clone ()                                 \
      {                                                           \
         ExceptionName *error = new ExceptionName("");            \
         _cloneTo(error);                                         \
         return error;                                            \
      }                                                           \
      virtual void throwSelf ()                                   \
      {                                                           \
         throw *this;                                             \
      }                                                           \
      ExceptionName (const ExceptionName &other) : Exception ()   \
      {                                                           \
         other._cloneTo(this);                                    \
      }                                                           \
   }

#define DEF_ERROR(error_prefix) \
   DEF_EXCEPTION(Error, error_prefix)

#define DEF_TIMEOUT_EXCEPTION(prefix) \
   DEF_EXCEPTION(TimeoutException, prefix " timeout")

}

#endif // __exception_h__
