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

#include "molecule/molecule_exact_matcher.h"

using namespace indigo;

IMPL_ERROR(MoleculeExactMatcher, "molecule exact matcher");

MoleculeExactMatcher::MoleculeExactMatcher(BaseMolecule& query, BaseMolecule& target) : _query(query), _target(target), _ee(target)
{
    flags = 0;
    rms_threshold = 0;

    _ee.cb_match_vertex = _matchAtoms;
    _ee.cb_match_edge = _matchBonds;
    _ee.cb_embedding = _embedding;

    _ee.userdata = this;

    _ee.setSubgraph(query);
}

void MoleculeExactMatcher::ignoreTargetAtom(int idx)
{
    _ee.ignoreSupergraphVertex(idx);
}

const int* MoleculeExactMatcher::getTargetMapping()
{
    return _ee.getSupergraphMapping();
}

void MoleculeExactMatcher::ignoreQueryAtom(int idx)
{
    _ee.ignoreSubgraphVertex(idx);
}

const int* MoleculeExactMatcher::getQueryMapping()
{
    return _ee.getSubgraphMapping();
}

bool MoleculeExactMatcher::find()
{
    int i;

    if ((flags & CONDITION_3D) && !_query.have_xyz)
        throw Error("cannot do 3D match without XYZ in the query");

    for (i = _query.vertexBegin(); i != _query.vertexEnd(); i = _query.vertexNext(i))
    {
        const Vertex& vertex = _query.getVertex(i);

        if (_query.getAtomNumber(i) == ELEM_H && vertex.degree() == 1 && _query.getAtomNumber(vertex.neiVertex(vertex.neiBegin())) != ELEM_H)
            if (_query.getAtomIsotope(i) == 0 || !(flags & CONDITION_ISOTOPE))
                _ee.ignoreSubgraphVertex(i);
    }

    for (i = _target.vertexBegin(); i != _target.vertexEnd(); i = _target.vertexNext(i))
    {
        const Vertex& vertex = _target.getVertex(i);

        if (_target.getAtomNumber(i) == ELEM_H && vertex.degree() == 1 && _target.getAtomNumber(vertex.neiVertex(vertex.neiBegin())) != ELEM_H)
            if (_target.getAtomIsotope(i) == 0 || !(flags & CONDITION_ISOTOPE))
                _ee.ignoreSupergraphVertex(i);
    }

    if (flags & CONDITION_FRAGMENTS)
    {
        // Basic check: number of edges and vertices must be the same
        if (_ee.countUnmappedSubgraphVertices() != _ee.countUnmappedSupergraphVertices())
            return false;

        if (_ee.countUnmappedSubgraphEdges() != _ee.countUnmappedSupergraphEdges())
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

void MoleculeExactMatcher::_collectConnectedComponentsInfo()
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

bool MoleculeExactMatcher::_matchAtoms(Graph& subgraph, Graph& supergraph, const int* /*core_sub*/, int sub_idx, int super_idx, void* userdata)
{
    BaseMolecule& query = (BaseMolecule&)subgraph;
    BaseMolecule& target = (BaseMolecule&)supergraph;

    MoleculeExactMatcher* self = (MoleculeExactMatcher*)userdata;
    int flags = self->flags;

    if (!(flags & CONDITION_FRAGMENTS))
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

    return matchAtoms(query, target, sub_idx, super_idx, flags);
}

bool MoleculeExactMatcher::_matchBonds(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    BaseMolecule& query = (BaseMolecule&)subgraph;
    BaseMolecule& molecule = (BaseMolecule&)supergraph;

    MoleculeExactMatcher* self = (MoleculeExactMatcher*)userdata;
    int flags = self->flags;

    return matchBonds(query, molecule, sub_idx, super_idx, flags);
}

int MoleculeExactMatcher::_embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata)
{
    MoleculeExactMatcher* self = (MoleculeExactMatcher*)userdata;
    BaseMolecule& query = (BaseMolecule&)subgraph;
    BaseMolecule& target = (BaseMolecule&)supergraph;

    if (self->flags & CONDITION_STEREO)
    {

        if (!MoleculeStereocenters::checkSub(query, target, core_sub, !(self->flags & CONDITION_ISOTOPE)))
            return 1;
        if (!MoleculeStereocenters::checkSub(target, query, core_super, !(self->flags & CONDITION_ISOTOPE)))
            return 1;

        if (!MoleculeCisTrans::checkSub(query, target, core_sub))
            return 1;

        if (!MoleculeAlleneStereo::checkSub(query, target, core_sub))
            return 1;
        if (!MoleculeAlleneStereo::checkSub(target, query, core_super))
            return 1;
    }

    if (self->flags & CONDITION_3D)
    {
        GraphAffineMatcher matcher(subgraph, supergraph, core_sub);

        matcher.cb_get_xyz = MoleculeSubstructureMatcher::getAtomPos;

        if (!matcher.match(self->rms_threshold))
            return 1;
    }

    return 0;
}

bool MoleculeExactMatcher::_MatchToken::compare(const char* text) const
{
    return strcasecmp(t_text, text) == 0 ? true : false;
}

void MoleculeExactMatcher::parseConditions(const char* params, int& flags, float& rms_threshold)
{
    if (params == 0)
        throw Error("zero pointer passed to parseConditions()");

    static const _MatchToken token_list[] = {{"NONE", CONDITION_NONE},  {"ELE", CONDITION_ELECTRONS}, {"MAS", CONDITION_ISOTOPE},
                                             {"STE", CONDITION_STEREO}, {"FRA", CONDITION_FRAGMENTS}, {"ALL", CONDITION_ALL}};

    flags = CONDITION_NONE;
    rms_threshold = 0;

    BufferScanner scanner(params);

    QS_DEF(Array<char>, word);

    scanner.skipSpace();
    if (scanner.isEOF())
    {
        flags = CONDITION_ALL;
        return;
    }

    while (!scanner.isEOF())
    {
        int i;

        scanner.readWord(word, 0);
        scanner.skipSpace();

        if (word.size() < 2)
            throw Error("internal error on token reading");

        for (i = 0; i < NELEM(token_list); i++)
        {
            if (token_list[i].compare(word.ptr()))
                flags |= token_list[i].t_flag;
            else if (word[0] == '-' && token_list[i].compare(word.ptr() + 1))
                flags &= ~token_list[i].t_flag;
            else
                continue;
            break;
        }
        if (i == NELEM(token_list))
        {
            BufferScanner scanner2(word.ptr());

            if (scanner2.tryReadFloat(rms_threshold))
            {
                flags |= CONDITION_3D;
                return;
            }

            throw Error("parseConditions(): unknown token %s", word.ptr());
        }
    }
}

bool MoleculeExactMatcher::matchAtoms(BaseMolecule& query, BaseMolecule& target, int sub_idx, int super_idx, int flags)
{
    if (query.isRSite(sub_idx) && target.isRSite(super_idx))
        return query.getRSiteBits(sub_idx) == target.getRSiteBits(super_idx);

    if (query.isRSite(sub_idx) || target.isRSite(super_idx))
        return false;

    if (query.isPseudoAtom(sub_idx) && target.isPseudoAtom(super_idx))
    {
        if (strcmp(query.getPseudoAtom(sub_idx), target.getPseudoAtom(super_idx)) != 0)
            return false;
    }
    else if (query.isTemplateAtom(sub_idx) && target.isTemplateAtom(super_idx))
    {
        if (strcmp(query.getTemplateAtom(sub_idx), target.getTemplateAtom(super_idx)) != 0)
            return false;
    }
    else if (!query.isPseudoAtom(sub_idx) && !target.isPseudoAtom(super_idx) && !query.isTemplateAtom(sub_idx) && !target.isTemplateAtom(super_idx))
    {
        if (query.getAtomNumber(sub_idx) != target.getAtomNumber(super_idx))
            return false;
    }
    else
        return false;

    if (flags & CONDITION_ISOTOPE)
        if (query.getAtomIsotope(sub_idx) != target.getAtomIsotope(super_idx))
            return false;

    if (flags & CONDITION_ELECTRONS)
    {
        int qcharge = query.getAtomCharge(sub_idx);
        int tcharge = target.getAtomCharge(super_idx);

        if (qcharge == CHARGE_UNKNOWN)
            qcharge = 0;
        if (tcharge == CHARGE_UNKNOWN)
            tcharge = 0;

        if (qcharge != tcharge)
            return false;

        int qval = -1;
        int tval = -1;
        if (!query.isPseudoAtom(sub_idx) && !query.isTemplateAtom(sub_idx))
        {
            if (!query.isQueryMolecule() && !target.isQueryMolecule())
            {
                qval = query.getAtomValence_NoThrow(sub_idx, -1);
                tval = target.getAtomValence_NoThrow(super_idx, -1);
                if ((qval != -1) && (tval != -1) && (qval != tval))
                    return false;
            }

            if ((qval != -1) && (tval != -1))
            {
                int qrad = query.getAtomRadical(sub_idx);
                int trad = target.getAtomRadical(super_idx);

                if (qrad == -1)
                    qrad = 0;
                if (trad == -1)
                    trad = 0;

                if (qrad != trad)
                    return false;

                if (query.isQueryMolecule())
                {
                    int qarom = query.getAtomAromaticity(sub_idx);
                    int tarom = target.getAtomAromaticity(super_idx);

                    if (qarom != -1 && tarom != -1)
                        if (qarom != tarom)
                            return false;
                }
            }
        }
    }

    if (flags & CONDITION_STEREO)
    {
        int qtype = query.stereocenters.getType(sub_idx);

        if (qtype != target.stereocenters.getType(super_idx))
            return false;
    }
    return true;
}

bool MoleculeExactMatcher::matchBonds(BaseMolecule& query, BaseMolecule& target, int sub_idx, int super_idx, int flags)
{
    if (flags & CONDITION_ELECTRONS)
        if (query.getBondOrder(sub_idx) != target.getBondOrder(super_idx))
            return false;

    return true;
}

bool MoleculeExactMatcher::needCoords()
{
    return (flags & CONDITION_3D) != 0;
}
