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

void SequenceLayout::make()
{
    // looking for a first template atom
    for (int i = _molecule.vertexBegin(); i != _molecule.vertexEnd(); i = _molecule.vertexNext(i))
    {
        if (_molecule.isTemplateAtom(i))
        {
            make(i);
            break;
        }
    }
}

void SequenceLayout::processPosition(int& row, int& col, int atom_from_idx, const std::pair<int,int>& dir)
{
    int row_spacing = kRowSpacing;
    int ap_id = dir.first;
    std::string from_class = _molecule.getTemplateAtomClass(atom_from_idx);
    std::string to_class = _molecule.getTemplateAtomClass(dir.second);

    if (!isBackboneClass(from_class))
        ap_id += kBranchAttachmentPointIdx + 1;

    switch (dir.first)
    {
    case kLeftAttachmentPointIdx:
        col--; // go left
        break;
    case kRightAttachmentPointIdx:
        col++; // go right
        break;
    case kBranchAttachmentPointIdx:
        row_spacing = 1;
        [[fallthrough]];
    default: // branch
        auto& v1 = _molecule.getAtomXyz(atom_from_idx);
        auto& v2 = _molecule.getAtomXyz(dir.second);
        row += v2.y < v1.y ? -row_spacing : row_spacing; // branch up or down?
        break;
    }
}

void SequenceLayout::make(int first_atom_idx)
{
    std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash> templates;
    _molecule.getTemplatesMap(templates);

    auto atoms_num = _molecule.vertexCount();
    std::unordered_map<int, uint8_t> vertices_visited;
    std::map<int, std::map<int, int>> directions_map;

    for (int i = _molecule.template_attachment_points.begin(); i != _molecule.template_attachment_points.end();
         i = _molecule.template_attachment_points.next(i))
    {
        auto& tap = _molecule.template_attachment_points[i];
        if (tap.ap_id.size())
        {
            Array<char> atom_label;
            _molecule.getAtomSymbol(tap.ap_occur_idx, atom_label);
            int ap_id = tap.ap_id[0] - 'A';
            directions_map[tap.ap_occur_idx].emplace(ap_id, tap.ap_aidx);
        }
    }

    // place first atom
    auto comparePair = [](const PriorityElement& lhs, const PriorityElement& rhs) { return lhs.dir > rhs.dir; };
    std::priority_queue<PriorityElement, std::vector<PriorityElement>, decltype(comparePair)> pq(comparePair);

    auto dirs_it = directions_map.find(first_atom_idx);
    if (dirs_it != directions_map.end() && dirs_it->second.size())
    {
        auto first_dir = dirs_it->second.begin()->first;
        pq.emplace(first_dir, first_atom_idx, 0, 0);
    }

    // bfs algorythm for a graph
    while (pq.size())
    {
        const auto te = pq.top(); // top element
        pq.pop();
        int current_atom_idx = te.atom_idx;
        vertices_visited[current_atom_idx] = 1; // mark as passed
        bool found_directions = false;
        bool aready_visited = false;
        _layout_sequence[te.row][te.col] = current_atom_idx;
        for (const auto& dir : directions_map[current_atom_idx])
        {
            int col = te.col;
            int row = te.row;
            // add to queue with priority. left, right, branch.
            if (vertices_visited[dir.second] == 0)
            {
                processPosition(row, col, current_atom_idx, dir);
                pq.emplace(dir.first, dir.second, col, row);
                found_directions = true;
            }
            else
                aready_visited = true;
        }
    }
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
                Vec3f v(MoleculeLayout::DEFAULT_BOND_LENGTH * x_int, MoleculeLayout::DEFAULT_BOND_LENGTH * y_int, 0);
                _molecule.setAtomXyz(col.second, v);
            }
        }
    }
}
