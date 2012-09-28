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

#ifndef __cyclic_array_h__
#define __cyclic_array_h__

#include "base_cpp/array.h"

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
      return _array[index % _array.size()];
   }

   T & operator [] (int index)
   {                        
      return _array[index % _array.size()];
   }

   void setOffset (int offset)
   {
      _offset = offset;
   }

protected:
   Array<T> _array;
   int _offset;

private:
   CyclicArray (const CyclicArray &); // no implicit copy
};

}

#endif // __cyclic_array_h__
