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

#ifndef __bingo_oracle__
#define __bingo_oracle__

#include <oci.h>

// oci.h on Solaris has typedef-ed dword.
// We define it in order to avoid conflict with base_c/defs.h
#define dword unsigned int

#include "base_cpp/list.h"
#include "core/bingo_error.h"
#include "oracle/ora_wrap.h"

#define ORAEXT CEXPORT

using namespace indigo;

extern OracleLogger logger;
extern const char* log_filename;

#define ORABLOCK_BEGIN                                                                                                                                         \
    logger.initIfClosed(log_filename);                                                                                                                         \
    try                                                                                                                                                        \
    {                                                                                                                                                          \
        try

#define ORABLOCK_END                                                                                                                                           \
    catch (Exception & e)                                                                                                                                      \
    {                                                                                                                                                          \
        throw OracleError(-1, "Error: %s", e.message());                                                                                                       \
    }                                                                                                                                                          \
    }                                                                                                                                                          \
    catch (OracleError & error)                                                                                                                                \
    {                                                                                                                                                          \
        error.raise(logger, ctx);                                                                                                                              \
    }                                                                                                                                                          \
    catch (...)                                                                                                                                                \
    {                                                                                                                                                          \
        OracleError(-1, "unknown exception").raise(logger, ctx);                                                                                               \
    }

#define ORA_SAFEBLOCK_BEGIN(name)                                                                                                                              \
    logger.initIfClosed(log_filename);                                                                                                                         \
    bool block_throw_error = false;                                                                                                                            \
    const char* block_name = name;                                                                                                                             \
    try                                                                                                                                                        \
    {                                                                                                                                                          \
        try

#define ORA_SAFEBLOCK_END                                                                                                                                      \
    catch (Exception & e)                                                                                                                                      \
    {                                                                                                                                                          \
        throw OracleError(-1, "Error: %s", e.message());                                                                                                       \
    }                                                                                                                                                          \
    }                                                                                                                                                          \
    catch (OracleError & error)                                                                                                                                \
    {                                                                                                                                                          \
        if (block_throw_error)                                                                                                                                 \
        {                                                                                                                                                      \
            error.raise(logger, ctx);                                                                                                                          \
        }                                                                                                                                                      \
        else                                                                                                                                                   \
        {                                                                                                                                                      \
            logger.dbgPrintfTS("%s: %s\n", block_name, error.message());                                                                                       \
        }                                                                                                                                                      \
    }                                                                                                                                                          \
    catch (...)                                                                                                                                                \
    {                                                                                                                                                          \
        if (block_throw_error)                                                                                                                                 \
        {                                                                                                                                                      \
            OracleError(-1, "unknown exception").raise(logger, ctx);                                                                                           \
        }                                                                                                                                                      \
        else                                                                                                                                                   \
        {                                                                                                                                                      \
            logger.dbgPrintfTS("%s: unknown exception\n", block_name);                                                                                         \
        }                                                                                                                                                      \
    }

#define ORA_TRY_FETCH_BEGIN try

#define ORA_TRY_FETCH_END                                                                                                                                      \
    catch (Exception & e)                                                                                                                                      \
    {                                                                                                                                                          \
        const char* rid_text = "<null>";                                                                                                                       \
        OraRowidText rid;                                                                                                                                      \
        if (fetch_engine.getLastRowid(rid))                                                                                                                    \
            rid_text = rid.ptr();                                                                                                                              \
        throw Exception("%s. Last rowid was %s", e.message(), rid_text);                                                                                       \
    }

int bingoPopRowidsToArray(OracleEnv& env, List<OraRowidText>& matched, int maxrows, OCIArray* array);
int bingoGetExactRightPart(OracleEnv& env, OCINumber* p_strt, OCINumber* p_stop, int flags);
void bingoBuildQueryID(OracleEnv& env, const char* oper, const std::string& query_buf, OCINumber* p_strt, OCINumber* p_stop, int flags, const char* params,
                       std::string& id);

#endif
