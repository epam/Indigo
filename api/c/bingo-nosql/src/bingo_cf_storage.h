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

#ifndef __cf_storage__
#define __cf_storage__

#include <fstream>
#include <string>
#include <vector>

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/tlscont.h"

#include "mmf/mmf_address.h"
#include "mmf/mmf_array.h"
#include "mmf/mmf_ptr.h"

namespace bingo
{
    class ByteBufferStorage
    {
    public:
        ByteBufferStorage(int block_size);

        static MMFAddress create(MMFPtr<ByteBufferStorage>& cf_ptr, int block_size);

        static void load(MMFPtr<ByteBufferStorage>& cf_ptr, MMFAddress offset);

        const byte* get(int idx, int& len);
        void add(const byte* data, int len, int idx);
        void remove(int idx);
        ~ByteBufferStorage();

    private:
        struct _Addr
        {
            unsigned long block_idx;
            unsigned long offset;
            long len;
        };

        int _block_size;
        int _free_pos;
        MMFArray<MMFPtr<byte>> _blocks;
        MMFArray<_Addr> _addresses;
    };
}; // namespace bingo

#endif /* __cf_storage__ */
