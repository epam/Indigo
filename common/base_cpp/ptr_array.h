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

#ifndef __ptr_array__
#define __ptr_array__

#include "base_cpp/array.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

DECL_EXCEPTION(PtrArrayError);

template <typename T> class PtrArray
{
public:
   explicit PtrArray ()
   {
   }

   virtual ~PtrArray ()
   {
      clear();
   }

   DECL_TPL_ERROR(PtrArrayError);

   T & add (T *obj)
   {
      _ptrarray.push(obj);
      return *obj;
   }

   T * pop (void)
   {
      return _ptrarray.pop();
   }

   T * top (void)
   {
      return at(size() - 1);
   }

   void expand (int newsize)
   {
      while (_ptrarray.size() < newsize)
         _ptrarray.push(0);
   }

   void clear (void)
   {
      int i;

      for (i = 0; i < _ptrarray.size(); i++)
      {
         if (_ptrarray[i] == 0)
            continue;

         delete _ptrarray[i];
         _ptrarray[i] = 0;
      }
      _ptrarray.clear();
   }

   int size () const { return _ptrarray.size(); }

   void resize (const int newsize)
   {
      int i, oldsize = _ptrarray.size();
      for (int i = newsize; i < oldsize; i++)
      {
         if (_ptrarray[i] == 0)
            continue;

         delete _ptrarray[i];
         _ptrarray[i] = 0;         
      }
      _ptrarray.resize(newsize);
      for (i = oldsize; i < newsize; i++)
         _ptrarray[i] = 0;
   }

   void removeLast ()
   {
      delete _ptrarray.pop();
   }

   void remove (int idx)
   {
      delete _ptrarray[idx];

      _ptrarray.remove(idx);
   }

   void set (int idx, T *obj)
   {
      if (_ptrarray[idx] != 0)
         throw Error("object #%d already set", idx);
      
      _ptrarray[idx] = obj;
   }

   void reset (int idx)
   {
      delete _ptrarray[idx];
      _ptrarray[idx] = 0;
   }

   T * release (int idx)
   {
      T *result = _ptrarray[idx];
      _ptrarray[idx] = 0;
      return result;
   }

   void qsort (int (*cmp)(T * const &, T * const &, const void *), const void *context)
   {
      _ptrarray.qsort(cmp, context);
   }

   
   const T *  operator[] (int index) const { return _ptrarray[index]; }
         T *& operator[] (int index)       { return _ptrarray[index]; }

   const T *  at (int index) const { return _ptrarray[index]; }
         T *& at (int index)       { return _ptrarray[index]; }

   const T * const* ptr () const { return _ptrarray.ptr(); }
               T ** ptr ()       { return _ptrarray.ptr(); }

protected:
   Array<T *> _ptrarray;
private:
   PtrArray (const PtrArray &); // no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
