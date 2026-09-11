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

        // Places `component` by its first link, then, when more than one link holds
        // it, refines the placement so that every one of them reaches its length -
        // a rigid fit, so the component keeps its own geometry.
        void _place(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position, const Array<int>& placed,
                    Array<Placement>& placement) const;
        void _refine(int component, const std::vector<const Link*>& links, const Array<Vec2f>& position, Array<Placement>& placement) const;

        BaseMolecule& _molecule;
        float _bond_length;
        float _group_bond_multiplier;

        // The bonds that take part, with both ends resolved. Filled by plan().
        std::vector<Link> _links;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
