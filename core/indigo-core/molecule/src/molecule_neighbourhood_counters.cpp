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

#include "molecule/molecule_neighbourhood_counters.h"

#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

using namespace indigo;

void MoleculeAtomNeighbourhoodCounters::calculate(Molecule& mol)
{
    _calculate(mol, false);
}

void MoleculeAtomNeighbourhoodCounters::calculate(QueryMolecule& mol)
{
    _calculate(mol, true);
}

void MoleculeAtomNeighbourhoodCounters::_calculate(BaseMolecule& mol, bool is_query)
{
    _per_atom_counters.resize(mol.vertexEnd());
    _per_atom_counters.zerofill();

    _use_atom.resize(mol.vertexEnd());
    _use_atom.zerofill();

    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (!mol.possibleAtomNumber(i, ELEM_H) && !mol.isRSite(i))
            _use_atom[i] = 1;
        /*
        if (mol.getAtomNumber(i) != -1 && mol.getAtomNumber(i) != ELEM_H)
           _use_atom[i] = 1;
        */
    }

    _calculateLevel0(mol, is_query);
    for (int r = 1; r < Counters::RADIUS; r++)
        _calculateNextLevel(mol, r);

    // Find differences to determine counter in neighbourhood ring slices
    for (int r = Counters::RADIUS - 1; r > 0; r--)
    {
        for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
        {
            Counters& cnt = _per_atom_counters[i];
            CountersPerRadius& cur = cnt.per_rad[r];
            CountersPerRadius& prev = cnt.per_rad[r - 1];

            cur.C_cnt -= prev.C_cnt;
            cur.hetero_cnt -= prev.hetero_cnt;
            cur.heteroN_cnt -= prev.heteroN_cnt;
            cur.heteroO_cnt -= prev.heteroO_cnt;
            cur.trip_cnt -= prev.trip_cnt;

            cur.degree_sum -= prev.degree_sum;
        }
    }
}

bool MoleculeAtomNeighbourhoodCounters::_isAtomInformationStored(int atom_idx) const
{
    if (atom_idx >= _use_atom.size() || _use_atom[atom_idx] == 0)
        return false;
    return true;
}

bool MoleculeAtomNeighbourhoodCounters::testSubstructure(const MoleculeAtomNeighbourhoodCounters& target_counters, int query_atom_idx, int target_atom_idx,
                                                         bool use_bond_types) const
{
    // Temporary patch to fix issure with RGroups
    if (!_isAtomInformationStored(query_atom_idx))
        return true;
    if (!target_counters._isAtomInformationStored(target_atom_idx))
        return true;
    // End

    const Counters& target_atom_counters = target_counters._per_atom_counters[target_atom_idx];
    const Counters& query_atom_counters = _per_atom_counters[query_atom_idx];

    return query_atom_counters.testSubstructure(target_atom_counters, use_bond_types);
}

bool MoleculeAtomNeighbourhoodCounters::CountersPerRadius::testSubstructure(const CountersPerRadius& target, bool use_bond_types) const
{
    if (C_cnt > target.C_cnt)
        return false;
    if (hetero_cnt > target.hetero_cnt)
        return false;
    if (heteroN_cnt > target.heteroN_cnt)
        return false;
    if (heteroO_cnt > target.heteroO_cnt)
        return false;
    if (trip_cnt > target.trip_cnt)
        return false;
    if (use_bond_types)
        if (degree_sum > target.degree_sum)
            return false;
    return true;
}

bool MoleculeAtomNeighbourhoodCounters::Counters::testSubstructure(const Counters& target, bool use_bond_types) const
{
    for (int i = RADIUS - 1; i >= 0; i--)
    {
        if (!per_rad[i].testSubstructure(target.per_rad[i], use_bond_types))
            return false;
    }
    return true;
}

void MoleculeAtomNeighbourhoodCounters::_calculateLevel0(BaseMolecule& mol, bool is_query)
{
    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        const Vertex& v = mol.getVertex(i);
        Counters& cnt = _per_atom_counters[i];
        CountersPerRadius& cnt0 = cnt.per_rad[0];

        if (!_use_atom[i])
            continue;

        int bonds_count = 0;
        for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
        {
            int nei_e = v.neiEdge(nei);

            int nei_vertex = v.neiVertex(nei);
            if (!_use_atom[nei_vertex])
                continue;

            int bond_order = mol.getBondOrder(nei_e);

            if (bond_order == BOND_SINGLE || bond_order == BOND_DOUBLE || bond_order == BOND_TRIPLE)
                cnt0.degree_sum += bond_order;
            else
            {
                if (is_query)
                    cnt0.degree_sum++; // Query or aromatic bond
                else
                    cnt0.degree_sum += 3; // Aromatic bond
            }

            bonds_count++;
            if (mol.getBondTopology(nei_e) == TOPOLOGY_RING)
                cnt0.in_ring_cnt++;
        }

        if (bonds_count >= 3)
            cnt0.trip_cnt = 1;

        int label = mol.getAtomNumber(i);
        if (label != -1)
        {
            if (label == ELEM_N)
                cnt0.heteroN_cnt++;
            else if (label == ELEM_O)
                cnt0.heteroO_cnt++;
            else if (label != ELEM_C)
                cnt0.hetero_cnt++;
            else
                cnt0.C_cnt++;
        }
    }
}

void MoleculeAtomNeighbourhoodCounters::_calculateNextLevel(BaseMolecule& mol, int r)
{
    // Calculate counters in each vertex neighbourhood with radius from 1 to RADIUS
    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        Counters& cnt = _per_atom_counters[i];
        CountersPerRadius& cnt_r = cnt.per_rad[r];

        if (!_use_atom[i])
            continue;

        const Vertex& v = mol.getVertex(i);
        int degree = 0;
        for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
        {
            int nei_vertex = v.neiVertex(nei);

            if (!_use_atom[nei_vertex])
                continue;

            Counters& nei_cnt = _per_atom_counters[nei_vertex];
            CountersPerRadius& nei_cnt_r = nei_cnt.per_rad[r - 1];

            cnt_r.C_cnt += nei_cnt_r.C_cnt;
            cnt_r.hetero_cnt += nei_cnt_r.hetero_cnt;
            cnt_r.heteroN_cnt += nei_cnt_r.heteroN_cnt;
            cnt_r.heteroO_cnt += nei_cnt_r.heteroO_cnt;
            cnt_r.in_ring_cnt += nei_cnt_r.in_ring_cnt;
            cnt_r.trip_cnt += nei_cnt_r.trip_cnt;

            cnt_r.degree_sum += nei_cnt_r.degree_sum;

            degree++;
        }

        if (r == 1)
        {
            CountersPerRadius& cnt_r1 = cnt.per_rad[r - 1];

            cnt_r.C_cnt += cnt_r1.C_cnt;
            cnt_r.hetero_cnt += cnt_r1.hetero_cnt;
            cnt_r.heteroN_cnt += cnt_r1.heteroN_cnt;
            cnt_r.heteroO_cnt += cnt_r1.heteroO_cnt;
            cnt_r.trip_cnt += cnt_r1.trip_cnt;
            cnt_r.degree_sum += cnt_r1.degree_sum;
        }
        else
        {
            int deg_1 = degree - 1;
            CountersPerRadius& cnt_r2 = cnt.per_rad[r - 2];

            cnt_r.C_cnt -= cnt_r2.C_cnt * deg_1;
            cnt_r.hetero_cnt -= cnt_r2.hetero_cnt * deg_1;
            cnt_r.heteroN_cnt -= cnt_r2.heteroN_cnt * deg_1;
            cnt_r.heteroO_cnt -= cnt_r2.heteroO_cnt * deg_1;
            cnt_r.trip_cnt -= cnt_r2.trip_cnt * deg_1;
            cnt_r.degree_sum -= cnt_r2.degree_sum * deg_1;
        }
    }
}

int MoleculeAtomNeighbourhoodCounters::_countersCmp(int& i1, int& i2, void* abstract_context)
{
    const MoleculeAtomNeighbourhoodCounters::Context* context = (const MoleculeAtomNeighbourhoodCounters::Context*)abstract_context;
    const MoleculeAtomNeighbourhoodCounters* self = context->cnt;

    // Check queryatoms
    bool is_query1 = (context->mol->getAtomNumber(i1) == -1);
    bool is_query2 = (context->mol->getAtomNumber(i2) == -1);
    if (is_query1 && !is_query2)
        return 1;
    if (!is_query1 && is_query2)
        return -1;

    const Counters& c1 = self->_per_atom_counters[i1];
    const Counters& c2 = self->_per_atom_counters[i2];

    const CountersPerRadius& c1r0 = c1.per_rad[0];
    const CountersPerRadius& c2r0 = c2.per_rad[0];
    int is_hetero1 = c1r0.hetero_cnt + c1r0.heteroN_cnt + c1r0.heteroO_cnt;
    int is_hetero2 = c2r0.hetero_cnt + c2r0.heteroN_cnt + c2r0.heteroO_cnt;

    // Heteroatoms
    if (is_hetero1 != is_hetero2)
        return is_hetero2 - is_hetero1;

    // Check loop
    if (c1r0.in_ring_cnt != c2r0.in_ring_cnt)
        return c2r0.in_ring_cnt - c1r0.in_ring_cnt;

    // Rare heteroatoms
    if (c1r0.hetero_cnt != c2r0.hetero_cnt)
        return c2r0.hetero_cnt - c1r0.hetero_cnt;

    for (int r = 0; r < Counters::RADIUS; r++)
    {
        const CountersPerRadius& c1r = c1.per_rad[r];
        const CountersPerRadius& c2r = c2.per_rad[r];

        int hetero1_sum = c1r.hetero_cnt + c1r.heteroN_cnt + c1r.heteroO_cnt;
        int hetero2_sum = c2r.hetero_cnt + c2r.heteroN_cnt + c2r.heteroO_cnt;

        if (hetero2_sum != hetero1_sum)
            return hetero2_sum - hetero1_sum;
    }

    int degree_sum1 = 0;
    int degree_sum2 = 0;
    for (int r = 0; r < Counters::RADIUS; r++)
    {
        degree_sum1 += c1.per_rad[r].degree_sum;
        degree_sum2 += c2.per_rad[r].degree_sum;
    }
    return degree_sum2 - degree_sum1;
}

void MoleculeAtomNeighbourhoodCounters::makeTranspositionForSubstructure(BaseMolecule& mol, Array<int>& output) const
{
    output.clear();
    if (mol.vertexCount() == 0)
        return;

    QS_DEF(Array<int>, sorted_indices);
    sorted_indices.clear();
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        sorted_indices.push(i);

    Context context;
    context.cnt = this;
    context.mol = &mol;
    sorted_indices.qsort(_countersCmp, &context);

    QS_DEF(Array<int>, vertices_state);
    enum
    {
        FREE = 0,
        FRONT = 1,
        PROCESSED = 2
    };
    vertices_state.resize(mol.vertexEnd());
    for (int i = 0; i < vertices_state.size(); i++)
        vertices_state[i] = FREE;

    // Choose first vertex
    int first_vertex = sorted_indices[0];
    vertices_state[first_vertex] = FRONT;

    while (true)
    {
        // Choose next vertex in front
        int front_vertex = -1;
        for (int i = 0; i < sorted_indices.size(); i++)
        {
            int idx = sorted_indices[i];
            if (vertices_state[idx] == FRONT)
            {
                front_vertex = idx;
                break;
            }
        }

        if (front_vertex == -1)
        {
            // If query isn't 1-connected than add other vertex to front
            int free_vertex = -1;
            for (int i = 0; i < sorted_indices.size(); i++)
            {
                int idx = sorted_indices[i];
                if (vertices_state[idx] == FREE)
                {
                    free_vertex = idx;
                    break;
                }
            }
            if (free_vertex == -1)
                break;

            vertices_state[free_vertex] = FRONT;
            front_vertex = free_vertex;
        }

        output.push(front_vertex);
        vertices_state[front_vertex] = PROCESSED;

        // Add neiboughbours vertices to front
        const Vertex& v = mol.getVertex(front_vertex);
        for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
        {
            int nei_vertex = v.neiVertex(nei);
            if (vertices_state[nei_vertex] != PROCESSED)
                vertices_state[nei_vertex] = FRONT;
        }
    }

    if (output.size() != mol.vertexCount())
        throw Exception("Internal error in makeTranspositionForSubstructure");
}
