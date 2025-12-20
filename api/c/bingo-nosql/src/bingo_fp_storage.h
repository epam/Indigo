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

#ifndef __bingo_fp_storage__
#define __bingo_fp_storage__

#include <fstream>
#include <vector>

#include "mmf/mmf_array.h"
#include "mmf/mmf_ptr.h"

namespace bingo
{
    class TranspFpStorage
    {
    public:
        TranspFpStorage(int fp_size, int block_size, int small_base_size);

        static MMFAddress create(MMFPtr<TranspFpStorage>& ptr, int fp_size, int block_size, int small_base_size);

        static void load(MMFPtr<TranspFpStorage>& ptr, MMFAddress offset);

        void add(const byte* fp);

        int getBlockSize(void) const;

        const byte* getBlock(int idx);

        int getBlockCount() const;

        const byte* getIncrement() const;

        int getIncrementSize(void) const;

        int getIncrementCapacity(void) const;

        int getPackCount() const;

        virtual ~TranspFpStorage();

        MMFArray<int>& getFpBitUsageCounts();

    protected:
        int _fp_size;
        int _block_count;
        int _block_size;
        int _pack_count;
        bool _small_flag;
        MMFArray<MMFPtr<byte>> _storage;

        MMFPtr<byte> _inc_buffer;
        int _inc_size;
        int _inc_fp_count;
        int _small_inc_size;

        MMFArray<int> _fp_bit_usage_counts;

        void _createFpStorage(int fp_size, int inc_fp_capacity, const char* inc_filename);

        void _addIncToStorage();
    };
}; // namespace bingo

#endif /* __bingo_fp_storage__ */
