/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#ifndef __cyclic_array_h__
#define __cyclic_array_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

namespace indigo {

// Cyclic array
template <typename T> struct CyclicArray
{
public:
   explicit CyclicArray (void)
   {
      _offset = 0;
   }

   void setSize (int max_size)
   {
      _array.resize(max_size);
   }
   
   void zeroFill (void)
   {
      _array.zerofill();
   }

   bool isInBound (int index) const
   {
      return index >= _offset && index < _offset + _array.size();
   }

   const T & operator [] (int index) const
   {
      int legnth = _array.size();
      if (legnth == 0) 
         throw Error("Zero length");
      int offset = index % _array.size();
      return index >= 0 ? _array[offset] : _array[length + offset];
   }

   T & operator [] (int index)
   {                        
      int length = _array.size();
      if (length == 0) 
         throw Error("Zero length");
      int offset = index % _array.size();
      return index >= 0 ? _array[offset] : _array[length + offset];
   }

   void setOffset (int offset)
   {
      _offset = offset;
   }

   DEF_ERROR("cycle array");

protected:
   Array<T> _array;
   int _offset;

private:
   CyclicArray (const CyclicArray &); // no implicit copy
};

}

#endif // __cyclic_array_h__
