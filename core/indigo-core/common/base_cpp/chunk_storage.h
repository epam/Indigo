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
        ChunkStorage();

        void clear();
        byte* add(int n_bytes);
        void add(const byte* data, int n_bytes);
        void add(const Array<char>& data);
        void add(const char* str);

        int count(void);

        byte* get(int i);
        int getSize(int i);

        void pop();

        DECL_ERROR;

    private:
        Array<byte> _arr;
        Array<int> _offset;
    };

} // namespace indigo

#endif // __chunk_storage_h__
