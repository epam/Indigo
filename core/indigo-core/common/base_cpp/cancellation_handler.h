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

#pragma once

#include <memory>
#include <string>

#include "base_c/defs.h"

namespace indigo
{
    class DLLEXPORT CancellationHandler
    {
    public:
        CancellationHandler() = default;
        CancellationHandler(CancellationHandler&&) = delete;
        CancellationHandler(const CancellationHandler&) = delete;
        CancellationHandler& operator=(CancellationHandler&&) = delete;
        CancellationHandler& operator=(const CancellationHandler&) = delete;
        virtual ~CancellationHandler() = default;

        virtual bool isCancelled() = 0;
        virtual const char* cancelledRequestMessage() = 0;

        static std::unique_ptr<CancellationHandler>& cancellation_handler();
    };

    class DLLEXPORT TimeoutCancellationHandler : public CancellationHandler
    {
    public:
        TimeoutCancellationHandler() = delete;
        explicit TimeoutCancellationHandler(int mseconds);
        TimeoutCancellationHandler(TimeoutCancellationHandler&&) = delete;
        TimeoutCancellationHandler(const TimeoutCancellationHandler&) = delete;
        TimeoutCancellationHandler& operator=(TimeoutCancellationHandler&&) = delete;
        TimeoutCancellationHandler& operator=(const TimeoutCancellationHandler&) = delete;
        ~TimeoutCancellationHandler() override = default;

        bool isCancelled() override;
        const char* cancelledRequestMessage() override;

        void reset(int mseconds);

    private:
        std::string _message;
        int _mseconds;
        qword _currentTime;
    };

    // Global thread-local cancellation handler
    DLLEXPORT CancellationHandler* getCancellationHandler();

    // Returns previous cancellation handler. TAKES Ownership!!!
    DLLEXPORT std::unique_ptr<CancellationHandler> resetCancellationHandler(CancellationHandler* handler);

    class AutoCancellationHandler
    {
    public:
        AutoCancellationHandler() = delete;
        explicit AutoCancellationHandler(CancellationHandler*);
        AutoCancellationHandler(AutoCancellationHandler&&) = delete;
        AutoCancellationHandler(const AutoCancellationHandler&) = delete;
        AutoCancellationHandler& operator=(AutoCancellationHandler&&) = delete;
        AutoCancellationHandler& operator=(const AutoCancellationHandler&) = delete;
        ~AutoCancellationHandler();
    };
}
