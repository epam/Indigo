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

#ifndef __nullable__
#define __nullable__

#include "base_cpp/exception.h"

namespace indigo
{

DECL_EXCEPTION(NullableError);

template <typename T>
class Nullable
{
public:
   Nullable () : _has_value(false), variable_name("<Undefined>") {}

   const T& get () const
   {
      if (!_has_value)
         throw Error("%s variable was not set");
      return _value;
   }

   operator const T& () const
   {
      return get();
   }

   Nullable<T>& operator= (const T& value)
   {
      set(value);
      return *this;
   }

   void set (const T& value)
   {
      _value = value;
      _has_value = true;
   }

   void reset ()
   {
      _has_value = false;
   }

   bool hasValue () const
   {
      return _has_value;
   }

   void setName (const char *name)
   {
      variable_name = name;
   }

   DECL_TPL_ERROR(NullableError);

private:
   T _value;
   bool _has_value;
   const char *variable_name;
};

}

#endif // __nullable__
