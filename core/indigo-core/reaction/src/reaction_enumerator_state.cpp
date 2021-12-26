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

#include "reaction/reaction_enumerator_state.h"
#include "reaction/reaction_product_enumerator.h"

#include "base_cpp/output.h"

#include "base_c/bitarray.h"
#include "base_cpp/gray_codes.h"
#include "base_cpp/tlscont.h"
#include "graph/dfs_walk.h"
#include "graph/embedding_enumerator.h"
#include "graph/graph.h"
#include "graph/spanning_tree.h"
#include "molecule/canonical_smiles_saver.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/molfile_saver.h"
#include "molecule/query_molecule.h"

using namespace indigo;

IMPL_ERROR(ReactionEnumeratorState::ReactionMonomers, "Reaction product enumerator");

CP_DEF(ReactionEnumeratorState::ReactionMonomers);

ReactionEnumeratorState::ReactionMonomers::ReactionMonomers()
    : CP_INIT, TL_CP_GET(_monomers), TL_CP_GET(_reactant_indexes), TL_CP_GET(_deep_levels), TL_CP_GET(_tube_indexes)
{
    _monomers.clear();
    _reactant_indexes.clear();
    _deep_levels.clear();
    _tube_indexes.clear();
}

int ReactionEnumeratorState::ReactionMonomers::size()
{
    return _monomers.size();
}

void ReactionEnumeratorState::ReactionMonomers::clear()
{
    _monomers.clear();
    _reactant_indexes.clear();
    _deep_levels.clear();
    _tube_indexes.clear();
}

Molecule& ReactionEnumeratorState::ReactionMonomers::getMonomer(int reactant_idx, int index)
{
    int cur_idx = 0;

    for (int i = 0; i < _reactant_indexes.size(); i++)
        if (_reactant_indexes[i] == reactant_idx)
            if (cur_idx++ == index)
                return *_monomers[i];

    throw Error("can't find reactant's #%d monomer #%d", reactant_idx, index);
}

Molecule& ReactionEnumeratorState::ReactionMonomers::getMonomer(int mon_index)
{
    if (mon_index >= _monomers.size() || mon_index < 0)
        throw Error("can't find monomer #%d", mon_index);
    else
        return *_monomers[mon_index];
}

void ReactionEnumeratorState::ReactionMonomers::addMonomer(int reactant_idx, Molecule& monomer, int deep_level, int tube_idx)
{
    Molecule* mol_ptr = new Molecule();
    _monomers.add(mol_ptr);

    Molecule& new_monomer = *mol_ptr;
    new_monomer.clone(monomer, NULL, NULL);

    _reactant_indexes.push(reactant_idx);
    _deep_levels.push(deep_level);
    _tube_indexes.push(tube_idx);
}

void ReactionEnumeratorState::ReactionMonomers::removeMonomer(int idx)
{
    for (int j = idx + 1; j < _monomers.size(); j++)
    {
        _reactant_indexes[j - 1] = _reactant_indexes[j];
        _monomers[j - 1]->clone(*_monomers[j], NULL, NULL);
        _deep_levels[j - 1] = _deep_levels[j];
        _tube_indexes[j - 1] = _tube_indexes[j];
    }

    _reactant_indexes.pop();
    delete (_monomers.pop());
    _deep_levels.pop();
    _tube_indexes.pop();
}

IMPL_ERROR(ReactionEnumeratorState, "Reaction product enumerator state");

CP_DEF(ReactionEnumeratorState);

ReactionEnumeratorState::ReactionEnumeratorState(ReactionEnumeratorContext& context, QueryReaction& cur_reaction, QueryMolecule& cur_full_product,
                                                 Array<int>& cur_product_aam_array, RedBlackStringMap<int>& cur_smiles_array,
                                                 ReactionMonomers& cur_reaction_monomers, int& cur_product_count, ObjArray<Array<int>>& cur_tubes_monomers)
    : _reaction(cur_reaction), _product_count(cur_product_count), _tubes_monomers(cur_tubes_monomers), _product_aam_array(cur_product_aam_array),
      _smiles_array(cur_smiles_array), _reaction_monomers(cur_reaction_monomers), _context(context), CP_INIT, TL_CP_GET(_fragments_aam_array),
      TL_CP_GET(_full_product), TL_CP_GET(_product_monomers), TL_CP_GET(_mapping), TL_CP_GET(_fragments), TL_CP_GET(_is_needless_atom),
      TL_CP_GET(_is_needless_bond), TL_CP_GET(_bonds_mapping_sub), TL_CP_GET(_bonds_mapping_super), TL_CP_GET(_att_points), TL_CP_GET(_fmcache),
      TL_CP_GET(_monomer_forbidden_atoms), TL_CP_GET(_product_forbidden_atoms), TL_CP_GET(_original_hydrogens)
{
    _reactant_idx = _reaction.reactantBegin();

    _fmcache.clear();
    _fragments_aam_array.clear();
    _full_product.clear();
    _full_product.clone(cur_full_product, NULL, NULL);
    _mapping.clear();
    _fragments.clear();
    _is_needless_atom.clear();
    _is_needless_bond.clear();
    _bonds_mapping_sub.clear();
    _bonds_mapping_super.clear();
    _original_hydrogens.clear();

    _att_points.clear();
    _att_points.resize(cur_full_product.vertexEnd());

    _monomer_forbidden_atoms.clear();
    _product_forbidden_atoms.clear();

    _product_monomers.clear();
    _am = NULL;
    _ee = NULL;

    is_multistep_reaction = false;
    is_self_react = false;
    is_one_tube = false;
    is_same_keeping = false;
    is_transform = false;
    _is_frag_search = false;
    _is_rg_exist = false;
    _is_simple_transform = false;

    _tube_idx = -1;

    for (int i = _reaction.reactantBegin(); i != _reaction.reactantEnd(); i = _reaction.reactantNext(i))
        if (_reaction.getQueryMolecule(i).countRSites() != 0)
            _is_rg_exist = true;

    _deep_level = 0;
    max_deep_level = 2;
    max_product_count = 1000;
    max_reuse_count = 10;

    refine_proc = NULL;
    product_proc = NULL;
    userdata = NULL;
}

ReactionEnumeratorState::ReactionEnumeratorState(ReactionEnumeratorState& cur_rpe_state)
    : _context(cur_rpe_state._context), _reaction(cur_rpe_state._reaction), _product_count(cur_rpe_state._product_count),
      _tubes_monomers(cur_rpe_state._tubes_monomers), _product_aam_array(cur_rpe_state._product_aam_array), _smiles_array(cur_rpe_state._smiles_array),
      _reaction_monomers(cur_rpe_state._reaction_monomers), CP_INIT, TL_CP_GET(_fragments_aam_array), TL_CP_GET(_full_product), TL_CP_GET(_product_monomers),
      TL_CP_GET(_mapping), TL_CP_GET(_fragments), TL_CP_GET(_is_needless_atom), TL_CP_GET(_is_needless_bond), TL_CP_GET(_bonds_mapping_sub),
      TL_CP_GET(_bonds_mapping_super), TL_CP_GET(_att_points), TL_CP_GET(_fmcache), TL_CP_GET(_monomer_forbidden_atoms), TL_CP_GET(_product_forbidden_atoms),
      TL_CP_GET(_original_hydrogens)
{
    _reactant_idx = cur_rpe_state._reactant_idx;

    _fmcache.clear();
    _fragments_aam_array.clear();
    _fragments_aam_array.copy(cur_rpe_state._fragments_aam_array);
    _full_product.clear();
    _full_product.clone(cur_rpe_state._full_product, NULL, NULL);
    _mapping.clear();
    _mapping.copy(cur_rpe_state._mapping);
    _fragments.clear();
    _fragments.clone(cur_rpe_state._fragments, NULL, NULL);
    _is_needless_atom.clear();
    _is_needless_atom.copy(cur_rpe_state._is_needless_atom);
    _is_needless_bond.clear();
    _is_needless_bond.copy(cur_rpe_state._is_needless_bond);
    _bonds_mapping_sub.clear();
    _bonds_mapping_sub.copy(cur_rpe_state._bonds_mapping_sub);
    _bonds_mapping_super.clear();
    _bonds_mapping_super.copy(cur_rpe_state._bonds_mapping_super);
    _att_points.clear();
    _monomer_forbidden_atoms.clear();
    _monomer_forbidden_atoms.copy(cur_rpe_state._monomer_forbidden_atoms);
    _product_forbidden_atoms.clear();
    _product_forbidden_atoms.copy(cur_rpe_state._product_forbidden_atoms);
    _original_hydrogens.clear();
    _original_hydrogens.copy(cur_rpe_state._original_hydrogens);

    for (int i = 0; i < cur_rpe_state._att_points.size(); i++)
    {
        Array<int>& new_array = _att_points.push();
        new_array.copy(cur_rpe_state._att_points[i]);
    }
    _am = cur_rpe_state._am;
    _ee = cur_rpe_state._ee;

    _product_monomers.clear();
    _product_monomers.copy(cur_rpe_state._product_monomers);

    max_product_count = cur_rpe_state.max_product_count;
    _tube_idx = cur_rpe_state._tube_idx;
    _deep_level = cur_rpe_state._deep_level;
    max_deep_level = cur_rpe_state.max_deep_level;
    max_reuse_count = cur_rpe_state.max_reuse_count;
    is_multistep_reaction = cur_rpe_state.is_multistep_reaction;
    is_self_react = cur_rpe_state.is_self_react;
    is_one_tube = cur_rpe_state.is_one_tube;
    is_same_keeping = cur_rpe_state.is_same_keeping;
    is_transform = cur_rpe_state.is_transform;
    _is_rg_exist = cur_rpe_state._is_rg_exist;
    _is_simple_transform = cur_rpe_state._is_simple_transform;

    _is_frag_search = false;

    refine_proc = cur_rpe_state.refine_proc;
    product_proc = cur_rpe_state.product_proc;
    userdata = cur_rpe_state.userdata;
}

int ReactionEnumeratorState::buildProduct(void)
{
    if (_product_count >= max_product_count)
        return 0;

    if (_reactant_idx == _reaction.reactantEnd())
    {
        /* Product is ready */
        _productProcess();
        return 0;
    }

    if (is_transform)
        return 0;

    for (int i = 0; i < _reaction_monomers._monomers.size(); i++)
    {
        QS_DEF(Molecule, ee_monomer);
        ee_monomer.clear();
        ee_monomer.clone(*_reaction_monomers._monomers[i], NULL, NULL);
        ee_monomer.buildCisTrans(NULL);

        if (!is_one_tube)
            if (!_isMonomerFromCurTube(i))
                continue;

        if (!is_self_react)
            if ((_reaction_monomers._deep_levels[i] != 0) && (_product_monomers.find(i) != -1))
                continue;

        ReactionEnumeratorState rpe_state(*this);

        rpe_state._deep_level += _reaction_monomers._deep_levels[i];

        if (rpe_state._deep_level - 1 > rpe_state.max_deep_level)
            return 0;

        rpe_state._product_monomers.push(i);

        rpe_state._startEmbeddingEnumerator(ee_monomer);
    }

    return 0;
}

int ReactionEnumeratorState::_findCurTube(void)
{
    int cur_tube_idx = -1;

    if (_product_monomers.size() == 0)
        return -1;

    for (int j = 0; j < _tubes_monomers.size(); j++)
    {
        int k;

        for (k = 0; k < _product_monomers.size(); k++)
            if (_tubes_monomers[j].find(_product_monomers[k]) == -1)
                break;

        if (k != _product_monomers.size())
            continue;

        if (cur_tube_idx == -1)
            cur_tube_idx = j;
        else
            return -1;
    }

    return cur_tube_idx;
}

bool ReactionEnumeratorState::_isMonomerFromCurTube(int monomer_idx)
{
    int j;
    if (!is_one_tube)
    {
        for (j = 0; j < _product_monomers.size(); j++)
        {
            if (_reaction_monomers._reactant_indexes[_product_monomers[j]] == _reaction_monomers._reactant_indexes[monomer_idx])
                break;
        }
        if (j != _product_monomers.size())
            return false;
    }

    _tube_idx = _findCurTube();

    if (_tube_idx != -1)
    {
        if (_tubes_monomers[_tube_idx].find(monomer_idx) == -1)
            return false;
    }

    /* Finding current tube index */
    else if (_reaction_monomers._tube_indexes[monomer_idx] != -1)
    {
        for (j = 0; j < _tubes_monomers.size(); j++)
        {
            if (_tubes_monomers[j].find(monomer_idx) == -1)
                continue;

            int k;

            for (k = 0; k < _product_monomers.size(); k++)
                if (_tubes_monomers[j].find(_product_monomers[k]) == -1)
                    break;

            if (k == _product_monomers.size())
                break;
        }
        if (_tubes_monomers.size() == 0)
            _tube_idx = -1;
        else if (j != _tubes_monomers.size())
            _tube_idx = j;
        else
            return false;
    }

    return true;
}

void ReactionEnumeratorState::_productProcess(void)
{
    if (_deep_level >= max_deep_level)
        return;

    QS_DEF(Molecule, ready_product);
    ready_product.clear();

    QS_DEF(Array<int>, ucfrag_mapping);
    ucfrag_mapping.clear();

    if (!_attachFragments(ready_product, ucfrag_mapping))
        return;

    if (!is_transform)
        _foldHydrogens(ready_product, 0, 0, &_mapping);

    ready_product.dearomatize(_context.arom_options);

    if (!is_same_keeping)
    {
        QS_DEF(Array<char>, cur_smiles);
        cur_smiles.clear();

        try
        {
            ArrayOutput arr_out(cur_smiles);
            CanonicalSmilesSaver product_cs_saver(arr_out);
            product_cs_saver.saveMolecule(ready_product);
        }
        catch (Exception&)
        {
            return;
        }

        cur_smiles.push(0);
        if (_smiles_array.find(cur_smiles.ptr()))
        {
            int* found_count = _smiles_array.at2(cur_smiles.ptr());
            (*found_count)++;
            return;
        }
        _product_count++;
        _smiles_array.insert(cur_smiles.ptr(), 1);
    }

    for (int i = 0; i < _product_monomers.size(); i++)
    {
        if (_reaction_monomers._monomers[_product_monomers[i]]->name.size() == 0)
            continue;

        bool is_deep = false;
        if (_reaction_monomers._monomers[_product_monomers[i]]->name.find('+') != -1)
        {
            is_deep = true;
            ready_product.name.push('(');
        }

        ready_product.name.concat(_reaction_monomers._monomers[_product_monomers[i]]->name);
        ready_product.name.pop();

        if (is_deep)
            ready_product.name.push(')');

        ready_product.name.push('+');
    }

    if (ready_product.name.size() != 0)
        ready_product.name.top() = 0;

    /* Adding a product to monomers lists */
    if (is_multistep_reaction && !is_transform)
    {
        int tube_idx = _findCurTube();

        for (int i = _reaction.reactantBegin(); i != _reaction.reactantEnd(); i = _reaction.reactantNext(i))
        {
            if (!is_one_tube)
                _tubes_monomers[tube_idx].push(_reaction_monomers.size());
            _reaction_monomers.addMonomer(i, ready_product, _deep_level + 1, tube_idx);
        }
    }

    if (!_is_simple_transform)
        ready_product.clearXyz();

    if (product_proc != NULL)
        product_proc(ready_product, _product_monomers, _mapping, userdata);
}

void ReactionEnumeratorState::_foldHydrogens(BaseMolecule& molecule, Array<int>* atoms_to_keep, Array<int>* original_hydrogens, Array<int>* mol_mapping)
{
    QS_DEF(Array<int>, hydrogens);
    hydrogens.clear();

    for (int i = molecule.vertexBegin(); i != molecule.vertexEnd(); i = molecule.vertexNext(i))
    {
        if ((atoms_to_keep != 0) && (atoms_to_keep->at(i)))
            continue;

        if ((original_hydrogens != 0) && (original_hydrogens->find(i) != -1))
            continue;

        if (molecule.getAtomNumber(i) != ELEM_H || (molecule.getAtomIsotope(i) != 0 && molecule.getAtomIsotope(i) != -1))
            continue;

        const Vertex& v = molecule.getVertex(i);

        if (v.degree() == 0)
            continue;

        if (v.degree() == 1)
        {
            int h_nei = v.neiVertex(v.neiBegin());

            if (molecule.getAtomNumber(h_nei) == ELEM_H && molecule.getAtomIsotope(h_nei) == 0)
                continue; // do not remove rare H-H fragment

            if (molecule.stereocenters.exists(h_nei) && molecule.stereocenters.getPyramid(h_nei)[3] == -1)
                continue;
        }

        hydrogens.push(i);
    }

    molecule.removeAtoms(hydrogens);

    if (mol_mapping != 0)
    {

        for (int i = 0; i < hydrogens.size(); i++)
        {
            int h_id = mol_mapping->find(hydrogens[i]);
            if (h_id != -1)
                mol_mapping->at(h_id) = -1;
        }
    }
}

bool ReactionEnumeratorState::_nextMatchProcess(EmbeddingEnumerator& ee, const QueryMolecule& reactant, const Molecule& monomer)
{
    ReactionEnumeratorState rpe_state(*this);

    rpe_state._ee = &ee;
    rpe_state._is_frag_search = _is_frag_search;

    ee.userdata = &rpe_state;

    bool stop_flag = ee.processNext();

    _bonds_mapping_sub.copy(rpe_state._bonds_mapping_sub);
    _bonds_mapping_super.copy(rpe_state._bonds_mapping_super);
    _product_forbidden_atoms.copy(rpe_state._product_forbidden_atoms);
    _original_hydrogens.copy(rpe_state._original_hydrogens);

    return stop_flag;
}

int ReactionEnumeratorState::_calcMaxHCnt(QueryMolecule& molecule)
{
    int max_possible_h_cnt = 0;
    for (int i = molecule.vertexBegin(); i != molecule.vertexEnd(); i = molecule.vertexNext(i))
    {
        int possible_h_cnt = 0;
        const Vertex& v = molecule.getVertex(i);

        for (int j = v.neiBegin(); j != v.neiEnd(); j = v.neiNext(j))
            if (molecule.possibleAtomNumber(v.neiVertex(j), ELEM_H))
                possible_h_cnt++;

        if (possible_h_cnt > max_possible_h_cnt)
            max_possible_h_cnt = possible_h_cnt;
    }

    return max_possible_h_cnt;
}

bool ReactionEnumeratorState::performSingleTransformation(Molecule& molecule, Array<int>& mapping, Array<int>& forbidden_atoms, Array<int>& original_hydrogens,
                                                          bool& need_layout)
{
    is_transform = true;

    _is_simple_transform = _checkForSimplicity();

    if (forbidden_atoms.size() != molecule.vertexEnd())
        throw Error("forbidden atoms array size is incorrect");

    _monomer_forbidden_atoms.copy(forbidden_atoms);

    _original_hydrogens.copy(original_hydrogens);

    _mapping.copy(mapping);

    if (!_startEmbeddingEnumerator(molecule))
    {
        _foldHydrogens(molecule, &forbidden_atoms, &_original_hydrogens, &_mapping);
        return false;
    }

    original_hydrogens.copy(_original_hydrogens);
    forbidden_atoms.copy(_product_forbidden_atoms);

    need_layout = !_is_simple_transform;

    return true;
}

bool ReactionEnumeratorState::_startEmbeddingEnumerator(Molecule& monomer)
{
    QS_DEF(QueryMolecule, ee_reactant);
    ee_reactant.clear();
    ee_reactant.clone(_reaction.getQueryMolecule(_reactant_idx), NULL, NULL);
    ee_reactant.buildCisTrans(NULL);

    ee_reactant.aromatize(_context.arom_options);

    for (int i = ee_reactant.edgeBegin(); i != ee_reactant.edgeEnd(); i = ee_reactant.edgeNext(i))
    {
        const Edge& edge = ee_reactant.getEdge(i);

        if (ee_reactant.isRSite(edge.beg) && ee_reactant.isRSite(edge.end))
            throw Error("one RGroup can't be a neighbor of another");
    }

    /* Finding in reactant query atoms with two neighbors */
    QS_DEF(Array<int>, qa_array);
    qa_array.clear();
    for (int j = ee_reactant.vertexBegin(); j != ee_reactant.vertexEnd(); j = ee_reactant.vertexNext(j))
    {
        const Vertex& vertex = ee_reactant.getVertex(j);

        if (!ee_reactant.isRSite(j))
            continue;

        if (vertex.degree() > 2)
            throw Error("query atom can't have more than two neighbors");

        if (vertex.degree() == 2)
            _changeQueryNode(ee_reactant, j);
    }

    QS_DEF(Molecule, ee_monomer);
    ee_monomer.clear();
    ee_monomer.clone(monomer, NULL, NULL);

    ee_monomer.aromatize(_context.arom_options);

    if (BaseMolecule::hasCoord(ee_monomer))
    {
        // Double Cis or Trans bonds are excluded from cis-trans build
        QS_DEF(Array<int>, cis_trans_excluded);
        cis_trans_excluded.clear_resize(ee_monomer.edgeEnd());
        cis_trans_excluded.zerofill();

        for (int i = ee_monomer.edgeBegin(); i < ee_monomer.edgeEnd(); i = ee_monomer.edgeNext(i))
        {
            if (ee_monomer.cis_trans.isIgnored(i))
                cis_trans_excluded[i] = 1;
        }

        ee_monomer.buildCisTrans(cis_trans_excluded.ptr());
    }

    QS_DEF(Obj<AromaticityMatcher>, am);
    am.free();
    am.create(ee_reactant, ee_monomer, _context.arom_options);
    _am = am.get();

    ee_monomer.unfoldHydrogens(NULL, _calcMaxHCnt(ee_reactant), true);

    _bonds_mapping_sub.clear_resize(ee_reactant.edgeEnd());
    _bonds_mapping_sub.fffill();
    _bonds_mapping_super.clear_resize(ee_monomer.edgeEnd());
    _bonds_mapping_super.fffill();

    EmbeddingEnumerator ee(ee_monomer);
    ee.cb_embedding = _embeddingCallback;
    ee.cb_match_vertex = _matchVertexCallback;
    ee.cb_match_edge = _matchEdgeCallback;
    ee.cb_vertex_remove = _removeAtomCallback;
    ee.cb_edge_add = _addBondCallback;
    ee.cb_allow_many_to_one = _allowManyToOneCallback;
    ee.userdata = this;
    ee.setSubgraph(ee_reactant);

    ee.allow_many_to_one = true;

    ee.processStart();

    while (true)
    {
        bool stop_flag = _nextMatchProcess(ee, ee_reactant, ee_monomer);
        if (!stop_flag)
            return false;
        if (is_transform)
            break;
    }
    return true;
}

void ReactionEnumeratorState::_changeQueryNode(QueryMolecule& ee_reactant, int change_atom_idx)
{
    QS_DEF(QueryMolecule, reactant_copy);
    reactant_copy.clear();
    reactant_copy.clone(ee_reactant, NULL, NULL);

    /* Making one query atom for one bond */
    const Vertex& vertex = ee_reactant.getVertex(change_atom_idx);
    QueryMolecule::Atom& atom = ee_reactant.getAtom(change_atom_idx);

    for (int j = vertex.neiNext(vertex.neiBegin()); j != vertex.neiEnd(); j = vertex.neiNext(j))
    {
        int nv_idx = vertex.neiVertex(j);

        int new_atom_idx = reactant_copy.addAtom(atom.clone());

        reactant_copy.setRSiteAttachmentOrder(new_atom_idx, nv_idx, 0);

        reactant_copy.flipBond(nv_idx, change_atom_idx, new_atom_idx);
    }

    ee_reactant.clone(reactant_copy, NULL, NULL);
}

bool ReactionEnumeratorState::_matchVertexCallback(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata)
{
    ReactionEnumeratorState* rpe_state = (ReactionEnumeratorState*)userdata;
    Molecule& supermolecule = (Molecule&)supergraph;
    QueryMolecule& submolecule = (QueryMolecule&)subgraph;
    QueryMolecule::Atom& qa_sub = submolecule.getAtom(sub_idx);
    const Vertex& sub_v = submolecule.getVertex(sub_idx);
    const Vertex& super_v = supermolecule.getVertex(super_idx);

    bool res = MoleculeSubstructureMatcher::matchQueryAtom(&qa_sub, supermolecule, super_idx, &(rpe_state->_fmcache), 0xFFFFFFFFUL);

    if (!res)
        return false;

    if (rpe_state->is_transform)
        if (super_idx < rpe_state->_monomer_forbidden_atoms.size()) // otherwise super atom is unfolded hydrogen
            if (rpe_state->_monomer_forbidden_atoms[super_idx] >= rpe_state->max_reuse_count)
                return false;

    if (supermolecule.getAtomNumber(super_idx) == ELEM_H && sub_v.degree() != 0 && super_v.degree() != 0)
    {
        int sub_free_rg_count = 0;

        int sub_nei = sub_v.neiVertex(sub_v.neiBegin());
        const Vertex& sub_nei_v = submolecule.getVertex(sub_nei);
        for (int i = sub_nei_v.neiBegin(); i != sub_nei_v.neiEnd(); i = sub_nei_v.neiNext(i))
        {
            if (rpe_state->_bonds_mapping_sub[sub_nei_v.neiEdge(i)] < 0 && sub_nei_v.neiVertex(i) != sub_idx)
                sub_free_rg_count++;
        }

        int super_free_atoms_count = 0;
        const Vertex& super_v = supermolecule.getVertex(super_idx);
        int super_nei = super_v.neiVertex(super_v.neiBegin());
        const Vertex& super_nei_v = supermolecule.getVertex(super_nei);
        for (int i = super_nei_v.neiBegin(); i != super_nei_v.neiEnd(); i = super_nei_v.neiNext(i))
        {
            if (supermolecule.getAtomNumber(super_nei_v.neiVertex(i)) == ELEM_H)
            {
                int h_idx = super_nei_v.neiVertex(i);
                if ((rpe_state->_ee->getSupergraphMapping()[h_idx] < 0) && (h_idx < super_idx))
                    return false;
            }
            else if (rpe_state->_bonds_mapping_super[super_nei_v.neiEdge(i)] < 0)
                super_free_atoms_count++;
        }

        if (rpe_state->_is_rg_exist && sub_free_rg_count < super_free_atoms_count)
            return false;
    }

    if (rpe_state->_is_rg_exist && !submolecule.isRSite(sub_idx) && !submolecule.isPseudoAtom(sub_idx))
    {
        int super_unfolded_h_cnt = supermolecule.getAtomTotalH(super_idx) - supermolecule.getImplicitH(super_idx);

        if (super_v.degree() - super_unfolded_h_cnt > sub_v.degree())
            return false;
    }

    return res;
}

bool ReactionEnumeratorState::_matchEdgeCallback(Graph& subgraph, Graph& supergraph, int self_idx, int other_idx, void* userdata)
{
    ReactionEnumeratorState* rpe_state = (ReactionEnumeratorState*)userdata;
    Molecule& supermolecule = (Molecule&)supergraph;
    QueryMolecule& submolecule = (QueryMolecule&)subgraph;
    QueryMolecule::Bond& qb_sub = submolecule.getBond(self_idx);

    if (rpe_state->_bonds_mapping_super[other_idx] >= 0)
        return false;

    bool res = MoleculeSubstructureMatcher::matchQueryBond(&qb_sub, supermolecule, self_idx, other_idx, rpe_state->_am, 0xFFFFFFFFUL);

    return res;
}

void ReactionEnumeratorState::_findFragAtoms(Array<byte>& unfrag_mon_atoms, QueryMolecule& submolecule, Molecule& fragment, int* core_sub, int* core_super)
{
    for (int i = submolecule.vertexBegin(); i != submolecule.vertexEnd(); i = submolecule.vertexNext(i))
    {
        if (_is_rg_exist && !submolecule.isRSite(i))
            unfrag_mon_atoms[core_sub[i]] = 1;

        const Vertex& sub_v = submolecule.getVertex(i);
        const Vertex& frag_v = fragment.getVertex(core_sub[i]);
        if (!_is_rg_exist && (sub_v.degree() == frag_v.degree()))
            unfrag_mon_atoms[core_sub[i]] = 1;
    }
}

void ReactionEnumeratorState::_cleanFragments(void)
{
    if (_is_rg_exist)
    {
        QS_DEF(Array<int>, is_attached_hydrogen);
        is_attached_hydrogen.clear();
        is_attached_hydrogen.resize(_fragments.vertexEnd());
        is_attached_hydrogen.zerofill();

        for (int i = 0; i < _att_points.size(); i++)
            for (int j = 0; j < _att_points[i].size(); j++)
                if (_fragments.getAtomNumber(_att_points[i][j]) == ELEM_H)
                    is_attached_hydrogen[_att_points[i][j]] = 1;

        for (int i = _fragments.vertexBegin(); i != _fragments.vertexEnd(); i = _fragments.vertexNext(i))
        {
            if (_fragments.getAtomNumber(i) != ELEM_H)
                continue;

            const Vertex& h = _fragments.getVertex(i);

            if (h.degree() == 0)
                continue;

            int h_nei = h.neiVertex(h.neiBegin());

            if (_fragments.stereocenters.exists(h_nei) && !_is_needless_atom[h_nei])
                continue;

            if (!is_attached_hydrogen[i])
                _fragments.removeAtom(i);
        }
    }

    for (int i = _fragments.vertexBegin(); i != _fragments.vertexEnd(); i = _fragments.vertexNext(i))
        if (_is_needless_atom[i])
            _fragments.removeAtom(i);

    for (int i = _fragments.edgeBegin(); i != _fragments.edgeEnd(); i = _fragments.edgeNext(i))
        if (_is_needless_bond[i])
            _fragments.removeBond(i);
}

void ReactionEnumeratorState::_findR2PMapping(QueryMolecule& reactant, Array<int>& mapping)
{
    const Array<int>& reactant_aam_array = _reaction.getAAMArray(_reactant_idx);

    for (int i = reactant.vertexBegin(); i != reactant.vertexEnd(); i = reactant.vertexNext(i))
    {
        if ((i >= reactant_aam_array.size()) && (!reactant.isRSite(i)))
            break;

        if (reactant.isRSite(i))
        {
            for (int j = _full_product.vertexBegin(); j != _full_product.vertexEnd(); j = _full_product.vertexNext(j))
            {
                if (!_full_product.isRSite(j))
                    continue;

                int pr_rg_idx = _full_product.getSingleAllowedRGroup(j);
                int sub_rg_idx = reactant.getSingleAllowedRGroup(i);
                if (pr_rg_idx != sub_rg_idx)
                    continue;

                mapping[i] = j;
                break;
            }
        }
        else if (reactant_aam_array[i] != 0)
            mapping[i] = _product_aam_array.find(reactant_aam_array[i]);
    }
}

void ReactionEnumeratorState::_invertStereocenters(Molecule& molecule, int edge_idx)
{
    const Edge& edge = molecule.getEdge(edge_idx);
    int edge_end_idx = edge.end;
    const Vertex& edge_end = molecule.getVertex(edge_end_idx);
    int other_end_idx = edge.findOtherEnd(edge_end_idx);

    QS_DEF(Array<int>, was_atoms);
    was_atoms.clear_resize(molecule.vertexEnd());
    was_atoms.zerofill();

    for (int i = edge_end.neiBegin(); i != edge_end.neiEnd(); i = edge_end.neiNext(i))
    {
        int nei_atom_idx = edge_end.neiVertex(i);

        if (nei_atom_idx == other_end_idx)
            continue;

        QS_DEF(Array<int>, ignored_atoms);
        ignored_atoms.clear_resize(molecule.vertexEnd());
        ignored_atoms.zerofill();
        ignored_atoms[edge_end_idx] = 1;
        QS_DEF(Array<int>, atom_ranks);
        atom_ranks.clear_resize(molecule.vertexEnd());
        atom_ranks.zerofill();
        atom_ranks[nei_atom_idx] = -1;

        DfsWalk dfs(molecule);
        dfs.ignored_vertices = ignored_atoms.ptr();
        dfs.vertex_ranks = atom_ranks.ptr();
        dfs.walk();
        const Array<DfsWalk::SeqElem>& atoms_to_reflect = dfs.getSequence();

        for (int j = 0; j < atoms_to_reflect.size(); j++)
        {
            if (((j > 0) && (atoms_to_reflect[j].parent_vertex == -1)) || was_atoms[atoms_to_reflect[j].idx])
                break;

            if (molecule.stereocenters.exists(atoms_to_reflect[j].idx))
                molecule.stereocenters.invertPyramid(atoms_to_reflect[j].idx);

            was_atoms[atoms_to_reflect[j].idx] = 1;
        }
    }
}

void ReactionEnumeratorState::_cistransUpdate(QueryMolecule& submolecule, Molecule& supermolecule, int* frag_mapping, const Array<int>& rp_mapping,
                                              int* core_sub)
{
    QS_DEF(Array<int>, cistrans_changed_bonds);
    cistrans_changed_bonds.clear();

    for (int i = submolecule.edgeBegin(); i != submolecule.edgeEnd(); i = submolecule.edgeNext(i))
    {
        if (!MoleculeCisTrans::isGeomStereoBond(submolecule, i, NULL, false))
            continue;

        const Edge& edge = submolecule.getEdge(i);
        const int* subs = submolecule.cis_trans.getSubstituents(i);

        if ((rp_mapping[edge.beg] == -1) || (rp_mapping[edge.end] == -1))
        {
            continue;
            // or throw Error("Incorrect AAM on stereo bond");
        }
        for (int j = 0; j < 2; j++)
        {
            if ((subs[j] != -1) && (rp_mapping[subs[j]] != -1) && (_full_product.findEdgeIndex(rp_mapping[subs[j]], rp_mapping[edge.beg]) == -1))
                return;
            if ((subs[j + 2] != -1) && (rp_mapping[subs[j + 2]] != -1) && (_full_product.findEdgeIndex(rp_mapping[subs[j + 2]], rp_mapping[edge.end]) == -1))
                return;
        }
        int ss_sign, sp_sign;

        ss_sign = MoleculeCisTrans::getMappingParitySign(submolecule, supermolecule, i, core_sub);
        sp_sign = MoleculeCisTrans::getMappingParitySign(submolecule, _full_product, i, rp_mapping.ptr());

        if (sp_sign > 0)
            continue;

        int product_edge_idx = _full_product.findMappedEdge(submolecule, _full_product, i, rp_mapping.ptr());
        if (product_edge_idx == -1)
            continue;

        if (_full_product.bondStereoCare(product_edge_idx))
            continue;

        if (sp_sign * ss_sign > 0)
        {
            int new_parity = _full_product.cis_trans.getParity(product_edge_idx);
            new_parity = (new_parity == MoleculeCisTrans::CIS ? MoleculeCisTrans::TRANS : MoleculeCisTrans::CIS);
            _full_product.cis_trans.setParity(product_edge_idx, new_parity);

            int super_edge_idx = supermolecule.findMappedEdge(submolecule, supermolecule, i, core_sub);
            _invertStereocenters(supermolecule, super_edge_idx);
        }
    }
}

QueryMolecule::Atom* ReactionEnumeratorState::_getReactantAtom(int atom_aam)
{
    for (int i = _reaction.reactantBegin(); i != _reaction.reactantEnd(); i = _reaction.reactantNext(i))
    {
        int atom_idx = _reaction.getAAMArray(i).find(atom_aam);
        if (atom_idx != -1)
        {
            return &_reaction.getQueryMolecule(i).getAtom(atom_idx);
        }
    }

    return NULL;
}

void ReactionEnumeratorState::_buildMolProduct(QueryMolecule& product, Molecule& mol_product, Molecule& uncleaned_fragments, Array<int>& all_forbidden_atoms,
                                               Array<int>& mapping_out)
{
    mol_product.clear();
    mapping_out.clear_resize(product.vertexEnd());
    mapping_out.fffill();

    for (int i = product.vertexBegin(); i != product.vertexEnd(); i = product.vertexNext(i))
    {
        bool has_aam = true, is_default = false;

        int pr_aam = _product_aam_array[i];
        int frags_idx = -1;
        if (pr_aam == 0)
            has_aam = false;
        else
        {
            frags_idx = _fragments_aam_array.find(pr_aam);
            if (frags_idx == -1)
                throw Error("Incorrect AAM");
        }

        int mol_atom_idx = -1;

        if (has_aam && (uncleaned_fragments.getAtomNumber(frags_idx) != product.getAtomNumber(i)) && (product.getAtomNumber(i) != -1))
            is_default = true;

        QueryMolecule::Atom* reactant_atom = _getReactantAtom(pr_aam);

        if ((product.getAtomNumber(i) == -1 && !product.isPseudoAtom(i)) && !is_default && !product.isRSite(i))
        {
            if (!has_aam)
                throw Error("Incorrect AAM");

            if (!product.getAtom(i).possibleValue(QueryMolecule::ATOM_NUMBER, uncleaned_fragments.getAtomNumber(frags_idx)))
                throw Error("product atom's impossible number");
            else
                mol_atom_idx = mol_product.addAtom(uncleaned_fragments.getAtomNumber(frags_idx));
        }
        else
        {
            mol_atom_idx = mol_product.addAtom(product.getAtomNumber(i));

            if (product.isPseudoAtom(i))
                mol_product.setPseudoAtom(i, product.getPseudoAtom(i));
        }

        /* "charge", "radical" or "isotope" parameters have no sense for pseudoatoms */
        if (!product.isPseudoAtom(i) && !(has_aam && uncleaned_fragments.isPseudoAtom(frags_idx)))
        {
            /* Charge copying */
            int reactant_atom_charge = CHARGE_UNKNOWN;
            if (reactant_atom != 0)
                reactant_atom->sureValue(QueryMolecule::ATOM_CHARGE, reactant_atom_charge);

            if ((product.getAtomCharge(i) == CHARGE_UNKNOWN) && (!is_default) && (reactant_atom_charge == product.getAtomCharge(i)))
            {
                if (has_aam)
                {
                    try
                    {
                        mol_product.setAtomCharge(mol_atom_idx, uncleaned_fragments.getAtomCharge(frags_idx));
                    }
                    catch (Element::Error&)
                    {
                    }
                    catch (Molecule::Error&)
                    {
                    }
                }
            }
            else
            {
                int pr_charge = product.getAtomCharge(i);
                mol_product.setAtomCharge(mol_atom_idx, (pr_charge != CHARGE_UNKNOWN ? pr_charge : 0));
            }

            /* Isotope copying*/
            int reactant_atom_isotope = -1;
            if (reactant_atom != 0)
                reactant_atom->sureValue(QueryMolecule::ATOM_ISOTOPE, reactant_atom_isotope);

            if ((product.getAtomIsotope(i) == -1) && (!is_default) && (reactant_atom_isotope == product.getAtomIsotope(i)))
            {
                if (has_aam)
                {
                    try
                    {
                        mol_product.setAtomIsotope(mol_atom_idx, uncleaned_fragments.getAtomIsotope(frags_idx));
                    }
                    catch (Element::Error&)
                    {
                    }
                    catch (Molecule::Error&)
                    {
                    }
                }
            }
            else
            {
                int pr_isotope = product.getAtomIsotope(i);
                mol_product.setAtomIsotope(mol_atom_idx, (pr_isotope != -1 ? pr_isotope : 0));
            }

            /* Radical copying */
            int reactant_atom_radical = -1;
            if (reactant_atom != 0)
                reactant_atom->sureValue(QueryMolecule::ATOM_RADICAL, reactant_atom_radical);

            if ((product.getAtomRadical(i) == -1) && (!is_default) && (reactant_atom_radical == product.getAtomRadical(i)))
            {
                if (has_aam)
                {
                    try
                    {
                        int frag_radical = uncleaned_fragments.getAtomRadical(frags_idx);
                        mol_product.setAtomRadical(mol_atom_idx, frag_radical);
                    }
                    catch (Element::Error&)
                    {
                    }
                    catch (Molecule::Error&)
                    {
                    }
                }
            }
            else
            {
                int pr_radical = product.getAtomRadical(i);
                mol_product.setAtomRadical(mol_atom_idx, (pr_radical != -1 ? pr_radical : 0));
            }
        }

        if (_is_simple_transform && frags_idx == -1)
            throw Error("Incorrect AAM");

        if (_is_simple_transform)
            mol_product.setAtomXyz(mol_atom_idx, uncleaned_fragments.getAtomXyz(frags_idx));
        else
            mol_product.setAtomXyz(mol_atom_idx, product.getAtomXyz(i).x, product.getAtomXyz(i).y, product.getAtomXyz(i).z);

        mapping_out[i] = mol_atom_idx;

        if (frags_idx != -1 && frags_idx < _monomer_forbidden_atoms.size())
            all_forbidden_atoms[mapping_out[i]] += _monomer_forbidden_atoms[frags_idx];
        else
            all_forbidden_atoms[mapping_out[i]] = max_reuse_count;
    }

    for (int i = product.edgeBegin(); i != product.edgeEnd(); i = product.edgeNext(i))
    {
        const Edge& pr_edge = product.getEdge(i);

        if (product.getBondOrder(i) == -1)
        {
            bool has_aam = true;

            int pr_beg_aam = _product_aam_array[pr_edge.beg];
            int pr_end_aam = _product_aam_array[pr_edge.end];
            int frags_beg = -1, frags_end = -1;

            if (pr_beg_aam != 0)
            {
                frags_beg = _fragments_aam_array.find(pr_beg_aam);
                if (frags_beg == -1)
                    throw Error("Incorrect AAM");
            }

            if (pr_end_aam != 0)
            {
                frags_end = _fragments_aam_array.find(pr_end_aam);
                if (frags_end == -1)
                    throw Error("Incorrect AAM");
            }

            if (frags_beg != -1 && frags_end == -1)
            {
                if (product.isRSite(pr_edge.end))
                {
                    frags_end = _att_points[pr_edge.end][0];
                    if (uncleaned_fragments.findEdgeIndex(frags_beg, frags_end) == -1)
                        frags_end = _att_points[pr_edge.end][1];
                }
            }
            else if (frags_beg == -1 && frags_end != -1)
            {
                if (product.isRSite(pr_edge.beg))
                {
                    frags_beg = _att_points[pr_edge.beg][0];
                    if (uncleaned_fragments.findEdgeIndex(frags_beg, frags_end) == -1)
                        frags_beg = _att_points[pr_edge.beg][1];
                }
            }

            int frags_bond_idx = -1;
            if ((frags_beg != -1) && (frags_end != -1))
                frags_bond_idx = uncleaned_fragments.findEdgeIndex(frags_beg, frags_end);

            if (frags_bond_idx == -1)
                has_aam = false;

            if (has_aam)
            {
                mol_product.addBond(mapping_out[pr_edge.beg], mapping_out[pr_edge.end], uncleaned_fragments.getBondOrder(frags_bond_idx));
            }
            else
            {
                // If there is no information about this bond in smarts
                QueryMolecule::Atom& q_pr_beg = product.getAtom(pr_edge.beg);
                QueryMolecule::Atom& q_pr_end = product.getAtom(pr_edge.end);

                // int beg_value, end_value;
                bool can_be_aromatic = product.getBond(i).possibleValue(QueryMolecule::BOND_ORDER, BOND_AROMATIC);
                bool can_be_single = product.getBond(i).possibleValue(QueryMolecule::BOND_ORDER, BOND_SINGLE);
                bool can_be_double = product.getBond(i).possibleValue(QueryMolecule::BOND_ORDER, BOND_DOUBLE);
                bool can_be_triple = product.getBond(i).possibleValue(QueryMolecule::BOND_ORDER, BOND_TRIPLE);
                if ((can_be_aromatic && can_be_single && !can_be_double && !can_be_triple)/* &&
                   (q_pr_beg.sureValue(QueryMolecule::ATOM_AROMATICITY, beg_value) &&
                    q_pr_end.sureValue(QueryMolecule::ATOM_AROMATICITY, end_value))
                    && (beg_value == ATOM_AROMATIC) && (end_value == ATOM_AROMATIC)*/)
                {
                    mol_product.addBond(mapping_out[pr_edge.beg], mapping_out[pr_edge.end], BOND_AROMATIC);
                }
                else if ((!can_be_aromatic && can_be_single && !can_be_double && !can_be_triple))
                {
                    mol_product.addBond(mapping_out[pr_edge.beg], mapping_out[pr_edge.end], BOND_SINGLE);
                }
                else
                    throw Error("There is no information about products bond #%d", i);
            }
        }
        else
            mol_product.addBond(mapping_out[pr_edge.beg], mapping_out[pr_edge.end], product.getBondOrder(i));
    }

    mol_product.buildOnSubmoleculeStereocenters(product, mapping_out.ptr());
    mol_product.buildOnSubmoleculeCisTrans(product, mapping_out.ptr());

    mol_product.mergeSGroupsWithSubmolecule(product, mapping_out);
}

void ReactionEnumeratorState::_stereocentersUpdate(QueryMolecule& submolecule, Molecule& supermolecule, const Array<int>& rp_mapping, int* core_sub,
                                                   int* core_super)
{
    QS_DEF(Array<int>, mp_mapping);
    mp_mapping.clear_resize(supermolecule.vertexEnd());
    mp_mapping.fffill();

    /* Finding of monomer to product atom to atom mapping */
    for (int i = supermolecule.vertexBegin(); i != supermolecule.vertexEnd(); i = supermolecule.vertexNext(i))
        mp_mapping[i] = ((core_super[i] != -1) ? rp_mapping[core_super[i]] : -1);

    for (int i = submolecule.vertexBegin(); i != submolecule.vertexEnd(); i = submolecule.vertexNext(i))
    {
        int sub_ex = 1, pr_ex = 1;

        if (submolecule.isRSite(i))
            continue;

        if (!submolecule.stereocenters.exists(i))
            sub_ex = -1;
        if (!_full_product.stereocenters.exists(rp_mapping[i]))
            pr_ex = -1;
        if (!supermolecule.stereocenters.exists(core_sub[i]))
            continue;

        if (rp_mapping[i] == -1)
            continue;
        if (_full_product.getVertex(rp_mapping[i]).degree() < 3)
            continue;

        int mon_type, mon_group, mon_pyramid[4];
        supermolecule.stereocenters.get(core_sub[i], mon_type, mon_group, mon_pyramid);

        int new_pr_pyramid[4];
        for (int j = 0; j < 4; j++)
            new_pr_pyramid[j] = ((mon_pyramid[j] != -1) ? mp_mapping[mon_pyramid[j]] : -1);

        MoleculeStereocenters::moveMinimalToEnd(new_pr_pyramid);

        if (new_pr_pyramid[0] == -1 || new_pr_pyramid[1] == -1 || new_pr_pyramid[2] == -1)
            continue;

        if (sub_ex * pr_ex < 0)
        {
            continue;
        }
        else if ((sub_ex < 0) && (pr_ex < 0))
        {
            /* if there is no stereo info in reaction take monomer stereo info*/
            _full_product.addStereocenters(rp_mapping[i], mon_type, mon_group, new_pr_pyramid);
            continue;
        }
        else
        {
            int rct_type = submolecule.stereocenters.getType(i);
            int pr_type = _full_product.stereocenters.getType(rp_mapping[i]);
            int pr_group = _full_product.stereocenters.getGroup(rp_mapping[i]);

            if ((rct_type == MoleculeStereocenters::ATOM_ANY) || (pr_type == MoleculeStereocenters::ATOM_ANY))
                continue;
            if (pr_type == MoleculeStereocenters::ATOM_ABS)
                continue;
            if (pr_type != MoleculeStereocenters::ATOM_OR)
            {
                pr_type = mon_type;
                pr_group = mon_group;
            }

            int mapping[4];
            /* Reactant to product stereocenter's pyramid mapping finding */
            MoleculeStereocenters::getPyramidMapping(submolecule, _full_product, i, rp_mapping.ptr(), mapping, false);

            _full_product.stereocenters.remove(rp_mapping[i]);
            _full_product.addStereocenters(rp_mapping[i], pr_type, pr_group, new_pr_pyramid);

            if (MoleculeStereocenters::isPyramidMappingRigid(mapping))
                continue;

            _full_product.stereocenters.invertPyramid(rp_mapping[i]);
            _full_product.clearBondDirections();
            _full_product.markBondsStereocenters();
            _full_product.markBondsAlleneStereo();
        }
    }
}

void ReactionEnumeratorState::_completeCisTrans(Molecule& product, Molecule& uncleaned_fragments, Array<int>& frags_mapping)
{
    for (int i = _fragments.edgeBegin(); i != _fragments.edgeEnd(); i = _fragments.edgeNext(i))
    {
        if ((_fragments.getBondOrder(i) != BOND_DOUBLE) || (_fragments.cis_trans.getParity(i) != 0) || (uncleaned_fragments.cis_trans.getParity(i) == 0))
            continue;

        const Edge& edge = uncleaned_fragments.getEdge(i);
        const int* subs = uncleaned_fragments.cis_trans.getSubstituents(i);
        int new_subs[4];

        int k;
        for (k = 0; k < 4; k++)
        {
            if (subs[k] == -1 || ((uncleaned_fragments.getAtomNumber(subs[k]) == ELEM_H) && frags_mapping[subs[k]] == -1)) // it's removed hydrogen
            {
                new_subs[k] = -1;
                continue;
            }

            if (frags_mapping[subs[k]] == -1)
            {
                int sub_aam = _fragments_aam_array[subs[k]];
                if (sub_aam == 0)
                    break;
                int substiuent = _product_aam_array.find(sub_aam);
                if (substiuent == -1)
                    break;
                new_subs[k] = substiuent;
                continue;
            }
            new_subs[k] = frags_mapping[subs[k]];
        }

        if (k < 4)
            continue;

        int pr_bond_idx = product.findEdgeIndex(frags_mapping[edge.beg], frags_mapping[edge.end]);

        /* if begin of edge in fragments matches end off edge in product subs pairs should be swaped */
        if (frags_mapping[edge.beg] == product.getEdge(pr_bond_idx).end)
        {
            std::swap(new_subs[0], new_subs[2]);
            std::swap(new_subs[1], new_subs[3]);
        }

        product.cis_trans.add(pr_bond_idx, new_subs, uncleaned_fragments.cis_trans.getParity(i));
    }
}

bool ReactionEnumeratorState::_checkValence(Molecule& mol, int atom_idx)
{
    if (mol.isPseudoAtom(atom_idx))
        return true;

    try
    {
        mol.getAtomValence(atom_idx);
    }
    catch (Element::Error&)
    {
        return false;
    }

    return true;
}

void ReactionEnumeratorState::_findFragments2ProductMapping(Array<int>& f2p_mapping)
{
    f2p_mapping.clear_resize(_fragments.vertexEnd());
    f2p_mapping.fffill();

    for (int i = _full_product.vertexBegin(); i != _full_product.vertexEnd(); i = _full_product.vertexNext(i))
    {
        int pr_aam = _product_aam_array[i];

        if (pr_aam <= 0)
            continue;

        int nei_in_fragments = -1;
        nei_in_fragments = _fragments_aam_array.find(pr_aam);

        if (nei_in_fragments == -1)
            continue;

        f2p_mapping[nei_in_fragments] = i;
    }
}

bool ReactionEnumeratorState::_attachFragments(Molecule& ready_product_out, Array<int>& ucfrag_mapping)
{
    QS_DEF(Array<int>, frags2product_mapping);
    _findFragments2ProductMapping(frags2product_mapping);

    QS_DEF(QueryMolecule, product);
    product.clear();
    product.clone(_full_product, NULL, NULL);

    QS_DEF(Molecule, uncleaned_fragments);
    uncleaned_fragments.clear();
    uncleaned_fragments.clone(_fragments, NULL, NULL);

    QS_DEF(Molecule, mol_product);
    mol_product.clear();
    QS_DEF(Array<int>, mapping);
    mapping.clear();

    QS_DEF(Array<int>, all_forbidden_atoms);
    all_forbidden_atoms.clear();

    all_forbidden_atoms.clear_resize(product.vertexEnd() + _fragments.vertexCount());
    all_forbidden_atoms.zerofill();

    for (int i = product.vertexBegin(); i != product.vertexEnd(); i = product.vertexNext(i))
        all_forbidden_atoms[i] = 1;

    _buildMolProduct(_full_product, mol_product, uncleaned_fragments, all_forbidden_atoms, mapping);

    _cleanFragments();

    QS_DEF(Array<int>, frags_mapping);
    frags_mapping.clear_resize(_fragments.vertexEnd());
    frags_mapping.fffill();
    mol_product.mergeWithMolecule(_fragments, &frags_mapping);

    for (int i = _fragments.vertexBegin(); i < _fragments.vertexEnd(); i = _fragments.vertexNext(i))
        if (i < _monomer_forbidden_atoms.size() && _monomer_forbidden_atoms[i])
            all_forbidden_atoms[frags_mapping[i]] = _monomer_forbidden_atoms[i];

    QS_DEF(Array<int>, product_mapping);
    product_mapping.clear_resize(_full_product.vertexEnd());
    for (int i = 0; i < _full_product.vertexEnd(); i++)
        product_mapping[i] = i;

    for (int i = _full_product.vertexBegin(); i != _full_product.vertexEnd(); i = _full_product.vertexNext(i))
    {
        if (_att_points[i].size() == 0)
            continue;

        const Vertex& pr_v = mol_product.getVertex(i);

        QS_DEF(Array<int>, pr_neibours);
        pr_neibours.clear();
        for (int j = pr_v.neiBegin(); j != pr_v.neiEnd(); j = pr_v.neiNext(j))
            pr_neibours.push(pr_v.neiVertex(j));

        if (_is_rg_exist && (pr_neibours.size() == 2))
            for (int j = 0; j < pr_neibours.size(); j++)
                if (_product_aam_array[pr_neibours[j]] == 0)
                    throw Error("There are no AAM on RGroups attachment points");

        if (_is_rg_exist)
        {
            if (pr_neibours.size() > 2)
                throw Error("RGroup atom can't have more than two neighbors");

            /* Setting the order of rgroup atom neighbors by AAM (less is first) */
            if (pr_neibours.size() == 2)
            {
                if (((_product_aam_array.size() > pr_neibours[0]) && (_product_aam_array.size() > pr_neibours[1])) &&
                    _product_aam_array[pr_neibours[0]] > _product_aam_array[pr_neibours[1]])
                {
                    int tmp = pr_neibours[0];
                    pr_neibours[0] = pr_neibours[1];
                    pr_neibours[1] = tmp;
                }
            }
        }

        bool is_valid = false;

        if (is_transform && _att_points[i].size() != 0 && _checkValence(mol_product, frags_mapping[_att_points[i][0]]))
            is_valid = true;

        if (_is_rg_exist)
        {
            for (int j = 0; j < pr_neibours.size(); j++)
            {
                if (mol_product.findEdgeIndex(pr_neibours[j], frags_mapping[_att_points[i][j]]) != -1)
                    return false;

                int atom_from = mapping[i];
                int atom_to = frags_mapping[_att_points[i][j]];

                if (mol_product.stereocenters.exists(atom_from) && mol_product.stereocenters.getPyramid(atom_from)[3] == -1)
                    return false;
                if (mol_product.stereocenters.exists(atom_to) && mol_product.stereocenters.getPyramid(atom_to)[3] != -1)
                    return false;

                mol_product.flipBond(pr_neibours[j], atom_from, atom_to);

                // TODO:
                // Check that corresponding R-group fragment in monomer has cis-trans bond
                // and check that AAM mapping is specified for that.
                // For example for reaction OC([*])=O>>OC([*])=O and monomer C\C=C\C(O)=O
                // product shouldn't have should have cis-trans bonds because
                // AAM is not specified on R-group atom neighbor
                // Cis-trans bonds should be saved for such reaction: O[C:1]([*])=O>>O[C:1]([*])=O
            }
            mol_product.removeAtom(mapping[i]);
        }
        else
        {
            for (int j = 0; j < _att_points[i].size(); j++)
            {
                int mon_atom = frags_mapping[_att_points[i][j]];
                int pr_atom = mapping[i];
                const Vertex& mon_v = mol_product.getVertex(mon_atom);
                const Vertex& pr_v = mol_product.getVertex(pr_atom);

                for (int k = mon_v.neiBegin(); k != mon_v.neiEnd(); k = mon_v.neiNext(k))
                    if (MoleculeCisTrans::isGeomStereoBond(mol_product, mon_v.neiEdge(k), NULL, false))
                        mol_product.cis_trans.setParity(mon_v.neiEdge(k), 0);
                if (mol_product.stereocenters.exists(mon_atom))
                    mol_product.stereocenters.remove(mon_atom);

                QS_DEF(Array<int>, neighbors);
                neighbors.clear();
                for (int k = mon_v.neiBegin(); k != mon_v.neiEnd(); k = mon_v.neiNext(k))
                    neighbors.push(mon_v.neiVertex(k));

                for (int k = 0; k < neighbors.size(); k++)
                    if (mol_product.findEdgeIndex(neighbors[k], pr_atom) == -1)
                        mol_product.flipBond(neighbors[k], mon_atom, pr_atom);

                frags_mapping[_att_points[i][j]] = pr_atom;
                mol_product.removeAtom(mon_atom);
                // if (mol_product.mergeAtoms(frags_mapping[_att_points[i][0]], mapping[i]) == -1)
                //   return false;
            }
        }

        product_mapping[mapping[i]] = frags_mapping[_att_points[i][0]];

        /*
        if (is_transform && _att_points[i].size() != 0 && is_valid && !_checkValence(mol_product, mapping[i]))
        {
           _product_forbidden_atoms.copy(_monomer_forbidden_atoms);
           _product_forbidden_atoms[_att_points[i][0]] = max_reuse_count;
           return false;
        }*/

        /* Border stereocenters copying */
        int nv_idx = 0;
        for (int j = 0; j < _att_points[i].size(); j++)
        {
            if (uncleaned_fragments.stereocenters.exists(_att_points[i][j]) && !mol_product.stereocenters.exists(frags_mapping[_att_points[i][j]]))
            {
                int type, group, pyramid[4];
                uncleaned_fragments.stereocenters.get(_att_points[i][j], type, group, pyramid);

                int new_pyramid[4];

                bool invalid_stereocenter = false;
                for (int k = 0; k < 4; k++)
                {
                    if (pyramid[k] == -1)
                        new_pyramid[k] = -1;
                    else if (!_is_needless_atom[pyramid[k]])
                        new_pyramid[k] = frags_mapping[pyramid[k]];
                    else if (frags2product_mapping[pyramid[k]] != -1)
                    {
                        new_pyramid[k] = frags2product_mapping[pyramid[k]];
                    }
                    else
                    {
                        invalid_stereocenter = true;
                        break;
                    }
                }

                if (!invalid_stereocenter)
                    mol_product.addStereocenters(frags_mapping[_att_points[i][j]], type, group, new_pyramid);
            }

            if (nv_idx == 2)
                break;
        }
    }

    /* Updating of cis-trans information on product & monomer's fragment border */
    _completeCisTrans(mol_product, uncleaned_fragments, frags_mapping);

    QS_DEF(Array<int>, out_mapping);
    out_mapping.clear_resize(mol_product.vertexEnd());
    ready_product_out.clone(mol_product, NULL, &out_mapping);

    _product_forbidden_atoms.clear_resize(ready_product_out.vertexEnd());
    _product_forbidden_atoms.zerofill();

    QS_DEF(Array<int>, temp_orig_hydr);
    temp_orig_hydr.clear();

    if (is_transform)
    {
        for (int i = mol_product.vertexBegin(); i != mol_product.vertexEnd(); i = mol_product.vertexNext(i))
            if (out_mapping[i] != -1 && all_forbidden_atoms[i])
                _product_forbidden_atoms[out_mapping[i]] = all_forbidden_atoms[i];

        for (int i = 0; i < _original_hydrogens.size(); i++)
        {
            int new_h_idx = frags_mapping[_original_hydrogens[i]];

            if (new_h_idx == -1)
                continue;

            temp_orig_hydr.push(out_mapping[new_h_idx]);
        }
        _original_hydrogens.copy(temp_orig_hydr);
    }

    ucfrag_mapping.clear_resize(_fragments.vertexEnd());
    ucfrag_mapping.fffill();

    QS_DEF(Array<int>, old_mapping);
    old_mapping.copy(_mapping);

    _mapping.clear_resize(_fragments.vertexEnd());
    _mapping.fffill();

    for (int i = uncleaned_fragments.vertexBegin(); i != uncleaned_fragments.vertexEnd(); i++)
    {
        if (frags_mapping[i] != -1)
            ucfrag_mapping[i] = frags_mapping[i];
        else if (frags2product_mapping[i] != -1)
            ucfrag_mapping[i] = frags2product_mapping[i];
        else
            continue;

        ucfrag_mapping[i] = out_mapping[ucfrag_mapping[i]];

        if (old_mapping.size() > 0)
        {
            int i_id = old_mapping.find(i);
            if ((i_id != -1) && (i_id < _mapping.size()))
                _mapping[i_id] = ucfrag_mapping[i];
        }
        else
            _mapping[i] = ucfrag_mapping[i];
    }

    if (refine_proc)
        return refine_proc(uncleaned_fragments, ready_product_out, ucfrag_mapping, userdata);

    return true;
}

bool ReactionEnumeratorState::_checkFragment(QueryMolecule& submolecule, Molecule& monomer, Array<byte>& unfrag_mon_atoms, int* core_sub)
{
    QS_DEF(ObjArray<Array<int>>, attachment_pairs);
    attachment_pairs.clear();

    QS_DEF(Molecule, fragment);
    fragment.clone(monomer, NULL, NULL);

    for (int i = submolecule.vertexBegin(); i != submolecule.vertexEnd(); i = submolecule.vertexNext(i))
    {
        if (!_is_rg_exist)
            continue;

        if (!submolecule.isRSite(i))
            continue;

        int rg_idx = submolecule.getSingleAllowedRGroup(i);

        if (attachment_pairs.size() <= rg_idx)
            attachment_pairs.expand(rg_idx + 1);

        attachment_pairs[rg_idx].push(core_sub[i]);
    }

    for (int i = fragment.vertexBegin(); i != fragment.vertexEnd(); i = fragment.vertexNext(i))
        if (unfrag_mon_atoms[i])
            fragment.removeAtom(i);

    QS_DEF(Array<int>, path);
    path.clear();

    for (int i = 0; i < attachment_pairs.size(); i++)
        if (attachment_pairs[i].size() == 2)
            if (!fragment.findPath(attachment_pairs[i][0], attachment_pairs[i][1], path))
                return false;

    return true;
}

void ReactionEnumeratorState::_checkFragmentNecessity(Array<int>& is_needless_att_point)
{
    QS_DEF(Array<int>, ranks);
    ranks.clear();
    ranks.resize(_fragments.vertexEnd());
    ranks.fill(1);

    for (int i = _fragments.vertexBegin(); i != _fragments.vertexEnd(); i = _fragments.vertexNext(i))
    {
        if (is_needless_att_point[i] != 1)
            continue;

        DfsWalk dfs(_fragments);

        ranks[i] = 0;

        dfs.ignored_vertices = _is_needless_atom.ptr();
        dfs.vertex_ranks = ranks.ptr();

        dfs.walk();

        const Array<DfsWalk::SeqElem>& sequence = dfs.getSequence();

        ranks[i] = 1;

        QS_DEF(Array<int>, needless_atoms);
        needless_atoms.clear();

        int j;
        bool is_fragment_needful = false;
        for (j = 0; j < sequence.size(); j++)
        {
            if ((sequence[j].parent_vertex == -1) && (j != 0))
                break;

            if (is_needless_att_point[sequence[j].idx] == 0)
            {
                is_fragment_needful = true;
                break;
            }

            needless_atoms.push(sequence[j].idx);
        }

        if (is_fragment_needful)
            continue;

        for (j = 0; j < needless_atoms.size(); j++)
            _is_needless_atom[needless_atoms[j]] = 1;
    }
}

bool ReactionEnumeratorState::_addFragment(Molecule& fragment, QueryMolecule& submolecule, Array<int>& rp_mapping, const Array<int>& sub_rg_atoms,
                                           int* core_sub, int* core_super)
{
    QS_DEF(Array<byte>, unfrag_mon_atoms);
    unfrag_mon_atoms.clear_resize(fragment.vertexEnd());
    unfrag_mon_atoms.zerofill();

    _findFragAtoms(unfrag_mon_atoms, submolecule, fragment, core_sub, core_super);

    // Checking for connectivity of rgroup fragments of one rgroup
    if (!_checkFragment(submolecule, fragment, unfrag_mon_atoms, core_sub))
        return false;

    const Array<int>& reactant_aam_array = _reaction.getAAMArray(_reactant_idx);
    QS_DEF(Array<int>, frag_mapping);

    for (int i = 0; i < frag_mapping.size(); i++)
        _fragments_aam_array.push(0);

    if (!_is_frag_search)
    {
        frag_mapping.clear_resize(fragment.vertexEnd());
        frag_mapping.fffill();
        _fragments.mergeWithMolecule(fragment, &frag_mapping);

        /* Fragments AAM array expanding */
        for (int i = 0; i < frag_mapping.size(); i++)
            _fragments_aam_array.push(0);
    }
    else
    {
        frag_mapping.clear_resize(_fragments.vertexEnd());
        for (int i = 0; i < frag_mapping.size(); i++)
            frag_mapping[i] = i;
    }

    /* Fragments AAM array updating */
    for (int i = submolecule.vertexBegin(); i != submolecule.vertexEnd(); i = submolecule.vertexNext(i))
    {
        if (_fragments_aam_array[frag_mapping[core_sub[i]]] != 0)
            return false;

        if (i < reactant_aam_array.size())
            _fragments_aam_array[frag_mapping[core_sub[i]]] = reactant_aam_array[i];
        else
            _fragments_aam_array[frag_mapping[core_sub[i]]] = 0;
    }

    /* _is_needless arrays expanding */
    for (int i = _is_needless_atom.size(); i < _fragments.vertexEnd(); i++)
        _is_needless_atom.push(0);
    for (int i = _is_needless_bond.size(); i < _fragments.edgeEnd(); i++)
        _is_needless_bond.push(0);
    /* marked atoms array expanding */
    // for (int i = 0; i < frag_mapping.size(); i++)
    //   _product_forbidden_atoms.push(0);

    /* _is_needless atom array updating */
    for (int i = 0; i < frag_mapping.size(); i++)
        if (unfrag_mon_atoms[i])
            _is_needless_atom[frag_mapping[i]] = 1;

    /* _is_needless bond array updating */
    for (int i = submolecule.edgeBegin(); i < submolecule.edgeEnd(); i = submolecule.edgeNext(i))
    {
        const Edge& sub_e = submolecule.getEdge(i);

        int fr_e_idx = fragment.findEdgeIndex(core_sub[sub_e.beg], core_sub[sub_e.end]);
        if (fr_e_idx == -1)
            continue;

        const Edge& fr_e = fragment.getEdge(fr_e_idx);
        if (_is_rg_exist && submolecule.isRSite(sub_e.beg) && submolecule.isRSite(sub_e.end))
        {
            _is_needless_bond[_fragments.findEdgeIndex(frag_mapping[fr_e.beg], frag_mapping[fr_e.end])] = 1;
            continue;
        }
        const Vertex& sub_beg_v = submolecule.getVertex(sub_e.beg);
        const Vertex& frag_beg_v = _fragments.getVertex(frag_mapping[core_sub[sub_e.beg]]);
        const Vertex& sub_end_v = submolecule.getVertex(sub_e.end);
        const Vertex& frag_end_v = _fragments.getVertex(frag_mapping[core_sub[sub_e.end]]);

        if ((!_is_rg_exist) && (sub_beg_v.degree() != frag_beg_v.degree()) && (sub_end_v.degree() != frag_end_v.degree()))
        {
            _is_needless_bond[_fragments.findEdgeIndex(frag_mapping[fr_e.beg], frag_mapping[fr_e.end])] = 1;
            continue;
        }
    }

    QS_DEF(Array<int>, is_needless_att_point);
    is_needless_att_point.clear();
    is_needless_att_point.resize(_fragments.vertexEnd());
    is_needless_att_point.fffill();

    for (int i = submolecule.vertexBegin(); i != submolecule.vertexEnd(); i = submolecule.vertexNext(i))
    {
        if (_is_rg_exist && !submolecule.isRSite(i))
            continue;

        const Vertex& sub_v = submolecule.getVertex(i);
        const Vertex& frag_v = _fragments.getVertex(frag_mapping[core_sub[i]]);

        if ((!_is_rg_exist) && (sub_v.degree() == frag_v.degree()))
            continue;

        int frag_rg_idx = frag_mapping[core_sub[i]];

        int pr_i = rp_mapping[i];
        if (pr_i == -1)
        {
            if (!_is_rg_exist && (reactant_aam_array[i] == 0))
                throw Error("Incorrect AAM");

            is_needless_att_point[frag_rg_idx] = 1;
            continue; // No such RGroup in product
        }
        is_needless_att_point[frag_rg_idx] = 0;

        if (_is_rg_exist)
        {
            int sub_nv_idx = sub_v.neiVertex(sub_v.neiBegin());
            _att_points[pr_i].push(frag_rg_idx);
            if (_att_points[pr_i].size() == 2)
            {
                int another_sub_v_idx = core_super[frag_mapping.find(_att_points[pr_i][0])];

                /* RGroup atom that have neighbor with less AAM is first */
                const Vertex& another_sub_v = submolecule.getVertex(another_sub_v_idx);
                int another_sub_nv_idx = another_sub_v.neiVertex(another_sub_v.neiBegin());

                if ((reactant_aam_array.size() <= another_sub_nv_idx) || (reactant_aam_array.size() <= sub_nv_idx))
                    throw Error("Incorrect AAM");

                if (reactant_aam_array[another_sub_nv_idx] > reactant_aam_array[sub_nv_idx])
                {
                    int tmp = _att_points[pr_i][0];
                    _att_points[pr_i][0] = _att_points[pr_i][1];
                    _att_points[pr_i][1] = tmp;
                }
            }
        }
        else
        {
            int frag_rg_idx = frag_mapping[core_sub[i]];
            _att_points[pr_i].push(frag_rg_idx);
        }
    }

    _checkFragmentNecessity(is_needless_att_point);

    return true;
}

bool ReactionEnumeratorState::_allowManyToOneCallback(Graph& subgraph, int sub_idx, void* userdata)
{
    QueryMolecule& submolecule = (QueryMolecule&)subgraph;

    if (submolecule.isRSite(sub_idx) && (submolecule.getVertex(sub_idx).degree() == 1))
        return true;

    return false;
}

void ReactionEnumeratorState::_removeAtomCallback(Graph& subgraph, int sub_idx, void* userdata)
{
    ReactionEnumeratorState* rpe_state = (ReactionEnumeratorState*)userdata;
    QueryMolecule& submolecule = (QueryMolecule&)subgraph;
    const Vertex& v = submolecule.getVertex(sub_idx);

    rpe_state->_am->unfixNeighbourQueryBond(sub_idx);

    for (int i = v.neiBegin(); i != v.neiEnd(); i = v.neiNext(i))
    {
        int bond_idx = v.neiEdge(i);
        int super_idx = rpe_state->_bonds_mapping_sub[bond_idx];
        if (super_idx >= 0)
        {
            rpe_state->_bonds_mapping_sub[bond_idx] = -1;
            rpe_state->_bonds_mapping_super[super_idx] = -1;
        }
    }
}

void ReactionEnumeratorState::_addBondCallback(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    ReactionEnumeratorState* rpe_state = (ReactionEnumeratorState*)userdata;

    Molecule& supermolecule = (Molecule&)supergraph;
    rpe_state->_am->fixQueryBond(sub_idx, supermolecule.getBondOrder(super_idx) == BOND_AROMATIC);

    rpe_state->_bonds_mapping_sub[sub_idx] = super_idx;
    rpe_state->_bonds_mapping_super[super_idx] = sub_idx;
}

bool ReactionEnumeratorState::_checkForNeverUsed(ReactionEnumeratorState* rpe_state, Molecule& supermolecule)
{
    int never_used_vertex = -1;
    for (int i = supermolecule.vertexBegin(); i != supermolecule.vertexEnd(); i = supermolecule.vertexNext(i))
    {
        if ((i >= rpe_state->_monomer_forbidden_atoms.size()) || (rpe_state->_monomer_forbidden_atoms[i] == 0))
        {
            never_used_vertex = i;
            return true;
        }
    }

    return false;
}

int ReactionEnumeratorState::_embeddingCallback(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata)
{
    ReactionEnumeratorState* rpe_state = (ReactionEnumeratorState*)userdata;
    Molecule& cur_monomer = (Molecule&)supergraph;
    QueryMolecule& cur_reactant = (QueryMolecule&)subgraph;

    QS_DEF(QueryMolecule, submolecule);
    submolecule.clear();
    submolecule.clone(cur_reactant, NULL, NULL);
    QS_DEF(Molecule, supermolecule);
    supermolecule.clear();
    supermolecule.clone(cur_monomer, NULL, NULL);

    if (!_checkForNeverUsed(rpe_state, supermolecule))
        return 1;

    QS_DEF(Array<int>, sub_qa_array);
    sub_qa_array.clear();
    QS_DEF(Molecule, mol_fragments);
    mol_fragments.clear();

    if (!rpe_state->_is_rg_exist && !rpe_state->_am->match(core_sub, core_super))
        return 1;

    if (!MoleculeStereocenters::checkSub(submolecule, supermolecule, core_sub, false))
        return 1;

    if (!MoleculeCisTrans::checkSub(submolecule, supermolecule, core_sub))
        return 1;

    /* Cis-Trans structure updating */
    QS_DEF(Array<int>, rp_mapping);
    rp_mapping.clear_resize(submolecule.vertexEnd());
    rp_mapping.fffill();

    rpe_state->_findR2PMapping(submolecule, rp_mapping);

    rpe_state->_cistransUpdate(submolecule, supermolecule, NULL, rp_mapping, core_sub);
    rpe_state->_stereocentersUpdate(submolecule, supermolecule, rp_mapping, core_sub, core_super);

    /* Finding indices of ignored query atoms */
    for (int i = submolecule.vertexBegin(); i != submolecule.vertexEnd(); i = submolecule.vertexNext(i))
    {
        if (submolecule.isRSite(i))
            sub_qa_array.push(i);
    }

    mol_fragments.clone(supermolecule, NULL, NULL);

    /* Updating fragments molecule */
    if (!rpe_state->_addFragment(mol_fragments, submolecule, rp_mapping, sub_qa_array, core_sub, core_super))
        return 1;

    int next_reactant_idx = rpe_state->_reaction.reactantNext(rpe_state->_reactant_idx);

    if (rpe_state->is_transform)
    {
        rpe_state->_productProcess();
        return 0;
    }

    if (rpe_state->is_one_tube && rpe_state->is_self_react)
    {
        ReactionEnumeratorState self_rxn_rpe_state(*rpe_state);
        self_rxn_rpe_state._is_frag_search = true;
        self_rxn_rpe_state._reactant_idx = next_reactant_idx;

        if (self_rxn_rpe_state._reactant_idx != self_rxn_rpe_state._reaction.reactantEnd())
            self_rxn_rpe_state._startEmbeddingEnumerator(self_rxn_rpe_state._fragments);
    }

    ReactionEnumeratorState new_rpe_state(*rpe_state);
    new_rpe_state._reactant_idx = next_reactant_idx;

    new_rpe_state.buildProduct();

    return 0;
}
