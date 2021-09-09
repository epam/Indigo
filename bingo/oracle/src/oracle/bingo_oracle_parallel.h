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

#ifndef __bingo_parallel_h__
#define __bingo_parallel_h__

#include "base_cpp/chunk_storage.h"
#include "base_cpp/os_thread_wrapper.h"

// Helper class for parallelizing algorithms with
// fetching packs of BLOBs or CLOBS from the database

using namespace indigo;

namespace indigo
{
    class OracleStatement;
    class OracleLOB;
    class OracleEnv;
} // namespace indigo

// Base class for command.
// This command contains pack of the blobs (molecules in the raw format)
// Subclasses should have additional data per each blob record from
// blob_storage.
class BingoOracleCommand : public OsCommand
{
public:
    virtual void execute(OsCommandResult& result) = 0;
    void clear() override;

    ChunkStorage blob_storage;
};

// Dispatcher for creating commands.
// Subclasses should overload _handleResult for result
// handling (if necessary)
// Each thread has the same Session ID as parent thread.
class BingoOracleDispatcher : public OsCommandDispatcher
{
public:
    // Parameters:
    //    method - HANDLING_ORDER_***
    //    set_parent_SID_for_threads - true if set SID for all
    //       child threads to the parent' SID
    BingoOracleDispatcher(int method, bool set_parent_SID_for_threads, int blobs_per_command);

    void setup(OracleStatement* statement, OracleLOB* lob, char* varchar2_text, bool read_from_LOB);

protected:
    bool _setupCommand(OsCommand& command) override;

    // This function is call _blobs_per_command times to
    // initialize additional data for each molecule
    virtual void _addCurrentRecordToCommand(BingoOracleCommand& cmd);

protected:
    OracleStatement* _statement;
    OracleLOB* _lob;
    char* _varchar2_text;
    bool _read_from_LOB;

    bool _finished;
    int _blobs_per_command;
};

#endif // __bingo_parallel_h__
