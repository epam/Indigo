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

#ifndef __bingo_core_c_parallel_h___
#define __bingo_core_c_parallel_h___

#include "base_cpp/os_thread_wrapper.h"
#include "base_cpp/chunk_storage.h"

// Helper classes for parallelized indexing

namespace indigo {
namespace bingo_core {

class BingoCore;
class IndexingCommandResult;

// This command contains pack of the binary data 
// (molecules or reactions in the raw format) 
class IndexingCommand : public OsCommand
{
public:
   virtual void execute (OsCommandResult &result);
   virtual void clear ();

   // Molecules or reactions
   ChunkStorage records;
   // Array of their internal indices
   Array<int> ids;

   BingoCore *core;
   OsLock *lock_for_exclusive_access;
};

class IndexingCommandResult : public OsCommandResult
{
public:
   virtual void clear ();

   virtual BingoIndex& getIndex (int index) = 0;

   // Array of processed indices
   Array<int> ids;

   // Array with error messages
   ChunkStorage error_messages;
   Array<int> error_ids;
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
   IndexingDispatcher (BingoCore &core, int method,  
      bool set_parent_SID_for_threads, int records_per_command);

   void *context;
   // If this function returns 0, then data is finished
   int (*get_next_record_cb) (void *context);
   void (*process_result_cb) (void *context);
   void (*process_error_cb) (int id, void *context);
   bool _finished;

protected:
   // This method should be overridden to setup current processed record so
   // it can be processed in process_result_cb callback.
   virtual void _exposeCurrentResult (int index, IndexingCommandResult &result) = 0;
protected:
   BingoCore &_core;

private:
   virtual OsCommand* _allocateCommand ();

   virtual bool _setupCommand (OsCommand &command);
   virtual void _handleResult (OsCommandResult &result);

   int _records_per_command;
   OsLock _lock_for_exclusive_access;
};

}
}

#endif // __bingo_core_c_parallel_h___
