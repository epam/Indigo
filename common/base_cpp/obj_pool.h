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

#ifndef __obj_pool_h__
#define __obj_pool_h__

#include "base_cpp/pool.h"

namespace indigo {

template <typename T> class ObjPool
{
public:
   ObjPool ()
   {
   }

   ~ObjPool ()
   {
      clear();
   }

   int add () 
   {
      int idx = _pool.add();

      void *addr = &_pool[idx];

      new (addr) T();

      return idx;
   }

   template<typename A> int add (A &a) 
   {
      int idx = _pool.add();

      void *addr = &_pool[idx];

      new (addr) T(a);

      return idx;
   }

   template<typename A, typename B> int add (A &a, B &b) 
   {
      int idx = _pool.add();

      void *addr = &_pool[idx];

      new (addr) T(a, b);

      return idx;
   }

   void remove (int idx)
   {
      T &elem = _pool[idx];

      elem.~T();
      _pool.remove(idx);
   }

   int size () const
   {
      return _pool.size();
   }

   int begin () const
   {
      return _pool.begin();
   }

   int next (int i) const
   {
      return _pool.next(i);
   }

   int end () const
   {
      return _pool.end();
   }

   void clear ()
   {
      for (int i = _pool.begin(); i != _pool.end(); i = _pool.next(i))
         _pool[i].~T();
      _pool.clear();
   }

   bool hasElement(int idx) const
   {
      return _pool.hasElement(idx);
   }

   const T & operator [] (int index) const
   {  
      return _pool[index];
   }

   T & operator [] (int index)
   {  
      return _pool[index];
   }

   const T & at (int index) const
   {                        
      return (*this)[index];
   }

   T & at (int index)
   {                        
      return (*this)[index];
   }

protected:
   Pool<T> _pool;

private:
   ObjPool (const ObjPool<T> & ); // no implicit copy
};

}

#endif
