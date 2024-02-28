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
    using SequenceLayoutMap = std::map<int, std::map<int, int>>;

    class DLLEXPORT SequenceLayout
    {
    public:
        struct PriorityElement
        {
            PriorityElement(const std::pair<int, int>& dir, const std::pair<int, int>& back_dir, int col, int row)
                : dir(dir), back_dir(back_dir), col(col), row(row)
            {
            }
            std::pair<int, int> dir;
            std::pair<int, int> back_dir;
            int col;
            int row;
        };

        static constexpr float DEFAULT_BOND_LENGTH = 1.6f;

        explicit SequenceLayout(BaseMolecule& molecule);
        void make();
        void calculateLayout(SequenceLayoutMap& layout_sequence);
        void calculateCoordinates(SequenceLayoutMap& layout_sequence);

        const std::vector<std::map<int, int>>& directionsMap();

        DECL_ERROR;

    private:
        bool _isMonomerBackbone(int atom_idx);
        void processPosition(BaseMolecule& mol, PriorityElement& pel, SequenceLayoutMap& layout_sequence);
        const std::pair<int, int> _getBackDir(int src_idx, int dst_idx);
        BaseMolecule& _molecule;
        SequenceLayoutMap _layout_sequence;
        std::vector<std::map<int, int>> _directions_map; // TODO: change to std::vector
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
