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
#include <functional>
#include <queue>
#include <vector>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    struct PriorityElement
    {
        PriorityElement(const std::pair<int, int>& dir, const std::pair<int, int>& back_dir) : dir(dir), from_dir(back_dir)
        {
        }

        PriorityElement(int to_dir_id, int to_atom, int back_dir_id, int back_atom) : dir(to_dir_id, to_atom), from_dir(back_dir_id, back_atom)
        {
        }

        std::pair<int, int> dir;      // left, right, branch -> destination atom
        std::pair<int, int> from_dir; // left, right, branch -> source atom
    };

    using SequenceLayoutMap = std::map<int, std::map<int, int>>;
    struct CompareDirectionsPair
    {
        bool operator()(const PriorityElement& lhs, const PriorityElement& rhs) const
        {
            return lhs.dir.first > rhs.dir.first;
        }
    };

    using DirectionsPriorityQueue = std::priority_queue<PriorityElement, std::vector<PriorityElement>, CompareDirectionsPair>;

    class DLLEXPORT SequenceLayout
    {
    public:
        static constexpr float DEFAULT_BOND_LENGTH = 1.6f;

        explicit SequenceLayout(BaseMolecule& molecule);
        void make();
        void sequenceExtract(std::vector<std::deque<int>>& sequences);
        void calculateCoordinates(SequenceLayoutMap& layout_sequence);

        const std::vector<std::map<int, int>>& directionsMap();

        DECL_ERROR;

    private:
        bool _isMonomerBackbone(int atom_idx);
        void addSequenceElement(BaseMolecule& mol, PriorityElement& pel, std::vector<std::deque<int>>& sequences);
        void addNeigbourDirections(BaseMolecule& mol, DirectionsPriorityQueue& pq, const std::set<int>& valid_atoms, int atom_idx);

        const std::pair<int, int> _getBackDir(int src_idx, int dst_idx);
        bool _isValidAndBackbone(const std::pair<int, int>& dir);
        BaseMolecule& _molecule;
        SequenceLayoutMap _layout_sequence;
        std::vector<std::map<int, int>> _directions_map;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
