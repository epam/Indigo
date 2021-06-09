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

void MoleculeLayoutGraphSmart::layout(BaseMolecule& molecule, float bond_length, const Filter* filter, bool respect_existing)
{
    if (molecule.vertexCount() == 0)
        return;

    int n_components = countComponents();

    if (fabs(bond_length) < EPSILON)
        throw Error("zero bond length");

    _molecule = &molecule;
    if (n_components > 1)
        _layoutMultipleComponents(molecule, respect_existing, filter, bond_length);
    else
        _layoutSingleComponent(molecule, respect_existing, filter, bond_length);
}

void MoleculeLayoutGraphSmart::_calcMorganCodes()
{
    MorganCode morgan(*this);
    QS_DEF(Array<long>, morgan_codes);

    morgan.calculate(morgan_codes, 3, 7);

    _total_morgan_code = 0;
    for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
    {
        _layout_vertices[i].morgan_code = morgan_codes[i];
        _total_morgan_code += morgan_codes[i];
    }
}

void MoleculeLayoutGraphSmart::_makeComponentsTree(BiconnectedDecomposer& decon, PtrArray<MoleculeLayoutGraph>& components, Array<int>& tree)
{
    int i, j, v, k;
    bool from;

    for (i = 0; i < tree.size(); i++)
        tree[i] = -1;

    for (i = 0; i < components.size(); i++)
    {
        for (k = components[i]->vertexBegin(); k < components[i]->vertexEnd(); k = components[i]->vertexNext(k))
        {
            v = components[i]->getLayoutVertex(k).ext_idx;

            if (decon.isArticulationPoint(v))
            {
                // if connection vertex belongs to i-th component
                from = false;

                for (j = 0; j < decon.getIncomingComponents(v).size(); j++)
                {
                    // and component doesn't come from this vertex
                    if (decon.getIncomingComponents(v)[j] == i)
                        from = true;
                }

                // TODO: try to remove tree[];
                if (!from)
                    tree[v] = i;
            }
        }
    }
}

void MoleculeLayoutGraphSmart::_layoutMultipleComponents(BaseMolecule& molecule, bool respect_existing, const Filter* filter, float bond_length)
{
    QS_DEF(Array<Vec2f>, src_layout);
    QS_DEF(Array<int>, molecule_edge_mapping);

    int n_components = countComponents();

    const Array<int>& decomposition = getDecomposition();
    int i, j, k;

    molecule_edge_mapping.clear_resize(edgeEnd());

    for (i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
        molecule_edge_mapping[i] = getEdgeExtIdx(i);

    _molecule_edge_mapping = molecule_edge_mapping.ptr();

    PtrArray<MoleculeLayoutGraph> components;

    components.clear();

    for (i = 0; i < n_components; i++)
    {
        Filter comp_filter(decomposition.ptr(), Filter::EQ, i);
        std::unique_ptr<MoleculeLayoutGraph> current_component(getInstance());
        components.add(current_component.release());
        MoleculeLayoutGraph& component = *components.top();

        component.cancellation = cancellation;

        component.makeLayoutSubgraph(*this, comp_filter);
        component.max_iterations = max_iterations;
        component.layout_orientation = layout_orientation;

        component._molecule = &molecule;
        component._molecule_edge_mapping = molecule_edge_mapping.ptr();

        src_layout.clear_resize(component.vertexEnd());

        if (respect_existing)
            for (j = component.vertexBegin(); j < component.vertexEnd(); j = component.vertexNext(j))
                src_layout[j] = getPos(component.getVertexExtIdx(j));
        else
            src_layout.zerofill();

        if (filter != 0)
        {
            component._fixed_vertices.resize(component.vertexEnd());
            component._fixed_vertices.zerofill();

            for (j = component.vertexBegin(); j < component.vertexEnd(); j = component.vertexNext(j))
                if (!filter->valid(component.getVertexExtIdx(j)))
                {
                    component._fixed_vertices[j] = 1;
                    component._n_fixed++;
                    component._layout_vertices[j].pos = getPos(component.getVertexExtIdx(j));
                }
        }

        if (component.vertexCount() > 1)
        {
            component._calcMorganCodes();
            component._assignAbsoluteCoordinates(bond_length);
        }
        component._assignFinalCoordinates(bond_length, src_layout);
    }

    // position components
    float x_min, x_max, x_start = 0.f, dx;
    float y_min, y_max, y_start = 0.f, max_height = 0.f, dy;
    int col_count;
    int row, col;
    int n_fixed = 0;

    // fixed first
    if (filter != 0)
    {
        x_min = 1.0E+20f;
        y_min = 1.0E+20f;

        // find fixed components
        for (i = 0; i < n_components; i++)
        {
            MoleculeLayoutGraph& component = *components[i];

            if (component._n_fixed > 0)
            {
                n_fixed++;

                for (j = component.vertexBegin(); j < component.vertexEnd(); j = component.vertexNext(j))
                {
                    const Vec2f& pos = component.getPos(j);

                    if (pos.x < x_min)
                        x_min = pos.x;
                    if (pos.y < y_min)
                        y_min = pos.y;
                    if (pos.y > y_start)
                        y_start = pos.y;
                }
            }
        }

        // position fixed
        if (n_fixed > 0)
        {
            dy = -y_min;
            dx = -x_min;

            for (i = 0; i < n_components; i++)
            {
                MoleculeLayoutGraph& component = *components[i];

                if (component._n_fixed > 0)
                    for (j = component.vertexBegin(); j < component.vertexEnd(); j = component.vertexNext(j))
                        _layout_vertices[component.getVertexExtIdx(j)].pos.sum(component.getPos(j), Vec2f(dx, dy));
            }

            y_start += dy + 2 * bond_length;
        }
    }

    col_count = (int)ceil(sqrt((float)n_components - n_fixed));

    for (i = 0, k = 0; i < n_components; i++)
    {
        MoleculeLayoutGraph& component = *components[i];

        if (component._n_fixed > 0)
            continue;

        // Component shifting
        row = k / col_count;
        col = k % col_count;

        x_min = 1.0E+20f;
        x_max = -1.0E+20f;
        y_min = 1.0E+20f;
        y_max = -1.0E+20f;

        for (j = component.vertexBegin(); j < component.vertexEnd(); j = component.vertexNext(j))
        {
            const Vec2f& pos = component.getPos(j);

            if (pos.x < x_min)
                x_min = pos.x;
            if (pos.x > x_max)
                x_max = pos.x;
            if (pos.y < y_min)
                y_min = pos.y;
            if (pos.y > y_max)
                y_max = pos.y;
        }

        if (col == 0 && row > 0)
        {
            y_start += max_height + 2 * bond_length;
            max_height = 0.f;
        }

        if (col > 0)
            dx = x_start - x_min + 2 * bond_length;
        else
            dx = -x_min;

        dy = y_start - y_min;

        for (j = component.vertexBegin(); j < component.vertexEnd(); j = component.vertexNext(j))
            _layout_vertices[component.getVertexExtIdx(j)].pos.sum(component.getPos(j), Vec2f(dx, dy));

        x_start = x_max + dx;

        if (y_max - y_min > max_height)
            max_height = y_max - y_min;

        k++;
    }
}

void MoleculeLayoutGraphSmart::_layoutSingleComponent(BaseMolecule& molecule, bool respect_existing, const Filter* filter, float bond_length)
{
    QS_DEF(Array<Vec2f>, src_layout);
    QS_DEF(Array<int>, molecule_edge_mapping);

    int i;

    molecule_edge_mapping.clear_resize(molecule.edgeEnd());

    for (i = 0; i < molecule_edge_mapping.size(); i++)
        molecule_edge_mapping[i] = i;

    _molecule = &molecule;
    _molecule_edge_mapping = molecule_edge_mapping.ptr();

    src_layout.clear_resize(vertexEnd());

    if (respect_existing)
        for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
            src_layout[i] = getPos(i);
    else
        src_layout.zerofill();

    if (filter != 0)
    {
        _fixed_vertices.resize(vertexEnd());
        _fixed_vertices.zerofill();

        for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
            if (!filter->valid(i))
            {
                _fixed_vertices[i] = 1;
                _n_fixed++;
            }
    }

    if (vertexCount() > 1)
    {
        _calcMorganCodes();
        _assignAbsoluteCoordinates(bond_length);
    }
    _assignFinalCoordinates(bond_length, src_layout);
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