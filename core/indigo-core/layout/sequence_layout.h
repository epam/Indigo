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

#ifndef __sequence_layout_h__
#define __sequence_layout_h__

#include "base_cpp/cancellation_handler.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

#include <deque>
#include <queue>
#include <vector>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    const int kRowSpacing = 4;
    class DLLEXPORT SequenceLayout
    {
    public:
        struct PriorityElement
        {
            PriorityElement(int dir, int atom_idx, int col, int row) : dir(dir), atom_idx(atom_idx), col(col), row(row)
            {
            }
            int dir;
            int atom_idx;
            int col;
            int row;
        };

        static constexpr float DEFAULT_BOND_LENGTH = 1.6f;

        explicit SequenceLayout(BaseMolecule& molecule);
        void make();
        void make(int first_atom_idx);
        void calculateLayout(int first_atom_idx, std::map<int, std::map<int, int>>& layout_sequence);
        const std::unordered_map<int, std::map<int, int>>& directionsMap();

        DECL_ERROR;

    private:
        static void processPosition(BaseMolecule& mol, int& row, int& col, int atom_from_idx, const std::pair<int, int>& dir);
        BaseMolecule& _molecule;
        std::map<int, std::map<int, int>> _layout_sequence;
        std::unordered_map<int, std::map<int, int>> _directions_map;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
