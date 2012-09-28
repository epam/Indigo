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

#ifndef __reusable_obj_array__
#define __reusable_obj_array__

#include "base_cpp/array.h"

namespace indigo {

template <typename T> class ReusableObjArray
{
public:
   explicit ReusableObjArray ()
   {
      _count = 0;
   }

   ~ReusableObjArray ()
   {
      for (int i = 0; i < _array.size(); i++) {
         _array[i].~T();
      }
   }

   const T & operator[] (int index) const
   {
      return _array[index];
   }

   T & operator[] (int index)
   {
      return _array[index];
   }

   const T & at (int index) const
   {
      return (*this)[index];
   }

   T & at (int index)
   {
      return (*this)[index];
   }

   int size (void) const { return _count; }

   void resize (const int newSize)
   {
      if (newSize <= _count) {
         _count = newSize;
      } else {
         _array.reserve(newSize);
         while (_count < newSize) {
            push();
         }
      }
   }

   T & push ()
   {
      T *addr;
      if (_count == _array.size()) {
         addr = &_array.push();
         new (addr) T();
      } else {
         addr = &_array[_count];
      }
      _count++;
      addr->clear();

      return *addr;
   }

   void pop ()
   {
      _count--;
   }

   T& top ()
   {
      return _array[_count - 1];
   }

   void clear ()
   {
      _count = 0;
   }

   void reserve (int to_reserve)
   {
      _array.reserve(to_reserve);
   }

protected:

   Array<T> _array;
   int      _count;
private:
   ReusableObjArray (const ReusableObjArray &); // no implicit copy
};

}

#endif
