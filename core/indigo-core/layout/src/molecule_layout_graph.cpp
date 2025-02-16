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

#include "layout/molecule_layout_graph.h"
#include "graph/biconnected_decomposer.h"
#include "graph/morgan_code.h"

#include <memory>

using namespace indigo;

IMPL_ERROR(MoleculeLayoutGraph, "layout_graph");

MoleculeLayoutGraph::MoleculeLayoutGraph() : Graph()
{
    _total_morgan_code = 0;
    _first_vertex_idx = -1;
    _n_fixed = 0;
    _molecule = 0;
    _molecule_edge_mapping = 0;
    cancellation = 0;
    _flipped = false;
    preserve_existing_layout = false;
}

MoleculeLayoutGraph::~MoleculeLayoutGraph()
{
}

void MoleculeLayoutGraph::clear()
{
    Graph::clear();
    _total_morgan_code = 0;
    _first_vertex_idx = -1;
    _n_fixed = 0;
    _layout_vertices.clear();
    _layout_edges.clear();
    _fixed_vertices.clear();
}

const LayoutVertex& MoleculeLayoutGraph::getLayoutVertex(int idx) const
{
    return _layout_vertices[idx];
}

const LayoutEdge& MoleculeLayoutGraph::getLayoutEdge(int idx) const
{
    return _layout_edges[idx];
}

bool MoleculeLayoutGraph::isSingleEdge() const
{
    return edgeCount() == 1 && vertexCount() == 2;
}

void MoleculeLayoutGraph::registerLayoutVertex(int idx, const LayoutVertex& vertex)
{
    _layout_vertices.expand(idx + 1);
    _layout_vertices[idx] = vertex;
}

void MoleculeLayoutGraph::registerLayoutEdge(int idx, const LayoutEdge& edge)
{
    _layout_edges.expand(idx + 1);
    _layout_edges[idx] = edge;
}

int MoleculeLayoutGraph::addLayoutVertex(int ext_idx, int type)
{
    int new_idx = Graph::addVertex();

    LayoutVertex new_vertex;

    new_vertex.ext_idx = ext_idx;
    new_vertex.type = type;

    registerLayoutVertex(new_idx, new_vertex);

    return new_idx;
}

int MoleculeLayoutGraph::addLayoutEdge(int beg, int end, int ext_idx, int type)
{
    int new_idx = Graph::addEdge(beg, end);

    LayoutEdge new_edge;

    new_edge.ext_idx = ext_idx;
    new_edge.type = type;

    registerLayoutEdge(new_idx, new_edge);

    return new_idx;
}

int MoleculeLayoutGraph::findVertexByExtIdx(int ext_idx) const
{
    for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
        if (getLayoutVertex(i).ext_idx == ext_idx)
            return i;

    return -1;
}

void MoleculeLayoutGraph::cloneLayoutGraph(MoleculeLayoutGraph& other, Array<int>* mapping)
{
    QS_DEF(Array<int>, mapping_tmp);

    clear();

    if (mapping == 0)
        mapping = &mapping_tmp;

    cloneGraph(other, mapping);

    LayoutVertex new_vertex;
    LayoutEdge new_edge;

    for (int i = other.vertexBegin(); i < other.vertexEnd(); i = other.vertexNext(i))
    {
        new_vertex = other.getLayoutVertex(i);
        new_vertex.ext_idx = i;

        registerLayoutVertex(mapping->at(i), new_vertex);
    }

    for (int i = other.edgeBegin(); i < other.edgeEnd(); i = other.edgeNext(i))
    {
        const Edge& edge = other.getEdge(i);

        new_edge = other.getLayoutEdge(i);
        new_edge.ext_idx = i;

        registerLayoutEdge(findEdgeIndex(mapping->at(edge.beg), mapping->at(edge.end)), new_edge);
    }
}

void MoleculeLayoutGraph::copyLayoutTo(MoleculeLayoutGraph& other, const Array<int>& mapping) const
{
    for (int i = other.vertexBegin(); i < other.vertexEnd(); i = other.vertexNext(i))
    {
        other._layout_vertices[i].type = _layout_vertices[mapping[i]].type;
        other._layout_vertices[i].pos = _layout_vertices[mapping[i]].pos;
    }

    for (int i = other.edgeBegin(); i < other.edgeEnd(); i = other.edgeNext(i))
    {
        const Edge& edge = other.getEdge(i);
        const Vertex& vert = other.getVertex(mapping[edge.beg]);
        int edge_idx = vert.neiEdge(vert.findNeiVertex(mapping[edge.end]));

        other._layout_edges[i].type = _layout_edges[edge_idx].type;
    }
}

void MoleculeLayoutGraph::makeOnGraph(Graph& graph)
{
    QS_DEF(Array<int>, mapping);

    clear();

    // vertices and edges
    cloneGraph(graph, &mapping);

    LayoutVertex new_vertex;
    LayoutEdge new_edge;

    new_vertex.type = ELEMENT_NOT_DRAWN;
    new_vertex.is_cyclic = false;

    for (int i = graph.vertexBegin(); i < graph.vertexEnd(); i = graph.vertexNext(i))
    {
        new_vertex.ext_idx = i;
        new_vertex.orig_idx = i;
        registerLayoutVertex(mapping[i], new_vertex);
    }

    new_edge.type = ELEMENT_NOT_DRAWN;

    for (int i = graph.edgeBegin(); i < graph.edgeEnd(); i = graph.edgeNext(i))
    {
        const Edge& edge = graph.getEdge(i);
        int idx = findEdgeIndex(mapping[edge.beg], mapping[edge.end]);

        new_edge.ext_idx = i;
        new_edge.orig_idx = i;
        registerLayoutEdge(idx, new_edge);
    }
}

IMPL_ERROR(MoleculeLayoutGraphSimple, "layout_graph");

MoleculeLayoutGraphSimple::MoleculeLayoutGraphSimple() : MoleculeLayoutGraph()
{
}

MoleculeLayoutGraphSimple::~MoleculeLayoutGraphSimple()
{
}

ObjArray<PatternLayout>& MoleculeLayoutGraphSimple::getPatterns()
{
    static LayoutPatternHolder _patternHolder;
    return _patternHolder.getPatterns();
}

MoleculeLayoutGraph* MoleculeLayoutGraphSimple::getInstance()
{
    return new MoleculeLayoutGraphSimple();
}

void MoleculeLayoutGraphSimple::clear()
{
    MoleculeLayoutGraph::clear();
}

void MoleculeLayoutGraphSimple::makeLayoutSubgraph(MoleculeLayoutGraph& graph, Filter& filter)
{
    QS_DEF(Array<int>, vertices);
    QS_DEF(Array<int>, mapping);

    clear();

    filter.collectGraphVertices(graph, vertices);

    makeSubgraph(graph, vertices, &mapping);

    LayoutVertex new_vertex;
    LayoutEdge new_edge;

    new_vertex.is_cyclic = false;

    for (int i = 0; i < vertices.size(); i++)
    {
        new_vertex.ext_idx = vertices[i];
        new_vertex.type = graph._layout_vertices[vertices[i]].type;
        new_vertex.morgan_code = graph._layout_vertices[vertices[i]].morgan_code;
        registerLayoutVertex(mapping[vertices[i]], new_vertex);
    }

    for (int i = edgeBegin(); i < edgeEnd(); i = edgeNext(i))
    {
        const Edge& edge = getEdge(i);
        int ext_idx = graph.findEdgeIndex(vertices[edge.beg], vertices[edge.end]);

        new_edge.ext_idx = ext_idx;
        new_edge.type = graph._layout_edges[ext_idx].type;
        registerLayoutEdge(i, new_edge);
    }
}

void MoleculeLayoutGraph::layout(BaseMolecule& molecule, float bond_length, const Filter* filter, bool respect_existing, std::optional<Vec2f> multiple_distance)
{
    if (molecule.vertexCount() == 0)
        return;

    int n_components = countComponents();

    if (fabs(bond_length) < EPSILON)
        throw Error("zero bond length");

    _molecule = &molecule;
    if (n_components > 1)
        _layoutMultipleComponents(molecule, respect_existing, filter, bond_length, multiple_distance);
    else
        _layoutSingleComponent(molecule, respect_existing, filter, bond_length);
}

void MoleculeLayoutGraph::_calcMorganCodes()
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

void MoleculeLayoutGraph::_makeComponentsTree(BiconnectedDecomposer& decon, PtrArray<MoleculeLayoutGraph>& components, Array<int>& tree)
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

int MoleculeLayoutGraphSimple::_pattern_cmp2(const PatternLayout& p1, int n_v, int n_e, long code)
{
    long diff = code - p1.morganCode();

    if (diff != 0)
        return diff;

    diff = n_v + n_e - p1.vertexCount() - p1.edgeCount();

    if (diff != 0)
        return diff;

    diff = n_v - p1.vertexCount();

    if (diff != 0)
        return diff;

    return n_e - p1.edgeCount();
}

void MoleculeLayoutGraph::_layoutMultipleComponents(BaseMolecule& molecule, bool respect_existing, const Filter* filter, float bond_length,
                                                    std::optional<Vec2f> multiple_distance)
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

        if (!respect_existing || !preserve_existing_layout || component.vertexCount() > component._n_fixed)
        {
            if (component.vertexCount() > 1)
            {
                component._calcMorganCodes();
                component._assignAbsoluteCoordinates(bond_length);
            }
            component._assignFinalCoordinates(bond_length, src_layout);
        }
    }

    if (respect_existing && preserve_existing_layout) // TODO:
    {
        for (int i = 0; i < n_components; i++)
        {
            copyCoordsFromComponent(*components[i]);
        }
    }
    else // Move fixed componets to touch (0,0), layout rest of components in grid
    {
        // position components
        float row_bottom = 0.f; // where to place non-fixed components
        int n_fixed = 0;

        // fixed first
        if (filter != 0)
        {
            bool first_component = true;
            Rect2f fixed_comps_bbox;

            // find left bottom corner and top of fixed components
            for (i = 0; i < n_components; i++)
            {
                MoleculeLayoutGraph& component = *components[i];

                if (component._n_fixed > 0)
                {
                    n_fixed++;
                    Rect2f bbox;
                    component.getBoundingBox(bbox);
                    if (first_component)
                    {
                        fixed_comps_bbox.copy(bbox);
                        first_component = false;
                    }
                    else
                    {
                        fixed_comps_bbox.extend(bbox);
                    }
                }
            }

            // position fixed - shift all rectangle with atoms to point left down corner to (0, 0)
            if (n_fixed > 0)
            {
                Vec2f shift = fixed_comps_bbox.leftBottom();
                shift.negate();
                for (i = 0; i < n_components; i++)
                {
                    MoleculeLayoutGraph& component = *components[i];
                    if (component._n_fixed > 0)
                        copyCoordsFromComponent(component, shift);
                }

                row_bottom += fixed_comps_bbox.height() + (multiple_distance.has_value() ? multiple_distance.value().y : bond_length * 2);
            }
        }

        // layout non-fixed components in square
        int col_count = (int)ceil(sqrt((float)n_components - n_fixed));
        float column_left = 0.f;
        float row_height = 0.f;

        for (i = 0, k = 0; i < n_components; i++)
        {
            MoleculeLayoutGraph& component = *components[i];

            if (component._n_fixed > 0)
                continue;

            int row = k / col_count;
            int col = k % col_count;

            // first column in row - add previous row height+space to row_bottom
            if (col == 0 && row > 0)
            {
                row_bottom += row_height + (multiple_distance.has_value() ? multiple_distance.value().y : bond_length * 2);
                row_height = 0.f;
                column_left = 0.f;
            }
            // Add space between columns
            if (col > 0)
                column_left += (multiple_distance.has_value() ? multiple_distance.value().x : bond_length * 2);

            // Component shifting
            Rect2f bbox;
            component.getBoundingBox(bbox);

            if (multiple_distance.has_value())
            {
                auto bw_size = Vec2f(bbox.width() < bond_length ? bond_length : bbox.width(), bbox.height() < bond_length ? bond_length : bbox.height());
                bbox = Rect2f(bbox.center() - bw_size / 2, bbox.center() + bw_size / 2);
            }

            Vec2f shift = bbox.leftBottom();
            shift.negate();
            shift.add(Vec2f(column_left, row_bottom));

            copyCoordsFromComponent(component, shift);

            column_left += bbox.width();

            row_height = std::max(row_height, bbox.height());

            k++;
        }
    }
}

void MoleculeLayoutGraph::copyCoordsFromComponent(MoleculeLayoutGraph& component, Vec2f shift)
{
    for (int i = component.vertexBegin(); i < component.vertexEnd(); i = component.vertexNext(i))
    {
        _layout_vertices[component.getVertexExtIdx(i)].pos.sum(component.getPos(i), shift);
    }
}

void MoleculeLayoutGraph::_layoutSingleComponent(BaseMolecule& molecule, bool respect_existing, const Filter* filter, float bond_length)
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

    if (!respect_existing || !preserve_existing_layout || vertexCount() > _n_fixed)
    {
        if (vertexCount() > 1)
        {
            _calcMorganCodes();
            _assignAbsoluteCoordinates(bond_length);
        }
        _assignFinalCoordinates(bond_length, src_layout);
    }
}

void MoleculeLayoutGraph::getBoundingBox(Vec2f& left_bottom, Vec2f& right_top) const
{
    bool first = true;
    for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
    {
        const Vec2f& pos = getPos(i);
        if (first)
        {
            left_bottom = right_top = pos;
            first = false;
        }
        else
        {
            left_bottom.min(pos);
            right_top.max(pos);
        }
    }
}

void MoleculeLayoutGraph::getBoundingBox(Rect2f& bbox) const
{
    Vec2f left_bottom;
    Vec2f right_top;
    getBoundingBox(left_bottom, right_top);
    bbox = Rect2f(left_bottom, right_top);
}

#ifdef M_LAYOUT_DEBUG

#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molfile_saver.h"

void MoleculeLayoutGraphSimple::saveDebug()
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

    if (id == 57)
        id = id;

    id++;
}

#endif
