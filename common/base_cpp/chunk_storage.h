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

#ifndef __chunk_storage_h__
#define __chunk_storage_h__

// Storage for chunks of bytes

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

namespace indigo
{

class ChunkStorage
{
public:
   ChunkStorage ();

   void  clear ();
   byte* add   (int n_bytes);
   void  add   (const byte *data, int n_bytes);
   void  add   (const Array<char> &data);
   void  add   (const char *str);

   int   count   (void);

   byte* get     (int i);
   int   getSize (int i);

   void pop ();

   DECL_ERROR;
private:
   Array<byte> _arr;
   Array<int> _offset;
};

}

#endif // __chunk_storage_h__
