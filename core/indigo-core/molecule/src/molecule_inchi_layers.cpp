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

#include "molecule/molecule_inchi_layers.h"

#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"
#include "graph/dfs_walk.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_inchi_utils.h"
#include "molecule/molecule_stereocenters.h"

using namespace indigo;
using namespace indigo::MoleculeInChILayers;

IMPL_ERROR(AbstractLayer, "InChI layer");

//
// AbstractLayer
//
AbstractLayer::AbstractLayer() : _mol(0)
{
}

void AbstractLayer::construct(Molecule& mol)
{
    _mol = &mol;
    _construct();
}

Molecule& AbstractLayer::_getMolecule()
{
    if (_mol == 0)
        throw Error("_mol == 0 (internal error)");

    return *_mol;
}

//
// MainLayerFormula
//

void MainLayerFormula::_construct()
{
    _collectAtomsCount();
}

void MainLayerFormula::_collectAtomsCount()
{
    Molecule& mol = _getMolecule();

    _atoms_count.resize(ELEM_MAX);
    _atoms_count.zerofill();

    int implicit_hydrogens_count = 0;
    for (int v_idx = mol.vertexBegin(); v_idx != mol.vertexEnd(); v_idx = mol.vertexNext(v_idx))
    {
        implicit_hydrogens_count += mol.getImplicitH(v_idx);
        _atoms_count[mol.getAtomNumber(v_idx)]++;
    }

    _atoms_count[ELEM_H] += implicit_hydrogens_count;
}

void MainLayerFormula::printFormula(Array<char>& result)
{
    ArrayOutput output(result);

    bool carbon_present = (_atoms_count[ELEM_C] != 0);
    if (carbon_present)
    {
        _printAtom(output, ELEM_C);
        _printAtom(output, ELEM_H);
    }

    const Array<int>& atom_lables_sorted = MoleculeInChIUtils::getLexSortedAtomLables();
    for (int i = 0; i < atom_lables_sorted.size(); i++)
    {
        int label = atom_lables_sorted[i];
        if (carbon_present && (label == ELEM_C || label == ELEM_H))
            continue;
        _printAtom(output, label);
    }
    result.push(0);
}

void MainLayerFormula::_printAtom(Output& output, int label) const
{
    int count = _atoms_count[label];
    if (count != 0)
    {
        output.printf("%s", Element::toString(label));
        if (count != 1)
            output.printf("%d", count);
    }
}

int MainLayerFormula::compareComponentsAtomsCountNoH(MainLayerFormula& comp1, MainLayerFormula& comp2)
{
    const Array<int>& atom_lables_sorted = MoleculeInChIUtils::getLexSortedAtomLables();

    for (int i = 0; i < atom_lables_sorted.size(); i++)
    {
        int label = atom_lables_sorted[i];
        if (label == ELEM_H)
            continue;
        int diff = comp1._atoms_count[label] - comp2._atoms_count[label];
        if (diff != 0)
            return -diff;
    }
    return 0;
}

int MainLayerFormula::compareComponentsTotalHydrogensCount(MainLayerFormula& comp1, MainLayerFormula& comp2)
{
    // Compare total number of hydrogens.
    // H3 < H2
    int diff = comp2._atoms_count[ELEM_H] - comp1._atoms_count[ELEM_H];
    if (diff != 0)
        return diff;
    return 0;
}

//
// MainLayerConnections
//

void MainLayerConnections::_construct()
{
    _linearizeConnectionTable();
}

void MainLayerConnections::_linearizeConnectionTable()
{
    const Molecule& mol = _getMolecule();

    _connection_table.clear();

    QS_DEF(Array<int>, nei_array);
    for (int v_idx = mol.vertexBegin(); v_idx != mol.vertexEnd(); v_idx = mol.vertexNext(v_idx))
    {
        const Vertex& vertex = mol.getVertex(v_idx);

        nei_array.clear();
        for (int nei_idx = vertex.neiBegin(); nei_idx != vertex.neiEnd(); nei_idx = vertex.neiNext(nei_idx))
        {
            int nei_vertex_index = vertex.neiVertex(nei_idx);
            if (v_idx > nei_vertex_index)
                nei_array.push(nei_vertex_index);
        }

        _connection_table.push(v_idx);
        MoleculeInChIUtils::stableSmallSort(nei_array, NULL);
        for (int i = 0; i < nei_array.size(); i++)
            _connection_table.push(nei_array[i]);
    }
}

int MainLayerConnections::compareMappings(const MoleculeInChIUtils::Mapping& m1, const MoleculeInChIUtils::Mapping& m2)
{
    Molecule& mol = _getMolecule();

    // Compare low triangles of connection tables row by row
    QS_DEF(Array<int>, tmp1);
    QS_DEF(Array<int>, tmp2);

    for (int i = 0; i < m1.mapping.size(); i++)
    {
        const Vertex& v1 = mol.getVertex(m1.mapping[i]);
        const Vertex& v2 = mol.getVertex(m2.mapping[i]);

        tmp1.clear();
        for (int j = v1.neiBegin(); j != v1.neiEnd(); j = v1.neiNext(j))
        {
            int v_idx = v1.neiVertex(j);

            if (m1.inv_mapping[v_idx] == -1)
                continue;

            tmp1.push(m1.inv_mapping[v_idx]);
        }
        MoleculeInChIUtils::stableSmallSort(tmp1, NULL);

        tmp2.clear();
        for (int j = v2.neiBegin(); j != v2.neiEnd(); j = v2.neiNext(j))
        {
            int v_idx = v2.neiVertex(j);

            if (m2.inv_mapping[v_idx] == -1)
                continue;

            tmp2.push(m2.inv_mapping[v_idx]);
        }
        MoleculeInChIUtils::stableSmallSort(tmp2, NULL);

        // Compare rows
        if (tmp1.size() != tmp2.size())
            throw Error("Internal error: vertices degree must be the same");

        for (int j = 0; j < tmp1.size(); j++)
        {
            int ret = tmp1[j] - tmp2[j];
            if (ret != 0)
                return -ret;
        }
    }
    return 0;
}

int MainLayerConnections::compareComponentsConnectionTables(MainLayerConnections& comp1, MainLayerConnections& comp2)
{
    // longer connection table first
    int size1 = comp1._connection_table.size();
    int size2 = comp2._connection_table.size();
    int diff = size1 - size2;
    if (diff != 0)
        return -diff;

    // greater connection table first
    for (int i = 0; i < size1; i++)
    {
        int value1 = comp1._connection_table[i];
        int value2 = comp2._connection_table[i];

        int diff = value1 - value2;
        if (diff != 0)
            return -diff;
    }
    return 0;
}

void MainLayerConnections::printConnectionTable(Array<char>& result)
{
    Molecule& cano_mol = _getMolecule();

    result.clear();
    if (cano_mol.edgeCount() == 0)
    {
        result.push(0);
        return;
    }
    ArrayOutput output(result);

    QS_DEF(Array<int>, vertex_ranks);
    vertex_ranks.clear_resize(cano_mol.vertexEnd());
    for (int i = 0; i < cano_mol.vertexEnd(); i++)
        vertex_ranks[i] = i;

    // Set minimum rank to the first vertex
    int min_degree = cano_mol.vertexEnd(), min_degree_vertex = -1;
    for (int v_idx = cano_mol.vertexBegin(); v_idx != cano_mol.vertexEnd(); v_idx = cano_mol.vertexNext(v_idx))
    {
        const Vertex& v = cano_mol.getVertex(v_idx);
        if (min_degree > v.degree())
        {
            min_degree = v.degree();
            min_degree_vertex = v_idx;
        }
    }
    vertex_ranks[min_degree_vertex] = -1;

    DfsWalk dfs_walk(cano_mol);
    dfs_walk.vertex_ranks = vertex_ranks.ptr();
    dfs_walk.walk();

    // Calculate size of the descedants of each vertex in DFS-tree
    const Array<DfsWalk::SeqElem>& sequence = dfs_walk.getSequence();
    QS_DEF(Array<int>, descedants_size);
    descedants_size.clear_resize(cano_mol.vertexEnd());
    descedants_size.zerofill();

    for (int i = sequence.size() - 1; i >= 0; i--)
    {
        const DfsWalk::SeqElem& item = sequence[i];

        if (item.parent_vertex != -1)
        {
            if (dfs_walk.isClosure(item.parent_edge))
                descedants_size[item.parent_vertex] += 1;
            else
            {
                descedants_size[item.idx]++;
                descedants_size[item.parent_vertex] += descedants_size[item.idx];
            }
        }
        else
            descedants_size[item.idx]++;
    }

    // DFS-walk based on previous walk but with order based on descedants_size
    QS_DEF(Array<int>, edge_in_dfs);
    edge_in_dfs.clear_resize(cano_mol.edgeEnd());
    edge_in_dfs.zerofill();
    for (int i = sequence.size() - 1; i >= 0; i--)
    {
        const DfsWalk::SeqElem& item = sequence[i];

        if (item.parent_edge != -1 && !dfs_walk.isClosure(item.parent_edge))
            edge_in_dfs[item.parent_edge] = 1;
    }

    QS_DEF(Array<int>, vertex_visited);
    vertex_visited.clear_resize(cano_mol.vertexEnd());
    vertex_visited.zerofill();

    QS_DEF(Array<int>, vertex_stack);
    vertex_stack.push(min_degree_vertex);

    QS_DEF(Array<int>, nei_visited_vertices);
    QS_DEF(Array<int>, nei_dfs_next_vertices);

    enum
    {
        PRINT_BRACKET = -1,
        PRINT_COMMA = -2
    };
    bool need_print_dash = false;
    while (vertex_stack.size() != 0)
    {
        int cur_vertex_index = vertex_stack.pop();
        // Print close bracket or comma if necessary
        if (cur_vertex_index == PRINT_BRACKET)
        {
            output.writeString(")");
            need_print_dash = false;
            continue;
        }
        else if (cur_vertex_index == PRINT_COMMA)
        {
            output.writeString(",");
            need_print_dash = false;
            continue;
        }

        if (need_print_dash)
            output.writeString("-");
        output.printf("%d", cur_vertex_index + 1);
        need_print_dash = true;

        const Vertex& cur_vertex = cano_mol.getVertex(cur_vertex_index);

        vertex_visited[cur_vertex_index] = 1;

        // Collect neighbourhood visited vertices and
        // neighbourhood vertices in DFS-tree
        nei_visited_vertices.clear();
        nei_dfs_next_vertices.clear();
        for (int nei_idx = cur_vertex.neiBegin(); nei_idx != cur_vertex.neiEnd(); nei_idx = cur_vertex.neiNext(nei_idx))
        {
            int nei_edge = cur_vertex.neiEdge(nei_idx);
            int nei_vertex = cur_vertex.neiVertex(nei_idx);
            if (edge_in_dfs[nei_edge])
            {
                if (!vertex_visited[nei_vertex])
                    nei_dfs_next_vertices.push(nei_vertex);
            }
            else
            {
                if (vertex_visited[nei_vertex])
                    nei_visited_vertices.push(nei_vertex);
            }
        }

        // Sort nei_visited_vertices according to their index
        MoleculeInChIUtils::stableSmallSort(nei_visited_vertices, NULL);
        // Sort nei_dfs_next_vertices by descedants_size and then by their index
        MoleculeInChIUtils::stableSmallSort(nei_dfs_next_vertices, NULL);
        MoleculeInChIUtils::stableSmallSort(nei_dfs_next_vertices, &descedants_size);

        int total_neighbours = nei_visited_vertices.size() + nei_dfs_next_vertices.size();
        if (total_neighbours > 1)
        {
            output.printf("(");
            need_print_dash = false;
        }
        int left_neighbours = total_neighbours;

        // Print visited neighbourhood vertices
        for (int i = 0; i < nei_visited_vertices.size(); i++)
        {
            if (total_neighbours > 1 && left_neighbours == 1)
                output.printf(")");
            else if (i != 0)
                output.printf(",");
            else if (total_neighbours == 1)
                output.printf("-");
            output.printf("%d", nei_visited_vertices[i] + 1);
            left_neighbours--;
        }

        if (total_neighbours >= 2 && nei_dfs_next_vertices.size() > 1 && nei_visited_vertices.size() > 0)
            output.printf(",");

        // Add next DFS-vertices to the stack in backward order
        for (int i = nei_dfs_next_vertices.size() - 1; i >= 0; i--)
        {
            vertex_stack.push(nei_dfs_next_vertices[i]);
            if (total_neighbours > 1 && left_neighbours == i + 1)
                vertex_stack.push(PRINT_BRACKET);
            else if (i != 0)
                vertex_stack.push(PRINT_COMMA);
        }
    }

    result.push(0);
}

//
// HydrogensLayer
//

void HydrogensLayer::_construct()
{
    Molecule& mol = _getMolecule();

    _atom_indices.clear();
    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
        _atom_indices.push(v);

    // Collect number of immobile and mobile hydrogens
    _per_atom_immobile.clear_resize(mol.vertexEnd());
    _per_atom_immobile.zerofill();
    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
        _per_atom_immobile[v] = mol.getImplicitH(v);
}

int HydrogensLayer::compareComponentsHydrogens(HydrogensLayer& comp1, HydrogensLayer& comp2)
{
    // Compare number of hydrogens for each atom.
    // C < CH4 < CH3 < CH2 < ...

    if (comp1._atom_indices.size() != comp2._atom_indices.size())
        // At this step number of atoms should be the same
        throw Error("Algorithmic error: Number of atoms should be the same");

    for (int i = 0; i < comp1._atom_indices.size(); i++)
    {
        int v1_idx = comp1._atom_indices[i];
        int v2_idx = comp2._atom_indices[i];

        int hyd1 = comp1._per_atom_immobile[v1_idx];
        int hyd2 = comp2._per_atom_immobile[v2_idx];

        int diff = MoleculeInChIUtils::compareHydrogens(hyd1, hyd2);
        if (diff != 0)
            return diff;
    }

    return 0;
}

void HydrogensLayer::print(Array<char>& result)
{
    // Print hydrogens sublayer for the main layer
    ArrayOutput output(result);

    // Find maximum number of hydrogens
    const Array<int>& hydrogens = _per_atom_immobile;
    int max_hydrogens = 0;
    for (int i = 0; i < hydrogens.size(); i++)
        if (max_hydrogens < hydrogens[i])
            max_hydrogens = hydrogens[i];

    // Print atoms indices for each number of hydrogens
    for (int h_num = 1; h_num <= max_hydrogens; h_num++)
    {
        int next_value_in_range = -1;
        bool print_range = false;

        for (int i = 0; i < hydrogens.size(); i++)
            if (hydrogens[i] == h_num)
            {
                if (next_value_in_range == i)
                {
                    next_value_in_range = i + 1;
                    print_range = true;
                    continue;
                }
                else
                {
                    if (print_range)
                        output.printf("-%d", next_value_in_range);
                    if (next_value_in_range != -1)
                        output.printf(",");
                }

                output.printf("%d", i + 1);
                print_range = false;
                next_value_in_range = i + 1;
            }

        if (next_value_in_range == -1)
            continue; // No atoms have such number of hydrogens

        // Print last atom index
        if (print_range)
        {
            output.printf("-%d", next_value_in_range);
        }

        output.writeString("H");
        if (h_num != 1)
            output.printf("%d", h_num);
        output.writeString(",");
    }

    // Remove last comma
    if (result.size() != 0)
        result.pop();

    result.push(0);
}

bool HydrogensLayer::checkAutomorphism(const Array<int>& mapping)
{
    Molecule& mol = _getMolecule();

    // Check that atoms have the same number of hydrogens
    for (int v_idx = mol.vertexBegin(); v_idx != mol.vertexEnd(); v_idx = mol.vertexNext(v_idx))
    {
        int mapped_idx = mapping[v_idx];
        if (mapped_idx == -1)
            continue;
        int hyd = _per_atom_immobile[v_idx];
        int hyd_mapped = _per_atom_immobile[mapped_idx];

        if (hyd != hyd_mapped)
            return false;
    }
    return true;
}

int HydrogensLayer::compareMappings(MoleculeInChIUtils::Mapping& m1, MoleculeInChIUtils::Mapping& m2)
{
    // Compare number of immobile hydrogens
    for (int i = 0; i < m1.mapping.size(); i++)
    {
        int hyd1 = _per_atom_immobile[m1.mapping[i]];
        int hyd2 = _per_atom_immobile[m2.mapping[i]];
        int diff = MoleculeInChIUtils::compareHydrogens(hyd1, hyd2);
        if (diff != 0)
            return diff;
    }
    return 0;
}

//
// CisTransStereochemistryLayer
//

void CisTransStereochemistryLayer::print(Array<char>& result)
{
    // Print hydrogens sublayer for the main layer
    ArrayOutput output(result);

    Molecule& mol = _getMolecule();
    QS_DEF(Array<int[2]>, dbl);

    dbl.clear_resize(mol.vertexEnd());
    dbl.fffill();

    for (int e_idx = mol.edgeBegin(); e_idx != mol.edgeEnd(); e_idx = mol.edgeNext(e_idx))
    {
        if (!bond_is_cis_trans[e_idx])
            continue; // This is not cis-trans bond

        const Edge& e = mol.getEdge(e_idx);

        int max_vertex = std::max(e.beg, e.end);
        int min_vertex = std::min(e.beg, e.end);

        int(&cp)[2] = dbl[max_vertex];

        cp[0] = min_vertex;
        cp[1] = e_idx;
    }
    for (int i = 0; i < mol.vertexEnd(); i++)
    {
        if (dbl[i][1] == -1)
            continue;

        if (result.size() != 0)
            output.printf(",");
        output.printf("%d-%d", i + 1, dbl[i][0] + 1);

        int parity = MoleculeInChIUtils::getParityInChI(mol, dbl[i][1]);
        if (parity == -1)
            output.printf("-");
        else
            output.printf("+");
    }

    result.push(0);
}

bool CisTransStereochemistryLayer::checkAutomorphism(const Array<int>& mapping)
{
    Molecule& mol = _getMolecule();

    Filter edge_filter(bond_is_cis_trans.ptr(), Filter::EQ, 1);

    if (!MoleculeCisTrans::isAutomorphism(mol, mapping, &edge_filter))
        return false;
    return true;
}

int CisTransStereochemistryLayer::compareMappings(const MoleculeInChIUtils::Mapping& m1, const MoleculeInChIUtils::Mapping& m2)
{
    Molecule& mol = _getMolecule();

    // Compare cis-trans for double bonds (>X=Y<) and cumulene (>W=X=Y=Z<).
    // TODO: handle cumulene

    QS_DEF(Array<int[2]>, dbl1);
    QS_DEF(Array<int[2]>, dbl2);

    dbl1.clear_resize(m1.mapping.size());
    dbl1.zerofill();
    dbl2.clear_resize(m1.mapping.size());
    dbl2.zerofill();

    for (int e_idx = mol.edgeBegin(); e_idx != mol.edgeEnd(); e_idx = mol.edgeNext(e_idx))
    {
        if (!bond_is_cis_trans[e_idx])
            continue; // This is not cis-trans bond

        const Edge& e = mol.getEdge(e_idx);

        int max_vertex = std::max(e.beg, e.end);
        int min_vertex = std::min(e.beg, e.end);

        // Get mapped parity
        int parity1;
        if (mol.cis_trans.applyMapping(e_idx, m1.mapping.ptr(), false) == MoleculeCisTrans::TRANS)
            parity1 = 1;
        else
            parity1 = 2;

        int parity2;
        if (mol.cis_trans.applyMapping(e_idx, m2.mapping.ptr(), false) == MoleculeCisTrans::TRANS)
            parity2 = 1;
        else
            parity2 = 2;

        int(&cp1)[2] = dbl1[m1.inv_mapping[max_vertex]];
        int(&cp2)[2] = dbl2[m1.inv_mapping[max_vertex]];

        cp1[0] = m1.inv_mapping[min_vertex];
        cp1[1] = parity1;

        cp2[0] = m2.inv_mapping[min_vertex];
        cp2[1] = parity2;
    }
    for (int i = 0; i < m1.mapping.size(); i++)
    {
        // Compare second vertex
        int diff = dbl1[i][0] - dbl2[i][0];
        if (diff != 0)
            return diff;

        // Compare parities
        diff = dbl1[i][1] - dbl2[i][1];
        if (diff != 0)
            return diff;
    }
    return 0;
}

void CisTransStereochemistryLayer::_construct()
{
    Molecule& mol = _getMolecule();

    bond_is_cis_trans.clear_resize(mol.edgeEnd());
    bond_is_cis_trans.zerofill();
    for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
    {
        bond_is_cis_trans[e] = true;
        if (mol.getBondTopology(e) == TOPOLOGY_RING)
            bond_is_cis_trans[e] = false;
        if (mol.cis_trans.getParity(e) == 0)
            bond_is_cis_trans[e] = false;
    }
}

int CisTransStereochemistryLayer::compareComponents(CisTransStereochemistryLayer& comp1, CisTransStereochemistryLayer& comp2)
{
    // TODO
    return 0;
}

//
// TetrahedralStereochemistryLayer
//

void TetrahedralStereochemistryLayer::print(Array<char>& result)
{
    ArrayOutput output(result);

    Molecule& mol = _getMolecule();
    int first_sign = 0;
    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
    {
        int sign = _getMappingSign(mol.stereocenters, 0, v);
        if (first_sign == 0)
            first_sign = -sign;
        if (sign != 0)
        {
            if (result.size() != 0)
                output.printf(",");
            output.printf("%d%c", v + 1, sign * first_sign == 1 ? '+' : '-');
        }
    }
    result.push(0);
}

int TetrahedralStereochemistryLayer::_getFirstSign()
{
    // Find first sign
    Molecule& mol = _getMolecule();
    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
    {
        int sign = _getMappingSign(mol.stereocenters, 0, v);
        if (sign != 0)
            return sign;
    }
    return 0;
}

void TetrahedralStereochemistryLayer::printEnantiomers(Array<char>& result)
{
    ArrayOutput output(result);

    int sign = _getFirstSign();
    if (sign != 0)
        output.printf("%d", sign == 1 ? 1 : 0);
    else
        output.printf(".");
    result.push(0);
}

bool TetrahedralStereochemistryLayer::checkAutomorphism(const Array<int>& mapping)
{
    Molecule& mol = _getMolecule();

    if (!MoleculeStereocenters::isAutomorphism(mol, mapping))
        return false;
    return true;
}

int TetrahedralStereochemistryLayer::compareMappings(const MoleculeInChIUtils::Mapping& m1, const MoleculeInChIUtils::Mapping& m2)
{
    Molecule& mol = _getMolecule();

    QS_DEF(Array<int[2]>, dbl1);
    QS_DEF(Array<int[2]>, dbl2);

    dbl1.clear_resize(m1.mapping.size());
    dbl1.zerofill();
    dbl2.clear_resize(m1.mapping.size());
    dbl2.zerofill();

    const MoleculeStereocenters& stereocenters = mol.stereocenters;

    int first_sign1 = 0, first_sign2 = 0;
    for (int i = 0; i < m1.mapping.size(); i++)
    {
        int s1 = _getMappingSign(stereocenters, &m1, i);
        int s2 = _getMappingSign(stereocenters, &m2, i);
        if (first_sign1 == 0)
            first_sign1 = -s1;
        if (first_sign2 == 0)
            first_sign2 = -s2;

        int diff = s1 * first_sign1 - s2 * first_sign2;
        if (diff != 0)
            return diff;
    }
    return 0;
}

int TetrahedralStereochemistryLayer::_getMappingSign(const MoleculeStereocenters& stereocenters, const MoleculeInChIUtils::Mapping* m, int index)
{
    int src_vertex = index;
    if (m != 0)
        src_vertex = m->mapping[index];
    if (!stereocenters.exists(src_vertex))
        return 0;

    int pyramid[4];
    memcpy(pyramid, stereocenters.getPyramid(src_vertex), 4 * sizeof(int));

    // Apply mapping to the pyramid
    if (m != 0)
        for (int i = 0; i < 4; i++)
        {
            if (pyramid[i] != -1)
                pyramid[i] = m->inv_mapping[pyramid[i]];
        }

    MoleculeStereocenters::moveMinimalToEnd(pyramid);

    int cnt = 0;
    for (int i = 0; i < 3; i++)
        if (pyramid[i] > pyramid[(i + 1) % 3])
            cnt++;
    if (cnt % 2 == 0)
        return 1;
    return -1;
}

int TetrahedralStereochemistryLayer::compareComponentsEnantiomers(TetrahedralStereochemistryLayer& comp1, TetrahedralStereochemistryLayer& comp2)
{
    int s1 = comp1._getFirstSign();
    int s2 = comp2._getFirstSign();
    return s2 - s1;
}

int TetrahedralStereochemistryLayer::compareComponents(TetrahedralStereochemistryLayer& comp1, TetrahedralStereochemistryLayer& comp2)
{
    return 0;
}
