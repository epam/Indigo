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
// Synchronization primitives support for POSIX
//

#include "base_c/os_sync.h"

#include <pthread.h>

//
// Mutex
//

void osMutexCreate(os_mutex* mutex)
{
    pthread_mutex_init((pthread_mutex_t*)mutex, NULL);
}

void osMutexDelete(os_mutex* mutex)
{
    pthread_mutex_destroy((pthread_mutex_t*)mutex);
}

void osMutexLock(os_mutex* mutex)
{
    pthread_mutex_lock((pthread_mutex_t*)mutex);
}

void osMutexUnlock(os_mutex* mutex)
{
    pthread_mutex_unlock((pthread_mutex_t*)mutex);
}

//
// Semaphore
//

void osSemaphoreCreate(os_semaphore* sem, int initial_count, int max_count)
{
#ifdef __APPLE__
    semaphore_create(mach_task_self(), &sem->data, SYNC_POLICY_FIFO, initial_count);
#else
    sem_init(&sem->data, 0, initial_count);
#endif
}

void osSemaphoreDelete(os_semaphore* sem)
{
#ifdef __APPLE__
    semaphore_destroy(mach_task_self(), sem->data);
#else
    sem_destroy(&sem->data);
#endif
}

void osSemaphoreWait(os_semaphore* sem)
{
#ifdef __APPLE__
    semaphore_wait(sem->data);
#else
    sem_wait(&sem->data);
#endif
}

void osSemaphorePost(os_semaphore* sem)
{
#ifdef __APPLE__
    semaphore_signal(sem->data);
#else
    sem_post(&sem->data);
#endif
}
