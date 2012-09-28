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

#include "base_cpp/chunk_storage.h"

using namespace indigo;

IMPL_ERROR(ChunkStorage, "chunk storage");

ChunkStorage::ChunkStorage ()
{
   _offset.push(0);
}

void ChunkStorage::clear ()
{
   _arr.clear();
   _offset.clear();
   _offset.push(0);
}

byte* ChunkStorage::add (int n_bytes)
{
   int prev_size = _offset.top();
   _arr.resize(prev_size + n_bytes);
   _offset.push(prev_size + n_bytes);
   return _arr.ptr() + prev_size;
}

void ChunkStorage::add (const byte *data, int n_bytes)
{
   byte *ptr = add(n_bytes);
   memcpy(ptr, data, n_bytes);
}

void ChunkStorage::add (const Array<char> &data)
{
   add((const byte *)data.ptr(), data.size());
}

void ChunkStorage::add (const char *str)
{
   add((const byte *)str, strlen(str) + 1);
}

byte* ChunkStorage::get (int i)
{
   return _arr.ptr() + _offset[i];
}

int ChunkStorage::getSize (int i)
{
   return _offset[i + 1] - _offset[i];
}

int ChunkStorage::count (void)
{
   return _offset.size() - 1;
}

void ChunkStorage::pop ()
{
   if (count() == 0)
      throw Error("Cannot pop element from empty chunk storage");
   _offset.pop();
   _arr.resize(_offset[_offset.size() - 1]);
}
