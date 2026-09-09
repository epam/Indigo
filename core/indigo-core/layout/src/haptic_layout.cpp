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

#include "layout/haptic_layout.h"

#include <algorithm>
#include <deque>

#include "molecule/base_molecule.h"
#include "molecule/molecule_attachment_groups.h"

using namespace indigo;

IMPL_ERROR(HapticLayout, "haptic layout");

// Fixed rather than derived from the coordinates: on a symmetric structure a
// computed direction differs between platforms, as the layout energy already does.
static const Vec2f DEFAULT_DIRECTION(0.f, -1.f);

HapticLayout::HapticLayout(BaseMolecule& molecule, float bond_length) : group_bond_multiplier(1.5f), _molecule(molecule), _bond_length(bond_length)
{
}

bool HapticLayout::_resolveEndpoint(const HapticBond::Endpoint& endpoint, const Array<int>& component_of, Endpoint& resolved) const
{
    resolved = Endpoint();
    resolved.is_group = endpoint.isGroup();
    resolved.index = endpoint.index();

    if (endpoint.isGroup())
    {
        if (!_molecule.attachment_groups.hasGroup(endpoint.index()))
            return false;
        resolved.atoms = _molecule.attachment_groups.group(endpoint.index()).atoms();
    }
    else
    {
        resolved.atoms.push_back(endpoint.index());
    }

    if (resolved.atoms.empty())
        return false;

    for (int atom : resolved.atoms)
    {
        if (atom < 0 || atom >= component_of.size() || !_molecule.hasVertex(atom))
            return false;

        const int component = component_of[atom];
        if (component < 0)
            return false;

        // A group spread over several components has no single place to move to.
        if (resolved.component >= 0 && resolved.component != component)
            return false;
        resolved.component = component;
    }

    // The substituents the haptic bond must not be drawn over: for a group, the
    // bonds of its members that leave it.
    for (int atom : resolved.atoms)
    {
        const Vertex& vertex = _molecule.getVertex(atom);
        for (int i = vertex.neiBegin(); i < vertex.neiEnd(); i = vertex.neiNext(i))
        {
            const int neighbour = vertex.neiVertex(i);
            if (std::find(resolved.atoms.begin(), resolved.atoms.end(), neighbour) != resolved.atoms.end())
                continue;
            if (std::find(resolved.neighbours.begin(), resolved.neighbours.end(), neighbour) == resolved.neighbours.end())
                resolved.neighbours.push_back(neighbour);
        }
    }

    return true;
}

bool HapticLayout::_sameEndpoint(const Endpoint& left, const Endpoint& right)
{
    return left.is_group == right.is_group && left.index == right.index;
}

Vec2f HapticLayout::_endpointPos(const Endpoint& endpoint, const Array<Vec2f>& position)
{
    std::vector<Vec2f> positions;
    positions.reserve(endpoint.atoms.size());
    for (int atom : endpoint.atoms)
        positions.push_back(position[atom]);

    return AttachmentGroup::centreOf(positions);
}

Vec2f HapticLayout::_shiftedEndpointPos(const Endpoint& endpoint, const Array<Vec2f>& position, const Array<Vec2f>& shift)
{
    Vec2f pos = _endpointPos(endpoint, position);
    pos.add(shift[endpoint.component]);
    return pos;
}

Vec2f HapticLayout::_freeDirection(const Endpoint& from, const Vec2f& from_pos, const std::vector<Vec2f>& taken, const Array<Vec2f>& position,
                                   const Array<Vec2f>& shift)
{
    std::vector<float> angles;
    angles.reserve(from.neighbours.size() + taken.size());

    const auto take = [&angles, &from_pos](const Vec2f& point) {
        Vec2f direction;
        direction.diff(point, from_pos);
        if (direction.lengthSqr() > EPSILON * EPSILON)
            angles.push_back(atan2f(direction.y, direction.x));
    };

    // A neighbour shares the component of the endpoint, hence its shift.
    for (int neighbour : from.neighbours)
    {
        Vec2f neighbour_pos = position[neighbour];
        neighbour_pos.add(shift[from.component]);
        take(neighbour_pos);
    }

    for (const Vec2f& point : taken)
        take(point);

    if (angles.empty())
        return DEFAULT_DIRECTION;

    std::sort(angles.begin(), angles.end());

    // Widest gap, wrapping around the circle. One taken direction means a gap of
    // the whole circle, whose bisector is its opposite side.
    float gap_start = angles.back();
    float widest = _2FLOAT(2. * M_PI) - (angles.back() - angles.front());

    for (size_t i = 1; i < angles.size(); i++)
    {
        const float gap = angles[i] - angles[i - 1];
        if (gap > widest)
        {
            widest = gap;
            gap_start = angles[i - 1];
        }
    }

    const float bisector = gap_start + widest / 2.f;
    return Vec2f(cosf(bisector), sinf(bisector));
}

void HapticLayout::_place(const Link& link, bool from_begin, const Array<Vec2f>& position, const Array<int>& placed, Array<Vec2f>& shift) const
{
    const Endpoint& from = from_begin ? link.begin : link.end;
    const Endpoint& to = from_begin ? link.end : link.begin;

    const Vec2f from_pos = _shiftedEndpointPos(from, position, shift);

    // The multiplier is for a group-to-atom bond only (requirement 4 of #3233).
    const float length = _bond_length * (link.group_end ? group_bond_multiplier : 1.f);

    // Haptic partners already placed take up room around the endpoint too.
    std::vector<Vec2f> taken;
    for (const Link& other : _links)
    {
        const bool other_begin = _sameEndpoint(other.begin, from);
        if (!other_begin && !_sameEndpoint(other.end, from))
            continue;

        const Endpoint& partner = other_begin ? other.end : other.begin;
        if (partner.component == to.component || !placed[partner.component])
            continue;

        taken.push_back(_shiftedEndpointPos(partner, position, shift));
    }

    const Vec2f direction = _freeDirection(from, from_pos, taken, position, shift);

    // Measured from the centre of the group: from a member atom the radius of the
    // ring would eat most of the length.
    Vec2f target(from_pos);
    target.addScaled(direction, length);

    shift[to.component].diff(target, _endpointPos(to, position));
}

int HapticLayout::plan(int n_components, const Array<int>& component_of, const Array<Vec2f>& position, const Array<int>& frozen, Array<int>& cluster_of,
                       Array<Vec2f>& shift)
{
    cluster_of.clear_resize(n_components);
    cluster_of.fffill();
    shift.clear_resize(n_components);
    shift.zerofill();

    Array<int> placed;
    placed.clear_resize(n_components);
    placed.zerofill();

    _links.clear();

    const MoleculeHapticBonds& bonds = _molecule.haptic_bonds;
    for (int i = bonds.begin(); i != bonds.end(); i = bonds.next(i))
    {
        const HapticBond& bond = bonds.at(i);

        // Variable attachment (#3731) shares the container and has its own geometry.
        if (bond.type() != _BOND_HAPTIC)
            continue;

        Link link;
        if (!_resolveEndpoint(bond.begin(), component_of, link.begin) || !_resolveEndpoint(bond.end(), component_of, link.end))
            continue;

        // One component: the edges between the ends already settle the distance.
        if (link.begin.component == link.end.component)
            continue;

        // A frozen component is not dragged, and moving only the other end would
        // put the bond at a length nobody asked for.
        if (frozen[link.begin.component] || frozen[link.end.component])
            continue;

        link.group_end = link.begin.is_group || link.end.is_group;
        _links.push_back(link);
    }

    // The grid places a cluster as a whole; otherwise it would tear apart what is
    // being put together here.
    int n_clusters = 0;
    std::deque<int> queue;

    for (int component = 0; component < n_components; component++)
    {
        if (cluster_of[component] >= 0)
            continue;

        const int cluster = n_clusters++;
        cluster_of[component] = cluster;
        placed[component] = 1;
        queue.push_back(component);

        // Every link is applied once, from the placed end towards the unplaced one.
        while (!queue.empty())
        {
            const int anchor = queue.front();
            queue.pop_front();

            for (const Link& link : _links)
            {
                const bool from_begin = link.begin.component == anchor;
                if (!from_begin && link.end.component != anchor)
                    continue;

                const Endpoint& to = from_begin ? link.end : link.begin;
                if (placed[to.component])
                    continue;

                _place(link, from_begin, position, placed, shift);

                cluster_of[to.component] = cluster;
                placed[to.component] = 1;
                queue.push_back(to.component);
            }
        }
    }

    return n_clusters;
}
