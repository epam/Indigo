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

#include "layout/sgroup_data_layout.h"

#include "molecule/base_molecule.h"

using namespace indigo;

SGroupDataLayout::SGroupDataLayout(BaseMolecule& molecule) : _molecule(molecule)
{
    MoleculeSGroups& sgroups = molecule.sgroups;
    for (int i = sgroups.begin(); i != sgroups.end(); i = sgroups.next(i))
    {
        const SGroup& sgroup = sgroups.getSGroup(i);
        if (sgroup.sgroup_type != SGroup::SG_TYPE_DAT)
            continue;
        const DataSGroup& data = static_cast<const DataSGroup&>(sgroup);
        if (data.display_pos.has_value())
            _initialValuePositions.emplace(i, data.display_pos.value());
    }
    if (_initialValuePositions.empty())
        return;
    _initialAtomPositions.resize(molecule.vertexEnd());
    for (int i = molecule.vertexBegin(); i != molecule.vertexEnd(); i = molecule.vertexNext(i))
        _initialAtomPositions[i] = _position(i);
}

void SGroupDataLayout::update()
{
    if (_initialValuePositions.empty())
        return;
    const std::vector<int> parents = _molecule.sgroups.resolveParents();
    for (const auto& [sgroup_idx, initial] : _initialValuePositions)
    {
        DataSGroup& sgroup = static_cast<DataSGroup&>(_molecule.sgroups.getSGroup(sgroup_idx));
        const SGroup* reference = _referenceGroup(sgroup, parents[sgroup_idx]);
        if (reference == nullptr || !_hasMoved(*reference))
            continue;
        _place(sgroup, initial, *reference);
    }
}

const SGroup* SGroupDataLayout::_referenceGroup(const DataSGroup& sgroup, int parent) const
{
    if (sgroup.atoms.size() > 0)
        return &sgroup;
    // The renderer draws a relative value without atoms from a corner of a box it computes anew: it needs no move
    if (sgroup.relative)
        return nullptr;

    if (parent < 0 || _molecule.sgroups.getSGroup(parent).atoms.size() == 0)
        return nullptr;
    return &_molecule.sgroups.getSGroup(parent);
}

bool SGroupDataLayout::_hasMoved(const SGroup& reference) const
{
    for (int atom : reference.atoms)
        if (Vec2f::distSqr(_position(atom), _initialAtomPositions.at(atom)) > EPSILON)
            return true;
    return false;
}

// The rotation and the shift that bring the old positions of the atoms closest to the new ones by least squares
void SGroupDataLayout::_place(DataSGroup& sgroup, const Vec2f& initial, const SGroup& reference) const
{
    Vec2f old_center, new_center;
    for (int atom : reference.atoms)
    {
        old_center.add(_initialAtomPositions.at(atom));
        new_center.add(_position(atom));
    }
    const float scale = 1.f / static_cast<float>(reference.atoms.size());
    old_center.scale(scale);
    new_center.scale(scale);

    float dot = 0.f, cross = 0.f, perfect_length = 0.f;
    for (int atom : reference.atoms)
    {
        Vec2f before = _initialAtomPositions.at(atom), after = _position(atom);
        before.sub(old_center);
        after.sub(new_center);
        dot += Vec2f::dot(before, after);
        cross += Vec2f::cross(before, after);
        perfect_length += before.length() * after.length();
    }
    // One atom, atoms at one point, or a turn too weak to tell from rounding give no rotation
    float si = 0.f, co = 1.f;
    const float length = HYPOT(dot, cross);
    if (length > EPSILON && length > WEAKEST_FIT * perfect_length)
    {
        si = cross / length;
        co = dot / length;
    }

    // A relative position is an offset from the first atom of the group: the point it marks moves as an absolute one
    Vec2f position = initial;
    if (sgroup.relative)
        position.add(_initialAtomPositions.at(sgroup.atoms[0]));
    position.sub(old_center);
    position.rotate(si, co);
    position.add(new_center);
    if (sgroup.relative)
        position.sub(_position(sgroup.atoms[0]));
    sgroup.display_pos.set(position);
}

Vec2f SGroupDataLayout::_position(int atom) const
{
    Vec2f position;
    Vec2f::projectZ(position, _molecule.getAtomXyz(atom));
    return position;
}
