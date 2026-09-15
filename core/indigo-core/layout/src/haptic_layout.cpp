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

// How close atoms of two different components may come, in bond lengths: below
// REPULSION they crowd each other, below COLLISION two labels are drawn on top of
// each other. An atom needs less room against a bond than against another atom -
// a line is thin where a label is not - which is what ON_BOND is for: a metal in
// the bay of a chelating ring passes it, and one drawn over the ring does not.
static const float REPULSION = 1.6f;
static const float COLLISION = 0.5f;
static const float ON_BOND = 0.3f;

// What a drawing pays for a placement. Two bonds crossing is the worst thing it
// can show and an atom sitting on a bond is next; the geometric rules of #3233
// only choose between placements that cost the same.
static const double CROSSING_COST = 15.;
static const double ON_BOND_COST = 20.;
static const double COLLISION_COST = 40.;
static const double CROWDING_COST = 2.;
static const double LINE_COST = 3.;
static const double PRIOR_COST = 0.35;

// The directions and rotations tried for a component. Fifteen degrees is finer
// than the difference a chemist would notice and coarse enough to stay cheap.
static const int DIRECTIONS = 24;
static const int ROTATIONS = 12;

// Passes of the rigid fit when several bonds hold a component and no construction
// answers outright, and the length error a drawing can still show.
static const int REFINE_PASSES = 40;
static const float LENGTH_TOLERANCE = 0.02f;

// Sweeps over the cluster after the first pass: a component placed before its
// neighbours existed is given the chance to move out of their way.
static const int SWEEPS = 3;

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

std::vector<HapticLayout::Held> HapticLayout::_heldBy(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position,
                                                      const Array<Placement>& placement, float stretch) const
{
    std::vector<Held> held;
    held.reserve(links.size());

    for (const Link* link : links)
    {
        const bool from_begin = link->end.component == component;

        Held entry;
        entry.source = _endpointPos(from_begin ? link->end : link->begin, position);
        entry.anchor = _placedEndpointPos(from_begin ? link->begin : link->end, position, placement);
        entry.length = _lengthOf(*link) * stretch;
        held.push_back(entry);
    }

    return held;
}

void HapticLayout::_buildBodies(int n_components, const Array<int>& component_of, const Array<Vec2f>& position)
{
    _bodies.assign(n_components, Body());

    std::map<int, int> row; // atom -> where it sits in the body of its component
    for (int atom = 0; atom < component_of.size(); atom++)
    {
        const int component = component_of[atom];
        if (component < 0 || component >= n_components)
            continue;

        row[atom] = static_cast<int>(_bodies[component].local.size());
        _bodies[component].local.push_back(position[atom]);
    }

    for (int e = _molecule.edgeBegin(); e != _molecule.edgeEnd(); e = _molecule.edgeNext(e))
    {
        const Edge& edge = _molecule.getEdge(e);
        if (edge.beg >= component_of.size() || edge.end >= component_of.size())
            continue;

        const int component = component_of[edge.beg];
        if (component < 0 || component != component_of[edge.end])
            continue;

        _bodies[component].bonds.push_back(std::make_pair(row[edge.beg], row[edge.end]));
    }

    for (Body& body : _bodies)
    {
        if (body.local.empty())
            continue;

        for (const Vec2f& point : body.local)
            body.centre.add(point);
        body.centre.scale(1.f / body.local.size());

        for (const Vec2f& point : body.local)
            body.radius = std::max(body.radius, Vec2f::dist(body.centre, point));
    }
}

std::vector<HapticLayout::Placed> HapticLayout::_placedBodies(int cluster, int except, const Array<int>& placed, const Array<Placement>& placement) const
{
    std::vector<Placed> out;

    for (int component = 0; component < static_cast<int>(_bodies.size()); component++)
    {
        if (component == except || !placed[component] || placement[component].cluster != cluster || _bodies[component].local.empty())
            continue;

        Placed entry;
        entry.component = component;
        entry.world.reserve(_bodies[component].local.size());
        for (const Vec2f& point : _bodies[component].local)
            entry.world.push_back(placement[component].apply(point));
        entry.centre = placement[component].apply(_bodies[component].centre);
        entry.radius = _bodies[component].radius;
        out.push_back(entry);
    }

    return out;
}

double HapticLayout::_cost(int component, const Placement& candidate, const std::vector<Placed>& placed, double& overlapping) const
{
    const Body& body = _bodies[component];
    if (body.local.empty() || placed.empty())
        return 0.;

    std::vector<Vec2f> world;
    world.reserve(body.local.size());
    for (const Vec2f& point : body.local)
        world.push_back(candidate.apply(point));

    const Vec2f centre = candidate.apply(body.centre);
    const float repulsion = REPULSION * _bond_length;
    const float collision = COLLISION * _bond_length;
    const float on_bond = ON_BOND * _bond_length;

    double cost = 0.;
    for (const Placed& other : placed)
    {
        if (Vec2f::dist(centre, other.centre) > body.radius + other.radius + repulsion)
            continue;

        for (const Vec2f& mine : world)
            for (const Vec2f& theirs : other.world)
            {
                const float distance = Vec2f::dist(mine, theirs);
                if (distance < collision)
                    overlapping += COLLISION_COST * (1. + (collision - distance) / collision);
                else if (distance < repulsion)
                {
                    const double crowding = (repulsion - distance) / (repulsion - collision);
                    cost += CROWDING_COST * crowding * crowding;
                }
            }

        const std::vector<std::pair<int, int>>& their_bonds = _bodies[other.component].bonds;

        for (const Vec2f& mine : world)
            for (const std::pair<int, int>& bond : their_bonds)
                if (Vec2f::distPointSegment(mine, other.world[bond.first], other.world[bond.second]) < on_bond)
                    overlapping += ON_BOND_COST;

        for (const Vec2f& theirs : other.world)
            for (const std::pair<int, int>& bond : body.bonds)
                if (Vec2f::distPointSegment(theirs, world[bond.first], world[bond.second]) < on_bond)
                    overlapping += ON_BOND_COST;

        for (const std::pair<int, int>& mine : body.bonds)
            for (const std::pair<int, int>& theirs : their_bonds)
                if (Vec2f::segmentsIntersectInternal(world[mine.first], world[mine.second], other.world[theirs.first], other.world[theirs.second]))
                    overlapping += CROSSING_COST;
    }

    return cost;
}

double HapticLayout::_lineCost(const Vec2f& from, const Vec2f& to, int skip, const std::vector<Placed>& placed) const
{
    // The bond itself may cross a ligand it belongs to - it ends at the centre of
    // the group, so it has to cross the ring. Anything else it runs over is a line
    // the reader has to disentangle.
    double cost = 0.;
    const float on_bond = ON_BOND * _bond_length;

    for (const Placed& other : placed)
    {
        if (other.component == skip)
            continue;

        for (const std::pair<int, int>& bond : _bodies[other.component].bonds)
            if (Vec2f::segmentsIntersectInternal(from, to, other.world[bond.first], other.world[bond.second]))
                cost += LINE_COST;

        for (const Vec2f& point : other.world)
            if (Vec2f::distPointSegment(point, from, to) < on_bond)
                cost += LINE_COST;
    }

    return cost;
}

bool HapticLayout::_fits(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position, const Array<Placement>& placement,
                         const Placement& candidate, float stretch) const
{
    for (const Link* link : links)
    {
        const bool from_begin = link->end.component == component;
        const Vec2f mine = candidate.apply(_endpointPos(from_begin ? link->end : link->begin, position));
        const Vec2f theirs = _placedEndpointPos(from_begin ? link->begin : link->end, position, placement);

        if (fabs(Vec2f::dist(mine, theirs) - _lengthOf(*link) * stretch) > LENGTH_TOLERANCE * _bond_length)
            return false;
    }

    return true;
}

HapticLayout::Candidate HapticLayout::_candidateAt(const Held& held, float direction, float rotation, const Vec2f& target)
{
    Candidate candidate;
    candidate.direction = direction;
    candidate.rotation = rotation;
    candidate.placement.rotation = rotation;
    candidate.placement.shift.zero();
    candidate.placement.shift.diff(target, candidate.placement.apply(held.source));
    return candidate;
}

void HapticLayout::_aroundOnePartner(int component, const Link& link, const std::vector<Held>& held, const Array<Vec2f>& position, float preferred,
                                     int rotations, std::vector<Candidate>& out) const
{
    const auto target_at = [&held](float direction) {
        Vec2f target(held[0].anchor);
        target.addScaled(Vec2f(cosf(direction), sinf(direction)), held[0].length);
        return target;
    };

    for (int r = 0; r < rotations; r++)
    {
        const float rotation = _2FLOAT(2. * M_PI) * r / rotations;
        for (int d = 0; d < DIRECTIONS; d++)
        {
            const float direction = _2FLOAT(2. * M_PI) * d / DIRECTIONS;
            out.push_back(_candidateAt(held[0], direction, rotation, target_at(direction)));
        }
    }

    // What the rules of #3233 answer on their own, exactly rather than to the
    // nearest step of the grid: the widest gap gives 126 degrees on a
    // cyclopentadienyl ring and the group axis a right angle, and neither number is
    // reachable by a grid that knows nothing about them.
    const Endpoint& to = link.end.component == component ? link.end : link.begin;

    float rule_rotation = 0.f;
    Vec2f axis;
    if (rotations > 1 && _groupAxis(to, position, axis))
    {
        const Vec2f along(cosf(preferred), sinf(preferred));
        rule_rotation = atan2f(-along.x, along.y) - atan2f(axis.y, axis.x);
    }

    out.push_back(_candidateAt(held[0], preferred, rule_rotation, target_at(preferred)));
}

void HapticLayout::_betweenTwoPartners(const std::vector<Held>& held, int rotations, std::vector<Candidate>& out) const
{
    // A component held at one point by two bonds - a metal between two ligands -
    // stands where the two circles meet. The midpoint between the partners is a
    // saddle no iteration would leave, so the answer is constructed. Circles that
    // touch rather than cut are the ordinary sandwich, where the two solutions
    // coincide; only a gap between them has no answer at all.
    const float apart = Vec2f::dist(held[0].anchor, held[1].anchor);
    if (apart < EPSILON)
        return;

    const float along = (held[0].length * held[0].length - held[1].length * held[1].length + apart * apart) / (2.f * apart);
    const float reach = held[0].length * held[0].length - along * along;
    if (reach < -LENGTH_TOLERANCE * _bond_length)
        return;

    Vec2f axis;
    axis.diff(held[1].anchor, held[0].anchor);
    axis.scale(1.f / apart);

    Vec2f foot(held[0].anchor);
    foot.addScaled(axis, along);

    const float across = sqrtf(std::max(0.f, reach));

    for (int side = 0; side < 2; side++)
    {
        Vec2f target(foot);
        target.addScaled(Vec2f(-axis.y, axis.x), side == 0 ? across : -across);

        Vec2f direction;
        direction.diff(target, held[0].anchor);
        const float angle = atan2f(direction.y, direction.x);

        for (int r = 0; r < rotations; r++)
            out.push_back(_candidateAt(held[0], angle, _2FLOAT(2. * M_PI) * r / rotations, target));
    }
}

void HapticLayout::_onAChordOfOnePartner(const std::vector<Held>& held, const Vec2f& source_centre, float preferred, std::vector<Candidate>& out)
{
    // Two points of this component held by one and the same partner - an eta-2
    // ligand, a chelating diene. They keep their distance, so they are the ends of a
    // chord of the circle of radius L around the partner: the midpoint of the chord
    // stands sqrt(L^2 - (d/2)^2) away and the chord is perpendicular to that
    // direction. Which way round the partner it goes is still free.
    const float half_chord = Vec2f::dist(held[0].source, held[1].source) / 2.f;
    const float wanted = (held[0].length + held[1].length) / 2.f;
    if (half_chord >= wanted)
        return;

    const float height = sqrtf(wanted * wanted - half_chord * half_chord);

    Vec2f chord;
    chord.diff(held[1].source, held[0].source);
    const float chord_angle = atan2f(chord.y, chord.x);

    const auto chord_at = [&](float direction) {
        const Vec2f along(cosf(direction), sinf(direction));

        Vec2f target(held[0].anchor);
        target.addScaled(along, height);

        Candidate candidate;
        candidate.direction = direction;
        candidate.rotation = atan2f(-along.x, along.y) - chord_angle;
        candidate.placement.rotation = candidate.rotation;
        candidate.placement.shift.zero();
        candidate.placement.shift.diff(target, candidate.placement.apply(source_centre));
        out.push_back(candidate);
    };

    for (int d = 0; d < DIRECTIONS; d++)
        chord_at(_2FLOAT(2. * M_PI) * d / DIRECTIONS);
    chord_at(preferred);
}

void HapticLayout::_byFitting(int component, const std::vector<const Link*>& links, const std::vector<Held>& held, const Array<Vec2f>& position,
                              const Array<Placement>& placement, float stretch, int rotations, std::vector<Candidate>& out) const
{
    // A ligand bridging two metals, a metal held at two different atoms: no
    // construction answers, so the fit is started from each direction in turn and
    // the ones that come out at the right lengths are kept. The whole grid is needed
    // here rather than a coarser one - the fit is a descent, and which answer a seed
    // reaches is not a continuous function of where it started.
    for (int r = 0; r < rotations; r++)
    {
        const float rotation = _2FLOAT(2. * M_PI) * r / rotations;
        for (int d = 0; d < DIRECTIONS; d++)
        {
            const float direction = _2FLOAT(2. * M_PI) * d / DIRECTIONS;

            Vec2f target(held[0].anchor);
            target.addScaled(Vec2f(cosf(direction), sinf(direction)), held[0].length);

            Candidate candidate = _candidateAt(held[0], direction, rotation, target);
            _refine(component, links, position, placement, stretch, candidate.placement, REFINE_PASSES);

            Vec2f leaving;
            leaving.diff(candidate.placement.apply(held[0].source), held[0].anchor);
            candidate.direction = atan2f(leaving.y, leaving.x);
            candidate.rotation = candidate.placement.rotation;

            out.push_back(candidate);
        }
    }
}

void HapticLayout::_candidatesAt(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position, const Array<Placement>& placement,
                                 float preferred, float stretch, std::vector<Candidate>& out) const
{
    out.clear();
    if (links.empty())
        return;

    const std::vector<Held> held = _heldBy(component, links, position, placement, stretch);

    // A single atom has nothing to turn; a body does, and the turn is what decides
    // whether its substituents point into the partner or away from it.
    const int rotations = _bodies[component].radius > EPSILON ? ROTATIONS : 1;

    if (links.size() == 1)
    {
        _aroundOnePartner(component, *links.front(), held, position, preferred, rotations, out);
        return;
    }

    Vec2f source_centre;
    for (const Held& entry : held)
        source_centre.add(entry.source);
    source_centre.scale(1.f / held.size());

    float spread = 0.f;
    for (const Held& entry : held)
        spread = std::max(spread, Vec2f::dist(entry.source, source_centre));

    if (spread < EPSILON)
        _betweenTwoPartners(held, rotations, out);
    else if (Vec2f::dist(held[0].anchor, held[1].anchor) < EPSILON)
        _onAChordOfOnePartner(held, source_centre, preferred, out);

    if (out.empty())
        _byFitting(component, links, held, position, placement, stretch, rotations, out);

    // A construction can miss a third bond, and a fit can fail to converge.
    std::vector<Candidate> fitting;
    for (const Candidate& candidate : out)
        if (_fits(component, links, position, placement, candidate.placement, stretch))
            fitting.push_back(candidate);

    if (!fitting.empty())
        out.swap(fitting);
}

float HapticLayout::_preferredDirection(int component, const Endpoint& from, const Vec2f& from_pos, const Array<Vec2f>& position, const Array<int>& placed,
                                        const Array<Placement>& placement) const
{
    // Haptic partners already placed take up room around the endpoint too, so the
    // widest gap is measured against them as well as against the bonds.
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

    const Vec2f preferred = _freeDirection(from, from_pos, taken, position, placement);
    return atan2f(preferred.y, preferred.x);
}

double HapticLayout::_place(int component, int cluster, const std::vector<const Link*>& links, const Array<Vec2f>& position, const Array<int>& placed,
                            Array<Placement>& placement) const
{
    if (links.empty())
        return 0.;

    const Link& first = *links.front();
    const bool from_begin = first.end.component == component;
    const Endpoint& from = from_begin ? first.begin : first.end;
    const Endpoint& to = from_begin ? first.end : first.begin;

    const Vec2f from_pos = _placedEndpointPos(from, position, placement);
    const float preferred_angle = _preferredDirection(component, from, from_pos, position, placed, placement);

    // A two- or three-atom end is a line and the bond has to cross it rather than
    // run along it - a preference here, not a construction, because a ligand held
    // twice cannot have both the right angle and the right length.
    Vec2f axis;
    const bool has_axis = _groupAxis(to, position, axis);

    const std::vector<Placed> others = _placedBodies(cluster, component, placed, placement);

    // The chemist's own drawings keep a haptic bond at its nominal length where
    // there is room and stretch it where there is not - to 2.3 bond lengths on a
    // cycloheptatrienyl molybdenum, to 3.9 on the one bond of the set that joins
    // two atoms. So the length is a floor, and the extra is paid for. A longer
    // bond is only looked at when the shorter one leaves something overlapping.
    static const float STRETCHES[] = {1.f, 1.25f, 1.5f, 2.f, 2.75f, 4.f};

    // What the choice is made on, in this order: what still overlaps, then how far
    // the bond had to be stretched, then how crowded the result is and how far it
    // is from what the rules of #3233 ask for. So a longer bond is drawn to clear
    // an overlap, never for room it does not need, and never longer than it must.
    bool found = false;
    double best_overlap = 0.;
    float best_stretch = 0.f;
    double best_soft = 0.;
    Placement best;

    std::vector<Candidate> candidates;
    for (float stretch : STRETCHES)
    {
        _candidatesAt(component, links, position, placement, preferred_angle, stretch, candidates);
        for (Candidate& candidate : candidates)
            candidate.stretch = stretch;

        // Where the component stands now is a candidate too, so a second look at it
        // can only improve on the first.
        if (stretch == 1.f && placed[component])
        {
            Candidate current;
            current.placement = placement[component];
            current.rotation = placement[component].rotation;

            Vec2f held;
            held.diff(current.placement.apply(_endpointPos(to, position)), from_pos);
            current.direction = atan2f(held.y, held.x);
            current.stretch = _lengthOf(first) > EPSILON ? held.length() / _lengthOf(first) : 1.f;
            candidates.push_back(current);
        }

        for (const Candidate& candidate : candidates)
        {
            double overlap = 0.;
            double soft = _cost(component, candidate.placement, others, overlap);
            overlap += _lineCost(from_pos, candidate.placement.apply(_endpointPos(to, position)), from.component, others);
            soft += PRIOR_COST * (1. - cos(candidate.direction - preferred_angle)) / 2.;

            if (has_axis)
            {
                Vec2f turned(axis);
                turned.rotate(candidate.rotation);
                soft += PRIOR_COST * fabs(Vec2f::dot(turned, Vec2f(cosf(candidate.direction), sinf(candidate.direction))));
            }

            bool better = !found || overlap < best_overlap - 1e-6;
            if (!better && found && overlap < best_overlap + 1e-6)
            {
                better = candidate.stretch < best_stretch - 1e-4f;
                if (!better && candidate.stretch < best_stretch + 1e-4f)
                    better = soft < best_soft - 1e-6;
            }

            if (better)
            {
                found = true;
                best_overlap = overlap;
                best_stretch = candidate.stretch;
                best_soft = soft;
                best = candidate.placement;
            }
        }

        if (found && best_overlap <= 0.)
            break;
    }

    if (!found)
        return 0.;

    const int cluster_of = placement[component].cluster;
    placement[component] = best;
    placement[component].cluster = cluster_of;

    return best_overlap;
}

void HapticLayout::_refine(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position, const Array<Placement>& placement, float stretch,
                           Placement& current, int passes) const
{
    // No single translation satisfies several bonds at once. Each pass turns the
    // component towards the lengths they ask for (the planar case of Kabsch,
    // skipped when the held points coincide and a rotation would mean nothing) and
    // then corrects the translation by the remaining errors. The component moves as
    // a rigid body throughout, so the ligand keeps its own geometry.
    const std::vector<Held> held = _heldBy(component, links, position, placement, stretch);

    Vec2f source_centre;
    for (const Held& entry : held)
        source_centre.add(entry.source);
    source_centre.scale(1.f / held.size());

    float spread = 0.f;
    for (const Held& entry : held)
        spread = std::max(spread, Vec2f::dist(entry.source, source_centre));

    for (int pass = 0; pass < passes; pass++)
    {
        std::vector<Vec2f> here;
        here.reserve(held.size());
        for (const Held& entry : held)
            here.push_back(current.apply(entry.source));

        if (spread > EPSILON)
        {
            std::vector<Vec2f> target;
            target.reserve(held.size());
            for (size_t i = 0; i < held.size(); i++)
            {
                Vec2f direction;
                direction.diff(here[i], held[i].anchor);
                if (!direction.normalize())
                    direction = DEFAULT_DIRECTION;

                Vec2f wanted(held[i].anchor);
                wanted.addScaled(direction, held[i].length);
                target.push_back(wanted);
            }

            Vec2f target_centre;
            for (const Vec2f& point : target)
                target_centre.add(point);
            target_centre.scale(1.f / target.size());

            float sin_sum = 0.f, cos_sum = 0.f;
            for (size_t i = 0; i < held.size(); i++)
            {
                Vec2f a, b;
                a.diff(held[i].source, source_centre);
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
                const Vec2f centre_before = current.apply(source_centre);
                current.rotation = atan2f(sin_sum, cos_sum);
                current.shift.zero();
                current.shift.diff(centre_before, current.apply(source_centre));
            }

            here.clear();
            for (const Held& entry : held)
                here.push_back(current.apply(entry.source));
        }

        // What the rotation cannot fix: every bond that is still too long or too
        // short pulls the component along its own direction, and the mean of those
        // pulls is the step.
        Vec2f correction;
        for (size_t i = 0; i < here.size(); i++)
        {
            Vec2f direction;
            direction.diff(here[i], held[i].anchor);
            const float distance = direction.length();
            if (distance < EPSILON || !direction.normalize())
                continue;
            correction.addScaled(direction, held[i].length - distance);
        }
        correction.scale(1.f / here.size());
        current.shift.add(correction);
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
    _buildBodies(n_components, component_of, position);

    // The grid places a cluster as a whole; otherwise it would tear apart what is
    // being put together here.
    int n_clusters = 0;
    std::deque<int> queue;
    std::vector<int> order;  // the components placed here, in the order they were
    double overlapping = 0.; // what the first pass could not keep clear

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

                overlapping += _place(neighbour, cluster, links, position, placed, placement);

                placement[neighbour].cluster = cluster;
                placed[neighbour] = 1;
                order.push_back(neighbour);
                queue.push_back(neighbour);
            }
        }
    }

    // Nothing overlapping means nothing to improve on, which is the ordinary case
    // and the cheap one.
    if (overlapping > 0.)
        _sweep(order, position, placed, placement);

    return n_clusters;
}

void HapticLayout::_sweep(const std::vector<int>& order, const Array<Vec2f>& position, const Array<int>& placed, Array<Placement>& placement) const
{
    // A component placed before its neighbours existed could not know about them.
    // Each sweep offers every component the whole choice again, against everything
    // that now stands around it; a component keeps its place unless another costs
    // strictly less, so the arrangement can only improve.
    for (int sweep = 0; sweep < SWEEPS; sweep++)
    {
        bool moved = false;
        double overlapping = 0.;

        for (int component : order)
        {
            std::vector<const Link*> links;
            for (const Link& link : _links)
                if (link.begin.component == component || link.end.component == component)
                    links.push_back(&link);

            const Placement before = placement[component];
            overlapping += _place(component, placement[component].cluster, links, position, placed, placement);

            if (fabs(before.rotation - placement[component].rotation) > EPSILON || Vec2f::dist(before.shift, placement[component].shift) > EPSILON)
                moved = true;
        }

        if (!moved || overlapping <= 0.)
            break;
    }
}
