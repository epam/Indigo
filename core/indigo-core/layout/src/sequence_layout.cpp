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

void SequenceLayout::make(int first_atom_idx)
{
    std::vector<uint8_t> vertices_visited(_molecule.vertexCount(), 0);
    std::vector<std::map<int, int>> directions_map(_molecule.vertexCount());

    for (int i = _molecule.template_attachment_points.begin(); i != _molecule.template_attachment_points.end();
         i = _molecule.template_attachment_points.next(i))
    {
        auto& t = _molecule.template_attachment_points[i];
        if (t.ap_id.size())
            directions_map.at(t.ap_occur_idx).emplace(t.ap_id[0] - 'A', t.ap_aidx);
    }

    // place first atom
    int row = 0, col = 0;
    auto comparePair = [](const auto& lhs, const auto& rhs) { return lhs.first > rhs.first; };
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, decltype(comparePair)> pq(comparePair);
    pq.emplace(0, first_atom_idx);

    bool found_dead_end = false;
    int prev_atom_idx = -1;
    while (pq.size())
    {
        auto& top_element = pq.top();
        int current_atom_idx = top_element.second;
        vertices_visited[current_atom_idx] = 1; // mark as passed
        auto& seq_element = _layout_sequence.at(col, row);
        switch (top_element.first)
        {
        case 0:
            seq_element.addLeft(col--, current_atom_idx);
            break;
        case 1:
            seq_element.addRight(col++, current_atom_idx);
            break;
        default:
            // go to next row
            if (prev_atom_idx > -1)
            {
                const auto& v1 = _molecule.getAtomXyz(prev_atom_idx);
                const auto& v2 = _molecule.getAtomXyz(current_atom_idx);
                if (v2.y < v1.y)
                    row--;
                else
                    row++;
            }
            _layout_sequence.at(col, row).addLeft(col, current_atom_idx);
            break;
        }
        prev_atom_idx = current_atom_idx;

        bool found_directions = false;
        for (const auto& dir : directions_map[current_atom_idx])
        {
            // add to queue with priority. left, right, branch.
            if (vertices_visited[dir.second] == 0)
            {
                pq.push(dir);
                found_directions = true;
            }
            else if (!found_dead_end)
            {
                // cycle detected.
            }
        }
        found_dead_end = !found_directions;
        if (found_dead_end)
        {
            pq.pop();
        }
    };
}

void SequenceLayout::setCancellationHandler(CancellationHandler* cancellation)
{
}
