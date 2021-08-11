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

#include "base_cpp/os_sync_wrapper.h"

using namespace indigo;

//
// Semaphore wrapper
//
OsSemaphore::OsSemaphore(int initial_count, int max_count)
{
    osSemaphoreCreate(&_sem, initial_count, max_count);
}

OsSemaphore::~OsSemaphore()
{
    osSemaphoreDelete(&_sem);
}

void OsSemaphore::Wait()
{
    osSemaphoreWait(&_sem);
}

void OsSemaphore::Post()
{
    osSemaphorePost(&_sem);
}

//
// Message system
//

void OsMessageSystem::SendMsg(int message, void* param)
{
    std::lock_guard<std::mutex> locker(_sendLock);

    _localMessage = message;
    _localParam = param;

    _sendSem.Post();
    _finishRecvSem.Wait();
}

void OsMessageSystem::RecvMsg(int* message, void** result)
{
    std::lock_guard<std::mutex> locker(_recvLock);

    // Wait for sending
    _sendSem.Wait();

    *message = _localMessage;
    if (result != NULL)
        *result = _localParam;

    _finishRecvSem.Post();
}

OsMessageSystem::OsMessageSystem() : _sendSem(0, 1), _finishRecvSem(0, 1)
{
}

//
// ThreadSafeStaticObj
//

namespace indigo
{
    DLLEXPORT std::mutex& osStaticObjConstructionLock()
    {
        static std::mutex _static_obj_construction_lock;
        return _static_obj_construction_lock;
    }
} // namespace indigo
