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

#ifndef __os_thread_wrapper_h__
#define __os_thread_wrapper_h__

//
// Thread wrapper based on command dispatcher:
// 1. osCommandDispatcher setups a osCommand
// 2. osCommand is executed (in the another thread) and
//    result is stored in the osCommandResult object.
// 3. osCommandDispatcher handles osCommandResult result in
//    the main thread.
//
// There is two options to handle results:
// OsCommandDispatcher::HANDLING_ORDER_ANY -
//
// Session ID for each thread, created by dispatcher, is
// released before thread exit automatically via TL_RELEASE_SESSION_ID.
//
// Note: OsCommand and OsCommandResult objects are reusable,
// so they shouldn't have specific parameters in
// constructors if the same dispatcher object is
// used many time.
//

#include "base_c/defs.h"
#include "base_cpp/array.h"
#include "base_cpp/cyclic_array.h"
#include "base_cpp/os_sync_wrapper.h"
#include "base_cpp/ptr_array.h"

namespace indigo
{

    class OsCommandResult
    {
    public:
        virtual ~OsCommandResult(){};
        virtual void clear(){};
    };

    class OsCommand
    {
    public:
        virtual ~OsCommand(){};
        virtual void clear(){};
        virtual void execute(OsCommandResult& result) = 0;

        int unique_id;
    };

    class Exception;

    class OsCommandDispatcher
    {
    public:
        enum
        {
            HANDLING_ORDER_ANY,
            HANDLING_ORDER_SERIAL
        };

        OsCommandDispatcher(int handling_order, bool same_session_IDs);
        virtual ~OsCommandDispatcher(){};

        void run();
        void run(int nthreads);

        void terminate();
        void markToTerminate();

        void _threadFunc(void);

    protected:
        void _run(int nthreads);

        //  For overloading
        virtual OsCommand* _allocateCommand() = 0;
        virtual OsCommandResult* _allocateResult();

        virtual bool _setupCommand(OsCommand& command) = 0;
        virtual void _handleResult(OsCommandResult& result)
        {
        }

        // Callback function to initialize thread-local variables
        // Custom Session ID can be set in this callback function.
        virtual void _prepareThread(void)
        {
        }
        // Callback function to cleanup thread-local variables
        virtual void _cleanupThread(void)
        {
        }

    private:
        // Methods
        void _startStandalone();

        void _onMsgNeedTask();
        void _onMsgHandleResult();
        void _onMsgHandleException(Exception* exception);

        void _recvCommandAndResult(OsCommandResult*& result, OsCommand*& command);

        void _mainLoop();

        OsCommand* _getVacantCommand();
        OsCommandResult* _getVacantResult();

        void _handleException(Exception* exception);
        void _handleResultWithCheck(OsCommandResult* result);

        void _wakeSuspended();

    private:
        // Variables
        PtrArray<OsCommand> _availableCommands;
        PtrArray<OsCommandResult> _availableResults;
        CyclicArray<OsCommandResult*> _storedResults;
        Array<OsSemaphore*> _syspendedThreads;

        Exception* _exception_to_forward;

        OsMessageSystem _baseMessageSystem;
        OsMessageSystem _privateMessageSystem;

        int _last_command_index;
        int _expected_command_index;
        int _handling_order;
        int _left_thread_count;
        bool _need_to_terminate;
        qword _session_id;
        int _last_unique_command_id;

        bool _same_session_IDs;
        qword _parent_session_ID;
    };

} // namespace indigo

#endif // __cmd_thread_h__
