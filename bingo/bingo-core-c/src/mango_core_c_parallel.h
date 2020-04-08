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

#ifndef __mango_core_c_parallel_h___
#define __mango_core_c_parallel_h___

#include "bingo_core_c_internal.h"
#include "bingo_core_c_parallel.h"

#include "base_cpp/reusable_obj_array.h"

namespace indigo
{
    namespace bingo_core
    {

        class MangoIndexingCommandResult : public IndexingCommandResult
        {
        public:
            virtual void clear();
            virtual BingoIndex& getIndex(int index);

            ReusableObjArray<MangoIndex> per_molecule_index;
        };

        class MangoIndexingDispatcher : public IndexingDispatcher
        {
        public:
            MangoIndexingDispatcher(BingoCore& core);

        protected:
            virtual void _exposeCurrentResult(int index, IndexingCommandResult& result);
            virtual OsCommandResult* _allocateResult();
        };

    } // namespace bingo_core
} // namespace indigo

#endif // __mango_core_c_parallel_h___
