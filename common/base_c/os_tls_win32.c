/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#include "base_c/defs.h"
#include "base_c/os_tls.h"

#ifdef _WIN32

#include <windows.h>

int osTlsAlloc (TLS_IDX_TYPE* key)
{     
   if ((*key = TlsAlloc()) == TLS_OUT_OF_INDEXES) {
      return FALSE;
   }
   return TRUE;
}

int osTlsFree (TLS_IDX_TYPE key)
{
   return !!TlsFree(key);
}

int osTlsSetValue (TLS_IDX_TYPE key, void* value)
{
   return !!TlsSetValue(key, value);
}

int osTlsGetValue (void** value, TLS_IDX_TYPE key)
{     
   *value = TlsGetValue(key);
   return *value != NULL;
}

qword osGetThreadID (void)
{
   return (qword)GetCurrentThreadId();
}

#endif
