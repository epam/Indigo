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

#include "molecule/molecule_substructure_matcher.h"
#include "base_cpp/array.h"
#include "graph/edge_rotation_matcher.h"
#include "graph/filter.h"
#include "graph/graph.h"
#include "graph/graph_affine_matcher.h"
#include "graph/graph_decomposer.h"
#include "math/algebra.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_3d_constraints.h"
#include "molecule/molecule_neighbourhood_counters.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/query_molecule.h"
#include <memory>

using namespace indigo;

CP_DEF(MoleculeSubstructureMatcher::MarkushContext);

MoleculeSubstructureMatcher::MarkushContext::MarkushContext(QueryMolecule& query_, BaseMolecule& target_)
    : CP_INIT, TL_CP_GET(query), TL_CP_GET(query_marking), TL_CP_GET(sites), depth(0)
{
    int i;

    query.clone(query_, 0, 0);

    sites.clear();

    for (i = query.vertexBegin(); i != query.vertexEnd(); i = query.vertexNext(i))
        if (query.isRSite(i))
            sites.push(i);

    query_marking.clear_resize(query.vertexEnd());

    for (i = query.vertexBegin(); i < query.vertexEnd(); i = query.vertexNext(i))
        query_marking[i] = -1;
}

IMPL_ERROR(MoleculeSubstructureMatcher, "molecule substructure matcher");

CP_DEF(MoleculeSubstructureMatcher);

MoleculeSubstructureMatcher::MoleculeSubstructureMatcher(BaseMolecule& target)
    : _target(target), CP_INIT, TL_CP_GET(_3d_constrained_atoms), TL_CP_GET(_unfolded_target_h), TL_CP_GET(_used_target_h)
{
    vertex_equivalence_handler = NULL;
    use_aromaticity_matcher = true;
    use_pi_systems_matcher = false;
    _query = 0;
    match_3d = 0;
    rms_threshold = 0;

    highlight = false;
    find_all_embeddings = false;
    find_unique_embeddings = false;
    find_unique_by_edges = false;
    save_for_iteration = false;
    disable_folding_query_h = false;

    not_ignore_first_atom = false;

    cb_embedding = 0;
    cb_embedding_context = 0;

    fmcache = 0;

    disable_unfolding_implicit_h = false;
    restore_unfolded_h = true;
    _h_unfold = false;

    _query_nei_counters = 0;
    _target_nei_counters = 0;

    _used_target_h.clear_resize(target.vertexEnd());

    // won't ignore target hydrogens because query can contain
    // 3d features, hydrogen isotopes, etc.
}

MoleculeSubstructureMatcher::~MoleculeSubstructureMatcher()
{
}

bool MoleculeSubstructureMatcher::_shouldUnfoldTargetHydrogens_A(QueryMolecule::Atom* atom, bool is_fragment, bool find_all_embeddings)
{
    if (atom->type == QueryMolecule::ATOM_FRAGMENT)
    {
        if (_shouldUnfoldTargetHydrogens(*atom->fragment, true, find_all_embeddings))
            return true;
    }
    else if (atom->type == QueryMolecule::OP_AND || atom->type == QueryMolecule::OP_OR || atom->type == QueryMolecule::OP_NOT)
    {
        int i;

        for (i = 0; i < atom->children.size(); i++)
            if (_shouldUnfoldTargetHydrogens_A((QueryMolecule::Atom*)atom->children[i], is_fragment, find_all_embeddings))
                return true;
    }

    return false;
}

bool MoleculeSubstructureMatcher::shouldUnfoldTargetHydrogens(QueryMolecule& query, bool disable_folding_query_h)
{
    return _shouldUnfoldTargetHydrogens(query, false, disable_folding_query_h);
}

bool MoleculeSubstructureMatcher::_shouldUnfoldTargetHydrogens(QueryMolecule& query, bool is_fragment, bool disable_folding_query_h)
{
    int i, j;

    for (i = query.vertexBegin(); i != query.vertexEnd(); i = query.vertexNext(i))
    {
        // skip R-atoms
        if (query.isRSite(i))
            continue;

        if (query.possibleAtomNumberAndIsotope(i, ELEM_H, 0))
        {
            const Vertex& vertex = query.getVertex(i);

            // Degree 2 or higher => definilely not a hydrogen
            if (vertex.degree() > 1)
                continue;

            // Can be lone hydrogen?
            if (vertex.degree() == 0)
                return true;

            // degree is 1 at this point
            int edge_idx = vertex.neiEdge(vertex.neiBegin());

            // is it is double or triple bond => not hydrogen
            if (query.getBondOrder(edge_idx) > 1)
                continue;

            // ring bond?
            if (query.getBondTopology(edge_idx) == TOPOLOGY_RING)
                continue;

            // can be something other than hydrogen?
            if (query.getAtomNumber(i) == -1)
                return true;
            if (is_fragment && i == query.vertexBegin())
                // If first atom in a fragment is hydrogen then hydrogens should
                // be unfolded because of the matching logic: when fragment will be
                // matched this first hydrogen should match some atom.
                // If hydrogens is not be unfolded in this case then
                // [$([#1][N])]C will not match NC.
                return true;

            // If we need to find all embeddings then query hydrogens cannot be ignored:
            // For example, if we are searching number of matcher for N-[#1] in N then
            // it should 3 instead of 1
            if (disable_folding_query_h)
                return true;

            // Check if hydrogen forms a cis-trans bond or stereocenter
            int nei_vertex_idx = vertex.neiVertex(vertex.neiBegin());
            if (query.stereocenters.exists(nei_vertex_idx))
                return true;

            // For example for this query hydrogens should be unfolded: [H]\\C=C/C
            const Vertex& nei_vertex = query.getVertex(nei_vertex_idx);
            for (int nei = nei_vertex.neiBegin(); nei != nei_vertex.neiEnd(); nei = nei_vertex.neiNext(nei))
            {
                int edge = nei_vertex.neiEdge(nei);
                if (query.cis_trans.getParity(edge) != 0)
                    return true;
            }
        }

        if (_shouldUnfoldTargetHydrogens_A(&query.getAtom(i), is_fragment, disable_folding_query_h))
            return true;
    }

    MoleculeRGroups& rgroups = query.rgroups;

    int n_rgroups = rgroups.getRGroupCount();
    for (i = 1; i <= n_rgroups; i++)
    {
        PtrPool<BaseMolecule>& frags = rgroups.getRGroup(i).fragments;
        for (j = frags.begin(); j != frags.end(); j = frags.next(j))
            if (_shouldUnfoldTargetHydrogens(frags[j]->asQueryMolecule(), is_fragment, disable_folding_query_h))
                return true;
    }

    return false;
}

void MoleculeSubstructureMatcher::setQuery(QueryMolecule& query)
{
    int i;

    if (query.rgroups.getRGroupCount() > 0)
    {
        _markush = std::make_unique<MarkushContext>(query, _target);
        _query = &_markush->query;
    }
    else
    {
        _markush.reset(nullptr);
        _query = &query;
    }

    QS_DEF(Array<int>, ignored);

    ignored.clear_resize(_query->vertexEnd());

    if (!disable_folding_query_h)
        // If hydrogens are folded then the number of the all matchers is different
        markIgnoredQueryHydrogens(*_query, ignored.ptr(), 0, 1);
    else
        ignored.zerofill();

    if (not_ignore_first_atom)
        ignored[_query->vertexBegin()] = 0;

    _3d_constrained_atoms.clear_resize(_query->vertexEnd());
    _3d_constrained_atoms.zerofill();

    {
        Molecule3dConstraintsChecker checker(query.spatial_constraints);

        checker.markUsedAtoms(_3d_constrained_atoms.ptr(), 1);
    }

    if (!disable_unfolding_implicit_h && shouldUnfoldTargetHydrogens(*_query, disable_folding_query_h) && !_target.isQueryMolecule())
    {
        _h_unfold = true;
    }
    else
        _h_unfold = false;

    if (_ee.get() != 0)
        _ee.free();

    _ee.create(_target);
    _ee->cb_match_vertex = _matchAtoms;
    _ee->cb_match_edge = _matchBonds;
    _ee->cb_vertex_remove = _removeAtom;
    _ee->cb_edge_add = _addBond;
    _ee->cb_embedding = _embedding;
    _ee->userdata = this;

    _ee->setSubgraph(*_query);
    for (i = _query->vertexBegin(); i != _query->vertexEnd(); i = _query->vertexNext(i))
    {
        if ((ignored[i] && !_3d_constrained_atoms[i]) || _query->isRSite(i))
            _ee->ignoreSubgraphVertex(i);
    }

    _embeddings_storage.free();
}

QueryMolecule& MoleculeSubstructureMatcher::getQuery()
{
    if (_query == 0)
        throw Error("query not set");

    return *_query;
}

void MoleculeSubstructureMatcher::setNeiCounters(const MoleculeAtomNeighbourhoodCounters* query_counters,
                                                 const MoleculeAtomNeighbourhoodCounters* target_counters)
{
    _query_nei_counters = query_counters;
    _target_nei_counters = target_counters;
}

bool MoleculeSubstructureMatcher::find()
{
    if (_query == 0)
        throw Error("no query");

    if (match_3d != 0 && !_query->have_xyz)
        throw Error("cannot do 3D match without XYZ in the query");

    if (match_3d != 0 && !_target.have_xyz)
        return false;

    if (_h_unfold)
    {
        _target.asMolecule().unfoldHydrogens(&_unfolded_target_h, -1, true);
        _ee->validate();
    }

    if (_canUseEquivalenceHeuristic(*_query))
        _ee->setEquivalenceHandler(vertex_equivalence_handler);
    else
        _ee->setEquivalenceHandler(NULL);

    _used_target_h.zerofill();

    if (use_aromaticity_matcher && AromaticityMatcher::isNecessary(*_query))
        _am.create(*_query, _target, arom_options);
    else
        _am.free();

    if (use_pi_systems_matcher && !_target.isQueryMolecule())
        _pi_systems_matcher.create(_target.asMolecule());
    else
        _pi_systems_matcher.free();

    _3d_constraints_checker.recreate(_query->spatial_constraints);
    _createEmbeddingsStorage();

    int result = _ee->process();

    if (_h_unfold && restore_unfolded_h)
        _removeUnfoldedHydrogens();

    if (!find_all_embeddings)
        return result == 0;
    else
    {
        if (_embeddings_storage.get() == 0)
            return false;
        return !_embeddings_storage->isEmpty();
    }
}

void MoleculeSubstructureMatcher::_createEmbeddingsStorage()
{
    _embeddings_storage.create();
    _embeddings_storage->unique_by_edges = find_unique_by_edges;
    _embeddings_storage->save_edges = save_for_iteration;
    _embeddings_storage->save_mapping = save_for_iteration;
    _embeddings_storage->check_uniquencess = find_unique_embeddings;
}

void MoleculeSubstructureMatcher::_removeUnfoldedHydrogens()
{
    QS_DEF(Array<int>, atoms_to_remove);
    atoms_to_remove.clear();

    for (int i = 0; i < _unfolded_target_h.size(); i++)
        if (_unfolded_target_h[i])
            atoms_to_remove.push(i);

    if (atoms_to_remove.size() > 0)
        _target.removeAtoms(atoms_to_remove);
}

bool MoleculeSubstructureMatcher::findNext()
{
    if (_h_unfold)
        _target.asMolecule().unfoldHydrogens(&_unfolded_target_h, -1, true);

    bool found = _ee->processNext();

    if (_h_unfold && restore_unfolded_h)
        _removeUnfoldedHydrogens();

    return found;
}

bool MoleculeSubstructureMatcher::matchQueryAtom(QueryMolecule::Atom* query, BaseMolecule& target, int super_idx, FragmentMatchCache* fmcache, dword flags)
{
    int i;

    switch (query->type)
    {
    case QueryMolecule::OP_NONE:
        return true;
    case QueryMolecule::OP_AND:
        for (i = 0; i < query->children.size(); i++)
            if (!matchQueryAtom(query->child(i), target, super_idx, fmcache, flags))
                return false;
        return true;
    case QueryMolecule::OP_OR:
        for (i = 0; i < query->children.size(); i++)
            if (matchQueryAtom(query->child(i), target, super_idx, fmcache, flags))
                return true;
        return false;
    case QueryMolecule::OP_NOT:
        return !matchQueryAtom(query->child(0), target, super_idx, fmcache, flags ^ MATCH_DISABLED_AS_TRUE);

    case QueryMolecule::ATOM_NUMBER:
        return query->valueWithinRange(target.getAtomNumber(super_idx));
    case QueryMolecule::ATOM_PSEUDO:
        return target.isPseudoAtom(super_idx) && strcmp(query->alias.ptr(), target.getPseudoAtom(super_idx)) == 0;
    case QueryMolecule::ATOM_TEMPLATE:
        return target.isTemplateAtom(super_idx) && strcmp(query->alias.ptr(), target.getTemplateAtom(super_idx)) == 0;
    case QueryMolecule::ATOM_RSITE:
        return true;
    case QueryMolecule::ATOM_ISOTOPE:
        return query->valueWithinRange(target.getAtomIsotope(super_idx));
    case QueryMolecule::ATOM_CHARGE: {
        if (flags & MATCH_ATOM_CHARGE)
            return query->valueWithinRange(target.getAtomCharge(super_idx));
        return (flags & MATCH_DISABLED_AS_TRUE) != 0;
    }
    case QueryMolecule::ATOM_RADICAL: {
        if (target.isPseudoAtom(super_idx) || target.isRSite(super_idx))
            return false;
        int radical = target.getAtomRadical_NoThrow(super_idx, -1);
        if (radical == -1)
            return false;
        return query->valueWithinRange(radical);
    }
    case QueryMolecule::ATOM_VALENCE: {
        if (flags & MATCH_ATOM_VALENCE)
        {
            if (target.isPseudoAtom(super_idx) || target.isRSite(super_idx))
                return false;
            int valence = target.getAtomValence_NoThrow(super_idx, -1);
            if (valence == -1)
                return false;
            return query->valueWithinRange(valence);
        }
        return (flags & MATCH_DISABLED_AS_TRUE) != 0;
    }
    case QueryMolecule::ATOM_CONNECTIVITY: {
        int conn = target.getVertex(super_idx).degree();
        if (!target.isPseudoAtom(super_idx) && !target.isRSite(super_idx))
            conn += target.asMolecule().getImplicitH_NoThrow(super_idx, 0);
        return query->valueWithinRange(conn);
    }
    case QueryMolecule::ATOM_TOTAL_BOND_ORDER: {
        // TODO: target.isPseudoAtom(super_idx) || target.isRSite(super_idx)
        int conn = target.asMolecule().getAtomConnectivity_NoThrow(super_idx, -1);
        if (conn == -1)
            return false;
        return query->valueWithinRange(conn);
    }
    case QueryMolecule::ATOM_TOTAL_H: {
        if (target.isPseudoAtom(super_idx) || target.isRSite(super_idx) || target.isTemplateAtom(super_idx))
            return false;
        return query->valueWithinRange(target.getAtomTotalH(super_idx));
    }
    case QueryMolecule::ATOM_SUBSTITUENTS:
    case QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN:
        return query->valueWithinRange(target.getAtomSubstCount(super_idx));
    case QueryMolecule::ATOM_SSSR_RINGS:
        return query->valueWithinRange(target.vertexCountSSSR(super_idx));
    case QueryMolecule::ATOM_SMALLEST_RING_SIZE:
        return query->valueWithinRange(target.vertexSmallestRingSize(super_idx));
    case QueryMolecule::ATOM_RING_BONDS:
    case QueryMolecule::ATOM_RING_BONDS_AS_DRAWN:
        return query->valueWithinRange(target.getAtomRingBondsCount(super_idx));
    case QueryMolecule::ATOM_PI_BONDED: {
        return query->valueWithinRange(static_cast<int>(target.asMolecule().isPiBonded(super_idx)));
    }
    case QueryMolecule::ATOM_UNSATURATION:
        return !target.isSaturatedAtom(super_idx);
    case QueryMolecule::ATOM_FRAGMENT: {
        if (fmcache == 0)
            throw Error("unexpected 'fragment' constraint");

        QueryMolecule* fragment = query->fragment.get();
        const char* smarts = fragment->fragment_smarts.ptr();

        if (fragment->vertexCount() == 0)
            throw Error("empty fragment");

        if (smarts != 0 && strlen(smarts) > 0)
        {
            fmcache->expand(super_idx + 1);
            int* value = fmcache->at(super_idx).at2(smarts);

            if (value != 0)
                return *value != 0;
        }

        MoleculeSubstructureMatcher matcher(target.asMolecule());

        matcher.not_ignore_first_atom = true;
        matcher.setQuery(*fragment);
        matcher.fmcache = fmcache;

        bool result = matcher.fix(fragment->vertexBegin(), super_idx);

        if (result)
            result = matcher.find();

        if (smarts != 0 && strlen(smarts) > 0)
        {
            fmcache->expand(super_idx + 1);
            fmcache->at(super_idx).insert(smarts, result ? 1 : 0);
        }

        return result;
    }
    case QueryMolecule::ATOM_AROMATICITY:
        return query->valueWithinRange(target.getAtomAromaticity(super_idx));
    case QueryMolecule::HIGHLIGHTING:
        return query->valueWithinRange((int)target.isAtomHighlighted(super_idx));
    case QueryMolecule::ATOM_IMPLICIT_H:
        return query->valueWithinRange(target.asMolecule().getImplicitH(super_idx));
    case QueryMolecule::ATOM_CHIRALITY:
        return target.stereocenters.getType(super_idx) == MoleculeStereocenters::ATOM_ABS; // TODO: Investigate right way to match
    default:
        throw Error("bad query atom type: %d", query->type);
    }
}

bool MoleculeSubstructureMatcher::matchQueryBond(QueryMolecule::Bond* query, BaseMolecule& target, int sub_idx, int super_idx, AromaticityMatcher* am,
                                                 dword flags)
{
    int i;

    // MR TODO: match topology. Query bond in ring cannot match
    // target bond that is not in ring. But R-groups should be
    // handled carefully

    switch (query->type)
    {
    case QueryMolecule::OP_NONE:
        return true;
    case QueryMolecule::OP_AND:
        for (i = 0; i < query->children.size(); i++)
            if (!matchQueryBond(query->child(i), target, sub_idx, super_idx, am, flags))
                return false;
        return true;
    case QueryMolecule::OP_OR:
        for (i = 0; i < query->children.size(); i++)
            if (matchQueryBond(query->child(i), target, sub_idx, super_idx, am, flags))
                return true;
        return false;
    case QueryMolecule::OP_NOT:
        return !matchQueryBond(query->child(0), target, sub_idx, super_idx, am, flags ^ MATCH_DISABLED_AS_TRUE);

    case QueryMolecule::BOND_ANY:
        return true;
    case QueryMolecule::BOND_ORDER: {
        if (flags & MATCH_BOND_TYPE)
        {
            if (am != 0)
            {
                if (target.getBondOrder(super_idx) == BOND_AROMATIC)
                    return am->canFixQueryBond(sub_idx, true);
                else
                {
                    if (!am->canFixQueryBond(sub_idx, false))
                        return false;
                }
            }
            return target.possibleBondOrder(super_idx, query->value);
        }
        return (flags & MATCH_DISABLED_AS_TRUE) != 0;
    }
    case QueryMolecule::BOND_TOPOLOGY:
        return target.getEdgeTopology(super_idx) == query->value;
    case QueryMolecule::HIGHLIGHTING:
        return query->value == (int)target.isAtomHighlighted(super_idx);
    default:
        throw Error("bad query bond type: %d", query->type);
    }
}

bool MoleculeSubstructureMatcher::_matchAtoms(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata)
{
    MoleculeSubstructureMatcher* self = (MoleculeSubstructureMatcher*)userdata;

    if (self->_h_unfold && (&subgraph == (Graph*)self->_query))
    {
        if (sub_idx < self->_3d_constrained_atoms.size() && self->_3d_constrained_atoms[sub_idx])
            // we can't check 3D constraint on unfolded atom, because it has no actual position
            if (self->_unfolded_target_h[super_idx])
                return false;
    }

    dword match_atoms_flags = 0xFFFFFFFF;
    // If target atom belongs to a pi-system then its charge
    // should be checked after embedding
    if (self->_pi_systems_matcher.get())
    {
        if (self->_pi_systems_matcher->isAtomInPiSystem(super_idx))
            match_atoms_flags &= ~(MATCH_ATOM_CHARGE | MATCH_ATOM_VALENCE);
    }

    QueryMolecule& query = (QueryMolecule&)subgraph;
    BaseMolecule& target = (BaseMolecule&)supergraph;

    if (!target.isPseudoAtom(super_idx) && !target.isRSite(super_idx) && !target.isTemplateAtom(super_idx))
    {
        int q_min_h;
        int t_max_h;
        try
        {
            q_min_h = query.getAtomMinH(sub_idx);
        }
        catch (Exception e)
        {
            q_min_h = 0;
        }
        try
        {
            t_max_h = target.getAtomMaxH(super_idx);
        }
        catch (Exception e)
        {
            t_max_h = 0;
        }
        if (q_min_h > 0 && t_max_h >= 0)
            if (q_min_h > t_max_h)
                return false;
    }

    if (query.components.size() > sub_idx && query.components[sub_idx] > 0)
    {
        int i;

        for (i = query.vertexBegin(); i != query.vertexEnd(); i = query.vertexNext(i))
        {
            if (i == sub_idx)
                continue;

            if (core_sub[i] < 0)
                continue;

            if (query.components.size() <= i || query.components[i] <= 0)
                continue;

            if (query.components[i] == query.components[sub_idx] && target.vertexComponent(core_sub[i]) != target.vertexComponent(super_idx))
                return false;
            if (query.components[i] != query.components[sub_idx] && target.vertexComponent(core_sub[i]) == target.vertexComponent(super_idx))
                return false;
        }
    }

    QueryMolecule::Atom& sub_atom = query.getAtom(sub_idx);

    if (!matchQueryAtom(&sub_atom, target, super_idx, self->fmcache, match_atoms_flags))
        return false;

    if (query.stereocenters.getType(sub_idx) > target.stereocenters.getType(super_idx))
        return false;

    if (self->_query_nei_counters != 0 && self->_target_nei_counters != 0)
    {
        bool use_bond_types = (self->_pi_systems_matcher.get() == 0);
        bool ret = self->_query_nei_counters->testSubstructure(*self->_target_nei_counters, sub_idx, super_idx, use_bond_types);
        if (!ret)
            return false;
    }

    if (self->match_3d == AFFINE)
    {
        QS_DEF(Array<int>, core_sub_full);

        core_sub_full.copy(core_sub, subgraph.vertexEnd());
        core_sub_full[sub_idx] = super_idx;

        GraphAffineMatcher matcher(subgraph, supergraph, core_sub_full.ptr());

        matcher.cb_get_xyz = getAtomPos;

        int total_fixed = query.vertexCount();

        if (query.fixed_atoms.size() > 0)
        {
            matcher.fixed_vertices = &query.fixed_atoms;
            total_fixed = query.fixed_atoms.size();
        }

        if (!matcher.match(self->rms_threshold * sqrt((float)total_fixed)))
            return false;
    }

    return true;
}

bool MoleculeSubstructureMatcher::_matchBonds(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    MoleculeSubstructureMatcher* self = (MoleculeSubstructureMatcher*)userdata;

    dword flags = 0xFFFFFFFF;
    // If target bond belongs to a pi-system then it
    // should be checked after embedding
    if (self->_pi_systems_matcher.get())
    {
        if (self->_pi_systems_matcher->isBondInPiSystem(super_idx))
            flags &= ~MATCH_BOND_TYPE;
    }

    QueryMolecule& query = (QueryMolecule&)subgraph;
    BaseMolecule& target = (BaseMolecule&)supergraph;
    QueryMolecule::Bond& sub_bond = query.getBond(sub_idx);

    if (!matchQueryBond(&sub_bond, target, sub_idx, super_idx, self->_am.get(), flags))
        return false;

    return true;
}

void MoleculeSubstructureMatcher::removeAtom(Graph& subgraph, int sub_idx, AromaticityMatcher* am)
{
    if (am == 0)
        return;

    am->unfixNeighbourQueryBond(sub_idx);
}

void MoleculeSubstructureMatcher::_removeAtom(Graph& subgraph, int sub_idx, void* userdata)
{
    MoleculeSubstructureMatcher* self = (MoleculeSubstructureMatcher*)userdata;

    removeAtom(subgraph, sub_idx, self->_am.get());
}

void MoleculeSubstructureMatcher::addBond(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, AromaticityMatcher* am)
{
    if (am == 0)
        return;

    BaseMolecule& target = (BaseMolecule&)supergraph;
    am->fixQueryBond(sub_idx, target.getBondOrder(super_idx) == BOND_AROMATIC);
}

void MoleculeSubstructureMatcher::_addBond(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    MoleculeSubstructureMatcher* self = (MoleculeSubstructureMatcher*)userdata;

    addBond(subgraph, supergraph, sub_idx, super_idx, self->_am.get());
}

int MoleculeSubstructureMatcher::_embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata)
{
    MoleculeSubstructureMatcher* self = (MoleculeSubstructureMatcher*)userdata;

    if (self->_markush.get() == 0 || self->_markush->sites.size() == self->_markush->depth)
        return self->_embedding_common(core_sub, core_super);
    else
        return self->_embedding_markush(core_sub, core_super);
}

int MoleculeSubstructureMatcher::_embedding_common(int* core_sub, int* core_super)
{
    QueryMolecule& query = *_query;

    if (!MoleculeStereocenters::checkSub(query, _target, core_sub, false))
        return 1;

    if (!MoleculeCisTrans::checkSub(query, _target, core_sub))
        return 1;

    if (!MoleculeAlleneStereo::checkSub(query, _target, core_sub))
        return 1;

    if (!_3d_constraints_checker->check(_target, core_sub))
        return 1;

    // Check possible aromatic configuration
    if (_am.get() != 0)
    {
        if (!_am->match(core_sub, core_super))
            return 1;
    }

    // Check possible pi-systems configurations
    if (_pi_systems_matcher.get() != 0)
    {
        if (!_pi_systems_matcher->checkEmbedding(query, core_sub))
            return 1;
    }

    // affine transformation match
    /*if (match_3d == AFFINE)
    {
       GraphAffineMatcher matcher(query, _target, core_sub);

       matcher.cb_get_xyz = getAtomPos;

       if (query.fixed_atoms.size() > 0)
          matcher.fixed_vertices = &query.fixed_atoms;

       if (!matcher.match(v1, v2))
          return 1;
    }*/

    // chemical conformation match
    if (match_3d == CONFORMATION)
    {
        QS_DEF(Array<int>, mapping);
        GraphDecomposer decomposer(query);

        Filter filter(core_sub, Filter::MORE, -1);
        decomposer.decompose(&filter);

        int i, comp;

        for (comp = 0; comp < decomposer.getComponentsCount(); comp++)
        {
            mapping.clear_resize(query.vertexEnd());
            mapping.fffill();
            for (i = query.vertexBegin(); i != query.vertexEnd(); i = query.vertexNext(i))
            {
                if (decomposer.getComponent(i) == comp)
                    mapping[i] = core_sub[i];
            }

            EdgeRotationMatcher matcher(query, _target, mapping.ptr());

            matcher.cb_get_xyz = getAtomPos;
            matcher.cb_can_rotate = _isSingleBond;
            matcher.equalize_edges = true;

            if (!matcher.match(rms_threshold, 0.1f))
                return 1;
        }
    }

    if (_markush.get() != 0)
        if (!_checkRGroupConditions())
            return 1;

    if (find_unique_embeddings || save_for_iteration)
    {
        if (!_embeddings_storage->addEmbedding(_target, query, core_sub))
            // This match has already been handled
            return 1;
    }

    if (highlight)
        _target.highlightSubmolecule(query, core_sub, true);

    if (cb_embedding != 0)
        if (!cb_embedding(query, _target, core_sub, core_super, cb_embedding_context))
            return 0;

    if (find_all_embeddings)
        return 1;

    return 0;
}

int MoleculeSubstructureMatcher::_embedding_markush(int* core_sub, int* core_super)
{
    QueryMolecule& g1 = *_query;
    MarkushContext& context = *_markush.get();

    // Current site atom index
    int old_site_idx = context.sites[context.depth];
    bool two_att_points;

    // Check number of attachment points for current site
    int site_degree = g1.getVertex(old_site_idx).degree();
    switch (site_degree)
    {
    case 1:
        two_att_points = false;
        break;
    case 2:
        two_att_points = true;
        break;
    default:
        throw Error("unsupported number of attachment points (%d)", site_degree);
    }

    // Save number of embeddings to check if new embedding appeared
    int embeddings_count = _embeddings_storage->count();
    bool find_all_embeddings_saved = find_all_embeddings;
    if (save_for_iteration)
        find_all_embeddings = true;

    // For all possible rgroups at current site
    // do not do this:
    // const Array<int> &rg_list = g1.getRGroups()->getSiteRGroups(old_site_idx);

    QS_DEF(Array<int>, old_site_rgroups);

    g1.getAllowedRGroups(old_site_idx, old_site_rgroups);

    bool all_have_rest_h = true;

    for (int rg_idx = 0; rg_idx < old_site_rgroups.size(); rg_idx++)
    {
        RGroup& rgroup = g1.rgroups.getRGroup(old_site_rgroups[rg_idx]);
        PtrPool<BaseMolecule>& frags = rgroup.fragments;

        all_have_rest_h &= (rgroup.rest_h > 0);
        // For all rgroup fragments
        for (int fr_idx = frags.begin(); fr_idx != frags.end(); fr_idx = frags.next(fr_idx))
        {
            QueryMolecule& fragment = frags[fr_idx]->asQueryMolecule();

            if (fragment.attachmentPointCount() > 2)
                throw Error("more than two attachment points");

            if (site_degree != fragment.attachmentPointCount())
                throw Error("number of attachment points must be equal to R-group site degree");

            int att_idx1;
            int att_idx2;
            int i, j;

            for (i = 0; (att_idx1 = fragment.getAttachmentPoint(1, i)) != -1; i++)
            {
                if (two_att_points)
                {
                    for (j = 0; (att_idx2 = fragment.getAttachmentPoint(2, j)) != -1; j++)
                        if (!_attachRGroupAndContinue(core_sub, core_super, &fragment, true, att_idx1, att_idx2, old_site_rgroups[rg_idx], false))
                            return 0;
                }
                else if (!_attachRGroupAndContinue(core_sub, core_super, &fragment, false, att_idx1, -1, old_site_rgroups[rg_idx], false))
                    return 0;
            }
        }
    }

    if (!two_att_points && !_attachRGroupAndContinue(core_sub, core_super, 0, false, -1, -1, -1, all_have_rest_h))
        return 0;

    find_all_embeddings = find_all_embeddings_saved;
    if (!find_all_embeddings && save_for_iteration)
        // If there was found an embedding then return false (stop searching)
        return _embeddings_storage->count() == embeddings_count;

    return 1;
}

bool MoleculeSubstructureMatcher::_attachRGroupAndContinue(int* core1, int* core2, QueryMolecule* fragment, bool two_attachment_points, int att_idx1,
                                                           int att_idx2, int rgroup_idx, bool rest_h)
{
    MarkushContext& context = *_markush.get();
    QS_DEF(Array<int>, fr_mapping);
    int i;
    int n_sites = 0;
    bool ok = true;
    int hydrogen_attached_to = -1;

    // Site atom index in new molecule
    int cur_site = context.sites[context.depth];

    int src_att_idx1 = -1, src_att_idx2 = -1;
    std::unique_ptr<QueryMolecule::Bond> rg_qbond1;

    // Parameters for stereocenter restoration
    bool stereo_was_saved = false;
    int saved_stereo_type, saved_stereo_group, saved_stereo_pyramid[4];

    // If some rgroup must be attached
    if (rgroup_idx != -1)
    {
        // Add rgroup fragment to new molecule
        context.query.mergeWithMolecule(*fragment, &fr_mapping);

        // Site atom vertex in new molecule
        const Vertex& cur_site_vertex = context.query.getVertex(cur_site);

        // First and second neighbors indices and neighboring atoms indices
        int nei_idx1 = cur_site_vertex.neiBegin();
        int nei_idx2 = cur_site_vertex.neiNext(nei_idx1);

        int nei_atom_idx1 = context.query.getRSiteAttachmentPointByOrder(cur_site, 0);
        int nei_atom_idx2 = context.query.getRSiteAttachmentPointByOrder(cur_site, 1);

        if (two_attachment_points)
        {
            if (cur_site_vertex.degree() != 2)
                throw Error("RGroup atom has wrong number of attachment points");

            if (cur_site_vertex.neiVertex(nei_idx1) != nei_atom_idx1)
            {
                // Swap attachment neighbors according to attachment orders (first and second)
                std::swap(nei_idx1, nei_idx2);
            }

            if (cur_site_vertex.neiVertex(nei_idx1) != nei_atom_idx1 || cur_site_vertex.neiVertex(nei_idx2) != nei_atom_idx2)
                throw Error("Internal or rgroup query file error!");
        }
        else
        {
            if (cur_site_vertex.degree() != 1)
                throw Error("RGroup atom has wrong number of attachment points");

            if (att_idx1 == -1)
            {
                // Second attachment index of fragment must be attached to second-ordered neighbor
                if (cur_site_vertex.neiVertex(nei_idx1) != nei_atom_idx2)
                    ok = false;
                att_idx1 = att_idx2;
            }
            else if (cur_site_vertex.neiVertex(nei_idx1) != nei_atom_idx1)
                // First attachment index of fragment must be attached to first-ordered neighbor
                ok = false;
        }

        if (ok)
        {
            // First attachment bond of current rgroup site
            src_att_idx1 = cur_site_vertex.neiVertex(nei_idx1);
            context.query.flipBond(src_att_idx1, cur_site, fr_mapping[att_idx1]);

            if (two_attachment_points)
            {
                // Second attachment bond of current rgroup site
                src_att_idx2 = cur_site_vertex.neiVertex(nei_idx2);
                context.query.flipBond(src_att_idx2, cur_site, fr_mapping[att_idx2]);
            }
        }
    }
    else
    {
        // no rgroup (rgroup = implicit hydrogen)
        // This might happen only for rgroup with one attachment point
        const Vertex& cur_site_vertex = context.query.getVertex(cur_site);

        if (cur_site_vertex.degree() != 1)
            throw Error("RGroup atom has wrong number of attachment points");

        int nei_idx1 = cur_site_vertex.neiBegin();
        src_att_idx1 = cur_site_vertex.neiVertex(nei_idx1);

        int target_idx = core1[src_att_idx1];

        if (rest_h)
        {
            if (_target.getAtomTotalH(target_idx) - _used_target_h[target_idx] <= 0)
                return true;
            _used_target_h[target_idx]++;
        }

        // Remove edge to site
        rg_qbond1.reset(context.query.releaseBond(cur_site_vertex.neiEdge(nei_idx1)));

        // Save stereocenter before bond removing because bond
        // removing can cause stereocenter destruction
        MoleculeStereocenters& qstereo = context.query.stereocenters;
        if (qstereo.exists(src_att_idx1))
        {
            qstereo.get(src_att_idx1, saved_stereo_type, saved_stereo_group, saved_stereo_pyramid);
            stereo_was_saved = true;
        }

        context.query.removeBond(cur_site_vertex.neiEdge(nei_idx1));
    }

    // From MR: Why create new EmbeddingEnumerator? Is it possible to use
    // current without recreating all structures? Just attach and continue...

    // Init new embedding enumerator context
    EmbeddingEnumerator ee(_target);

    ee.cb_match_vertex = _matchAtoms;
    ee.cb_match_edge = _matchBonds;
    ee.cb_vertex_remove = _removeAtom;
    ee.cb_edge_add = _addBond;
    ee.cb_embedding = _embedding;

    ee.setSubgraph(context.query);

    ee.userdata = this;

    if (rgroup_idx != -1)
    {
        context.query_marking.expand(context.query.vertexEnd());

        // Check if there are explicit hydrogens or new sites attached- ignore them in further matching
        // Mark new vertices and add new sites if any
        QS_DEF(Array<int>, ignored);

        ignored.clear_resize(fragment->vertexEnd());

        markIgnoredQueryHydrogens(*fragment, ignored.ptr(), 0, 1);

        if (hydrogen_attached_to != -1)
        {
            int idx = fragment->vertexBegin();

            context.query_marking[fr_mapping[idx]] = context.depth;

            if (fragment->getAtomNumber(idx) == ELEM_H && fragment->possibleAtomIsotope(idx, 0))
                ee.ignoreSubgraphVertex(fr_mapping[idx]);
        }
        else
        {
            for (i = fragment->vertexBegin(); i < fragment->vertexEnd(); i = fragment->vertexNext(i))
            {
                context.query_marking[fr_mapping[i]] = context.depth;

                if (ignored[i])
                    ee.ignoreSubgraphVertex(fr_mapping[i]);
                else if (fragment->isRSite(i))
                {
                    context.sites.push(fr_mapping[i]);
                    n_sites++;
                    ee.ignoreSubgraphVertex(fr_mapping[i]);
                }
            }
        }

        if (_am.get() != 0)
            _am->validateQuery();
    }

    // Copy ingnored query vertices from previous state
    for (i = context.query.vertexBegin(); i < context.query.vertexEnd(); i = context.query.vertexNext(i))
    {
        if (context.query_marking[i] == context.depth)
            continue;

        if (core1[i] == EmbeddingEnumerator::IGNORE)
            ee.ignoreSubgraphVertex(i);
    }

    // Copy ignored target vertices
    for (i = _target.vertexBegin(); i < _target.vertexEnd(); i = _target.vertexNext(i))
        if (core2[i] == EmbeddingEnumerator::IGNORE)
            ee.ignoreSupergraphVertex(i);

    // Copy partial mapping from previous state
    for (i = context.query.vertexBegin(); i < context.query.vertexEnd(); i = context.query.vertexNext(i))
    {
        if (context.query_marking[i] == context.depth)
            continue;

        if (core1[i] >= 0 && !ee.fix(i, core1[i]))
            ok = false;
    }

    // Store attached rgroup index at current site
    context.sites[context.depth] = rgroup_idx;

    context.depth++;

    // Continue in new state

    // Call embedding enumerator recursively
    if (ok && !ee.process())
        return false;

    // Restore state
    context.depth--;

    context.sites[context.depth] = cur_site;

    if (rgroup_idx != -1)
    {
        context.query.flipBond(src_att_idx1, fr_mapping[att_idx1], cur_site);

        if (src_att_idx2 != -1)
            context.query.flipBond(src_att_idx2, fr_mapping[att_idx2], cur_site);
    }
    else // no rgroup
    {
        // Restore edge to site
        context.query.addBond(src_att_idx1, cur_site, rg_qbond1.release());

        // Restore used hydrogen
        if (rest_h)
            _used_target_h[core1[src_att_idx1]]--;

        // Restore stereocenter
        if (stereo_was_saved)
        {
            if (context.query.stereocenters.exists(src_att_idx1))
                context.query.stereocenters.remove(src_att_idx1);
            context.query.addStereocenters(src_att_idx1, saved_stereo_type, saved_stereo_group, saved_stereo_pyramid);
        }
    }

    if (rgroup_idx != -1)
    {
        Filter remove_filter(context.query_marking.ptr(), Filter::EQ, context.depth);
        context.query.removeAtoms(remove_filter);

        while (n_sites-- > 0)
            context.sites.pop();

        if (_am.get() != 0)
            _am->validateQuery();
    }

    return true;
}

bool MoleculeSubstructureMatcher::_checkRGroupConditions()
{
    QS_DEF(Array<int>, occurrences);
    QS_DEF(Array<int>, conditions);
    QS_DEF(Array<int>, nontop_rgroups);
    bool resth_satisfied;
    int i, k;
    MarkushContext& context = *_markush.get();
    MoleculeRGroups& rgroups = _query->rgroups;

    int n_rgroups = rgroups.getRGroupCount();

    occurrences.clear_resize(n_rgroups + 1);
    occurrences.zerofill();
    conditions.clear_resize(n_rgroups + 1);
    conditions.zerofill();
    nontop_rgroups.clear_resize(n_rgroups + 1);
    nontop_rgroups.zerofill();

    // Count occurrences of each rgroup
    for (i = 0; i < context.sites.size(); i++)
        if (context.sites[i] >= 0)
            occurrences[context.sites[i]]++;

    for (int i = 1; i <= n_rgroups; i++)
    {
        RGroup& rgroup = rgroups.getRGroup(i);

        if (rgroup.fragments.size() == 0)
        {
            conditions[i] = -1;
            continue;
        }

        if (rgroup.if_then > 0)
            nontop_rgroups[rgroup.if_then] = 1;

        // Check occurrence
        if (!rgroup.occurrenceSatisfied(occurrences[i]))
            continue;

        resth_satisfied = true;

        // Check RestH
        if (rgroup.rest_h == 1)
        {
            int site_idx, j = 0;

            for (site_idx = _query->vertexBegin(); site_idx != _query->vertexEnd(); site_idx = _query->vertexNext(site_idx))
            {
                if (!_query->isRSite(site_idx))
                    continue;

                QS_DEF(Array<int>, rg_list);

                _query->getAllowedRGroups(site_idx, rg_list);
                for (k = 0; k < rg_list.size() && resth_satisfied; k++)
                    if (rg_list[k] == i)
                    {
                        if (context.sites[j] != i)
                            resth_satisfied = false;
                        break;
                    }

                if (!resth_satisfied)
                    break;
                j++;
            }
            if (!resth_satisfied)
                continue;
        }

        // Without if/then
        conditions[i] = 1;
    }

    // Check if/then relations
    for (int i = 1; i <= n_rgroups; i++)
    {
        if (nontop_rgroups[i] == 1 || conditions[i] == -1)
            continue;

        const RGroup& rgroup = rgroups.getRGroup(i);
        const RGroup* iter = &rgroup;
        int i_iter = i;
        int counter = 0;

        do
        {
            if (conditions[i_iter] != 1 || counter >= n_rgroups)
                return false;

            if ((i_iter = iter->if_then) > 0)
                iter = &rgroups.getRGroup(i_iter);

            counter++;
        } while (i_iter > 0);
    }

    return true;
}

const int* MoleculeSubstructureMatcher::getQueryMapping()
{
    return _ee->getSubgraphMapping();
}

const int* MoleculeSubstructureMatcher::getTargetMapping()
{
    return _ee->getSupergraphMapping();
}

void MoleculeSubstructureMatcher::ignoreQueryAtom(int idx)
{
    _ee->ignoreSubgraphVertex(idx);
}

void MoleculeSubstructureMatcher::ignoreTargetAtom(int idx)
{
    _ee->ignoreSupergraphVertex(idx);
}

bool MoleculeSubstructureMatcher::fix(int query_atom_idx, int target_atom_idx)
{
    return _ee->fix(query_atom_idx, target_atom_idx);
}

void MoleculeSubstructureMatcher::markIgnoredHydrogens(BaseMolecule& mol, int* arr, int value_keep, int value_ignore)
{
    int i;

    for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        arr[i] = value_keep;

    for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (mol.getAtomNumber(i) != ELEM_H)
            continue;

        if (!mol.possibleAtomIsotope(i, 0))
            continue;

        if (mol.isQueryMolecule())
        {
            // Check if atom has fragment constraint.
            // For example [$([#1][N])] should be ignored
            if (mol.asQueryMolecule().getAtom(i).hasConstraint(QueryMolecule::ATOM_FRAGMENT))
                continue;
        }

        const Vertex& vertex = mol.getVertex(i);

        if (vertex.degree() == 1)
        {
            int nei_idx = vertex.neiVertex(vertex.neiBegin());

            if (mol.getAtomNumber(nei_idx) == ELEM_H && mol.possibleAtomIsotope(nei_idx, 0))
                continue; // do not ignore rare H-H fragment

            // Check if hydrogen forms a cis-trans bond or stereocenter
            int nei_vertex_idx = vertex.neiVertex(vertex.neiBegin());
            if (mol.stereocenters.exists(nei_vertex_idx))
                continue;

            // For example for this query hydrogens should be unfolded: [H]\\C=C/C
            const Vertex& nei_vertex = mol.getVertex(nei_vertex_idx);
            bool not_ignore = false;
            for (int nei = nei_vertex.neiBegin(); nei != nei_vertex.neiEnd(); nei = nei_vertex.neiNext(nei))
            {
                int edge = nei_vertex.neiEdge(nei);
                if (mol.cis_trans.getParity(edge) != 0)
                {
                    not_ignore = true;
                    break;
                }
            }
            if (not_ignore)
                continue;

            arr[i] = value_ignore;
        }
    }
}

void MoleculeSubstructureMatcher::markIgnoredQueryHydrogens(QueryMolecule& mol, int* arr, int value_keep, int value_ignore)
{
    markIgnoredHydrogens(mol, arr, value_keep, value_ignore);

    // keep hydrogens that are parts of 3d constraints
    {
        Molecule3dConstraintsChecker checker(mol.spatial_constraints);

        checker.markUsedAtoms(arr, value_keep);
    }
}

void MoleculeSubstructureMatcher::getAtomPos(Graph& graph, int vertex_idx, Vec3f& pos)
{
    pos.copy(((BaseMolecule&)graph).getAtomXyz(vertex_idx));
}

bool MoleculeSubstructureMatcher::_isSingleBond(Graph& graph, int edge_idx)
{
    return ((BaseMolecule&)graph).getBondOrder(edge_idx) == BOND_SINGLE;
}

const GraphEmbeddingsStorage& MoleculeSubstructureMatcher::getEmbeddingsStorage() const
{
    return _embeddings_storage.ref();
}

bool MoleculeSubstructureMatcher::needCoords(int match_3d, QueryMolecule& query)
{
    if (match_3d != 0)
        return true;

    if (query.spatial_constraints.haveConstraints())
        return true;

    return false;
}

bool MoleculeSubstructureMatcher::_canUseEquivalenceHeuristic(QueryMolecule& query)
{
    if (query.spatial_constraints.haveConstraints())
        return false;
    if (query.stereocenters.size() > 0)
        return false;
    return true;
}

int MoleculeSubstructureMatcher::_compare_degree_asc(BaseMolecule& mol, int i1, int i2)
{
    return mol.getVertex(i2).degree() - mol.getVertex(i1).degree();
}

int MoleculeSubstructureMatcher::_compare_frequency_base(BaseMolecule& mol, int i1, int i2)
{
    int label1 = mol.getAtomNumber(i1);
    int label2 = mol.getAtomNumber(i2);

    if (label1 == 0 && label2 != 0)
        return 1;
    if (label1 != 0 && label2 != 1)
        return -1;

    int is_hetero1 = (label1 != 0 && label1 != ELEM_C && label1 != ELEM_H);
    int is_hetero2 = (label2 != 0 && label2 != ELEM_C && label2 != ELEM_H);
    return is_hetero2 - is_hetero1;
}

int MoleculeSubstructureMatcher::_compare_frequency_asc(BaseMolecule& mol, int i1, int i2)
{
    static int labels_by_freq[] = {ELEM_C, ELEM_H, ELEM_O, ELEM_N, ELEM_P, ELEM_F, ELEM_S, ELEM_Si, ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};

    int label1 = mol.getAtomNumber(i1);
    int label2 = mol.getAtomNumber(i2);
    int idx1, idx2;

    for (idx1 = 0; idx1 < NELEM(labels_by_freq); idx1++)
        if (label1 == labels_by_freq[idx1])
            break;
    for (idx2 = 0; idx2 < NELEM(labels_by_freq); idx2++)
        if (label2 == labels_by_freq[idx2])
            break;

    return idx2 - idx1;
}

int MoleculeSubstructureMatcher::_compare_in_loop(BaseMolecule& mol, int i1, int i2)
{
    const Vertex& v1 = mol.getVertex(i1);
    const Vertex& v2 = mol.getVertex(i2);

    int in_loop1 = 0;
    for (int nei = v1.neiBegin(); nei != v1.neiEnd(); nei = v1.neiNext(nei))
    {
        int nei_edge = v1.neiEdge(nei);
        if (mol.getEdgeTopology(nei_edge) == TOPOLOGY_RING)
        {
            in_loop1 = 1;
            break;
        }
    }
    int in_loop2 = 0;
    for (int nei = v2.neiBegin(); nei != v2.neiEnd(); nei = v2.neiNext(nei))
    {
        int nei_edge = v2.neiEdge(nei);
        if (mol.getEdgeTopology(nei_edge) == TOPOLOGY_RING)
        {
            in_loop2 = 1;
            break;
        }
    }
    return in_loop2 - in_loop1;
}

int MoleculeSubstructureMatcher::_compare(int& i1, int& i2, void* context)
{
    BaseMolecule& mol = *(BaseMolecule*)context;

    bool is_pseudo1 = mol.isPseudoAtom(i1);
    bool is_pseudo2 = mol.isPseudoAtom(i2);
    if (is_pseudo1 && !is_pseudo2)
        return -1;
    if (!is_pseudo1 && is_pseudo2)
        return 1;
    if (is_pseudo1)
        return 0; // All pseudoatoms are the same for transposition for substructure

    bool is_template1 = mol.isTemplateAtom(i1);
    bool is_template2 = mol.isTemplateAtom(i2);
    if (is_template1 && !is_template2)
        return -1;
    if (!is_template1 && is_template2)
        return 1;
    if (is_template1)
        return 0; // All template atoms are the same for transposition for substructure (?)

    int res;

    res = _compare_frequency_base(mol, i1, i2);
    if (res != 0)
        return res;

    res = _compare_in_loop(mol, i1, i2);
    if (res != 0)
        return res;

    res = _compare_frequency_asc(mol, i1, i2);
    if (res != 0)
        return res;

    return _compare_degree_asc(mol, i1, i2);
}

void MoleculeSubstructureMatcher::makeTransposition(BaseMolecule& mol, Array<int>& transposition_out)
{
    int i;

    transposition_out.clear();

    for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
        transposition_out.push(i);

    transposition_out.qsort(_compare, &mol);
}
