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

#include "graph/biconnected_decomposer.h"
#include "graph/cycle_enumerator.h"
#include "graph/embedding_enumerator.h"
#include "graph/morgan_code.h"
#include "layout/attachment_layout.h"
#include "layout/molecule_layout_graph.h"

#include <memory>

using namespace indigo;

enum
{
    QUERY_BOND_SINGLE_OR_DOUBLE = 5,
    QUERY_BOND_SINGLE_OR_AROMATIC = 6,
    QUERY_BOND_DOUBLE_OR_AROMATIC = 7,
    QUERY_BOND_ANY = 8
};

// Make relative coordinates of a component absolute
void MoleculeLayoutGraph::_copyLayout(MoleculeLayoutGraph& component)
{
    int i;

    for (i = component.vertexBegin(); i < component.vertexEnd(); i = component.vertexNext(i))
    {
        LayoutVertex& vert = component._layout_vertices[i];

        _layout_vertices[vert.ext_idx].pos.copy(vert.pos);
        _layout_vertices[vert.ext_idx].type = vert.type;
    }

    for (i = component.edgeBegin(); i < component.edgeEnd(); i = component.edgeNext(i))
    {
        LayoutEdge& edge = component._layout_edges[i];

        _layout_edges[edge.ext_idx].type = edge.type;
    }
}

static int _vertex_cmp(int& n1, int& n2, void* context)
{
    const MoleculeLayoutGraph& graph = *(MoleculeLayoutGraph*)context;
    const LayoutVertex& v1 = graph.getLayoutVertex(n1);
    const LayoutVertex& v2 = graph.getLayoutVertex(n2);

    if (v1.is_cyclic != v2.is_cyclic)
    {
        if (v1.is_cyclic == true)
            return 1;
        return -1;
    }

    return v1.morgan_code - v2.morgan_code;
}

void MoleculeLayoutGraph::_assignAbsoluteCoordinates(float bond_length)
{
    BiconnectedDecomposer bc_decom(*this);
    QS_DEF(Array<int>, bc_tree);
    PtrArray<MoleculeLayoutGraph> bc_components;
    QS_DEF(Array<int>, fixed_components);
    bool all_trivial = true;

    int n_comp = bc_decom.decompose();

    fixed_components.clear_resize(n_comp);
    fixed_components.zerofill();

    bc_components.clear();

    for (int i = 0; i < n_comp; i++)
    {
        Filter comp;
        bc_decom.getComponent(i, comp);
        std::unique_ptr<MoleculeLayoutGraph> tmp(getInstance());
        tmp->makeLayoutSubgraph(*this, comp);
        bc_components.add(tmp.release());
    }

    bc_tree.clear_resize(vertexEnd());
    _makeComponentsTree(bc_decom, bc_components, bc_tree);

    // 1. Find biconnected components forming connected subgraph from fixed vertices
    _findFixedComponents(bc_decom, fixed_components, bc_components);

    all_trivial = _assignComponentsRelativeCoordinates(bc_components, fixed_components, bc_decom);

    _findFirstVertexIdx(n_comp, fixed_components, bc_components, all_trivial);

    int i, j = -1;

    // ( 1] atoms assigned absolute coordinates and adjacent to atoms not;
    //   assigned coordinates are put on a list;
    QS_DEF(Array<int>, assigned_list);
    QS_DEF(Array<int>, adjacent_list);

    while (true)
    {
        if (cancellation && cancellation->isCancelled())
            throw Error("Molecule layout has been cancelled: %s", cancellation->cancelledRequestMessage());

        if (!_prepareAssignedList(assigned_list, bc_decom, bc_components, bc_tree))
            return;

        // ( 3.i] let k = 0  ( top of the list];;
        while (assigned_list.size() != 0)
        {
            int k = assigned_list.pop();
            const Vertex& vert_k = getVertex(k);

            // ( 3.ii] a list of atoms adjacent to atom Uzel and not previously;
            //		 assigned coordinates is created and ordered with cyclic atoms;
            //       at the top of the list with descending ATCD numbers and acyclic atoms;
            //       at the bottom of the list with descending ATCD numbers;;
            adjacent_list.clear();

            for (i = vert_k.neiBegin(); i < vert_k.neiEnd(); i = vert_k.neiNext(i))
                if (_layout_vertices[vert_k.neiVertex(i)].type == ELEMENT_NOT_DRAWN)
                    adjacent_list.push(vert_k.neiVertex(i));

            if (adjacent_list.size() == 0)
                break;

            // When all components outgoing from vertex are trivial (edges) then use tree algorithm
            all_trivial = true;

            for (i = 0; i < bc_decom.getIncomingCount(k); i++)
                if (!bc_components[bc_decom.getIncomingComponents(k)[i]]->isSingleEdge())
                {
                    all_trivial = false;
                    break;
                }

            if (all_trivial && bc_tree[k] != -1 && !bc_components[bc_tree[k]]->isSingleEdge())
                all_trivial = false;

            if (all_trivial)
            {
                adjacent_list.qsort(_vertex_cmp, this);

                _attachDandlingVertices(k, adjacent_list);
            }
            else
            {
                // ( 3.iii] Look over all possible orders of component layouts
                //         (vertex itself is already drawn means one component is already drawn)
                // ( 3.iv]  Choose layout with minimal energy
                _layout_component(bc_decom, bc_components, bc_tree, fixed_components, k);
            }
            // ( 3.v] let k = k + 1;;
            // ( 3.vi] repeat steps 3.ii-3.v until all atoms in the list have been processed;;
        }
        // ( 4] repeat steps 1-3 until all atoms have been assigned absolute coordinates.;
    }
}

void MoleculeLayoutGraphSimple::_layout_component(BiconnectedDecomposer& bc_decom, PtrArray<MoleculeLayoutGraph>& bc_components, Array<int>& bc_tree,
                                                  Array<int>& fixed_components, int src_vertex)
{
    // Component layout in current vertex should have the same angles between components.
    // So it depends on component order and their flipping (for nontrivial components)
    AttachmentLayoutSimple att_layout(bc_decom, bc_components, bc_tree, *this, src_vertex);

    LayoutChooser layout_chooser(att_layout, fixed_components);

    layout_chooser.perform();

    att_layout.markDrawnVertices();
}

bool MoleculeLayoutGraphSimple::_match_pattern_bond(Graph& subgraph, Graph& supergraph, int self_idx, int other_idx, void* userdata)
{
    if (userdata == 0 || ((MoleculeLayoutGraphSimple*)userdata)->_molecule == 0)
        return true;

    BaseMolecule& mol = *((MoleculeLayoutGraphSimple*)userdata)->_molecule;
    const int* mapping = ((MoleculeLayoutGraphSimple*)userdata)->_molecule_edge_mapping;

    int layout_idx = ((const MoleculeLayoutGraphSimple&)supergraph).getLayoutEdge(other_idx).ext_idx;
    const PatternBond& pattern_bond = ((const PatternLayout&)subgraph).getBond(self_idx);

    switch (pattern_bond.type)
    {
    case BOND_SINGLE:
    case BOND_DOUBLE:
    case BOND_TRIPLE:
    case BOND_AROMATIC:
        if (!mol.possibleBondOrder(mapping[layout_idx], pattern_bond.type))
            return false;
        break;
    case QUERY_BOND_SINGLE_OR_DOUBLE:
        if (!mol.possibleBondOrder(mapping[layout_idx], BOND_SINGLE) && !mol.possibleBondOrder(mapping[layout_idx], BOND_DOUBLE))
            return false;
        break;
    case QUERY_BOND_SINGLE_OR_AROMATIC:
        if (!mol.possibleBondOrder(mapping[layout_idx], BOND_SINGLE) && !mol.possibleBondOrder(mapping[layout_idx], BOND_AROMATIC))
            return false;
        break;
    case QUERY_BOND_DOUBLE_OR_AROMATIC:
        if (!mol.possibleBondOrder(mapping[layout_idx], BOND_DOUBLE) && !mol.possibleBondOrder(mapping[layout_idx], BOND_AROMATIC))
            return false;
        break;
    }

    int parity = mol.cis_trans.getParity(mapping[layout_idx]);

    if (parity != 0 && parity != pattern_bond.parity)
        return false;

    return true;
}

int MoleculeLayoutGraphSimple::_pattern_embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata)
{
    if (userdata == 0)
        return 1;

    MoleculeLayoutGraphSimple& layout_graph = *(MoleculeLayoutGraphSimple*)userdata;
    const PatternLayout& pattern_graph = (const PatternLayout&)subgraph;

    int i;

    // TODO: correct element marking (internal and non-planar)?
    for (i = layout_graph.vertexBegin(); i < layout_graph.vertexEnd(); i = layout_graph.vertexNext(i))
    {
        layout_graph._layout_vertices[i].pos = pattern_graph.getAtom(core_super[i]).pos;
        layout_graph._layout_vertices[i].type = ELEMENT_BOUNDARY;
    }

    for (i = layout_graph.edgeBegin(); i < layout_graph.edgeEnd(); i = layout_graph.edgeNext(i))
        layout_graph._layout_edges[i].type = ELEMENT_BOUNDARY;

    layout_graph._first_vertex_idx = layout_graph.vertexBegin();

    if (layout_graph._outline.get() == 0)
        layout_graph._outline.create();
    layout_graph._outline->copy(pattern_graph.getOutline());

    return 0;
}

void MoleculeLayoutGraphSimple::_assignRelativeCoordinates(int& fixed_component, const MoleculeLayoutGraph& supergraph)
{
    int i;

    if (isSingleEdge())
    {
        _assignRelativeSingleEdge(fixed_component, supergraph);
        return;
    }

    //	2.1. Use layout of fixed components and find border edges and vertices
    if (fixed_component)
    {
        for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
            _layout_vertices[i].pos = supergraph.getPos(getVertexExtIdx(i));

        CycleEnumerator ce(*this);

        ce.context = this;
        ce.cb_handle_cycle = _border_cb;

        if (ce.process())
            return;

        fixed_component = 0;
    }
    else
    {
        if (_tryToFindPattern(fixed_component))
            return;
    }

    // TODO: repair exception with vec2f

    QS_DEF(ObjPool<Cycle>, cycles);
    QS_DEF(Array<int>, sorted_cycles);

    cycles.clear();
    int n_cycles = sssrCount();

    for (i = 0; i < n_cycles; i++)
    {
        int cycle_idx = cycles.add(sssrEdges(i), *this);

        cycles[cycle_idx].canonize();
    }

    sorted_cycles.clear();
    for (i = cycles.begin(); i < cycles.end(); i = cycles.next(i))
    {
        cycles[i].calcMorganCode(*this);
        sorted_cycles.push(i);
    }
    sorted_cycles.qsort(Cycle::compare_cb, &cycles);

    _assignFirstCycle(cycles[sorted_cycles[0]]);

    cycles.remove(sorted_cycles[0]);
    sorted_cycles.remove(0);

    bool chain_attached;

    // Try to attach chains with one, two or more common edges outside drawn part
    do
    {
        chain_attached = false;

        for (i = 0; !chain_attached && i < sorted_cycles.size();)
        {
            if (_attachCycleOutside(cycles[sorted_cycles[i]], 1.f, 1))
            {
                cycles.remove(sorted_cycles[i]);
                sorted_cycles.remove(i);
                chain_attached = true;
            }
            else
                i++;
        }

        for (i = 0; !chain_attached && i < sorted_cycles.size();)
        {
            if (_attachCycleOutside(cycles[sorted_cycles[i]], 1.f, 2))
            {
                cycles.remove(sorted_cycles[i]);
                sorted_cycles.remove(i);
                chain_attached = true;
            }
            else
                i++;
        }

        for (i = 0; !chain_attached && i < sorted_cycles.size();)
        {
            if (_attachCycleOutside(cycles[sorted_cycles[i]], 1.f, 0))
            {
                cycles.remove(sorted_cycles[i]);
                sorted_cycles.remove(i);
                chain_attached = true;
            }
            else
                i++;
        }
    } while (chain_attached);

    // Try to attach chains inside
    for (i = 0; i < sorted_cycles.size();)
    {
        if (_attachCycleInside(cycles[sorted_cycles[i]], 1.f))
        {
            cycles.remove(sorted_cycles[i]);
            sorted_cycles.remove(i);
        }
        else
            i++;
    }

    // Try to attach chains inside with lower edge length
    for (i = 0; i < sorted_cycles.size();)
    {
        if (_attachCycleInside(cycles[sorted_cycles[i]], 0.75f))
        {
            cycles.remove(sorted_cycles[i]);
            sorted_cycles.remove(i);
        }
        else
            i++;
    }

    do
    {
        chain_attached = false;

        for (i = 0; !chain_attached && i < sorted_cycles.size();)
        {
            // 1.5f (> 1) means to calculate new length;
            if (_attachCycleOutside(cycles[sorted_cycles[i]], 1.5f, 0))
            {
                cycles.remove(sorted_cycles[i]);
                sorted_cycles.remove(i);
                chain_attached = true;
            }
            else
                i++;
        }
    } while (chain_attached);

    do
    {
        chain_attached = false;

        for (i = 0; !chain_attached && i < sorted_cycles.size();)
        {
            if (_attachCycleWithIntersections(cycles[sorted_cycles[i]], 1.f))
            {
                cycles.remove(sorted_cycles[i]);
                sorted_cycles.remove(i);
                chain_attached = true;
            }
            else
                i++;
        }
    } while (chain_attached);

    _attachCrossingEdges();

    for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
    {
        if (_layout_edges[i].type == ELEMENT_NOT_PLANAR)
        {
            _buildOutline();
            break;
        }
    }
}

void MoleculeLayoutGraphSimple::_assignFirstCycle(const Cycle& cycle)
{
    // TODO: Start drawing from vertex with maximum code and continue to the right with one of two which has maximum code
    int i, n;
    float phi;

    n = cycle.vertexCount();

    for (i = 0; i < n; i++)
    {
        _layout_vertices[cycle.getVertex(i)].type = ELEMENT_BOUNDARY;
        _layout_edges[cycle.getEdge(i)].type = ELEMENT_BOUNDARY;
    }

    _first_vertex_idx = cycle.getVertex(0);

    _layout_vertices[cycle.getVertex(0)].pos.set(0.f, 0.f);
    _layout_vertices[cycle.getVertex(1)].pos.set(1.f, 0.f);

    phi = (float)M_PI * (n - 2) / n;

    for (i = 1; i < n - 1; i++)
    {
        const Vec2f& v1 = _layout_vertices[cycle.getVertex(i - 1)].pos;
        const Vec2f& v2 = _layout_vertices[cycle.getVertex(i)].pos;

        _layout_vertices[cycle.getVertex(i + 1)].pos.rotateAroundSegmentEnd(v1, v2, phi);
    }
}

// If vertices are already drawn
// draw edges with intersections
void MoleculeLayoutGraph::_attachCrossingEdges()
{
    int i, j, pr;
    bool intersection;

    for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
    {
        const Edge& edge_i = getEdge(i);

        if (_layout_vertices[edge_i.beg].type != ELEMENT_NOT_DRAWN && _layout_vertices[edge_i.end].type != ELEMENT_NOT_DRAWN &&
            _layout_edges[i].type == ELEMENT_NOT_DRAWN)
        {
            intersection = true;

            while (intersection)
            {
                intersection = false;

                for (j = edgeBegin(); j < edgeEnd(); j = edgeNext(j))
                {
                    if (_layout_edges[j].type != ELEMENT_NOT_DRAWN)
                    {
                        pr = _calcIntersection(i, j);
                        // 1. If the edge 1 ends on the edge 2
                        // then shift edge 2 from edge 1 by epsilon orthogonally to the edge 2
                        if (pr == 222 || pr == 223)
                        {
                            _shiftEdge(j, 0.2f);
                            intersection = true;
                            break;
                        }
                        if (pr == 224 || pr == 225)
                        {
                            _shiftEdge(i, 0.2f);
                            intersection = true;
                            break;
                        }
                        // 2. If the edge 1 overlaps some other edge shift it by epsilon orthogonally
                        if (pr == 3 || pr == 4)
                        {
                            _shiftEdge(i, 0.2f);
                            intersection = true;
                            break;
                        }
                    }
                }
            }
            _layout_edges[i].type = ELEMENT_NOT_PLANAR;
        }
    }
}

void MoleculeLayoutGraph::_buildOutline(void)
{
    Vec2f v, inter;
    Vec2f pos_i;
    int i, j;
    int first_idx = vertexBegin();
    float min_y = getPos(first_idx).y;
    const float EPS = 0.0001f;
    const float EPS_ANGLE = 1e-6f;

    for (i = vertexNext(first_idx); i < vertexEnd(); i = vertexNext(i))
    {
        if (getPos(i).y < min_y)
        {
            min_y = getPos(i).y;
            first_idx = i;
        }
    }

    i = first_idx;

    float max_angle, cur_angle;
    float i_angle = 0;
    int next_nei = 0;

    pos_i = getPos(i);

    if (_outline.get() == 0)
        _outline.create();
    else
        _outline->clear();

    while (true)
    {
        const Vertex& vert = getVertex(i);

        if (i != first_idx)
        {
            v = pos_i;
            pos_i = getPos(i);
            v.sub(pos_i);

            i_angle = v.tiltAngle2();
        }
        else if (_outline->size() > 0)
            break;

        _outline->push(pos_i);

        max_angle = 0.f;

        for (j = vert.neiBegin(); j < vert.neiEnd(); j = vert.neiNext(j))
        {
            const Vec2f& pos_nei = getPos(vert.neiVertex(j));

            v.diff(pos_nei, pos_i);

            cur_angle = v.tiltAngle2() - i_angle;

            // If cur_angle is almost zero but negative due to numeric errors (-1e-8) then
            // on some structures the results are not stable and even inifinite loop appreas
            // Example of such structure: ClC1(C(=O)C2(Cl)C3(Cl)C14Cl)C5(Cl)C2(Cl)C3(Cl)C(Cl)(Cl)C45Cl
            if (fabs(cur_angle) < EPS_ANGLE)
                cur_angle = 0;
            if (cur_angle < 0.f)
                cur_angle += _2FLOAT(2. * M_PI);

            if (max_angle < cur_angle)
            {
                max_angle = cur_angle;
                next_nei = j;
            }
        }

        i = vert.neiVertex(next_nei);

        float dist, min_dist = 0.f;
        int int_edge = -1;
        Vec2f cur_v1 = pos_i;
        Vec2f cur_v2 = getPos(i);
        int prev_edge = -1;
        int cur_edge = vert.neiEdge(next_nei);

        while (min_dist < 10000.f)
        {
            min_dist = 10001.f;

            for (j = edgeBegin(); j < edgeEnd(); j = edgeNext(j))
            {
                const Edge& edge = getEdge(j);
                const Vec2f& cur_v3 = getPos(edge.beg);
                const Vec2f& cur_v4 = getPos(edge.end);

                if (Vec2f::intersection(cur_v1, cur_v2, cur_v3, cur_v4, v))
                    if ((dist = Vec2f::dist(cur_v1, v)) < min_dist)
                    {
                        if (dist > EPS && j != prev_edge && j != cur_edge)
                        {
                            inter = v;
                            min_dist = dist;
                            int_edge = j;
                        }
                    }
            }

            if (min_dist < 10000.f)
            {
                if (min_dist > EPSILON)
                    _outline->push(v);

                const Edge& edge = getEdge(int_edge);
                const Vec2f& cur_v3 = getPos(edge.beg);
                const Vec2f& cur_v4 = getPos(edge.end);

                Vec2f cur_v1v;
                Vec2f cur_v3v;
                Vec2f cur_v4v;

                cur_v1v.diff(cur_v1, inter);
                cur_v3v.diff(cur_v3, inter);
                cur_v4v.diff(cur_v4, inter);

                float angle1 = cur_v1v.tiltAngle2();
                float angle3 = cur_v3v.tiltAngle2() - angle1;
                float angle4 = cur_v4v.tiltAngle2() - angle1;

                if (angle3 < 0.f)
                    angle3 += _2FLOAT(2. * M_PI);
                if (angle4 < 0.f)
                    angle4 += _2FLOAT(2. * M_PI);

                cur_v1 = inter;

                if (angle3 > angle4)
                {
                    cur_v2 = cur_v3;
                    i = edge.beg;
                }
                else
                {
                    cur_v2 = cur_v4;
                    i = edge.end;
                }

                prev_edge = cur_edge;
                cur_edge = int_edge;
            }
        }
    }
}

// Return 1 - with maximum code, 2 - neighbor of the 1 with maximum code
// 3 - neighbor with maximum code from the rest or -1 if it doesn't exist.
void MoleculeLayoutGraph::_getAnchor(int& v1, int& v2, int& v3) const
{
    int i;

    if (vertexCount() == 1)
    {
        v1 = v2 = vertexBegin();
        v3 = -1;
        return;
    }

    if (vertexCount() == 2)
    {
        v1 = vertexBegin();
        v2 = vertexNext(v1);
        v3 = -1;

        if (_layout_vertices[v1].morgan_code < _layout_vertices[v2].morgan_code)
        {
            v2 = vertexBegin();
            v1 = vertexNext(v2);
        }
        return;
    }

    v1 = vertexBegin();
    for (i = vertexNext(v1); i < vertexEnd(); i = vertexNext(i))
        if (_layout_vertices[i].morgan_code > _layout_vertices[v1].morgan_code)
            v1 = i;

    const Vertex& vert = getVertex(v1);

    v2 = vert.neiBegin();
    for (i = vert.neiNext(v2); i < vert.neiEnd(); i = vert.neiNext(i))
        if (_layout_vertices[vert.neiVertex(i)].morgan_code > _layout_vertices[vert.neiVertex(v2)].morgan_code)
            v2 = i;

    if (vert.degree() < 2)
    {
        v2 = vert.neiVertex(v2);
        v3 = -1;
        return;
    }

    v3 = vert.neiBegin();

    if (v3 == v2)
        v3 = vert.neiNext(v2);

    for (i = vert.neiBegin(); i < vert.neiEnd(); i = vert.neiNext(i))
        if (i != v2 && _layout_vertices[vert.neiVertex(i)].morgan_code > _layout_vertices[vert.neiVertex(v3)].morgan_code)
            v3 = i;

    v2 = vert.neiVertex(v2);
    v3 = vert.neiVertex(v3);
}

// Scale and transform
void MoleculeLayoutGraph::_assignFinalCoordinates(float bond_length, const Array<Vec2f>& src_layout)
{
    int i;

    if (_n_fixed > 0)
    {
        for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
            _layout_vertices[i].pos.scale(bond_length);
        return;
    }

    if (vertexCount() == 1)
    {
        getPos(vertexBegin()).set(0.f, 0.f);
        return;
    }

    // Flip according to various rules
    if (_molecule != 0 && _n_fixed == 0)
    {
        if (_molecule->countRSites() > 1)
        {
            // flip molecule vertically if R1 is not above other R-groups
            // flip molecule horizontally if R1 is not on the left
            QS_DEF(Array<int>, rgroup_list);
            Vec2f r1_pos, highest_pos(0.f, -1000.f);
            bool r1_exist = false;
            float center_x = 0.f;

            for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
            {
                if (_molecule->isRSite(_layout_vertices[i].ext_idx))
                {
                    _molecule->getAllowedRGroups(_layout_vertices[i].ext_idx, rgroup_list);
                    if (rgroup_list.size() == 1 && rgroup_list[0] == 1)
                    {
                        r1_pos = _layout_vertices[i].pos;
                        r1_exist = true;
                    }
                    else if (_layout_vertices[i].pos.y > highest_pos.y)
                    {
                        highest_pos = _layout_vertices[i].pos;
                    }
                }
                center_x += _layout_vertices[i].pos.x;
            }

            center_x /= vertexCount();

            if (r1_exist)
            {
                if (r1_pos.y < highest_pos.y)
                    for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
                        _layout_vertices[i].pos.y *= -1;
                if (r1_pos.x > center_x)
                    for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
                        _layout_vertices[i].pos.x *= -1;
            }
        }
        else
        {
            // flip molecule horizontally if the first atom is righter than the last one
            int first = vertexBegin();
            int last = first;
            for (i = first; i != vertexEnd(); i = vertexNext(i))
                last = i;

            const float EPS = 0.0001f;
            float diff = _layout_vertices[first].pos.x - _layout_vertices[last].pos.x;

            if (diff > EPS)
                for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
                    _layout_vertices[i].pos.x *= -1;
        }
    }

    // 1. Choose scale ratio and first edge to match
    float scale = bond_length, src_norm, norm;
    int v1, v2, v3;
    Vec2f p1, p2, p;

    _getAnchor(v1, v2, v3);

    p1.diff(src_layout[v2], src_layout[v1]);
    p2.diff(getPos(v2), getPos(v1));

    src_norm = p1.length();
    norm = p2.length();

    if (norm < 0.0001)
        throw Error("too small edge");

    // 2.1. If matching edge has zero length - just move to this point and scale
    if (src_norm < 0.001)
    {
        p1 = src_layout[v1];
        p2 = getPos(v1);

        for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
        {
            p.diff(getPos(i), p2);
            p.scale(scale);
            _layout_vertices[i].pos.sum(p1, p);
        }
        return;
    }

    // 2.2. If it has length from L/2 to 2L - scale by it, otherwise by L
    if (src_norm >= bond_length / 2 && src_norm <= 2 * bond_length)
        scale = src_norm / norm;

    // 3. Move first vertex to (0,0)
    p = getPos(v1);
    for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
        _layout_vertices[i].pos.sub(p);

    // 4. Rotate CCW on Alpha angle between (first, second) edge and (first, second) edge in source graph
    float phi1, phi2, alpha, sina, cosa;

    phi1 = p1.tiltAngle();
    phi2 = p2.tiltAngle();
    alpha = phi1 - phi2;
    sina = sin(alpha);
    cosa = cos(alpha);

    for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
        _layout_vertices[i].pos.rotate(sina, cosa);

    // 5. Scale
    for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
        _layout_vertices[i].pos.scale(scale);

    // 6. Match first vertices - shift by vector Pos(first)
    for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
        _layout_vertices[i].pos.add(src_layout[v1]);

    // 7. If needed turn around (first, second)
    if (vertexCount() > 2)
    {
        float crit = 0.f;
        // If v3 lays on the other side of line (first, second) - turn
        p1 = getPos(v1);
        p.diff(getPos(v2), p1);

        if (v3 == -1)
        {
            for (v3 = vertexBegin(); v3 < vertexEnd(); v3 = vertexNext(v3))
            {
                if (fabs(p.x) < 0.001f)
                    crit = (src_layout[v3].x - p1.x) * (getPos(v3).x - p1.x);
                else if (fabs(p.y) < 0.001f)
                    crit = (src_layout[v3].y - p1.y) * (getPos(v3).y - p1.y);
                else
                {
                    crit = (p.y * (src_layout[v3].x - p1.x) - p.x * (src_layout[v3].y - p1.y)) * (p.y * (getPos(v3).x - p1.x) - p.x * (getPos(v3).y - p1.y));
                }

                if (fabs(crit) > 0.001)
                    break;
            }
        }
        else
            crit = -1.0;

        if (crit < 0 && v3 < vertexEnd())
        {
            // Move first vertex to (0,0)
            for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
                _layout_vertices[i].pos.sub(p1);

            // Turn by -phi1 and flip vertically
            sina = -sin(phi1);
            cosa = cos(phi1);
            for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
            {
                _layout_vertices[i].pos.rotate(sina, cosa);
                _layout_vertices[i].pos.y *= -1;
            }

            // Turn by phi1 and translate back
            sina = -sina;
            for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
            {
                _layout_vertices[i].pos.rotate(sina, cosa);
                _layout_vertices[i].pos.add(p1);
            }
        }
    }
}

void MoleculeLayoutGraph::_findFixedComponents(BiconnectedDecomposer& bc_decom, Array<int>& fixed_components, PtrArray<MoleculeLayoutGraph>& bc_components)
{
    // 1. Find biconnected components forming connected subgraph from fixed vertices
    if (_n_fixed == 0)
        return;

    int n_comp = bc_decom.componentsCount();
    QS_DEF(Array<int>, fixed_count);

    fixed_count.clear_resize(n_comp);
    fixed_count.zerofill();

    // calculate number of fixed vertices in each component
    for (int i = 0; i < n_comp; i++)
    {
        Filter filter;

        bc_decom.getComponent(i, filter);

        for (int j = vertexBegin(); j < vertexEnd(); j = vertexNext(j))
            if (filter.valid(j) && _fixed_vertices[j])
                fixed_count[i]++;
    }

    // keep only with fixed number greater than a half
    for (int i = 0; i < n_comp; i++)
    {
        Filter filter;

        bc_decom.getComponent(i, filter);

        // if (fixed_count[i] > filter.count(*this) / 2)
        if (fixed_count[i] == filter.count(*this))
            fixed_components[i] = 1;
    }

    _fixed_vertices.zerofill();

    // update fixed vertices
    for (int i = 0; i < n_comp; i++)
    {
        if (!fixed_components[i])
            continue;

        MoleculeLayoutGraph& component = *bc_components[i];

        for (int j = component.vertexBegin(); j < component.vertexEnd(); j = component.vertexNext(j))
            _fixed_vertices[component.getVertexExtIdx(j)] = 1;
    }

    Filter fixed_filter(_fixed_vertices.ptr(), Filter::EQ, 1);

    Graph fixed_graph;
    QS_DEF(Array<int>, fixed_mapping);
    QS_DEF(Array<int>, fixed_inv_mapping);

    fixed_graph.makeSubgraph(*this, fixed_filter, &fixed_mapping, &fixed_inv_mapping);

    if (Graph::isConnected(fixed_graph))
        _n_fixed = fixed_filter.count(*this);
    else
    {
        // fixed subgraph is not connected - choose its greatest component
        int n = fixed_graph.countComponents();
        const Array<int>& decomposition = fixed_graph.getDecomposition();

        fixed_count.clear_resize(n);
        fixed_count.zerofill();

        for (int i = fixed_graph.vertexBegin(); i < fixed_graph.vertexEnd(); i = fixed_graph.vertexNext(i))
            fixed_count[decomposition[i]]++;

        int j = 0;
        for (int i = 1; i < n; i++)
            if (fixed_count[i] > fixed_count[j])
                j = i;

        Filter max_filter(decomposition.ptr(), Filter::EQ, j);

        // update fixed vertices
        _fixed_vertices.zerofill();
        _n_fixed = 0;

        for (int i = fixed_graph.vertexBegin(); i < fixed_graph.vertexEnd(); i = fixed_graph.vertexNext(i))
        {
            if (max_filter.valid(i))
            {
                _fixed_vertices[fixed_mapping[i]] = 1;
                _n_fixed++;
            }
        }

        for (int i = 0; i < n_comp; i++)
        {
            if (!fixed_components[i])
                continue;

            MoleculeLayoutGraph& component = *bc_components[i];

            int comp_v = component.getVertexExtIdx(component.vertexBegin());
            int mapped = fixed_inv_mapping[comp_v];
            if (!max_filter.valid(mapped))
                fixed_components[i] = 0;
        }
    }
}

bool MoleculeLayoutGraph::_assignComponentsRelativeCoordinates(PtrArray<MoleculeLayoutGraph>& bc_components, Array<int>& fixed_components,
                                                               BiconnectedDecomposer& bc_decom)
{
    bool all_trivial = true;
    int n_comp = bc_decom.componentsCount();

    // Possible solutions:
    // 1. a) vertex code is calculated inside component (doesn't depend on neighbors) or
    //    b) vertex code is calculated respecting whole graph
    // 2. a) component code is the sum of 1a codes
    //    b) component code is the sum of 1b codes
    // Initially was 1a and 2b then changed to 1b and 2b
    for (int i = 0; i < n_comp; i++)
    {
        MoleculeLayoutGraph& component = *bc_components[i];
        component.max_iterations = max_iterations;
        component.layout_orientation = layout_orientation;

        // component._calcMorganCodes();
        component._total_morgan_code = 0;

        for (int j = component.vertexBegin(); j < component.vertexEnd(); j = component.vertexNext(j))
            component._total_morgan_code += _layout_vertices[component.getLayoutVertex(j).ext_idx].morgan_code;

        // Mark cyclic atoms
        if (!component.isSingleEdge())
        {
            all_trivial = false;

            for (int j = component.vertexBegin(); j < component.vertexEnd(); j = component.vertexNext(j))
            {
                component._layout_vertices[j].is_cyclic = true;
                _layout_vertices[component._layout_vertices[j].ext_idx].is_cyclic = true;
            }

            for (int j = component.edgeBegin(); j < component.edgeEnd(); j = component.edgeNext(j))
            {
                component._layout_edges[j].is_cyclic = true;
                _layout_edges[component._layout_edges[j].ext_idx].is_cyclic = true;
            }
        }

        int fixed = fixed_components[i];

        component._assignRelativeCoordinates(fixed, *this);

        if (fixed != fixed_components[i])
        {
            fixed_components[i] = fixed;

            // update fixed vertices
            _fixed_vertices.resize(vertexEnd());
            _fixed_vertices.zerofill();
            _n_fixed = 0;
            for (int j = 0; j < n_comp; j++)
            {
                if (!fixed_components[j])
                    continue;

                Filter fix_filter;

                bc_decom.getComponent(j, fix_filter);

                for (int k = vertexBegin(); k < vertexEnd(); k = vertexNext(k))
                    if (!_fixed_vertices[k])
                    {
                        _fixed_vertices[k] = 1;
                        _n_fixed++;
                    }
            }
        }
    }
    return all_trivial;
}

void MoleculeLayoutGraph::_assignRelativeSingleEdge(int& fixed_component, const MoleculeLayoutGraph& supergraph)
{
    // Trivial component layout
    int idx1 = vertexBegin();
    int idx2 = vertexNext(idx1);

    _layout_vertices[idx1].type = ELEMENT_BOUNDARY;
    _layout_vertices[idx2].type = ELEMENT_BOUNDARY;

    if (fixed_component)
    {
        _layout_vertices[idx1].pos = supergraph.getPos(getVertexExtIdx(idx1));
        _layout_vertices[idx2].pos = supergraph.getPos(getVertexExtIdx(idx2));
    }
    else
    {
        _layout_vertices[idx1].pos.set(0.f, 0.f);
        _layout_vertices[idx2].pos.set(0.f, 1.f);
    }

    _layout_edges[edgeBegin()].type = ELEMENT_BOUNDARY;
}

bool MoleculeLayoutGraphSimple::_tryToFindPattern(int& fixed_component)
{
    auto& _patterns = getPatterns();

    MorganCode morgan(*this);
    QS_DEF(Array<long>, morgan_codes);

    morgan.calculate(morgan_codes, 3, 7);

    long morgan_code = 0;

    for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
        morgan_code += morgan_codes[i];

    int left = 0;
    int right = _patterns.size() - 1;
    int pat_idx = 0;
    int cmp;

    while (left < right)
    {
        if (right - left == 1)
        {
            if (_pattern_cmp2(_patterns[left], vertexCount(), edgeCount(), morgan_code) == 0)
                pat_idx = left;
            else if (_pattern_cmp2(_patterns[right], vertexCount(), edgeCount(), morgan_code) == 0)
                pat_idx = right;
            break;
        }

        pat_idx = (right + left) / 2;

        cmp = _pattern_cmp2(_patterns[pat_idx], vertexCount(), edgeCount(), morgan_code);

        if (cmp < 0)
            left = pat_idx;
        else
            right = pat_idx;
    }

    while (pat_idx > 0 && _pattern_cmp2(_patterns[pat_idx - 1], vertexCount(), edgeCount(), morgan_code) == 0)
        pat_idx--;

    while (pat_idx < _patterns.size() && _pattern_cmp2(_patterns[pat_idx], vertexCount(), edgeCount(), morgan_code) == 0)
    {
        // Match pattern
        // TODO: check different attachment points
        PatternLayout& pattern = _patterns[pat_idx];

        EmbeddingEnumerator ee(*this);

        ee.setSubgraph(pattern);
        ee.cb_match_edge = _match_pattern_bond;
        ee.cb_embedding = _pattern_embedding;
        ee.userdata = this;

        if (!ee.process())
        {
            if (pattern.isFixed())
                fixed_component = 1;
            return true;
        }

        pat_idx++;
    }

    return false;
}

void MoleculeLayoutGraph::_findFirstVertexIdx(int n_comp, Array<int>& fixed_components, PtrArray<MoleculeLayoutGraph>& bc_components, bool all_trivial)
{
    if (_n_fixed > 0)
    {
        int j = -1;
        for (int i = 0; i < n_comp; i++)
            if (fixed_components[i])
            {
                _copyLayout(*bc_components[i]);
                j = i;
            }

        if (j == -1)
            throw Error("Internal error: cannot find a fixed component with fixed vertices");

        MoleculeLayoutGraph& component = *bc_components[j];

        _first_vertex_idx = component._layout_vertices[component.vertexBegin()].ext_idx;
    }
    else
    {
        // ( 0]. Nucleus.;
        //   Begin from nontrivial component with maximum code
        //   if there's no then begin from vertex with maximum code and its neighbor with maximum code too
        int nucleus_idx = 0;

        if (!all_trivial)
        {
            nucleus_idx = -1;
            for (int i = 0; i < n_comp; i++)
            {
                MoleculeLayoutGraph& component = *bc_components[i];

                if (!component.isSingleEdge())
                {
                    if (nucleus_idx == -1 || component._total_morgan_code > bc_components[nucleus_idx]->_total_morgan_code)
                        nucleus_idx = i;
                }
            }

            if (nucleus_idx < 0)
                throw Error("Internal error: cannot find nontrivial component");

            MoleculeLayoutGraph& nucleus = *bc_components[nucleus_idx];

            _copyLayout(nucleus);
            _first_vertex_idx = nucleus._layout_vertices[nucleus._first_vertex_idx].ext_idx;
        }
        else
        {
            for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
                if (_layout_vertices[i].morgan_code > _layout_vertices[nucleus_idx].morgan_code)
                    nucleus_idx = i;

            const Vertex& nucleus = getVertex(nucleus_idx);
            int nucleus_idx2 = nucleus.neiBegin();

            for (int j = nucleus.neiNext(nucleus_idx2); j < nucleus.neiEnd(); j = nucleus.neiNext(j))
                if (_layout_vertices[nucleus.neiVertex(j)].morgan_code > _layout_vertices[nucleus.neiVertex(nucleus_idx2)].morgan_code)
                    nucleus_idx2 = j;

            int nucleus_edge = nucleus.neiEdge(nucleus_idx2);
            nucleus_idx2 = nucleus.neiVertex(nucleus_idx2);

            _first_vertex_idx = nucleus_idx;
            _layout_vertices[nucleus_idx].type = ELEMENT_BOUNDARY;
            _layout_vertices[nucleus_idx].pos.set(0.f, 0.f);
            _layout_vertices[nucleus_idx2].type = ELEMENT_BOUNDARY;
            _layout_vertices[nucleus_idx2].pos.set(1.f, 0.f);
            _layout_edges[nucleus_edge].type = ELEMENT_BOUNDARY;
        }
    }
}

bool MoleculeLayoutGraph::_prepareAssignedList(Array<int>& assigned_list, BiconnectedDecomposer& bc_decom, PtrArray<MoleculeLayoutGraph>& bc_components,
                                               Array<int>& bc_tree)
{
    assigned_list.clear();

    for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
    {
        if (_layout_vertices[i].type == ELEMENT_NOT_DRAWN)
            continue;

        const Vertex& vert = getVertex(i);

        for (int j = vert.neiBegin(); j < vert.neiEnd(); j = vert.neiNext(j))
        {
            if (_layout_vertices[vert.neiVertex(j)].type == ELEMENT_NOT_DRAWN)
            {
                assigned_list.push(i);
                break;
            }
        }
    }

    if (assigned_list.size() == 0)
    {
        // restore ignored ears in chains
        for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
            if (_layout_vertices[i].type == ELEMENT_IGNORE)
                _layout_vertices[i].type = ELEMENT_BOUNDARY;

        _refineCoordinates(bc_decom, bc_components, bc_tree);
        return false;
    }

    // ( 2] the list is ordered with cyclic atoms at the top of the list;
    //   with descending ATCD numbers and acyclic atoms at the bottom;
    //   of the list with descending ATCD numbers;;
    assigned_list.qsort(_vertex_cmp, this);
    return true;
}
