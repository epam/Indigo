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

#include "bingo_core_c_internal.h"
#include "mango_core_c_parallel.h"

#include <string.h>

using namespace indigo::bingo_core;

//
// MangoIndexingCommandResult
//
void MangoIndexingCommandResult::clear ()
{
   IndexingCommandResult::clear();

   per_molecule_index.clear();
}

BingoIndex& MangoIndexingCommandResult::getIndex (int index)
{
   per_molecule_index.resize(index + 1);
   return per_molecule_index[index];
}

//
// MangoIndexingDispatcher
//
MangoIndexingDispatcher::MangoIndexingDispatcher (BingoCore &core) : 
   IndexingDispatcher(core, HANDLING_ORDER_ANY, true, 30)
{
}

void MangoIndexingDispatcher::_exposeCurrentResult (int index, IndexingCommandResult &res)
{
   MangoIndexingCommandResult &result = (MangoIndexingCommandResult &)res;
   _core.mango_index = &result.per_molecule_index[index];
   _core.index_record_data_id = result.ids[index];
}

OsCommandResult* MangoIndexingDispatcher::_allocateResult  ()
{
   return new MangoIndexingCommandResult();
}
