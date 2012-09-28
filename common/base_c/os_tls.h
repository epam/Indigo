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

#ifndef __os_tls_h__
#define __os_tls_h__

#ifdef _WIN32
   typedef int TLS_IDX_TYPE;
#else
#include <pthread.h>
   typedef pthread_key_t TLS_IDX_TYPE;
#endif

#ifdef __cplusplus
extern "C" {
#endif

int   osTlsAlloc    (TLS_IDX_TYPE* key);
int   osTlsFree     (TLS_IDX_TYPE key);
int   osTlsSetValue (TLS_IDX_TYPE key, void* value);
int   osTlsGetValue (void** value, TLS_IDX_TYPE key);
qword   osGetThreadID (void);

#ifdef __cplusplus
}
#endif


#endif /*__os_tls_h__*/
