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

#include "oracle/ringo_shadow_fetch.h"
#include "base_cpp/output.h"
#include "core/ringo_matchers.h"
#include "graph/embedding_enumerator.h"
#include "molecule/elements.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molecule_pi_systems_matcher.h"
#include "molecule/molfile_loader.h"
#include "oracle/ringo_fetch_context.h"
#include "oracle/ringo_oracle.h"
#include "reaction/rxnfile_loader.h"

IMPL_ERROR(RingoShadowFetch, "ringo shadow fetch");

RingoShadowFetch::RingoShadowFetch(RingoFetchContext& context) : _context(context)
{
    _total_count = -1;

    StringOutput output(_table_name);

    output.printf("SHADOW_%d", context.context_id);

    _executed = false;
    _fetch_type = 0;
    _processed_rows = 0;
    _end = false;

    _rowid.ptr()[0] = 0;
}

RingoShadowFetch::~RingoShadowFetch()
{
}

bool RingoShadowFetch::getLastRowid(OraRowidText& id)
{
    if (_rowid.ptr()[0] == 0)
        return false;
    memcpy(&id, &_rowid, sizeof(_rowid));
    return true;
}

int RingoShadowFetch::getTotalCount(OracleEnv& env)
{
    if (_total_count < 0)
    {
        if (!OracleStatement::executeSingleInt(_total_count, env, "SELECT COUNT(*) FROM %s", _table_name.c_str()))
            throw Error("getTotalCount() error");
    }

    return _total_count;
}

int RingoShadowFetch::countOracleBlocks(OracleEnv& env)
{
    int res;

    env.dbgPrintf("countOracleBlocks\n");

    if (!OracleStatement::executeSingleInt(res, env,
                                           "select blocks from user_tables where "
                                           "table_name = upper('%s')",
                                           _table_name.c_str()))
        return 0;

    return res;
}

int RingoShadowFetch::getIOCost(OracleEnv& env, float selectivity)
{
    return (int)(countOracleBlocks(env) * selectivity);
}

bool RingoShadowFetch::end()
{
    return _end;
}

float RingoShadowFetch::calcSelectivity(OracleEnv& env, int total_count)
{
    int nrows_select_total;

    if (_counting_select.size() > 0)
    {
        OracleStatement statement(env);

        statement.append("%s", _counting_select.c_str());
        statement.prepare();
        statement.defineIntByPos(1, &nrows_select_total);
        if (_fetch_type == _EXACT && _right_part == 1)
        {
            const char* hash = _context.exact.getQueryHashStr();
            statement.bindStringByName(":hash", hash, strlen(hash) + 1);
        }

        if (!statement.executeAllowNoData())
            throw Error("selectivity: cannot count rows");
    }
    else
        nrows_select_total = total_count;

    if (_processed_rows == 0)
    {
        if (nrows_select_total == 0)
            return 0;
        throw Error("selectivity: no processed rows");
    }

    return (float)nrows_select_total * matched.size() / (total_count * _processed_rows);
}

void RingoShadowFetch::prepareNonSubstructure(OracleEnv& env)
{
    env.dbgPrintf("preparing shadow table for non-substructure match\n");

    _fetch_type = _NON_SUBSTRUCTURE;

    _env.reset(new OracleEnv(env.ctx(), env.logger()));
    _statement.reset(new OracleStatement(_env.ref()));

    _lob_crf.reset(new OracleLOB(_env.ref()));
    _statement->append("SELECT rid, crf FROM %s", _table_name.c_str());
    _statement->prepare();
    _statement->defineStringByPos(1, _rowid.ptr(), sizeof(_rowid));
    _statement->defineBlobByPos(2, _lob_crf.ref());

    _counting_select.clear();
}

void RingoShadowFetch::prepareExact(OracleEnv& env, int right_part)
{
    RingoExact& instance = _context.exact;

    if (right_part == 1)
        env.dbgPrintf("preparing shadow table for exact\n");
    else
        env.dbgPrintf("preparing shadow table for non-exact\n");

    _fetch_type = _EXACT;
    _right_part = right_part;

    _env.reset(new OracleEnv(env.ctx(), env.logger()));
    _statement.reset(new OracleStatement(_env.ref()));
    _lob_crf.reset(new OracleLOB(_env.ref()));

    _statement->append("SELECT sh.rid, sh.crf FROM %s sh", _table_name.c_str());

    if (right_part == 1)
        _statement->append(" WHERE hash = :hash");

    _statement->prepare();
    _statement->defineStringByPos(1, _rowid.ptr(), sizeof(_rowid));
    _statement->defineBlobByPos(2, _lob_crf.ref());
    if (_right_part == 1)
    {
        const char* hash_str = instance.getQueryHashStr();
        _statement->bindStringByName(":hash", hash_str, strlen(hash_str) + 1);
    }

    StringOutput output_cnt(_counting_select);
    output_cnt.printf("SELECT COUNT(*) FROM %s sh", _table_name.c_str());
    if (right_part == 1)
        output_cnt.printf(" WHERE hash = :hash");
}

void RingoShadowFetch::fetch(OracleEnv& env, int maxrows)
{
    matched.clear();

    if (_statement.get() == 0)
        return;

    if (maxrows < 1 || _end)
        return;

    env.dbgPrintf("fetching up to %d rows using shadowtable... ", maxrows);

    while (matched.size() < maxrows)
    {
        bool fetch_res;

        if (!_executed)
        {
            fetch_res = _statement->executeAllowNoData();
            _executed = true;
        }
        else
            fetch_res = _statement->fetch();

        if (!fetch_res)
        {
            _end = true;
            break;
        }

        bool have_match = false;

        TRY_READ_TARGET_RXN
        {

            if (_fetch_type == _NON_SUBSTRUCTURE)
            {
                RingoSubstructure& instance = _context.substructure;
                QS_DEF(std::string, crf);

                _lob_crf->readAll(crf, false);

                if (!instance.matchBinary(crf))
                    have_match = true;
            }
            else if (_fetch_type == _EXACT)
            {
                RingoExact& instance = _context.exact;
                QS_DEF(std::string, crf);

                _lob_crf->readAll(crf, false);

                have_match = (instance.matchBinary(crf) == (_right_part == 1));
            }
            else
                throw Error("unexpected fetch type %d", _fetch_type);
        }
        CATCH_READ_TARGET_RXN(have_match = false)

        if (have_match)
            matched.add(_rowid);
        _processed_rows++;
    }

    env.dbgPrintf("fetched %d\n", matched.size());

    return;
}
