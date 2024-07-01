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

// #include "api/src/indigo_internal.h"

#include "base_cpp/profiling.h"
#include "graph/biconnected_decomposer.h"
#include "graph/cycle_enumerator.h"
#include "graph/embedding_enumerator.h"
#include "graph/morgan_code.h"
#include "layout/attachment_layout.h"
#include "layout/layout_pattern_smart.h"
#include "layout/molecule_layout_graph.h"
#include "layout/molecule_layout_macrocycles.h"

#include <algorithm>
#include <math/random.h>
#include <memory>
#include <vector>

using namespace indigo;

enum
{
    QUERY_BOND_SINGLE_OR_DOUBLE = 5,
    QUERY_BOND_SINGLE_OR_AROMATIC = 6,
    QUERY_BOND_DOUBLE_OR_AROMATIC = 7,
    QUERY_BOND_ANY = 8
};

// Make relative coordinates of a component absolute

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

void MoleculeLayoutGraphSmart::_layout_component(BiconnectedDecomposer& bc_decom, PtrArray<MoleculeLayoutGraph>& bc_components, Array<int>& bc_tree,
                                                 Array<int>& fixed_components, int src_vertex)
{
    // Component layout in current vertex should have the same angles between components.
    // So it depends on component order and their flipping (for nontrivial components)
    AttachmentLayoutSmart att_layout(bc_decom, bc_components, bc_tree, *this, src_vertex);

    LayoutChooser layout_chooser(att_layout, fixed_components);

    layout_chooser.perform();

    att_layout.markDrawnVertices();
}

void MoleculeLayoutGraphSmart::_get_toches_to_component(Cycle& cycle, int component_number, Array<interval>& interval_list)
{
    if (component_number < 0 || component_number >= _layout_component_count)
        return;
    QS_DEF(Array<bool>, touch_to_current_component);
    touch_to_current_component.clear_resize(cycle.vertexCount());
    touch_to_current_component.zerofill();
    for (int i = 0; i < cycle.vertexCount(); i++)
    {
        const Vertex& vert = getVertex(cycle.getVertex(i));
        for (int n = vert.neiBegin(); n != vert.neiEnd(); n = vert.neiNext(n))
        {
            if (getEdgeType(vert.neiEdge(n)) != ELEMENT_NOT_DRAWN && _layout_component_number[vert.neiEdge(n)] == component_number)
                touch_to_current_component[i] = true;
        }
    }

    int first_start = -1;
    for (int i = 0; i < cycle.vertexCount(); i++)
        if (touch_to_current_component[i] && _layout_component_number[cycle.getEdgeC(i)] != component_number)
        {
            first_start = i;
            break;
        }

    interval_list.clear();
    if (first_start == -1)
        return;

    int start = first_start;
    int finish = 0;

    while (true)
    {
        finish = (start + 1) % cycle.vertexCount();
        while (!touch_to_current_component[finish])
            finish = (finish + 1) % cycle.vertexCount();

        interval_list.push();
        interval_list.top().init(start, finish);

        start = finish;
        while (_layout_component_number[cycle.getEdge(start)] == component_number)
            start = (start + 1) % cycle.vertexCount();

        if (start == first_start)
            break;
    }
}

int MoleculeLayoutGraphSmart::_search_separated_component(Cycle& cycle, Array<interval>& interval_list)
{
    for (int i = 0; i < _layout_component_count; i++)
    {
        _get_toches_to_component(cycle, i, interval_list);
        if (interval_list.size() > 1)
            return i;
    }
    return -1;
}

void MoleculeLayoutGraphSmart::_search_path(int start, int finish, Array<int>& path, int component_number)
{
    QS_DEF(Array<bool>, visited);
    visited.clear_resize(vertexEnd());
    visited.zerofill();
    visited[start] = true;

    QS_DEF(Array<int>, vertices_list);
    QS_DEF(Array<int>, previous_list);
    vertices_list.clear();
    vertices_list.push(start);
    previous_list.clear_resize(vertexEnd());

    for (int i = 0; i < vertices_list.size(); i++)
    {
        if (vertices_list[i] == finish)
        {
            while (finish != start)
            {
                path.push(finish);
                finish = previous_list[finish];
            }
            path.push(finish);
            for (int j = 0; j < path.size() / 2; j++)
                swap(path[j], path[path.size() - 1 - j]);
            return;
        }
        const Vertex& vert = getVertex(vertices_list[i]);
        for (int n = vert.neiBegin(); n != vert.neiEnd(); n = vert.neiNext(n))
        {
            int e = vert.neiEdge(n);
            int v = vert.neiVertex(n);
            if (_layout_component_number[e] == component_number && !visited[v])
            {
                visited[v] = true;
                vertices_list.push(v);
                previous_list[v] = vertices_list[i];
            }
        }
    }
}

void MoleculeLayoutGraphSmart::_assignRelativeCoordinates(int& fixed_component, const MoleculeLayoutGraph& supergraph)
{
    profTimerStart(t, "_assignRelativeCoordinates");
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
        if (PatternLayoutFinder::tryToFindPattern(*this))
            return;
    }

    // TODO: repair exception with vec2f

    QS_DEF(ObjPool<Cycle>, cycles);

    cycles.clear();
    int n_cycles = sssrCount();

    for (i = 0; i < n_cycles; i++)
    {
        int cycle_idx = cycles.add(sssrEdges(i), *this);

        cycles[cycle_idx].canonize();
    }

    while (cycles.size() != 0)
    {

        QS_DEF(Array<int>, unused_count);
        unused_count.clear_resize(cycles.end());
        unused_count.zerofill();
        for (i = cycles.begin(); i != cycles.end(); i = cycles.next(i))
        {
            for (int j = 0; j < cycles[i].vertexCount(); j++)
            {
                if (_layout_component_number[cycles[i].getEdge(j)] == -1)
                    unused_count[i]++;
            }
        }
        for (i = cycles.begin(); i != cycles.end(); i = cycles.next(i))
            unused_count[i] *= cycles[i].vertexCount();
        for (i = cycles.begin(); i != cycles.end(); i = cycles.next(i))
            cycles[i].calcMorganCode(supergraph);

        int min_i = cycles.begin();
        for (i = cycles.begin(); i != cycles.end(); i = cycles.next(i))
        {
            if (unused_count[i] < unused_count[min_i] || (unused_count[i] == unused_count[min_i] && cycles[i].morganCode() > cycles[min_i].morganCode()))
                min_i = i;
        }

        if (unused_count[min_i] > 0)
        {

            QS_DEF(Array<interval>, interval_list);
            int separating_component = _search_separated_component(cycles[min_i], interval_list);
            if (separating_component >= 0)
            {
                for (i = 0; i < interval_list.size(); i++)
                {
                    int start = interval_list[i].left;
                    int finish = interval_list[i].right;
                    QS_DEF(Array<int>, verts);
                    QS_DEF(Array<int>, edges);
                    verts.clear();
                    edges.clear();
                    _search_path(cycles[min_i].getVertex(finish), cycles[min_i].getVertex(start), verts, separating_component);
                    for (int j = (start + 1) % cycles[min_i].vertexCount(); j != finish; j = (j + 1) % cycles[min_i].vertexCount())
                        verts.push(cycles[min_i].getVertex(j));
                    for (int j = 0; j < verts.size(); j++)
                    {
                        int e = findEdgeIndex(verts[j], verts[(j + 1) % verts.size()]);
                        edges.push(e);
                    }
                    cycles.add(verts, edges);
                }
            }
            else
            {
                _assignEveryCycle(cycles[min_i]);
            }
        }
        cycles.remove(min_i);
    }
}

void MoleculeLayoutGraphSmart::_assignEveryCycle(const Cycle& cycle)
{
    profTimerStart(t, "_assignFirstCycle");
    const int size = cycle.vertexCount();
    _first_vertex_idx = cycle.getVertex(0);

    MoleculeLayoutMacrocyclesLattice layout(size);

    if (size <= 6)
        for (int i = 0; i < size; i++)
            _molecule->cis_trans.setParity(_layout_edges[cycle.getEdge(i)].orig_idx, 0);

    for (int i = 0; i < size; i++)
    {

        // edge parallels

        // !!
        int order_next = 0;
        int edge_number = cycle.getEdge(i);
        LayoutEdge edge = _layout_edges[edge_number];
        int ext_edge_number = edge.orig_idx;
        int order = _molecule->getBondOrder(ext_edge_number);
        switch (order)
        {
        case BOND_SINGLE:
            order_next = 1;
            break;
        case BOND_DOUBLE:
            order_next = 2;
            break;
        case BOND_TRIPLE:
            order_next = 3;
            break;
        default:
            order_next = 1;
        }
        int order_prev;
        int ext_edge_number_prev = _layout_edges[cycle.getEdgeC(i - 1)].orig_idx;
        switch (_molecule->getBondOrder(ext_edge_number_prev))
        {
        case BOND_SINGLE:
            order_prev = 1;
            break;
        case BOND_DOUBLE:
            order_prev = 2;
            break;
        case BOND_TRIPLE:
            order_prev = 3;
            break;
        default:
            order_prev = 1;
        }

        layout.setVertexEdgeParallel(i, order_next + order_prev >= 4);

        // tras-cis configuration
        int next_vertex = _layout_vertices[cycle.getEdgeFinish(i + 1)].orig_idx;
        int prev_vertex = _layout_vertices[cycle.getEdgeStart(i - 1)].orig_idx;

        if (_molecule->cis_trans.getParity(ext_edge_number))
        {
            int _sameside = _molecule->cis_trans.sameside(ext_edge_number, prev_vertex, next_vertex);
            if (_sameside)
                layout.setEdgeStereo(i, MoleculeCisTrans::CIS);
            else
                layout.setEdgeStereo(i, MoleculeCisTrans::TRANS);
        }
        else
        {
            //         if (_layout_vertices[cycle.getVertex(i)].type != ELEMENT_NOT_DRAWN &&
            //          _layout_vertices[cycle.getVertex((i + 1) % size)].type != ELEMENT_NOT_DRAWN) {
            if (_layout_edges[cycle.getEdge(i)].type != ELEMENT_NOT_DRAWN)
            {

                Vec2f prev_point;
                if (_layout_edges[cycle.getEdgeC(i - 1)].type != ELEMENT_NOT_DRAWN)
                    prev_point = _layout_vertices[cycle.getVertexC(i - 1)].pos;
                else
                {
                    for (int j = getVertex(cycle.getVertex(i)).neiBegin(); j != getVertex(cycle.getVertex(i)).neiEnd();
                         j = getVertex(cycle.getVertex(i)).neiNext(j))
                        if (_layout_edges[getVertex(cycle.getVertex(i)).neiEdge(j)].type != ELEMENT_NOT_DRAWN &&
                            getVertex(cycle.getVertex(i)).neiVertex(j) != cycle.getVertexC(i + 1))
                            prev_point = _layout_vertices[getVertex(cycle.getVertex(i)).neiVertex(j)].pos;
                }

                Vec2f next_point;
                if (_layout_edges[cycle.getEdgeC(i + 1)].type != ELEMENT_NOT_DRAWN)
                    next_point = _layout_vertices[cycle.getVertexC(i + 2)].pos;
                else
                {
                    for (int j = getVertex(cycle.getVertexC(i + 1)).neiBegin(); j != getVertex(cycle.getVertexC(i + 1)).neiEnd();
                         j = getVertex(cycle.getVertexC(i + 1)).neiNext(j))
                        if (_layout_edges[getVertex(cycle.getVertexC(i + 1)).neiEdge(j)].type != ELEMENT_NOT_DRAWN &&
                            getVertex(cycle.getVertexC(i + 1)).neiVertex(j) != cycle.getVertex(i))
                            next_point = _layout_vertices[getVertex(cycle.getVertexC(i + 1)).neiVertex(j)].pos;
                }

                int _sameside =
                    _isCisConfiguratuin(prev_point, _layout_vertices[cycle.getVertexC(i)].pos, _layout_vertices[cycle.getVertexC(i + 1)].pos, next_point);

                if (_layout_edges[cycle.getEdgeC(i - 1)].type != ELEMENT_NOT_DRAWN && _layout_edges[cycle.getEdgeC(i + 1)].type != ELEMENT_NOT_DRAWN)
                {
                    if (_sameside)
                        layout.setEdgeStereo(i, MoleculeCisTrans::CIS);
                    else
                        layout.setEdgeStereo(i, MoleculeCisTrans::TRANS);
                }
                else
                {
                    if ((_layout_edges[cycle.getEdgeC(i - 1)].type != ELEMENT_NOT_DRAWN) ^ (_layout_edges[cycle.getEdgeC(i + 1)].type != ELEMENT_NOT_DRAWN))
                    {
                        if (_sameside)
                            layout.setEdgeStereo(i, MoleculeCisTrans::TRANS);
                        else
                            layout.setEdgeStereo(i, MoleculeCisTrans::CIS);
                    }
                    else
                        layout.setEdgeStereo(i, MoleculeCisTrans::CIS);
                }
            }
        }

        layout.setVertexDrawn(i, _layout_vertices[cycle.getVertex(i)].type != ELEMENT_NOT_DRAWN);

        // trees sizes
    }

    /*
     * A patrial fix for Ketcher issues 486 and 487
     *
     * The _segment_smoothing_prepearing() contains an error which leads to
     * a crash when cycles with double bonds are being processed. Currently,
     * we skip smoothing altogether. This seems not to create any regressions
     */
    QS_DEF(ObjArray<MoleculeLayoutSmoothingSegment>, segment);
    QS_DEF(Array<Vec2f>, rotation_point);
    QS_DEF(Array<int>, rotation_vertex);

    segment.clear();
    rotation_point.zerofill();
    rotation_vertex.zerofill();

    //   _segment_smoothing_prepearing(cycle, rotation_vertex, rotation_point, segment, layout);

    int segment_count = segment.size();

    for (int i = 0; i < segment_count; i++)
    {
        for (int v = segment[i]._graph.vertexBegin(); v != segment[i]._graph.vertexEnd(); v = segment[i]._graph.vertexNext(v))
        {
            if (segment[i].is_start(v))
                if (segment[i]._graph.getVertex(v).degree() > 2)
                    layout.setEdgeStereo(rotation_vertex[i], 0);
            if (segment[i].is_finish(v))
                if (segment[i]._graph.getVertex(v).degree() > 2)
                    layout.setEdgeStereo((rotation_vertex[(i + 1) % segment_count] - 1 + size) % size, 0);
        }
    }
    /*bool easy_case = size <= 9;
    if (easy_case) {
    QS_DEF(Array<int>, last);
    last.clear_resize(_layout_component_count);
    last.fill(-1);
    for (int i = 0; i < size; i++) {
    int comp = _layout_component_number[cycle.getEdge(i)];
    if (comp >= 0) {
    if (last[comp] >= 0) easy_case = false;
    last[comp] = i;
    }
    }

    QS_DEF(Array<int>, order);
    order.clear_resize(size);
    for (int i = 0; i < size; i++) {
    order[i] = _molecule->getBondOrder(getEdgeOrigIdx(cycle.getEdge(i)));
    if (order[i] > 3) order[i] = 1;
    }
    order.push(order[0]);
    for (int i = 0; i < size; i++) easy_case &= (order[i] + order[i + 1] < 4);

    for (int i = 0; i < size; i++) {
    int next_vertex = _layout_vertices[cycle.getEdgeFinish(i + 1)].orig_idx;
    int prev_vertex = _layout_vertices[cycle.getEdgeStart(i - 1)].orig_idx;

    if (_molecule->cis_trans.getParity(getEdgeOrigIdx(cycle.getEdge(i)))) {
    easy_case &= _molecule->cis_trans.sameside(getEdgeOrigIdx(cycle.getEdge(i)), prev_vertex, next_vertex);
    }
    }
    if (easy_case) {
    for (int i = 0; i < size; i++) {
    layout.getPos(cycle.getVertex(i)) = Vec2f(1, 0);
    layout.getPos(cycle.getVertex(i)).rotate(2 * M_PI / size * i);
    }

    for (int i = 0; i < size; i++)
    if (getVertexType(cycle.getVertex(i)) == ELEMENT_NOT_DRAWN)
    getPos(cycle.getVertex(i)) = layout.getPos(i);

    for (int i = 0; i < size; i++)
    {
    setVertexType(cycle.getVertex(i), ELEMENT_DRAWN);
    setEdgeType(cycle.getEdge(i), ELEMENT_DRAWN);
    }


    }
    */
    // printf("%d do layout cycle \n", size);

    // calculate target angle

    for (int s = 0; s < segment_count; s++)
    {
        for (int i = rotation_vertex[s]; i != rotation_vertex[(s + 1) % segment_count]; i = (i + 1) % size)
        {
            int prev_layout_component = _layout_component_number[cycle.getEdgeC(i - 1)];
            int next_layout_component = _layout_component_number[cycle.getEdge(i)];

            if (prev_layout_component < 0 && next_layout_component < 0)
            {
                layout.setTargetAngle(i, _2FLOAT(2. * M_PI / 3.));
                layout.setAngleImportance(i, 0.2f);
            }
            else if ((prev_layout_component < 0) ^ (next_layout_component < 0))
            {
                const MoleculeLayoutSmoothingSegment& calc_segment = prev_layout_component < 0 ? segment[s] : segment[(s + segment_count - 1) % segment_count];
                int calc_vertex = prev_layout_component < 0 ? calc_segment.get_start() : calc_segment.get_finish();

                Cycle border;
                calc_segment._graph._getBorder(border);
                int calc_vertex_in_border = -1;
                for (int j = 0; j < border.vertexCount(); j++)
                {
                    if (border.getVertex(j) == calc_vertex)
                    {
                        calc_vertex_in_border = j;
                        break;
                    }
                }

                float angle = 0;
                int prev_vertex = -1;
                int next_vertex = -1;
                if (border.vertexCount() != 0 && calc_vertex_in_border >= 0)
                {
                    prev_vertex = border.getVertexC(calc_vertex_in_border - 1);
                    next_vertex = border.getVertexC(calc_vertex_in_border + 1);
                }
                else
                {
                    for (int n : calc_segment._graph.getVertex(calc_vertex).neighbors())
                    {
                        int v = calc_segment._graph.getVertex(calc_vertex).neiVertex(n);
                        if (prev_vertex < 0 || calc_segment.getIntPosition(v).y > calc_segment.getIntPosition(prev_vertex).y)
                            prev_vertex = v;
                        if (next_vertex < 0 || calc_segment.getIntPosition(v).y < calc_segment.getIntPosition(next_vertex).y)
                            next_vertex = v;
                    }
                    if (next_layout_component < 0)
                    {
                        int temp = prev_vertex;
                        prev_vertex = next_vertex;
                        next_vertex = temp;
                    }
                }
                angle = (calc_segment.getIntPosition(next_vertex) - calc_segment.getIntPosition(calc_vertex)).tiltAngle2();
                angle -= (calc_segment.getIntPosition(prev_vertex) - calc_segment.getIntPosition(calc_vertex)).tiltAngle2();

                while (angle < 0.f)
                    angle += _2FLOAT(2. * M_PI);
                while (angle >= _2FLOAT(2. * M_PI))
                    angle -= _2FLOAT(2 * M_PI);

                layout.setTargetAngle(i, _2FLOAT(M_PI - angle / 2.));
            }
            else if (prev_layout_component == next_layout_component)
            {
                float angle = (getPos(cycle.getVertexC(i - 1)) - getPos(cycle.getVertexC(i))).tiltAngle2();
                angle -= (getPos(cycle.getVertexC(i + 1)) - getPos(cycle.getVertexC(i))).tiltAngle2();

                while (angle < 0.f)
                    angle += _2FLOAT(2. * M_PI);
                while (angle >= _2FLOAT(2. * M_PI))
                    angle -= _2FLOAT(2. * M_PI);
                if (angle > _2FLOAT(M_PI))
                    angle = _2FLOAT(2. * M_PI - angle);

                layout.setTargetAngle(i, angle);
            }
            // temporary value
            else
            {
                layout.setTargetAngle(i, _2FLOAT(M_PI));
                layout.setAngleImportance(i, 0.2f);
            }
        }
    }

    QS_DEF(Array<int>, _is_vertex_taken);
    enum
    {
        NOT_CONSIDERED,
        IN_LIST,
        NOT_IN_LIST
    };
    QS_DEF(Array<int>, _list_of_vertex);
    QS_DEF(Array<int>, _segment_weight_outside);

    _segment_weight_outside.clear_resize(segment_count);
    _segment_weight_outside.zerofill();

    for (int i = 0; i < size; i++)
        if (_layout_component_number[cycle.getEdge(i)] < 0)
            _layout_component_number[cycle.getEdge(i)] = _layout_component_count;

    _layout_component_count++;

    QS_DEF(Array<bool>, _is_layout_component_incoming);
    _is_layout_component_incoming.clear_resize(_layout_component_count);
    _is_layout_component_incoming.zerofill();
    for (int i = 0; i < size; i++)
        _is_layout_component_incoming[_layout_component_number[cycle.getEdge(i)]] = true;

    for (int i = 0; i < segment_count; i++)
    {
        for (int up = 0; up <= 1; up++)
        {
            _is_vertex_taken.clear_resize(_graph->vertexEnd());
            _is_vertex_taken.fill(NOT_CONSIDERED);

            if (i == segment_count - 1)
            {
                // int x = 5;
            }

            _list_of_vertex.clear_resize(0);

            bool is_segment_trivial =
                segment[i].get_layout_component_number() == -1 && segment[(i + segment_count - 1) % segment_count].get_layout_component_number() == -1 && up;

            for (int v = segment[i]._graph.vertexBegin(); v != segment[i]._graph.vertexEnd(); v = segment[i]._graph.vertexNext(v))
            {
                if ((!segment[i].is_finish(v) && !segment[i].is_start(v) && segment[i].isVertexUp(v) ^ !up) || (is_segment_trivial && !segment[i].is_finish(v)))
                {

                    int ext_v = segment[i]._graph.getVertexExtIdx(v);
                    _is_vertex_taken[getVertexExtIdx(ext_v)] = IN_LIST;
                    _list_of_vertex.push(ext_v);
                }
            }

            bool touch_to_another_segment = false;

            for (int j = 0; j < _list_of_vertex.size(); j++)
            {
                const Vertex& vert = getVertex(_list_of_vertex[j]);
                for (int n = vert.neiBegin(); n != vert.neiEnd(); n = vert.neiNext(n))
                {
                    int vn = vert.neiVertex(n);

                    if (_is_vertex_taken[getVertexExtIdx(vn)] != NOT_CONSIDERED)
                        continue;

                    bool is_this_comp = false;
                    for (int n2 = getVertex(vn).neiBegin(); n2 != getVertex(vn).neiEnd(); n2 = getVertex(vn).neiNext(n2))
                        if (_layout_component_number[getVertex(vn).neiEdge(n2)] >= 0)
                        {
                            if (_is_layout_component_incoming[_layout_component_number[getVertex(vn).neiEdge(n2)]])
                                is_this_comp = true;
                            if (!is_segment_trivial && _layout_component_number[getVertex(vn).neiEdge(n2)] != segment[i].get_layout_component_number() &&
                                _layout_component_number[getVertex(vn).neiEdge(n2)] != _layout_component_count - 1)
                                touch_to_another_segment = true;
                        }

                    if (!is_this_comp)
                    {
                        _list_of_vertex.push(vn);
                        _is_vertex_taken[getVertexExtIdx(vn)] = IN_LIST;
                    }
                    else
                        _is_vertex_taken[getVertexExtIdx(vn)] = NOT_IN_LIST;
                }
            }

            for (int j = 0; j < _list_of_vertex.size(); j++)
                _list_of_vertex[j] = getVertexExtIdx(_list_of_vertex[j]);

            for (int j = 0; j < _list_of_vertex.size(); j++)
            {
                const Vertex& vert = _graph->getVertex(_list_of_vertex[j]);
                for (int n = vert.neiBegin(); n != vert.neiEnd(); n = vert.neiNext(n))
                {
                    int vn = vert.neiVertex(n);

                    if (_is_vertex_taken[vn] != NOT_CONSIDERED)
                        continue;

                    _list_of_vertex.push(vn);
                    _is_vertex_taken[vn] = IN_LIST;
                }
            }

            _segment_weight_outside[i] += (up ? 1 : -1) * (touch_to_another_segment ? 3 : 1) * _list_of_vertex.size();
        }
    }

    QS_DEF(Array<int>, _index_in_cycle);
    _index_in_cycle.clear_resize(vertexEnd());
    _index_in_cycle.fffill();
    for (int i = 0; i < size; i++)
        _index_in_cycle[cycle.getVertex(i)] = i;

    for (int i = 0; i < segment_count; i++)
    {
        if (segment[i].get_layout_component_number() < 0 && segment[(i + segment_count - 1) % segment_count].get_layout_component_number() < 0)
            layout.addVertexOutsideWeight(rotation_vertex[i], _segment_weight_outside[i] - 1);
        else
        {
            layout.setComponentFinish(rotation_vertex[i], rotation_vertex[(i + 1) % segment_count]);
            layout.setVertexAddedSquare(rotation_vertex[i], segment[i].get_square());

            Cycle border;
            if (segment[i].get_layout_component_number() >= 0)
                segment[i]._graph._getBorder(border);

            int count_neibourhoods_outside = 0;

            if (segment[i].get_layout_component_number() >= 0 && border.vertexCount() != 0)
            {
                int start_in_border = -1;
                int finish_in_border = -1;
                for (int j = 0; j < border.vertexCount(); j++)
                {
                    if (border.getVertex(j) == segment[i].get_start())
                        start_in_border = j;
                    if (border.getVertex(j) == segment[i].get_finish())
                        finish_in_border = j;
                }

                if (start_in_border >= 0 && finish_in_border >= 0)
                {
                    for (int j = (start_in_border + 1) % border.vertexCount(); j != finish_in_border; j = (j + 1) % border.vertexCount())
                    {
                        if (_index_in_cycle[segment[i]._graph.getVertexExtIdx(border.getVertex(j))] == (rotation_vertex[i] + 1) % size)
                            count_neibourhoods_outside++;
                        if (_index_in_cycle[segment[i]._graph.getVertexExtIdx(border.getVertex(j))] ==
                            (rotation_vertex[(i + 1) % segment_count] - 1 + size) % size)
                            count_neibourhoods_outside++;
                    }

                    for (int j = (finish_in_border + 1) % border.vertexCount(); j != start_in_border; j = (j + 1) % border.vertexCount())
                    {
                        if (_index_in_cycle[segment[i]._graph.getVertexExtIdx(border.getVertex(j))] == (rotation_vertex[i] + 1) % size)
                            count_neibourhoods_outside--;
                        if (_index_in_cycle[segment[i]._graph.getVertexExtIdx(border.getVertex(j))] ==
                            (rotation_vertex[(i + 1) % segment_count] - 1 + size) % size)
                            count_neibourhoods_outside--;
                    }
                }
            }
            bool right_orientation;

            if (count_neibourhoods_outside > 0)
                right_orientation = true;
            else if (count_neibourhoods_outside < 0)
                right_orientation = false;
            else
            {
                float y1 = 0, y2 = 0;
                for (int v = segment[i]._graph.vertexBegin(); v != segment[i]._graph.vertexEnd(); v = segment[i]._graph.vertexNext(v))
                {
                    if (_index_in_cycle[segment[i]._graph.getVertexExtIdx(v)] == (rotation_vertex[i] + 1) % size)
                    {
                        y1 = segment[i].getIntPosition(v).y;
                    }
                    if (_index_in_cycle[segment[i]._graph.getVertexExtIdx(v)] == (rotation_vertex[(i + 1) % segment_count] + size - 1) % size)
                    {
                        y2 = segment[i].getIntPosition(v).y;
                    }
                }

                if ((y1 + y2) / 2 > EPSILON || ((fabs((y1 + y2) / 2) <= EPSILON) && (y1 + y2) / 2 > segment[i].getIntCenter().y))
                {
                    right_orientation = true;
                }
                else
                {
                    right_orientation = false;
                }
            }
            if (right_orientation)
            {
                layout.addVertexOutsideWeight(rotation_vertex[i], -_segment_weight_outside[i]);
                layout.addVertexOutsideWeight(rotation_vertex[(i + 1) % segment_count], -_segment_weight_outside[i]);
            }
            else
            {
                layout.addVertexOutsideWeight(rotation_vertex[i], _segment_weight_outside[i]);
                layout.addVertexOutsideWeight(rotation_vertex[(i + 1) % segment_count], _segment_weight_outside[i]);
            }
        }
    }

    layout.doLayout();

    // now we must to smooth just made layout
    // lets check if all cycle is layouted ealier in single biconnected compenent

    /*int start = -1;
    bool undrawn = false;
    for (int i = 0; i < size; i++) undrawn |= _layout_vertices[cycle.getVertex(i)].type == ELEMENT_NOT_DRAWN;
    if (undrawn) {
    for (int i = size - 1; i >= 0; i--) if (_layout_vertices[cycle.getVertex(i)].type != ELEMENT_NOT_DRAWN) start = i;
    if (start == 0 && _layout_vertices[cycle.getVertex(size - 1)].type != ELEMENT_NOT_DRAWN) {
    while (_layout_vertices[cycle.getVertex(start)].type != ELEMENT_NOT_DRAWN) start = (start + 1) % size;
    while (_layout_vertices[cycle.getVertex(start)].type == ELEMENT_NOT_DRAWN) start = (start + 1) % size;
    }
    }*/

    QS_DEF(Array<bool>, need_to_insert);
    need_to_insert.clear_resize(size);
    need_to_insert.zerofill();

    for (int i = 0; i < size; i++)
        need_to_insert[i] = _layout_vertices[cycle.getVertex(i)].type != ELEMENT_NOT_DRAWN;

    // int start = 0;

    bool componentIsWholeCycle = false;

    QS_DEF(Array<bool>, _is_component_touch);
    _is_component_touch.clear_resize(_layout_component_count);

    for (int index = 0; index < size; index++)
        if (need_to_insert[index])
        {
            // 1. search of connected component
            QS_DEF(Array<int>, insideVertex);
            insideVertex.clear_resize(0);
            insideVertex.push(cycle.getVertex(index));

            QS_DEF(Array<bool>, takenVertex);
            takenVertex.clear_resize(vertexCount());
            takenVertex.zerofill();
            takenVertex[cycle.getVertex(index)] = true;

            _is_component_touch.zerofill();

            for (int i = 0; i < insideVertex.size(); i++)
                for (int j = getVertex(insideVertex[i]).neiBegin(); j != getVertex(insideVertex[i]).neiEnd(); j = getVertex(insideVertex[i]).neiNext(j))
                {
                    int vertj = getVertex(insideVertex[i]).neiVertex(j);
                    if (_layout_edges[getVertex(insideVertex[i]).neiEdge(j)].type != ELEMENT_NOT_DRAWN && !takenVertex[vertj])
                    {
                        _is_component_touch[_layout_component_number[getVertex(insideVertex[i]).neiEdge(j)]] = true;
                        insideVertex.push(vertj);
                        takenVertex[vertj] = true;
                    }
                }

            if (!componentIsWholeCycle)
            {
                componentIsWholeCycle = true;
                for (int i = 0; i < size; i++)
                    componentIsWholeCycle &= takenVertex[cycle.getVertex(i)];
            }

            for (int i = 0; i < size; i++)
                if (takenVertex[cycle.getVertex(i)])
                    need_to_insert[i] = false;

            if (componentIsWholeCycle)
                break;

            int startIndex = index;
            int endIndex = index;

            while (takenVertex[cycle.getVertex(startIndex)])
                startIndex = (startIndex - 1 + size) % size;
            startIndex = (startIndex + 1) % size;
            while (takenVertex[cycle.getVertex(endIndex)])
                endIndex = (endIndex + 1) % size;

            // 2. flip
            bool need_to_flip = false;
            float rotate1 = Vec2f::cross(layout.getPos((startIndex + 1) % size) - layout.getPos(startIndex),
                                         layout.getPos((startIndex + 2) % size) - layout.getPos((startIndex + 1) % size));
            if (isEdgeDrawn(cycle.getEdgeC(startIndex + 1)))
            {
                float rotate2 = Vec2f::cross(getPos(cycle.getVertexC(startIndex + 1)) - getPos(cycle.getVertexC(startIndex)),
                                             getPos(cycle.getVertexC(startIndex + 2)) - getPos(cycle.getVertexC(startIndex + 1)));

                if (isEdgeDrawn(cycle.getEdgeC(startIndex)))
                    need_to_flip = rotate1 * rotate2 < 0;
            }
            else
            {
                float rotate1_next = rotate1;
                float rotate1_prev = Vec2f::cross(layout.getPos(startIndex) - layout.getPos((startIndex - 1 + size) % size),
                                                  layout.getPos((startIndex + 1) % size) - layout.getPos(startIndex));
                int do_flip_cnt = 0;
                int dont_flip_cnt = 0;

                int ind0 = cycle.getVertexC(startIndex);
                int ind1 = cycle.getVertexC(startIndex + 1);
                const Vertex& v0 = getVertex(ind0);
                const Vertex& v1 = getVertex(ind1);

                for (int j = v0.neiBegin(); j != v0.neiEnd(); j = v0.neiNext(j))
                    if (v0.neiVertex(j) != ind1 && isEdgeDrawn(v0.neiEdge(j)))
                    {
                        float current_rotate = Vec2f::cross(getPos(ind0) - getPos(v0.neiVertex(j)), getPos(ind1) - getPos(ind0));
                        if (current_rotate * rotate1_prev > 0)
                            do_flip_cnt++;
                        else
                            dont_flip_cnt++;
                    }

                for (int j = v1.neiBegin(); j != v1.neiEnd(); j = v1.neiNext(j))
                    if (v1.neiVertex(j) != ind0 && isEdgeDrawn(v1.neiEdge(j)))
                    {
                        float current_rotate = Vec2f::cross(getPos(ind1) - getPos(ind0), getPos(v1.neiVertex(j)) - getPos(ind1));
                        if (current_rotate * rotate1_next > 0)
                            do_flip_cnt++;
                        else
                            dont_flip_cnt++;
                    }

                need_to_flip = do_flip_cnt > dont_flip_cnt;
            }

            /*float rotate1 = Vec2f::cross(layout.getPos((startIndex + 1) % size) - layout.getPos(startIndex), layout.getPos((startIndex + 2) % size) -
            layout.getPos((startIndex + 1) % size)); Vec2f next_point; if (isEdgeDrawn(cycle.getEdgeC(startIndex + 1))) next_point =
            getPos(cycle.getVertexC(startIndex + 2)); else { for (int j = getVertex(cycle.getVertexC(startIndex + 1)).neiBegin(); j !=
            getVertex(cycle.getVertexC(startIndex + 1)).neiEnd(); j = getVertex(cycle.getVertexC(startIndex + 1)).neiNext(j)) if
            (isEdgeDrawn(getVertex(cycle.getVertexC(startIndex + 1)).neiEdge(j)) && getVertex(cycle.getVertexC(startIndex + 1)).neiVertex(j) !=
            cycle.getVertex(startIndex)) next_point = _layout_vertices[getVertex(cycle.getVertexC(startIndex + 1)).neiVertex(j)].pos;
            }

            float rotate2 = Vec2f::cross(getPos(cycle.getVertexC(startIndex + 1)) - getPos(cycle.getVertexC(startIndex)),
                next_point - getPos(cycle.getVertexC(startIndex + 1)));

            if (!isEdgeDrawn(cycle.getEdgeC(startIndex + 1))) {
                need_to_flip = rotate1 * rotate2 > 0;
            }
            else if (isEdgeDrawn(cycle.getEdgeC(startIndex))) need_to_flip = rotate1 * rotate2 < 0;
            */

            if (need_to_flip)
            {
                for (int i = 0; i < insideVertex.size(); i++)
                    getPos(insideVertex[i]).x *= -1;

                for (int i = 0; i < segment.size(); i++)
                    if (segment[i].get_layout_component_number() >= 0 && _is_component_touch[segment[i].get_layout_component_number()])
                        segment[i].inverse();
            }

            // 3. shift

            Vec2f middle_host;
            Vec2f middle_new;
            int countVertex = 0;
            for (int i = startIndex; i != endIndex; i = (i + 1) % size)
            {
                middle_host += _layout_vertices[cycle.getVertex(i)].pos;
                middle_new += layout.getPos(i);
                countVertex++;
            }
            middle_host /= _2FLOAT(countVertex);
            middle_new /= _2FLOAT(countVertex);

            for (int i = 0; i < insideVertex.size(); i++)
                _layout_vertices[insideVertex[i]].pos += middle_new - middle_host;

            // 4. rotate

            Vec2f direction_host;
            Vec2f direction_new;
            if (countVertex > 1)
            {
                int currentIndex = 0;
                for (int i = startIndex; i != endIndex; i = (i + 1) % size)
                {
                    if (2 * currentIndex < countVertex - 1)
                    {
                        direction_host += _layout_vertices[cycle.getVertex(i)].pos;
                        direction_new += layout.getPos(i);
                    }
                    else if (2 * currentIndex > countVertex - 1)
                    {
                        direction_host -= _layout_vertices[cycle.getVertex(i)].pos;
                        direction_new -= layout.getPos(i);
                    }
                    currentIndex++;
                }

                float dot = Vec2f::dot(direction_host, direction_new) / (direction_host.length() * direction_new.length());
                if (dot > 1)
                    dot = 1;
                if (dot < -1)
                    dot = -1;
                float angle = acos(dot);
                if (Vec2f::cross(direction_host, direction_new) < 0)
                    angle = -angle;
                for (int i = 0; i < insideVertex.size(); i++)
                    _layout_vertices[insideVertex[i]].pos.rotateAroundSegmentEnd(_layout_vertices[insideVertex[i]].pos, middle_new, angle);
            }
        }

    for (int i = 0; i < size; i++)
        if (getVertexType(cycle.getVertex(i)) == ELEMENT_NOT_DRAWN)
            getPos(cycle.getVertex(i)) = layout.getPos(i);

    for (int i = 0; i < size; i++)
    {
        setVertexType(cycle.getVertex(i), ELEMENT_DRAWN);
        setEdgeType(cycle.getEdge(i), ELEMENT_DRAWN);
    }

    // 5. smoothing
    for (int e = edgeBegin(); e != edgeEnd(); e = edgeNext(e))
        if (_layout_component_number[e] >= 0 && _is_layout_component_incoming[_layout_component_number[e]])
            _layout_component_number[e] = _layout_component_count - 1;

    _segment_smoothing(cycle, layout, rotation_vertex, rotation_point, segment);
}

void MoleculeLayoutGraphSmart::_segment_smoothing(const Cycle& cycle, const MoleculeLayoutMacrocyclesLattice& layout, Array<int>& rotation_vertex,
                                                  Array<Vec2f>& rotation_point, ObjArray<MoleculeLayoutSmoothingSegment>& segment)
{
    QS_DEF(Array<float>, target_angle);

    _segment_update_rotation_points(cycle, rotation_vertex, rotation_point, segment);
    _segment_calculate_target_angle(layout, rotation_vertex, target_angle, segment);

    if (segment.size() > 2)
    {
        _segment_smoothing_unstick(segment);
        // _do_segment_smoothing(rotation_point, target_angle, segment);
        _do_segment_smoothing_gradient(rotation_point, target_angle, segment);
    }
}

void MoleculeLayoutGraphSmart::_segment_update_rotation_points(const Cycle& cycle, Array<int>& rotation_vertex, Array<Vec2f>& rotation_point,
                                                               ObjArray<MoleculeLayoutSmoothingSegment>& segment)
{
    for (int i = 0; i < rotation_vertex.size(); i++)
        rotation_point[i] = getPos(cycle.getVertex(rotation_vertex[i]));

    for (int i = 0; i < segment.size(); i++)
        segment[i].updateStartFinish();
}

void MoleculeLayoutGraphSmart::_segment_calculate_target_angle(const MoleculeLayoutMacrocyclesLattice& layout, Array<int>& rotation_vertex,
                                                               Array<float>& target_angle, ObjArray<MoleculeLayoutSmoothingSegment>& segment)
{
    int segments_count = rotation_vertex.size();

    target_angle.clear_resize(segments_count);

    for (int i = 0; i < segments_count; i++)
    {
        Vec2f p1 = layout.getPos(rotation_vertex[(i - 1 + segments_count) % segments_count]);
        Vec2f p2 = layout.getPos(rotation_vertex[i]);
        Vec2f p3 = layout.getPos(rotation_vertex[(i + 1) % segments_count]);
        target_angle[i] = p2.calc_angle(p3, p1);
        while (target_angle[i] < 0.f)
            target_angle[i] += _2FLOAT(2. * M_PI);
    }

    for (int i = 0; i < segments_count; i++)
        for (int v = segment[i]._graph.vertexBegin(); v != segment[i]._graph.vertexEnd(); v = segment[i]._graph.vertexNext(v))
        {
            if (segment[i].is_start(v))
                if (segment[i]._graph.getVertex(v).degree() > 2)
                    target_angle[i] = _2FLOAT(M_PI);
            if (segment[i].is_finish(v))
                if (segment[i]._graph.getVertex(v).degree() > 2)
                    target_angle[(i + 1) % segments_count] = _2FLOAT(M_PI);
        }
}

void MoleculeLayoutGraphSmart::_segment_smoothing_unstick(ObjArray<MoleculeLayoutSmoothingSegment>& segment)
{

    int segment_count = segment.size();

    // prepearing of list of sticked pairs of vertices

    QS_DEF(Array<float>, min_x);
    min_x.clear_resize(segment_count);
    for (int i = 0; i < segment_count; i++)
        min_x[i] = segment[i].get_min_x();

    QS_DEF(Array<float>, max_x);
    max_x.clear_resize(segment_count);
    for (int i = 0; i < segment_count; i++)
        max_x[i] = segment[i].get_max_x();

    QS_DEF(Array<float>, min_y);
    min_y.clear_resize(segment_count);
    for (int i = 0; i < segment_count; i++)
        min_y[i] = segment[i].get_min_y();

    QS_DEF(Array<float>, max_y);
    max_y.clear_resize(segment_count);
    for (int i = 0; i < segment_count; i++)
        max_y[i] = segment[i].get_max_y();

    QS_DEF(Array<int>, component1);
    QS_DEF(Array<int>, component2);
    QS_DEF(Array<int>, vertex1);
    QS_DEF(Array<int>, vertex2);

    component1.clear_resize(0);
    component2.clear_resize(0);
    vertex1.clear_resize(0);
    vertex2.clear_resize(0);

    for (int i = 0; i < segment_count; i++)
        for (int j = (i + segment_count / 2) % segment_count; j != i; j = (j + segment_count - 1) % segment_count)
        {

            if (segment_count % 2 == 0 && j + segment_count / 2 == i)
                continue;

            if (min_x[i] <= max_x[j] && min_x[j] <= max_x[i] && min_y[i] <= max_y[j] && min_y[j] <= max_y[i])
            {
                for (int v1 = segment[i]._graph.vertexBegin(); v1 != segment[i]._graph.vertexEnd(); v1 = segment[i]._graph.vertexNext(v1))
                    for (int v2 = segment[j]._graph.vertexBegin(); v2 != segment[j]._graph.vertexEnd(); v2 = segment[j]._graph.vertexNext(v2))
                    {
                        if (Vec2f::distSqr(segment[i].getPosition(v1), segment[j].getPosition(v2)) < 0.1)
                            if ((i + 1) % segment_count != j || !segment[i].is_finish(v1))
                            {
                                component1.push(i);
                                component2.push(j);
                                vertex1.push(v1);
                                vertex2.push(v2);
                            }
                    }
            }
        }

    int count_sticked_vertices = component1.size();

    bool something_done = true;
    bool something_to_do = false;
    while (something_done)
    {
        something_done = false;
        something_to_do = false;

        for (int index = 0; index < count_sticked_vertices; index++)
        {
            int i = component1[index];
            int j = component2[index];
            int v1 = vertex1[index];
            int v2 = vertex2[index];

            if (Vec2f::distSqr(segment[i].getPosition(v1), segment[j].getPosition(v2)) < EPSILON)
            {
                something_to_do = true;

                bool exist_sepatate_vertex = false;
                const Vertex& vert1 = segment[i]._graph.getVertex(v1);
                const Vertex& vert2 = segment[j]._graph.getVertex(v2);

                for (int u1 = vert1.neiBegin(); u1 != vert1.neiEnd() && !exist_sepatate_vertex; u1 = vert1.neiNext(u1))
                {
                    bool exist_same_vertex = false;
                    int nei1 = vert1.neiVertex(u1);
                    for (int u2 = vert2.neiBegin(); u2 != vert2.neiEnd() && !exist_same_vertex; u2 = vert2.neiNext(u2))
                    {
                        int nei2 = vert2.neiVertex(u2);
                        if (Vec2f::dist(segment[i].getPosition(nei1), segment[j].getPosition(nei2)) < EPSILON)
                            exist_same_vertex = true;
                    }
                    if (!exist_same_vertex)
                        exist_sepatate_vertex = true;
                }

                if (exist_sepatate_vertex)
                {
                    Vec2f direction;
                    if (vert1.degree() == 2)
                    {
                        direction = (segment[i].getPosition(vert1.neiVertex(vert1.neiBegin())) +
                                     segment[i].getPosition(vert1.neiVertex(vert1.neiNext(vert1.neiBegin())))) /
                                    2;
                        direction -= segment[i].getPosition(v1);
                    }
                    else if (vert2.degree() == 2)
                    {
                        direction = (segment[j].getPosition(vert2.neiVertex(vert2.neiBegin())) +
                                     segment[j].getPosition(vert2.neiVertex(vert2.neiNext(vert2.neiBegin())))) /
                                    2;
                        direction -= segment[i].getPosition(v1);
                    }
                    else if (vert1.degree() == 1)
                    {
                        direction = segment[i].getPosition(vert1.neiVertex(vert1.neiBegin()));
                        direction -= segment[i].getPosition(v1);
                        direction.rotate(1, 0);
                    }
                    else if (vert2.degree() == 1)
                    {
                        direction = segment[j].getPosition(vert2.neiVertex(vert2.neiBegin()));
                        direction -= segment[i].getPosition(v1);
                        direction.rotate(1, 0);
                    }
                    else
                        continue;

                    direction /= 3.f;

                    bool moved = false;
                    for (int sign = 1; sign >= -1 && !moved; sign -= 2)
                    {

                        Vec2f newpos = segment[i].getPosition(v1) + (direction * _2FLOAT(sign));
                        bool can_to_move = true;

                        for (int u1 = vert1.neiBegin(); u1 != vert1.neiEnd() && can_to_move; u1 = vert1.neiNext(u1))
                        {
                            int nei1 = vert1.neiVertex(u1);
                            for (int u2 = vert2.neiBegin(); u2 != vert2.neiEnd() && can_to_move; u2 = vert2.neiNext(u2))
                            {
                                int nei2 = vert2.neiVertex(u2);
                                if (Vec2f::segmentsIntersectInternal(newpos, segment[i].getPosition(nei1), segment[j].getPosition(v2),
                                                                     segment[j].getPosition(nei2)))
                                    can_to_move = false;
                            }
                        }

                        if (can_to_move)
                        {
                            something_done = true;
                            moved = true;

                            segment[i].shiftStartBy(direction * _2FLOAT(sign) / 2.f);
                            segment[i].shiftFinishBy(direction * _2FLOAT(sign) / 2.f);

                            segment[j].shiftStartBy(direction * _2FLOAT(-sign) / 2.f);
                            segment[j].shiftFinishBy(direction * _2FLOAT(-sign) / 2.f);
                        }
                    }
                }
            }
        }
    }

    for (int i = 0; i < segment_count; i++)
        for (int v = segment[i]._graph.vertexBegin(); v != segment[i]._graph.vertexEnd(); v = segment[i]._graph.vertexNext(v))
            getPos(segment[i]._graph.getVertexExtIdx(v)).copy(segment[i].getPosition(v));
}

void MoleculeLayoutGraphSmart::_update_touching_segments(Array<local_pair_ii>& pairs, ObjArray<MoleculeLayoutSmoothingSegment>& segment)
{
    int segments_count = segment.size();
    float min_dist = 0.7f;
    pairs.clear();

    for (int i = 0; i < segments_count; i++)
        for (int j = i + 2; j < segments_count; j++)
            if (i != 0 || j != segments_count - 1)
            {
                if (segment[i].get_layout_component_number() >= 0 || segment[j].get_layout_component_number() >= 0)
                    continue;
                bool interseced = false;

                for (int v1 = segment[j]._graph.vertexBegin(); v1 != segment[j]._graph.vertexEnd() && !interseced; v1 = segment[j]._graph.vertexNext(v1))
                {
                    for (int v2 = segment[i]._graph.vertexBegin(); v2 != segment[i]._graph.vertexEnd() && !interseced; v2 = segment[i]._graph.vertexNext(v2))
                    {
                        if (Vec2f::distSqr(segment[j].getPosition(v1), segment[i].getPosition(v2)) < min_dist * min_dist)
                            interseced = true;
                    }
                }

                if (interseced)
                {
                    pairs.push(local_pair_ii(i, j));
                    pairs.push(local_pair_ii(j, i));
                }
            }
}

void MoleculeLayoutGraphSmart::_do_segment_smoothing(Array<Vec2f>& rotation_point, Array<float>& target_angle,
                                                     ObjArray<MoleculeLayoutSmoothingSegment>& segment)
{
    // profTimerStart(t, "_do_segment_smoothing");
    Random rand(34577);

    int segments_count = segment.size();

    QS_DEF(Array<local_pair_ii>, touching_segments);

    for (int i = 0; i < 10000; i++)
    {
        if ((i & (i - 1)) == 0)
            _update_touching_segments(touching_segments, segment);
        if (i % 100 == 0 && touching_segments.size() == 0)
        {
            bool all_right = true;
            for (int j = 0; all_right && j < segments_count; j++)
                all_right &= fabs(target_angle[j] - rotation_point[j].calc_angle(rotation_point[(j + 1) % segments_count],
                                                                                 rotation_point[(j + segments_count - 1) % segments_count])) < 1e-3;
            if (all_right)
                break;
        }
        _segment_improoving(rotation_point, target_angle, segment, rand.next() % segments_count, 0.1f, touching_segments);
    }

    for (int i = 0; i < segments_count; i++)
        for (int v = segment[i]._graph.vertexBegin(); v != segment[i]._graph.vertexEnd(); v = segment[i]._graph.vertexNext(v))
            getPos(segment[i]._graph.getVertexExtIdx(v)).copy(segment[i].getPosition(v));
}

void MoleculeLayoutGraphSmart::_segment_smoothing_prepearing(const Cycle& cycle, Array<int>& rotation_vertex, Array<Vec2f>& rotation_point,
                                                             ObjArray<MoleculeLayoutSmoothingSegment>& segment, MoleculeLayoutMacrocyclesLattice& layout)
{
    int cycle_size = cycle.vertexCount();

    QS_DEF(Array<bool>, layout_comp_touch);
    layout_comp_touch.clear_resize(_layout_component_count);
    layout_comp_touch.zerofill();

    for (int i = 0; i < cycle_size; i++)
    {
        if (_layout_component_number[cycle.getEdge(i)] >= 0)
            layout_comp_touch[_layout_component_number[cycle.getEdge(i)]] = true;
    }

    QS_DEF(ObjArray<Filter>, segments_filter);
    segments_filter.clear();

    QS_DEF(Array<int>, segment_start);
    segment_start.clear_resize(0);
    segment_start.fffill();

    QS_DEF(Array<bool>, touch_to_current_component);
    touch_to_current_component.clear_resize(cycle_size);

    QS_DEF(Array<int>, segment_component_number);
    segment_component_number.clear();

    segment.clear();

    for (int i = 0; i < _layout_component_count; i++)
        if (layout_comp_touch[i])
        {

            // search of vertices touch to i-th layout component
            touch_to_current_component.zerofill();
            for (int j = 0; j < cycle_size; j++)
            {
                const Vertex& vert = getVertex(cycle.getVertex(j));
                for (int nei = vert.neiBegin(); nei != vert.neiEnd(); nei = vert.neiNext(nei))
                    if (getEdgeType(vert.neiEdge(nei)) != ELEMENT_NOT_DRAWN)
                    {
                        if (_layout_component_number[vert.neiEdge(nei)] == i)
                            touch_to_current_component[j] = true;
                    }
            }

            // search of start and finish of occupated segment of cycle
            // if there is at least two starts of finishes then it is separationg layout component
            int start = -1;
            int finish = -1;

            for (int j = 0; j < cycle_size; j++)
                if (touch_to_current_component[j] && _layout_component_number[cycle.getEdgeC(j - 1)] != i)
                {
                    if (start != -1)
                        throw Exception("Separating layout component in cycle\n");
                    else
                        start = j;
                }

            for (int j = 0; j < cycle_size; j++)
                if (touch_to_current_component[j] && _layout_component_number[cycle.getEdge(j)] != i)
                {
                    if (finish != -1)
                        throw Exception("Separating layout component in cycle\n");
                    else
                        finish = j;
                }

            if (start != finish)
            {
                segments_filter.push();
                segments_filter.top().initNone(vertexEnd());
                for (int e = edgeBegin(); e != edgeEnd(); e = edgeNext(e))
                    if (_layout_component_number[e] == i)
                    {
                        segments_filter.top().unhide(getEdge(e).beg);
                        segments_filter.top().unhide(getEdge(e).end);
                    }

                segment_start.push(start);
                segment_component_number.push(i);
            }
        }

    for (int i = 0; i < cycle_size; i++)
        if (_layout_component_number[cycle.getEdge(i)] < 0)
        {
            int i_1 = (i + cycle_size - 1) % cycle_size; // i - 1
            if (_layout_component_number[cycle.getEdge(i_1)] < 0 && !layout.getVertexStereo(i))
                continue;
            int last = i;
            while (_layout_component_number[cycle.getEdgeC(last + 1)] < 0 && !layout.getVertexStereo((last + 1) % cycle_size))
                last = (last + 1) % cycle_size;

            segment_start.push(i);

            segments_filter.push();
            segments_filter.top().initNone(vertexEnd());
            for (int v = i; v != (last + 2) % cycle_size; v = (v + 1) % cycle_size)
                segments_filter.top().unhide(cycle.getVertex(v));
            // segments_filter.top().unhide(cycle.getVertex(i));
            // segments_filter.top().unhide(cycle.getVertexC(i + 1));

            segment_component_number.push(-1);
        }

    int segments_count = segments_filter.size();

    if (segments_count == 0)
        return;

    QS_DEF(Array<int>, number_of_segment);
    number_of_segment.clear_resize(cycle_size);
    number_of_segment.fffill();
    for (int i = 0; i < segments_count; i++)
        number_of_segment[segment_start[i]] = i;

    rotation_vertex.clear_resize(0);
    for (int i = 0; i < cycle_size; i++)
        if (number_of_segment[i] != -1)
            rotation_vertex.push(segment_start[number_of_segment[i]]);

    rotation_point.clear_resize(segments_count);
    _segment_update_rotation_points(cycle, rotation_vertex, rotation_point, segment);

    QS_DEF(ObjArray<MoleculeLayoutGraphSmart>, segment_graph);
    segment_graph.clear();
    for (int i = 0; i < segments_count; i++)
    {
        segment_graph.push().makeLayoutSubgraph(*this, segments_filter[i]);
    }

    // int segment_count = segment_graph.size();

    int current_number = 0;
    for (int i = 0; i < cycle_size; i++)
        if (number_of_segment[i] != -1)
        {
            segment.push(segment_graph[number_of_segment[i]], rotation_point[current_number], rotation_point[(1 + current_number) % segments_count]);
            segment.top().set_layout_component_number(segment_component_number[number_of_segment[i]]);
            segment.top().set_start_finish_number(cycle.getVertex(rotation_vertex[current_number]),
                                                  cycle.getVertex(rotation_vertex[(current_number + 1) % segments_count]));
            current_number++;
        }
}

void MoleculeLayoutGraphSmart::_segment_improoving(Array<Vec2f>& point, Array<float>& target_angle, ObjArray<MoleculeLayoutSmoothingSegment>& segment,
                                                   int move_vertex, float coef, Array<local_pair_ii>& touching_segments)
{
    int segments_count = segment.size();
    Vec2f move_vector(0, 0);

    // fix intersections to other components
    for (int i = 0; i < touching_segments.size(); i++)
        if (touching_segments[i].left == move_vertex || touching_segments[i].left == (move_vertex + 1) % segments_count)
        {
            int another_segment = touching_segments[i].right;
            float min_dist = 0.7f;
            // float dist2 = min_dist;
            bool interseced = false;
            for (int v1 = segment[move_vertex]._graph.vertexBegin(); !interseced && v1 != segment[move_vertex]._graph.vertexEnd();
                 v1 = segment[move_vertex]._graph.vertexNext(v1))
            {
                for (int v2 = segment[another_segment]._graph.vertexBegin(); !interseced && v2 != segment[another_segment]._graph.vertexEnd();
                     v2 = segment[another_segment]._graph.vertexNext(v2))
                {
                    if ((segment[move_vertex].getPosition(v1) - segment[another_segment].getPosition(v2)).lengthSqr() < min_dist * min_dist)
                        interseced = true;
                    // dist2 = min(dist2, (segment[move_vertex].getPosition(v1) - segment[another_segment].getPosition(v2)).lengthSqr());
                }
            }
            // dist2 = max(dist2, 0.25f);
            if (interseced)
            {
                Vec2f shift1(segment[move_vertex].getCenter());
                Vec2f shift2(segment[touching_segments[i].right].getCenter());
                Vec2f shift(shift1 - shift2);
                shift.normalize();
                move_vector += shift;
            }
        }

    // fix angle
    Vec2f prev_point(point[(move_vertex + segments_count - 1) % segments_count]);
    Vec2f this_point(point[move_vertex]);
    Vec2f next_point(point[(move_vertex + 1) % segments_count]);

    if (fabs(target_angle[move_vertex] - M_PI) > 0.01)
    {
        Vec2f chord(next_point - prev_point);

        Vec2f center(prev_point + chord / 2);
        Vec2f rot_chord(chord);
        rot_chord.rotate(1, 0);
        center += rot_chord / _2FLOAT(tan(M_PI - target_angle[move_vertex])) / 2.f;

        float radii = (prev_point - center).length();
        float dist = (this_point - center).length();

        move_vector += (this_point - center) * (radii - dist) / radii;
        // move_vector += get_move_vector(this_point, center, radii);
    }
    else
    {
        float l1 = segment[(move_vertex + segments_count - 1) % segments_count].getLength();
        float l2 = segment[move_vertex].getLength();
        Vec2f center(prev_point * l2 + next_point * l1);

        center /= l1 + l2;

        Vec2f chord(next_point - prev_point);
        chord.rotate(1, 0);
        center += chord * _2FLOAT(target_angle[move_vertex] - M_PI) * l1 * l2 / (l1 + l2);

        move_vector += (center - this_point);
    }

    // fix distance to neighborhoods
    move_vector += (this_point - next_point) * segment[move_vertex].getLengthCoef();
    move_vector += (this_point - prev_point) * segment[(move_vertex + segments_count - 1) % segments_count].getLengthCoef();
    //   move_vector += get_move_vector(this_point, prev_point, segment[(move_vertex + segments_count - 1) % segments_count].getLength());
    //   move_vector += get_move_vector(this_point, next_point, segment[move_vertex].getLength());

    // apply
    point[move_vertex] += move_vector * coef;
}

void MoleculeLayoutGraphSmart::_do_segment_smoothing_gradient(Array<Vec2f>& rotation_point, Array<float>& target_angle,
                                                              ObjArray<MoleculeLayoutSmoothingSegment>& segment)
{
    SmoothingCycle cycle(rotation_point, target_angle, segment);
    cycle._do_smoothing(100);

    for (int i = 0; i < cycle.cycle_length; i++)
        for (int v = segment[i]._graph.vertexBegin(); v != segment[i]._graph.vertexEnd(); v = segment[i]._graph.vertexNext(v))
            getPos(segment[i]._graph.getVertexExtIdx(v)).copy(segment[i].getPosition(v));
}

CP_DEF(SmoothingCycle);

SmoothingCycle::SmoothingCycle(Array<Vec2f>& p, Array<float>& t_a) : CP_INIT, point(p), target_angle(t_a), segment(0), cycle_length(-1), TL_CP_GET(edge_length)
{
}

SmoothingCycle::SmoothingCycle(Array<Vec2f>& p, Array<float>& t_a, Array<int>& e_l, int l) : SmoothingCycle(p, t_a)
{
    cycle_length = l;
    edge_length.clear_resize(cycle_length);
    for (int i = 0; i < cycle_length; i++)
        edge_length[i] = _2FLOAT(e_l[i]);
}

SmoothingCycle::SmoothingCycle(Array<Vec2f>& p, Array<float>& t_a, ObjArray<MoleculeLayoutSmoothingSegment>& s) : SmoothingCycle(p, t_a)
{
    segment = &s[0];
    cycle_length = s.size();
    edge_length.clear_resize(cycle_length);
    for (int i = 0; i < cycle_length; i++)
        edge_length[i] = _2FLOAT(s[i].getLength());
}

void SmoothingCycle::_do_smoothing(int /* iter_count */)
{
    QS_DEF(Array<local_pair_ii>, touching_segments);
    touching_segments.clear();

    float coef = 1.0f;
    // float multiplyer = std::max(0.5f, std::min(0.999f, _2FLOAT(1. - 10.0 / iter_count)));
    for (int i = 0; i < 100; i++, coef *= 0.9f)
    {
        _gradient_step(coef, touching_segments, 0);
    }
}

void SmoothingCycle::_gradient_step(float coef, Array<local_pair_ii>& /* touching_segments */, bool /* flag */)
{
    QS_DEF(Array<Vec2f>, change);
    change.clear_resize(cycle_length);
    for (int i = 0; i < cycle_length; i++)
        change[i] = Vec2f(0, 0);

    const float eps = 0.01f;
    for (int i = 0; i < cycle_length; i++)
    {
        int i_1 = (i - 1 + cycle_length) % cycle_length; // i - 1
        int i1 = (i + 1) % cycle_length;                 // i + 1

        change[i] += _get_len_derivative(point[i1] - point[i], get_length(i), false) * (is_simple_component(i) ? 1.f : 5.f);
        change[i] += _get_len_derivative(point[i_1] - point[i], get_length(i_1), false) * (is_simple_component(i_1) ? 1.f : 5.f);

        if (fabs(target_angle[i] - M_PI) > eps)
            change[i] += _get_angle_derivative(point[i] - point[i_1], point[i1] - point[i], _2FLOAT(M_PI - target_angle[i]), false);
    }

    for (int i = 0; i < cycle_length; i++)
        for (int j = i + 2; j < cycle_length; j++)
            if (j - i != cycle_length - 1)
                if (!is_simple_component(i) && !is_simple_component(j))
                {
                    float current_dist = (get_center(i) - get_center(j)).length();
                    float target_dist = get_radius(i) + get_radius(j) + 1.0f;
                    if (current_dist < target_dist)
                    {
                        float importance = 1;
                        Vec2f ch = _get_len_derivative_simple(get_center(i) - get_center(j), target_dist);
                        change[j] += ch / 2 * importance;
                        change[(j + 1) % cycle_length] += ch / 2 * importance;
                        change[i] -= ch / 2 * importance;
                        change[(i + 1) % cycle_length] -= ch / 2 * importance;
                    }
                }

    float len = 0;
    for (int i = 0; i < cycle_length; i++)
        len += change[i].lengthSqr();
    len = sqrt(len);
    if (len > 1)
        for (int i = 0; i < cycle_length; i++)
            change[i] /= len;

    for (int i = 0; i < cycle_length; i++)
        point[i] -= change[i] * coef;
}

Vec2f SmoothingCycle::_get_len_derivative(Vec2f current_vector, float target_dist, bool /* flag */)
{
    float dist = current_vector.length();
    // dist = std::max(dist, 0.01f);
    float coef = 1;
    if (dist >= target_dist)
    {
        coef = (dist / target_dist - 1) * 2 / target_dist / dist;
    }
    else
    {
        coef = -(target_dist / dist - 1) * 2 * target_dist / dist / dist / dist;
    }
    return current_vector * -coef;
}

Vec2f SmoothingCycle::_get_len_derivative_simple(Vec2f current_vector, float /* target_dist */)
{
    // float dist = current_vector.length();
    // dist = std::max(dist, 0.01f);
    float coef = -1; // dist - target_dist;
    return current_vector * -coef;
}

Vec2f SmoothingCycle::_get_angle_derivative(Vec2f left_point, Vec2f right_point, float target_angle, bool /* flag */)
{
    float len1_sq = left_point.lengthSqr();
    float len2_sq = right_point.lengthSqr();
    float len12 = sqrt(len1_sq * len2_sq);
    float cross = Vec2f::cross(left_point, right_point);
    float signcross = cross > 0.f ? 1.f : cross == 0.f ? 0.f : -1.f;
    float dot = Vec2f::dot(left_point, right_point);
    float signdot = dot > 0.f ? 1.f : dot == 0.f ? 0.f : -1.f;
    float cos = dot / len12;
    float alpha;
    Vec2f alphadv;
    if (fabs(cos) < 0.5)
    {
        Vec2f cosdv = ((right_point - left_point) * len12 - (left_point * len2_sq - right_point * len1_sq) * dot / len12) / (len1_sq * len2_sq);
        alpha = acos(cos) * signcross;
        alphadv = cosdv * _2FLOAT(-1. / sqrt(1. - cos * cos)) * signcross;
    }
    else
    {
        float sin = cross / len12;
        Vec2f vec = left_point + right_point;
        vec.rotate(-1, 0);
        Vec2f sindv = (vec * len12 - (left_point * len2_sq - right_point * len1_sq) * cross / len12) / (len1_sq * len2_sq);
        alphadv = sindv * _2FLOAT(1. / sqrt(1. - sin * sin)) * signdot;
        alpha = asin(sin);
        if (cos < 0)
        {
            if (alpha > 0)
                alpha = _2FLOAT(M_PI - alpha);
            else
                alpha = _2FLOAT(-M_PI - alpha);
        }
    }
    // float diff = fabs(alpha) > fabs(target_angle) ? alpha / target_angle - 1 : target_angle / alpha - 1;
    // Vec2f result = fabs(alpha) > fabs(target_angle) ? alphadv / target_angle : alphadv * (- target_angle) / (alpha * alpha);
    // return result * diff * 2;
    return alphadv * (alpha - target_angle) * 2.f;
}
