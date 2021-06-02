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

#ifndef __bingo_oracle_context__
#define __bingo_oracle_context__

#include "base_cpp/exception.h"
#include "core/bingo_context.h"
#include "oracle/bingo_storage.h"
#include "oracle/warnings_table.h"

using namespace indigo;

namespace ingido
{
    class BingoContext;
    class OracleEnv;
    class SharedMemory;
} // namespace ingido

class BingoOracleContext : public BingoContext
{
public:
    explicit BingoOracleContext(OracleEnv& env, int id);
    virtual ~BingoOracleContext();

    BingoStorage storage;
    WarningsTable warnings;

    int sim_screening_pass_mark;
    int sub_screening_pass_mark;
    int sub_screening_max_bits;

    static BingoOracleContext& get(OracleEnv& env, int id, bool lock, bool* config_reloaded);

    bool configGetInt(OracleEnv& env, const char* name, int& value);
    void configSetInt(OracleEnv& env, const char* name, int value);
    bool configGetIntDef(OracleEnv& env, const char* name, int& value, int default_value);
    bool configGetFloat(OracleEnv& env, const char* name, float& value);
    void configSetFloat(OracleEnv& env, const char* name, float value);
    bool configGetString(OracleEnv& env, const char* name, ArrayChar& value);
    void configSetString(OracleEnv& env, const char* name, const char* value);
    bool configGetBlob(OracleEnv& env, const char* name, ArrayChar& value);
    void configSetBlob(OracleEnv& env, const char* name, const ArrayChar& value);
    bool configGetClob(OracleEnv& env, const char* name, ArrayChar& value);
    void configSetClob(OracleEnv& env, const char* name, const ArrayChar& value);

    void configResetAll(OracleEnv& env);
    void configReset(OracleEnv& env, const char* name);

    void tautomerLoadRules(OracleEnv& env);
    void fingerprintLoadParameters(OracleEnv& env);

    void saveCmfDict(OracleEnv& env);
    void saveRidDict(OracleEnv& env);

    void longOpInit(OracleEnv& env, int total, const char* operation, const char* target, const char* units);
    void longOpUpdate(OracleEnv& env, int sofar);

    void parseParameters(OracleEnv& env, const char* str);

    void atomicMassLoad(OracleEnv& env);
    void atomicMassSave(OracleEnv& env);

    void setLogTableWithColumns(OracleEnv& env, const char* tableWithColumns);

    void lock(OracleEnv& env);
    void unlock(OracleEnv& env);

protected:
    bool _config_changed;

    int _longop_slno;
    int _longop_rindex;
    ArrayChar _longop_operation;
    ArrayChar _longop_units;
    ArrayChar _longop_target; // actually, it is the source table
    int _longop_total;

    ArrayChar _id;
    SharedMemory* _shmem;

    void _loadConfigParameters(OracleEnv& env);
};

#endif
