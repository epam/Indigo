/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "bingo_core_c.h"
#include "bingo_core_c_parallel.h"

#include "base_cpp/profiling.h"

using namespace indigo::bingo_core;

CEXPORT int bingoSetIndexRecordData (int id, const char *data, int data_size)
{
   BINGO_BEGIN
   {
      if (id < 0)
         throw BingoError("Negative id isn't allowed: id = %d", id);

      self.index_record_data->copy(data, data_size);
      self.index_record_data_id = id;
   }
   BINGO_END(0, -1)
}

IndexingDispatcher::IndexingDispatcher (BingoCore &core, int method,  
      bool set_parent_SID_for_threads, int records_per_command) : 
   _core(core), OsCommandDispatcher(method, set_parent_SID_for_threads)
{
   _finished = false;
   _records_per_command = records_per_command;

   process_error_cb = 0;
   get_next_record_cb = 0;
   process_result_cb = 0;
}

bool IndexingDispatcher::_setupCommand (OsCommand &cmd)
{
   if (_finished)
      return false;

   profTimerStart(tfing, "parallel.setupCommand");

   IndexingCommand &command = (IndexingCommand &)cmd;
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

void IndexingDispatcher::_handleResult (OsCommandResult &res)
{
   profTimerStart(tfing, "parallel.handleResult");
   IndexingCommandResult &result = (IndexingCommandResult &)res;
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
         _core.warning.copy((const char *)result.error_messages.get(i), result.error_messages.getSize(i));
         process_error_cb(result.error_ids[i], context);
      }
   }
}

void IndexingCommand::clear ()
{
   records.clear();
   ids.clear();
}

void IndexingCommandResult::clear ()
{
   ids.clear();
   error_ids.clear();
   error_messages.clear();
}
