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

#include "oracle/bingo_profiling.h"
#include "oracle/bingo_oracle.h"
#include "oracle/ora_logger.h"

#include "base_cpp/output.h"
#include "base_cpp/profiling.h"
#include "base_cpp/tlscont.h"

void bingoProfilingPrintStatistics(bool print_all)
{
    // Print profiling statistics
    QS_DEF(std::string, buffer);
    StringOutput output(buffer);
    profGetStatistics(output, print_all);
    buffer.push(0);
    logger.dbgPrintf("Profiling statistics:\n");
    logger.dbgPrintf(buffer.ptr());
    logger.dbgPrintf("\n");
}

ORAEXT OCINumber* oraProfilingGetCount(OCIExtProcContext* ctx, char* key_name, short key_name_indicator, short* return_indicator)
{
    OCINumber* result = NULL;

    ORABLOCK_BEGIN
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (key_name_indicator != OCI_IND_NOTNULL)
            throw BingoError("Null key is given");

        qword value;

        // Try to find in profiling data
        ProfilingSystem& inst = ProfilingSystem::getInstance();
        if (inst.hasLabel(key_name))
            // throw BingoError("Key wasn't found");
            value = inst.getLabelCallCount(key_name);
        else
            value = 0;

        result = (OCINumber*)OCIExtProcAllocCallMemory(ctx, sizeof(OCINumber));

        if (result == NULL)
            throw BingoError("can't allocate memory for number");

        env.callOCI(OCINumberFromInt(env.errhp(), &value, sizeof(qword), OCI_NUMBER_SIGNED, result));

        *return_indicator = OCI_IND_NOTNULL;
    }
    ORABLOCK_END

    return result;
}

ORAEXT OCINumber* oraProfilingGetTime(OCIExtProcContext* ctx, char* key_name, short key_name_indicator, short* return_indicator)
{
    OCINumber* result = NULL;

    ORABLOCK_BEGIN
    {
        *return_indicator = OCI_IND_NULL;

        OracleEnv env(ctx, logger);

        if (key_name_indicator != OCI_IND_NOTNULL)
            throw BingoError("Null key is given");

        float value;

        // Try to find in profiling data
        ProfilingSystem& inst = ProfilingSystem::getInstance();
        if (inst.hasLabel(key_name))
            // throw BingoError("Key wasn't found");
            value = inst.getLabelExecTime(key_name);
        else
            value = 0;

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

ORAEXT void oraProfilingPrint(OCIExtProcContext* ctx, OCINumber* print_all, short print_all_indicator)
{
    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        if (print_all_indicator != OCI_IND_NOTNULL)
            throw BingoError("Null value is given");

        ub4 print_all_int;

        env.callOCI(OCINumberToInt(env.errhp(), print_all, sizeof(print_all_int), OCI_NUMBER_UNSIGNED, &print_all_int));

        bingoProfilingPrintStatistics(print_all_int != 0);
    }
    ORABLOCK_END
}
