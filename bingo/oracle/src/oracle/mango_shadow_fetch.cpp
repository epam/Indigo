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

#include "base_cpp/output.h"
#include "core/mango_matchers.h"
#include "oracle/mango_shadow_fetch.h"

#include "base_cpp/profiling.h"
#include "molecule/elements.h"
#include "molecule/icm_loader.h"
#include "molecule/molecule_auto_loader.h"
#include "molecule/molfile_loader.h"
#include "molecule/smiles_loader.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/mango_fetch_context.h"

IMPL_ERROR(MangoShadowFetch, "mango shadow fetch");

MangoShadowFetch::MangoShadowFetch(MangoFetchContext& context) : _context(context)
{
    _total_count = -1;

    StringOutput output(_table_name);
    output.printf(context.context().shadow_table.getName());

    StringOutput output2(_components_table_name);
    output2.printf(context.context().shadow_table.getComponentsName());

    _executed = false;
    _fetch_type = 0;
    _processed_rows = 0;
    _end = false;

    _rowid.ptr()[0] = 0;
}

MangoShadowFetch::~MangoShadowFetch()
{
}

int MangoShadowFetch::getTotalCount(OracleEnv& env)
{
    if (_total_count < 0)
    {
        if (!OracleStatement::executeSingleInt(_total_count, env, "SELECT COUNT(*) FROM %s", _table_name.c_str()))
            throw Error("getTotalCount() failed");
    }

    return _total_count;
}

bool MangoShadowFetch::getLastRowid(OraRowidText& id)
{
    if (_rowid.ptr()[0] == 0)
        return false;
    memcpy(&id, &_rowid, sizeof(_rowid));
    return true;
}

int MangoShadowFetch::countOracleBlocks(OracleEnv& env)
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

int MangoShadowFetch::getIOCost(OracleEnv& env, float selectivity)
{
    return (int)(countOracleBlocks(env) * selectivity);
}

bool MangoShadowFetch::end()
{
    return _end;
}

float MangoShadowFetch::calcSelectivity(OracleEnv& env, int total_count)
{
    int nrows_select_total;

    if (_counting_select.size())
    {
        OracleStatement statement(env);

        statement.append("%s", _counting_select.c_str());
        statement.prepare();
        statement.defineIntByPos(1, &nrows_select_total);
        if (_fetch_type == _MASS)
        {
            statement.bindFloatByName(":mass_min", &_context.mass.bottom);
            statement.bindFloatByName(":mass_max", &_context.mass.top);
        }
        else if (_fetch_type == _TAUTOMER)
        {
            const char* gross = _context.tautomer.getQueryGross();
            statement.bindStringByName(":gross", gross, strlen(gross) + 1);
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

void MangoShadowFetch::prepareNonSubstructure(OracleEnv& env)
{
    env.dbgPrintf("preparing shadow table for non-substructure match\n");

    _fetch_type = _NON_SUBSTRUCTURE;
    _need_xyz = _context.substructure.needCoords();

    _env.reset(new OracleEnv(env.ctx(), env.logger()));
    _statement.reset(new OracleStatement(_env.ref()));

    if (_need_xyz)
    {
        _lob_cmf.reset(new OracleLOB(_env.ref()));
        _lob_xyz.reset(new OracleLOB(_env.ref()));
        _statement->append("SELECT mol_rowid, cmf, xyz FROM %s", _table_name.c_str());
        _statement->prepare();
        _statement->defineStringByPos(1, _rowid.ptr(), sizeof(_rowid));
        _statement->defineBlobByPos(2, _lob_cmf.ref());
        _statement->defineBlobByPos(3, _lob_xyz.ref());
    }
    else
    {
        _lob_cmf.reset(new OracleLOB(_env.ref()));
        _statement->append("SELECT mol_rowid, cmf FROM %s", _table_name.c_str());
        _statement->prepare();
        _statement->defineStringByPos(1, _rowid.ptr(), sizeof(_rowid));
        _statement->defineBlobByPos(2, _lob_cmf.ref());
    }

    _counting_select.clear();
}

void MangoShadowFetch::prepareNonTautomerSubstructure(OracleEnv& env)
{
    env.dbgPrintf("preparing shadow table for non-tautomer-substructure match\n");
    _fetch_type = _NON_TAUTOMER_SUBSTRUCTURE;

    _env.reset(new OracleEnv(env.ctx(), env.logger()));
    _statement.reset(new OracleStatement(_env.ref()));
    _lob_cmf.reset(new OracleLOB(_env.ref()));

    _statement->append("SELECT mol_rowid, cmf FROM %s", _table_name.c_str());
    _statement->prepare();
    _statement->defineStringByPos(1, _rowid.ptr(), sizeof(_rowid));
    _statement->defineBlobByPos(2, _lob_cmf.ref());

    _counting_select.clear();
}

void MangoShadowFetch::prepareTautomer(OracleEnv& env, int right_part)
{
    if (right_part == 1)
        env.dbgPrintf("preparing shadow table for tautomer match\n");
    else
        env.dbgPrintf("preparing shadow table for non-tautomer match\n");

    _fetch_type = _TAUTOMER;
    _right_part = right_part;

    _env.reset(new OracleEnv(env.ctx(), env.logger()));
    _statement.reset(new OracleStatement(_env.ref()));
    _lob_cmf.reset(new OracleLOB(_env.ref()));

    _statement->append("SELECT mol_rowid, cmf FROM %s", _table_name.c_str());

    if (right_part == 1)
        _statement->append(" WHERE gross = :gross OR gross LIKE :grossh");

    _statement->prepare();
    _statement->defineStringByPos(1, _rowid.ptr(), sizeof(_rowid));
    _statement->defineBlobByPos(2, _lob_cmf.ref());

    if (right_part == 1)
    {
        StringOutput output(_counting_select);

        output.printf("SELECT COUNT(*) FROM %s WHERE gross = :gross", _table_name.c_str());
    }
    else
        _counting_select.clear();
}

void MangoShadowFetch::prepareExact(OracleEnv& env, int right_part)
{
    const MangoExact& instance = _context.exact;

    if (right_part == 1)
        env.dbgPrintf("preparing shadow table for exact\n");
    else
        env.dbgPrintf("preparing shadow table for non-exact\n");

    _fetch_type = _EXACT;
    _right_part = right_part;
    _need_xyz = instance.needCoords();

    _env.reset(new OracleEnv(env.ctx(), env.logger()));
    _statement.reset(new OracleStatement(_env.ref()));
    _lob_cmf.reset(new OracleLOB(_env.ref()));

    _statement->append("SELECT sh.mol_rowid, sh.cmf");
    if (_need_xyz)
        _statement->append(", sh.xyz", _table_name.c_str());
    _statement->append(" FROM %s sh", _table_name.c_str());

    QS_DEF(std::string, table_copies);
    QS_DEF(std::string, where_clause);
    _prepareExactQueryStrings(table_copies, where_clause);

    _statement->append(table_copies.c_str());
    _statement->append(where_clause.c_str());

    _statement->prepare();
    _statement->defineStringByPos(1, _rowid.ptr(), sizeof(_rowid));
    _statement->defineBlobByPos(2, _lob_cmf.ref());

    if (_need_xyz)
    {
        _lob_xyz.reset(new OracleLOB(_env.ref()));
        _statement->defineBlobByPos(3, _lob_xyz.ref());
    }

    StringOutput output_cnt(_counting_select);
    output_cnt.printf("SELECT COUNT(*) FROM %s sh", _table_name.c_str());
    output_cnt.printf("%s", table_copies.c_str());
    output_cnt.printf("%s", where_clause.c_str());
}

void MangoShadowFetch::_prepareExactQueryStrings(std::string& table_copies, std::string& where_clause)
{
    const MangoExact& instance = _context.exact;
    const MangoExact::Hash& hash = instance.getQueryHash();

    StringOutput output_tables(table_copies);
    if (_right_part == 1)
    {
        for (int i = 0; i < hash.size(); i++)
            output_tables.printf(", %s t%d", _components_table_name.c_str(), i);
    }

    // Create complex WHERE clause
    StringOutput output(where_clause);
    if (_right_part == 1)
    {
        bool where_was_added = false;
        if (hash.size() > 0)
        {

            output.printf(" WHERE ");
            where_was_added = true;
            // molecule ids must be same
            output.printf("sh.mol_rowid = t0.mol_rowid and ");
            for (int i = 1; i < hash.size(); i++)
                output.printf("t%d.mol_rowid = t%d.mol_rowid and ", i - 1, i);
            // query components must match target components
            for (int i = 0; i < hash.size(); i++)
                output.printf("t%d.hash = '%08X' and ", i, hash[i].hash);
            // components count mast must target components count
            const char* rel;
            if (instance.needComponentMatching())
                rel = ">=";
            else
                rel = "=";

            for (int i = 0; i < hash.size(); i++)
            {
                if (i != 0)
                    output.printf("and ");
                output.printf("t%d.count %s %d ", i, rel, hash[i].count);
            }
        }
        if (!instance.needComponentMatching())
        {
            if (!where_was_added)
                output.printf(" WHERE ");
            else
                output.printf("and ");

            // There must be no other components in target
            int query_fragments_count = 0;
            for (int i = 0; i < hash.size(); i++)
                query_fragments_count += hash[i].count;
            output.printf("sh.fragments = %d", query_fragments_count);
        }
    }

    output.writeChar(0);
}

void MangoShadowFetch::prepareGross(OracleEnv& env, int right_part)
{
    MangoGross& instance = _context.gross;

    env.dbgPrintf("preparing shadow table for gross formula match\n");

    _fetch_type = _GROSS;
    _right_part = right_part;
    _env.reset(new OracleEnv(env.ctx(), env.logger()));
    _statement.reset(new OracleStatement(_env.ref()));
    _statement->append("SELECT mol_rowid, gross FROM %s ", _table_name.c_str());
    if (*instance.getConditions() != 0 && right_part == 1)
        _statement->append("WHERE %s", instance.getConditions());
    _statement->prepare();
    _statement->defineStringByPos(1, _rowid.ptr(), sizeof(_rowid));
    _statement->defineStringByPos(2, _gross, sizeof(_gross));

    StringOutput output(_counting_select);
    output.printf("SELECT COUNT(*) FROM %s WHERE %s", _table_name.c_str(), instance.getConditions());
}

void MangoShadowFetch::prepareMass(OracleEnv& env)
{
    env.dbgPrintf("preparing shadow table for molecular mass match\n");

    QS_DEF(std::string, where);

    {
        StringOutput where_out(where);
        where_out.printf("");
        where_out.writeChar(0);
    }

    _fetch_type = _MASS;
    _env.reset(new OracleEnv(env.ctx(), env.logger()));
    _statement.reset(new OracleStatement(_env.ref()));

    _statement->append("SELECT mol_rowid FROM %s WHERE mass >= :mass_min AND mass <= :mass_max", _table_name.c_str());

    _statement->prepare();
    _statement->defineStringByPos(1, _rowid.ptr(), sizeof(_rowid));

    StringOutput output(_counting_select);
    output.printf("SELECT COUNT(*) FROM %s WHERE WHERE mass >= :mass_min AND mass <= :mass_max", _table_name.c_str());
}

void MangoShadowFetch::fetch(OracleEnv& env, int maxrows)
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

        if (_fetch_type == _MASS)
        {
            _statement->bindFloatByName(":mass_min", &_context.mass.bottom);
            _statement->bindFloatByName(":mass_max", &_context.mass.top);
        }
        else if (_fetch_type == _TAUTOMER && _right_part == 1)
        {
            const char* gross = _context.tautomer.getQueryGross();
            _statement->bindStringByName(":gross", gross, strlen(gross) + 1);
            QS_DEF(std::string, grossh);
            grossh = gross;
            grossh += " H%";
            _statement->bindStringByName(":grossh", grossh.data(), grossh.size());
        }

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

        TRY_READ_TARGET_MOL
        {

            if (_fetch_type == _NON_SUBSTRUCTURE)
            {
                MangoSubstructure& instance = _context.substructure;
                QS_DEF(std::string, cmf);

                _lob_cmf->readAll(cmf);

                if (_need_xyz)
                {
                    if (_statement->gotNull(3)) // xyz == NULL?
                        have_match = true;
                    else
                    {
                        QS_DEF(std::string, xyz);

                        _lob_xyz->readAll(xyz);
                        if (!instance.matchBinary(cmf, &xyz))
                            have_match = true;
                    }
                }
                else if (!instance.matchBinary(cmf, 0))
                    have_match = true;
            }
            else if (_fetch_type == _NON_TAUTOMER_SUBSTRUCTURE)
            {
                MangoTautomer& instance = _context.tautomer;
                QS_DEF(std::string, cmf);

                _lob_cmf->readAll(cmf);

                if (!instance.matchBinary(cmf))
                    have_match = true;
            }
            else if (_fetch_type == _TAUTOMER)
            {
                MangoTautomer& instance = _context.tautomer;
                QS_DEF(std::string, cmf);

                _lob_cmf->readAll(cmf);

                if (instance.matchBinary(cmf) == (_right_part == 1))
                    have_match = true;
            }
            else if (_fetch_type == _EXACT)
            {
                MangoExact& instance = _context.exact;
                QS_DEF(std::string, cmf);

                profTimerStart(tlobread, "exact.lobread");
                _lob_cmf->readAll(cmf);
                profTimerStop(tlobread);

                if (_need_xyz)
                {
                    if (_statement->gotNull(3)) // xyz == NULL?
                        have_match = (_right_part == 0);
                    else
                    {
                        QS_DEF(std::string, xyz);

                        profTimerStart(txyzlobread, "exact.xyzlobread");
                        _lob_xyz->readAll(xyz);
                        profTimerStop(txyzlobread);

                        profTimerStart(tmatch, "exact.match");
                        if (instance.matchBinary(cmf, &xyz) == (_right_part == 1))
                            have_match = true;
                    }
                }
                else
                {
                    profTimerStart(tmatch, "exact.match");
                    if (instance.matchBinary(cmf, 0) == (_right_part == 1))
                        have_match = true;
                }
            }
            else if (_fetch_type == _GROSS)
            {
                MangoGross& instance = _context.gross;

                if (instance.checkGross(_gross) == (_right_part == 1))
                    have_match = true;
            }
            else if (_fetch_type == _MASS)
            {
                have_match = true;
            }
            else
                throw Error("unexpected fetch type %d", _fetch_type);
        }
        CATCH_READ_TARGET_MOL(have_match = false)

        if (have_match)
            matched.add(_rowid);
        _processed_rows++;
    }

    env.dbgPrintf("fetched %d\n", matched.size());

    return;
}
