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

#include "mango_core_c_parallel.h"

#include <string.h>

using namespace indigo::bingo_core;

// Method for index molecules
CEXPORT int mangoIndexProcess (
   int (*get_next_record_cb) (void *context), 
   void (*process_result_cb) (void *context),
   void (*process_error_cb) (int id, void *context), void *context )
{
   BINGO_BEGIN
   {
      MangoIndexingDispatcher dispatcher(self);
      dispatcher.context = context;
      dispatcher.get_next_record_cb = get_next_record_cb;
      dispatcher.process_result_cb = process_result_cb;
      dispatcher.process_error_cb = process_error_cb;
      dispatcher.run();
   }
   BINGO_END(0, -1)
}

//
// MangoIndexingCommand
//
void MangoIndexingCommand::execute (OsCommandResult &res)
{
   MangoIndexingCommandResult &result = (MangoIndexingCommandResult &)res;
   for (int i = 0; i < ids.size(); i++)
   {
      MangoIndex &index = result.per_molecule_index.push();
      index.init(*core->bingo_context);

      BufferScanner scanner(records.get(i), records.getSize(i));
      NullOutput output;

      bool exception_found = false;
      TRY_READ_TARGET_MOL
      {
         try
         {
            index.prepare(scanner, output, lock_for_exclusive_access);
         }
         catch (CmfSaver::Error &e) 
         {
            exception_found = true;
            result.error_messages.add(e.message());
         }
      }
      CATCH_READ_TARGET_MOL(result.error_messages.add(e.message()); exception_found = true;);

      if (exception_found)
      {
         result.per_molecule_index.pop();
         result.error_ids.push(ids[i]);
      }
      else
      {
         result.ids.push(ids[i]);
      }
   }
}

//
// MangoIndexingCommandResult
//
void MangoIndexingCommandResult::clear ()
{
   IndexingCommandResult::clear();

   per_molecule_index.clear();
}

//
// MangoIndexingDispatcher
//
MangoIndexingDispatcher::MangoIndexingDispatcher (BingoCore &core) : 
   IndexingDispatcher(core, HANDLING_ORDER_ANY, true, 50)
{
}

void MangoIndexingDispatcher::_exposeCurrentResult (int index, IndexingCommandResult &res)
{
   MangoIndexingCommandResult &result = (MangoIndexingCommandResult &)res;
   _core.mango_index = &result.per_molecule_index[index];
   _core.index_record_data_id = result.ids[index];
}

OsCommand* MangoIndexingDispatcher::_allocateCommand ()
{
   return new MangoIndexingCommand();
}

OsCommandResult* MangoIndexingDispatcher::_allocateResult  ()
{
   return new MangoIndexingCommandResult();
}
