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

#ifndef __os_sync_h__
#define __os_sync_h__

#include "base_c/defs.h"

#ifndef _WIN32
#include <pthread.h>
#ifdef __APPLE__
#include <mach/semaphore.h>
#include <mach/task.h>
#else
#include <semaphore.h>
#endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Cross-platform mutex support
typedef struct tag_os_mutex
{
#ifdef _WIN32
   void *data;
#else
   pthread_mutex_t data;
#endif
} os_mutex;

DLLEXPORT void osMutexCreate (os_mutex *mutex);
DLLEXPORT void osMutexDelete (os_mutex *mutex);
DLLEXPORT void osMutexLock   (os_mutex *mutex);
DLLEXPORT void osMutexUnlock (os_mutex *mutex);

//
// Semaphore
//
typedef struct tag_os_semaphore
{
#ifdef _WIN32
   void *data;
#elif __APPLE__
   semaphore_t data;
#else
   sem_t data;
#endif
} os_semaphore;

void osSemaphoreCreate  (os_semaphore *sem, int initial_count, int max_count);
void osSemaphoreDelete  (os_semaphore *sem);
void osSemaphoreWait    (os_semaphore *sem);
void osSemaphorePost    (os_semaphore *sem);

#ifdef __cplusplus
}
#endif

#endif // __os_sync_h__

