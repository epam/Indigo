/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#include "oracle/bingo_oracle_parallel.h"

#include "base_cpp/tlscont.h"
#include "oracle/ora_wrap.h"

//
// BingoOracleCommand
//
void BingoOracleCommand::clear ()
{
   blob_storage.clear();
   OsCommand::clear();
}

//
// BingoOracleDispatcher
//
BingoOracleDispatcher::BingoOracleDispatcher (int method, 
      bool set_parent_SID_for_threads, int blobs_per_command) 
   : OsCommandDispatcher(method, set_parent_SID_for_threads)
{
   _blobs_per_command = blobs_per_command;
}

void BingoOracleDispatcher::setup (OracleStatement *statement, 
                                   OracleLOB *lob, char *varchar2_text, 
                                   bool read_from_LOB)
{
   _statement = statement;
   _lob = lob;
   _varchar2_text = varchar2_text;
   _read_from_LOB = read_from_LOB;
   _finished = false;
}

bool BingoOracleDispatcher::_setupCommand (OsCommand &command)
{
   if (_finished)
      return false;

   BingoOracleCommand &cmd = (BingoOracleCommand &)command;
   cmd.blob_storage.clear();
   int count = 0;
   bool is_fetched;
   do 
   {
      _addCurrentRecordToCommand(cmd);
      count++;
   } while ((is_fetched = _statement->fetch()) && count < _blobs_per_command);

   if (!is_fetched)
      _finished = true;

   return true;
}

void BingoOracleDispatcher::_addCurrentRecordToCommand (BingoOracleCommand &cmd)
{
   int size;
   if (_read_from_LOB)
      size = _lob->getLength();
   else
      size = strlen(_varchar2_text);

   byte *buffer = cmd.blob_storage.add(size);

   if (_read_from_LOB)
      _lob->read(0, (char*)buffer, size);
   else
      memcpy(buffer, _varchar2_text, size);
}
