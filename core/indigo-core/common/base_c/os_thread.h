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

#ifndef __os_thread_h__
#define __os_thread_h__

//
// Crossplatform thread support
//

#ifdef _WIN32

#define THREAD_RET unsigned long
#define THREAD_MOD __stdcall
#define THREAD_END return 0

#else

#include <pthread.h>

#define THREAD_RET void*
#define THREAD_MOD
#define THREAD_END                                                                                                                                             \
    pthread_exit(NULL);                                                                                                                                        \
    return 0

#endif

#ifdef __cplusplus
extern "C"
{
#endif

    void osThreadCreate(THREAD_RET(THREAD_MOD* func)(void* param), void* param);

#ifdef __cplusplus
}
#endif

#endif // __os_thread_h__
