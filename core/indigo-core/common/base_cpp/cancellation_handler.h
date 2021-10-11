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

#ifndef __cancellation_handler_h__
#define __cancellation_handler_h__

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include <memory>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT CancellationHandler
    {
    public:
        virtual bool isCancelled() = 0;
        virtual const char* cancelledRequestMessage() = 0;
        virtual ~CancellationHandler() = default;
    };

    class DLLEXPORT TimeoutCancellationHandler : public CancellationHandler
    {
    public:
        explicit TimeoutCancellationHandler(int mseconds = 0);
        ~TimeoutCancellationHandler() override;

        bool isCancelled() override;
        const char* cancelledRequestMessage() override;

        void reset(int mseconds);

    private:
        Array<char> _message;
        int _mseconds;
        qword _currentTime;
    };

    // Global thread-local cancellation handler
    DLLEXPORT CancellationHandler* getCancellationHandler();
    // Returns previous cancellation handler.
    // TAKES Ownership!!!
    DLLEXPORT std::unique_ptr<CancellationHandler> resetCancellationHandler(CancellationHandler* handler);
    void createCancellationHandler(qword id);

    class AutoCancellationHandler
    {
    public:
        AutoCancellationHandler(CancellationHandler*);
        ~AutoCancellationHandler();
    };
} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif /* __cancellation_handler_h__ */

/* END OF 'cancellation_handler.H' FILE */
