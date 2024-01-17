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

#include "reaction/query_reaction.h"
#include "molecule/elements.h"
#include "molecule/query_molecule.h"

using namespace indigo;

QueryReaction::QueryReaction()
{
}

QueryReaction::~QueryReaction()
{
}

void QueryReaction::clear()
{
    BaseReaction::clear();
    _ignorableAAM.clear();
}

QueryMolecule& QueryReaction::getQueryMolecule(int index)
{
    return (QueryMolecule&)getBaseMolecule(index);
}

Array<int>& QueryReaction::getExactChangeArray(int index)
{
    BaseMolecule& mol = *_allMolecules.at(index);
    return mol.reaction_atom_exact_change;
}

int QueryReaction::getExactChange(int index, int atom)
{
    BaseMolecule& mol = *_allMolecules.at(index);
    return mol.reaction_atom_exact_change[atom];
}

void QueryReaction::_addedBaseMolecule(int idx, int side, BaseMolecule& mol)
{
    BaseReaction::_addedBaseMolecule(idx, side, mol);
    _ignorableAAM.expand(idx + 1);
    _ignorableAAM[idx].clear_resize(mol.vertexEnd());
    _ignorableAAM[idx].zerofill();
}

void QueryReaction::makeTransposedForSubstructure(QueryReaction& other)
{
    QS_DEF(Array<int>, transposition);

    clear();

    for (int i = other.begin(); i < other.end(); i = other.next(i))
    {
        other._transposeMoleculeForSubstructure(i, transposition);
        int index = _allMolecules.add(new QueryMolecule());

        QueryMolecule& qmol = *(QueryMolecule*)_allMolecules[index];

        qmol.makeSubmolecule(other.getQueryMolecule(i), transposition, 0);

        _addedBaseMolecule(index, other._types[i], qmol);

        for (int j = 0; j < transposition.size(); j++)
        {
            getAAMArray(index).at(j) = other.getAAM(i, transposition[j]);
            getInversionArray(index).at(j) = other.getInversion(i, transposition[j]);
            getExactChangeArray(index).at(j) = other.getExactChange(i, transposition[j]);
        }

        for (int j = qmol.edgeBegin(); j != qmol.edgeEnd(); j = qmol.edgeNext(j))
        {
            const Edge& edge = getBaseMolecule(index).getEdge(j);
            int edge_idx = other.getBaseMolecule(i).findEdgeIndex(transposition[edge.beg], transposition[edge.end]);
            getReactingCenterArray(index).at(j) = other.getReactingCenter(i, edge_idx);
        }
    }
}

void QueryReaction::_transposeMoleculeForSubstructure(int index, Array<int>& transposition)
{
    QS_DEF(Array<int>, has_reacting_info);
    QueryMolecule& mol = *(QueryMolecule*)_allMolecules[index];

    Array<int>& aam = getAAMArray(index);
    Array<int>& rc = getReactingCenterArray(index);
    Array<int>& inv = getInversionArray(index);
    Array<int>& ex = getExactChangeArray(index);

    has_reacting_info.clear_resize(mol.vertexEnd());
    has_reacting_info.zerofill();
    transposition.clear();

    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (aam[i] > 0)
            has_reacting_info[i] += 4;
        if (inv[i] > 0 || ex[i] > 0)
            has_reacting_info[i] += 1;

        transposition.push(i);
    }

    for (int i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
        if (rc[i] > 0)
        {
            const Edge& edge = mol.getEdge(i);
            has_reacting_info[edge.beg] += 2;
            has_reacting_info[edge.end] += 2;
        }

    _SortingContext context(mol, has_reacting_info);

    transposition.qsort(_compare, &context);
}

int QueryReaction::_compare(int& i1, int& i2, void* c)
{
    _SortingContext& context = *(_SortingContext*)c;

    bool is_pseudo1 = context.m.isPseudoAtom(i1);
    bool is_pseudo2 = context.m.isPseudoAtom(i2);
    if (is_pseudo1 != is_pseudo2)
    {
        if (is_pseudo1)
            return -1;
        return 1;
    }

    // Compare by AAM, reacting centers and other reacting flags
    int res = context.rdata[i2] - context.rdata[i1];

    if (res != 0 || is_pseudo1)
        return res;

    // Compare by atom frequency
    int labels_by_freq[] = {ELEM_C, ELEM_H, ELEM_O, ELEM_N, ELEM_P, ELEM_F, ELEM_S, ELEM_Si, ELEM_Cl, ELEM_Br, ELEM_I, ELEM_At};

    int label1 = context.m.getAtomNumber(i1);
    int label2 = context.m.getAtomNumber(i2);
    int idx1, idx2;

    for (idx1 = 0; idx1 < NELEM(labels_by_freq); idx1++)
        if (label1 == labels_by_freq[idx1])
            break;
    for (idx2 = 0; idx2 < NELEM(labels_by_freq); idx2++)
        if (label2 == labels_by_freq[idx2])
            break;

    res = idx2 - idx1;

    if (res != 0)
        return res;

    // compare by degree
    return context.m.getVertex(i2).degree() - context.m.getVertex(i1).degree();
}

int QueryReaction::_addBaseMolecule(int side)
{
    int idx = _allMolecules.add(new QueryMolecule());
    _addedBaseMolecule(idx, side, *_allMolecules[idx]);
    return idx;
}

bool QueryReaction::aromatize(const AromaticityOptions& options)
{
    bool arom_found = false;
    for (int i = begin(); i < end(); i = next(i))
    {
        arom_found |= QueryMoleculeAromatizer::aromatizeBonds(*(QueryMolecule*)_allMolecules[i], options);
    }

    return arom_found;
}

void QueryReaction::_clone(BaseReaction& other, int index, int i, ObjArray<Array<int>>* mol_mappings)
{
    BaseMolecule& rmol = other.getBaseMolecule(i);
    // for query
    getExactChangeArray(index).resize(other.asQueryReaction().getExactChangeArray(i).size());
    if (getExactChangeArray(index).size() > 0)
    {
        for (int j = rmol.vertexBegin(); j < rmol.vertexEnd(); j = rmol.vertexNext(j))
        {
            getExactChangeArray(index).at(j) = other.asQueryReaction().getExactChange(i, mol_mappings->at(i)[j]);
        }
    }
}

QueryReaction& QueryReaction::asQueryReaction()
{
    return *this;
}

bool QueryReaction::isQueryReaction()
{
    return true;
}

BaseReaction* QueryReaction::neu()
{
    return new QueryReaction();
}

Array<int>& QueryReaction::getIgnorableAAMArray(int index)
{
    return _ignorableAAM[index];
}

int QueryReaction::getIgnorableAAM(int index, int atom)
{
    return _ignorableAAM[index][atom];
}

void QueryReaction::optimize()
{
    for (int i = begin(); i < end(); i = next(i))
        _allMolecules[i]->asQueryMolecule().optimize();
}
