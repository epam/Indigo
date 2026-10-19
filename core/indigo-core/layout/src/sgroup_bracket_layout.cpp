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

#include "layout/sgroup_bracket_layout.h"

#include <algorithm>
#include <cmath>

#include "layout/metalayout.h"
#include "molecule/base_molecule.h"

using namespace indigo;

SGroupBracketLayout::SGroupBracketLayout(BaseMolecule& molecule) : _molecule(molecule)
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
    for (auto& [idx, parent] : _parents)
        parent = _findParent(idx);
    // Parents read from a file can form a loop: it is cut where it is found
    for (auto& [idx, parent] : _parents)
        if (_depth(idx) >= static_cast<int>(_parents.size()))
            parent = -1;

    // A parent encloses the brackets of its children: the deepest groups are placed first
    std::stable_sort(_bracketed.begin(), _bracketed.end(), [this](int a, int b) { return _depth(a) > _depth(b); });
}

bool SGroupBracketLayout::isBracketed(int sgroup_type)
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

void SGroupBracketLayout::placeAll()
{
    _place(false);
}

void SGroupBracketLayout::placeMissing()
{
    _place(true);
}

void SGroupBracketLayout::_place(bool only_missing)
{
    if (_bracketed.empty() || !_hasCoordinates())
        return;
    _bondLength = _averageBondLength(_molecule);
    for (int idx : _bracketed)
    {
        SGroup& sgroup = _molecule.sgroups.getSGroup(idx);
        if (!only_missing || !_hasBrackets(sgroup))
            _setBrackets(sgroup, _boundingBox(idx));
    }
}

// The copies of a multiple group stand at one point until it is laid out: bonds of zero length do not count
float SGroupBracketLayout::_averageBondLength(BaseMolecule& molecule)
{
    float total = 0;
    int counted = 0;
    for (int i = molecule.edgeBegin(); i != molecule.edgeEnd(); i = molecule.edgeNext(i))
    {
        const Edge& edge = molecule.getEdge(i);
        Vec2f begin, end;
        Vec2f::projectZ(begin, molecule.getAtomXyz(edge.beg));
        Vec2f::projectZ(end, molecule.getAtomXyz(edge.end));
        const float length = Vec2f::dist(begin, end);
        if (length > EPSILON)
        {
            total += length;
            counted++;
        }
    }
    return counted > 0 ? total / counted : LayoutOptions::DEFAULT_BOND_LENGTH;
}

// A structure without coordinates has all its atoms at one point
bool SGroupBracketLayout::_hasCoordinates() const
{
    if (_molecule.vertexCount() < 2)
        return true;
    const Vec3f& first = _molecule.getAtomXyz(_molecule.vertexBegin());
    for (int i = _molecule.vertexNext(_molecule.vertexBegin()); i != _molecule.vertexEnd(); i = _molecule.vertexNext(i))
    {
        const Vec3f& other = _molecule.getAtomXyz(i);
        if (std::abs(other.x - first.x) > EPSILON || std::abs(other.y - first.y) > EPSILON)
            return true;
    }
    return false;
}

// The parent from the file if it is a bracketed group around this one: an index into the groups goes stale after a
// merge or a clone. Otherwise the smallest bracketed group strictly containing its atoms (#3954).
int SGroupBracketLayout::_findParent(int sgroup_idx) const
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
    for (int idx : _bracketed)
    {
        const SGroup& other = sgroups.getSGroup(idx);
        if (idx == sgroup_idx || other.atoms.size() <= sgroup.atoms.size())
            continue;
        if (smallest >= 0 && other.atoms.size() >= sgroups.getSGroup(smallest).atoms.size())
            continue;
        if (_contains(idx, atoms))
            smallest = idx;
    }
    return smallest;
}

bool SGroupBracketLayout::_contains(int sgroup_idx, const std::vector<int>& sorted_atoms) const
{
    const SGroup& sgroup = _molecule.sgroups.getSGroup(sgroup_idx);
    std::vector<int> group_atoms(sgroup.atoms.begin(), sgroup.atoms.end());
    std::sort(group_atoms.begin(), group_atoms.end());
    return std::includes(group_atoms.begin(), group_atoms.end(), sorted_atoms.begin(), sorted_atoms.end());
}

int SGroupBracketLayout::_depth(int sgroup_idx) const
{
    int depth = 0;
    for (int parent = _parents.at(sgroup_idx); parent >= 0 && depth < static_cast<int>(_parents.size()); parent = _parents.at(parent))
        depth++;
    return depth;
}

Rect2f SGroupBracketLayout::_boundingBox(int sgroup_idx) const
{
    const SGroup& sgroup = _molecule.sgroups.getSGroup(sgroup_idx);
    Vec2f point;
    Vec2f::projectZ(point, _molecule.getAtomXyz(sgroup.atoms[0]));
    Rect2f bbox(point, point);
    for (int atom : sgroup.atoms)
    {
        Vec2f::projectZ(point, _molecule.getAtomXyz(atom));
        bbox = Rect2f(bbox, Rect2f(point, point));
    }
    for (const auto& [child, parent] : _parents)
        if (parent == sgroup_idx)
            if (const std::optional<Rect2f> nested = _bracketsBox(_molecule.sgroups.getSGroup(child)))
                bbox = Rect2f(bbox, *nested);
    return bbox;
}

void SGroupBracketLayout::_setBrackets(SGroup& sgroup, const Rect2f& bbox) const
{
    const float gap = GAP * _bondLength;
    const float left = bbox.left() - gap, right = bbox.right() + gap;
    float bottom = bbox.bottom() - gap, top = bbox.top() + gap;
    const float shortfall = MIN_LENGTH * _bondLength - (top - bottom);
    if (shortfall > 0)
    {
        bottom -= shortfall / 2;
        top += shortfall / 2;
    }

    sgroup.brackets.clear();
    Vec2f* opening = sgroup.brackets.push();
    opening[0].set(left, bottom);
    opening[1].set(left, top);
    Vec2f* closing = sgroup.brackets.push();
    closing[0].set(right, top);
    closing[1].set(right, bottom);
}

bool SGroupBracketLayout::_hasBrackets(const SGroup& sgroup)
{
    if (sgroup.brackets.size() == 0)
        return false;
    for (int i = 0; i < sgroup.brackets.size(); ++i)
        if (Vec2f::distSqr(sgroup.brackets[i][0], sgroup.brackets[i][1]) <= EPSILON)
            return false;
    return true;
}

std::optional<Rect2f> SGroupBracketLayout::_bracketsBox(const SGroup& sgroup)
{
    if (!_hasBrackets(sgroup))
        return std::nullopt;
    Rect2f box(sgroup.brackets[0][0], sgroup.brackets[0][1]);
    for (int i = 1; i < sgroup.brackets.size(); ++i)
        box = Rect2f(box, Rect2f(sgroup.brackets[i][0], sgroup.brackets[i][1]));
    return box;
}
