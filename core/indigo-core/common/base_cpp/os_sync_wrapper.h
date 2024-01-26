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

#ifndef __os_sync_wrapper_h__
#define __os_sync_wrapper_h__

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <condition_variable>
#include <mutex>

#include "base_c/defs.h"
#include "base_cpp/exception.h"

namespace indigo
{

    //
    // Semaphore
    //
    class DLLEXPORT OsSemaphore
    {
    public:
        OsSemaphore(int initial_count, int max_count);
        ~OsSemaphore();

        void Wait();
        void Post();

    private:
        std::mutex _mutex;
        std::condition_variable _cond;
        int _count;
        const int _max_count;
    };

    //
    // Message system
    //

    class OsMessageSystem
    {
    public:
        OsMessageSystem();

        void SendMsg(int message, void* param = 0);
        void RecvMsg(int* message, void** result = 0);

    private:
        OsSemaphore _sendSem;
        OsSemaphore _finishRecvSem;

        std::mutex _sendLock;
        std::mutex _recvLock;

        volatile int _localMessage;
        void* volatile _localParam;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // __os_sync_wrapper_h__
