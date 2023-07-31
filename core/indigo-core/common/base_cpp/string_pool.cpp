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

#include <string.h>

#include "base_cpp/string_pool.h"

using namespace indigo;

IMPL_ERROR(StringPool, "string pool");

StringPool::StringPool()
{
}

StringPool::~StringPool()
{
}

int StringPool::_add(const char* str, int size)
{
    int idx = _pool.add();

    // Save self into to the pool to check used items
    _pool[idx] = idx;

    if (idx >= _storage.size())
        _storage.resize(idx + 1);
    if (size == -1 && str == 0)
        throw Error("Internal error: size == -1 && str == 0");

    if (size == -1)
        size = strlen(str);
    _storage.at(idx).resize(size + 1);
    if (str != 0 && size != 0)
        memcpy(at(idx), str, size);
    at(idx)[size] = 0;
    return idx;
}

int StringPool::add(const char* str)
{
    return _add(str, -1);
}

int StringPool::add(int size)
{
    return _add(0, size);
}

int StringPool::add(Array<char>& str)
{
    return _add(str.ptr(), str.size());
}

void StringPool::remove(int idx)
{
    _pool.remove(idx);
}

char* StringPool::at(int idx)
{
    return _storage[_pool[idx]].ptr();
}

const char* StringPool::at(int idx) const
{
    return _storage[_pool[idx]].ptr();
}

int StringPool::size() const
{
    return _pool.size();
}

int StringPool::begin() const
{
    return _pool.begin();
}

int StringPool::end() const
{
    return _pool.end();
}

int StringPool::next(int i) const
{
    return _pool.next(i);
}

void StringPool::clear()
{
    _pool.clear();
    // Do not clear storage to enable memory reuse
    // _storage.clear();
}
