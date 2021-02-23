/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "base_cpp/chunk_storage.h"

using namespace indigo;

IMPL_ERROR(ChunkStorage, "chunk storage");

ChunkStorage::ChunkStorage()
{
    _offset.push(0);
}

void ChunkStorage::clear()
{
    _arr.clear();
    _offset.clear();
    _offset.push(0);
}

byte* ChunkStorage::add(int n_bytes)
{
    int prev_size = _offset.top();
    _arr.resize(prev_size + n_bytes);
    _offset.push(prev_size + n_bytes);
    return _arr.ptr() + prev_size;
}

void ChunkStorage::add(const byte* data, int n_bytes)
{
    byte* ptr = add(n_bytes);
    memcpy(ptr, data, n_bytes);
}

void ChunkStorage::add(const Array<char>& data)
{
    add((const byte*)data.ptr(), data.size());
}

void ChunkStorage::add(const char* str)
{
    add((const byte*)str, strlen(str) + 1);
}

byte* ChunkStorage::get(int i)
{
    return _arr.ptr() + _offset[i];
}

int ChunkStorage::getSize(int i)
{
    return _offset[i + 1] - _offset[i];
}

int ChunkStorage::count(void)
{
    return _offset.size() - 1;
}

void ChunkStorage::pop()
{
    if (count() == 0)
        throw Error("Cannot pop element from empty chunk storage");
    _offset.pop();
    _arr.resize(_offset[_offset.size() - 1]);
}
