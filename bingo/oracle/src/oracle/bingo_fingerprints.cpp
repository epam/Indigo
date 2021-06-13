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

#include "oracle/ora_wrap.h"

#include "base_c/bitarray.h"
#include "base_c/nano.h"
#include "base_cpp/output.h"
#include "base_cpp/profiling.h"
#include "base_cpp/scanner.h"
#include "bingo_storage.h"
#include "core/bingo_context.h"
#include "oracle/bingo_fingerprints.h"
#include "oracle/ora_wrap.h"

IMPL_ERROR(BingoFingerprints, "screening");

CP_DEF(BingoFingerprints);

BingoFingerprints::BingoFingerprints(int context_id) : CP_INIT, TL_CP_GET(_table_name)
{
    ArrayOutput output(_table_name);

    output.printf("FINGERPRINTS_%d", context_id);
    output.writeChar(0);

    _part_adding = -1;
    _total_count_cached = -1;
}

void BingoFingerprints::init(BingoContext& context, int fp_bytes, int fp_priority_bytes_min, int fp_priority_bytes_max)
{
    _fp_bytes = fp_bytes;
    if (fp_priority_bytes_min != -1)
    {
        _fp_priority_bytes_min = fp_priority_bytes_min;
        _fp_priority_bytes_max = fp_priority_bytes_max;
    }
    else
    {
        _fp_priority_bytes_min = 0;
        _fp_priority_bytes_max = fp_bytes;
    }

    _chunk_qwords = context.fp_chunk_qwords;
}

void BingoFingerprints::create(OracleEnv& env)
{
    const char* tn = _table_name.ptr();
    const char* securefile = "";

    if (env.serverMajorVersion() >= 11)
        securefile = "securefile";

    OracleStatement::executeSingle(env,
                                   "CREATE TABLE %s(part number, used number, counters BLOB, mapping BLOB, "
                                   "bit_starts BLOB, bit_ends BLOB, bits BLOB) PCTFREE 0 NOLOGGING "
                                   "lob(bits) store as %s (DISABLE STORAGE IN ROW CACHE READS NOLOGGING PCTVERSION 0) "
                                   "lob(counters) store as %s (DISABLE STORAGE IN ROW CACHE READS NOLOGGING PCTVERSION 0) "
                                   "lob(bit_starts) store as %s (DISABLE STORAGE IN ROW CACHE READS NOLOGGING PCTVERSION 0) "
                                   "lob(bit_ends) store as %s (DISABLE STORAGE IN ROW CACHE READS NOLOGGING PCTVERSION 0) "
                                   "lob(mapping) store as %s (DISABLE STORAGE IN ROW CACHE READS NOLOGGING PCTVERSION 0) ",
                                   tn, securefile, securefile, securefile, securefile, securefile);
    OracleStatement::executeSingle(env, "CREATE INDEX %s_part ON %s(part)", tn, tn);
}

void BingoFingerprints::drop(OracleEnv& env)
{
    OracleStatement::executeSingle(env, "BEGIN DropTable('%s'); END;", _table_name.ptr());
}

void BingoFingerprints::truncate(OracleEnv& env)
{
    OracleStatement::executeSingle(env, "TRUNCATE TABLE %s", _table_name.ptr());
}

void BingoFingerprints::addFingerprint(OracleEnv& env, const byte* fp)
{
    if (_pending_block.used == _chunk_qwords * 64)
        flush(env);

    byte* ptr = (byte*)_pending_bits.ptr();

    for (int i = 0; i < _fp_bytes * 8; i++)
    {
        if (bitGetBit(fp, i))
        {
            bitSetBit(ptr, _pending_block.used, 1);
            if (_pending_block.counters[i] < 65535)
                _pending_block.counters[i]++;
            _pending_block.bit_ends[i] = _pending_block.used;
        }
        ptr += 8 * _chunk_qwords;
    }
    _pending_block.mapping.push(_pending_block.mapping.size());
    _pending_block.used++;
}

void BingoFingerprints::flush(OracleEnv& env)
{
    if (_pending_block.used < 1)
        return;

    env.dbgPrintfTS("flushing %d items\n", _pending_block.used);

    // Transpose bits
    env.dbgPrintf("  optimizing pending block...");
    _optimizePendingBlock();
    env.dbgPrintf("ok\n");

    if (!_flush_Update(env, true))
        _flush_Insert(env);

    _initBlock(_pending_block, true);
    _part_adding++;

    _all_blocks.clear();
}

void BingoFingerprints::_initBlock(Block& block, bool is_pending_block)
{
    block.used = 0;

    if (is_pending_block)
    {
        _pending_bits.clear_resize(_fp_bytes * _chunk_qwords * 8);
        _pending_bits.zerofill();
    }

    block.bit_starts.clear_resize(_fp_bytes * 8);
    block.bit_starts.zerofill();
    block.bit_ends.clear_resize(_fp_bytes * 8);
    block.bit_ends.zerofill();
    block.counters.clear_resize(_fp_bytes * 8);
    block.counters.zerofill();
    block.mapping.clear();
}

bool BingoFingerprints::_flush_Update(OracleEnv& env, bool update_counter)
{
    OracleStatement statement(env);
    OracleLOB lob_counters(env), lob_bits(env), lob_mapping(env), lob_bit_starts(env), lob_bit_ends(env);

    statement.append("SELECT counters, mapping, bit_starts, bit_ends, bits from %s WHERE part = :part FOR UPDATE", _table_name.ptr());
    statement.prepare();
    statement.bindIntByName(":part", &_part_adding);
    statement.defineBlobByPos(1, lob_counters);
    statement.defineBlobByPos(2, lob_mapping);
    statement.defineBlobByPos(3, lob_bit_starts);
    statement.defineBlobByPos(4, lob_bit_ends);
    statement.defineBlobByPos(5, lob_bits);

    if (!statement.executeAllowNoData())
        return false;

    lob_bits.write(0, (char*)_pending_bits.ptr(), _pending_bits.size() * 8);

    Block& block = _pending_block;
    lob_counters.write(0, (char*)block.counters.ptr(), block.counters.sizeInBytes());
    lob_mapping.write(0, (char*)block.mapping.ptr(), block.mapping.sizeInBytes());
    lob_bit_starts.write(0, (char*)block.bit_starts.ptr(), block.bit_starts.sizeInBytes());
    lob_bit_ends.write(0, (char*)block.bit_ends.ptr(), block.bit_ends.sizeInBytes());

    if (update_counter)
    {
        OracleStatement statement1(env);
        statement1.append("UPDATE %s SET used = :used WHERE part = :part", _table_name.ptr());
        statement1.prepare();
        statement1.bindIntByName(":used", &_pending_block.used);
        statement1.bindIntByName(":part", &_part_adding);
        statement1.execute();
    }
    return true;
}

void BingoFingerprints::_flush_Insert(OracleEnv& env)
{
    OracleStatement statement(env);
    statement.append("INSERT INTO %s VALUES(:part, :used, "
                     "empty_blob(), empty_blob(), empty_blob(), empty_blob(), empty_blob())",
                     _table_name.ptr());

    statement.prepare();
    statement.bindIntByName(":part", &_part_adding);
    statement.bindIntByName(":used", &_pending_block.used);
    statement.execute();

    _flush_Update(env, false);
}

void BingoFingerprints::_flush_Insert_OLD(OracleEnv& env)
{
    OracleStatement statement(env);
    OracleLOB lob_counters(env), lob_bits(env), lob_mapping(env), lob_bit_starts(env), lob_bit_ends(env);

    lob_counters.createTemporaryBLOB();
    lob_bits.createTemporaryBLOB();
    lob_mapping.createTemporaryBLOB();
    lob_bit_starts.createTemporaryBLOB();
    lob_bit_ends.createTemporaryBLOB();

    Block& block = _pending_block;
    lob_bits.write(0, (char*)_pending_bits.ptr(), _pending_bits.size() * 8);
    lob_counters.write(0, (char*)block.counters.ptr(), block.counters.sizeInBytes());
    lob_mapping.write(0, (char*)block.mapping.ptr(), block.mapping.sizeInBytes());
    lob_bit_starts.write(0, (char*)block.bit_starts.ptr(), block.bit_starts.sizeInBytes());
    lob_bit_ends.write(0, (char*)block.bit_ends.ptr(), block.bit_ends.sizeInBytes());

    statement.append("INSERT INTO %s VALUES(:part, :used, :counters, "
                     ":mapping, :bit_starts, :bit_ends, :bits)",
                     _table_name.ptr());

    statement.prepare();
    statement.bindIntByName(":used", &_pending_block.used);
    statement.bindIntByName(":part", &_part_adding);
    statement.bindBlobByName(":counters", lob_counters);
    statement.bindBlobByName(":mapping", lob_mapping);
    statement.bindBlobByName(":bit_starts", lob_bit_starts);
    statement.bindBlobByName(":bit_ends", lob_bit_ends);
    statement.bindBlobByName(":bits", lob_bits);
    statement.execute();
}

void BingoFingerprints::screenInit(const byte* fp, Screening& screening)
{
    screening.query_ones.clear();
    screening.passed.clear();

    for (int i = 0; i < _fp_bytes * 8; i++)
        if (bitGetBit(fp, i))
            screening.query_ones.push(i);

    screening.part = 0;
    screening.block = 0;
    screening.items_read = 0;
    screening.items_passed = 0;
}

bool BingoFingerprints::ableToScreen(Screening& screening)
{
    return screening.query_ones.size() > 0;
}

bool BingoFingerprints::screenPart_Init(OracleEnv& env, Screening& screening)
{
    screening.passed.clear();

    if (screening.query_ones.size() < 1)
        throw Error("no screening bits");

    int used_qwords;

    if (screening.part != _part_adding)
    {
        if (screening.part >= _all_blocks.size())
            return false;

        screening.block = &_all_blocks[screening.part];

        screening.statement.create(env);
        screening.bits_lob.create(env);
        screening.statement->append("SELECT bits FROM %s WHERE part = :part", _table_name.ptr());
        screening.statement->prepare();
        screening.statement->bindIntByName(":part", &screening.part);
        screening.statement->defineBlobByPos(1, screening.bits_lob.ref());
        screening.statement->execute();
    }
    else
        screening.block = &_pending_block;

    used_qwords = (screening.block->used + 63) / 64;
    screening.items_read += screening.block->used;

    if (used_qwords == 0)
        return false;

    screening.data = this;
    screening.query_ones.qsort(_cmp_counters, &screening);

    screening.fp_final.clear_resize(used_qwords);
    screening.fp_final.zerofill();
    screening.query_bit_idx = 0;

    return true;
}

bool BingoFingerprints::screenPart_Next(OracleEnv& env, Screening& screening)
{
    QS_DEF(ArrayNew<qword>, fp_inter); // i.e. intermediate
    int used_bytes = (screening.block->used + 7) / 8;

    if (screening.query_bit_idx >= screening.query_ones.size())
        return false;

    int used_qwords = screening.fp_final.size();

    fp_inter.clear_resize(used_qwords);
    screening.passed_pre.clear();

    int query_bit = screening.query_ones[screening.query_bit_idx];

    if (screening.query_bit_idx == 0)
    {
        profTimerStart(tread, "fingerprints.read");

        word bit_start_read = screening.block->bit_starts[query_bit];
        word bit_end_read = screening.block->bit_ends[query_bit];

        if (bit_start_read > bit_end_read)
        {
            // This bit wasn't used in fingerprints in current block
            screening.query_bit_idx++;
            screening.fp_final.zerofill();
            screening.start_offset = -1;
            screening.end_offset = -1;
            return true;
        }

        screening.start_offset = bit_start_read / 8;
        screening.end_offset = bit_end_read / 8;

        int read_bytes_count = screening.end_offset - screening.start_offset + 1;

        if (screening.part != _part_adding)
            screening.bits_lob->read(query_bit * _chunk_qwords * 8 + screening.start_offset, (char*)screening.fp_final.ptr() + screening.start_offset,
                                     read_bytes_count);
        else
            memcpy(screening.fp_final.ptr(), _pending_bits.ptr() + query_bit * _chunk_qwords, used_qwords * 8);

        profIncCounter("fingerprints.read_nbytes", read_bytes_count);
        profIncCounter("fingerprints.read_nbytes_old", used_qwords * 8);
    }
    else
    {
        profTimerStart(tread, "fingerprints.read");

        if (screening.start_offset < 0)
        {
            // The first bit was not used in fingerprints in current block
            screening.query_bit_idx++;
            return true;
        }

        // reduce range to reduce number of reading bytes
        const char* bits_final = (const char*)screening.fp_final.ptr();

        while (bits_final[screening.start_offset] == 0)
        {
            screening.start_offset++;
            if (screening.start_offset >= used_qwords * 8)
                break;
        }
        while (screening.end_offset >= 0 && bits_final[screening.end_offset] == 0)
        {
            screening.end_offset--;
        }

        int read_bytes_count = screening.end_offset - screening.start_offset + 1;

        if (read_bytes_count <= 0)
        {
            screening.fp_final.zerofill();
            screening.query_bit_idx++;
            return true;
        }
        else
        {
            if (screening.part != _part_adding)
                screening.bits_lob->read(query_bit * _chunk_qwords * 8 + screening.start_offset, (char*)fp_inter.ptr() + screening.start_offset,
                                         read_bytes_count);
            else
                memcpy(fp_inter.ptr(), _pending_bits.ptr() + query_bit * _chunk_qwords, used_qwords * 8);

            profTimerStop(tread);
            profIncCounter("fingerprints.read_nbytes", read_bytes_count);
            profIncCounter("fingerprints.read_nbytes_old", used_qwords * 8);

            bitAnd((byte*)screening.fp_final.ptr(), (byte*)fp_inter.ptr(), used_bytes);
        }
    }

    for (int i = 0; i < screening.block->used; i++)
        if (bitGetBit(screening.fp_final.ptr(), i))
            screening.passed_pre.push(i);

    screening.query_bit_idx++;

    return true;
}

void BingoFingerprints::screenPart_End(OracleEnv& env, Screening& screening)
{
    int i;

    screening.bits_lob.free();
    screening.statement.free();

    profTimerStart(tsort, "fingerprints.end");
    for (i = 0; i < screening.passed_pre.size(); i++)
        screening.passed_pre[i] = getStorageIndex(screening, screening.passed_pre[i]);

    screening.passed_pre.qsort(_cmp_int, 0);

    for (i = 0; i < screening.passed_pre.size(); i++)
        screening.passed.add(screening.passed_pre[i]);

    screening.part++;
}

int BingoFingerprints::_cmp_int(int a, int b, void* context)
{
    return a - b;
}

int BingoFingerprints::_cmp_counters(int a, int b, void* context)
{
    const Screening& screening = *(const Screening*)context;
    const BingoFingerprints& self = *(const BingoFingerprints*)screening.data;

    if (screening.part == self._part_adding)
        return self._pending_block.counters[a] - self._pending_block.counters[b];

    const Block& block = self._all_blocks[screening.part];
    return block.counters[a] - block.counters[b];
}

int BingoFingerprints::getStorageIndex(Screening& screening, int local_idx)
{
    return screening.block->mapping[local_idx] + screening.part * (_chunk_qwords * 64);
}

int BingoFingerprints::getStorageIndex_NoMap(Screening& screening, int local_idx)
{
    return local_idx + screening.part * (_chunk_qwords * 64);
}

bool BingoFingerprints::countOnes_Init(OracleEnv& env, Screening& screening)
{
    if (screening.query_ones.size() < 1)
        throw Error("no screening bits");

    if (screening.part != _part_adding)
    {
        if (screening.part >= _all_blocks.size())
            return false;

        screening.statement.create(env);
        screening.bits_lob.create(env);
        screening.statement->append("SELECT bits FROM %s WHERE part = :part", _table_name.ptr());
        screening.statement->prepare();
        screening.statement->bindIntByName(":part", &screening.part);
        screening.statement->defineBlobByPos(1, screening.bits_lob.ref());
        screening.statement->execute();

        screening.block = &_all_blocks[screening.part];
    }
    else
        screening.block = &_pending_block;

    screening.data = this;
    screening.query_ones.qsort(_cmp_counters, &screening);

    screening.one_counters.clear_resize(screening.block->used);
    screening.one_counters.zerofill();

    screening.query_bit_idx = 0;

    return true;
}

bool BingoFingerprints::countOnes_Next(OracleEnv& env, Screening& screening)
{
    int used_qwords = (screening.block->used + 63) / 64;
    int used_bytes = (screening.block->used + 7) / 8;

    QS_DEF(ArrayNew<qword>, fp);

    fp.clear_resize(used_qwords);

    if (screening.query_bit_idx >= screening.query_ones.size())
        return false;

    if (screening.part != _part_adding)
        screening.bits_lob->read(screening.query_ones[screening.query_bit_idx] * _chunk_qwords * 8, (char*)fp.ptr(), used_qwords * 8);
    else
        memcpy(fp.ptr(), _pending_bits.ptr() + screening.query_ones[screening.query_bit_idx] * _chunk_qwords, used_qwords * 8);

    for (int j = 0; j < 8 * used_bytes; j++)
    {
        if (bitGetBit(fp.ptr(), j))
            screening.one_counters[screening.block->mapping[j]]++;
    }
    screening.query_bit_idx++;
    return true;
}

void BingoFingerprints::countOnes_End(OracleEnv& env, Screening& screening)
{
    screening.bits_lob.free();
    screening.statement.free();
    screening.part++;
}

void BingoFingerprints::validateForUpdate(OracleEnv& env)
{
    OracleStatement statement(env);
    OracleLOB counters_lob(env), mapping_lob(env), bit_starts_lob(env), bit_ends_lob(env), bits_lob(env);
    int used, part = -1;

    statement.append("SELECT used, counters, mapping, bit_starts, bit_ends, part, bits FROM %s ORDER BY part DESC", _table_name.ptr());

    statement.prepare();
    statement.defineIntByPos(1, &used);
    statement.defineBlobByPos(2, counters_lob);
    statement.defineBlobByPos(3, mapping_lob);
    statement.defineBlobByPos(4, bit_starts_lob);
    statement.defineBlobByPos(5, bit_ends_lob);
    statement.defineIntByPos(6, &part);
    statement.defineBlobByPos(7, bits_lob);

    bool ex = statement.executeAllowNoData();

    if (ex && used < _chunk_qwords * 64)
    {
        if (_part_adding != part)
        {
            _part_adding = part;

            _initBlock(_pending_block, true);
            Block& block = _pending_block;

            block.used = used;

            counters_lob.read(0, (char*)block.counters.ptr(), block.counters.sizeInBytes());
            bit_starts_lob.read(0, (char*)block.bit_starts.ptr(), block.bit_starts.sizeInBytes());
            bit_ends_lob.read(0, (char*)block.bit_ends.ptr(), block.bit_ends.sizeInBytes());

            block.mapping.clear_resize(used);
            mapping_lob.read(0, (char*)block.mapping.ptr(), block.mapping.sizeInBytes());

            _pending_bits_2.clear_resize(_pending_bits.size());
            _pending_bits_2.zerofill();

            bits_lob.read(0, (char*)_pending_bits_2.ptr(), _pending_bits.size() * 8);

            int i;

            for (i = 0; i < _fp_bytes * 8; i++)
            {
                int offset = i * _chunk_qwords * 8;
                const byte* fp_row_src = (byte*)_pending_bits_2.ptr() + offset;
                byte* fp_row_dest = (byte*)_pending_bits.ptr() + offset;

                for (int j = 0; j < _pending_block.used; j++)
                {
                    int mapped_bit = _pending_block.mapping[j];
                    if (bitGetBit(fp_row_src, j))
                        bitSetBit(fp_row_dest, mapped_bit, 1);
                }
            }

            for (i = 0; i < _pending_block.mapping.size(); i++)
                _pending_block.mapping[i] = i;
        }
    }
    else
    {
        if (ex)
        {
            if (_part_adding == part + 1) // are we already inserting into pending block?
                ;
            else
                _pending_block.used = 0;
        }
        if (_pending_block.used == 0)
        {
            _part_adding = part + 1; // will start from 0 if there are no rows in the table
            _initBlock(_pending_block, true);
        }
    }
}

void BingoFingerprints::validate(OracleEnv& env)
{
    env.dbgPrintf("validating screening data... ");

    if (_all_blocks.size() > 0)
    {
        // TODO: reload data if table was updated
        env.dbgPrintf("already loaded\n");
        return;
    }

    profTimerStart(tread, "fingerprints.validate");

    OracleStatement statement(env);
    OracleLOB counters_lob(env), mapping_lob(env), bit_starts_lob(env), bit_ends_lob(env);
    int used;

    statement.append("SELECT used, counters, mapping, bit_starts, bit_ends FROM %s ORDER BY part", _table_name.ptr());
    statement.prepare();
    statement.defineIntByPos(1, &used);
    statement.defineBlobByPos(2, counters_lob);
    statement.defineBlobByPos(3, mapping_lob);
    statement.defineBlobByPos(4, bit_starts_lob);
    statement.defineBlobByPos(5, bit_ends_lob);

    if (!statement.executeAllowNoData())
    {
        env.dbgPrintf("no data?\n");
        return;
    }

    do
    {
        Block& block = _all_blocks.push();

        _initBlock(block, false);
        block.used = used;

        counters_lob.read(0, (char*)block.counters.ptr(), block.counters.sizeInBytes());
        bit_starts_lob.read(0, (char*)block.bit_starts.ptr(), block.bit_starts.sizeInBytes());
        bit_ends_lob.read(0, (char*)block.bit_ends.ptr(), block.bit_ends.sizeInBytes());

        block.mapping.clear_resize(used);
        mapping_lob.read(0, (char*)block.mapping.ptr(), block.mapping.sizeInBytes());
    } while (statement.fetch());

    env.dbgPrintf("\n");
}

void BingoFingerprints::analyze(OracleEnv& env)
{
    env.dbgPrintf("analyzing fingerprints table\n");
    OracleStatement::executeSingle(env, "ANALYZE TABLE %s ESTIMATE STATISTICS", _table_name.ptr());
}

int BingoFingerprints::countOracleBlocks(OracleEnv& env)
{
    int res;

    if (_chunk_qwords < 1)
        return 0;

    if (!OracleStatement::executeSingleInt(res, env, "select sum(length(bits)) / %d from %s", _chunk_qwords * 8, _table_name.ptr()))
        return 0;

    return res;
}

float BingoFingerprints::queryOnesRatio(Screening& screening)
{
    if (_fp_bytes == 0)
        throw Error("_fp_bytes = 0 -> division by zero");

    return (float)screening.query_ones.size() / (_fp_bytes * 8);
}

int BingoFingerprints::getTotalCount(OracleEnv& env)
{
    if (_total_count_cached >= 0)
        return _total_count_cached;

    _total_count_cached = 0;

    if (_all_blocks.size() > 0)
    {
        for (int i = 0; i < _all_blocks.size(); i++)
            _total_count_cached += _all_blocks[i].used;
    }
    else
    {
        OracleStatement::executeSingleInt(_total_count_cached, env, "SELECT sum(used) FROM %s", _table_name.ptr());
    }

    return _total_count_cached;
}

int BingoFingerprints::_cmp_optimize_counters(int a, int b, void* context)
{
    const BingoFingerprints& self = *(const BingoFingerprints*)context;

    bool a_priority = a >= self._fp_priority_bytes_min * 8 && a < self._fp_priority_bytes_max * 8;
    bool b_priority = b >= self._fp_priority_bytes_min * 8 && b < self._fp_priority_bytes_max * 8;
    if (a_priority && !b_priority)
        return -1;
    if (!a_priority && b_priority)
        return 1;

    return self._pending_block.counters[a] - self._pending_block.counters[b];
}

void BingoFingerprints::_optimizePendingBlock()
{
    profTimerStart(tall, "fingerprints.optimize");
    // Find better ordering for molecules to reduce
    // number of bytes to read during screening

    // Find bit order for better screening
    QS_DEF(ArrayNew<int>, counters_order);
    counters_order.clear_resize(_fp_bytes * 8);
    for (int i = 0; i < _fp_bytes * 8; i++)
        counters_order[i] = i;

    counters_order.qsort(_cmp_optimize_counters, this);

    // Find better mapping
    QS_DEF(ArrayNew<word>, post_mapping);
    post_mapping.clear_resize(_pending_block.used);
    for (int i = 0; i < _pending_block.used; i++)
        post_mapping[i] = i;

    int last_zero_start = 0;
    for (int i = 0; i < _fp_bytes * 8; i++)
    {
        int bit = counters_order[i];
        int offset = bit * _chunk_qwords * 8;
        const byte* fp_row = (byte*)_pending_bits.ptr() + offset;

        // Split molecules by specified bit
        int low_start = last_zero_start;
        int high_start = _pending_block.used - 1;
        while (true)
        {
            int low = low_start;
            while (low < _pending_block.used && bitGetBit(fp_row, post_mapping[low]) != 0)
                low++;

            int high = high_start;
            while (high > low && bitGetBit(fp_row, post_mapping[high]) != 1)
                high--;

            if (high <= low)
            {
                last_zero_start = low;
                break;
            }

            // swap
            int tmp = post_mapping[low];
            post_mapping[low] = post_mapping[high];
            post_mapping[high] = tmp;

            low_start = low + 1;
            high_start = high - 1;
        }
    }
    // Apply post_mapping to the existing mapping
    for (int i = 0; i < _pending_block.used; i++)
        _pending_block.mapping[i] = post_mapping[_pending_block.mapping[i]];

    // Find bit start and end positions
    _pending_block.bit_starts.fffill();
    _pending_block.bit_ends.zerofill();
    for (int i = 0; i < _fp_bytes * 8; i++)
    {
        int offset = i * _chunk_qwords * 8;
        const byte* fp_row = (byte*)_pending_bits.ptr() + offset;

        for (int j = 0; j < _pending_block.used; j++)
        {
            int mapped_bit = post_mapping[j];
            if (bitGetBit(fp_row, mapped_bit))
            {
                word& start_index = _pending_block.bit_starts[i];
                if (start_index > j)
                    start_index = j;

                word& end_index = _pending_block.bit_ends[i];
                if (end_index < j)
                    end_index = j;
            }
        }
    }

    // Reorder bits according to the found mapping
    _pending_bits_2.clear_resize(_pending_bits.size());
    _pending_bits_2.zerofill();

    for (int i = 0; i < _fp_bytes * 8; i++)
    {
        int offset = i * _chunk_qwords * 8;
        const byte* fp_row_src = (byte*)_pending_bits.ptr() + offset;
        byte* fp_row_dest = (byte*)_pending_bits_2.ptr() + offset;

        for (int j = 0; j < _pending_block.used; j++)
        {
            // int mapped_bit = _pending_block.mapping[j];
            int mapped_bit = post_mapping[j];
            if (bitGetBit(fp_row_src, mapped_bit))
                bitSetBit(fp_row_dest, j, 1);
        }
    }
    _pending_bits.swap(_pending_bits_2);
}

//
// BingoFingerprints::Screening
//

CP_DEF(BingoFingerprints::Screening);

BingoFingerprints::Screening::Screening()
    : CP_INIT, TL_CP_GET(query_ones), TL_CP_GET(passed), TL_CP_GET(one_counters), TL_CP_GET(passed_pre), TL_CP_GET(fp_final)
{
}
