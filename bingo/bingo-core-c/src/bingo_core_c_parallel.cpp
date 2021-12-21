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

#include "bingo_core_c_parallel.h"
#include "bingo_core_c_internal.h"

#include "mango_core_c_parallel.h"
#include "ringo_core_c_parallel.h"

#include "base_cpp/profiling.h"
#include "molecule/cmf_saver.h"
#include "reaction/crf_saver.h"

using namespace indigo::bingo_core;

CEXPORT int bingoSetIndexRecordData(int id, const char* data, int data_size){BINGO_BEGIN{self.index_record_data->copy(data, data_size);
self.index_record_data_id = id;
}
BINGO_END(0, -1)
}

// Method for index molecules
CEXPORT int bingoIndexProcess(bool is_reaction, int (*get_next_record_cb)(void* context), void (*process_result_cb)(void* context),
                              void (*process_error_cb)(int id, void* context), void* context){BINGO_BEGIN{
    if (self.parallel_indexing_dispatcher.get() == 0){if (is_reaction) self.parallel_indexing_dispatcher = std::make_unique<RingoIndexingDispatcher>(self);
else self.parallel_indexing_dispatcher = std::make_unique<MangoIndexingDispatcher>(self);
}

self.parallel_indexing_dispatcher->context = context;
self.parallel_indexing_dispatcher->get_next_record_cb = get_next_record_cb;
self.parallel_indexing_dispatcher->process_result_cb = process_result_cb;
self.parallel_indexing_dispatcher->process_error_cb = process_error_cb;
self.parallel_indexing_dispatcher->_finished = false;
self.parallel_indexing_dispatcher->run(self.bingo_context->nthreads);
//      self.parallel_indexing_dispatcher.reset(0);
}
BINGO_END(0, -1)
}

//
// IndexingDispatcher
//
IndexingDispatcher::IndexingDispatcher(BingoCore& core, int method, bool set_parent_SID_for_threads, int records_per_command)
    : _core(core), OsCommandDispatcher(method, set_parent_SID_for_threads)
{
    _finished = false;
    _records_per_command = records_per_command;

    process_error_cb = 0;
    get_next_record_cb = 0;
    process_result_cb = 0;
}

OsCommand* IndexingDispatcher::_allocateCommand()
{
    return new IndexingCommand();
}

bool IndexingDispatcher::_setupCommand(OsCommand& cmd)
{
    if (_finished)
        return false;

    profTimerStart(tfing, "parallel.setupCommand");

    IndexingCommand& command = (IndexingCommand&)cmd;
    command.core = &_core;
    command.lock_for_exclusive_access = &_lock_for_exclusive_access;
    while (command.ids.size() < _records_per_command)
    {
        if (!get_next_record_cb(context))
        {
            _finished = true;
            break;
        }
        command.records.add(_core.index_record_data.ref());
        command.ids.push(_core.index_record_data_id);
    }
    return command.ids.size() != 0;
}

void IndexingDispatcher::_handleResult(OsCommandResult& res)
{
    profTimerStart(tfing, "parallel.handleResult");
    IndexingCommandResult& result = (IndexingCommandResult&)res;
    for (int i = 0; i < result.ids.size(); i++)
    {
        _exposeCurrentResult(i, result);
        process_result_cb(context);
    }
    if (process_error_cb)
    {
        // Process results with errors
        for (int i = 0; i < result.error_ids.size(); i++)
        {
            _core.warning.copy((const char*)result.error_messages.get(i), result.error_messages.getSize(i));
            process_error_cb(result.error_ids[i], context);
        }
    }
}

//
// IndexingCommand
//
void IndexingCommand::clear()
{
    records.clear();
    ids.clear();
}

void IndexingCommand::execute(OsCommandResult& result_)
{
    IndexingCommandResult& result = (IndexingCommandResult&)result_;
    for (int i = 0; i < ids.size(); i++)
    {
        BufferScanner scanner(records.get(i), records.getSize(i));
        NullOutput output;

        try
        {
            bool exception_found = false;
            TRY_READ_TARGET_RXN
            {
                TRY_READ_TARGET_MOL
                {
                    try
                    {
                        BingoIndex& index = result.getIndex(result.ids.size());
                        index.skip_calculate_fp = core->skip_calculate_fp;
                        index.init(*core->bingo_context);
                        index.prepare(scanner, output, lock_for_exclusive_access);
                    }
                    catch (CmfSaver::Error& e)
                    {
                        if (core->bingo_context->reject_invalid_structures)
                            throw;
                        exception_found = true;
                        result.error_messages.add(e.message());
                    }
                    catch (CrfSaver::Error& e)
                    {
                        if (core->bingo_context->reject_invalid_structures)
                            throw;
                        exception_found = true;
                        result.error_messages.add(e.message());
                    }
                }
                CATCH_READ_TARGET_MOL({
                    if (core->bingo_context->reject_invalid_structures)
                        throw;
                    result.error_messages.add(e.message());
                    exception_found = true;
                });
            }
            CATCH_READ_TARGET_RXN({
                if (core->bingo_context->reject_invalid_structures)
                    throw;
                result.error_messages.add(e.message());
                exception_found = true;
            });

            if (exception_found)
                result.error_ids.push(ids[i]);
            else
                result.ids.push(ids[i]);
        }
        catch (Exception& e)
        {
            // Check unhandled exceptions
            e.appendMessage(" ERROR ON id=%d", ids[i]);
            throw;
        }
    }
}

//
// IndexingCommandResult
//
void IndexingCommandResult::clear()
{
    ids.clear();
    error_ids.clear();
    error_messages.clear();
}
