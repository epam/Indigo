/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "oracle/ringo_shadow_table.h"
#include "core/ringo_index.h"
#include "molecule/elements.h"
#include "base_cpp/output.h"

RingoShadowTable::RingoShadowTable (int context_id)
{
   _table_name.push(0);

   ArrayOutput output(_table_name);

   output.printf("SHADOW_%d", context_id);
   output.writeChar(0);
}

void RingoShadowTable::addReaction (OracleEnv &env, RingoIndex &index, const char *rowid, int blockno, int offset)
{
   OracleLOB crf(env);

   crf.createTemporaryBLOB();

   crf.write(0, index.getCrf());

   OracleStatement statement(env);

   statement.append("INSERT INTO %s VALUES('%s', %d, %d, :crf)",
      _table_name.ptr(), rowid, blockno, offset);

   statement.prepare();
   statement.bindBlobByName(":crf", crf);

   statement.execute();
}

void RingoShadowTable::create (OracleEnv &env)
{
   OracleStatement statement(env);
   const char *tn = _table_name.ptr();

   statement.append("CREATE TABLE %s "
      "(rid VARCHAR2(100), blockno NUMBER, offset NUMBER, crf BLOB) NOLOGGING", tn);

   statement.prepare();
   statement.execute();

   OracleStatement::executeSingle(env, "CREATE UNIQUE INDEX %s_rid ON %s(rid)", tn, tn);
}

void RingoShadowTable::drop (OracleEnv &env)
{
   OracleStatement::executeSingle(env, "BEGIN DropTable('%s'); END;", _table_name.ptr());
}

void RingoShadowTable::truncate (OracleEnv &env)
{
   OracleStatement::executeSingle(env, "TRUNCATE TABLE %s", _table_name.ptr());
}

void RingoShadowTable::analyze (OracleEnv &env)
{
   env.dbgPrintf("analyzing shadow table\n");
   OracleStatement::executeSingle(env, "ANALYZE TABLE %s ESTIMATE STATISTICS", _table_name.ptr());
}

int RingoShadowTable::countOracleBlocks (OracleEnv &env)
{
   int res;

   if (!OracleStatement::executeSingleInt(res, env, "select blocks from user_tables where "
              "table_name = upper('%s')", _table_name.ptr()))
      return 0;

   return res;
}

const char * RingoShadowTable::getName ()
{
   return _table_name.ptr();
}

bool RingoShadowTable::getReactionLocation (OracleEnv &env, const char *rowid, int &blockno, int &offset)
{
   OracleStatement statement(env);

   statement.append("SELECT blockno, offset FROM %s WHERE rid = '%s'",
                    _table_name.ptr(), rowid);

   statement.prepare();
   statement.defineIntByPos(1, &blockno);
   statement.defineIntByPos(2, &offset);

   return statement.executeAllowNoData();
}

void RingoShadowTable::deleteReaction (OracleEnv &env, const char *rowid)
{
   OracleStatement::executeSingle(env, "DELETE FROM %s WHERE rid = '%s'",
                                   _table_name.ptr(), rowid);
}

