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

#ifndef __queue_h__
#define __queue_h__

#include "base_c/defs.h"
#include "base_cpp/exception.h"
#include "base_cpp/array.h"

namespace indigo {

DECL_EXCEPTION(QueueError);

// Queue with fixed max length
template <typename T> class Queue
{
public:
   DECL_TPL_ERROR(QueueError);

   explicit Queue (void)
   {
      _start = 0;
      _end = 0;
   }

   void setLength (int max_size)
   {
      _array.resize(max_size);
   }

   void clear  (void)
   {
      _start = 0;
      _end = 0;
   }

   bool isEmpty (void)
   {
      return _start == _end;
   }

   T & push (const T& elem)
   {
      int idx = (_end + 1) % _array.size();
      if (idx == _start)
         throw Error("queue is full");
      int end = _end;
      _array[_end] = elem;
      _end = idx;
      return _array[end];
   }
   
   T& pop (void)
   {
      if (isEmpty())
         throw Error("queue is empty");
      int idx = _start;
      _start = (_start + 1) % _array.size();
      return _array[idx];
   }

protected:
   Array<T> _array;
   int _start, _end;

private:
   Queue (const Queue &); // no implicit copy
};

}

#endif // __queue_h__
