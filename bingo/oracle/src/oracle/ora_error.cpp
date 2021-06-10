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

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <nzt.h>
#include <oci.h>

// oci.h on Solaris has typedef-ed dword.
// We define it in order to avoid conflict with base_c/defs.h
#define dword unsigned int

#include "oracle/ora_logger.h"
#include "oracle/ora_wrap.h"

#include "base_c/defs.h"

using namespace indigo;

OracleError::OracleError(OCIError* errhp, int oracle_rc, const char* message, int my_rc) : Exception("bingo-oracle: ")
{
    if (oracle_rc == OCI_NO_DATA)
        snprintf(_message, sizeof(_message), "%s: no data", message);
    else if (oracle_rc == OCI_NEED_DATA)
        snprintf(_message, sizeof(_message), "%s: need data", message);
    else if (oracle_rc == OCI_INVALID_HANDLE)
        snprintf(_message, sizeof(_message), "%s: invalid handle", message);
    else if (oracle_rc == OCI_ERROR)
    {
        if (errhp != 0)
        {
            text errbuf[512];

            OCIErrorGet(errhp, 1, NULL, &oracle_rc, errbuf, (ub4)sizeof(errbuf), OCI_HTYPE_ERROR);
            snprintf(_message, sizeof(_message), "%s: %.*s\n", message, 512, errbuf);
        }
        else
            snprintf(_message, sizeof(_message), "(can not get the error message because errhp is null)\n");
    }
    _code = my_rc;
}

OracleError::OracleError(int my_rc, const char* format, ...) : Exception("bingo-oracle: ")
{
    va_list args;
    int n;

    va_start(args, format);
    n = vsnprintf(_message, sizeof(_message), format, args);
    va_end(args);
    _code = my_rc;
}

void OracleError::raise(OracleLogger& logger, OCIExtProcContext* ctx)
{
    logger.dbgPrintf("%s\n", _message);
    if (_code == -1)
    {
        if (OCIExtProcRaiseExcpWithMsg(ctx, 20352, (text*)_message, 0) == OCIEXTPROC_ERROR)
            logger.dbgPrintf("Error raising OCI exception\n");
    }
    else
    {
        if (OCIExtProcRaiseExcpWithMsg(ctx, 20355, (text*)_message, 0) == OCIEXTPROC_ERROR)
            logger.dbgPrintf("Error raising OCI exception\n");
    }
}
