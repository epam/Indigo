/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include <oci.h>
#include <nzt.h>

// oci.h on Solaris has typedef-ed dword.
// We define it in order to avoid conflict with base_c/defs.h
#define dword unsigned int

#include "oracle/ora_wrap.h"
#include "oracle/ora_logger.h"

#include "base_c/defs.h"

using namespace indigo;

OracleError::OracleError (OCIError *errhp, int oracle_rc, const char *message, int my_rc)
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
   _my_rc = my_rc;
}

OracleError::OracleError (int my_rc, const char *format, ...)
{
   va_list args;
   int n;
  
   va_start(args, format);
   n = vsnprintf(_message, sizeof(_message), format, args);
   va_end(args);
   _my_rc = my_rc;
}

OracleError::~OracleError ()
{
}

void OracleError::raise (OracleLogger &logger, OCIExtProcContext *ctx)
{
   logger.dbgPrintf("%s\n", _message);
   if (_my_rc == -1)
   {
      if (OCIExtProcRaiseExcpWithMsg(ctx, 20352, (text *)_message, 0) == OCIEXTPROC_ERROR)
         logger.dbgPrintf("Error raising OCI exception\n");
   } else {
      if (OCIExtProcRaiseExcpWithMsg(ctx, 20355, (text *)_message, 0) == OCIEXTPROC_ERROR)
         logger.dbgPrintf("Error raising OCI exception\n");
   }
}
