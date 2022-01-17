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

#include "ringo_core_c_parallel.h"
#include "bingo_core_c_internal.h"

#include <string.h>

using namespace indigo;
using namespace indigo::bingo_core;

//
// MangoIndexingCommandResult
//
void RingoIndexingCommandResult::clear()
{
    IndexingCommandResult::clear();
    per_reaction_index.clear();
}

indigo::BingoIndex& RingoIndexingCommandResult::getIndex(int index)
{
    per_reaction_index.resize(index + 1);
    return per_reaction_index[index];
}

//
// MangoIndexingDispatcher
//
RingoIndexingDispatcher::RingoIndexingDispatcher(BingoCore& core) : IndexingDispatcher(core, HANDLING_ORDER_ANY, true, 30)
{
}

void RingoIndexingDispatcher::_exposeCurrentResult(int index, IndexingCommandResult& res)
{
    RingoIndexingCommandResult& result = (RingoIndexingCommandResult&)res;
    _core.ringo_index = &result.per_reaction_index[index];
    _core.index_record_data_id = result.ids[index];
}

OsCommandResult* RingoIndexingDispatcher::_allocateResult()
{
    return new RingoIndexingCommandResult();
}
