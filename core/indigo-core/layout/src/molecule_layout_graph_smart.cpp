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
#include "graph/morgan_code.h"
#include "layout/molecule_layout_graph.h"

#include <memory>
#include <vector>

using namespace indigo;

IMPL_ERROR(MoleculeLayoutGraphSmart, "layout_graph_smart");

MoleculeLayoutGraphSmart::MoleculeLayoutGraphSmart() : MoleculeLayoutGraph()
{
}

MoleculeLayoutGraphSmart::~MoleculeLayoutGraphSmart()
{
}

MoleculeLayoutGraph* MoleculeLayoutGraphSmart::getInstance()
{
    return new MoleculeLayoutGraphSmart();
}

void MoleculeLayoutGraphSmart::clear()
{
    MoleculeLayoutGraph::clear();
    _layout_component_number.clear();
    _layout_component_count = 0;
}

void MoleculeLayoutGraphSmart::makeLayoutSubgraph(MoleculeLayoutGraph& graph, Filter& vertex_filter)
{
    makeLayoutSubgraph(graph, vertex_filter, 0);
}

void MoleculeLayoutGraphSmart::makeLayoutSubgraph(MoleculeLayoutGraph& graph, Filter& vertex_filter, Filter* edge_filter)
{
    _molecule = graph._molecule;
    _graph = &graph;
    _molecule_edge_mapping = graph._molecule_edge_mapping;

    QS_DEF(Array<int>, vertices);
    QS_DEF(Array<int>, vertex_mapping);
    QS_DEF(Array<int>, edges);
    QS_DEF(Array<int>, edge_mapping);

    clear();

    vertex_filter.collectGraphVertices(graph, vertices);
    if (edge_filter != 0)
        (*edge_filter).collectGraphEdges(graph, edges);

    if (edge_filter != 0)
        makeSubgraph(graph, vertices, &vertex_mapping, &edges, &edge_mapping);
    else
        makeSubgraph(graph, vertices, &vertex_mapping);

    LayoutVertex new_vertex;
    LayoutEdge new_edge;

    new_vertex.is_cyclic = false;

    for (int i = 0; i < vertices.size(); i++)
    {
        new_vertex.ext_idx = vertices[i];
        new_vertex.orig_idx = graph._layout_vertices[vertices[i]].orig_idx;
        new_vertex.type = graph._layout_vertices[vertices[i]].type;
        new_vertex.morgan_code = graph._layout_vertices[vertices[i]].morgan_code;
        new_vertex.pos.copy(graph._layout_vertices[vertices[i]].pos);
        registerLayoutVertex(vertex_mapping[vertices[i]], new_vertex);
    }

    int index = 0;
    for (int i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
    {
        const Edge& edge = getEdge(i);
        int ext_idx = graph.findEdgeIndex(vertices[edge.beg], vertices[edge.end]);

        new_edge.ext_idx = ext_idx;
        new_edge.orig_idx = graph._layout_edges[ext_idx].orig_idx;
        new_edge.type = graph._layout_edges[ext_idx].type;
        registerLayoutEdge(i, new_edge);
    }

    _layout_component_number.clear_resize(edgeEnd());
    _layout_component_number.fffill();
    _layout_component_count = 0;
}

MoleculeLayoutSmoothingSegment::MoleculeLayoutSmoothingSegment(MoleculeLayoutGraphSmart& mol, Vec2f& start, Vec2f& finish)
    : _graph(mol), _start(start), _finish(finish)
{
    _center.zero();
    Vec2f diameter = (_finish - _start);
    _length = diameter.length();
    Vec2f rotate_vector = diameter / diameter.lengthSqr();
    rotate_vector.y *= -1;
    _pos.clear_resize(_graph.vertexEnd());

    bool is_line = false;
    for (int v : _graph.vertices())
        if (_graph.getVertex(v).degree() == 1)
            is_line = true;

    if (!is_line)
    {
        for (int v : _graph.vertices())
        {
            _pos[v].copy(_graph.getPos(v));

            _pos[v] -= _start;
            _pos[v].rotate(rotate_vector);
        }
    }
    else
    {
        // this is straight line
        QS_DEF(Array<int>, vert);
        vert.clear(); // list or vertices in order of connection
        for (int v : _graph.vertices())
            if (_graph.getVertex(v).degree() == 1)
            {
                vert.push(v);
                break;
            }
        vert.push(_graph.getVertex(vert[0]).neiVertex(_graph.getVertex(vert[0]).neiBegin()));
        while (vert.size() < _graph.vertexCount())
        {
            for (int n = _graph.getVertex(vert.top()).neiBegin(); n != _graph.getVertex(vert.top()).neiEnd(); n = _graph.getVertex(vert.top()).neiNext(n))
                if (_graph.getVertex(vert.top()).neiVertex(n) != vert[vert.size() - 2])
                {
                    vert.push(_graph.getVertex(vert.top()).neiVertex(n));
                }
        }
        for (int i = 0; i < vert.size(); i++)
            _pos[vert[i]].set(i * 1.f / _2FLOAT(vert.size() - 1), 0.f);
    }

    // double ternary search of center of component
    float MLx = 0, Ly = 0, MRx = 0, Ry = 0, Lx, Rx;
    for (int v : _graph.vertices())
    {
        MLx = std::min(MLx, _pos[v].x);
        MRx = std::max(MRx, _pos[v].x);
        Ly = std::min(Ly, _pos[v].y);
        Ry = std::max(Ry, _pos[v].y);
    }
    while (Ry - Ly > EPSILON)
    {
        float dy = (Ry - Ly) / 3;
        float ry[2];
        float My = Ly + dy;
        for (int i = 0; i < 2; i++)
        {
            Lx = MLx, Rx = MRx;
            float rx[2];
            while (Rx - Lx > EPSILON)
            {
                float dx = (Rx - Lx) / 3;
                float Mx = Lx + dx;
                for (int j = 0; j < 2; j++)
                {
                    rx[j] = calc_radius(Vec2f(Mx, My));
                    Mx += dx;
                }
                if (rx[0] > rx[1])
                    Lx += dx;
                else
                    Rx -= dx;
            }
            ry[i] = calc_radius(Vec2f(Rx, My));
            My += dy;
        }
        if (ry[0] > ry[1])
            Ly += dy;
        else
            Ry -= dy;
    }
    _center = Vec2f(Rx, Ry);
    _radius = calc_radius(_center);
    /*
    _radius = 0;
    Vec2f center(0.5, 0);
    for (int v : _graph.vertices()) {
       float dist = (center - _pos[v]).length();
       if (dist > _radius) _radius = dist;
    }
    _center = center;*/
    _square = 0;
}

float MoleculeLayoutSmoothingSegment::calc_radius(Vec2f c)
{
    float answer = 0;
    for (int v : _graph.vertices())
        answer = std::max(answer, (c - _pos[v]).lengthSqr());
    return sqrt(answer);
}

Vec2f MoleculeLayoutSmoothingSegment::_getPosition(Vec2f p)
{
    Vec2f point;
    point.copy(p);
    point.rotate(_finish - _start);
    return point + _start;
}

void MoleculeLayoutSmoothingSegment::updateStartFinish()
{
    _length = (_start - _finish).length();
}

float MoleculeLayoutSmoothingSegment::get_radius()
{
    return _radius * _length;
}

bool MoleculeLayoutSmoothingSegment::can_touch_to(MoleculeLayoutSmoothingSegment& seg)
{
    return ((_start + _finish) / 2 - (seg._start - seg._finish) / 2).length() <= get_radius() + seg.get_radius();
}

bool MoleculeLayoutSmoothingSegment::isVertexUp(int v)
{
    return _pos[v].y > 0;
}

Vec2f MoleculeLayoutSmoothingSegment::getPosition(int v)
{
    return _getPosition(_pos[v]);
}

Vec2f MoleculeLayoutSmoothingSegment::getIntPosition(int v) const
{
    return _pos[v];
}

void MoleculeLayoutSmoothingSegment::shiftStartBy(Vec2f shift)
{
    _start += shift;
}

void MoleculeLayoutSmoothingSegment::shiftFinishBy(Vec2f shift)
{
    _finish += shift;
}

float MoleculeLayoutSmoothingSegment::getLength() const
{
    return _length;
}

float MoleculeLayoutSmoothingSegment::getLengthCoef() const
{
    float l = (_finish - _start).length();
    return (_graph.vertexCount() > 2 ? 5 : 1) * (_length - l) / l;
}

float MoleculeLayoutSmoothingSegment::get_min_x()
{
    float answer = 1000000.0;

    for (int v : _graph.vertices())
    {
        float xx = getPosition(v).x;
        answer = std::min(answer, xx);
    }

    return answer;
}

float MoleculeLayoutSmoothingSegment::get_min_y()
{
    float answer = 1000000.0;

    for (int v : _graph.vertices())
    {
        float yy = getPosition(v).y;
        answer = std::min(answer, yy);
    }

    return answer;
}

float MoleculeLayoutSmoothingSegment::get_max_x()
{
    float answer = -1000000.0;

    for (int v : _graph.vertices())
    {
        float xx = getPosition(v).x;
        answer = std::max(answer, getPosition(v).x);
    }

    return answer;
}

float MoleculeLayoutSmoothingSegment::get_max_y()
{
    float answer = -1000000.0;

    for (int v : _graph.vertices())
    {
        float yy = getPosition(v).y;
        answer = std::max(answer, yy);
    }

    return answer;
}

/*Vec2f& MoleculeLayoutSmoothingSegment::getStart() {
   return _start;
}

Vec2f& MoleculeLayoutSmoothingSegment::getFinish() {
   return _finish;
}*/

Vec2f MoleculeLayoutSmoothingSegment::getCenter()
{
    return _getPosition(_center);
}

Vec2f MoleculeLayoutSmoothingSegment::getIntCenter()
{
    return _center;
}

int MoleculeLayoutSmoothingSegment::get_layout_component_number()
{
    return _layout_component_number;
}

void MoleculeLayoutSmoothingSegment::set_layout_component_number(int number)
{
    _layout_component_number = number;
}

void MoleculeLayoutSmoothingSegment::inverse()
{
    for (int v : _graph.vertices())
        _pos[v].y *= -1;
}

void MoleculeLayoutSmoothingSegment::set_start_finish_number(int s, int f)
{
    for (int v : _graph.vertices())
    {
        if (_graph.getVertexExtIdx(v) == s)
            _start_number = v;
        if (_graph.getVertexExtIdx(v) == f)
            _finish_number = v;
    }

    if (get_layout_component_number() == -1)
    {
        _pos[_start_number].set(0, 0);
        _pos[_finish_number].set(1, 0);
    }
    for (int v : _graph.vertices())
        _center += _pos[v];
    _center /= _2FLOAT(_graph.vertexCount());

    _radius = 0;
    for (int v : _graph.vertices())
    {
        float dist = (_center - _pos[v]).length();
        if (dist > _radius)
            _radius = dist;
    }

    calculate_square();
}

float MoleculeLayoutSmoothingSegment::get_square()
{
    return _square;
}

void MoleculeLayoutSmoothingSegment::calculate_square()
{
    if (_layout_component_number >= 0)
        _square = _graph._get_square();
    else
        _square = 0;
}

int MoleculeLayoutSmoothingSegment::get_start() const
{
    return _start_number;
}
int MoleculeLayoutSmoothingSegment::get_finish() const
{
    return _finish_number;
}

#ifdef M_LAYOUT_DEBUG

#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molfile_saver.h"

void MoleculeLayoutGraphSmart::saveDebug()
{
    int i;
    Molecule mol;
    QS_DEF(Array<int>, mapping);

    mapping.clear_resize(vertexEnd());

    for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
    {
        if (getVertexType(i) == ELEMENT_NOT_DRAWN)
            continue;

        mapping[i] = mol.addAtom(ELEM_C);
        mol.setAtomXyz(mapping[i], getPos(i).x, getPos(i).y, 0);
    }

    for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
    {
        if (getEdgeType(i) == ELEMENT_NOT_DRAWN)
            continue;

        const Edge& edge = getEdge(i);

        mol.addBond(mapping[edge.beg], mapping[edge.end], BOND_SINGLE);
    }

    static int id = 0;
    char out_name[100];

    sprintf_s(out_name, "D:\\mf\\draw\\trace_my\\%03d.mol", id);

    FileOutput fo(out_name);
    MolfileSaver ms(fo);

    ms.saveMolecule(mol);

    id++;
}

#endif