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
#include <map>

#include "molecule/base_molecule.h"
#include "molecule/molecule_attachment_groups.h"

using namespace indigo;

// Fixed rather than derived from the coordinates: on a symmetric structure a
// computed direction differs between platforms, as the layout energy already does.
static const Vec2f DEFAULT_DIRECTION(0.f, -1.f);

HapticLayout::HapticLayout(BaseMolecule& molecule, float bond_length, float group_bond_multiplier)
    : _molecule(molecule), _bond_length(bond_length), _group_bond_multiplier(group_bond_multiplier)
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

Vec2f HapticLayout::_placedEndpointPos(const Endpoint& endpoint, const Array<Vec2f>& position, const Array<Placement>& placement)
{
    return placement[endpoint.component].apply(_endpointPos(endpoint, position));
}

bool HapticLayout::_groupAxis(const Endpoint& endpoint, const Array<Vec2f>& position, Vec2f& axis)
{
    // The major axis of the atoms, not the line from the first member to the last:
    // the members are in the order the file listed them, which for an allyl group
    // is as likely to be the middle atom first as it is either end.
    if (endpoint.atoms.size() < 2 || endpoint.atoms.size() > 3)
        return false;

    Vec2f centre;
    for (int atom : endpoint.atoms)
        centre.add(position[atom]);
    centre.scale(1.f / endpoint.atoms.size());

    float xx = 0.f, xy = 0.f, yy = 0.f;
    for (int atom : endpoint.atoms)
    {
        Vec2f offset;
        offset.diff(position[atom], centre);
        xx += offset.x * offset.x;
        xy += offset.x * offset.y;
        yy += offset.y * offset.y;
    }

    const float trace = xx + yy;
    const float delta = sqrtf(std::max(0.f, (xx - yy) * (xx - yy) + 4.f * xy * xy));
    const float major = (trace + delta) / 2.f;

    if (major < EPSILON)
        return false;

    axis = fabs(xy) > EPSILON ? Vec2f(major - yy, xy) : Vec2f(xx >= yy ? 1.f : 0.f, xx >= yy ? 0.f : 1.f);
    return axis.normalize();
}

bool HapticLayout::_acrossTheGroup(const Endpoint& from, const Vec2f& from_pos, const Array<Vec2f>& position, const Array<Placement>& placement, Vec2f& axis)
{
    // A group of two or three atoms is a line, not a disc, and a bond leaving it
    // along that line runs over the ligand's own bonds - the overlap a chemist
    // objects to. Such a group is left across its own axis, at a right angle to the
    // bond its atoms share. A ring is round and has no such axis.
    if (from.atoms.size() < 2)
        return false;

    float xx = 0.f, xy = 0.f, yy = 0.f;
    for (int atom : from.atoms)
    {
        Vec2f offset;
        offset.diff(placement[from.component].apply(position[atom]), from_pos);
        xx += offset.x * offset.x;
        xy += offset.x * offset.y;
        yy += offset.y * offset.y;
    }

    // Eigenvalues of the 2x2 scatter matrix; the group counts as a line when the
    // smaller one is a small fraction of the larger.
    const float trace = xx + yy;
    const float delta = sqrtf(std::max(0.f, (xx - yy) * (xx - yy) + 4.f * xy * xy));
    const float major = (trace + delta) / 2.f;
    const float minor = (trace - delta) / 2.f;

    if (major < EPSILON || minor > major * 0.15f)
        return false;

    // The eigenvector of the major axis, turned by a right angle.
    Vec2f along(fabs(xy) > EPSILON ? Vec2f(major - yy, xy) : Vec2f(xx >= yy ? 1.f : 0.f, xx >= yy ? 0.f : 1.f));
    if (!along.normalize())
        return false;

    axis.set(-along.y, along.x);
    return true;
}

Vec2f HapticLayout::_freeDirection(const Endpoint& from, const Vec2f& from_pos, const std::vector<Vec2f>& taken, const Array<Vec2f>& position,
                                   const Array<Placement>& placement)
{
    std::vector<float> angles;
    angles.reserve(from.neighbours.size() + taken.size());

    const auto take = [&angles, &from_pos](const Vec2f& point) {
        Vec2f direction;
        direction.diff(point, from_pos);
        if (direction.lengthSqr() > EPSILON * EPSILON)
            angles.push_back(atan2f(direction.y, direction.x));
    };

    // A neighbour shares the component of the endpoint, hence its placement.
    for (int neighbour : from.neighbours)
        take(placement[from.component].apply(position[neighbour]));

    for (const Vec2f& point : taken)
        take(point);

    Vec2f across;
    const bool has_axis = _acrossTheGroup(from, from_pos, position, placement, across);

    if (angles.empty())
        return has_axis ? across : DEFAULT_DIRECTION;

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
    const Vec2f free_direction(cosf(bisector), sinf(bisector));

    if (!has_axis)
        return free_direction;

    // Of the two ways across the group, the one pointing away from what surrounds it.
    return Vec2f::dot(across, free_direction) >= 0.f ? across : Vec2f(-across.x, -across.y);
}

float HapticLayout::_lengthOf(const Link& link) const
{
    // The multiplier is for a group-to-atom bond only (requirement 4 of #3233).
    return _bond_length * (link.group_end ? _group_bond_multiplier : 1.f);
}

void HapticLayout::_place(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position, const Array<int>& placed,
                          Array<Placement>& placement) const
{
    const Link& first = *links.front();
    const bool from_begin = first.end.component == component;
    const Endpoint& from = from_begin ? first.begin : first.end;
    const Endpoint& to = from_begin ? first.end : first.begin;

    const Vec2f from_pos = _placedEndpointPos(from, position, placement);

    // Haptic partners already placed take up room around the endpoint too.
    std::vector<Vec2f> taken;
    for (const Link& other : _links)
    {
        const bool other_begin = _sameEndpoint(other.begin, from);
        if (!other_begin && !_sameEndpoint(other.end, from))
            continue;

        const Endpoint& partner = other_begin ? other.end : other.begin;
        if (partner.component == component || !placed[partner.component])
            continue;

        taken.push_back(_placedEndpointPos(partner, position, placement));
    }

    const Vec2f direction = _freeDirection(from, from_pos, taken, position, placement);

    // Measured from the centre of the group: from a member atom the radius of the
    // ring would eat most of the length.
    Vec2f target(from_pos);
    target.addScaled(direction, _lengthOf(first));

    placement[component].rotation = 0.f;

    // A two- or three-atom end is a line, and it has to meet the bond at a right
    // angle rather than lie along it - the same rule as _acrossTheGroup applies to
    // the end being placed, where it is a rotation instead of a direction.
    Vec2f axis;
    if (_groupAxis(to, position, axis))
    {
        const float axis_angle = atan2f(axis.y, axis.x);
        const float wanted_angle = atan2f(-direction.x, direction.y);
        placement[component].rotation = wanted_angle - axis_angle;
    }

    placement[component].shift.zero();
    placement[component].shift.diff(target, placement[component].apply(_endpointPos(to, position)));

    if (links.size() > 1)
        _refine(component, links, position, placement);
}

void HapticLayout::_refine(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position, Array<Placement>& placement) const
{
    // A chelating or bridging ligand is held by several bonds at once, and no
    // single translation satisfies them all. Each pass turns the component towards
    // the lengths its bonds ask for (the planar case of Kabsch, skipped when the
    // held points coincide and a rotation would mean nothing) and then corrects the
    // translation by the remaining length errors. The component moves as a rigid
    // body throughout, so the ligand keeps its own geometry.
    const int PASSES = 40;

    std::vector<Vec2f> source; // the held points, in the component's own coordinates
    std::vector<Vec2f> anchor; // the partner of each, already placed
    std::vector<float> length;

    for (const Link* link : links)
    {
        const bool from_begin = link->end.component == component;
        source.push_back(_endpointPos(from_begin ? link->end : link->begin, position));
        anchor.push_back(_placedEndpointPos(from_begin ? link->begin : link->end, position, placement));
        length.push_back(_lengthOf(*link));
    }

    Vec2f source_centre;
    for (const Vec2f& point : source)
        source_centre.add(point);
    source_centre.scale(1.f / source.size());

    float spread = 0.f;
    for (const Vec2f& point : source)
        spread = std::max(spread, Vec2f::dist(point, source_centre));

    // A single point held by two bonds - a metal atom between two ligands - has an
    // exact answer, and needs one: the midpoint between the two partners is a
    // saddle where the two corrections cancel and the loop below would stall on it.
    if (spread < EPSILON && source.size() == 2)
    {
        Vec2f axis;
        axis.diff(anchor[1], anchor[0]);
        const float distance = axis.length();

        if (distance > EPSILON && distance < length[0] + length[1] && distance > fabs(length[0] - length[1]))
        {
            axis.scale(1.f / distance);
            const float along = (length[0] * length[0] - length[1] * length[1] + distance * distance) / (2.f * distance);
            const float across = sqrtf(std::max(0.f, length[0] * length[0] - along * along));

            Vec2f normal(-axis.y, axis.x);
            Vec2f foot(anchor[0]);
            foot.addScaled(axis, along);

            // Of the two mirror solutions, the one further from everything already
            // placed: it is the side with room for the bond to be seen.
            Vec2f first(foot), second(foot);
            first.addScaled(normal, across);
            second.addScaled(normal, -across);

            float first_room = 0.f, second_room = 0.f;
            for (int i = 0; i < position.size(); i++)
            {
                first_room = std::max(first_room, -Vec2f::dist(first, position[i]));
                second_room = std::max(second_room, -Vec2f::dist(second, position[i]));
            }

            placement[component].rotation = 0.f;
            placement[component].shift.diff(first_room >= second_room ? first : second, source[0]);
            return;
        }
    }

    // Two bonds to one and the same partner - an eta-2 ligand, a chelating diene -
    // have an exact answer too. The two held points keep their distance `d`, so they
    // are the ends of a chord of the circle of radius L around the partner: the
    // midpoint of the chord goes sqrt(L^2 - (d/2)^2) away from the partner and the
    // chord stands perpendicular to that direction. Fitting them onto the circle
    // without that constraint drags the ligand onto the partner instead.
    if (source.size() == 2 && Vec2f::dist(anchor[0], anchor[1]) < EPSILON && spread > EPSILON)
    {
        const float half_chord = Vec2f::dist(source[0], source[1]) / 2.f;
        const float target_length = (length[0] + length[1]) / 2.f;

        if (half_chord < target_length)
        {
            Vec2f current_centre = placement[component].apply(source_centre);
            Vec2f direction;
            direction.diff(current_centre, anchor[0]);
            if (!direction.normalize())
                direction = Vec2f(0.f, -1.f);

            Vec2f wanted(anchor[0]);
            wanted.addScaled(direction, sqrtf(target_length * target_length - half_chord * half_chord));

            // The chord perpendicular to the bond: that is also what a chemist asks
            // for - the haptic bond meets the ligand bond at a right angle.
            Vec2f chord;
            chord.diff(source[1], source[0]);
            const float chord_angle = atan2f(chord.y, chord.x);
            const float wanted_angle = atan2f(-direction.x, direction.y);

            placement[component].rotation = wanted_angle - chord_angle;
            placement[component].shift.zero();
            placement[component].shift.diff(wanted, placement[component].apply(source_centre));
            return;
        }
    }

    for (int pass = 0; pass < PASSES; pass++)
    {
        std::vector<Vec2f> current;
        current.reserve(source.size());
        for (const Vec2f& point : source)
            current.push_back(placement[component].apply(point));

        if (spread > EPSILON)
        {
            std::vector<Vec2f> target;
            target.reserve(source.size());
            for (size_t i = 0; i < source.size(); i++)
            {
                Vec2f direction;
                direction.diff(current[i], anchor[i]);
                if (!direction.normalize())
                    direction = Vec2f(0.f, -1.f);

                Vec2f wanted(anchor[i]);
                wanted.addScaled(direction, length[i]);
                target.push_back(wanted);
            }

            Vec2f target_centre;
            for (const Vec2f& point : target)
                target_centre.add(point);
            target_centre.scale(1.f / target.size());

            float sin_sum = 0.f, cos_sum = 0.f;
            for (size_t i = 0; i < source.size(); i++)
            {
                Vec2f a, b;
                a.diff(source[i], source_centre);
                b.diff(target[i], target_centre);
                cos_sum += a.x * b.x + a.y * b.y;
                sin_sum += a.x * b.y - a.y * b.x;
            }

            if (fabs(sin_sum) > EPSILON || fabs(cos_sum) > EPSILON)
            {
                // Turn about where the component stands now, and leave the moving
                // to the correction below: recentring on the targets here would
                // undo it every pass, and two bonds to one and the same partner
                // would then drag the component onto that partner.
                const Vec2f centre_before = placement[component].apply(source_centre);
                placement[component].rotation = atan2f(sin_sum, cos_sum);
                placement[component].shift.zero();
                placement[component].shift.diff(centre_before, placement[component].apply(source_centre));
            }

            current.clear();
            for (const Vec2f& point : source)
                current.push_back(placement[component].apply(point));
        }

        // What the rotation cannot fix: every bond that is still too long or too
        // short pulls the component along its own direction, and the mean of those
        // pulls is the step. For two bonds to one partner this converges on the
        // point that is at the right distance from both.
        Vec2f correction;
        for (size_t i = 0; i < current.size(); i++)
        {
            Vec2f direction;
            direction.diff(current[i], anchor[i]);
            const float distance = direction.length();
            if (distance < EPSILON || !direction.normalize())
                continue;
            correction.addScaled(direction, length[i] - distance);
        }
        correction.scale(1.f / current.size());
        placement[component].shift.add(correction);
    }
}

void HapticLayout::_collectLinks(const Array<int>& component_of, const Array<int>& frozen)
{
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
}

int HapticLayout::plan(int n_components, const Array<int>& component_of, const Array<Vec2f>& position, const Array<int>& frozen, Array<Placement>& placement)
{
    placement.clear_resize(n_components);
    for (int i = 0; i < n_components; i++)
        placement[i] = Placement();

    Array<int> placed;
    placed.clear_resize(n_components);
    placed.zerofill();

    _collectLinks(component_of, frozen);

    // The grid places a cluster as a whole; otherwise it would tear apart what is
    // being put together here.
    int n_clusters = 0;
    std::deque<int> queue;

    for (int component = 0; component < n_components; component++)
    {
        if (placement[component].cluster >= 0)
            continue;

        const int cluster = n_clusters++;
        placement[component].cluster = cluster;
        placed[component] = 1;
        queue.push_back(component);

        while (!queue.empty())
        {
            const int anchor = queue.front();
            queue.pop_front();

            // Every unplaced neighbour is taken with ALL the links that hold it to
            // components already placed: a chelating ligand reaches its partner
            // twice, and satisfying only the first bond leaves the second at
            // whatever length happens to come out.
            std::vector<int> neighbours;
            std::map<int, std::vector<const Link*>> holding;

            for (const Link& link : _links)
            {
                const bool from_anchor = link.begin.component == anchor;
                if (!from_anchor && link.end.component != anchor)
                    continue;

                const int other = from_anchor ? link.end.component : link.begin.component;
                if (placed[other])
                    continue;

                if (holding.find(other) == holding.end())
                    neighbours.push_back(other);
                holding[other].push_back(&link);
            }

            for (int neighbour : neighbours)
            {
                std::vector<const Link*>& links = holding[neighbour];

                // Links to any other component already placed hold it too.
                for (const Link& link : _links)
                {
                    const bool to_neighbour = link.end.component == neighbour;
                    if (!to_neighbour && link.begin.component != neighbour)
                        continue;

                    const int other = to_neighbour ? link.begin.component : link.end.component;
                    if (other == anchor || !placed[other])
                        continue;
                    if (std::find(links.begin(), links.end(), &link) == links.end())
                        links.push_back(&link);
                }

                _place(neighbour, links, position, placed, placement);

                placement[neighbour].cluster = cluster;
                placed[neighbour] = 1;
                queue.push_back(neighbour);
            }
        }
    }

    return n_clusters;
}
