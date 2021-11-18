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

#include "oracle/ringo_fast_index.h"

#include "base_cpp/profiling.h"
#include "core/ringo_matchers.h"
#include "graph/embedding_enumerator.h"
#include "molecule/elements.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_pi_systems_matcher.h"
#include "molecule/molfile_loader.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/ringo_fetch_context.h"
#include "oracle/ringo_oracle.h"
#include "oracle/rowid_loader.h"
#include "reaction/rxnfile_loader.h"

IMPL_ERROR(RingoFastIndex, "ringo fast fetch");

RingoFastIndex::RingoFastIndex(RingoFetchContext& context) : _context(context)
{
    _fetch_type = 0;
    _last_id = -1;
}

RingoFastIndex::~RingoFastIndex()
{
}

void RingoFastIndex::_decompressRowid(const Array<char>& stored, OraRowidText& rid)
{
    BufferScanner scanner(stored.ptr() + 2, stored[1]);

    RowIDLoader loader(_context.context().context().rid_dict, scanner);
    QS_DEF(Array<char>, rowid);

    loader.loadRowID(rowid);

    if (rowid.size() != 18)
        throw Error("rowid size=%d?", rowid.size());

    memcpy(rid.ptr(), rowid.ptr(), 18);
    rid.ptr()[18] = 0;
}

void RingoFastIndex::_match(OracleEnv& env, int idx)
{
    _last_id = idx;

    BingoStorage& storage = this->_context.context().context().storage;
    QS_DEF(Array<char>, stored);

    storage.get(idx, stored);

    if (stored[0] != 0)
        return; // reaction was removed from index

    BufferScanner scanner(stored);

    scanner.skip(1);                  // skip the deletion mark
    scanner.skip(scanner.readByte()); // skip the compessed rowid

    profTimerStart(tall, "match");
    bool res = false;

    TRY_READ_TARGET_RXN
    {
        res = _context.substructure.matchBinary(scanner);
    }
    CATCH_READ_TARGET_RXN(res = false)

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

void RingoFastIndex::fetch(OracleEnv& env, int max_matches)
{
    env.dbgPrintf("requested %d hits\n", max_matches);
    matched.clear();

    BingoFingerprints& fingerprints = _context.context().fingerprints;

    if (_fetch_type == _SUBSTRUCTURE)
    {
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
                        ;
                    fingerprints.screenPart_End(env, _screening);
                    _unmatched += _screening.block->used - _screening.passed.size();
                }
                else
                {
                    env.dbgPrintfTS("screening ended\n");
                    break;
                }

                _screening.items_passed += _screening.passed.size();

                env.dbgPrintfTS("%d reactions passed screening\n", _screening.passed.size());
            }
        }
        else
        {
            while (matched.size() < max_matches && _cur_idx < _context.context().context().storage.count())
                _match(env, _cur_idx++);

            env.dbgPrintfTS("%d reactions matched\n", matched.size());
        }
    }
    else
        throw Error("unexpected fetch type: %d", _fetch_type);
}

void RingoFastIndex::prepareSubstructure(OracleEnv& env)
{
    env.dbgPrintf("preparing fastindex for reaction substructure search\n");

    _context.context().context().storage.validate(env);
    _context.context().fingerprints.validate(env);
    _context.context().fingerprints.screenInit(_context.substructure.getQueryFingerprint(), _screening);
    _fetch_type = _SUBSTRUCTURE;
    _cur_idx = 0;
    _matched = 0;
    _unmatched = 0;
}

double RingoFastIndex::calcSelectivity(OracleEnv& env, int total_count)
{
    if (_matched + _unmatched == 0)
        throw Error("calcSelectivity() called before fetch()");

    BingoFingerprints& fingerprints = _context.context().fingerprints;

    if (fingerprints.ableToScreen(_screening))
    {
        return (double)_matched * _screening.items_passed / (_screening.items_read * (_matched + _unmatched));
    }
    else
    {
        if (_matched == 0)
            return 0;
        return (double)_matched / (_matched + _unmatched);
    }
}

int RingoFastIndex::getIOCost(OracleEnv& env, double selectivity)
{
    BingoFingerprints& fingerprints = _context.context().fingerprints;

    int blocks = fingerprints.countOracleBlocks(env);
    double ratio = fingerprints.queryOnesRatio(_screening);

    return (int)(blocks * ratio);
}

bool RingoFastIndex::getLastRowid(OraRowidText& id)
{
    if (_last_id < 0)
        return false;

    BingoStorage& storage = this->_context.context().context().storage;
    QS_DEF(Array<char>, stored);

    storage.get(_last_id, stored);
    _decompressRowid(stored, id);
    return true;
}

int RingoFastIndex::getTotalCount(OracleEnv& env)
{
    return _context.context().fingerprints.getTotalCount(env);
}

bool RingoFastIndex::end()
{
    return false;
}
