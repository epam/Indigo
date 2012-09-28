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

#ifndef __objarray_h__
#define __objarray_h__

#include "base_cpp/array.h"

namespace indigo {

template <typename T> class ObjArray
{
public:
   explicit ObjArray ()
   {
   }

   ~ObjArray ()
   {
      while (size() > 0)
         pop();
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

   int size (void) const { return _array.size(); }

   T & push ()
   {
      void *addr = &_array.push();

      new (addr) T();

      return _array.top();
   }

   template <typename A> T & push (A &a)
   {
      void *addr = &_array.push();

      new (addr) T(a);

      return _array.top();
   }

   template <typename A, typename B> T & push (A &a, B *b)
   {
      void *addr = &_array.push();

      new (addr) T(a, b);

      return _array.top();
   }

   template <typename A, typename B, typename C> T & push (A &a, B *b, C c)
   {
      void *addr = &_array.push();

      new (addr) T(a, b, c);

      return _array.top();
   }

   void clear ()
   {
      while (size() > 0)
         pop();
   }

   void resize (int newSize)
   {
      while  (newSize < size()) {
         pop();
      }

      while  (newSize > size()) {
         push();
      }
   }
   void reserve (int size)
   {
      _array.reserve(size);
   }

   void expand (int newSize)
   {
      while  (newSize > size()) {
         push();
      }
   }

   void remove (int idx)
   {
      _array[idx].~T();
      _array.remove(idx);
   }

   T & top ()
   {
      return _array.top();
   }

   const T & top () const
   {
      return _array.top();
   }

   void pop ()
   {
      _array.top().~T();
      _array.pop();
   }

   template <typename T1, typename T2>
   void qsort (int (*cmp)(T1, T2, void *), void *context)
   {
      _array.qsort(cmp, context);
   }
   
protected:

   Array<T> _array;
private:
   ObjArray (const ObjArray &); // no implicit copy
};

}

#endif
