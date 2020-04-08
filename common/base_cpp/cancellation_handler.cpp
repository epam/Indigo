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

#include "base_cpp/cancellation_handler.h"

#include "base_c/nano.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"

namespace indigo
{
    //
    // TimeoutCancellationHandler
    //

    TimeoutCancellationHandler::TimeoutCancellationHandler(int mseconds)
    {
        reset(mseconds);
    }

    TimeoutCancellationHandler::~TimeoutCancellationHandler()
    {
    }

    bool TimeoutCancellationHandler::isCancelled()
    {
        qword dif_time = nanoClock() - _currentTime;
        if (_mseconds > 0 && nanoHowManySeconds(dif_time) * 1000 > _mseconds)
        {
            ArrayOutput mes_out(_message);
            mes_out.printf("The operation timed out: %d ms", _mseconds);
            mes_out.writeChar(0);
            return true;
        }
        return false;
    }

    const char* TimeoutCancellationHandler::cancelledRequestMessage()
    {
        return _message.ptr();
    }

    void TimeoutCancellationHandler::reset(int mseconds)
    {
        _mseconds = mseconds;
        _currentTime = nanoClock();
    }

    //
    // Global thread-local cancellation handler
    //

    class CancellationHandlerWrapper
    {
    public:
        CancellationHandlerWrapper() : handler(nullptr)
        {
        }

        std::unique_ptr<CancellationHandler> handler;
    };

    static _SessionLocalContainer<CancellationHandlerWrapper> cancellation_handler;

    CancellationHandler* getCancellationHandler()
    {
        CancellationHandlerWrapper& wrapper = cancellation_handler.getLocalCopy();
        return wrapper.handler.get();
    }

    std::unique_ptr<CancellationHandler> resetCancellationHandler(CancellationHandler* handler)
    {
        CancellationHandlerWrapper& wrapper = cancellation_handler.getLocalCopy();
        std::unique_ptr<CancellationHandler> prev(wrapper.handler.release());
        wrapper.handler.reset(handler);
        return prev;
    }

    AutoCancellationHandler::AutoCancellationHandler(CancellationHandler* hand)
    {
        resetCancellationHandler(hand);
    }

    AutoCancellationHandler::~AutoCancellationHandler()
    {
        resetCancellationHandler(nullptr);
    }

} // namespace indigo
