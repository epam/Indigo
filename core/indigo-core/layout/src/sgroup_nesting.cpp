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

#include "layout/sgroup_nesting.h"

#include <algorithm>

#include "molecule/base_molecule.h"

using namespace indigo;

SGroupNesting::SGroupNesting(BaseMolecule& molecule) : _molecule(molecule)
{
    for (int i = molecule.sgroups.begin(); i != molecule.sgroups.end(); i = molecule.sgroups.next(i))
    {
        const SGroup& sgroup = molecule.sgroups.getSGroup(i);
        // A group without atoms would be nested in every other one
        if (sgroup.atoms.size() == 0)
            continue;
        if (isBracketed(sgroup.sgroup_type))
            _bracketed.push_back(i);
        if (isBracketed(sgroup.sgroup_type) || sgroup.sgroup_type == SGroup::SG_TYPE_MUL)
            _parents[i] = -1;
    }
    for (auto& [sgroup_idx, parent] : _parents)
        parent = _findParent(sgroup_idx);
    // Parents read from a file can form a loop: it is cut where it is found
    for (auto& [sgroup_idx, parent] : _parents)
        if (_depth(sgroup_idx) >= static_cast<int>(_parents.size()))
            parent = -1;

    std::stable_sort(_bracketed.begin(), _bracketed.end(), [this](int a, int b) { return _depth(a) > _depth(b); });
}

bool SGroupNesting::isBracketed(int sgroup_type)
{
    switch (sgroup_type)
    {
    case SGroup::SG_TYPE_SRU:
    case SGroup::SG_TYPE_COP:
    case SGroup::SG_TYPE_COM:
    case SGroup::SG_TYPE_MIX:
    case SGroup::SG_TYPE_FOR:
        return true;
    default:
        return false;
    }
}

const std::vector<int>& SGroupNesting::bracketedDeepestFirst() const
{
    return _bracketed;
}

std::vector<int> SGroupNesting::children(int sgroup_idx) const
{
    std::vector<int> found;
    for (const auto& [child, parent] : _parents)
        if (parent == sgroup_idx)
            found.push_back(child);
    return found;
}

// The parent from the file, checked to be a bracketed group around this one because an index into the groups goes
// stale after a merge or a clone. Otherwise the smallest bracketed group strictly containing its atoms.
int SGroupNesting::_findParent(int sgroup_idx) const
{
    MoleculeSGroups& sgroups = _molecule.sgroups;
    const SGroup& sgroup = sgroups.getSGroup(sgroup_idx);
    std::vector<int> atoms(sgroup.atoms.begin(), sgroup.atoms.end());
    std::sort(atoms.begin(), atoms.end());

    int parent = sgroup.parent_idx.value_or(-1);
    if (parent < 0 || !sgroups.hasSGroup(parent))
    {
        const int id = sgroup.parent_group.value_or(0);
        parent = id > 0 ? sgroups.findSGroupById(id) : -1;
    }
    if (parent >= 0 && parent != sgroup_idx && isBracketed(sgroups.getSGroup(parent).sgroup_type) && _contains(parent, atoms))
        return parent;

    int smallest = -1;
    for (int other_idx : _bracketed)
    {
        const SGroup& other = sgroups.getSGroup(other_idx);
        if (other_idx == sgroup_idx || other.atoms.size() <= sgroup.atoms.size())
            continue;
        if (smallest >= 0 && other.atoms.size() >= sgroups.getSGroup(smallest).atoms.size())
            continue;
        if (_contains(other_idx, atoms))
            smallest = other_idx;
    }
    return smallest;
}

bool SGroupNesting::_contains(int sgroup_idx, const std::vector<int>& sorted_atoms) const
{
    const SGroup& sgroup = _molecule.sgroups.getSGroup(sgroup_idx);
    std::vector<int> group_atoms(sgroup.atoms.begin(), sgroup.atoms.end());
    std::sort(group_atoms.begin(), group_atoms.end());
    return std::includes(group_atoms.begin(), group_atoms.end(), sorted_atoms.begin(), sorted_atoms.end());
}

int SGroupNesting::_depth(int sgroup_idx) const
{
    int depth = 0;
    for (int parent = _parents.at(sgroup_idx); parent >= 0 && depth < static_cast<int>(_parents.size()); parent = _parents.at(parent))
        depth++;
    return depth;
}

bool SGroupNesting::_isAncestor(int ancestor_idx, int sgroup_idx) const
{
    for (int parent = _parents.at(sgroup_idx); parent >= 0; parent = _parents.at(parent))
        if (parent == ancestor_idx)
            return true;
    return false;
}

std::vector<int> SGroupNesting::groupsBetween(int first_atom, int second_atom) const
{
    std::vector<int> around_first, around_second;
    for (int sgroup_idx : _bracketed)
    {
        const bool has_first = _hasAtom(sgroup_idx, first_atom), has_second = _hasAtom(sgroup_idx, second_atom);
        if (has_first && !has_second)
            around_first.push_back(sgroup_idx);
        else if (has_second && !has_first)
            around_second.push_back(sgroup_idx);
    }
    std::stable_sort(around_second.begin(), around_second.end(), [this](int a, int b) { return _depth(a) < _depth(b); });
    around_first.insert(around_first.end(), around_second.begin(), around_second.end());
    return around_first;
}

std::vector<bool> SGroupNesting::loneAtoms(int sgroup_idx) const
{
    std::vector<bool> lone(_molecule.vertexEnd(), true);
    for (int other_idx : _bracketed)
        if (!_isAncestor(other_idx, sgroup_idx))
            for (int atom : _molecule.sgroups.getSGroup(other_idx).atoms)
                lone[atom] = false;
    return lone;
}

bool SGroupNesting::_hasAtom(int sgroup_idx, int atom_idx) const
{
    return _molecule.sgroups.getSGroup(sgroup_idx).atoms.find(atom_idx) >= 0;
}
