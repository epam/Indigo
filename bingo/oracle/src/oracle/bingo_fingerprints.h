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

#ifndef __bingo_fingerprints__
#define __bingo_fingerprints__

#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/ptr_array.h"
#include "base_cpp/tlscont.h"
#include "core/bingo_context.h"

using namespace indigo;

namespace indigo
{
    class OracleEnv;
    class SharedMemory;
    class OracleStatement;
    class OracleLOB;
    class OracleEnv;
} // namespace indigo
class BingoFingerprints
{
public:
    struct Block
    {
        Block() : used(0)
        {
        }

        // Number of molecules in this block (max. 65536)
        // All blocks (except maybe the last one) are full
        int used;

        Array<word> counters;
        // Mapping between original molecules order and sorted
        Array<word> mapping;
        // Low and high bounds for data in transposed fingerprint row
        Array<word> bit_starts, bit_ends;
    };

    class Screening
    {
    public:
        Screening();

        int part;
        int items_read;
        int items_passed;

        int start_offset, end_offset;

        CP_DECL;

        TL_CP_DECL(ArrayNew<int>, query_ones);
        TL_CP_DECL(List<int>, passed);
        TL_CP_DECL(ArrayNew<int>, one_counters);
        TL_CP_DECL(ArrayNew<int>, passed_pre);
        TL_CP_DECL(Array<qword>, fp_final);

        void* data;

        Obj<OracleStatement> statement;
        Obj<OracleLOB> bits_lob;
        Block* block;
        int query_bit_idx;
    };

    explicit BingoFingerprints(int context_id);

    void create(OracleEnv& env);
    void drop(OracleEnv& env);
    void truncate(OracleEnv& env);
    void analyze(OracleEnv& env);

    void addFingerprint(OracleEnv& env, const byte* fp);
    void flush(OracleEnv& env);

    void validate(OracleEnv& env);
    void validateForUpdate(OracleEnv& env);

    // fp_bytes - size of the one fingerprint
    // fp_priority_bytes - size of the range of fingerprint bits for the bigenning
    //                     that are used more frequentry than other bits.
    //                     -1 - default value.
    void init(BingoContext& context, int fp_bytes, int fp_priority_bytes_min = -1, int fp_priority_bytes_max = -1);
    void screenInit(const byte* fp, Screening& screening);
    bool ableToScreen(Screening& screening);

    bool screenPart_Init(OracleEnv& env, Screening& screening);
    bool screenPart_Next(OracleEnv& env, Screening& screening);
    void screenPart_End(OracleEnv& env, Screening& screening);

    bool countOnes_Init(OracleEnv& env, Screening& screening);
    bool countOnes_Next(OracleEnv& env, Screening& screening);
    void countOnes_End(OracleEnv& env, Screening& screening);

    int getStorageIndex(Screening& screening, int local_idx);
    int getStorageIndex_NoMap(Screening& screening, int local_idx);

    float queryOnesRatio(Screening& screening);
    int countOracleBlocks(OracleEnv& env);
    int getTotalCount(OracleEnv& env);

    DECL_ERROR;

protected:
    // constant configuration parameters
    int _fp_bytes, _fp_priority_bytes_min, _fp_priority_bytes_max;
    int _chunk_qwords;
    CP_DECL;
    TL_CP_DECL(ArrayChar, _table_name);

    // when adding fingerprints

    int _part_adding;
    Array<qword> _pending_bits, _pending_bits_2;
    Block _pending_block;

    int _total_count_cached;

    void _optimizePendingBlock();
    void _initBlock(Block& block, bool is_pending_block);
    bool _flush_Update(OracleEnv& env, bool update_counter);
    void _flush_Insert(OracleEnv& env);
    void _flush_Insert_OLD(OracleEnv& env);

    static int _cmp_optimize_counters(int a, int b, void* context);

    // when screening
    ObjArray<Block> _all_blocks;

    static int _cmp_counters(int a, int b, void* context);
    static int _cmp_int(int a, int b, void* context);
};

#endif
