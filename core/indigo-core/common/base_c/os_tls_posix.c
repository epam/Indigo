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

#include "base_c/defs.h"
#include "base_c/os_tls.h"

int osTlsAlloc(TLS_IDX_TYPE* key)
{
    return !pthread_key_create(key, NULL);
}

int osTlsFree(TLS_IDX_TYPE key)
{
    return pthread_key_delete(key);
}

int osTlsSetValue(TLS_IDX_TYPE key, void* value)
{
    return !pthread_setspecific(key, value);
}

int osTlsGetValue(void** value, TLS_IDX_TYPE key)
{
    *value = pthread_getspecific(key);
    return *value != NULL;
}

//#if sizeof(pthread_t) > 4
//#error 'pthread_t' - type is too large. Should be 4 bytes long or less.
//#else

qword osGetThreadID(void)
{
    return (qword)pthread_self();
}

//#endif
