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

#ifndef __obj_list_h__
#define __obj_list_h__

#include "base_cpp/list.h"

namespace indigo {

template <typename T> class ObjList
{
public:

   typedef typename List<T>::Elem Elem;

   ObjList ()
   {
   }

   explicit ObjList (Pool<Elem> &pool) : _list(pool)
   {
   }

   ~ObjList ()
   {
      clear();
   }

   int add ()
   {     
      int idx = _list.add();

      new (&_list[idx]) T();

      return idx;      
   }

   template <typename A> int add (A &a)
   {     
      int idx = _list.add();

      new (&_list[idx]) T(a);

      return idx;      
   }

   int insertAfter (int existing)
   {
      int idx = _list.insertAfter(existing);

      new (&_list[idx]) T();

      return idx;
   }

   template <typename A> int insertAfter (int existing, A &a)
   {
      int idx = _list.insertAfter(existing);

      new (&_list[idx]) T(a);

      return idx;
   }

   int insertBefore (int existing)
   {
      int idx = _list.insertBefore(existing);

      new (&_list[idx]) T();

      return idx;
   }

   template <typename A> int insertBefore (int existing, A &a)
   {
      int idx = _list.insertBefore(existing);

      new (&_list[idx]) T(a);

      return idx;
   }

   void remove (int idx)
   {
      _list[idx].~T();
      _list.remove(idx);
   }

   int size  () const { return _list.size(); }
   int begin () const { return _list.begin(); }
   int end () const { return _list.end(); }
   int next (int idx) const { return _list.next(idx); }

   void clear ()
   {
      while (_list.size() > 0)
         remove(_list.tail());
   }

   T & operator [] (int index) const
   {                        
      return _list[index];
   }

   T & at (int index) const
   {                        
      return _list[index];
   }

protected:

   List<T> _list;

private:

   ObjList (const ObjList<T> & ); // no implicit copy
};

}

#endif
