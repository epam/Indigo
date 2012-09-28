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
// Synchronization primitives support for WIN32
//

#include <windows.h>
#include <stdlib.h>

#include "base_c/os_sync.h"

//
// Mutex
//

void osMutexCreate (os_mutex *mutex)
{
   mutex->data = malloc(sizeof(CRITICAL_SECTION));
   InitializeCriticalSection((CRITICAL_SECTION *)mutex->data);
}

void osMutexDelete (os_mutex *mutex)
{
   DeleteCriticalSection((CRITICAL_SECTION *)mutex->data);
   free(mutex->data);
   mutex->data = NULL;
}

void osMutexLock   (os_mutex *mutex)
{
   if (mutex->data == NULL)
      return;
   EnterCriticalSection((CRITICAL_SECTION *)mutex->data);
}

void osMutexUnlock (os_mutex *mutex)
{
   if (mutex->data == NULL)
      return;
   LeaveCriticalSection((CRITICAL_SECTION *)mutex->data);
}

//
// Semaphore
//

void osSemaphoreCreate (os_semaphore *sem, int initial_count, int max_count)
{
   sem->data = malloc(sizeof(HANDLE));
   *(HANDLE *)sem->data = CreateSemaphore(NULL, initial_count, max_count, NULL);
}

void osSemaphoreDelete (os_semaphore *sem)
{
   CloseHandle(*(HANDLE *)sem->data);
   free(sem->data);
}

void osSemaphoreWait (os_semaphore *sem)
{
   WaitForSingleObject(*(HANDLE *)sem->data, INFINITE);
}

void osSemaphorePost (os_semaphore *sem)
{
   ReleaseSemaphore(*(HANDLE *)sem->data, 1, NULL);
}

