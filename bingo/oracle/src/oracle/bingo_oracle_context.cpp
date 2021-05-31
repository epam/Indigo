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

#include "oracle/bingo_oracle_context.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/shmem.h"
#include "core/bingo_context.h"
#include "core/bingo_error.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_tautomer.h"
#include "oracle/bingo_oracle.h"
#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"

BingoOracleContext::BingoOracleContext(OracleEnv& env, int id_) : BingoContext(id_), storage(env, id_), _config_changed(false)
{
    StringOutput output(_id);

    output.printf("BINGO_%d", id_);
    output.writeChar(0);

    _shmem = 0;
}

BingoOracleContext::~BingoOracleContext()
{
    delete _shmem;
}

BingoOracleContext& BingoOracleContext::get(OracleEnv& env, int id, bool lock, bool* config_reloaded)
{
    BingoOracleContext* already = (BingoOracleContext*)_get(id);

    if (config_reloaded != 0)
        *config_reloaded = false;

    if (already != 0)
    {
        if (lock)
            already->lock(env);
        if (already->_config_changed)
        {
            env.dbgPrintfTS("reloading config\n");
            already->_loadConfigParameters(env);
            if (config_reloaded != 0)
                *config_reloaded = true;
        }
        return *already;
    }

    AutoPtr<BingoOracleContext> res(new BingoOracleContext(env, id));

    if (lock)
        res->lock(env);
    res->_loadConfigParameters(env);
    if (config_reloaded != 0)
        *config_reloaded = true;

    OsLocker locker(_instances_lock);
    TL_GET(PtrArray<BingoContext>, _instances);

    _instances.add(res.release());

    return *(BingoOracleContext*)_instances.top();
}

void BingoOracleContext::_loadConfigParameters(OracleEnv& env)
{
    fingerprintLoadParameters(env);
    tautomerLoadRules(env);
    atomicMassLoad(env);

    int val;

    configGetIntDef(env, "TREAT_X_AS_PSEUDOATOM", val, 0);
    treat_x_as_pseudoatom = (val != 0);

    configGetIntDef(env, "IGNORE_CLOSING_BOND_DIRECTION_MISMATCH", val, 0);
    ignore_closing_bond_direction_mismatch = (val != 0);

    configGetIntDef(env, "IGNORE_STEREOCENTER_ERRORS", val, 0);
    ignore_stereocenter_errors = (val != 0);

    configGetIntDef(env, "IGNORE_CISTRANS_ERRORS", val, 0);
    ignore_cistrans_errors = (val != 0);

    configGetIntDef(env, "STEREOCHEMISTRY_BIDIRECTIONAL_MODE", val, 0);
    stereochemistry_bidirectional_mode = (val != 0);

    configGetIntDef(env, "STEREOCHEMISTRY_DETECT_HAWORTH_PROJECTION", val, 0);
    stereochemistry_detect_haworth_projection = (val != 0);

    configGetIntDef(env, "ALLOW_NON_UNIQUE_DEAROMATIZATION", val, 0);
    allow_non_unique_dearomatization = (val != 0);

    configGetIntDef(env, "ZERO_UNKNOWN_AROMATIC_HYDROGENS", val, 0);
    zero_unknown_aromatic_hydrogens = (val != 0);

    configGetIntDef(env, "REJECT_INVALID_STRUCTURES", val, 0);
    reject_invalid_structures = (val != 0);

    configGetIntDef(env, "IGNORE_BAD_VALENCE", val, 0);
    ignore_bad_valence = (val != 0);

    QS_DEF(std::string, cmfdict);

    if (configGetBlob(env, "CMFDICT", cmfdict))
    {
        BufferScanner scanner(cmfdict);

        cmf_dict.load(scanner);
    }

    QS_DEF(std::string, riddict);

    if (configGetBlob(env, "RIDDICT", riddict))
    {
        BufferScanner scanner(riddict);

        rid_dict.load(scanner);
    }

    configGetInt(env, "SUB_SCREENING_PASS_MARK", sub_screening_pass_mark);
    configGetInt(env, "SIM_SCREENING_PASS_MARK", sim_screening_pass_mark);
    configGetInt(env, "SUB_SCREENING_MAX_BITS", sub_screening_max_bits);

    QS_DEF(std::string, log_table);
    if (configGetString(env, "LOG_TABLE", log_table))
        warnings.setTableNameAndColumns(env, log_table.c_str());
    else
        warnings.reset();

    _config_changed = false;
}

void BingoOracleContext::saveCmfDict(OracleEnv& env)
{
    env.dbgPrintfTS("saving cmf dictionary\n");

    QS_DEF(std::string, cmfdict);

    StringOutput output(cmfdict);
    cmf_dict.saveFull(output);
    cmf_dict.resetModified();

    configSetBlob(env, "CMFDICT", cmfdict);
}

void BingoOracleContext::saveRidDict(OracleEnv& env)
{
    env.dbgPrintfTS("saving rowid dictionary\n");

    QS_DEF(std::string, riddict);

    StringOutput output(riddict);
    rid_dict.saveFull(output);
    rid_dict.resetModified();

    configSetBlob(env, "RIDDICT", riddict);
}

bool BingoOracleContext::configGetIntDef(OracleEnv& env, const char* name, int& value, int default_value)
{
    if (!OracleStatement::executeSingleInt(value, env,
                                           "SELECT value FROM "
                                           "(SELECT value FROM config_int WHERE name = upper('%s') AND n in (0, %d) "
                                           "ORDER BY n DESC) WHERE rownum <= 1",
                                           name, id))
    {
        return false;
        value = default_value;
    }
    return true;
}

bool BingoOracleContext::configGetInt(OracleEnv& env, const char* name, int& value)
{
    if (!OracleStatement::executeSingleInt(value, env,
                                           "SELECT value FROM "
                                           "(SELECT value FROM config_int WHERE name = upper('%s') AND n in (0, %d) "
                                           "ORDER BY n DESC) WHERE rownum <= 1",
                                           name, id))
        return false;

    return true;
}

void BingoOracleContext::configResetAll(OracleEnv& env)
{
    OracleStatement::executeSingle(env, "DELETE FROM CONFIG_INT WHERE n = %d", id);
    OracleStatement::executeSingle(env, "DELETE FROM CONFIG_STR WHERE n = %d", id);
    OracleStatement::executeSingle(env, "DELETE FROM CONFIG_FLOAT WHERE n = %d", id);
    OracleStatement::executeSingle(env, "DELETE FROM CONFIG_BLOB WHERE n = %d", id);
    OracleStatement::executeSingle(env, "DELETE FROM CONFIG_CLOB WHERE n = %d", id);
    _config_changed = true;
}

void BingoOracleContext::configReset(OracleEnv& env, const char* name)
{
    OracleStatement::executeSingle(env, "DELETE FROM CONFIG_INT WHERE name = upper('%s') AND n = %d", name, id);
    OracleStatement::executeSingle(env, "DELETE FROM CONFIG_STR WHERE name = upper('%s') AND n = %d", name, id);
    OracleStatement::executeSingle(env, "DELETE FROM CONFIG_FLOAT WHERE name = upper('%s') AND n = %d", name, id);
    OracleStatement::executeSingle(env, "DELETE FROM CONFIG_BLOB WHERE name = upper('%s') AND n = %d", name, id);
    OracleStatement::executeSingle(env, "DELETE FROM CONFIG_CLOB WHERE name = upper('%s') AND n = %d", name, id);
    _config_changed = true;
}

void BingoOracleContext::configSetInt(OracleEnv& env, const char* name, int value)
{
    configReset(env, name);
    OracleStatement::executeSingle(env, "INSERT INTO CONFIG_INT VALUES(%d, upper('%s'), %d)", id, name, value);
    _config_changed = true;
}

bool BingoOracleContext::configGetFloat(OracleEnv& env, const char* name, float& value)
{
    if (!OracleStatement::executeSingleFloat(value, env,
                                             "SELECT value FROM "
                                             "(SELECT value FROM CONFIG_FLOAT WHERE name = upper('%s') AND n in (0, %d) "
                                             "ORDER BY n DESC) WHERE rownum <= 1",
                                             name, id))
        return false;

    return true;
}

void BingoOracleContext::configSetFloat(OracleEnv& env, const char* name, float value)
{
    configReset(env, name);
    OracleStatement::executeSingle(env, "INSERT INTO CONFIG_FLOAT VALUES(%d, upper('%s'), %f)", id, name, value);
    _config_changed = true;
}

bool BingoOracleContext::configGetString(OracleEnv& env, const char* name, std::string& value)
{
    if (!OracleStatement::executeSingleString(value, env,
                                              "SELECT value FROM "
                                              "(SELECT value FROM config_str WHERE name = upper('%s') AND n in (0, %d) "
                                              "ORDER BY n DESC) WHERE rownum <= 1",
                                              name, id))
        return false;

    return true;
}

void BingoOracleContext::configSetString(OracleEnv& env, const char* name, const char* value)
{
    configReset(env, name);
    OracleStatement::executeSingle(env, "INSERT INTO CONFIG_STR VALUES(%d, upper('%s'), '%s')", id, name, value);
    _config_changed = true;
}

bool BingoOracleContext::configGetBlob(OracleEnv& env, const char* name, std::string& value)
{
    if (!OracleStatement::executeSingleBlob(value, env,
                                            "SELECT value FROM "
                                            "(SELECT value FROM CONFIG_BLOB WHERE name = upper('%s') AND n in (0, %d) "
                                            "ORDER BY n DESC) WHERE rownum <= 1",
                                            name, id))
        return false;

    return true;
}

void BingoOracleContext::configSetBlob(OracleEnv& env, const char* name, const std::string& value)
{
    configReset(env, name);

    OracleLOB lob(env);
    OracleStatement statement(env);

    lob.createTemporaryBLOB();
    lob.write(0, value.c_str(), value.size());
    statement.append("INSERT INTO config_blob VALUES (%d, upper('%s'), :value)", id, name);

    statement.prepare();
    statement.bindBlobByName(":value", lob);
    statement.execute();

    _config_changed = true;
}

bool BingoOracleContext::configGetClob(OracleEnv& env, const char* name, std::string& value)
{
    if (!OracleStatement::executeSingleClob(value, env,
                                            "SELECT value FROM "
                                            "(SELECT value FROM CONFIG_CLOB WHERE name = upper('%s') AND n in (0, %d) "
                                            "ORDER BY n DESC) WHERE rownum <= 1",
                                            name, id))
        return false;

    return true;
}

void BingoOracleContext::configSetClob(OracleEnv& env, const char* name, const std::string& value)
{
    configReset(env, name);

    OracleLOB lob(env);
    OracleStatement statement(env);

    lob.createTemporaryCLOB();
    lob.write(0, value.c_str(), value.size());
    statement.append("INSERT INTO config_clob VALUES (%d, upper('%s'), :value)", id, name);

    statement.prepare();
    statement.bindBlobByName(":value", lob);
    statement.execute();

    _config_changed = true;
}

void BingoOracleContext::tautomerLoadRules(OracleEnv& env)
{
    tautomer_rules.clear();

    OracleStatement statement(env);
    int n;
    char param1[128], param2[128];

    statement.append("SELECT id, beg, end FROM tautomer_rules ORDER BY id ASC");
    statement.prepare();
    statement.defineIntByPos(1, &n);
    statement.defineStringByPos(2, param1, sizeof(param1));
    statement.defineStringByPos(3, param2, sizeof(param2));

    if (statement.executeAllowNoData())
        do
        {
            if (n < 1 || n >= 32)
                throw BingoError("tautomer rule index %d is out of range", n);

            AutoPtr<TautomerRule> rule(new TautomerRule());

            bingoGetTauCondition(param1, rule->aromaticity1, rule->list1);
            bingoGetTauCondition(param2, rule->aromaticity2, rule->list2);

            tautomer_rules.expand(n);
            tautomer_rules[n - 1] = rule.release();
        } while (statement.fetch());
}

void BingoOracleContext::fingerprintLoadParameters(OracleEnv& env)
{
    configGetInt(env, "FP_ORD_SIZE", fp_parameters.ord_qwords);
    configGetInt(env, "FP_TAU_SIZE", fp_parameters.tau_qwords);
    configGetInt(env, "FP_SIM_SIZE", fp_parameters.sim_qwords);
    configGetInt(env, "FP_ANY_SIZE", fp_parameters.any_qwords);
    fp_parameters.ext = true;
    fp_parameters_ready = true;

    configGetInt(env, "FP_STORAGE_CHUNK", fp_chunk_qwords);
}

void BingoOracleContext::longOpInit(OracleEnv& env, int total, const char* operation, const char* target, const char* units)
{
    _longop_slno = 0;
    _longop_total = total;
    _longop_operation = operation;
    _longop_target = target;
    _longop_units = units;

    OracleStatement statement(env);

    statement.append("BEGIN :longop_rindex := DBMS_APPLICATION_INFO.set_session_longops_nohint; END;");
    statement.prepare();
    statement.bindIntByName(":longop_rindex", &_longop_rindex);
    statement.execute();
}

void BingoOracleContext::longOpUpdate(OracleEnv& env, int sofar)
{
    OracleStatement statement(env);

    statement.append("BEGIN dbms_application_info.set_session_longops("
                     "rindex    => :longop_rindex, "
                     "slno      => :longop_slno, "
                     "op_name   => '%s', sofar => %d, totalwork => %d, target_desc => '%s', "
                     "units => '%s'); END;",
                     _longop_operation.c_str(), sofar, _longop_total, _longop_target.c_str(), _longop_units.c_str());

    statement.prepare();
    statement.bindIntByName(":longop_rindex", &_longop_rindex);
    statement.bindIntByName(":longop_slno", &_longop_slno);
    statement.execute();
}

void BingoOracleContext::parseParameters(OracleEnv& env, const char* str)
{
    BufferScanner scanner(str);

    QS_DEF(std::string, param_name);
    QS_DEF(std::string, param_value);

    static const char* PARAMETERS_INT[] = {
        "FP_ORD_SIZE",
        "FP_TAU_SIZE",
        "FP_ORD_BPC",
        "FP_TAU_BPC",
        "FP_MAX_CYCLE_LEN",
        "FP_MIN_TREE_EDGES",
        "FP_MAX_TREE_EDGES",
        "FP_STORAGE_CHUNK",
        "TREAT_X_AS_PSEUDOATOM",
        "IGNORE_CLOSING_BOND_DIRECTION_MISMATCH",
        "IGNORE_STEREOCENTER_ERRORS",
        "IGNORE_CISTRANS_ERRORS",
        "ALLOW_NON_UNIQUE_DEAROMATIZATION",
        "ZERO_UNKNOWN_AROMATIC_HYDROGENS",
        "REJECT_INVALID_STRUCTURES",
    };

    bool config_changed = false;

    scanner.skipSpace();
    while (!scanner.isEOF())
    {
        scanner.readWord(param_name, " =");
        scanner.skipSpace();
        if (scanner.readChar() != '=')
            throw Error("can't parse parameters: '%s'", str);

        scanner.skipSpace();

        bool parameter_found = false;
        for (int i = 0; i < NELEM(PARAMETERS_INT); i++)
            if (strcasecmp(param_name.c_str(), PARAMETERS_INT[i]) == 0)
            {
                int value = scanner.readInt();
                configSetInt(env, PARAMETERS_INT[i], value);

                parameter_found = true;
                config_changed = true;
                break;
            }

        if (strcasecmp(param_name.c_str(), "NTHREADS") == 0)
        {
            nthreads = scanner.readInt();
            parameter_found = true;
        }

        if (strcasecmp(param_name.c_str(), "LOG_TABLE") == 0)
        {
            scanner.readWord(param_value, " ");
            setLogTableWithColumns(env, param_value.c_str());
            parameter_found = true;
        }

        if (!parameter_found)
            throw Error("unknown parameter %s", param_name.c_str());

        scanner.skipSpace();
    }

    if (config_changed)
        _loadConfigParameters(env);
}

void BingoOracleContext::atomicMassLoad(OracleEnv& env)
{
    relative_atomic_mass_map.clear();

    if (!configGetString(env, "RELATIVE_ATOMIC_MASS", _relative_atomic_mass))
        return;

    const char* buffer = _relative_atomic_mass.c_str();
    QS_DEF(std::string, element_str);
    element_str.resize(_relative_atomic_mass.size());

    float mass;
    int pos;

    while (sscanf(buffer, "%s%f%n", element_str.c_str(), &mass, &pos) > 1)
    {
        int elem = Element::fromString(element_str.c_str());
        if (relative_atomic_mass_map.find(elem))
            throw Error("element '%s' duplication in atomic mass list", element_str.c_str());

        relative_atomic_mass_map.insert(elem, mass);
        buffer += pos;

        while (*buffer == ' ')
            buffer++;
        if (buffer[0] == ';')
            buffer++;
    }

    // Print debug information
    if (relative_atomic_mass_map.size() != 0)
    {
        env.dbgPrintfTS("Relative atomic mass read: ");
        for (int i = relative_atomic_mass_map.begin(); i != relative_atomic_mass_map.end(); i = relative_atomic_mass_map.next(i))
        {
            int elem = relative_atomic_mass_map.key(i);
            float mass = relative_atomic_mass_map.value(i);
            env.dbgPrintf("%s %g; ", Element::toString(elem), mass);
        }
        env.dbgPrintf("\n");
    }
}

void BingoOracleContext::atomicMassSave(OracleEnv& env)
{
    if (_relative_atomic_mass.size() > 1)
        configSetString(env, "RELATIVE_ATOMIC_MASS", _relative_atomic_mass.c_str());
    else
        configSetString(env, "RELATIVE_ATOMIC_MASS", "");
}

void BingoOracleContext::setLogTableWithColumns(OracleEnv& env, const char* tableWithColumns)
{
    configSetString(env, "LOG_TABLE", tableWithColumns);
}

void BingoOracleContext::lock(OracleEnv& env)
{
    // TODO: implement a semaphore?
    env.dbgPrintf("    locking %s... ", _id.c_str());

    if (_shmem != 0)
    {
        env.dbgPrintf("already locked\n");
        return;
    }

    while (1)
    {
        _shmem = new SharedMemory(_id.c_str(), 1, false);

        if (_shmem->wasFirst())
            break;

        delete _shmem;
    }
    env.dbgPrintf("locked\n");
}

void BingoOracleContext::unlock(OracleEnv& env)
{
    if (_shmem == 0)
    {
        env.dbgPrintf("%s is not locked by this process\n", _id.c_str());
        return;
    }
    env.dbgPrintf("unlocking %s\n", _id.c_str());
    delete _shmem;
    _shmem = 0;
}
