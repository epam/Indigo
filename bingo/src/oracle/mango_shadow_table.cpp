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
      _main_table_statement->append("INSERT /*+ NOLOGGING */ INTO %s\n", _table_name.ptr());
   }
   
   _PendingLOB &cmf = _pending_lobs.push(env, "cmf", _main_table_statement_count);
   _PendingLOB &xyz = _pending_lobs.push(env, len_xyz == 0 ? 0 : "xyz", _main_table_statement_count);
   _PendingInt &p_blockno = _pending_ints.push(blockno, "blockno", _main_table_statement_count);
   _PendingInt &p_offset = _pending_ints.push(offset, "offset", _main_table_statement_count);
   _PendingFloat &p_mass = _pending_floats.push(molecular_mass, "mass", _main_table_statement_count);
   _PendingString &p_rowid = _pending_strings.push(rowid, "rowid", _main_table_statement_count);
   _PendingString &p_gross = _pending_strings.push(gross, "gross", _main_table_statement_count);

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

   if (_main_table_statement_count > 0)
      _main_table_statement->append("UNION ALL ");
   _main_table_statement->append(
      "SELECT %s, %s, %s, %s, %s, %s, %s, %d%s FROM DUAL\n",
      p_rowid.name, p_blockno.name, p_offset.name, p_gross.name,
           cmf.name, xyz.name, p_mass.name, fragments_count, counters);

   _main_table_statement_count++;

   // Insert into components shadow table
   if (_components_table_statement_count > 20)
      _flushComponents(env);

   if (_components_table_statement.get() == 0)
   {
      _components_table_statement.create(env);
      _components_table_statement->append("INSERT /*+ NOLOGGING */ INTO %s\n", _components_table_name.ptr());
      _components_table_statement_count = 0;
   }

   QS_DEF(Array<char>, hash_hex);

   for (int i = 0; i < hash.size(); i++)
   {
      ArrayOutput out(hash_hex);
      out.printf("%08X", hash[i].hash);
      hash_hex.push(0);
      const char *hash_hex_ptr = hash_hex.ptr();

      if (_components_table_statement_count > 0)
         _components_table_statement->append("UNION ALL ");

      _PendingString &rid_p = _pending_strings_comp.push(rowid, "rid", _components_table_statement_count);
      _PendingString &hash_p = _pending_strings_comp.push(hash_hex_ptr, "hash", _components_table_statement_count);
      _PendingInt &count_p = _pending_ints_comp.push(hash[i].count, "count", _components_table_statement_count);
      _components_table_statement->append("SELECT %s, %s, %s FROM DUAL\n", rid_p.name, hash_p.name, count_p.name);
      _components_table_statement_count++;
   }
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
         int i;

         profTimerStart(tmain, "moleculeIndex.register_shadow_main");
         _main_table_statement->prepare();
         for (i = 0; i < _pending_lobs.size(); i++)
         {
            _PendingLOB &plob = _pending_lobs[i];
            if (strcmp(plob.name, "NULL") != 0)
               _main_table_statement->bindBlobByName(plob.name, plob.lob);
         }
         for (i = 0; i < _pending_ints.size(); i++)
         {
            _PendingInt &pint = _pending_ints[i];
            _main_table_statement->bindIntByName(pint.name, &pint.value);
         }
         for (i = 0; i < _pending_floats.size(); i++)
         {
            _PendingFloat &pfloat = _pending_floats[i];
            _main_table_statement->bindFloatByName(pfloat.name, &pfloat.value);
         }
         for (i = 0; i < _pending_strings.size(); i++)
         {
            _PendingString &pstring = _pending_strings[i];
            _main_table_statement->bindStringByName(pstring.name, pstring.value.ptr(), pstring.value.size());
         }

         _main_table_statement->execute();
         profTimerStop(tmain);

         _pending_lobs.clear();
         _pending_ints.clear();
         _pending_floats.clear();
         _pending_strings.clear();
      }
      _main_table_statement.free();
      _main_table_statement_count = 0;
   }
}

void MangoShadowTable::_flushComponents (OracleEnv &env)
{
   int i;
   
   // Flusing components table
   if (_components_table_statement.get() != 0)
   {
      if (_components_table_statement_count != 0)
      {
         profTimerStart(tcomp, "moleculeIndex.register_shadow_comp");
         _components_table_statement->prepare();
         for (i = 0; i < _pending_ints_comp.size(); i++)
         {
            _PendingInt &pint = _pending_ints_comp[i];
            _components_table_statement->bindIntByName(pint.name, &pint.value);
         }
         for (i = 0; i < _pending_strings_comp.size(); i++)
         {
            _PendingString &pstring = _pending_strings_comp[i];
            _components_table_statement->bindStringByName(pstring.name, pstring.value.ptr(), pstring.value.size());
         }
         _components_table_statement->execute();
         _pending_ints_comp.clear();
         _pending_strings_comp.clear();
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
      " gross VARCHAR2(500), cmf BLOB, xyz BLOB, MASS number, fragments NUMBER", mi);

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
}

void MangoShadowTable::drop (OracleEnv &env)
{
   OracleStatement::executeSingle(env, "BEGIN DropTable('%s'); DropTable('%s'); END;",
           _table_name.ptr(), _components_table_name.ptr());
}

void MangoShadowTable::truncate (OracleEnv &env)
{
   OracleStatement::executeSingle(env, "TRUNCATE TABLE %s", _table_name.ptr());
   OracleStatement::executeSingle(env, "TRUNCATE TABLE %s", _components_table_name.ptr());
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

bool MangoShadowTable::getMoleculeLocation (OracleEnv &env, const char *rowid, int &blockno, int &offset)
{
   OracleStatement statement(env);

   statement.append("SELECT blockno, offset FROM %s WHERE mol_rowid = :rid", _table_name.ptr());
   statement.prepare();
   statement.bindStringByName(":rid", rowid, strlen(rowid) + 1);
   statement.defineIntByPos(1, &blockno);
   statement.defineIntByPos(2, &offset);

   return statement.executeAllowNoData();
}

void MangoShadowTable::deleteMolecule (OracleEnv &env, const char *rowid)
{
   OracleStatement::executeSingle_BindString(env, ":rid", rowid,
           "DELETE FROM %s WHERE mol_rowid = :rid", _table_name.ptr());
   OracleStatement::executeSingle_BindString(env, ":rid", rowid,
           "DELETE FROM %s WHERE mol_rowid = :rid", _components_table_name.ptr());
}

MangoShadowTable::_PendingValue::_PendingValue (const char *basename, int number)
{
   if (basename == 0)
      strncpy(name, "NULL", sizeof(name));
   else
      snprintf(name, NELEM(name), ":%s_%d", basename, number);
}

MangoShadowTable::_PendingLOB::_PendingLOB (OracleEnv &env, const char *basename, int number) :
_PendingValue(basename, number),
lob(env)
{
}

MangoShadowTable::_PendingInt::_PendingInt (int val, const char *basename, int number) :
_PendingValue(basename, number)
{
   value = val;
}

MangoShadowTable::_PendingFloat::_PendingFloat (float val, const char *basename, int number) :
_PendingValue(basename, number)
{
   value = val;
}

MangoShadowTable::_PendingString::_PendingString (const char *val, const char *basename, int number) :
_PendingValue(basename, number)
{
   value.readString(val, true);
}
