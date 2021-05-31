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

#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"

#include "base_c/bitarray.h"
#include "base_cpp/profiling.h"
#include "base_cpp/scanner.h"
#include "bingo_oracle.h"
#include "core/mango_matchers.h"
#include "molecule/elements.h"
#include "molecule/icm_loader.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molfile_loader.h"
#include "molecule/smiles_loader.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/mango_fast_index.h"
#include "oracle/mango_fetch_context.h"
#include "oracle/mango_oracle.h"
#include "oracle/mango_shadow_table.h"
#include "oracle/rowid_loader.h"

IMPL_ERROR(MangoFastIndex, "mango fast fetch");

MangoFastIndex::MangoFastIndex(MangoFetchContext& context) : _context(context)
{
    _fetch_type = 0;
    _last_id = -1;
}

MangoFastIndex::~MangoFastIndex()
{
}

void MangoFastIndex::_decompressRowid(const std::string& stored, OraRowidText& rid)
{
    BufferScanner scanner(stored.c_str() + 2, stored[1]);

    RowIDLoader loader(_context.context().context().rid_dict, scanner);
    QS_DEF(std::string, rowid);

    loader.loadRowID(rowid);

    if (rowid.size() != 18)
        throw Error("rowid size=%d?", rowid.size());

    memcpy(rid.ptr(), rowid.c_str(), 18);
    rid.ptr()[18] = 0;
}

bool MangoFastIndex::getLastRowid(OraRowidText& id)
{
    if (_last_id < 0)
        return false;

    BingoStorage& storage = this->_context.context().context().storage;
    QS_DEF(std::string, stored);

    storage.get(_last_id, stored);
    _decompressRowid(stored, id);
    return true;
}

void MangoFastIndex::_match(OracleEnv& env, int idx)
{
    _last_id = idx;

    BingoStorage& storage = this->_context.context().context().storage;
    QS_DEF(std::string, stored);

    storage.get(idx, stored);

    if (stored[0] != 0)
        return; // molecule was removed from index

    BufferScanner scanner(stored);

    scanner.skip(1);                  // skip the deletion mark
    scanner.skip(scanner.readByte()); // skip the compessed rowid
    scanner.skip(2);                  // skip 'ord' bits count

    bool res = false;

    profTimerStart(tall, "match");

    TRY_READ_TARGET_MOL
    {

        if (_fetch_type == _SUBSTRUCTURE)
        {
            QS_DEF(std::string, xyz_buf);

            if (_context.substructure.needCoords())
            {
                OraRowidText rid;

                _decompressRowid(stored, rid);
                if (_loadCoords(env, rid.ptr(), xyz_buf))
                {
                    BufferScanner xyz_scanner(xyz_buf);

                    res = _context.substructure.matchBinary(scanner, &xyz_scanner);
                }
                else
                    // no XYZ --> skip the molecule
                    res = false;
            }
            else
                res = _context.substructure.matchBinary(scanner, 0);
        }
        else if (_fetch_type == _TAUTOMER_SUBSTRUCTURE)
            res = _context.tautomer.matchBinary(scanner);
        else // _fetch_type == _SIMILARITY
            res = _context.similarity.matchBinary(scanner);
    }
    CATCH_READ_TARGET_MOL(res = false)

    profTimerStop(tall);

    if (res)
    {
        OraRowidText& rid = matched.at(matched.add());

        _decompressRowid(stored, rid);

        profIncTimer("match.found", profTimerGetTime(tall));
        _matched++;
    }
    else
    {
        profIncTimer("match.not_found", profTimerGetTime(tall));
        _unmatched++;
    }
}

void MangoFastIndex::fetch(OracleEnv& env, int max_matches)
{
    env.dbgPrintf("requested %d hits\n", max_matches);
    matched.clear();

    if (_fetch_type == _SUBSTRUCTURE || _fetch_type == _TAUTOMER_SUBSTRUCTURE)
        _fetchSubstructure(env, max_matches);
    else if (_fetch_type == _SIMILARITY)
        _fetchSimilarity(env, max_matches);
    else
        throw Error("unexpected fetch type: %d", _fetch_type);
}

void MangoFastIndex::_fetchSubstructure(OracleEnv& env, int max_matches)
{
    BingoFingerprints& fingerprints = _context.context().fingerprints;

    if (fingerprints.ableToScreen(_screening))
    {
        while (matched.size() < max_matches)
        {
            if (_screening.passed.size() > 0)
            {
                int idx = _screening.passed.begin();
                _match(env, _screening.passed.at(idx));
                _screening.passed.remove(idx);
                continue;
            }

            if (fingerprints.screenPart_Init(env, _screening))
            {
                while (fingerprints.screenPart_Next(env, _screening))
                {
                    if (_screening.passed_pre.size() <= _context.context().context().sub_screening_pass_mark ||
                        _screening.query_bit_idx >= _context.context().context().sub_screening_max_bits)
                    {
                        env.dbgPrintfTS("stopping at bit #%d; ", _screening.query_bit_idx);
                        break;
                    }
                }
                fingerprints.screenPart_End(env, _screening);
                _unmatched += _screening.block->used - _screening.passed.size();
            }
            else
            {
                env.dbgPrintfTS("screening ended\n");
                break;
            }

            _screening.items_passed += _screening.passed.size();
            env.dbgPrintf("%d molecules passed screening\n", _screening.passed.size());
        }
    }
    else
    {
        while (matched.size() < max_matches && _cur_idx < _context.context().context().storage.count())
            _match(env, _cur_idx++);

        env.dbgPrintfTS("%d molecules matched of tested %d\n", matched.size(), _cur_idx);
    }
}

void MangoFastIndex::_fetchSimilarity(OracleEnv& env, int max_matches)
{
    BingoFingerprints& fingerprints = _context.context().fingerprints;
    int i;

    if (!fingerprints.ableToScreen(_screening))
    {
        env.dbgPrintfTS("no bits in query fingerprint, can not do similarity search\n");
        return;
    }

    profTimerStart(tsimfetch, "sim.fetch");
    while (matched.size() < max_matches)
    {
        if (!fingerprints.countOnes_Init(env, _screening))
        {
            env.dbgPrintfTS("screening ended\n");
            break;
        }

        BingoStorage& storage = _context.context().context().storage;

        QS_DEF(Array<int>, max_common_ones);
        QS_DEF(Array<int>, min_common_ones);
        QS_DEF(Array<int>, target_ones);
        QS_DEF(std::string, stored);

        max_common_ones.clear_resize(_screening.block->used);
        min_common_ones.clear_resize(_screening.block->used);
        target_ones.clear_resize(_screening.block->used);

        for (i = 0; i < _screening.block->used; i++)
        {
            storage.get(fingerprints.getStorageIndex_NoMap(_screening, i), stored);

            BufferScanner scanner(stored);

            scanner.skip(1);                  // skip the deletion mark
            scanner.skip(scanner.readByte()); // skip the compessed rowid
            target_ones[i] = scanner.readBinaryWord();
            max_common_ones[i] = _context.similarity.getUpperBound(target_ones[i]);
            min_common_ones[i] = _context.similarity.getLowerBound(target_ones[i]);
        }

        bool first = true;
        bool entire = false;

        _screening.passed.clear();

        while (true)
        {
            if (!fingerprints.countOnes_Next(env, _screening))
            {
                env.dbgPrintf("read all %d bits, writing %d results... ", _screening.query_ones.size(), _screening.passed.size());

                entire = true;
                break;
            }

            if (first)
            {
                first = false;
                for (i = 0; i < _screening.block->used; i++)
                {
                    int min_possible_ones = _screening.one_counters[i];
                    int max_possible_ones = _screening.one_counters[i] + _screening.query_ones.size() - _screening.query_bit_idx;

                    if (min_possible_ones <= max_common_ones[i] && max_possible_ones >= min_common_ones[i])
                        _screening.passed.add(i);
                }
            }
            else
            {
                int j;

                for (j = _screening.passed.begin(); j != _screening.passed.end();)
                {
                    i = _screening.passed[j];

                    int min_possible_ones = _screening.one_counters[i];
                    int max_possible_ones = _screening.one_counters[i] + _screening.query_ones.size() - _screening.query_bit_idx;

                    int next_j = _screening.passed.next(j);

                    if (min_possible_ones > max_common_ones[i] || max_possible_ones < min_common_ones[i])
                        _screening.passed.remove(j);

                    j = next_j;
                }
            }

            if (_screening.passed.size() <= _context.context().context().sim_screening_pass_mark)
            {
                env.dbgPrintfTS("stopping reading fingerprints on bit %d/%d; have %d molecules to check...  ", _screening.query_bit_idx,
                                _screening.query_ones.size(), _screening.passed.size());
                _unmatched += _screening.block->used - _screening.passed.size();
                break;
            }
        }

        if (entire)
        {
            for (i = 0; i < _screening.block->used; i++)
            {
                if (_context.similarity.match(target_ones[i], _screening.one_counters[i]))
                {
                    OraRowidText& rid = matched.at(matched.add());

                    storage.get(fingerprints.getStorageIndex_NoMap(_screening, i), stored);
                    _decompressRowid(stored, rid);
                    _matched++;
                }
                else
                    _unmatched++;
            }
        }
        else if (_screening.passed.size() > 0)
        {
            profTimerStart(tfine, "sim.fetch.fine");
            for (i = _screening.passed.begin(); i != _screening.passed.end(); i = _screening.passed.next(i))
                _match(env, fingerprints.getStorageIndex_NoMap(_screening, _screening.passed[i]));
            profTimerStop(tfine);
        }
        env.dbgPrintf("done\n");

        fingerprints.countOnes_End(env, _screening);
    }
    profTimerStop(tsimfetch);
}

bool MangoFastIndex::_loadCoords(OracleEnv& env, const char* rowid, std::string& coords)
{
    MangoOracleContext& moc = MangoOracleContext::get(env, _context.context_id, false);

    return moc.shadow_table.getXyz(env, rowid, coords);
}

void MangoFastIndex::prepareSubstructure(OracleEnv& env)
{
    env.dbgPrintf("preparing fastindex for substructure search\n");

    _context.context().context().storage.validate(env);
    _context.context().fingerprints.validate(env);
    _context.context().fingerprints.screenInit(_context.substructure.getQueryFingerprint(), _screening);

    env.dbgPrintfTS("Have %d bits in query fingerprint\n", _screening.query_ones.size());

    _fetch_type = _SUBSTRUCTURE;
    _cur_idx = 0;
    _matched = 0;
    _unmatched = 0;
}

void MangoFastIndex::prepareSimilarity(OracleEnv& env)
{
    env.dbgPrintfTS("preparing fastindex for similarity search\n");
    _context.context().context().storage.validate(env);
    _context.context().fingerprints.validate(env);
    _context.context().fingerprints.screenInit(_context.similarity.getQueryFingerprint(), _screening);
    _fetch_type = _SIMILARITY;
    _cur_idx = 0;
    _matched = 0;
    _unmatched = 0;
}

void MangoFastIndex::prepareTautomerSubstructure(OracleEnv& env)
{
    env.dbgPrintfTS("preparing fastindex for tautomer substructure search\n");
    _context.context().context().storage.validate(env);
    _context.context().fingerprints.validate(env);
    _context.context().fingerprints.screenInit(_context.tautomer.getQueryFingerprint(), _screening);
    _fetch_type = _TAUTOMER_SUBSTRUCTURE;
    _cur_idx = 0;
    _matched = 0;
    _unmatched = 0;
}

float MangoFastIndex::calcSelectivity(OracleEnv& env, int total_count)
{
    if (_matched + _unmatched == 0)
        throw Error("calcSelectivity() called before fetch()");

    BingoFingerprints& fingerprints = _context.context().fingerprints;

    if (_fetch_type == _SUBSTRUCTURE || _fetch_type == _TAUTOMER_SUBSTRUCTURE)
    {
        if (fingerprints.ableToScreen(_screening))
        {
            return (float)_matched * _screening.items_passed / (_screening.items_read * (_matched + _unmatched));
        }
        else
        {
            if (_matched == 0)
                return 0;
            return (float)_matched / (_matched + _unmatched);
        }
    }
    else // _fetch_type == _SIMILARITY
    {
        if (_matched == 0)
            return 0;
        return (float)_matched / (_matched + _unmatched);
    }
}

int MangoFastIndex::getIOCost(OracleEnv& env, float selectivity)
{
    BingoFingerprints& fingerprints = _context.context().fingerprints;

    int blocks = fingerprints.countOracleBlocks(env);
    float ratio = fingerprints.queryOnesRatio(_screening);

    return (int)(blocks * ratio);
}

bool MangoFastIndex::end()
{
    return false;
}
