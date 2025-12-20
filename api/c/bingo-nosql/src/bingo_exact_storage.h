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

#ifndef __bingo_exact_storage__
#define __bingo_exact_storage__

#include "molecule/molecule.h"
#include "reaction/reaction.h"

#include "mmf/mmf_ptr.h"
#include "src/mmf/mmf_mapping.h"

namespace bingo
{
    class ExactStorage
    {
    public:
        ExactStorage();

        static MMFAddress create(MMFPtr<ExactStorage>& exact_ptr);

        static void load(MMFPtr<ExactStorage>& exact_ptr, MMFAddress offset);

        size_t getOffset();

        void add(dword hash, int id);

        void findCandidates(dword query_hash, indigo::Array<int>& candidates, int part_id = -1, int part_count = -1);

        static dword calculateMolHash(indigo::Molecule& mol);

        static dword calculateRxnHash(indigo::Reaction& rxn);

    private:
        MMFMapping _molecule_hashes;
    };
} // namespace bingo

#endif //__bingo_exact_storage__
