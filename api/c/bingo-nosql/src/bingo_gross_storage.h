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

#ifndef __bingo_gross_storage__
#define __bingo_gross_storage__

#include "base_cpp/scanner.h"
#include "molecule/molecule.h"
#include "molecule/molecule_gross_formula.h"
#include "reaction/reaction.h"

#include "bingo_cf_storage.h"
#include "mmf/mmf_ptr.h"
#include "src/mmf/mmf_mapping.h"

namespace bingo
{
    class GrossStorage
    {
    public:
        GrossStorage(size_t gross_block_size);

        static MMFAddress create(MMFPtr<GrossStorage>& gross_ptr, size_t gross_block_size);

        static void load(MMFPtr<GrossStorage>& gross_ptr, MMFAddress offset);

        void add(const indigo::Array<char>& gross_formula, int id);

        void find(indigo::Array<char>& query_formula, indigo::Array<int>& indices, int part_id = -1, int part_count = -1);

        void findCandidates(indigo::Array<char>& query_formula, indigo::Array<int>& candidates, int part_id = -1, int part_count = -1);

        int findNext(indigo::Array<char>& query_formula, indigo::Array<int>& candidates, int& cur_candidate);

        bool tryCandidate(indigo::Array<int>& query_array, int id);

        static void calculateMolFormula(indigo::Molecule& mol, indigo::Array<char>& gross_formula);

        static void calculateRxnFormula(indigo::Reaction& rxn, indigo::Array<char>& gross_formula);

    private:
        MMFMapping _hashes;
        ByteBufferStorage _gross_formulas;

        static dword _calculateGrossHashForMolArray(indigo::Array<int>& gross_array);

        static dword _calculateGrossHashForMol(const char* gross_str, int len);

        static dword _calculateGrossHash(const char* gross_str, int len);
    };
} // namespace bingo

#endif //__bingo_gross_storage__
