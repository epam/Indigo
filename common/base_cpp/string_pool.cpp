/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#include <string.h>

#include "base_cpp/string_pool.h"

using namespace indigo;

StringPool::StringPool ()
{
}

StringPool::~StringPool ()
{
}

int StringPool::add (const char *str)
{
   int idx = _pool.add();
   Desc &desc = _pool[idx];

   desc.start = _storage.size();
   desc.length = (int)strlen(str) + 1;

   _storage.resize(desc.start + desc.length);
   if (desc.length > 0)
      memcpy(_storage.ptr() + desc.start, str, desc.length - 1);
   _storage[desc.start + desc.length - 1] = 0;
   return idx;
}

int StringPool::add (int size)
{
   int idx = _pool.add();
   Desc &desc = _pool[idx];

   desc.start = _storage.size();
   desc.length = size;

   _storage.resize(desc.start + desc.length);
   return idx;
}

void StringPool::remove (int idx)
{
   _pool.remove(idx);
}

char * StringPool::at (int idx)
{
   return _storage.ptr() + _pool[idx].start;
}

const char * StringPool::at (int idx) const
{
   return _storage.ptr() + _pool[idx].start;
}

int StringPool::size () const
{
   return _pool.size();
}

int StringPool::begin () const
{
   return _pool.begin();
}

int StringPool::end () const
{
   return _pool.end();
}

int StringPool::next (int i) const
{
   return _pool.next(i);
}

void StringPool::clear ()
{
   _pool.clear();
   _storage.clear();
}
