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

#include "molecule/molecule_arom_match.h"

#include "base_cpp/obj.h"
#include "graph/filter.h"
#include "graph/spanning_tree.h"
#include "molecule/molecule_dearom.h"
#include "molecule/query_molecule.h"

using namespace indigo;

IMPL_ERROR(AromaticityMatcher, "AromaticityMatcher");

CP_DEF(AromaticityMatcher);

AromaticityMatcher::AromaticityMatcher(QueryMolecule& query, BaseMolecule& base, const AromaticityOptions& arom_options)
    : _query(query), _base(base), CP_INIT, TL_CP_GET(_matching_edges_state)
{
    _submolecule.reset(base.neu());
    _matching_edges_state.clear();

    _arom_options = arom_options;

    validateQuery();
}

bool AromaticityMatcher::isNecessary(QueryMolecule& query)
{
    for (int e = query.edgeBegin(); e < query.edgeEnd(); e = query.edgeNext(e))
    {
        if (!query.aromaticity.canBeAromatic(e))
            continue;

        QueryMolecule::Bond& bond = query.getBond(e);
        // Check if bond isn't aromatic but can be aromatic
        if (bond.possibleValue(QueryMolecule::BOND_ORDER, BOND_SINGLE))
            return true;
        if (bond.possibleValue(QueryMolecule::BOND_ORDER, BOND_DOUBLE))
            return true;
    }

    // Check R-groups
    MoleculeRGroups& rgroups = query.rgroups;
    int n_rgroups = rgroups.getRGroupCount();
    for (int i = 1; i <= n_rgroups; i++)
    {
        PtrPool<BaseMolecule>& frags = rgroups.getRGroup(i).fragments;

        for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
        {
            QueryMolecule& fragment = frags[j]->asQueryMolecule();
            if (AromaticityMatcher::isNecessary(fragment))
                return true;
        }
    }

    return false;
}

void AromaticityMatcher::validateQuery()
{
    int old_size = _matching_edges_state.size();
    _matching_edges_state.resize(_query.edgeEnd());
    for (int i = old_size; i < _query.edgeEnd(); i++)
    {
        _matching_edges_state[i] = 0;
    }
}

bool AromaticityMatcher::canFixQueryBond(int query_edge_idx, bool aromatic)
{
    // Check if bond is fixed then it aromatic state must be the same
    int _bond_state = _matching_edges_state[query_edge_idx];
    if (_bond_state != ANY)
        return (_bond_state == AROMATIC) == aromatic;

    if (aromatic && !_query.aromaticity.canBeAromatic(query_edge_idx))
        return false;

    // MR: TODO: Handle this case in more details.
    // Aromatic bonds connectivity graph can be decomposed
    // into connected linear components and each component
    // is aromatic or not. If one edge in such component is
    // marked as aromatic then all component must be mapped
    // on aromatic bonds in the target and vise-versa.
    return true;
}

void AromaticityMatcher::fixQueryBond(int query_edge_idx, bool aromatic)
{
    int& bond_state = _matching_edges_state[query_edge_idx];
    int new_bond_state = aromatic ? AROMATIC : NONAROMATIC;
    if (bond_state != ANY && bond_state != new_bond_state)
        throw Error("bond has already been fixed with another state");

    bond_state = aromatic ? AROMATIC : NONAROMATIC;
}

void AromaticityMatcher::unfixQueryBond(int query_edge_idx)
{
    if (_matching_edges_state[query_edge_idx] == ANY)
        return;

    _matching_edges_state[query_edge_idx] = ANY;
}

void AromaticityMatcher::unfixNeighbourQueryBond(int query_arom_idx)
{
    const Vertex& vertex = _query.getVertex(query_arom_idx);
    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
        unfixQueryBond(vertex.neiEdge(i));
}

bool AromaticityMatcher::match(int* core_sub, int* core_super)
{
    // Check if detailed checking is necessary
    bool needCheck = false;
    for (int i = _query.edgeBegin(); i != _query.edgeEnd(); i = _query.edgeNext(i))
    {
        if (!_query.getBond(i).hasConstraint(QueryMolecule::BOND_ORDER))
            continue;

        if (_matching_edges_state[i] == AROMATIC && _query.getBondOrder(i) != BOND_AROMATIC)
        {
            needCheck = true;
            break;
        }
    }

    if (!needCheck)
        return true;

    // By our rules submolecule in the query, that maps on aromatic bonds in
    // the target, must have aromatic bond configuration to match the target.
    // To check this such submolecule from query molecule is extracted, then
    // all bonds are marked as aromatic, and then dearomatizer tries to find
    // aromatic bonds configuration with partially fixed bonds.

    // 1. Extract query submolecule that maps on aromatic bonds. It is the same as in target.
    // Set skip all additional informatio during copying
    QS_DEF(Array<int>, mapping);

    mapping.clear();
    for (int v_idx = _query.vertexBegin(); v_idx < _query.vertexEnd(); v_idx = _query.vertexNext(v_idx))
    {
        int target_idx = core_sub[v_idx];
        if (target_idx < 0)
            continue;
        mapping.push(target_idx);
    }

    QS_DEF(Array<int>, edges);
    QS_DEF(Array<int>, base_edges_mask);
    edges.clear();
    base_edges_mask.clear_resize(_base.edgeEnd());
    base_edges_mask.zerofill();
    for (int e_idx = _query.edgeBegin(); e_idx < _query.edgeEnd(); e_idx = _query.edgeNext(e_idx))
    {
        const Edge& e = _query.getEdge(e_idx);
        if (core_sub[e.beg] < 0 || core_sub[e.end] < 0)
            continue;

        int target_idx = _base.findEdgeIndex(core_sub[e.beg], core_sub[e.end]);
        if (target_idx == -1)
            throw Error("(AromaticityMatcher::match) target edge wasn't found");

        edges.push(target_idx);
        base_edges_mask[target_idx] = 1;
    }

    QS_DEF(Array<int>, inv_mapping);
    _submolecule->makeEdgeSubmolecule(_base, mapping, edges, &inv_mapping, SKIP_ALL);

    QS_DEF(Array<int>, external_conn);
    external_conn.resize(_submolecule->vertexEnd());
    external_conn.zerofill();
    // Calculate external connectivity
    for (int i = 0; i < mapping.size(); i++)
    {
        int base_idx = mapping[i];
        const Vertex& v = _base.getVertex(base_idx);
        int cur_external_conn = 0;
        for (int ni = v.neiBegin(); ni != v.neiEnd(); ni = v.neiNext(ni))
        {
            int ni_edge = v.neiEdge(ni);
            if (!base_edges_mask[ni_edge])
            {
                int bond_order_diff = _base.getBondOrder(ni_edge);
                if (bond_order_diff == BOND_AROMATIC)
                    bond_order_diff = 1;

                cur_external_conn += bond_order_diff;
            }
        }
        external_conn[i] = cur_external_conn;
    }

    // 1b. Find bonds in aromatic rings in query and skip aromatic
    // bonds that are not in cycles
    QS_DEF(Array<int>, is_edge_in_aromatic_cycle);
    is_edge_in_aromatic_cycle.clear_resize(_submolecule->edgeEnd());
    is_edge_in_aromatic_cycle.zerofill();
    // At first just mark aromatic bonds
    for (int e_idx = _submolecule->edgeBegin(); e_idx < _submolecule->edgeEnd(); e_idx = _submolecule->edgeNext(e_idx))
    {
        if (_submolecule->getBondOrder(e_idx) == BOND_AROMATIC)
            is_edge_in_aromatic_cycle[e_idx] = 1;
    }
    Filter aromatic_edge_filter(is_edge_in_aromatic_cycle.ptr(), Filter::EQ, 1);
    SpanningTree arom_edges_st(*_submolecule, 0, &aromatic_edge_filter);
    // Store in is_edge_in_aromatic_cycle marks about such bonds
    is_edge_in_aromatic_cycle.zerofill();
    arom_edges_st.markAllEdgesInCycles(is_edge_in_aromatic_cycle.ptr(), 1);

    enum
    {
        AROMATIC_BOND_NOT_IN_AROMATIC_CYCLE = 2
    };

    for (int e_idx = _submolecule->edgeBegin(); e_idx < _submolecule->edgeEnd(); e_idx = _submolecule->edgeNext(e_idx))
    {
        if (_submolecule->getBondOrder(e_idx) == BOND_AROMATIC && is_edge_in_aromatic_cycle[e_idx] == 0)
        {
            // Such bond is marked as aromatic but it isn't in aromatic ring
            // Here just change bond order to nonaromatic (single) and later
            // check if such query bond can be aromatic (not by aromatizer)

            if (_submolecule->isQueryMolecule())
            {
                QueryMolecule& qmol = _submolecule->asQueryMolecule();

                std::unique_ptr<QueryMolecule::Bond> bond(qmol.releaseBond(e_idx));
                bond->removeConstraints(QueryMolecule::BOND_ORDER);

                std::unique_ptr<QueryMolecule::Bond> arom_bond = std::make_unique<QueryMolecule::Bond>(QueryMolecule::BOND_ORDER, BOND_AROMATIC);

                qmol.resetBond(e_idx, QueryMolecule::Bond::und(bond.release(), arom_bond.release()));
            }
            else
                _submolecule->asMolecule().setBondOrder(e_idx, BOND_SINGLE, false);

            is_edge_in_aromatic_cycle[e_idx] = AROMATIC_BOND_NOT_IN_AROMATIC_CYCLE;
        }
    }

    // 2. Try to find suitable dearomatization
    QS_DEF(DearomatizationsStorage, dearomatizations);

    // Dearomatizer and DearomatizationMatcher will be created on demand
    Obj<Dearomatizer> dearomatizer;
    Obj<DearomatizationMatcher> dearomatizationMatcher;

    // Check edges
    for (int e = _submolecule->edgeBegin(); e != _submolecule->edgeEnd(); e = _submolecule->edgeNext(e))
    {
        const Edge& edge = _submolecule->getEdge(e);

        int target_edge_index = _base.findEdgeIndex(mapping[edge.beg], mapping[edge.end]);
        const Edge& target_edge = _base.getEdge(target_edge_index);

        int query_edge_index = _query.findEdgeIndex(core_super[target_edge.beg], core_super[target_edge.end]);
        if (query_edge_index == -1)
            throw Error("(AromaticityMatcher::match) query edge wasn't found");

        if (_matching_edges_state[query_edge_index] != AROMATIC)
            // Such target bond isn't aromatic. So we don't need to fix such bond
            continue;

        QueryMolecule::Bond& qbond = _query.getBond(query_edge_index);

        bool can_have_arom_order = qbond.possibleValuePair(QueryMolecule::BOND_ORDER, BOND_AROMATIC, QueryMolecule::BOND_TOPOLOGY, TOPOLOGY_RING);
        if (can_have_arom_order)
            // This query bond is aromatic.
            continue;

        if (is_edge_in_aromatic_cycle[e] == AROMATIC_BOND_NOT_IN_AROMATIC_CYCLE)
        {
            // Such bond cannot be aromatic without aromatic
            // cycle because 'can_have_arom_order' is false
            return false;
        }

        bool has_other = !qbond.hasNoConstraintExcept({QueryMolecule::BOND_ORDER, QueryMolecule::BOND_TOPOLOGY, QueryMolecule::BOND_ANY});
        if (has_other)
            throw Error("Only bond with order and topology constraints are supported");

        bool can_have_single = qbond.possibleValuePair(QueryMolecule::BOND_ORDER, BOND_SINGLE, QueryMolecule::BOND_TOPOLOGY, TOPOLOGY_RING);
        bool can_have_double = qbond.possibleValuePair(QueryMolecule::BOND_ORDER, BOND_DOUBLE, QueryMolecule::BOND_TOPOLOGY, TOPOLOGY_RING);
        // Check if query bond can be only single or only double
        if (can_have_single && can_have_double)
            // Don't fix such bond. It can be single/double, single/aromatic and etc.
            continue;
        if (!can_have_single && !can_have_double)
            // Bond without and specification and etc.
            continue;

        // Find dearomatization
        if (dearomatizer.get() == NULL)
        {
            dearomatizer.create(*_submolecule, external_conn.ptr(), _arom_options);
            dearomatizer->enumerateDearomatizations(dearomatizations);
            dearomatizationMatcher.create(dearomatizations, *_submolecule, external_conn.ptr());
        }

        // Fix bond
        int type = can_have_single ? BOND_SINGLE : BOND_DOUBLE;

        if (!dearomatizationMatcher->isAbleToFixBond(e, type))
            return false;
        dearomatizationMatcher->fixBond(e, type);
    }

    return true;
}
