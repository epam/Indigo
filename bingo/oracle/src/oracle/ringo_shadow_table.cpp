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

#include "oracle/ringo_shadow_table.h"
#include "base_cpp/output.h"
#include "core/ringo_index.h"
#include "molecule/elements.h"

IMPL_ERROR(RingoShadowTable, "ringo shadow table");

RingoShadowTable::RingoShadowTable(int context_id)
{
    StringOutput output(_table_name);
    output.printf("SHADOW_%d", context_id);
}

void RingoShadowTable::addReaction(OracleEnv& env, RingoIndex& index, const char* rowid, int blockno, int offset)
{
    OracleLOB crf(env);

    crf.createTemporaryBLOB();

    crf.write(0, index.getCrf());

    OracleStatement statement(env);

    statement.append("INSERT INTO %s VALUES(:rxnrowid, :blockno, :offset, :crf, :rxnhash)", _table_name.c_str());

    statement.prepare();
    statement.bindStringByName(":rxnrowid", rowid, strlen(rowid) + 1);
    statement.bindIntByName(":blockno", &blockno);
    statement.bindIntByName(":offset", &offset);
    statement.bindBlobByName(":crf", crf);
    statement.bindStringByName(":rxnhash", index.getHashStr(), strlen(index.getHashStr()) + 1);

    statement.execute();
}

void RingoShadowTable::create(OracleEnv& env)
{
    OracleStatement statement(env);
    const char* tn = _table_name.c_str();

    statement.append("CREATE TABLE %s "
                     "(rid VARCHAR2(18), blockno NUMBER, offset NUMBER, crf BLOB, hash VARCHAR2(8)) NOLOGGING",
                     tn);

    statement.prepare();
    statement.execute();

    OracleStatement::executeSingle(env, "CREATE UNIQUE INDEX %s_rid ON %s(rid)", tn, tn);
}

void RingoShadowTable::drop(OracleEnv& env)
{
    OracleStatement::executeSingle(env, "BEGIN DropTable('%s'); END;", _table_name.c_str());
}

void RingoShadowTable::truncate(OracleEnv& env)
{
    OracleStatement::executeSingle(env, "TRUNCATE TABLE %s", _table_name.c_str());
}

void RingoShadowTable::analyze(OracleEnv& env)
{
    env.dbgPrintf("analyzing shadow table\n");
    OracleStatement::executeSingle(env, "ANALYZE TABLE %s ESTIMATE STATISTICS", _table_name.c_str());
}

int RingoShadowTable::countOracleBlocks(OracleEnv& env)
{
    int res;

    if (!OracleStatement::executeSingleInt(res, env,
                                           "select blocks from user_tables where "
                                           "table_name = upper('%s')",
                                           _table_name.c_str()))
        return 0;

    return res;
}

const char* RingoShadowTable::getName()
{
    return _table_name.c_str();
}

bool RingoShadowTable::getReactionLocation(OracleEnv& env, const char* rowid, int& blockno, int& offset)
{
    OracleStatement statement(env);

    statement.append("SELECT blockno, offset FROM %s WHERE rid = :rid", _table_name.c_str());
    statement.prepare();
    statement.bindStringByName(":rid", rowid, strlen(rowid));
    statement.defineIntByPos(1, &blockno);
    statement.defineIntByPos(2, &offset);

    return statement.executeAllowNoData();
}

void RingoShadowTable::deleteReaction(OracleEnv& env, const char* rowid)
{
    OracleStatement::executeSingle_BindString(env, ":rid", rowid, "DELETE FROM %s WHERE rid = :rid", _table_name.c_str());
}
