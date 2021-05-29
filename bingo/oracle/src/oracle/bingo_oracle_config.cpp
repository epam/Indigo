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

#include "base_cpp/tlscont.h"
#include "oracle/bingo_oracle.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"

//
// Oracle wrappers for bingo config
//

ORAEXT void oraConfigResetAll(OCIExtProcContext* ctx, int context_id){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

BingoOracleContext& context = BingoOracleContext::get(env, context_id, false, 0);
context.configResetAll(env);
}
ORABLOCK_END
}

ORAEXT void oraConfigSetInt(OCIExtProcContext* ctx, int context_id, char* key_name, short key_name_indicator, OCINumber* value,
                            short value_indicator){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

if (key_name_indicator != OCI_IND_NOTNULL)
    throw BingoError("Null key is given");
if (value_indicator != OCI_IND_NOTNULL)
    throw BingoError("Null value is given");

ub4 value_int;

env.callOCI(OCINumberToInt(env.errhp(), value, sizeof(value_int), OCI_NUMBER_UNSIGNED, &value_int));

BingoOracleContext& context = BingoOracleContext::get(env, context_id, false, 0);
context.configSetInt(env, key_name, value_int);
}
ORABLOCK_END
}

ORAEXT void oraConfigSetFloat(OCIExtProcContext* ctx, int context_id, char* key_name, short key_name_indicator, OCINumber* value,
                              short value_indicator){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

if (key_name_indicator != OCI_IND_NOTNULL)
    throw BingoError("Null key is given");
if (value_indicator != OCI_IND_NOTNULL)
    throw BingoError("Null value is given");

double value_float;

env.callOCI(OCINumberToReal(env.errhp(), value, sizeof(value_float), &value_float));

BingoOracleContext& context = BingoOracleContext::get(env, context_id, false, 0);
context.configSetFloat(env, key_name, (float)value_float);
}
ORABLOCK_END
}

ORAEXT void oraConfigSetString(OCIExtProcContext* ctx, int context_id, char* key_name, short key_name_indicator, char* value,
                               short value_indicator){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

if (key_name_indicator != OCI_IND_NOTNULL)
    throw BingoError("Null key is given");
if (value_indicator != OCI_IND_NOTNULL)
    throw BingoError("Null value is given");

BingoOracleContext& context = BingoOracleContext::get(env, context_id, false, 0);
context.configSetString(env, key_name, value);
}
ORABLOCK_END
}

ORAEXT OCINumber* oraConfigGetInt(OCIExtProcContext* ctx, int context_id, char* key_name, short key_name_indicator, short* return_indicator)
{
    OCINumber* result = NULL;

    ORABLOCK_BEGIN
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (key_name_indicator != OCI_IND_NOTNULL)
            throw BingoError("Null key is given");

        int value;
        BingoOracleContext& context = BingoOracleContext::get(env, context_id, false, 0);
        if (!context.configGetInt(env, key_name, value))
            throw BingoError("Key wasn't found");

        result = (OCINumber*)OCIExtProcAllocCallMemory(ctx, sizeof(OCINumber));

        if (result == NULL)
            throw BingoError("can't allocate memory for number");

        env.callOCI(OCINumberFromInt(env.errhp(), &value, sizeof(int), OCI_NUMBER_SIGNED, result));

        *return_indicator = OCI_IND_NOTNULL;
    }
    ORABLOCK_END

    return result;
}

ORAEXT OCINumber* oraConfigGetFloat(OCIExtProcContext* ctx, int context_id, char* key_name, short key_name_indicator, short* return_indicator)
{
    OCINumber* result = NULL;

    ORABLOCK_BEGIN
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (key_name_indicator != OCI_IND_NOTNULL)
            throw BingoError("Null key is given");

        float value;
        BingoOracleContext& context = BingoOracleContext::get(env, context_id, false, 0);
        if (!context.configGetFloat(env, key_name, value))
            throw BingoError("Key wasn't found");

        result = (OCINumber*)OCIExtProcAllocCallMemory(ctx, sizeof(OCINumber));

        if (result == NULL)
            throw BingoError("can't allocate memory for number");

        double value_double = value;
        env.callOCI(OCINumberFromReal(env.errhp(), &value_double, sizeof(value_double), result));

        *return_indicator = OCI_IND_NOTNULL;
    }
    ORABLOCK_END

    return result;
}

ORAEXT OCIString* oraConfigGetString(OCIExtProcContext* ctx, int context_id, char* key_name, short key_name_indicator, short* return_indicator)
{
    OCIString* result = NULL;

    ORABLOCK_BEGIN
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (key_name_indicator != OCI_IND_NOTNULL)
            throw BingoError("Null key is given");

        QS_DEF(std::string, value);
        BingoOracleContext& context = BingoOracleContext::get(env, context_id, false, 0);
        if (!context.configGetString(env, key_name, value))
            throw BingoError("Key wasn't found");

        env.callOCI(OCIStringAssignText(env.envhp(), env.errhp(), (text*)value.ptr(), value.size() - 1, &result));

        *return_indicator = OCI_IND_NOTNULL;
    }
    ORABLOCK_END

    return result;
}

ORAEXT void oraConfigReset(OCIExtProcContext* ctx, int context_id, char* key_name, short key_name_indicator)
{
    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        if (key_name_indicator != OCI_IND_NOTNULL)
            throw BingoError("Null key is given");

        BingoOracleContext& context = BingoOracleContext::get(env, context_id, false, 0);
        context.configReset(env, key_name);
    }
    ORABLOCK_END
}
