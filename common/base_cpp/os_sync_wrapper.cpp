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

#include "base_cpp/os_sync_wrapper.h"

using namespace indigo;

//
// osLock
//

OsLock::OsLock ()
{
   osMutexCreate(&_mutex);
}

OsLock::~OsLock ()
{
   osMutexDelete(&_mutex);
}

void OsLock::Lock ()
{
   osMutexLock(&_mutex);
}

void OsLock::Unlock ()
{
   osMutexUnlock(&_mutex);
}

//
// Semaphore wrapper
//
OsSemaphore::OsSemaphore  (int initial_count, int max_count)
{
   osSemaphoreCreate(&_sem, initial_count, max_count);
}

OsSemaphore::~OsSemaphore ()
{
   osSemaphoreDelete(&_sem);
}

void OsSemaphore::Wait  ()
{
   osSemaphoreWait(&_sem);
}

void OsSemaphore::Post ()
{
   osSemaphorePost(&_sem);
}

//
// Message system
//

void OsMessageSystem::SendMsg (int message, void *param)
{
   OsLocker locker(_sendLock);

   _localMessage = message;
   _localParam = param;

   _sendSem.Post();
   _finishRecvSem.Wait();
}

void OsMessageSystem::RecvMsg (int *message, void **result)
{
   OsLocker locker(_recvLock);

   // Wait for sending
   _sendSem.Wait();

   *message = _localMessage;
   if (result != NULL)
      *result = _localParam;

   _finishRecvSem.Post();
}

OsMessageSystem::OsMessageSystem() :
   _sendSem(0, 1), _finishRecvSem(0, 1)
{
}

//
// ThreadSafeStaticObj
//

namespace indigo
{
DLLEXPORT OsLock & osStaticObjConstructionLock ()
{
   static OsLock _static_obj_construction_lock;
   return _static_obj_construction_lock;
}
}
