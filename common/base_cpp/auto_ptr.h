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

#ifndef __auto_ptr__
#define __auto_ptr__

#include "base_cpp/exception.h"

namespace indigo {

DECL_EXCEPTION(AutoPtrError);

template <typename T> class AutoPtr
{
public:
   explicit AutoPtr (T *ptr = 0)
   {
      _ptr = ptr;
   }

   ~AutoPtr ()
   {
      delete _ptr;
   }

   T * get () const
   {
      return _ptr;
   }

   T & ref () const
   {
      if (_ptr == 0)
         throw Error("no reference");

      return *_ptr;
   }

   T * operator -> () const
   {
      if (_ptr == 0)
         throw Error("no reference");

      return _ptr;
   }

   T * release () 
   {
      if (_ptr == 0)
         throw Error("nothing to release");

      T *ptr = _ptr;

      _ptr = 0;
      return ptr;
   }

   void reset (T *ptr)
   {
      if (ptr != _ptr)
      {
         delete _ptr;
         _ptr = ptr;
      }
   }

   void free ()
   {
      reset(0);
   }

   AutoPtr<T>& operator= (T *ptr)
   {
      reset(ptr);
      return *this;
   }

   void create ()
   {
      reset(new T());
   }

   template<typename A>
   void create (A &a)
   {
      reset(new T(a));
   }

   DECL_TPL_ERROR(AutoPtrError);

protected:
   T *_ptr;

private:
   AutoPtr (const AutoPtr &); // no implicit copy
   AutoPtr<T> & operator = (const AutoPtr<T> &other);
};

}

#endif
