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

#include <oci.h>

#include "core/ringo_matchers.h"
#include "oracle/bingo_oracle.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"
#include "oracle/ringo_fetch_context.h"
#include "oracle/ringo_oracle.h"

static void _ringoIndexStart(OracleEnv& env, RingoFetchContext& context, const char* oper, const Array<char>& query_buf, OCINumber* p_strt, OCINumber* p_stop,
                             const char* params)
{
    RingoShadowFetch& shadow_fetch = *context.shadow_fetch;
    RingoFastIndex& fast_index = *context.fast_index;

    if (strcasecmp(oper, "RSUB") == 0)
    {
        if (!context.substructure.parse(params))
            throw BingoError("can not parse parameters: %s", params);

        context.substructure.loadQuery(query_buf);

        int right = bingoGetExactRightPart(env, p_strt, p_stop, 64);

        if (right == 1)
        {
            fast_index.prepareSubstructure(env);
            context.fetch_engine = &fast_index;
        }
        else // right == 0
        {
            shadow_fetch.prepareNonSubstructure(env);
            context.fetch_engine = &shadow_fetch;
        }
    }
    else if (strcasecmp(oper, "RSMARTS") == 0)
    {
        context.substructure.loadSMARTS(query_buf);

        int right = bingoGetExactRightPart(env, p_strt, p_stop, 64);

        if (right == 1)
        {
            fast_index.prepareSubstructure(env);
            context.fetch_engine = &fast_index;
        }
        else // right == 0
        {
            shadow_fetch.prepareNonSubstructure(env);
            context.fetch_engine = &shadow_fetch;
        }
    }
    else if (strcasecmp(oper, "REXACT") == 0)
    {
        context.exact.setParameters(params);
        context.exact.loadQuery(query_buf);

        int right = bingoGetExactRightPart(env, p_strt, p_stop, 64);

        shadow_fetch.prepareExact(env, right);
        context.fetch_engine = &shadow_fetch;
    }
    else
        throw BingoError("unknown operator: %s", oper);
}

ORAEXT int oraRingoIndexStart(OCIExtProcContext* ctx, int context_id, const char* oper, short oper_ind, OCILobLocator* query_loc, short query_ind,
                              OCINumber* p_strt, short strt_ind, OCINumber* p_stop, short stop_ind, const char* params, short params_ind)
{
    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        env.dbgPrintf("IndexStart... ");

        if (oper_ind != OCI_IND_NOTNULL)
            throw BingoError("null operator given");
        if (query_ind != OCI_IND_NOTNULL)
            throw BingoError("null query given");
        if (params_ind != OCI_IND_NOTNULL)
            params = 0;
        if (strt_ind != OCI_IND_NOTNULL)
            p_strt = 0;
        if (stop_ind != OCI_IND_NOTNULL)
            p_stop = 0;

        QS_DEF(Array<char>, query_buf);
        OracleLOB query_lob(env, query_loc);

        query_lob.readAll(query_buf, false);

        QS_DEF(Array<char>, query_id);

        bingoBuildQueryID(env, oper, query_buf, p_strt, p_stop, 0, params, query_id);

        RingoOracleContext& context = RingoOracleContext::get(env, context_id, false);
        RingoFetchContext* fetch_context = RingoFetchContext::findFresh(context_id, query_id);

        if (fetch_context == 0)
        {
            fetch_context = &RingoFetchContext::create(context, query_id);
            _ringoIndexStart(env, *fetch_context, oper, query_buf, p_strt, p_stop, params);
            env.dbgPrintf("created fetcher #%d\n", fetch_context->id);
        }
        else
        {
            fetch_context->fresh = false;
            env.dbgPrintf("found fresh fetcher #%d\n", fetch_context->id);
        }
        return fetch_context->id;
    }
    ORABLOCK_END

    return -1;
}

ORAEXT int oraRingoIndexFetch(OCIExtProcContext* ctx, int fetch_id, int maxrows, OCIArray** array, short array_indicator)
{
    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        RingoFetchContext& context = RingoFetchContext::get(fetch_id);

        if (context.fetch_engine == 0)
            throw BingoError("fetch_engine = 0 in oraRingoIndexFetch()");

        BingoFetchEngine& fetch_engine = *context.fetch_engine;

        ORA_TRY_FETCH_BEGIN
        {
            if (maxrows > 100)
                maxrows = 100;

            // can have fetched rowid-s from the selectivity computation phase
            maxrows -= bingoPopRowidsToArray(env, fetch_engine.matched, maxrows, *array);

            if (maxrows > 0 && !fetch_engine.end())
            {
                env.dbgPrintf("[fetcher #%d] ", context.id);

                fetch_engine.fetch(env, maxrows);
                maxrows -= bingoPopRowidsToArray(env, fetch_engine.matched, maxrows, *array);
            }

            return fetch_engine.end() ? 0 : 1;
        }
        ORA_TRY_FETCH_END
    }
    ORABLOCK_END

    return 0;
}

ORAEXT void oraRingoIndexClose(OCIExtProcContext* ctx, int fetch_id){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

env.dbgPrintfTS("closing fetch #%d\n", fetch_id);

RingoFetchContext::remove(fetch_id);
}
ORABLOCK_END
}

ORAEXT OCINumber* oraRingoIndexSelectivity(OCIExtProcContext* ctx, int context_id, const char* oper, short oper_ind, OCILobLocator* query_loc, short query_ind,
                                           OCINumber* p_strt, short strt_ind, OCINumber* p_stop, short stop_ind, const char* params, short params_ind,
                                           short* return_ind)
{
    *return_ind = OCI_IND_NULL;

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        env.dbgPrintf("IndexSelectivity... ");

        if (oper_ind != OCI_IND_NOTNULL)
            throw BingoError("null operator given");
        if (query_ind != OCI_IND_NOTNULL)
            throw BingoError("null query given");
        if (params_ind != OCI_IND_NOTNULL)
            params = 0;
        if (strt_ind != OCI_IND_NOTNULL)
            p_strt = 0;
        if (stop_ind != OCI_IND_NOTNULL)
            p_stop = 0;

        RingoOracleContext& context = RingoOracleContext::get(env, context_id, false);

        QS_DEF(Array<char>, query_id);
        QS_DEF(Array<char>, query_buf);
        OracleLOB query_lob(env, query_loc);

        query_lob.readAll(query_buf, false);

        bingoBuildQueryID(env, oper, query_buf, p_strt, p_stop, 0, params, query_id);

        RingoFetchContext* fetch_context = RingoFetchContext::findFresh(context_id, query_id);

        if (fetch_context == 0)
        {
            fetch_context = &RingoFetchContext::create(context, query_id);
            env.dbgPrintf("created fresh fetcher #%d\n", fetch_context->id);
        }
        else
            env.dbgPrintf("found fresh fetcher #%d\n", fetch_context->id);

        _ringoIndexStart(env, *fetch_context, oper, query_buf, p_strt, p_stop, params);

        env.dbgPrintf("[fetcher #%d] ", fetch_context->id);
        fetch_context->fetch_engine->fetch(env, 100);
        fetch_context->fresh = true;

        float res = fetch_context->fetch_engine->calcSelectivity(env, context.fingerprints.getTotalCount(env));

        env.dbgPrintfTS("calculated selectivity = %.2f\n", res);

        OCINumber* result = OracleExtproc::createDouble(env, (double)res);

        *return_ind = OCI_IND_NOTNULL;
        return result;
    }
    ORABLOCK_END

    return 0;
}

ORAEXT void oraRingoIndexCost(OCIExtProcContext* ctx, int context_id, OCINumber* p_sel, short sel_ind, const char* oper, short oper_ind,
                              OCILobLocator* query_loc, short query_ind, OCINumber* p_strt, short strt_ind, OCINumber* p_stop, short stop_ind,
                              const char* params, short params_ind, int* iocost, int* cpucost){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

env.dbgPrintfTS("IndexCost... ");

if (sel_ind != OCI_IND_NOTNULL)
    throw BingoError("null selectivity given");

if (oper_ind != OCI_IND_NOTNULL)
    throw BingoError("null operator given");
if (query_ind != OCI_IND_NOTNULL)
    throw BingoError("null query given");
if (params_ind != OCI_IND_NOTNULL)
    params = 0;
if (strt_ind != OCI_IND_NOTNULL)
    p_strt = 0;
if (stop_ind != OCI_IND_NOTNULL)
    p_stop = 0;

float sel = OracleUtil::numberToFloat(env, p_sel);

QS_DEF(Array<char>, query_buf);
OracleLOB query_lob(env, query_loc);

query_lob.readAll(query_buf, false);

QS_DEF(Array<char>, query_id);

bingoBuildQueryID(env, oper, query_buf, p_strt, p_stop, 0, params, query_id);

RingoOracleContext& context = RingoOracleContext::get(env, context_id, false);
RingoFetchContext* fetch_context = RingoFetchContext::findFresh(context_id, query_id);

if (fetch_context == 0)
{
    fetch_context = &RingoFetchContext::create(context, query_id);
    _ringoIndexStart(env, *fetch_context, oper, query_buf, p_strt, p_stop, params);
    fetch_context->fresh = true;
    env.dbgPrintf("created fresh fetcher #%d\n", fetch_context->id);
}
else
    env.dbgPrintf("found fresh fetcher #%d\n", fetch_context->id);

*iocost = fetch_context->fetch_engine->getIOCost(env, sel);
*cpucost = (int)(context.fingerprints.getTotalCount(env) * sel);

env.dbgPrintfTS("calculated iocost = %d, cpucost = %d\n", *iocost, *cpucost);
}
ORABLOCK_END
}

ORAEXT void oraRingoCollectStatistics(OCIExtProcContext* ctx, int context_id)
{
    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        RingoOracleContext& context = RingoOracleContext::get(env, context_id, false);

        context.shadow_table.analyze(env);
    }
    ORABLOCK_END
}
