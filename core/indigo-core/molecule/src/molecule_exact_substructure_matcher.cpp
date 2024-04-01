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

#include "molecule/molecule_exact_substructure_matcher.h"

#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_exact_matcher.h"

using namespace indigo;

IMPL_ERROR(MoleculeExactSubstructureMatcher, "molecule exact substructure matcher");

MoleculeExactSubstructureMatcher::MoleculeExactSubstructureMatcher(Molecule& query, Molecule& target) : _query(query), _target(target), _ee(target)
{
    flags = 0xFFFFFFFFUL;

    _ee.cb_match_vertex = _matchAtoms;
    _ee.cb_match_edge = _matchBonds;
    _ee.cb_embedding = _embedding;

    _ee.userdata = this;

    _ee.setSubgraph(query);
}

const int* MoleculeExactSubstructureMatcher::getQueryMapping()
{
    return _ee.getSubgraphMapping();
}

void MoleculeExactSubstructureMatcher::ignoreTargetAtom(int idx)
{
    _ee.ignoreSupergraphVertex(idx);
}

void MoleculeExactSubstructureMatcher::ignoreQueryAtom(int idx)
{
    _ee.ignoreSubgraphVertex(idx);
}

bool MoleculeExactSubstructureMatcher::find()
{
    int i;

    for (i = _query.vertexBegin(); i != _query.vertexEnd(); i = _query.vertexNext(i))
    {
        const Vertex& vertex = _query.getVertex(i);

        if (_query.getAtomNumber(i) == ELEM_H && vertex.degree() == 1 && _query.getAtomNumber(vertex.neiVertex(vertex.neiBegin())) != ELEM_H)
            if (_query.getAtomIsotope(i) == 0 || !(flags & MoleculeExactMatcher::CONDITION_ISOTOPE))
                _ee.ignoreSubgraphVertex(i);
    }

    for (i = _target.vertexBegin(); i != _target.vertexEnd(); i = _target.vertexNext(i))
    {
        const Vertex& vertex = _target.getVertex(i);

        if (_target.getAtomNumber(i) == ELEM_H && vertex.degree() == 1 && _target.getAtomNumber(vertex.neiVertex(vertex.neiBegin())) != ELEM_H)
            if (_target.getAtomIsotope(i) == 0 || !(flags & MoleculeExactMatcher::CONDITION_ISOTOPE))
                _ee.ignoreSupergraphVertex(i);
    }

    if (flags & MoleculeExactMatcher::CONDITION_FRAGMENTS)
    {
        if (_ee.countUnmappedSubgraphVertices() > _ee.countUnmappedSupergraphVertices())
            return false;

        if (_ee.countUnmappedSubgraphEdges() > _ee.countUnmappedSupergraphEdges())
            return false;
    }
    else
    {
        _collectConnectedComponentsInfo();

        // Basic check: the query must contain no more fragments than the target
        int query_components = _query_decomposer->getComponentsCount();
        int target_components = _target_decomposer->getComponentsCount();

        if (query_components > target_components)
            return false;
    }

    if (_ee.process() == 0)
        return true;

    return false;
}

bool MoleculeExactSubstructureMatcher::find_withHydrogens()
{

    if (flags & MoleculeExactMatcher::CONDITION_FRAGMENTS)
    {
        if (_ee.countUnmappedSubgraphVertices() > _ee.countUnmappedSupergraphVertices())
            return false;

        if (_ee.countUnmappedSubgraphEdges() > _ee.countUnmappedSupergraphEdges())
            return false;
    }
    else
    {
        _collectConnectedComponentsInfo();

        // Basic check: the query must contain no more fragments than the target
        int query_components = _query_decomposer->getComponentsCount();
        int target_components = _target_decomposer->getComponentsCount();

        if (query_components > target_components)
            return false;
    }

    if (_ee.process() == 0)
        return true;

    return false;
}

bool MoleculeExactSubstructureMatcher::findNext()
{
    bool found = _ee.processNext();

    return found;
}

void MoleculeExactSubstructureMatcher::_collectConnectedComponentsInfo()
{
    // Target vertices filter initialization
    Filter target_vertices_filter;
    target_vertices_filter.init(_ee.getSupergraphMapping(), Filter::NEQ, EmbeddingEnumerator::IGNORE);

    // Target decomposition
    _target_decomposer.create(_target);
    _target_decomposer->decompose(&target_vertices_filter);

    // Query vertices filter initialization
    Filter query_vertices_filter;
    query_vertices_filter.init(_ee.getSubgraphMapping(), Filter::NEQ, EmbeddingEnumerator::IGNORE);

    // Query decomposition
    _query_decomposer.create(_query);
    _query_decomposer->decompose(&query_vertices_filter);
}

bool MoleculeExactSubstructureMatcher::_matchAtoms(Graph& subgraph, Graph& supergraph, const int* /* core_sub */, int sub_idx, int super_idx, void* userdata)
{
    Molecule& query = (Molecule&)subgraph;
    Molecule& target = (Molecule&)supergraph;

    MoleculeExactSubstructureMatcher* self = (MoleculeExactSubstructureMatcher*)userdata;
    int flags = self->flags;

    if (flags & MoleculeExactMatcher::CONDITION_FRAGMENTS)
    {
        const GraphDecomposer& target_decomposer = self->_target_decomposer.ref();
        const GraphDecomposer& query_decomposer = self->_query_decomposer.ref();

        int super_component = target_decomposer.getComponent(super_idx);
        int sub_component = query_decomposer.getComponent(sub_idx);

        int super_vertices = target_decomposer.getComponentVerticesCount(super_component);
        int sub_vertices = query_decomposer.getComponentVerticesCount(sub_component);
        if (super_vertices != sub_vertices)
            return false;

        int super_edges = target_decomposer.getComponentEdgesCount(super_component);
        int sub_edges = query_decomposer.getComponentEdgesCount(sub_component);
        if (super_edges != sub_edges)
            return false;
    }

    return MoleculeExactMatcher::matchAtoms(query, target, sub_idx, super_idx, flags);
}

bool MoleculeExactSubstructureMatcher::_matchBonds(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    Molecule& query = (Molecule&)subgraph;
    Molecule& molecule = (Molecule&)supergraph;

    MoleculeExactSubstructureMatcher* self = (MoleculeExactSubstructureMatcher*)userdata;
    int flags = self->flags;

    return MoleculeExactMatcher::matchBonds(query, molecule, sub_idx, super_idx, flags);
}

int MoleculeExactSubstructureMatcher::_embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata)
{
    MoleculeExactSubstructureMatcher* self = (MoleculeExactSubstructureMatcher*)userdata;
    Molecule& query = (Molecule&)subgraph;
    Molecule& target = (Molecule&)supergraph;

    if (self->flags & MoleculeExactMatcher::CONDITION_STEREO)
    {

        if (!MoleculeStereocenters::checkSub(query, target, core_sub, !(self->flags & MoleculeExactMatcher::CONDITION_ISOTOPE)))
            return 1;
        if (!MoleculeStereocenters::checkSub(target, query, core_super, !(self->flags & MoleculeExactMatcher::CONDITION_ISOTOPE)))
            return 1;

        if (!MoleculeCisTrans::checkSub(query, target, core_sub))
            return 1;
    }

    return 0;
}
