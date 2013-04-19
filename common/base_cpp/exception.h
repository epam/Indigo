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

#define DECL_EXCEPTION_BODY(ExceptionName, Parent) \
   ExceptionName : public Parent       \
   {                                                              \
   public:                                                        \
      explicit ExceptionName (const char *format, ...);           \
      virtual ~ExceptionName ();                                  \
      virtual Exception* clone ();                                \
      virtual void throwSelf ();                                  \
      ExceptionName (const ExceptionName &other);                 \
   protected:                                                     \
      explicit ExceptionName ();           \
   }

#define DECL_EXCEPTION2(ExceptionName, Parent) \
   class DLLEXPORT DECL_EXCEPTION_BODY(ExceptionName, Parent)

#define DECL_EXCEPTION(ExceptionName) DECL_EXCEPTION2(ExceptionName, indigo::Exception)

#define DECL_EXCEPTION_NO_EXP2(ExceptionName, Parent) \
   class DECL_EXCEPTION_BODY(ExceptionName, Parent)

#define DECL_EXCEPTION_NO_EXP(ExceptionName) DECL_EXCEPTION_NO_EXP2(ExceptionName, indigo::Exception)

#define IMPL_EXCEPTION2(Namespace, ExceptionName, Parent, prefix) \
   Namespace::ExceptionName::ExceptionName () \
   {                                                                                   \
   }                                                                                   \
                                                                                       \
   Namespace::ExceptionName::ExceptionName (const char *format, ...)                   \
      : Parent()                                                                       \
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
   Namespace::ExceptionName::ExceptionName (const ExceptionName &other) : Parent () \
   {                                                                                   \
      other._cloneTo(this);                                                            \
   }                                                                                   \

#define IMPL_EXCEPTION(Namespace, ExceptionName, prefix) \
   IMPL_EXCEPTION2(Namespace, ExceptionName, indigo::Exception, prefix)

#define DECL_ERROR2(Parent) DECL_EXCEPTION2(Error, Parent)
#define DECL_ERROR DECL_ERROR2(indigo::Exception)
#define DECL_ERROR_NO_EXP DECL_EXCEPTION_NO_EXP(Error)

#define DECL_TPL_ERROR(CommonErrorName) typedef CommonErrorName Error

#define IMPL_ERROR2(Namespace, Parent, error_prefix) \
   IMPL_EXCEPTION2(Namespace, Error, Parent, error_prefix)

#define IMPL_ERROR(Namespace, error_prefix) IMPL_ERROR2(Namespace, indigo::Exception, error_prefix)

#define DECL_TIMEOUT_EXCEPTION DECL_EXCEPTION(TimeoutException)
#define IMPL_TIMEOUT_EXCEPTION(Namespace, prefix) \
   IMPL_EXCEPTION(Namespace, TimeoutException, prefix " timeout")

}

#endif // __exception_h__
