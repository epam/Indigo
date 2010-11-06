/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#ifndef __bingo_parallel_h__
#define __bingo_parallel_h__

#include "base_cpp/os_thread_wrapper.h"
#include "base_cpp/chunk_storage.h"

// Helper class for parallelizing algorithms with
// fetching packs of BLOBs or CLOBS from the database

using namespace indigo;

namespace indigo
{
   class OracleStatement;
   class OracleLOB;
   class OracleEnv;
}

// Base class for command.
// This command contains pack of the blobs (molecules in the raw format) 
// Subclasses should have additional data per each blob record from 
// blob_storage.
class BingoOracleCommand : public OsCommand
{
public:
   virtual void execute (OsCommandResult &result) = 0;
   virtual void clear ();

   ChunkStorage blob_storage;
};

// Dispatcher for creating commands.
// Subclasses should overload _handleResult for result 
// handling (if nessesary)
// Each thread has the same Session ID as parent thread.
class BingoOracleDispatcher : public OsCommandDispatcher
{
public:
   // Parameters:
   //    method - HANDLING_ORDER_***
   //    set_parent_SID_for_threads - true if set SID for all 
   //       child threads to the parent' SID
   BingoOracleDispatcher (int method,  
      bool set_parent_SID_for_threads, int blobs_per_command);

   void setup (OracleStatement *statement, OracleLOB *lob,
      char *varchar2_text, bool read_from_LOB);

protected:
   virtual bool _setupCommand    (OsCommand &command);

   // This function is call _blobs_per_command times to 
   // initializer additional data for each molecule
   virtual void _addCurrentRecordToCommand (BingoOracleCommand &cmd);

protected:
   OracleStatement *_statement;
   OracleLOB *_lob;
   char *_varchar2_text;
   bool _read_from_LOB;

   bool _finished;
   int _blobs_per_command;
};


#endif // __bingo_parallel_h__
