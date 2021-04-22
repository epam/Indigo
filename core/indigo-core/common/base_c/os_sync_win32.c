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

//
// Synchronization primitives support for WIN32
//

#include <stdlib.h>
#include <windows.h>

#include "base_c/os_sync.h"

//
// Mutex
//

void osMutexCreate(os_mutex* mutex)
{
    mutex->data = malloc(sizeof(CRITICAL_SECTION));
    InitializeCriticalSection((CRITICAL_SECTION*)mutex->data);
}

void osMutexDelete(os_mutex* mutex)
{
    DeleteCriticalSection((CRITICAL_SECTION*)mutex->data);
    free(mutex->data);
    mutex->data = NULL;
}

void osMutexLock(os_mutex* mutex)
{
    if (mutex->data == NULL)
        return;
    EnterCriticalSection((CRITICAL_SECTION*)mutex->data);
}

void osMutexUnlock(os_mutex* mutex)
{
    if (mutex->data == NULL)
        return;
    LeaveCriticalSection((CRITICAL_SECTION*)mutex->data);
}

//
// Semaphore
//

void osSemaphoreCreate(os_semaphore* sem, int initial_count, int max_count)
{
    sem->data = malloc(sizeof(HANDLE));
    *(HANDLE*)sem->data = CreateSemaphore(NULL, initial_count, max_count, NULL);
}

void osSemaphoreDelete(os_semaphore* sem)
{
    CloseHandle(*(HANDLE*)sem->data);
    free(sem->data);
}

void osSemaphoreWait(os_semaphore* sem)
{
    WaitForSingleObject(*(HANDLE*)sem->data, INFINITE);
}

void osSemaphorePost(os_semaphore* sem)
{
    ReleaseSemaphore(*(HANDLE*)sem->data, 1, NULL);
}
