/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#include "bingo_core_c_internal.h"
#include "ringo_core_c_parallel.h"

#include <string.h>

using namespace indigo::bingo_core;

//
// MangoIndexingCommandResult
//
void RingoIndexingCommandResult::clear ()
{
   IndexingCommandResult::clear();
   per_reaction_index.clear();
}

BingoIndex& RingoIndexingCommandResult::getIndex (int index)
{
   per_reaction_index.resize(index + 1);
   return per_reaction_index[index];
}

//
// MangoIndexingDispatcher
//
RingoIndexingDispatcher::RingoIndexingDispatcher (BingoCore &core) : 
   IndexingDispatcher(core, HANDLING_ORDER_ANY, true, 30)
{
}

void RingoIndexingDispatcher::_exposeCurrentResult (int index, IndexingCommandResult &res)
{
   RingoIndexingCommandResult &result = (RingoIndexingCommandResult &)res;
   _core.ringo_index = &result.per_reaction_index[index];
   _core.index_record_data_id = result.ids[index];
}

OsCommandResult* RingoIndexingDispatcher::_allocateResult  ()
{
   return new RingoIndexingCommandResult();
}
