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

#ifndef __obj_h__
#define __obj_h__

#include "base_cpp/exception.h"

namespace indigo {

DECL_EXCEPTION(ObjError);

// Reusable storage for object
template <typename T> class Obj
{
public:
   Obj () : _initialized(false)
   {
   }

   ~Obj ()
   {
      free();
   }

   T * get () const
   {
      if (!_initialized)
         return 0;
      return _ptr();
   }

   T * operator -> () const
   {
      if (!_initialized)
         throw Error("no object");

      return _ptr();
   }

   T & ref () const
   {
      if (!_initialized)
         throw Error("no object");

      return *_ptr();
   }

   T & create ()
   {
      if (_initialized)
         throw Error("create(): already have object");

      new (_storage) T();
      _initialized = true;
      return *_ptr();
   }

   template<typename A> T & create (A &a)
   {
      if (_initialized)
         throw Error("create(): already have object");

      new (_storage) T(a);
      _initialized = true;
      return *_ptr();
   }

   template<typename A> T & recreate (A &a)
   {
      free();
      return create(a);
   }

   template<typename A> T & create (const A &a)
   {
      if (_initialized)
         throw Error("create(): already have object");

      new (_storage) T(a);
      _initialized = true;
      return *_ptr();
   }

   template<typename A, typename B> T & create (A &a, B &b)
   {
      if (_initialized)
         throw Error("create(): already have object");

      new (_storage) T(a, b);
      _initialized = true;
      return *_ptr();
   }

   template<typename A, typename B> T & create (A &a, B *b)
   {
      if (_initialized)
         throw Error("create(): already have object");

      new (_storage) T(a, b);
      _initialized = true;
      return *_ptr();
   }

   template<typename A, typename B, typename C> T & create (A &a, B &b, C *c)
   {
      if (_initialized)
         throw Error("create(): already have object");

      new (_storage) T(a, b, c);
      _initialized = true;
      return *_ptr();
   }

   template<typename A, typename B, typename C> T & create (A &a, B &b, C &c)
   {
      if (_initialized)
         throw Error("create(): already have object");

      new (_storage) T(a, b, c);
      _initialized = true;
      return *_ptr();
   }

   template<typename A, typename B, typename C> T & create (A &a, B *b, C &c)
   {
      if (_initialized)
         throw Error("create(): already have object");

      new (_storage) T(a, b, c);
      _initialized = true;
      return *_ptr();
   }

   void free ()
   {
      if (_initialized)
      {
         _ptr()->~T();
         _initialized = false;
      }
   }

   DECL_TPL_ERROR(ObjError);
protected:
   T* _ptr () const
   {
      return (T*)_storage;
   }

   char _storage[sizeof(T)];
   bool _initialized;

private:
   Obj (const Obj &); // no implicit copy
};

}

#endif
