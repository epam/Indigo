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

#ifndef __os_sync_h__
#define __os_sync_h__

#include "base_c/defs.h"

#ifndef _WIN32
#include <pthread.h>
#ifdef __APPLE__
#include <mach/mach_init.h>
#include <mach/semaphore.h>
#include <mach/task.h>
#else
#include <semaphore.h>
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    // Cross-platform mutex support
    typedef struct tag_os_mutex
    {
#ifdef _WIN32
        void* data;
#else
    pthread_mutex_t data;
#endif
    } os_mutex;

    DLLEXPORT void osMutexCreate(os_mutex* mutex);
    DLLEXPORT void osMutexDelete(os_mutex* mutex);
    DLLEXPORT void osMutexLock(os_mutex* mutex);
    DLLEXPORT void osMutexUnlock(os_mutex* mutex);

    //
    // Semaphore
    //
    typedef struct tag_os_semaphore
    {
#ifdef _WIN32
        void* data;
#elif __APPLE__
    semaphore_t data;
#else
    sem_t data;
#endif
    } os_semaphore;

    DLLEXPORT void osSemaphoreCreate(os_semaphore* sem, int initial_count, int max_count);
    DLLEXPORT void osSemaphoreDelete(os_semaphore* sem);
    DLLEXPORT void osSemaphoreWait(os_semaphore* sem);
    DLLEXPORT void osSemaphorePost(os_semaphore* sem);

#ifdef __cplusplus
}
#endif

#endif // __os_sync_h__
