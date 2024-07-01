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

#include "molecule/molecule_tautomer.h"

#include "base_cpp/queue.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_fingerprint.h"
#include "molecule/molecule_substructure_matcher.h"

using namespace indigo;

// Tautomer superstrucure - molecule with all bonds that can appear in tautomer
// This structure is needed to enumerate all submolecule of a tautomer

// Tautomer template rules:
// A - any atom
// G - heteroatom: N, O, S, P, As, Sb, Se, Te
// Tautomer bond appear/disappear template:
//    G=C   G-H        <-->  G-C-G-H
//    (A=A-)nG=C  G-H  <-->  H-(A-A=)nG-C-G
// Atoms can't have any other double/triple bonds
// Let's say thet C can emit bond and G with H can accept bond

CP_DEF(TautomerSuperStructure);

TautomerSuperStructure::TautomerSuperStructure(Molecule& mol)
    : CP_INIT, TL_CP_GET(_atomsEmitBond), TL_CP_GET(_atomsAcceptBond), TL_CP_GET(_isBondAttachedArray), TL_CP_GET(_mapping), TL_CP_GET(_inv_mapping),
      TL_CP_GET(_edge_mapping), TL_CP_GET(_total_h)
{
    int i;

    _inside_ctor = true;

    clone(mol, &_inv_mapping, &_mapping);

    _edge_mapping.clear_resize(edgeEnd());
    _edge_mapping.fffill();

    for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        const Edge& edge = mol.getEdge(i);

        _edge_mapping[findEdgeIndex(_inv_mapping[edge.beg], _inv_mapping[edge.end])] = i;
    }

    // Collect atom properties
    _collectAtomProperties();

    // Detect distances from _atomsEmitBond elements to _atomsAcceptBond elements
    QS_DEF(Array<int>, distancesMatrix);
    distancesMatrix.resize(_atomsEmitBond.size() * _atomsAcceptBond.size());
    for (i = 0; i < _atomsEmitBond.size(); i++)
    {
        int* result = distancesMatrix.ptr() + _atomsAcceptBond.size() * i;
        _findMinDistance(_atomsEmitBond[i], 6, _atomsAcceptBond, result);
    }

    QS_DEF(Array<int>, attachedBonds);
    attachedBonds.clear();
    for (i = 0; i < _atomsEmitBond.size(); i++)
        for (int j = 0; j < _atomsAcceptBond.size(); j++)
        {
            int v1 = _atomsEmitBond[i];
            int v2 = _atomsAcceptBond[j];
            if (findEdgeIndex(v1, v2) != -1)
                continue;
            // Check new loop size: 5 or 6
            int size = distancesMatrix[_atomsAcceptBond.size() * i + j];
            if (size != 4 && size != 5)
                continue;

            attachedBonds.push(addEdge(v1, v2));
        }

    _isBondAttachedArray.resize(edgeEnd());
    _isBondAttachedArray.zerofill();
    for (i = 0; i < attachedBonds.size(); i++)
        _isBondAttachedArray[attachedBonds[i]] = true;

    _inside_ctor = false;
}

TautomerSuperStructure::~TautomerSuperStructure()
{
}

void TautomerSuperStructure::clear()
{
    if (_inside_ctor)
        Molecule::clear();
    else
        throw Exception("clear(): not supported");
}

int TautomerSuperStructure::getBondOrder(int idx) const
{
    if (!_inside_ctor && _isBondAttachedArray[idx])
        return -1;
    return Molecule::getBondOrder(idx);
}

int TautomerSuperStructure::getBondTopology(int idx)
{
    if (!_inside_ctor && _isBondAttachedArray[idx])
        return -1;
    return Molecule::getBondTopology(idx);
}

bool TautomerSuperStructure::possibleBondOrder(int idx, int order)
{
    if (!_inside_ctor && _isBondAttachedArray[idx])
        return order == 0 || order == BOND_SINGLE;

    return Molecule::possibleBondOrder(idx, order);
}

int TautomerSuperStructure::getSubgraphType(const Array<int>& /*vertices*/, const Array<int>& edges)
{
    // For any atoms number of attached bonds must be 0 or 1

    QS_DEF(Array<int>, per_vertex_attached_bonds);

    per_vertex_attached_bonds.clear_resize(vertexEnd());
    per_vertex_attached_bonds.zerofill();

    int attached_bonds = 0;
    for (int i = 0; i < edges.size(); i++)
    {
        int edge_index = edges[i];

        if (!_isBondAttachedArray[edge_index])
            continue;

        const Edge& edge = getEdge(edge_index);
        per_vertex_attached_bonds[edge.beg]++;
        per_vertex_attached_bonds[edge.end]++;
        if (per_vertex_attached_bonds[edge.beg] > 1 || per_vertex_attached_bonds[edge.end] > 1)
            return NONE;
        attached_bonds++;
    }

    if (attached_bonds == 0)
        return ORIGINAL;
    return TAUTOMER;
}

void TautomerSuperStructure::_getDoubleBondsCount(int i, int& double_count, int& arom_count)
{
    double_count = 0;
    arom_count = 0;

    const Vertex& vertex = getVertex(i);
    for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
    {
        int idx = vertex.neiEdge(j);

        if (idx >= _edge_mapping.size())
            continue;

        if (Molecule::getBondOrder(idx) == BOND_DOUBLE || Molecule::getBondOrder(idx) == BOND_TRIPLE)
            double_count++;
        else if (Molecule::getBondOrder(idx) == BOND_AROMATIC)
            arom_count++;
    }
}

bool TautomerSuperStructure::_isAcceptingHeteroatom(int idx)
{
    static const int list[] = {ELEM_N, ELEM_O, ELEM_P, ELEM_S, ELEM_As, ELEM_Se, ELEM_Sb, ELEM_Te};

    return atomNumberBelongs(idx, list, NELEM(list));
}

bool TautomerSuperStructure::_isEmittingHeteroatom(int idx)
{
    static const int list[] = {ELEM_O, ELEM_N};

    return atomNumberBelongs(idx, list, NELEM(list));
}

int TautomerSuperStructure::getAtomTotalH(int idx)
{
    return _total_h[idx];
}

void TautomerSuperStructure::_collectAtomProperties(void)
{
    _atomsAcceptBond.clear();
    _atomsEmitBond.clear();
    _total_h.clear_resize(vertexEnd());

    for (int i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
    {
        _total_h[i] = 0;
        if (!isPseudoAtom(i) && !isRSite(i) && !isTemplateAtom(i))
            _total_h[i] = Molecule::getAtomTotalH(i);

        int double_bonds_count, arom_bonds_count;

        _getDoubleBondsCount(i, double_bonds_count, arom_bonds_count);

        // Detect atom type
        if (double_bonds_count == 0 && _isAcceptingHeteroatom(i))
        {
            bool have_hydrogen = (getAtomTotalH(i) != 0);

            if (arom_bonds_count != 0)
                have_hydrogen = true;

            if (have_hydrogen)
                _atomsAcceptBond.push(i);
        }

        // Aromatic bond can be double
        if ((double_bonds_count == 1 || arom_bonds_count != 0) && getAtomNumber(i) == ELEM_C)
        {
            int nei_heteroatoms = _hetroatomsCount(i);
            if (nei_heteroatoms != 1)
                continue;

            // Check all possible double bond neighbors to have emit atom type
            const Vertex& vertex = getVertex(i);
            for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
            {
                // Double bond can jump from non-heteroatom to heteroatom
                // that why only existance of heteroatom and existance of any attached double bond
                // is required (double bond count already has been checked above)

                if (!_isEmittingHeteroatom(vertex.neiVertex(j)))
                    continue;

                int nei_double_bonds_count, nei_arom_bonds_count;
                _getDoubleBondsCount(vertex.neiVertex(j), nei_double_bonds_count, nei_arom_bonds_count);
                if (nei_double_bonds_count <= 1)
                {
                    _atomsEmitBond.push(i);
                    break;
                }
            }
        }
    }
}

// Find minimum distance between source vertex and vertices from dest array
// and check tautomer chain property:
//    * only one bond can be in ring
//    * if no one bond is in ring then only one bond can be double and no one can be triple
void TautomerSuperStructure::_findMinDistance(int source, int maxDist, Array<int>& dest, int* result)
{
    QS_DEF(Array<int>, distances);
    QS_DEF(Array<int>, parents);

    distances.resize(vertexEnd());
    parents.resize(vertexEnd());

    // Fill distances by infinity
    for (int j = 0; j < distances.size(); j++)
    {
        distances[j] = INT_MAX / 2;
        parents[j] = -1;
    }
    QS_DEF(Queue<int>, front);
    front.clear();
    front.setLength(vertexEnd());

    distances[source] = 0;
    parents[source] = -1;

    front.push(source);
    while (!front.isEmpty())
    {
        int active = front.pop();
        if (distances[active] == maxDist)
            break;

        const Vertex& vertex = getVertex(active);
        for (int j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
        {
            int vn = vertex.neiVertex(j);
            if (distances[vn] == INT_MAX / 2)
            {
                distances[vn] = distances[active] + 1;
                parents[vn] = active;
                front.push(vn);
            }
        }
    }
    for (int j = 0; j < dest.size(); j++)
    {
        int cur = dest[j];
        int cur_dest = dest[j];
        // Check chain
        if (distances[cur_dest] != INT_MAX / 2)
        {
            int inRingCount = 0;
            int doubleBondsCount = 0, tripleBondsCount = 0;

            int prev = parents[cur];
            while (prev != -1)
            {
                int edge_idx = findEdgeIndex(cur, prev);
                if (edge_idx >= 0)
                {
                    if (Molecule::getBondTopology(edge_idx) == TOPOLOGY_RING)
                        inRingCount++;
                    if (Molecule::getBondOrder(edge_idx) == BOND_DOUBLE)
                        doubleBondsCount++;
                    if (Molecule::getBondOrder(edge_idx) == BOND_TRIPLE)
                        tripleBondsCount++;
                }

                cur = prev;
                prev = parents[prev];
            }

            if (inRingCount > 1)
                distances[cur_dest] = INT_MAX;
            else if (inRingCount == 0)
                if (doubleBondsCount > 1 || tripleBondsCount > 0)
                    distances[cur_dest] = INT_MAX;
        }
        result[j] = distances[cur_dest];
    }
}

int TautomerSuperStructure::_hetroatomsCount(int idx)
{
    const Vertex& vertex = getVertex(idx);
    int count = 0;
    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        if (_isAcceptingHeteroatom(vertex.neiVertex(i)))
            count++;
    }
    return count;
}

const int* TautomerSuperStructure::getMapping()
{
    return _mapping.ptr();
}

const Array<int>& TautomerSuperStructure::getInvMapping()
{
    return _inv_mapping;
}

bool TautomerSuperStructure::isZeroedBond(int idx)
{
    return _isBondAttachedArray[idx];
}
