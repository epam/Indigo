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
// Semaphore
//
OsSemaphore::OsSemaphore(int initial_count, int max_count) : _max_count{max_count < 1 ? 1 : max_count}
{
    _count = initial_count < 0 ? 0 : initial_count > _max_count ? _max_count : initial_count;
}

OsSemaphore::~OsSemaphore()
{
}

void OsSemaphore::Wait()
{
    std::unique_lock<std::mutex> lock{_mutex};

    _cond.wait(lock, [this]() { return _count > 0; });

    --_count;
}

void OsSemaphore::Post()
{
    std::unique_lock<std::mutex> lock{_mutex};

    if (_count < _max_count)
    {
        ++_count;
        _cond.notify_one();
    }
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
