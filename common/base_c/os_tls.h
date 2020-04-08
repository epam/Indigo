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

#ifndef __os_tls_h__
#define __os_tls_h__

#ifdef _WIN32
typedef int TLS_IDX_TYPE;
#else
#include <pthread.h>
typedef pthread_key_t TLS_IDX_TYPE;
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    int osTlsAlloc(TLS_IDX_TYPE* key);
    int osTlsFree(TLS_IDX_TYPE key);
    int osTlsSetValue(TLS_IDX_TYPE key, void* value);
    int osTlsGetValue(void** value, TLS_IDX_TYPE key);
    qword osGetThreadID(void);

#ifdef __cplusplus
}
#endif

#endif /*__os_tls_h__*/
