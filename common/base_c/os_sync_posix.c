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

//
// Synchronization primitives support for POSIX
//

#include "base_c/os_sync.h"

#include <pthread.h>

//
// Mutex
//

void osMutexCreate (os_mutex *mutex)
{
   pthread_mutex_init((pthread_mutex_t *)mutex, NULL);
}

void osMutexDelete (os_mutex *mutex)
{
   pthread_mutex_destroy((pthread_mutex_t *)mutex);
}

void osMutexLock   (os_mutex *mutex)
{
   pthread_mutex_lock((pthread_mutex_t *)mutex);
}

void osMutexUnlock (os_mutex *mutex)
{
   pthread_mutex_unlock((pthread_mutex_t *)mutex);
}

//
// Semaphore
//

void osSemaphoreCreate (os_semaphore *sem, int initial_count, int max_count)
{
#ifdef __APPLE__
   semaphore_create(mach_task_self(), &sem->data, SYNC_POLICY_FIFO, initial_count);
#else
   sem_init(&sem->data, 0, initial_count);
#endif
}

void osSemaphoreDelete (os_semaphore *sem)
{
#ifdef __APPLE__
   semaphore_destroy(mach_task_self(), sem->data);
#else
   sem_destroy(&sem->data);
#endif
}

void osSemaphoreWait (os_semaphore *sem)
{
#ifdef __APPLE__
   semaphore_wait(sem->data);
#else
   sem_wait(&sem->data);
#endif
}

void osSemaphorePost (os_semaphore *sem)
{
#ifdef __APPLE__
   semaphore_signal(sem->data);
#else
   sem_post(&sem->data);
#endif
}

