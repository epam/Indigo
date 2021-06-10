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

#ifndef __bingo_core_c_parallel_h___
#define __bingo_core_c_parallel_h___

#include "base_cpp/chunk_storage.h"
#include "base_cpp/os_thread_wrapper.h"

// Helper classes for parallelized indexing

class BingoIndex;

namespace indigo
{
    namespace bingo_core
    {

        class BingoCore;
        class IndexingCommandResult;

        // This command contains pack of the binary data
        // (molecules or reactions in the raw format)
        class IndexingCommand : public OsCommand
        {
        public:
            void execute(OsCommandResult& result) override;
            void clear() override;

            // Molecules or reactions
            ChunkStorage records;
            // Array of their internal indices
            ArrayInt ids;

            BingoCore* core;
            OsLock* lock_for_exclusive_access;
        };

        class IndexingCommandResult : public OsCommandResult
        {
        public:
            void clear() override;

            virtual BingoIndex& getIndex(int index) = 0;

            // Array of processed indices
            ArrayInt ids;

            // Array with error messages
            ChunkStorage error_messages;
            ArrayInt error_ids;
        };

        // Dispatcher for creating commands.
        // Subclasses should override _handleResult for result
        // handling (if necessary)
        // Each thread has the same Session ID as parent thread.
        class IndexingDispatcher : public OsCommandDispatcher
        {
        public:
            // Parameters:
            //    method - HANDLING_ORDER_***
            //    set_parent_SID_for_threads - true if set SID for all
            //       child threads to the parent' SID
            IndexingDispatcher(BingoCore& core, int method, bool set_parent_SID_for_threads, int records_per_command);

            void* context;
            // If this function returns 0, then data is finished
            int (*get_next_record_cb)(void* context);
            void (*process_result_cb)(void* context);
            void (*process_error_cb)(int id, void* context);
            bool _finished;

        protected:
            // This method should be overridden to setup current processed record so
            // it can be processed in process_result_cb callback.
            virtual void _exposeCurrentResult(int index, IndexingCommandResult& result) = 0;

        protected:
            BingoCore& _core;

        private:
            OsCommand* _allocateCommand() override;

            bool _setupCommand(OsCommand& command) override;
            void _handleResult(OsCommandResult& result) override;

            int _records_per_command;
            OsLock _lock_for_exclusive_access;
        };

    } // namespace bingo_core
} // namespace indigo

#endif // __bingo_core_c_parallel_h___
