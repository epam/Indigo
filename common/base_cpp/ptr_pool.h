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

#ifndef __ptr_pool__
#define __ptr_pool__

#include "base_cpp/pool.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

DECL_EXCEPTION(PtrPoolError);

template <typename T> class PtrPool
{
public:
   explicit PtrPool ()
   {
   }

   virtual ~PtrPool ()
   {
      clear();
   }

   DECL_TPL_ERROR(PtrPoolError);

   int add (T *obj)
   {
      return _ptrpool.add(obj);
   }

   void remove (int idx)
   {
      delete _ptrpool[idx];
      _ptrpool.remove(idx);
   }

   int size ()
   {
      return _ptrpool.size();
   }

   int begin ()
   {
      return _ptrpool.begin();
   }

   int end ()
   {
      return _ptrpool.end();
   }

   int next (int i)
   {
      return _ptrpool.next(i);
   }

   void clear (void)
   {
      int i;

      for (i = _ptrpool.begin(); i != _ptrpool.end(); i = _ptrpool.next(i))
         delete _ptrpool[i];

      _ptrpool.clear();
   }

   const T *  operator[] (int index) const { return _ptrpool[index]; }
         T *& operator[] (int index)       { return _ptrpool[index]; }

   const T *  at (int index) const { return _ptrpool[index]; }
         T *& at (int index)       { return _ptrpool[index]; }

protected:
   Pool<T *> _ptrpool;
private:
   PtrPool (const PtrPool &); // no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
