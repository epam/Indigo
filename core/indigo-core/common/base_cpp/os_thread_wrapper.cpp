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

//
// Thread support based on command dispatcher:
//

#ifdef _WIN32
#include <windows.h>
#endif
#ifdef __APPLE__
#include <sys/sysctl.h>
#endif

#include "base_cpp/exception.h"
#include "base_cpp/os_thread_wrapper.h"
#include "base_cpp/profiling.h"
#include "base_cpp/tlscont.h"
#include <memory>
#include <thread>

using namespace indigo;

// Messages
enum
{
    MSG_NEED_TASK,
    MSG_RECV_COMMAND,
    MSG_RECV_RESULT,
    MSG_RECV_INDEX,
    MSG_NO_TASK,
    MSG_HANDLE_RESULT,
    MSG_SYSPEND_RESULT,
    MSG_SYSPEND_SEMAPHORE,
    MSG_OK,
    MSG_CONNECT,
    MSG_HANDLE_EXCEPTION,
    MSG_TERMINATE
};

// Maximum number of results that are kept in queue if
// _handling_order is HANDLING_ORDER_SERIAL
static const int _MAX_RESULTS = 1000;

OsCommandDispatcher::OsCommandDispatcher(int handling_order, bool same_session_IDs)
{
    _storedResults.setSize(_MAX_RESULTS);
    _storedResults.zeroFill();
    _handling_order = handling_order;
    _session_id = TL_GET_SESSION_ID();
    _last_unique_command_id = 0;
    _same_session_IDs = same_session_IDs;
}

void OsCommandDispatcher::run()
{
    _run(3 * std::thread::hardware_concurrency() / 2 + 1);
}

void OsCommandDispatcher::run(int nthreads)
{
    if (nthreads < 0)
        // Use automatic thread count selection
        run();
    else
        _run(nthreads);
}

void OsCommandDispatcher::_run(int nthreads)
{
    _last_command_index = 0;
    _expected_command_index = 0;
    _need_to_terminate = false;
    _exception_to_forward = NULL;

    _left_thread_count = nthreads;

    if (_left_thread_count == 0)
    {
        _startStandalone();
        return;
    }

    _parent_session_ID = TL_GET_SESSION_ID();

    // Create handling threads
    for (int i = 0; i < _left_thread_count; i++)
    {
        std::thread thread{[this]() { this->_threadFunc(); }};
        thread.detach();
    }

    _mainLoop();
}

void OsCommandDispatcher::_mainLoop()
{
    profTimerStart(t, "dispatcher.main_loop");

    // Main loop
    int msg;
    while (_left_thread_count != 0)
    {
        void* parameter;
        _baseMessageSystem.RecvMsg(&msg, &parameter);

        if (msg == MSG_NEED_TASK)
            _onMsgNeedTask();
        if (msg == MSG_HANDLE_RESULT)
            _onMsgHandleResult();
        if (msg == MSG_HANDLE_EXCEPTION)
            _onMsgHandleException((Exception*)parameter);
    }

    if (_exception_to_forward != NULL)
    {
        Exception* cur = _exception_to_forward;
        _exception_to_forward = NULL;
        throw *cur;
    }
}

void OsCommandDispatcher::markToTerminate()
{
    _need_to_terminate = true;
}

void OsCommandDispatcher::terminate()
{
    markToTerminate();
    _mainLoop();
}

OsCommand* OsCommandDispatcher::_getVacantCommand()
{
    OsCommand* command = nullptr;
    if (_availableCommands.size() == 0)
    {
        command = _allocateCommand();
        command->unique_id = _last_unique_command_id++;
    }
    else
        command = _availableCommands.pop();

    command->clear();

    return command;
}

OsCommandResult* OsCommandDispatcher::_getVacantResult()
{
    OsCommandResult* result;

    if (_availableResults.size() == 0)
        result = _allocateResult();
    else
        result = _availableResults.pop();
    result->clear();

    return result;
}

void OsCommandDispatcher::_onMsgNeedTask()
{
    if (_need_to_terminate)
    {
        _privateMessageSystem.SendMsg(MSG_NO_TASK, NULL);
        _left_thread_count--;
        return;
    }

    OsCommandResult* result = _getVacantResult();
    OsCommand* command = _getVacantCommand();

    if (!_setupCommand(*command))
    {
        _availableResults.add(result);
        _availableCommands.add(command);

        _privateMessageSystem.SendMsg(MSG_NO_TASK, NULL);
        _left_thread_count--;
        return;
    }
    _privateMessageSystem.SendMsg(MSG_RECV_INDEX, &_last_command_index);
    _privateMessageSystem.SendMsg(MSG_RECV_COMMAND, command);
    _privateMessageSystem.SendMsg(MSG_RECV_RESULT, result);

    _last_command_index++;
}

void OsCommandDispatcher::_handleResultWithCheck(OsCommandResult* result)
{
    Exception* exception = 0;
    try
    {
        if (!_need_to_terminate)
            _handleResult(*result);
    }
    catch (Exception& e)
    {
        exception = new Exception(e);
    }
    catch (...)
    {
        exception = new Exception("Unknown exception");
    }
    if (exception != NULL)
        _handleException(exception);
}

void OsCommandDispatcher::_onMsgHandleResult()
{
    OsCommandResult* result = nullptr;
    OsCommand* command = nullptr;
    int index, msg;
    void* param;

    _privateMessageSystem.RecvMsg(&msg, &param);
    if (msg != MSG_RECV_INDEX)
        throw Exception("cmdDispatcher::_OnMsgHandleResult: internal error");
    index = *(int*)param;

    if (_handling_order == HANDLING_ORDER_SERIAL)
        if (!_storedResults.isInBound(index))
        {
            profIncCounter("dispatcher.syspend_count", 1);

            _privateMessageSystem.SendMsg(MSG_SYSPEND_RESULT);
            _privateMessageSystem.RecvMsg(&msg, &param);
            if (msg != MSG_SYSPEND_SEMAPHORE)
                throw Exception("cmdDispatcher::_OnMsgHandleResult: internal error #2");

            OsSemaphore* sem = (OsSemaphore*)param;
            _syspendedThreads.push(sem);

            return;
        }

    _privateMessageSystem.SendMsg(MSG_OK);
    _recvCommandAndResult(result, command);
    _availableCommands.add(command);

    if (_handling_order == HANDLING_ORDER_ANY)
    {
        // Handle result in parallel mode
        _handleResultWithCheck(result);
        _availableResults.add(result);
    }
    else
    {
        // Handle results in correct order
        _storedResults[index] = result;
        while (_storedResults[_expected_command_index] != NULL)
        {
            OsCommandResult* current = _storedResults[_expected_command_index];
            _storedResults[_expected_command_index] = NULL;

            _handleResultWithCheck(current);
            _availableResults.add(current);
            _expected_command_index++;
        }
        _storedResults.setOffset(_expected_command_index);

        _wakeSuspended();
    }
}

void OsCommandDispatcher::_wakeSuspended()
{
    // Wake up all syspended threads
    for (int i = 0; i < _syspendedThreads.size(); i++)
        _syspendedThreads[i]->Post();
    _syspendedThreads.clear();
}

void OsCommandDispatcher::_onMsgHandleException(Exception* exception)
{
    OsCommandResult* result = nullptr;
    OsCommand* command = nullptr;

    _recvCommandAndResult(result, command);
    _availableResults.add(result);
    _availableCommands.add(command);

    _handleException(exception);
}

void OsCommandDispatcher::_handleException(Exception* exception)
{
    if (!_need_to_terminate)
    {
        _need_to_terminate = true;
        // Store exception to correct memory deallocation in the next time...
        TL_DECL(std::unique_ptr<Exception>, exception_ptr);
        TL_GET(std::unique_ptr<Exception>, exception_ptr);

        exception_ptr.reset(exception);
        _exception_to_forward = exception;

        _wakeSuspended();
    }
    else
    {
        // This is second exception. Skip it
        delete exception;
    }
}

void OsCommandDispatcher::_threadFunc(void)
{
    qword initial_SID = TL_GET_SESSION_ID();

    if (_same_session_IDs)
        TL_SET_SESSION_ID(_parent_session_ID);

    _prepareThread();

    OsSemaphore syspendSem(0, 1);
    while (true)
    {
        int msg;
        _baseMessageSystem.SendMsg(MSG_NEED_TASK, NULL);

        void* param;
        _privateMessageSystem.RecvMsg(&msg, &param);
        if (msg == MSG_NO_TASK)
            break;
        if (msg != MSG_RECV_INDEX)
            throw Exception("cmdDispatcher::_ThreadFunc: internal error");

        int index;
        index = *(int*)param;

        OsCommandResult* result = nullptr;
        OsCommand* command = nullptr;
        _recvCommandAndResult(result, command);

        Exception* exception = nullptr;
        try
        {
            result->clear();
            command->execute(*result);
        }
        catch (Exception& e)
        {
            exception = new Exception(e);
        }
        catch (...)
        {
            exception = new Exception("Unknown exception");
        }

        if (exception)
        {
            // Forward exception into main thread
            _baseMessageSystem.SendMsg(MSG_HANDLE_EXCEPTION, exception);
            // Send current command and result to add
            // them to the vacant list
            _privateMessageSystem.SendMsg(MSG_RECV_COMMAND, command);
            _privateMessageSystem.SendMsg(MSG_RECV_RESULT, result);
            continue;
        }

        while (true)
        {
            _baseMessageSystem.SendMsg(MSG_HANDLE_RESULT, NULL);

            _privateMessageSystem.SendMsg(MSG_RECV_INDEX, &index);
            _privateMessageSystem.RecvMsg(&msg, &param);

            if (msg == MSG_SYSPEND_RESULT)
            {
                // Enter syspend mode
                _privateMessageSystem.SendMsg(MSG_SYSPEND_SEMAPHORE, &syspendSem);
                syspendSem.Wait();
                continue;
            }
            else if (msg == MSG_OK)
            {
                _privateMessageSystem.SendMsg(MSG_RECV_COMMAND, command);
                _privateMessageSystem.SendMsg(MSG_RECV_RESULT, result);
                break;
            }
            else
                // This should terminate application
                throw Exception("cmdDispatcher::_ThreadFunc: internal error #2");
        }
    }

    _cleanupThread();

    TL_RELEASE_SESSION_ID(initial_SID);
}

void OsCommandDispatcher::_recvCommandAndResult(OsCommandResult*& result, OsCommand*& command)
{
    for (int i = 0; i < 2; i++)
    {
        void* param;
        int msg;

        _privateMessageSystem.RecvMsg(&msg, &param);
        if (msg == MSG_RECV_RESULT)
            result = (OsCommandResult*)param;
        else if (msg == MSG_RECV_COMMAND)
            command = (OsCommand*)param;
        else
            throw Exception("cmdDispatcher::_RecvCommandAndResult");
    }
}

OsCommandResult* OsCommandDispatcher::_allocateResult()
{
    // Create empty results
    return new OsCommandResult;
}

void OsCommandDispatcher::_startStandalone()
{
    OsCommandResult* result = _getVacantResult();
    OsCommand* command = _getVacantCommand();

    while (_setupCommand(*command))
    {
        command->execute(*result);
        _handleResult(*result);

        command->clear();
        result->clear();
    }

    _availableResults.add(result);
    _availableCommands.add(command);
}
