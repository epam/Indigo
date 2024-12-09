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

#include "layout/sequence_layout.h"
#include "../layout/molecule_layout.h"
#include "../molecule/monomer_commons.h"

using namespace indigo;

IMPL_ERROR(SequenceLayout, "sequence_layout");

SequenceLayout::SequenceLayout(BaseMolecule& molecule) : _molecule(molecule)
{
}

bool indigo::SequenceLayout::_isMonomerBackbone(int atom_idx)
{
    return _molecule.isTemplateAtom(atom_idx) && isBackboneClass(_molecule.getTemplateAtomClass(atom_idx));
}

bool SequenceLayout::_isValidAndBackbone(const std::pair<int, int>& dir)
{
    return dir.first >= 0 && dir.second >= 0 && dir.first < kBranchAttachmentPointIdx;
}

const std::pair<int, int> SequenceLayout::_getBackDir(int src_idx, int dst_idx)
{
    if (dst_idx > -1)
        for (const auto& back_dir : _directions_map[dst_idx])
        {
            if (back_dir.second == src_idx)
                return back_dir;
        }
    return std::make_pair(-1, src_idx);
}

void SequenceLayout::addNeigbourDirections(BaseMolecule& mol, DirectionsPriorityQueue& pq, const std::set<int>& valid_atoms, int atom_idx)
{
    auto& dirs = _directions_map[atom_idx];
    for (const auto& nei_dir : dirs)
    {
        // add to queue with priority. left, right, branch.
        if (valid_atoms.find(nei_dir.second) != valid_atoms.end())
        {
            auto back_dir = _getBackDir(atom_idx, nei_dir.second);
            if (_isValidAndBackbone(nei_dir) && _isValidAndBackbone(back_dir))
            {
                std::string to_class = mol.getTemplateAtomClass(nei_dir.second);
                std::string from_class = mol.getTemplateAtomClass(back_dir.second);
                bool isBothAminoAcid = isAminoAcidClass(to_class) && isAminoAcidClass(from_class);
                bool isBothNucleic = isNucleicClass(to_class) && isNucleicClass(from_class);
                // if to_class and from_class are different backbone types, treat the connection as branch
                if (!(isBothAminoAcid || isBothNucleic))
                {
                    pq.emplace(kBranchAttachmentPointIdx, nei_dir.second, kBranchAttachmentPointIdx, back_dir.second);
                    continue;
                }
            }
            pq.emplace(nei_dir, back_dir);
        }
    }
}

void SequenceLayout::addSequenceElement(BaseMolecule& mol, PriorityElement& pel, std::vector<std::deque<int>>& sequences)
{
    if (pel.dir.second >= 0 && mol.isTemplateAtom(pel.dir.second))
    {
        std::string to_class = mol.getTemplateAtomClass(pel.dir.second);
        if (isBackboneClass(to_class))
        {
            if (sequences.size() == 0)
                sequences.push_back({});

            auto& seq_item = sequences.back();

            // check if we have 'from' monomer
            if (pel.from_dir.second >= 0 && mol.isTemplateAtom(pel.from_dir.second))
            {
                std::string from_class = mol.getTemplateAtomClass(pel.from_dir.second);
                bool isNucleoFrom = isNucleicClass(from_class) || isNucleotideClass(from_class);
                bool isNucleoTo = isNucleicClass(to_class) || isNucleotideClass(to_class);
                bool isAAFrom = isAminoAcidClass(from_class);
                bool isAATo = isAminoAcidClass(to_class);

                if ((isNucleoFrom && isNucleoTo) || (isAAFrom && isAATo))
                {
                    if (pel.from_dir.first == kRightAttachmentPointIdx && pel.dir.first == kLeftAttachmentPointIdx)
                    {
                        seq_item.emplace_front(pel.dir.second);
                        return;
                    }
                    else if (pel.from_dir.first == kLeftAttachmentPointIdx && pel.dir.first == kRightAttachmentPointIdx)
                    {
                        seq_item.emplace_back(pel.dir.second);
                        return;
                    }
                }
                // break sequence
                if (seq_item.size())
                {
                    sequences.push_back({});
                    sequences.back().emplace_back(pel.dir.second);
                    return;
                }
            }
            seq_item.emplace_back(pel.dir.second);
        }
    }
}

const std::vector<std::map<int, int>>& SequenceLayout::directionsMap()
{
    return _directions_map;
}

void SequenceLayout::sequenceExtract(std::vector<std::deque<int>>& sequences)
{
    sequences.clear();
    _molecule.getTemplateAtomDirectionsMap(_directions_map);
    // place first atom
    DirectionsPriorityQueue pq;

    // collect all atoms
    std::set<int> remaining_atoms;
    for (int i = _molecule.vertexBegin(); i < _molecule.vertexEnd(); i = _molecule.vertexNext(i))
    {
        if (_molecule.isTemplateAtom(i))
            remaining_atoms.insert(i);
    }

    // bfs algorythm for a graph
    while (pq.size() || remaining_atoms.size())
    {
        if (pq.size() == 0)
        {
            pq.emplace(std::make_pair(-1, *remaining_atoms.begin()), std::make_pair(-1, -1));
            if (sequences.empty() || sequences.back().size())
                sequences.push_back({});
        }
        auto te = pq.top(); // top element
        pq.pop();
        if (remaining_atoms.find(te.dir.second) != remaining_atoms.end())
        {
            addSequenceElement(_molecule, te, sequences);                         // add current monomer to the sequences vector
            remaining_atoms.erase(te.dir.second);                                 // monomer consumed
            addNeigbourDirections(_molecule, pq, remaining_atoms, te.dir.second); // add destination neighbours into the queue
        }
    }
}

void SequenceLayout::calculateCoordinates(SequenceLayoutMap& layout_sequence)
{
    if (layout_sequence.size())
    {
        auto row_it = layout_sequence.begin();
        auto col_it = row_it->second.begin();
        int base_col = col_it->first;
        int base_row = row_it->first;
        const auto& origin = _molecule.getAtomXyz(col_it->second);
        for (auto& row : layout_sequence)
        {
            int y_int = row.first - base_row;
            for (auto& col : row.second)
            {
                int x_int = col.first - base_col;
                Vec3f v(LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH * x_int, -LayoutOptions::DEFAULT_MONOMER_BOND_LENGTH * y_int, 0);
                _molecule.setAtomXyz(col.second, v);
            }
        }
    }
}

void SequenceLayout::make()
{
    calculateCoordinates(_layout_sequence);
}
