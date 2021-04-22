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

#ifdef _WIN32

#include <windows.h>

int osTlsAlloc(TLS_IDX_TYPE* key)
{
    if ((*key = TlsAlloc()) == TLS_OUT_OF_INDEXES)
    {
        return FALSE;
    }
    return TRUE;
}

int osTlsFree(TLS_IDX_TYPE key)
{
    return !!TlsFree(key);
}

int osTlsSetValue(TLS_IDX_TYPE key, void* value)
{
    return !!TlsSetValue(key, value);
}

int osTlsGetValue(void** value, TLS_IDX_TYPE key)
{
    *value = TlsGetValue(key);
    return *value != NULL;
}

qword osGetThreadID(void)
{
    return (qword)GetCurrentThreadId();
}

#endif
