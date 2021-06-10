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

#include "molecule/molecule_tautomer_substructure_matcher.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/query_molecule.h"

using namespace indigo;

IMPL_ERROR(MoleculeTautomerSubstructureMatcher, "molecule tautomer substructure matcher");

CP_DEF(MoleculeTautomerSubstructureMatcher);

MoleculeTautomerSubstructureMatcher::MoleculeTautomerSubstructureMatcher(BaseMolecule& target, TautomerMethod method)
    : _tautomerEnumerator(target.asMolecule(), method), CP_INIT
{
    _allLayersFound = false;
    _layerBeg = 0;
    _layerEnd = 1;
    // Instead of full enumeration of tautomers we are using lazy enumeration now.
    //_tautomerEnumerator.enumerateAll(false);
    find_all_embeddings = false;
    find_unique_embeddings = false;
    find_unique_by_edges = false;
    save_for_iteration = false;
    _needAromatize = target.asMolecule().isAromatized();
}

MoleculeTautomerSubstructureMatcher::~MoleculeTautomerSubstructureMatcher()
{
}

void MoleculeTautomerSubstructureMatcher::setQuery(QueryMolecule& query)
{
    _query = &query;

    // We should check that query has any trace of aromatization. Otherwise it has no sense to perform expensive procedure of _tautomerEnumerator aromatization.
    // if(query.isAromatic())
    {
        _tautomerEnumerator.aromatize();
    }

    QS_DEF(Array<int>, ignored);

    ignored.clear_resize(_query->vertexEnd());

    ignored.zerofill();

    if (_ee.get() != 0)
        _ee.free();

    _ee.create(_tautomerEnumerator.layeredMolecules);
    _ee->cb_match_vertex = _matchAtomsHyper;
    _ee->cb_match_edge = _matchBondsSubHyper;
    _ee->cb_edge_add = _edgeAddHyper;
    _ee->cb_vertex_add = NULL;
    _ee->cb_vertex_remove = _vertexRemoveHyper;
    _ee->cb_embedding = _preliminaryEmbeddingHyper;
    _breadcrumps.self = this;
    _ee->userdata = &_breadcrumps;

    _ee->setSubgraph(*_query);

    _embeddings_storage.free();
    _masks.clear();
}

bool MoleculeTautomerSubstructureMatcher::_matchAtoms(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata)
{
    QueryMolecule& query = ((BaseMolecule&)subgraph).asQueryMolecule();
    QueryMolecule::Atom* atom = &query.getAtom(sub_idx);
    BaseMolecule& target = (BaseMolecule&)supergraph;

    if (!MoleculeSubstructureMatcher::matchQueryAtom(atom, (BaseMolecule&)supergraph, super_idx, 0, 0xFFFFFFFFUL))
        return false;

    if (query.stereocenters.getType(sub_idx) > target.stereocenters.getType(super_idx))
        return false;

    if (query.stereocenters.getType(sub_idx) > 0)
        if (!target.isPseudoAtom(super_idx) && !target.isRSite(super_idx))
            if (query.getAtomMinH(sub_idx) > target.getAtomMaxH(super_idx))
                return false;

    return true;
}

bool MoleculeTautomerSubstructureMatcher::_matchAtomsHyper(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata)
{
    // Currently use common atom match procedure
    return _matchAtoms(subgraph, supergraph, core_sub, sub_idx, super_idx, userdata);
}

bool MoleculeTautomerSubstructureMatcher::_matchBondsSubHyper(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    SubstructureSearchBreadcrumps& breadcrumps = *(SubstructureSearchBreadcrumps*)userdata;
    LayeredMolecules& layeredMolecules = (LayeredMolecules&)supergraph;
    QueryMolecule& query = ((BaseMolecule&)subgraph).asQueryMolecule();

    int sub_bond_order = query.getBondOrder(sub_idx);
    const Dbitset& mask = layeredMolecules.getBondMask(super_idx, sub_bond_order);

    return mask.intersects(breadcrumps.mask);
}

void MoleculeTautomerSubstructureMatcher::_edgeAddHyper(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    SubstructureSearchBreadcrumps& breadcrumps = *(SubstructureSearchBreadcrumps*)userdata;
    LayeredMolecules& layeredMolecules = (LayeredMolecules&)supergraph;
    QueryMolecule& query = ((BaseMolecule&)subgraph).asQueryMolecule();

    int sub_bond_order = query.getBondOrder(sub_idx);
    const Dbitset& mask = layeredMolecules.getBondMask(super_idx, sub_bond_order);

    breadcrumps.maskHistory.expand(breadcrumps.maskHistory.size() + 1);
    breadcrumps.maskHistory.top().copy(breadcrumps.mask);

    breadcrumps.mask.andWith(mask);
}

void MoleculeTautomerSubstructureMatcher::_vertexRemoveHyper(Graph& subgraph, int sub_idx, void* userdata)
{
    SubstructureSearchBreadcrumps& breadcrumps = *(SubstructureSearchBreadcrumps*)userdata;
    if (breadcrumps.maskHistory.size())
    {
        breadcrumps.mask.copy(breadcrumps.maskHistory.top());
        breadcrumps.maskHistory.pop();
    }
}

int MoleculeTautomerSubstructureMatcher::_preliminaryEmbeddingHyper(Graph& g1, Graph& g2, int* core_sub, int* core_super, void* userdata)
{
    SubstructureSearchBreadcrumps& breadcrumps = *(SubstructureSearchBreadcrumps*)userdata;
    MoleculeTautomerSubstructureMatcher* self = breadcrumps.self;
    Dbitset& mask = breadcrumps.mask;

    return self->_embedding_common(core_sub, core_super, mask);

    return 0;
}

bool MoleculeTautomerSubstructureMatcher::find()
{
    if (_query == 0)
        throw Error("no query");

    _createEmbeddingsStorage();

    _breadcrumps.maskHistory.clear();
    _breadcrumps.mask.resize(_tautomerEnumerator.layeredMolecules.layers);
    _breadcrumps.mask.zeroFill();
    _breadcrumps.mask.flip(_layerBeg, _layerEnd);
    int result = _ee->process();

    while (result == 1)
    {
        _layerBeg = _layerEnd;
        if (!_tautomerEnumerator.enumerateLazy())
        {
            _layerEnd = _tautomerEnumerator.layeredMolecules.layers;
            _breadcrumps.maskHistory.clear();
            _breadcrumps.mask.resize(_tautomerEnumerator.layeredMolecules.layers);
            _breadcrumps.mask.zeroFill();
            _breadcrumps.mask.flip(_layerBeg, _layerEnd);

            _ee->setSubgraph(*_query);
            result = _ee->process();
        }
        else
        {
            break;
        }
    }

    return result == 0;
}

bool MoleculeTautomerSubstructureMatcher::findNext()
{
    bool found = _ee->processNext();

    while (!found)
    {
        _layerBeg = _layerEnd;
        if (!_tautomerEnumerator.enumerateLazy())
        {
            _layerEnd = _tautomerEnumerator.layeredMolecules.layers;
            _breadcrumps.maskHistory.clear();
            _breadcrumps.mask.resize(_tautomerEnumerator.layeredMolecules.layers);
            _breadcrumps.mask.zeroFill();
            _breadcrumps.mask.flip(_layerBeg, _layerEnd);

            _ee->setSubgraph(*_query);
            found = _ee->process() != 1;
        }
        else
        {
            break;
        }
    }

    return found;
}

void MoleculeTautomerSubstructureMatcher::_createEmbeddingsStorage()
{
    _embeddings_storage.create();
    _embeddings_storage->unique_by_edges = find_unique_by_edges;
    _embeddings_storage->save_edges = save_for_iteration;
    _embeddings_storage->save_mapping = save_for_iteration;
    _embeddings_storage->check_uniquencess = false; // find_unique_embeddings;
}

int MoleculeTautomerSubstructureMatcher::_embedding_common(int* core_sub, int* core_super, Dbitset& mask)
{
    QueryMolecule& query = *_query;

    if (find_unique_embeddings || save_for_iteration)
    {
        if (!_embeddings_storage->addEmbedding(_tautomerEnumerator.layeredMolecules, query, core_sub))
            // This match has already been handled
            return 1;
        _masks.push();
        _masks.top().copy(mask);
        if (_needAromatize)
        {
            int layer = _masks.top().nextSetBit(0);
            _tautomerEnumerator.beginAromatized();
            while (layer != -1)
            {
                // Magic! layer is a non-negative number. We know that we enumerate aromatized tautomers. That means that the range is [-1, -2, -3, ...]
                if (!_tautomerEnumerator.isValid(-1 - layer))
                {
                    _masks.top().reset(layer);
                }
                else
                {
                    _tautomerEnumerator.next(-1 - layer);
                }
                layer = _masks.top().nextSetBit(layer + 1);
            }
        }
    }

    return 0;
}

const int* MoleculeTautomerSubstructureMatcher::getQueryMapping()
{
    return _ee->getSubgraphMapping();
}

const int* MoleculeTautomerSubstructureMatcher::getTargetMapping()
{
    return _ee->getSupergraphMapping();
}

const GraphEmbeddingsStorage& MoleculeTautomerSubstructureMatcher::getEmbeddingsStorage() const
{
    return _embeddings_storage.ref();
}

const Dbitset& MoleculeTautomerSubstructureMatcher::getMask(int ind) const
{
    return _masks.at(ind);
}

void MoleculeTautomerSubstructureMatcher::getTautomerFound(Molecule& mol, int enumInd, int tauInd) const
{
    const Dbitset& mask = _masks.at(enumInd);
    int layer = mask.nextSetBit(tauInd);

    return _tautomerEnumerator.constructMolecule(mol, layer, _needAromatize);
}
