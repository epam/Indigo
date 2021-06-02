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

#ifndef __bingo_storage__
#define __bingo_storage__

#include "base_cpp/array.h"
#include "base_cpp/ptr_array.h"
#include "base_cpp/shmem.h"

using namespace indigo;

namespace indigo
{
    class OracleEnv;
    class OracleLOB;
    class SharedMemory;
} // namespace indigo

class BingoStorage
{
public:
    explicit BingoStorage(OracleEnv& env, int context_id);
    virtual ~BingoStorage();

    void validateForInsert(OracleEnv& env);
    void add(OracleEnv& env, const ArrayChar& data, int& blockno, int& offset);

    void create(OracleEnv& env);
    void drop(OracleEnv& env);
    void truncate(OracleEnv& env);
    void validate(OracleEnv& env);

    void flush(OracleEnv& env);
    void finish(OracleEnv& env);

    void lock(OracleEnv& env);
    void markRemoved(OracleEnv& env, int blockno, int offset);

    int count();
    void get(int n, ArrayChar& out);

    DECL_ERROR;

protected:
    enum
    {
        _STATE_EMPTY = 0,
        _STATE_BULDING = 1,
        _STATE_LOADING = 2,
        _STATE_READY = 3
    };

    enum
    {
        _MAX_BLOCK_SIZE = 5 * 1024 * 1024
    };

    struct _Block
    {
        int size;
    };

    struct _State
    {
        int state; // see STATE_***
        int age;
        int age_loaded;
    };

    struct _Addr
    {
        short blockno;
        short length;
        int offset;
    };

    SharedMemory* _shmem_state;
    Array<_Block> _blocks;
    int _n_added;

    int _age_loaded;

    PtrArray<SharedMemory> _shmem_array;

    void* _getShared(SharedMemory*& sh_mem, char* name, int shared_size, bool allow_first);
    _State* _getState(bool allow_first);
    void _insertLOB(OracleEnv& env, int no);
    OracleLOB* _getLob(OracleEnv& env, int no);
    void _finishTopLob(OracleEnv& env);
    void _finishIndexLob(OracleEnv& env);

    ArrayChar _shmem_id;
    ArrayChar _table_name;

    Array<_Addr> _index;
    ArrayChar _top_lob_pending_data;
    ArrayChar _index_lob_pending_data;
    int _top_lob_pending_mark;
    int _index_lob_pending_mark;
};

#endif
