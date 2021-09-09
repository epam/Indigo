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

#include "oracle/bingo_storage.h"
#include <memory>
#include "base_cpp/output.h"
#include "base_cpp/shmem.h"
#include "base_cpp/tlscont.h"
#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"

IMPL_ERROR(BingoStorage, "storage");

BingoStorage::BingoStorage(OracleEnv& env, int context_id)
{
    _shmem_state = 0;
    _age_loaded = -1;
    _top_lob_pending_mark = 0;
    _index_lob_pending_mark = 0;

    QS_DEF(Array<char>, instance);
    QS_DEF(Array<char>, schema);

    OracleStatement::executeSingleString(instance, env, "SELECT PROPERTY_VALUE from database_properties where property_name = 'GLOBAL_DB_NAME'");
    OracleStatement::executeSingleString(schema, env, "SELECT SYS_CONTEXT('USERENV', 'CURRENT_SCHEMA') from dual");

    ArrayOutput output1(_shmem_id);

    output1.printf("%s#%s#%d#bs2", instance.ptr(), schema.ptr(), context_id);
    output1.writeChar(0);

    ArrayOutput output2(_table_name);

    output2.printf("STORAGE_%d", context_id);
    output2.writeChar(0);
}

BingoStorage::~BingoStorage()
{
    delete _shmem_state;
}

void BingoStorage::create(OracleEnv& env)
{
    const char* tn = _table_name.ptr();
    OracleStatement::executeSingle(env,
                                   "CREATE TABLE %s(id number, bindata BLOB) "
                                   "NOLOGGING lob(bindata) store as (CACHE READS NOLOGGING PCTVERSION 0)",
                                   tn);
    OracleStatement::executeSingle(env, "CREATE INDEX %s_id ON %s(id)", tn, tn);
    OracleStatement::executeSingle(env, "INSERT INTO %s VALUES(0, EMPTY_BLOB())", tn);
}

void BingoStorage::drop(OracleEnv& env)
{
    OracleStatement::executeSingle(env, "BEGIN DropTable('%s'); END;", _table_name.ptr());
    delete _shmem_state;
    _shmem_state = 0;
    _age_loaded = -1;
}

void BingoStorage::truncate(OracleEnv& env)
{
    const char* tn = _table_name.ptr();

    OracleStatement::executeSingle(env, "TRUNCATE TABLE %s", tn);
    OracleStatement::executeSingle(env, "INSERT INTO %s VALUES(0, EMPTY_BLOB())", tn);
    delete _shmem_state;
    _shmem_state = 0;
    _age_loaded = -1;
}

void BingoStorage::validateForInsert(OracleEnv& env)
{
    _blocks.clear();

    OracleStatement statement(env);

    int id, length;
    OracleLOB lob(env);

    statement.append("SELECT id, length(bindata), bindata FROM %s ORDER BY id", _table_name.ptr());

    statement.prepare();
    statement.defineIntByPos(1, &id);
    statement.defineIntByPos(2, &length);
    statement.defineBlobByPos(3, lob);
    statement.execute();

    _n_added = -1;
    do
    {
        if (id == 0)
        {
            if ((length % sizeof(_Addr)) != 0)
                throw Error("unexpected LOB size %d is not multiple of %d", length, sizeof(_Addr));
            _n_added = length / sizeof(_Addr);
            continue;
        }

        _Block& block = _blocks.push();

        block.size = length;
    } while (statement.fetch());

    if (_n_added < 0)
        throw Error("missing index LOB");

    if (_blocks.size() > 0)
        _top_lob_pending_mark = _blocks.top().size;

    _index_lob_pending_mark = _n_added * sizeof(_Addr);
}

void BingoStorage::validate(OracleEnv& env)
{
    env.dbgPrintfTS("validating storage... ");

    if (_shmem_state != 0 && strcmp(_shmem_state->getID(), _shmem_id.ptr()) != 0)
    {
        delete _shmem_state;
        _shmem_state = 0;
        _age_loaded = -1;
    }

    _State* state = _getState(true);

    // TODO: implement a semaphore
    while (state->state == _STATE_LOADING)
    {
        delete _shmem_state;
        _shmem_state = 0;
        _age_loaded = -1;

        state = _getState(true);

        if (state == 0)
            throw Error("can't get shared info");

        env.dbgPrintf(".");
    }

    if (state->state == _STATE_READY)
    {
        if (state->age_loaded == state->age)
        {
            if (_age_loaded == state->age)
            {
                env.dbgPrintf("up to date\n");
                return;
            }
            else
                env.dbgPrintf("loaded by the other process\n");
        }
        else
        {
            env.dbgPrintf("has changed, reloading\n");
            state->state = _STATE_LOADING;
        }
    }
    else
    {
        state->state = _STATE_LOADING;
        env.dbgPrintf("loading ... \n");
    }

    _shmem_array.clear();
    _blocks.clear();

    OracleStatement statement(env);

    int id, length;
    OracleLOB lob(env);
    QS_DEF(Array<char>, block_name);

    statement.append("SELECT id, length(bindata), bindata FROM %s ORDER BY id", _table_name.ptr());

    statement.prepare();
    statement.defineIntByPos(1, &id);
    statement.defineIntByPos(2, &length);
    statement.defineBlobByPos(3, lob);
    statement.execute();

    do
    {
        ArrayOutput output(block_name);
        output.printf("%s_%d_%d", _shmem_id.ptr(), id, state->age);
        output.writeByte(0);

        if (length < 1)
        {
            if (id == 0)
            {
                _index.clear();
                break;
            }
            throw Error("cannot validate block #%d: length=%d", id, length);
        }

        _shmem_array.add(new SharedMemory(block_name.ptr(), length, state->state == _STATE_READY));

        void* ptr = _shmem_array.top()->ptr();

        if (ptr == 0)
        {
            if (state->state == _STATE_READY)
            {
                // That's rare case, but possible.
                // Reload the storage.
                env.dbgPrintf("shared memory is gone, resetting... \n");
                state->state = _STATE_EMPTY;
                validate(env);
                return;
            }
            else
                throw Error("can't map block #%d", id);
        }

        if (state->state != _STATE_READY)
            lob.read(0, (char*)ptr, length);

        if (id == 0)
        {
            if ((length % sizeof(_Addr)) != 0)
                throw Error("LOB size %d (expected a multiple of %d)", length, sizeof(_Addr));
            if (length > 0)
                _index.copy((_Addr*)_shmem_array[0]->ptr(), length / sizeof(_Addr));
        }

        _Block& block = _blocks.push();

        block.size = length;
    } while (statement.fetch());

    state->state = _STATE_READY;
    state->age_loaded = state->age;
    _age_loaded = state->age;
}

OracleLOB* BingoStorage::_getLob(OracleEnv& env, int no)
{
    OracleStatement statement(env);
    std::unique_ptr<OracleLOB> lob = std::make_unique<OracleLOB>(env);

    statement.append("SELECT bindata FROM %s where ID = :id FOR UPDATE", _table_name.ptr());
    statement.prepare();
    statement.bindIntByName(":id", &no);
    statement.defineBlobByPos(1, *lob);
    statement.execute();

    if (statement.fetch())
        env.dbgPrintf("WARNING: more than 1 row have id = %d in table %s\n", no, _table_name.ptr());

    lob->enableBuffering();
    return lob.release();
}

void BingoStorage::_finishTopLob(OracleEnv& env)
{
    OracleLOB* top_lob = _getLob(env, _blocks.size());
    env.dbgPrintf("flushing storage LOB\n");
    top_lob->write(_top_lob_pending_mark, _top_lob_pending_data);
    _top_lob_pending_mark += _top_lob_pending_data.size();
    _top_lob_pending_data.clear();
    delete top_lob;
    top_lob = 0;
}

void BingoStorage::_finishIndexLob(OracleEnv& env)
{
    env.dbgPrintf("flushing index LOB\n");
    OracleLOB* index_lob = _getLob(env, 0);
    index_lob->write(_index_lob_pending_mark, _index_lob_pending_data);
    _index_lob_pending_mark += _index_lob_pending_data.size();
    _index_lob_pending_data.clear();
    delete index_lob;
}

void BingoStorage::_insertLOB(OracleEnv& env, int no)
{
    _finishTopLob(env);

    OracleStatement statement(env);

    statement.append("INSERT INTO %s VALUES(%d, EMPTY_BLOB())", _table_name.ptr(), no);
    statement.prepare();
    statement.execute();

    if (no > 0)
    {
        _Block& block = _blocks.push();

        block.size = 0;
    }

    _top_lob_pending_mark = 0;
}

void BingoStorage::add(OracleEnv& env, const Array<char>& data, int& blockno, int& offset)
{
    if (_blocks.size() < 1 || _blocks.top().size + data.size() > _MAX_BLOCK_SIZE)
        _insertLOB(env, _blocks.size() + 1);

    _Block& top = _blocks.top();

    blockno = _blocks.size() - 1;
    offset = top.size;

    _top_lob_pending_data.concat(data);

    _Addr addr;

    addr.blockno = _blocks.size() - 1;
    addr.length = data.size();
    addr.offset = top.size;
    top.size += data.size();

    _index_lob_pending_data.concat((char*)&addr, sizeof(_Addr));

    _n_added++;

    _State* state = _getState(true);

    if (state != 0)
        state->age++;
}

int BingoStorage::count()
{
    return _index.size();
}

void BingoStorage::get(int n, Array<char>& out)
{
    const _Addr& addr = _index[n];

    const char* ptr = (const char*)_shmem_array[addr.blockno + 1]->ptr();

    out.copy(ptr + addr.offset, addr.length);
}

BingoStorage::_State* BingoStorage::_getState(bool allow_first)
{
    return (_State*)_getShared(_shmem_state, _shmem_id.ptr(), sizeof(_State), allow_first);
}

void* BingoStorage::_getShared(SharedMemory*& sh_mem, char* name, int shared_size, bool allow_first)
{
    if (sh_mem != 0 && strcmp(sh_mem->getID(), name) != 0)
    {
        delete sh_mem;
        sh_mem = 0;
    }

    if (sh_mem == 0)
        sh_mem = new SharedMemory(name, shared_size, !allow_first);

    if (sh_mem->ptr() == 0)
    {
        delete sh_mem;
        sh_mem = 0;
        return 0;
    }

    return sh_mem->ptr();
}

void BingoStorage::flush(OracleEnv& env)
{
    finish(env);
    OracleStatement::executeSingle(env, "COMMIT");
}

void BingoStorage::finish(OracleEnv& env)
{
    _finishTopLob(env);
    _finishIndexLob(env);
}

void BingoStorage::lock(OracleEnv& env)
{
    OracleStatement::executeSingle(env, "LOCK TABLE %s IN EXCLUSIVE MODE", _table_name.ptr());
}

void BingoStorage::markRemoved(OracleEnv& env, int blockno, int offset)
{
    OracleStatement statement(env);
    OracleLOB lob(env);

    statement.append("SELECT bindata FROM %s WHERE id = :id FOR UPDATE", _table_name.ptr());
    statement.prepare();
    statement.bindIntByName(":id", &blockno);
    statement.defineBlobByPos(1, lob);
    statement.execute();

    byte mark = 1;

    lob.write(offset, (char*)&mark, 1);

    _State* state = _getState(true);

    if (state != 0)
        state->age++;
}
