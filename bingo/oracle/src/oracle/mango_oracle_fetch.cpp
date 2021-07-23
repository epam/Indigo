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

#include "base_c/nano.h"
#include "base_cpp/profiling.h"
#include "core/mango_matchers.h"
#include "oracle/bingo_oracle.h"
#include "oracle/bingo_oracle_context.h"
#include "oracle/bingo_profiling.h"
#include "oracle/mango_fetch_context.h"
#include "oracle/mango_oracle.h"
#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"

static void _mangoIndexStart(OracleEnv& env, MangoFetchContext& context, const char* oper, const Array<char>& query_buf, OCINumber* p_strt, OCINumber* p_stop,
                             int flags, const char* params)
{
    MangoShadowFetch& shadow_fetch = *context.shadow_fetch;
    MangoFastIndex& fast_index = *context.fast_index;
    if (strcasecmp(oper, "SUB") == 0)
    {
        if (context.substructure.parse(params))
        {
            context.substructure.loadQuery(query_buf);

            int right = bingoGetExactRightPart(env, p_strt, p_stop, flags);

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
        else if (context.tautomer.parseSub(params))
        {
            context.tautomer.loadQuery(query_buf);

            int right = bingoGetExactRightPart(env, p_strt, p_stop, flags);

            if (right == 1)
            {
                fast_index.prepareTautomerSubstructure(env);
                context.fetch_engine = &fast_index;
            }
            else // right == 0
            {
                shadow_fetch.prepareNonTautomerSubstructure(env);
                context.fetch_engine = &shadow_fetch;
            }
        }
        else
            throw BingoError("can't parse parameters: '%s'", params);
    }
    else if (strcasecmp(oper, "SMARTS") == 0)
    {
        context.substructure.loadSMARTS(query_buf);

        int right = bingoGetExactRightPart(env, p_strt, p_stop, flags);

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
    else if (strcasecmp(oper, "EXACT") == 0)
    {
        if (context.tautomer.parseExact(params))
        {
            context.tautomer.loadQuery(query_buf);

            int right = bingoGetExactRightPart(env, p_strt, p_stop, flags);

            shadow_fetch.prepareTautomer(env, right);
            context.fetch_engine = &shadow_fetch;
        }
        else if (context.exact.parse(params))
        {
            context.exact.loadQuery(query_buf);

            int right = bingoGetExactRightPart(env, p_strt, p_stop, flags);

            shadow_fetch.prepareExact(env, right);
            context.fetch_engine = &shadow_fetch;
        }
        else
            throw BingoError("can't parse parameters: '%s'", params);
    }
    else if (strcasecmp(oper, "SIM") == 0)
    {
        context.similarity.setMetrics(params);
        context.similarity.loadQuery(query_buf);

        float bottom = -0.1f;
        float top = 1.1f;

        if (p_strt == 0 && p_stop == 0)
            throw BingoError("no bounds for similarity search");

        if (p_strt != 0)
            bottom = OracleUtil::numberToFloat(env, p_strt);
        if (p_stop != 0)
            top = OracleUtil::numberToFloat(env, p_stop);

        if (flags & 64)
            throw BingoError("exact match not allowed");

        context.similarity.include_bottom = ((flags & 16) != 0);
        context.similarity.include_top = ((flags & 32) != 0);
        context.similarity.bottom = bottom;
        context.similarity.top = top;

        fast_index.prepareSimilarity(env);
        context.fetch_engine = &fast_index;
    }
    else if (strcasecmp(oper, "GROSS") == 0)
    {
        int right = bingoGetExactRightPart(env, p_strt, p_stop, flags);

        MangoGross& instance = context.gross;

        instance.parseQuery(query_buf);

        shadow_fetch.prepareGross(env, right);
        context.fetch_engine = &shadow_fetch;
    }
    else if (strcasecmp(oper, "MASS") == 0)
    {
        float bottom = 0;
        float top = 1e10;

        if (p_strt == 0 && p_stop == 0)
            throw BingoError("no bounds for molecular mass search");

        if (p_strt != 0)
            bottom = OracleUtil::numberToFloat(env, p_strt);
        if (p_stop != 0)
            top = OracleUtil::numberToFloat(env, p_stop);

        context.mass.bottom = bottom;
        context.mass.top = top;

        shadow_fetch.prepareMass(env);
        context.fetch_engine = &shadow_fetch;
    }
    else
        throw BingoError("unknown operator: %s", oper);
}

ORAEXT int oraMangoIndexStart(OCIExtProcContext* ctx, int context_id, const char* oper, short oper_ind, OCILobLocator* query_loc, short query_ind,
                              OCINumber* p_strt, short strt_ind, OCINumber* p_stop, short stop_ind, int flags, const char* params, short params_ind)
{
    ORABLOCK_BEGIN
    {
        profTimerStart(tstart, "fetch_start");

        OracleEnv env(ctx, logger);

        env.dbgPrintfTS("IndexStart... ");

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

        bingoBuildQueryID(env, oper, query_buf, p_strt, p_stop, flags, params, query_id);

        MangoOracleContext& context = MangoOracleContext::get(env, context_id, false);
        MangoFetchContext* fetch_context = MangoFetchContext::findFresh(context_id, query_id);

        if (fetch_context == 0)
        {
            profTimersReset();

            fetch_context = &MangoFetchContext::create(context, query_id);
            _mangoIndexStart(env, *fetch_context, oper, query_buf, p_strt, p_stop, flags, params);
            env.dbgPrintfTS("created fetcher #%d\n", fetch_context->id);
        }
        else
        {
            fetch_context->fresh = false;
            env.dbgPrintfTS("found fresh fetcher #%d\n", fetch_context->id);
        }

        profTimerStop(tstart);

        return fetch_context->id;
    }
    ORABLOCK_END

    return -1;
}

ORAEXT int oraMangoIndexFetch(OCIExtProcContext* ctx, int fetch_id, int maxrows, OCIArray** array, short array_indicator)
{
    profTimerStart(t0, "fetch");

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        env.dbgPrintfTS("oraMangoIndexFetch started\n");
        MangoFetchContext& context = MangoFetchContext::get(fetch_id);

        if (context.fetch_engine == 0)
            throw BingoError("fetch_engine = 0 in oraMangoIndexFetch()");

        BingoFetchEngine& fetch_engine = *context.fetch_engine;

        ORA_TRY_FETCH_BEGIN
        {
            if (maxrows > 100)
                maxrows = 100;

            // can have fetched rowid-s from the selectivity computation phase
            maxrows -= bingoPopRowidsToArray(env, fetch_engine.matched, maxrows, *array);

            if (maxrows > 0 && !fetch_engine.end())
            {
                env.dbgPrintfTS("[fetcher #%d] ", context.id);

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

ORAEXT void oraMangoIndexClose(OCIExtProcContext* ctx, int fetch_id){ORABLOCK_BEGIN{OracleEnv env(ctx, logger);

env.dbgPrintfTS("closing fetch #%d\n", fetch_id);

MangoFetchContext::remove(fetch_id);

// Print statistics
bingoProfilingPrintStatistics(false);
}
ORABLOCK_END
}

ORAEXT OCINumber* oraMangoIndexSelectivity(OCIExtProcContext* ctx, int context_id, const char* oper, short oper_ind, OCILobLocator* query_loc, short query_ind,
                                           OCINumber* p_strt, short strt_ind, OCINumber* p_stop, short stop_ind, int flags, const char* params,
                                           short params_ind, short* return_ind)
{
    profTimersReset();

    *return_ind = OCI_IND_NULL;

    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        env.dbgPrintfTS("IndexSelectivity... ");

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

        MangoOracleContext& context = MangoOracleContext::get(env, context_id, false);

        QS_DEF(Array<char>, query_id);
        QS_DEF(Array<char>, query_buf);
        OracleLOB query_lob(env, query_loc);

        query_lob.readAll(query_buf, false);

        bingoBuildQueryID(env, oper, query_buf, p_strt, p_stop, flags, params, query_id);

        MangoFetchContext* fetch_context = MangoFetchContext::findFresh(context_id, query_id);

        if (fetch_context == 0)
        {
            fetch_context = &MangoFetchContext::create(context, query_id);
            env.dbgPrintf("created fresh fetcher #%d\n", fetch_context->id);
        }
        else
            env.dbgPrintf("found fresh fetcher #%d\n", fetch_context->id);

        _mangoIndexStart(env, *fetch_context, oper, query_buf, p_strt, p_stop, flags, params);

        env.dbgPrintfTS("[fetcher #%d] ", fetch_context->id);
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

ORAEXT void oraMangoIndexCost(OCIExtProcContext* ctx, int context_id, OCINumber* p_sel, short sel_ind, const char* oper, short oper_ind,
                              OCILobLocator* query_loc, short query_ind, OCINumber* p_strt, short strt_ind, OCINumber* p_stop, short stop_ind, int flags,
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

bingoBuildQueryID(env, oper, query_buf, p_strt, p_stop, flags, params, query_id);

MangoOracleContext& context = MangoOracleContext::get(env, context_id, false);
MangoFetchContext* fetch_context = MangoFetchContext::findFresh(context_id, query_id);

if (fetch_context == 0)
{
    fetch_context = &MangoFetchContext::create(context, query_id);
    _mangoIndexStart(env, *fetch_context, oper, query_buf, p_strt, p_stop, flags, params);
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

ORAEXT void oraMangoCollectStatistics(OCIExtProcContext* ctx, int context_id)
{
    ORABLOCK_BEGIN
    {
        OracleEnv env(ctx, logger);

        MangoOracleContext& context = MangoOracleContext::get(env, context_id, false);

        context.shadow_table.analyze(env);
    }
    ORABLOCK_END
}
