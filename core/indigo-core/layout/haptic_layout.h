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

    // Placing the components a haptic bond joins: feature #3233, ticket #3844.
    //
    // A haptic bond is not an edge (model #3837), so the layout forces never see
    // it and its two ends land in different connected components. Laid out by the
    // ordinary grid, ferrocene becomes two rings and a metal in separate cells.
    // This class decides where those components go relative to each other; the
    // grid then places the results, see MoleculeLayoutGraph::_layoutMultipleComponents.
    //
    // It answers in shifts and never writes to the molecule, so the caller keeps
    // the single place where coordinates are assigned.
    class DLLEXPORT HapticLayout
    {
    public:
        DECL_ERROR;

        HapticLayout(BaseMolecule& molecule, float bond_length);

        // The 1.5 of requirement 4 of #3233: a group-to-atom haptic bond is that
        // many standard bond lengths long. A parameter and not a constant because
        // variable attachment (#3731) is expected to ask for another value.
        float group_bond_multiplier;

        // `component_of` and `position` are indexed by atom; an atom outside the
        // layout carries component -1. `frozen` is indexed by component and marks
        // the ones the caller will not move (the fixed part of a partial layout):
        // a bond that reaches one is left alone rather than dragging it.
        //
        // Fills `cluster_of` and `shift`, both indexed by component: the cluster a
        // component belongs to, and the translation that puts it where the haptic
        // bonds ask. A component no haptic bond reaches gets a cluster of its own
        // and a zero shift, in the original component order, so a molecule without
        // haptic bonds comes out of here exactly as it came in.
        //
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

        // Where the endpoint sits: an atom's position, or the centre of a group's
        // member atoms — AttachmentGroup::centreOf(), the rule the render and the
        // V3000 saver read the same way (#3844).
        static Vec2f _endpointPos(const Endpoint& endpoint, const Array<Vec2f>& position);
        static Vec2f _shiftedEndpointPos(const Endpoint& endpoint, const Array<Vec2f>& position, const Array<Vec2f>& shift);

        // The direction a bond leaves an endpoint in: the bisector of the widest
        // angular gap between the directions already taken around it — the bonds
        // of the endpoint itself and the haptic partners already placed. That one
        // rule gives 120 degrees on a hexagon, 126 on a cyclopentadienyl ring, and
        // the far side of the metal for the second ring of a sandwich.
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
