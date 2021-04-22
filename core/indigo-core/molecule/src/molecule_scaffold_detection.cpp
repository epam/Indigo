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

#include "molecule/molecule_scaffold_detection.h"
#include "base_cpp/array.h"
#include "molecule/max_common_submolecule.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/query_molecule.h"

using namespace indigo;

IMPL_ERROR(MoleculeScaffoldDetection, "Molecule Scaffold detection");

MoleculeScaffoldDetection::MoleculeScaffoldDetection(ObjArray<Molecule>* mol_set) : ScaffoldDetection(0), searchStructures(mol_set), basketStructures(0)
{
    cbEdgeWeight = matchBonds;
    cbVerticesColor = matchAtoms;
}

void MoleculeScaffoldDetection::_searchScaffold(QueryMolecule& scaffold, bool approximate)
{
    QS_DEF(ObjArray<QueryMolecule>, temp_set);
    if (basketStructures == 0)
    {
        basketStructures = &temp_set;
    }
    MoleculeBasket mol_basket;
    mol_basket.initBasket(searchStructures, basketStructures, GraphBasket::MAX_MOLECULES_NUMBER);

    if (approximate)
        _searchApproximateScaffold(mol_basket);
    else
        _searchExactScaffold(mol_basket);

    int max_index = mol_basket.getMaxGraphIndex();
    if (basketStructures->size() == 0)
        throw Error("There are no scaffolds found");

    // clear all stereocenters
    for (int i = 0; i < basketStructures->size(); ++i)
        basketStructures->at(i).stereocenters.clear();
    // get max scaffold from basket
    scaffold.clone(basketStructures->at(max_index), 0, 0);
}

void MoleculeScaffoldDetection::clone(QueryMolecule& mol, Molecule& other)
{
    QS_DEF(Array<int>, v_list);
    QS_DEF(Array<int>, e_list);
    v_list.clear();
    e_list.clear();
    for (int v_idx = other.vertexBegin(); v_idx != other.vertexEnd(); v_idx = other.vertexNext(v_idx))
    {
        v_list.push(v_idx);
    }
    for (int e_idx = other.edgeBegin(); e_idx != other.edgeEnd(); e_idx = other.edgeNext(e_idx))
    {
        e_list.push(e_idx);
    }
    makeEdgeSubmolecule(mol, other, v_list, e_list);
}

void MoleculeScaffoldDetection::makeEdgeSubmolecule(QueryMolecule& mol, Molecule& other, Array<int>& v_list, Array<int>& e_list)
{
    QS_DEF(Array<int>, tmp_mapping);
    Array<int>* v_mapping = 0;
    mol.clear();
    int i;

    if (v_mapping == 0)
        v_mapping = &tmp_mapping;

    v_mapping->clear_resize(other.vertexEnd());

    for (i = other.vertexBegin(); i < other.vertexEnd(); i = other.vertexNext(i))
        v_mapping->at(i) = -1;

    for (i = 0; i < v_list.size(); i++)
    {
        int idx = v_list[i];

        if (v_mapping->at(idx) != -1)
            throw Error("makeEdgeSubmolecule(): repeated vertex #%d", idx);

        v_mapping->at(idx) = mol.addAtom(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, other.getAtomNumber(idx)));
    }

    for (i = 0; i < e_list.size(); i++)
    {
        int edge_idx = e_list[i];
        const Edge& edge = other.getEdge(edge_idx);
        int beg = v_mapping->at(edge.beg);
        int end = v_mapping->at(edge.end);

        mol.addBond(beg, end, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, other.getBondOrder(edge_idx)));
    }
}

bool MoleculeScaffoldDetection::matchBonds(Graph& g1, Graph& g2, int i, int j, void*)
{
    BaseMolecule& mol1 = (BaseMolecule&)g1;
    BaseMolecule& mol2 = (BaseMolecule&)g2;
    QueryMolecule::Bond* q_bond = 0;
    int sub_idx, super_idx;
    BaseMolecule* target = 0;
    if (mol1.isQueryMolecule())
    {
        q_bond = &mol1.asQueryMolecule().getBond(i);
        sub_idx = i;
        super_idx = j;
        target = &mol2;
    }
    else if (mol2.isQueryMolecule())
    {
        q_bond = &mol2.asQueryMolecule().getBond(j);
        sub_idx = j;
        super_idx = i;
        target = &mol1;
    }
    else
    {
        return MoleculeExactMatcher::matchBonds(mol1, mol2, i, j, MoleculeExactMatcher::CONDITION_ELECTRONS);
    }
    return MoleculeSubstructureMatcher::matchQueryBond(q_bond, *target, sub_idx, super_idx, 0, 0xFFFFFFFF);
}

bool MoleculeScaffoldDetection::matchAtoms(Graph& g1, Graph& g2, const int*, int i, int j, void* userdata)
{
    BaseMolecule& mol1 = (BaseMolecule&)g1;
    BaseMolecule& mol2 = (BaseMolecule&)g2;
    QueryMolecule::Atom* q_atom = 0;
    int super_idx;
    BaseMolecule* target = 0;
    if (mol1.isQueryMolecule())
    {
        q_atom = &mol1.asQueryMolecule().getAtom(i);
        super_idx = j;
        target = &mol2;
    }
    else if (mol2.isQueryMolecule())
    {
        q_atom = &mol2.asQueryMolecule().getAtom(j);
        super_idx = i;
        target = &mol1;
    }
    else
    {
        return MoleculeExactMatcher::matchAtoms(mol1, mol2, i, j, 0);
    }

    return MoleculeSubstructureMatcher::matchQueryAtom(q_atom, *target, super_idx, 0, 0xFFFFFFFF);
}

IMPL_ERROR(MoleculeScaffoldDetection::MoleculeBasket, "Mol basket");

MoleculeScaffoldDetection::MoleculeBasket::MoleculeBasket() : cbSortSolutions(0), _searchStructures(0), _basketStructures(0)
{
}

MoleculeScaffoldDetection::MoleculeBasket::~MoleculeBasket()
{
}

void MoleculeScaffoldDetection::MoleculeBasket::initBasket(ObjArray<Molecule>* mol_set, ObjArray<QueryMolecule>* basket_set, int max_number)
{

    if (mol_set == 0)
        throw Error("Graph set null pointer");
    if (basket_set == 0)
        throw Error("Basket set null pointer");

    _searchStructures = mol_set;
    _basketStructures = basket_set;

    _sortGraphsInSet();

    _basketStructures->clear();

    for (int i = 0; i < max_number; i++)
        _basketStructures->push();

    _directIterator.resize(max_number);
    _reverseIterator.resize(max_number);
    _reverseIterator.set();

    clone(_basketStructures->at(0), _searchStructures->at(_orderArray[0]));
    _reverseIterator.set(0, false);
    _directIterator.set(0);
}

QueryMolecule& MoleculeScaffoldDetection::MoleculeBasket::pickOutNextMolecule()
{

    int empty_index = _reverseIterator.nextSetBit(0);

    if (empty_index == -1)
    {
        _directIterator.resize(_directIterator.size() + NEXT_SOLUTION_SIZE_SUM);
        _reverseIterator.resize(_directIterator.size());
        for (int i = _directIterator.size() - NEXT_SOLUTION_SIZE_SUM; i < _directIterator.size(); i++)
            _reverseIterator.set(i);
        empty_index = _basketStructures->size();
        for (int i = 0; i < NEXT_SOLUTION_SIZE_SUM; i++)
            _basketStructures->push();
    }

    _reverseIterator.set(empty_index, false);
    return _basketStructures->at(empty_index);
}

void MoleculeScaffoldDetection::MoleculeBasket::addToNextEmptySpot(Graph& graph, Array<int>& v_list, Array<int>& e_list)
{
    Molecule& mol = (Molecule&)graph;
    QueryMolecule& b_mol = pickOutNextMolecule();
    makeEdgeSubmolecule(b_mol, mol, v_list, e_list);
}

int MoleculeScaffoldDetection::MoleculeBasket::getMaxGraphIndex()
{

    for (int x = _reverseIterator.nextSetBit(0); x >= 0; x = _reverseIterator.nextSetBit(x + 1))
    {
        QueryMolecule& mol_basket = _basketStructures->at(x);

        if (mol_basket.vertexCount() > 0)
            mol_basket.clear();
    }

    if (cbSortSolutions == 0)
        _basketStructures->qsort(_compareRingsCount, 0);
    else
        _basketStructures->qsort(cbSortSolutions, userdata);

    while (_basketStructures->size() && _basketStructures->top().vertexCount() == 0)
        _basketStructures->pop();

    return 0;
}

Graph& MoleculeScaffoldDetection::MoleculeBasket::getGraph(int index) const
{
    if (index >= _basketStructures->size())
        throw Error("basket size < index");
    return (Graph&)_basketStructures->at(index);
}

void MoleculeScaffoldDetection::MoleculeBasket::_sortGraphsInSet()
{
    int set_size = _searchStructures->size();

    if (set_size == 0)
        throw Error("Graph set size == 0");

    _orderArray.clear();
    for (int i = 0; i < set_size; i++)
    {
        if (_searchStructures->at(i).vertexCount() > 0)
        {
            _orderArray.push(i);
            ++_graphSetSize;
        }
    }

    // sort in order of edgeCount
    _orderArray.qsort(_compareEdgeCount, _searchStructures);
}

int MoleculeScaffoldDetection::MoleculeBasket::_compareEdgeCount(int& i1, int& i2, void* context)
{
    ObjArray<Molecule>& graph_set = *(ObjArray<Molecule>*)context;
    return graph_set.at(i1).edgeCount() - graph_set.at(i2).edgeCount();
}

int MoleculeScaffoldDetection::MoleculeBasket::_compareRingsCount(BaseMolecule& g1, BaseMolecule& g2, void*)
{
    // maximize number of the rings/ v-e+r=2 there v- number of vertices e - number of edges r - number of rings
    int result = (g2.edgeCount() - g2.vertexCount()) - (g1.edgeCount() - g1.vertexCount());
    if (result == 0 || g1.edgeCount() == 0 || g2.edgeCount() == 0)
        result = g2.edgeCount() - g1.edgeCount();
    return result;
}
