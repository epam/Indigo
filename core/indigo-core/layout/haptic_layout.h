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
#include "base_cpp/exception.h"
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
    class DLLEXPORT HapticLayout
    {
    public:
        DECL_ERROR;

        HapticLayout(BaseMolecule& molecule, float bond_length);

        // Length of a group-to-atom haptic bond in standard bond lengths
        // (requirement 4 of #3233); variable attachment (#3731) will want another.
        float group_bond_multiplier;

        // `component_of` and `position` are indexed by atom, -1 for an atom outside
        // the layout; `frozen`, `cluster_of` and `shift` by component. A frozen
        // component is never moved and never joins a cluster. A component no haptic
        // bond reaches becomes a cluster of its own with a zero shift, in the
        // original order - a molecule without haptic bonds comes out unchanged.
        // Returns the number of clusters.
        int plan(int n_components, const Array<int>& component_of, const Array<Vec2f>& position, const Array<int>& frozen, Array<int>& cluster_of,
                 Array<Vec2f>& shift);

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

        bool _resolveEndpoint(const HapticBond::Endpoint& endpoint, const Array<int>& component_of, Endpoint& resolved) const;

        static bool _sameEndpoint(const Endpoint& left, const Endpoint& right);

        // A group answers AttachmentGroup::centreOf() over its members, not the
        // position of its anchor atom: the anchor belongs to the partner's
        // component, so its place is fixed before this class gets to choose one.
        static Vec2f _endpointPos(const Endpoint& endpoint, const Array<Vec2f>& position);
        static Vec2f _shiftedEndpointPos(const Endpoint& endpoint, const Array<Vec2f>& position, const Array<Vec2f>& shift);

        // Bisector of the widest angular gap between the directions already taken
        // around the endpoint. Gives 120 degrees on a hexagon and 126 on a
        // cyclopentadienyl ring, which is why neither number is a constant here.
        static Vec2f _freeDirection(const Endpoint& from, const Vec2f& from_pos, const std::vector<Vec2f>& taken, const Array<Vec2f>& position,
                                    const Array<Vec2f>& shift);

        void _place(const Link& link, bool from_begin, const Array<Vec2f>& position, const Array<int>& placed, Array<Vec2f>& shift) const;

        BaseMolecule& _molecule;
        float _bond_length;

        // The bonds that take part, with both ends resolved. Filled by plan().
        std::vector<Link> _links;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
