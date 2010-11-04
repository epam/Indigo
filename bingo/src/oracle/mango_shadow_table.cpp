/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#include "oracle/mango_shadow_table.h"

#include "base_cpp/profiling.h"
#include "molecule/elements.h"
#include "core/mango_index.h"

MangoShadowTable::MangoShadowTable (int context_id)
{
   _table_name.push(0);

   ArrayOutput output(_table_name);

   output.printf("SHADOW_%d", context_id);
   output.writeChar(0);

   ArrayOutput output2(_components_table_name);
   output2.printf("HASHES_%d", context_id);
   output2.writeChar(0);

   ArrayOutput output3(_fp_table_name);
   output3.printf("SIMFP_%d", context_id);
   output3.writeChar(0);

   _main_table_statement_count = 0;
   _components_table_statement_count = 0;
}

MangoShadowTable::~MangoShadowTable ()
{
}

void MangoShadowTable::addMolecule (OracleEnv &env, const char *rowid, 
                                    int blockno, int offset,
                                    const char *data_cmf, int len_cmf,
                                    const char *data_xyz, int len_xyz,
                                    const MangoExact::Hash &hash,
                                    const char *gross,
                                    const char *counters,
                                    float molecular_mass,
                                    const char *fp_sim)
{
   if (_main_table_statement_count > 5)
      _flushMain(env);

   if (_main_table_statement.get() == 0)
   {
      _main_table_statement.create(env);
      _main_table_statement_count = 0;
      _main_table_statement->append("INSERT ALL /*+ NOLOGGING */\n");
   }
   PendingLOB &cmf = _pending_lobs.add(new PendingLOB(env));
   PendingLOB &xyz = _pending_lobs.add(new PendingLOB(env));

   cmf.lob.createTemporaryBLOB();
   cmf.lob.write(0, data_cmf, len_cmf);

   if (len_xyz > 0)
   {
      xyz.lob.createTemporaryBLOB();
      xyz.lob.write(0, data_xyz, len_xyz);
   }

   int fragments_count = 0;
   for (int i = 0; i < hash.size(); i++)
      fragments_count += hash[i].count;

   if (len_xyz == 0)
      strncpy(xyz.name, "NULL", NELEM(xyz.name));
   else
      snprintf(xyz.name, NELEM(xyz.name), ":xyz_%d", _main_table_statement_count);

   snprintf(cmf.name, NELEM(cmf.name), ":cmf_%d", _main_table_statement_count);

   _main_table_statement->append(
      // "INSERT /*+ NOLOGGING */ "
      "INTO %s VALUES('%s', %d, %d, '%s', %s, %s, %f, %d%s)\n",
      _table_name.ptr(), rowid, blockno, offset, gross, cmf.name, xyz.name, 
      molecular_mass, fragments_count, counters);

   _main_table_statement_count++;
   
   // Insert into components shadow table
   if (_components_table_statement_count > 20)
      _flushComponents(env);

   if (_components_table_statement.get() == 0)
   {
      _components_table_statement.create(env);
      _components_table_statement->append("INSERT ALL /*+ NOLOGGING */\n");
      _components_table_statement_count = 0;
   }

   for (int i = 0; i < hash.size(); i++)
   {
      _components_table_statement->append(
      // OracleStatement::executeSingle(env, */
      //   "INSERT /*+ NOLOGGING */  "
         "INTO %s VALUES('%s', '%08X', %d)\n", _components_table_name.ptr(),
         rowid, hash[i].hash, hash[i].count);
      _components_table_statement_count++;
   }

   /*
   profTimerStart(tsim, "moleculeIndex.register_shadow_fpsim");
   OracleStatement::executeSingle(env, "INSERT INTO %s VALUES('%s', '%s')",
                                  _fp_table_name.ptr(), rowid, fp_sim);
   profTimerStop(tsim);
   */
}

void MangoShadowTable::flush (OracleEnv &env)
{
   _flushMain(env);
   _flushComponents(env);
}

void MangoShadowTable::_flushMain (OracleEnv &env)
{
   // Flusing data to the main table
   if (_main_table_statement.get() != 0)
   {
      if (_main_table_statement_count != 0)
      {
         _main_table_statement->append("SELECT * FROM dual");

         profTimerStart(tmain, "moleculeIndex.register_shadow_main");
         _main_table_statement->prepare();
         for (int i = 0; i < _pending_lobs.size(); i++)
         {
            PendingLOB *plob = _pending_lobs[i];
            if (strlen(plob->name) > 0 && strcmp(plob->name, "NULL") != 0)
               _main_table_statement->bindBlobByName(plob->name, plob->lob);
         }
         _main_table_statement->execute();
         profTimerStop(tmain);

         _pending_lobs.clear();
      }
      _main_table_statement.free();
      _main_table_statement_count = 0;
   }
}

void MangoShadowTable::_flushComponents (OracleEnv &env)
{
   // Flusing components table
   if (_components_table_statement.get() != 0)
   {
      if (_components_table_statement_count != 0)
      {
         _components_table_statement->append("SELECT * FROM dual");

         profTimerStart(tcomp, "moleculeIndex.register_shadow_comp");
         _components_table_statement->prepare();
         _components_table_statement->execute();
         profTimerStop(tcomp);
      }
      _components_table_statement.free();  
      _components_table_statement_count = 0;
   }
}


void MangoShadowTable::addMolecule (OracleEnv &env, const MangoIndex &index, 
                                    const char *rowid, int blockno, int offset)
{
   addMolecule(env, rowid, blockno, offset,
               index.getCmf().ptr(), index.getCmf().size(),
               index.getXyz().ptr(), index.getXyz().size(),
               index.getHash(), index.getGrossString(), 
               index.getCountedElementsString(),
               index.getMolecularMass(),
               index.getFingerprint_Sim_Str());
}

void MangoShadowTable::create (OracleEnv &env)
{
   OracleStatement s1(env);
   const char *mi = _table_name.ptr();
   int i;

   s1.append("CREATE TABLE %s "
      "(mol_rowid VARCHAR2(18), blockno NUMBER, offset NUMBER, "
      " gross VARCHAR2(500), cmf BLOB, xyz BLOB, MASS number, fragments number ", mi);

   for (i = 0; i < (int)NELEM(MangoIndex::counted_elements); i++)
      s1.append(", cnt_%s INTEGER", Element::toString(MangoIndex::counted_elements[i]));
   s1.append(") NOLOGGING");
   s1.append(" LOB(cmf) STORE AS (NOCACHE NOLOGGING PCTVERSION 0)"
             " LOB(xyz) STORE AS (NOCACHE NOLOGGING PCTVERSION 0)");

   s1.prepare();
   s1.execute();

   // Create shadow table for molecule components
   const char *cmi = _components_table_name.ptr();
   OracleStatement::executeSingle(env, "CREATE TABLE %s "
      " (mol_rowid VARCHAR2(18), hash VARCHAR2(8), count INT) NOLOGGING", cmi);

   // Create table for similarity fingerprints
   OracleStatement::executeSingle(env, "CREATE TABLE %s "
      " (mol_rowid VARCHAR2(18), fingerprint VARCHAR2(128)) NOLOGGING",
           _fp_table_name.ptr());

}

void MangoShadowTable::createIndices (OracleEnv &env)
{
   const char *mi = _table_name.ptr();

   OracleStatement::executeSingle(env, "CREATE UNIQUE INDEX %s_rid ON %s(mol_rowid) NOLOGGING", mi, mi);
   OracleStatement::executeSingle(env, "CREATE INDEX %s_gross ON %s(gross) NOLOGGING", mi, mi);
   OracleStatement::executeSingle(env, "CREATE INDEX %s_mass ON %s(mass) NOLOGGING", mi, mi);

   if (NELEM(MangoIndex::counted_elements) > 0)
   {
      OracleStatement s2(env);
      int i;

      s2.append("CREATE INDEX %s_CNT ON %s(cnt_%s", mi, mi,
         Element::toString(MangoIndex::counted_elements[0]));
      for (i = 1; i < NELEM(MangoIndex::counted_elements); i++)
         s2.append(", cnt_%s", Element::toString(MangoIndex::counted_elements[i]));
      s2.append(") NOLOGGING");
      s2.prepare();
      s2.execute();
   }

   const char *cmi = _components_table_name.ptr();

   OracleStatement::executeSingle(env, "CREATE INDEX %s_rid ON %s(mol_rowid) NOLOGGING", cmi, cmi);
   OracleStatement::executeSingle(env, "CREATE INDEX %s_hash ON %s(hash) NOLOGGING", cmi, cmi);
   OracleStatement::executeSingle(env, "CREATE INDEX %s_count ON %s(count) NOLOGGING", cmi, cmi);

   const char *fp = _fp_table_name.ptr();

   OracleStatement::executeSingle(env, "CREATE INDEX %s_rid ON %s(mol_rowid) NOLOGGING", fp, fp);
}

void MangoShadowTable::drop (OracleEnv &env)
{
   OracleStatement::executeSingle(env, "BEGIN DropTable('%s'); "
      "DropTable('%s'); DropTable('%s'); END;",
           _table_name.ptr(), _components_table_name.ptr(), _fp_table_name.ptr());
}

void MangoShadowTable::truncate (OracleEnv &env)
{
   OracleStatement::executeSingle(env, "TRUNCATE TABLE %s", _table_name.ptr());
   OracleStatement::executeSingle(env, "TRUNCATE TABLE %s", _components_table_name.ptr());
   OracleStatement::executeSingle(env, "TRUNCATE TABLE %s", _fp_table_name.ptr());
}

void MangoShadowTable::analyze (OracleEnv &env)
{
   env.dbgPrintf("analyzing shadow table\n");
   OracleStatement::executeSingle(env, "ANALYZE TABLE %s ESTIMATE STATISTICS", 
      _table_name.ptr());
   OracleStatement::executeSingle(env, "ANALYZE TABLE %s ESTIMATE STATISTICS", 
      _components_table_name.ptr());
}

bool MangoShadowTable::getXyz (OracleEnv &env, const char *rowid, Array<char> &xyz)
{
   if (!OracleStatement::executeSingleBlob(xyz, env,
      "SELECT xyz FROM %s where mol_rowid='%s'", _table_name.ptr(), rowid))
      return false;
   return true;
}

const char * MangoShadowTable::getName ()
{
   return _table_name.ptr();
}

const char * MangoShadowTable::getComponentsName ()
{
   return _components_table_name.ptr();
}

const char * MangoShadowTable::getSimFPName ()
{
   return _fp_table_name.ptr();
}

bool MangoShadowTable::getMoleculeLocation (OracleEnv &env, const char *rowid, int &blockno, int &offset)
{
   OracleStatement statement(env);

   statement.append("SELECT blockno, offset FROM %s WHERE mol_rowid = '%s'",
                    _table_name.ptr(), rowid);

   statement.prepare();
   statement.defineIntByPos(1, &blockno);
   statement.defineIntByPos(2, &offset);

   return statement.executeAllowNoData();
}

void MangoShadowTable::deleteMolecule (OracleEnv &env, const char *rowid)
{
   OracleStatement::executeSingle(env, "DELETE FROM %s WHERE mol_rowid = '%s'",
                                   _table_name.ptr(), rowid);
   OracleStatement::executeSingle(env, "DELETE FROM %s WHERE mol_rowid = '%s'",
                                   _components_table_name.ptr(), rowid);
   OracleStatement::executeSingle(env, "DELETE FROM %s WHERE mol_rowid = '%s'",
                                   _fp_table_name.ptr(), rowid);
}
