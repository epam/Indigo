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

int osTlsAlloc (TLS_IDX_TYPE* key)
{         
   return !pthread_key_create(key, NULL);
}

int osTlsFree (TLS_IDX_TYPE key)
{
   return pthread_key_delete(key);
}

int osTlsSetValue (TLS_IDX_TYPE key, void* value)
{
   return !pthread_setspecific(key, value);
}

int osTlsGetValue (void** value, TLS_IDX_TYPE key)
{
   *value = pthread_getspecific(key);
   return *value != NULL;
}

//#if sizeof(pthread_t) > 4
//#error 'pthread_t' - type is too large. Should be 4 bytes long or less.
//#else

qword osGetThreadID (void)
{                                
   return (qword)pthread_self();
}

//#endif
