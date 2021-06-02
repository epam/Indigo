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

#include "oracle/mango_shadow_table.h"

#include "base_cpp/profiling.h"
#include "core/mango_index.h"
#include "molecule/elements.h"

IMPL_ERROR(MangoShadowTable, "shadow table");

MangoShadowTable::MangoShadowTable(int context_id)
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
    _commit_main = false;
    _commit_comp = false;
}

MangoShadowTable::~MangoShadowTable()
{
}

void MangoShadowTable::addMolecule(OracleEnv& env, const char* rowid, int blockno, int offset, const char* data_cmf, int len_cmf, const char* data_xyz,
                                   int len_xyz, const MangoExact::Hash& hash, const char* gross, const Array<int>& counters, float molecular_mass,
                                   const char* fp_sim, bool append)
{
    if (_main_table_statement_count >= 4096)
        _flushMain(env);

    int i;

    if (_main_table_statement.get() == 0)
    {
        _main_table_statement.create(env);
        _main_table_statement_count = 0;

        /*
        APPEND_VALUES hint causes Oracle internal error:
        OCI call error: ORA-00600: internal error code, arguments:
           [koklGetLocAndFlag1], [], [], [], [], [], [], [], [], [], [], []
        But it should be faster. Replace 'flase' with append when Oracle bug is fixed.
        The error appears on Oracle 11.2 Win 7 x64
        */
        _main_table_statement->append("INSERT %s INTO %s VALUES ("
                                      ":rid, :blockno, :offset, :gross, :cmf, :xyz, :mass, :fragcount",
                                      false ? "/*+ APPEND_VALUES */" : "", _table_name.ptr());

        if (append)
            _commit_main = true;

        for (i = 0; i < counters.size(); i++)
        {
            char name[10] = {0};
            snprintf(name, sizeof(name), ":counter%d", i);
            _main_table_statement->append(", %s", name);
        }
        _main_table_statement->append(")");
    }

    _pending_rid.push();
    strncpy(_pending_rid.top(), rowid, 19);
    _pending_blockno.push(blockno);
    _pending_offset.push(offset);
    _pending_gross.push();
    strncpy(_pending_gross.top(), gross, 512);
    _pending_mass.push(molecular_mass);

    _pending_cmf.push(env);
    _pending_cmf.top().assignBytes(data_cmf, len_cmf);

    _pending_xyz.push(env);
    if (len_xyz > 0)
        _pending_xyz.top().assignBytes(data_xyz, len_xyz);

    int fragments_count = 0;
    for (i = 0; i < hash.size(); i++)
        fragments_count += hash[i].count;

    _pending_fragcount.push(fragments_count);

    if (_pending_counters.size() != counters.size())
        _pending_counters.resize(counters.size());

    for (i = 0; i < counters.size(); i++)
        _pending_counters[i].push(counters[i]);
    _main_table_statement_count++;

    // Insert into components shadow table
    if (_components_table_statement_count >= 8192)
        _flushComponents(env);

    if (_components_table_statement.get() == 0)
    {
        _components_table_statement.create(env);
        _components_table_statement->append("INSERT %s INTO %s VALUES (:rid, :hash, :count)", append ? "/*+ APPEND_VALUES */" : "",
                                            _components_table_name.ptr());
        _components_table_statement_count = 0;
        if (append)
            _commit_comp = true;
    }

    for (int i = 0; i < hash.size(); i++)
    {
        _pending_comp_hash.push();
        snprintf(_pending_comp_hash.top(), 9, "%08X", hash[i].hash);
        _pending_comp_rid.push();
        strncpy(_pending_comp_rid.top(), rowid, 19);
        _pending_comp_count.push(hash[i].count);
        _components_table_statement_count++;
    }
}

void MangoShadowTable::flush(OracleEnv& env)
{
    _flushMain(env);
    _flushComponents(env);
}

void MangoShadowTable::_flushMain(OracleEnv& env)
{
    // Flusing data to the main table
    if (_main_table_statement.get() != 0)
    {
        if (_main_table_statement_count != 0)
        {
            int i;

            profTimerStart(tmain, "moleculeIndex.register_shadow_main");
            _main_table_statement->prepare();

            _main_table_statement->bindStringByName(":rid", _pending_rid[0], 19);
            _main_table_statement->bindIntByName(":blockno", _pending_blockno.ptr());
            _main_table_statement->bindIntByName(":offset", _pending_offset.ptr());
            _main_table_statement->bindStringByName(":gross", _pending_gross[0], 512);

            QS_DEF(ArrayChar, cmf);
            QS_DEF(ArrayChar, xyz);
            QS_DEF(Array<short>, xyz_ind);
            int maxallocsize_cmf = 0;
            int maxallocsize_xyz = 0;

            cmf.clear();
            for (i = 0; i < _pending_cmf.size(); i++)
            {
                int allocsize = _pending_cmf[i].getAllocSize();

                if (allocsize > maxallocsize_cmf)
                    maxallocsize_cmf = allocsize;
            }

            cmf.clear_resize((maxallocsize_cmf + 4) * _pending_cmf.size());
            cmf.zerofill();
            for (i = 0; i < _pending_cmf.size(); i++)
                memcpy(cmf.ptr() + i * (maxallocsize_cmf + 4), _pending_cmf[i].get(), _pending_cmf[i].getAllocSize() + 4);

            xyz.clear();
            xyz_ind.clear();
            for (i = 0; i < _pending_xyz.size(); i++)
            {
                if (_pending_xyz[i].get() == 0)
                    continue;

                int allocsize = _pending_xyz[i].getAllocSize();

                if (allocsize > maxallocsize_xyz)
                    maxallocsize_xyz = allocsize;
            }

            if (maxallocsize_xyz == 0)
                maxallocsize_xyz = 8; // or we get ORA-01459

            xyz.clear_resize((maxallocsize_xyz + 4) * _pending_xyz.size());
            xyz.zerofill();
            for (i = 0; i < _pending_xyz.size(); i++)
            {
                if (_pending_xyz[i].get() != 0)
                {
                    memcpy(xyz.ptr() + i * (maxallocsize_xyz + 4), _pending_xyz[i].get(), _pending_xyz[i].getAllocSize() + 4);
                    xyz_ind.push(0); // OCI_IND_NOTNULL
                }
                else
                    xyz_ind.push(-1); // OCI_IND_NULL
            }

            _main_table_statement->bindRawPtrByName(":cmf", (OCIRaw*)cmf.ptr(), maxallocsize_cmf, 0);
            _main_table_statement->bindRawPtrByName(":xyz", (OCIRaw*)xyz.ptr(), maxallocsize_xyz, xyz_ind.ptr());
            _main_table_statement->bindFloatByName(":mass", _pending_mass.ptr());
            _main_table_statement->bindIntByName(":fragcount", _pending_fragcount.ptr());
            for (i = 0; i < _pending_counters.size(); i++)
            {
                char name[10] = {0};
                snprintf(name, sizeof(name), ":counter%d", i);
                _main_table_statement->bindIntByName(name, _pending_counters[i].ptr());
            }

            _main_table_statement->executeMultiple(_main_table_statement_count);
            if (_commit_main)
            {
                OracleStatement::executeSingle(env, "COMMIT");
                _commit_main = false;
            }
            profTimerStop(tmain);

            _main_table_statement.free();
            _pending_rid.clear();
            _pending_blockno.clear();
            _pending_offset.clear();
            _pending_gross.clear();
            _pending_cmf.clear();
            _pending_xyz.clear();
            _pending_mass.clear();
            _pending_fragcount.clear();
            for (i = 0; i < _pending_counters.size(); i++)
                _pending_counters[i].clear();
        }
        _main_table_statement_count = 0;
    }
}

void MangoShadowTable::_flushComponents(OracleEnv& env)
{
    // Flusing components table
    if (_components_table_statement.get() != 0)
    {
        if (_components_table_statement_count != 0)
        {
            profTimerStart(tcomp, "moleculeIndex.register_shadow_comp");
            _components_table_statement->prepare();
            _components_table_statement->bindIntByName(":count", _pending_comp_count.ptr());
            _components_table_statement->bindStringByName(":rid", _pending_comp_rid[0], 19);
            _components_table_statement->bindStringByName(":hash", _pending_comp_hash[0], 9);
            _components_table_statement->executeMultiple(_components_table_statement_count);
            if (_commit_comp)
            {
                OracleStatement::executeSingle(env, "COMMIT");
                _commit_comp = false;
            }
            _pending_comp_count.clear();
            _pending_comp_rid.clear();
            _pending_comp_hash.clear();
            profTimerStop(tcomp);
        }
        _components_table_statement.free();
        _components_table_statement_count = 0;
    }
}

void MangoShadowTable::addMolecule(OracleEnv& env, const MangoIndex& index, const char* rowid, int blockno, int offset, bool append)
{
    addMolecule(env, rowid, blockno, offset, index.getCmf().ptr(), index.getCmf().size(), index.getXyz().ptr(), index.getXyz().size(), index.getHash(),
                index.getGrossString(), index.getCountedElements(), index.getMolecularMass(), index.getFingerprint_Sim_Str(), append);
}

void MangoShadowTable::create(OracleEnv& env)
{
    OracleStatement s1(env);
    const char* mi = _table_name.ptr();
    int i;

    s1.append("CREATE TABLE %s "
              "(mol_rowid VARCHAR2(18), blockno NUMBER, offset NUMBER, "
              " gross VARCHAR2(500), cmf BLOB, xyz BLOB, MASS number, fragments NUMBER",
              mi);

    for (i = 0; i < (int)NELEM(MangoIndex::counted_elements); i++)
        s1.append(", cnt_%s INTEGER", Element::toString(MangoIndex::counted_elements[i]));
    s1.append(") NOLOGGING");
    s1.append(" LOB(cmf) STORE AS (NOCACHE NOLOGGING PCTVERSION 0)"
              " LOB(xyz) STORE AS (NOCACHE NOLOGGING PCTVERSION 0)");

    s1.prepare();
    s1.execute();

    // Create shadow table for molecule components
    const char* cmi = _components_table_name.ptr();
    OracleStatement::executeSingle(env,
                                   "CREATE TABLE %s "
                                   " (mol_rowid VARCHAR2(18), hash VARCHAR2(8), count INT) NOLOGGING",
                                   cmi);
}

void MangoShadowTable::createIndices(OracleEnv& env)
{
    const char* mi = _table_name.ptr();

    OracleStatement::executeSingle(env, "CREATE UNIQUE INDEX %s_rid ON %s(mol_rowid) NOLOGGING", mi, mi);
    OracleStatement::executeSingle(env, "CREATE INDEX %s_gross ON %s(gross) NOLOGGING", mi, mi);
    OracleStatement::executeSingle(env, "CREATE INDEX %s_mass ON %s(mass) NOLOGGING", mi, mi);

    if (NELEM(MangoIndex::counted_elements) > 0)
    {
        OracleStatement s2(env);
        int i;

        s2.append("CREATE INDEX %s_CNT ON %s(cnt_%s", mi, mi, Element::toString(MangoIndex::counted_elements[0]));
        for (i = 1; i < NELEM(MangoIndex::counted_elements); i++)
            s2.append(", cnt_%s", Element::toString(MangoIndex::counted_elements[i]));
        s2.append(") NOLOGGING");
        s2.prepare();
        s2.execute();
    }

    const char* cmi = _components_table_name.ptr();

    OracleStatement::executeSingle(env, "CREATE INDEX %s_rid ON %s(mol_rowid) NOLOGGING", cmi, cmi);
    OracleStatement::executeSingle(env, "CREATE INDEX %s_hash ON %s(hash) NOLOGGING", cmi, cmi);
    OracleStatement::executeSingle(env, "CREATE INDEX %s_count ON %s(count) NOLOGGING", cmi, cmi);
}

void MangoShadowTable::drop(OracleEnv& env)
{
    OracleStatement::executeSingle(env, "BEGIN DropTable('%s'); DropTable('%s'); END;", _table_name.ptr(), _components_table_name.ptr());
}

void MangoShadowTable::truncate(OracleEnv& env)
{
    OracleStatement::executeSingle(env, "TRUNCATE TABLE %s", _table_name.ptr());
    OracleStatement::executeSingle(env, "TRUNCATE TABLE %s", _components_table_name.ptr());
}

void MangoShadowTable::analyze(OracleEnv& env)
{
    env.dbgPrintf("analyzing shadow table\n");
    OracleStatement::executeSingle(env, "ANALYZE TABLE %s ESTIMATE STATISTICS", _table_name.ptr());
    OracleStatement::executeSingle(env, "ANALYZE TABLE %s ESTIMATE STATISTICS", _components_table_name.ptr());
}

bool MangoShadowTable::getXyz(OracleEnv& env, const char* rowid, ArrayChar& xyz)
{
    if (!OracleStatement::executeSingleBlob(xyz, env, "SELECT xyz FROM %s where mol_rowid='%s'", _table_name.ptr(), rowid))
        return false;
    return true;
}

const char* MangoShadowTable::getName()
{
    return _table_name.ptr();
}

const char* MangoShadowTable::getComponentsName()
{
    return _components_table_name.ptr();
}

bool MangoShadowTable::getMoleculeLocation(OracleEnv& env, const char* rowid, int& blockno, int& offset)
{
    OracleStatement statement(env);

    statement.append("SELECT blockno, offset FROM %s WHERE mol_rowid = :rid", _table_name.ptr());
    statement.prepare();
    statement.bindStringByName(":rid", rowid, strlen(rowid) + 1);
    statement.defineIntByPos(1, &blockno);
    statement.defineIntByPos(2, &offset);

    return statement.executeAllowNoData();
}

void MangoShadowTable::deleteMolecule(OracleEnv& env, const char* rowid)
{
    OracleStatement::executeSingle_BindString(env, ":rid", rowid, "DELETE FROM %s WHERE mol_rowid = :rid", _table_name.ptr());
    OracleStatement::executeSingle_BindString(env, ":rid", rowid, "DELETE FROM %s WHERE mol_rowid = :rid", _components_table_name.ptr());
}
