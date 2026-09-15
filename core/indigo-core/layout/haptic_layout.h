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

#ifndef __haptic_layout_h__
#define __haptic_layout_h__

// Layout of haptic bonds: feature #3233, ticket #3844.

#include <vector>

#include "base_cpp/array.h"
#include "math/algebra.h"
#include "molecule/molecule_haptic_bonds.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class BaseMolecule;

    // Places the components a haptic bond joins, which the layout forces cannot see
    // because the bond is not an edge. Answers in shifts and never writes to the
    // molecule, so coordinates keep a single assignment point in the caller.
    // Never throws: a bond it cannot act on is left to the ordinary grid.
    //
    // A bond fixes a distance and leaves a direction and a rotation free, and those
    // are chosen by what the drawing costs: two components of one cluster may not
    // cross or sit on top of each other, and the geometric rules of #3233 - the
    // widest gap, the right angle across a two-atom group - only break the tie.
    // Choosing them by rule alone is what put a ligand through a metal's other
    // bonds, because a rule reads one atom and not the body hanging off it.
    class DLLEXPORT HapticLayout
    {
    public:
        // Where one component ends up: p' = rotate(p, rotation) + shift, about the
        // origin of the component's own coordinates. A chelating ligand is held by
        // two haptic bonds at once and cannot satisfy both by translation alone,
        // which is why the answer carries a rotation.
        struct Placement
        {
            int cluster = -1;
            float rotation = 0.f;
            Vec2f shift;

            Vec2f apply(const Vec2f& point) const
            {
                const float c = cosf(rotation), s = sinf(rotation);
                return Vec2f(point.x * c - point.y * s + shift.x, point.x * s + point.y * c + shift.y);
            }
        };

        // `group_bond_multiplier` is the length of a group-to-atom bond in standard
        // bond lengths; an atom-to-atom one always keeps the ordinary length.
        HapticLayout(BaseMolecule& molecule, float bond_length, float group_bond_multiplier);

        // `component_of` and `position` are indexed by atom, -1 for an atom outside
        // the layout; `frozen` and `placement` by component. A frozen component is
        // never moved and never joins a cluster. A component no haptic bond reaches
        // becomes a cluster of its own with an identity placement, in the original
        // order - a molecule without haptic bonds comes out unchanged. Returns the
        // number of clusters.
        int plan(int n_components, const Array<int>& component_of, const Array<Vec2f>& position, const Array<int>& frozen, Array<Placement>& placement);

    private:
        // One end of a haptic bond, resolved against the components.
        struct Endpoint
        {
            bool is_group = false;
            int index = -1;              // group index, or atom index
            int component = -1;          // the component every atom of it belongs to
            std::vector<int> atoms;      // the member atoms of a group, or the single atom
            std::vector<int> neighbours; // atoms outside the endpoint that bond to it
        };

        struct Link
        {
            Endpoint begin;
            Endpoint end;
            bool group_end = false; // an end is a group: the multiplier applies
        };

        // A component as a rigid body - the shape a placement has to keep clear of.
        // Held here because a haptic bond says nothing about the atoms hanging off
        // its ends, and those are what collide.
        struct Body
        {
            std::vector<Vec2f> local;               // atom positions in the component's own frame
            std::vector<std::pair<int, int>> bonds; // pairs of indices into `local`
            Vec2f centre;
            float radius = 0.f;
        };

        // A component already placed, with its atoms where they now stand.
        struct Placed
        {
            int component = -1;
            std::vector<Vec2f> world;
            Vec2f centre;
            float radius = 0.f;
        };

        // Where one bond holds a component: the point of the component it holds, in
        // the component's own coordinates, the point it is held from, already
        // placed, and the length it asks for.
        struct Held
        {
            Vec2f source;
            Vec2f anchor;
            float length = 0.f;
        };

        // A placement being considered, before it is known to be the best one.
        struct Candidate
        {
            Placement placement;
            float direction = 0.f; // of the bond that holds the component, for the prior
            float rotation = 0.f;  // of the component itself, likewise
            float stretch = 1.f;   // how much longer than nominal its bonds are drawn
        };

        // Fills _links with the bonds this class can act on: haptic, both ends
        // resolved, ends in two different components, neither of them frozen.
        void _collectLinks(const Array<int>& component_of, const Array<int>& frozen);

        bool _resolveEndpoint(const HapticBond::Endpoint& endpoint, const Array<int>& component_of, Endpoint& resolved) const;

        static bool _sameEndpoint(const Endpoint& left, const Endpoint& right);

        // A group answers AttachmentGroup::centreOf() over its members, not the
        // position of its anchor atom: the anchor belongs to the partner's
        // component, so its place is fixed before this class gets to choose one.
        static Vec2f _endpointPos(const Endpoint& endpoint, const Array<Vec2f>& position);
        static Vec2f _placedEndpointPos(const Endpoint& endpoint, const Array<Vec2f>& position, const Array<Placement>& placement);

        // The axis of a two- or three-atom group, in the component's own
        // coordinates; false for a ring, which has none.
        static bool _groupAxis(const Endpoint& endpoint, const Array<Vec2f>& position, Vec2f& axis);

        // True when the group is a line rather than a disc (an eta-2 or eta-3 end),
        // answering the direction across it - the one that leaves the ligand's own
        // bonds at a right angle instead of running over them.
        static bool _acrossTheGroup(const Endpoint& from, const Vec2f& from_pos, const Array<Vec2f>& position, const Array<Placement>& placement, Vec2f& axis);

        // Bisector of the widest angular gap between the directions already taken
        // around the endpoint. Gives 120 degrees on a hexagon and 126 on a
        // cyclopentadienyl ring, which is why neither number is a constant here.
        static Vec2f _freeDirection(const Endpoint& from, const Vec2f& from_pos, const std::vector<Vec2f>& taken, const Array<Vec2f>& position,
                                    const Array<Placement>& placement);

        // The length a bond of this link has to end up with.
        float _lengthOf(const Link& link) const;

        // What holds this component, one entry per link, with every length taken
        // `stretch` times its nominal value.
        std::vector<Held> _heldBy(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position, const Array<Placement>& placement,
                                  float stretch) const;

        // Every placement that satisfies the links holding this component: each of
        // them puts the bonds at their length, and they differ in where around the
        // partner the component sits and how it is turned.
        // `preferred` is the direction the rules of #3233 ask for: it is in the set
        // exactly, so a grid step never costs the angle they derive. `stretch` is
        // how much longer than nominal the bonds of this component are drawn.
        void _candidatesAt(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position, const Array<Placement>& placement,
                           float preferred, float stretch, std::vector<Candidate>& out) const;

        // The four ways a component can be held, each with its own construction: one
        // bond leaves the direction and the rotation free; two bonds to two partners
        // meet at the intersection of two circles; two bonds to one partner put the
        // held points on a chord of one; anything else is fitted numerically.
        static Candidate _candidateAt(const Held& held, float direction, float rotation, const Vec2f& target);
        void _aroundOnePartner(int component, const Link& link, const std::vector<Held>& held, const Array<Vec2f>& position, float preferred, int rotations,
                               std::vector<Candidate>& out) const;
        void _betweenTwoPartners(const std::vector<Held>& held, int rotations, std::vector<Candidate>& out) const;
        static void _onAChordOfOnePartner(const std::vector<Held>& held, const Vec2f& source_centre, float preferred, std::vector<Candidate>& out);
        void _byFitting(int component, const std::vector<const Link*>& links, const std::vector<Held>& held, const Array<Vec2f>& position,
                        const Array<Placement>& placement, float stretch, int rotations, std::vector<Candidate>& out) const;

        // Offers every component of `order` its choice again, now that the rest of
        // the cluster stands around it. Stops as soon as a sweep changes nothing or
        // leaves nothing overlapping.
        void _sweep(const std::vector<int>& order, const Array<Vec2f>& position, const Array<int>& placed, Array<Placement>& placement) const;

        // The direction the rules of #3233 ask the bond to leave `from` in, as an
        // angle: the widest gap left by the bonds and the haptic partners around it.
        float _preferredDirection(int component, const Endpoint& from, const Vec2f& from_pos, const Array<Vec2f>& position, const Array<int>& placed,
                                  const Array<Placement>& placement) const;

        // Places `component` where it costs the drawing least of everything that
        // satisfies its links, and answers what that placement still overlaps.
        double _place(int component, int cluster, const std::vector<const Link*>& links, const Array<Vec2f>& position, const Array<int>& placed,
                      Array<Placement>& placement) const;

        // Turns `current` towards the lengths the bonds ask for, as a rigid body,
        // from wherever it stands - the general case of several links, where no
        // construction gives the answer outright.
        void _refine(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position, const Array<Placement>& placement, float stretch,
                     Placement& current, int passes) const;

        // True when every link reaches its length, within what a drawing can show.
        bool _fits(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position, const Array<Placement>& placement,
                   const Placement& candidate, float stretch) const;

        void _buildBodies(int n_components, const Array<int>& component_of, const Array<Vec2f>& position);

        // The components of this cluster that are already placed, `except` left out.
        std::vector<Placed> _placedBodies(int cluster, int except, const Array<int>& placed, const Array<Placement>& placement) const;

        // What a drawing pays for this placement. `overlapping` collects the part of
        // it that is a defect rather than a preference - bonds crossing, an atom on
        // a bond, two atoms in the same place - which is what a longer bond may be
        // spent on and what another sweep is worth trying for.
        double _cost(int component, const Placement& candidate, const std::vector<Placed>& placed, double& overlapping) const;
        double _lineCost(const Vec2f& from, const Vec2f& to, int skip, const std::vector<Placed>& placed) const;

        BaseMolecule& _molecule;
        float _bond_length;
        float _group_bond_multiplier;

        // The bonds that take part, with both ends resolved. Filled by plan().
        std::vector<Link> _links;

        // One entry per component, in the caller's numbering. Filled by plan().
        std::vector<Body> _bodies;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
