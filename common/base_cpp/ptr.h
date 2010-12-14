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

#ifndef __ptr_h__
#define __ptr_h__

#include "base_cpp/exception.h"

namespace indigo {

// Pointer with default constructor
template <typename T> class Ptr
{
public:
   Ptr () : _ptr(0)
   {
   }

   T* get ()
   {
      return _ptr;
   }

   void set (T *ptr)
   {
      _ptr = ptr;
   }

   T* operator-> ()
   {
      if (_ptr == 0)
         throw Error("no object");

      return _ptr;
   }

   T& operator* ()
   {
      if (_ptr == 0)
         throw Error("no object");

      return *_ptr;
   }

   T& ref ()
   {
      if (_ptr == 0)
         throw Error("no object");

      return *_ptr;
   }

   T* operator= (T* ptr)
   {
      return _ptr = ptr;
   }

   operator T* ()
   {
      if (_ptr == 0)
         throw Error("no object");

      return _ptr;
   }

   void dispose ()
   {
      if (_ptr != 0) {
         delete _ptr;
         _ptr = 0;
      }
   }

   ~Ptr () {
      dispose();
   }

   DEF_ERROR("ptr");
private:
   T *_ptr;

   Ptr (const Ptr &); // no implicit copy
};

}

#endif // __ptr_h__
