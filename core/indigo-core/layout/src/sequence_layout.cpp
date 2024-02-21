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

const std::pair<int, int> SequenceLayout::_getBackDir(int src_idx, int dst_idx)
{
    for (const auto& back_dir : _directions_map[dst_idx])
    {
        if (back_dir.second == src_idx)
            return back_dir;
    }
    return std::make_pair(-1, -1);
}

void SequenceLayout::processPosition(BaseMolecule& mol, PriorityElement& pel, SequenceLayoutMap& layout_sequence)
{
    if (mol.isTemplateAtom(pel.dir.second) && mol.isTemplateAtom(pel.back_dir.second))
    {
        std::string from_class = mol.getTemplateAtomClass(pel.back_dir.second);
        std::string to_class = mol.getTemplateAtomClass(pel.dir.second);
        bool isNucleoFrom = isNucleicClass(from_class) || isNucleotideClass(from_class);
        bool isNucleoTo = isNucleicClass(to_class) || isNucleotideClass(to_class);
        bool isAAFrom = isAminoAcidClass(from_class);
        bool isAATo = isAminoAcidClass(to_class);

        // if nucleo-nucleo or amino-amino, then change only col position
        if ((isNucleoFrom && isNucleoTo) || (isAAFrom && isAATo))
        {
            if (pel.dir.first == kLeftAttachmentPointIdx && pel.back_dir.first == kRightAttachmentPointIdx)
            {
                pel.col--;
                return;
            }
            else if (pel.dir.first == kRightAttachmentPointIdx && pel.back_dir.first == kLeftAttachmentPointIdx)
            {
                pel.col++;
                return;
            }
        }
        pel.row++;
        // check if we have this row already
        auto row_it = layout_sequence.find(pel.row);
        if (row_it != layout_sequence.end())
        {
            auto& col_map = row_it->second;
            if (col_map.find(pel.col) != col_map.end()) // check if we have a collision
            {
                // move all rows below the row
                auto rit = std::prev(layout_sequence.end());
                bool last = false;
                while (!last)
                {
                    auto key = rit->first;
                    if (rit != row_it)
                        rit--;
                    else
                        last = true;
                    auto nh = layout_sequence.extract(key);
                    nh.key()++;
                    layout_sequence.insert(std::move(nh));
                }
            }
        }
    }
}

const std::unordered_map<int, std::map<int, int>>& SequenceLayout::directionsMap()
{
    return _directions_map;
}

void SequenceLayout::calculateLayout(SequenceLayoutMap& layout_sequence)
{
    std::unordered_map<int, uint8_t> vertices_visited;
    _molecule.getTemplateAtomDirectionsMap(_directions_map);
    int row = -1;
    // place first atom
    auto comparePair = [](const PriorityElement& lhs, const PriorityElement& rhs) { return lhs.dir.first > rhs.dir.first; };
    std::priority_queue<PriorityElement, std::vector<PriorityElement>, decltype(comparePair)> pq(comparePair);

    // collect all atoms
    std::unordered_set<int> atoms;
    for (int i = _molecule.vertexBegin(); i < _molecule.vertexEnd(); i = _molecule.vertexNext(i))
        atoms.emplace(i);

    while (atoms.size())
    {
        row++;       // increase row for next fragment
        int col = 0; // every fragment starts from column = 0
        int first_atom_idx = -1;
        for (auto atom_idx : atoms)
        {
            if (_isMonomerBackbone(atom_idx))
            {
                first_atom_idx = atom_idx;
                break;
            }
        }

        if (first_atom_idx < 0)
            break;

        std::pair<int, int> first_dir(-1, first_atom_idx), back_dir(-1, -1);
        auto dirs_it = _directions_map.find(first_atom_idx);
        if (dirs_it != _directions_map.end() && dirs_it->second.size())
            first_dir = *dirs_it->second.begin();

        pq.emplace(first_dir, _getBackDir(first_atom_idx, first_dir.second), col, row);

        // bfs algorythm for a graph
        while (pq.size())
        {
            auto te = pq.top(); // top element
            pq.pop();
            int current_atom_idx = te.dir.second;
            if (vertices_visited[current_atom_idx] == 0)
            {
                atoms.erase(current_atom_idx);
                vertices_visited[current_atom_idx] = 1; // mark as passed
                if (_molecule.isTemplateAtom(current_atom_idx))
                {
                    processPosition(_molecule, te, layout_sequence);
                    layout_sequence[te.row][te.col] = current_atom_idx;
                }
                for (const auto& dir : _directions_map[current_atom_idx])
                {
                    // add to queue with priority. left, right, branch.
                    if (vertices_visited[dir.second] == 0)
                        pq.emplace(dir, _getBackDir(current_atom_idx, dir.second), te.col, te.row);
                }
            }
        }
    }
}

void SequenceLayout::make()
{
    calculateLayout(_layout_sequence);
    if (_layout_sequence.size())
    {
        auto row_it = _layout_sequence.begin();
        auto col_it = row_it->second.begin();
        int base_col = col_it->first;
        int base_row = row_it->first;
        const auto& origin = _molecule.getAtomXyz(col_it->second);
        for (auto& row : _layout_sequence)
        {
            int y_int = row.first - base_row;
            for (auto& col : row.second)
            {
                int x_int = col.first - base_col;
                Vec3f v(MoleculeLayout::DEFAULT_BOND_LENGTH * x_int, -MoleculeLayout::DEFAULT_BOND_LENGTH * y_int, 0);
                _molecule.setAtomXyz(col.second, v);
            }
        }
    }
}
