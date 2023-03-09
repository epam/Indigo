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

#include <stdarg.h>
#include <stdio.h>

#include "base_c/defs.h"
#include "base_cpp/tlscont.h"
#include "graph/cycle_basis.h"
#include "graph/graph.h"
#include "graph/graph_decomposer.h"
#include "graph/spanning_tree.h"

using namespace indigo;

NeighborsAuto Vertex::neighbors() const
{
    return NeighborsAuto(*this);
}

int Vertex::findNeiVertex(int idx) const
{
    for (int i = neighbors_list.begin(); i < neighbors_list.end(); i = neighbors_list.next(i))
        if (neighbors_list[i].v == idx)
            return i;

    return -1;
}

int Vertex::findNeiEdge(int idx) const
{
    for (int i = neighbors_list.begin(); i < neighbors_list.end(); i = neighbors_list.next(i))
        if (neighbors_list[i].e == idx)
            return i;

    return -1;
}

IMPL_ERROR(Graph, "graph");

Graph::Graph()
{
    _vertices = new ObjPool<Vertex>();
    _neighbors_pool = new Pool<List<VertexEdge>::Elem>();
    _sssr_pool = 0;
    _components_valid = false;
}

Graph::~Graph()
{
    delete _vertices;
    delete _neighbors_pool;
    if (_sssr_pool != 0)
    {
        _sssr_vertices.clear();
        _sssr_edges.clear();
        delete _sssr_pool;
    }
}

int Graph::addVertex()
{
    changed();
    return _vertices->add(*_neighbors_pool);
}

int Graph::findEdgeIndex(int beg, int end) const
{
    const Vertex& vbeg = getVertex(beg);

    for (int i = vbeg.neiBegin(); i != vbeg.neiEnd(); i = vbeg.neiNext(i))
        if (vbeg.neiVertex(i) == end)
            return vbeg.neiEdge(i);

    return -1;
}

bool Graph::haveEdge(int beg, int end) const
{
    return findEdgeIndex(beg, end) != -1;
}

bool Graph::hasEdge(int idx) const
{
    return _edges.hasElement(idx);
}

bool Graph::hasVertex(int idx) const
{
    return _vertices->hasElement(idx);
}

int Graph::getEdgeEnd(int beg, int edge) const
{
    const Edge& e = getEdge(edge);
    if (e.beg == beg)
        return e.end;
    if (e.end == beg)
        return e.beg;
    return -1;
}

int Graph::addEdge(int beg, int end)
{
    if (beg == end)
        throw Error("can't have loop-edge on vertex %d", beg);

    if (findEdgeIndex(beg, end) != -1)
        throw Error("already have edge between vertices %d and %d", beg, end);

    int edge_idx = _edges.add();

    Vertex& vbeg = _vertices->at(beg);
    Vertex& vend = _vertices->at(end);

    int ve1_idx = vbeg.neighbors_list.add();
    int ve2_idx = vend.neighbors_list.add();

    VertexEdge& ve1 = vbeg.neighbors_list[ve1_idx];
    VertexEdge& ve2 = vend.neighbors_list[ve2_idx];

    ve1.v = end;
    ve2.v = beg;
    ve1.e = edge_idx;
    ve2.e = edge_idx;

    _edges[edge_idx].beg = beg;
    _edges[edge_idx].end = end;

    _topology_valid = false;
    _sssr_valid = false;
    _components_valid = false;
    changed();
    return edge_idx;
}

void Graph::swapEdgeEnds(int edge_idx)
{

    std::swap(_edges[edge_idx].beg, _edges[edge_idx].end);
}

void Graph::removeEdge(int idx)
{
    Edge edge = _edges[idx];

    Vertex& beg = _vertices->at(edge.beg);
    Vertex& end = _vertices->at(edge.end);

    _edges.remove(idx);

    beg.neighbors_list.remove(beg.findNeiEdge(idx));
    end.neighbors_list.remove(end.findNeiEdge(idx));

    _topology_valid = false;
    _sssr_valid = false;
    _components_valid = false;
    changed();
}

void Graph::removeAllEdges()
{
    for (int i = _vertices->begin(); i != _vertices->end(); i = _vertices->next(i))
        _vertices->at(i).neighbors_list.clear();

    _edges.clear();
    _topology_valid = false;
    _sssr_valid = false;
    _components_valid = false;
    changed();
}

void Graph::removeVertex(int idx)
{
    QS_DEF(Array<int>, edges);

    const Vertex& vertex = getVertex(idx);

    int i;

    edges.clear();

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
        edges.push(vertex.neiEdge(i));

    for (i = 0; i < edges.size(); i++)
        removeEdge(edges[i]);

    _vertices->remove(idx);

    _topology_valid = false;
    _sssr_valid = false;
    _components_valid = false;
    changed();
}

const Vertex& Graph::getVertex(int idx) const
{
    return _vertices->at(idx);
}

const Edge& Graph::getEdge(int idx) const
{
    return _edges[idx];
}

bool Graph::isConnected(Graph& graph)
{
    return graph.countComponents() == 1;
}

struct BfsState
{
    int state;
    int prev;
    int edge;
};

/* Finds path, writes edge indices into path_out. Returns false if no path. */
bool Graph::findPath(int from, int where, Array<int>& path_out) const
{
    path_out.clear();

    QS_DEF(Array<int>, queue);
    QS_DEF(Array<BfsState>, states);

    queue.clear_resize(_vertices->size());
    states.clear_resize(_vertices->end());

    states.zerofill();

    int top = 1, bottom = 0;
    bool have_path = false;

    states[where].state = 1;
    queue[0] = where;

    while (top != bottom)
    {
        if (queue[bottom] == from)
        {
            have_path = true;
            break;
        }

        const Vertex& vertex = getVertex(queue[bottom]);

        states[queue[bottom]].state = 2;

        for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
        {
            int other = vertex.neiVertex(i);

            if (states[other].state == 0)
            {
                queue[top++] = other;
                states[other].state = 1;
                states[other].prev = queue[bottom];
                states[other].edge = vertex.neiEdge(i);
            }
        }
        bottom++;
    }

    if (have_path)
    {
        while (from != where)
        {
            path_out.push(states[from].edge);
            from = states[from].prev;
        }
        return true;
    }

    return false;
}

VerticesAuto Graph::vertices()
{
    return VerticesAuto(*this);
}

EdgesAuto Graph::edges()
{
    return EdgesAuto(*this);
}

void Graph::changed()
{
}

void Graph::clear()
{
    _vertices->clear();
    _edges.clear();
    _topology_valid = false;
    _sssr_valid = false;
    _components_valid = false;
    changed();
}

bool Graph::isChain_AssumingConnected(const Graph& graph)
{
    // ensure it is a tree
    if (graph.vertexCount() - graph.edgeCount() != 1)
        return false;

    for (int i = graph.vertexBegin(); i < graph.vertexEnd(); i = graph.vertexNext(i))
        if (graph.getVertex(i).degree() > 2)
            return false;

    return true;
}

bool Graph::isTree(Graph& graph)
{
    if (!Graph::isConnected(graph))
        return false;

    if (graph.vertexCount() != graph.edgeCount() + 1)
        return false;

    return true;
}

void Graph::filterVertices(const Graph& graph, const int* filter, int filter_type, int filter_value, Array<int>& result)
{
    result.clear();

    for (int i = graph.vertexBegin(); i != graph.vertexEnd(); i = graph.vertexNext(i))
    {
        if (filter != 0)
        {
            if (filter_type == FILTER_EQ && filter_value != filter[i])
                continue;
            if (filter_type == FILTER_NEQ && filter_value == filter[i])
                continue;
        }
        result.push(i);
    }
}

void Graph::filterEdges(const Graph& graph, const int* filter, int filter_type, int filter_value, Array<int>& result)
{
    result.clear();

    for (int i = graph.edgeBegin(); i != graph.edgeEnd(); i = graph.edgeNext(i))
    {
        if (filter != 0)
        {
            if (filter_type == FILTER_EQ && filter_value != filter[i])
                continue;
            if (filter_type == FILTER_NEQ && filter_value == filter[i])
                continue;
        }
        result.push(i);
    }
}

void Graph::_mergeWithSubgraph(const Graph& other, const Array<int>& vertices, const Array<int>* edges, Array<int>* vertex_mapping, Array<int>* edge_mapping)
{
    QS_DEF(Array<int>, tmp_mapping);
    int i;

    if (vertex_mapping == 0)
        vertex_mapping = &tmp_mapping;

    vertex_mapping->clear_resize(other.vertexEnd());
    vertex_mapping->fffill();

    if (edge_mapping != 0)
    {
        edge_mapping->clear_resize(other.edgeEnd());
        edge_mapping->fffill();
    }

    for (i = 0; i < vertices.size(); i++)
    {
        int idx = vertices[i];

        if (vertex_mapping->at(idx) != -1)
            throw Error("makeSubgraph(): repeated vertex #%d", idx);

        vertex_mapping->at(idx) = addVertex();
    }

    if (edges != 0)
    {
        for (i = 0; i != edges->size(); i++)
        {
            const Edge& edge = other.getEdge(edges->at(i));

            int beg = vertex_mapping->at(edge.beg);
            int end = vertex_mapping->at(edge.end);

            if (beg == -1 || end == -1)
                throw Error("_mergeWithSubgraph: edge %d maps to (%d, %d)", edges->at(i), beg, end);

            int idx = addEdge(beg, end);

            if (edge_mapping != 0)
                edge_mapping->at(edges->at(i)) = idx;
        }
    }
    else
        for (i = other.edgeBegin(); i < other.edgeEnd(); i = other.edgeNext(i))
        {
            const Edge& edge = other.getEdge(i);
            int beg = vertex_mapping->at(edge.beg);
            int end = vertex_mapping->at(edge.end);

            if (beg != -1 && end != -1)
            {
                int idx = addEdge(beg, end);
                if (edge_mapping != 0)
                    edge_mapping->at(i) = idx;
            }
        }
}

void Graph::buildEdgeMapping(const Graph& other, Array<int>* mapping, Array<int>* edge_mapping)
{
    for (int i = other.edgeBegin(); i < other.edgeEnd(); i = other.edgeNext(i))
    {
        const Edge& edge = other.getEdge(i);
        int beg = mapping->at(edge.beg);
        int end = mapping->at(edge.end);

        if (beg != -1 && end != -1)
        {
            int idx = findEdgeIndex(beg, end);
            if (edge_mapping != 0)
                edge_mapping->at(i) = idx;
        }
    }
}

void Graph::mergeWith(const Graph& other, Array<int>* mapping)
{
    QS_DEF(Array<int>, vertices);
    int i;

    vertices.clear();

    for (i = other.vertexBegin(); i != other.vertexEnd(); i = other.vertexNext(i))
        vertices.push(i);

    _mergeWithSubgraph(other, vertices, 0, mapping, 0);
}

void Graph::makeSubgraph(const Graph& other, const Array<int>& vertices, Array<int>* vertex_mapping)
{
    clear();
    _mergeWithSubgraph(other, vertices, 0, vertex_mapping, 0);
}

void Graph::makeSubgraph(const Graph& other, const Array<int>& vertices, Array<int>* vertex_mapping, const Array<int>* edges, Array<int>* edge_mapping)
{
    clear();
    _mergeWithSubgraph(other, vertices, edges, vertex_mapping, edge_mapping);
}

void Graph::makeSubgraph(const Graph& other, const Filter& filter, Array<int>* mapping_out, Array<int>* inv_mapping)
{
    QS_DEF(Array<int>, vertices);

    if (mapping_out == 0)
        mapping_out = &vertices;

    filter.collectGraphVertices(other, *mapping_out);

    makeSubgraph(other, *mapping_out, inv_mapping);
}

void Graph::cloneGraph(const Graph& other, Array<int>* mapping)
{
    QS_DEF(Array<int>, vertices);
    vertices.clear();

    for (int i = other.vertexBegin(); i < other.vertexEnd(); i = other.vertexNext(i))
        vertices.push(i);
    makeSubgraph(other, vertices, mapping);
}

void Graph::makeEdgeSubgraph(const Graph& other, const Array<int>& vertices, const Array<int>& edges, Array<int>* v_mapping, Array<int>* e_mapping)
{
    QS_DEF(Array<int>, tmp_mapping);
    int i;

    if (v_mapping == 0)
        v_mapping = &tmp_mapping;

    v_mapping->clear_resize(other.vertexEnd());

    for (i = other.vertexBegin(); i < other.vertexEnd(); i = other.vertexNext(i))
        v_mapping->at(i) = -1;

    if (e_mapping != 0)
        e_mapping->clear_resize(other.edgeEnd());

    clear();

    for (i = 0; i < vertices.size(); i++)
    {
        int idx = vertices[i];

        if (v_mapping->at(idx) != -1)
            throw Error("makeEdgeSubgraph(): repeated vertex #%d", idx);

        v_mapping->at(idx) = addVertex();
    }

    for (i = 0; i < edges.size(); i++)
    {
        int edge_idx = edges[i];
        const Edge& edge = other.getEdge(edge_idx);
        int beg = v_mapping->at(edge.beg);
        int end = v_mapping->at(edge.end);

        int new_edge_idx = addEdge(beg, end);
        if (e_mapping != 0)
            e_mapping->at(edge_idx) = new_edge_idx;
    }
}

int Graph::findMappedEdge(const Graph& graph, const Graph& mapped_graph, int edge_idx, const int* mapping)
{
    const Edge& edge = graph.getEdge(edge_idx);

    int beg = mapping[edge.beg];
    int end = mapping[edge.end];

    if (beg == -1 || end == -1)
        return -1;

    return mapped_graph.findEdgeIndex(beg, end);
}

int Graph::getEdgeTopology(int idx)
{
    if (!_topology_valid)
        _calculateTopology();

    return _topology[idx];
}

bool Graph::vertexInRing(int idx)
{
    const Vertex& vertex = getVertex(idx);
    int i;

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
        if (getEdgeTopology(vertex.neiEdge(i)) == TOPOLOGY_RING)
            return true;

    return false;
}

void Graph::_calculateTopology()
{
    SpanningTree spt(*this, 0);
    int i;

    _topology.clear_resize(_edges.end());

    for (i = _edges.begin(); i != _edges.end(); i = _edges.next(i))
        _topology[i] = TOPOLOGY_CHAIN;

    spt.markAllEdgesInCycles(_topology.ptr(), TOPOLOGY_RING);
    _topology_valid = true;
}

void Graph::setEdgeTopology(int idx, int topology)
{
    _topology.expandFill(idx + 1, -1);
    _topology[idx] = topology;
}

void Graph::validateEdgeTopologies()
{
    _topology_valid = true;
}

int Graph::vertexCountSSSR(int idx)
{
    if (!_sssr_valid)
        _calculateSSSR();

    return _v_sssr_count[idx];
}

int Graph::vertexSmallestRingSize(int idx)
{
    if (!_sssr_valid)
        _calculateSSSR();

    return _v_smallest_ring_size[idx];
}

int Graph::edgeSmallestRingSize(int idx)
{
    if (!_sssr_valid)
        _calculateSSSR();

    return _e_smallest_ring_size[idx];
}

void Graph::_calculateSSSRInit()
{
    _v_smallest_ring_size.clear_resize(vertexEnd());
    _e_smallest_ring_size.clear_resize(edgeEnd());
    _v_sssr_count.clear_resize(vertexEnd());

    _v_smallest_ring_size.fffill();
    _e_smallest_ring_size.fffill();
    _v_sssr_count.zerofill();

    if (_sssr_pool == 0)
        _sssr_pool = new Pool<List<int>::Elem>();

    _sssr_vertices.clear();
    _sssr_edges.clear();
}

void Graph::_calculateSSSRByCycleBasis(CycleBasis& basis)
{
    _calculateSSSRInit();

    for (int i = 0; i < basis.getCyclesCount(); i++)
    {
        const Array<int>& cycle = basis.getCycle(i);

        List<int>& vertices = _sssr_vertices.push(*_sssr_pool);
        List<int>& edges = _sssr_edges.push(*_sssr_pool);

        _calculateSSSRAddEdgesAndVertices(cycle, edges, vertices);

        for (int j = vertices.begin(); j != vertices.end(); j = vertices.next(j))
        {
            int idx = vertices[j];

            if (_v_smallest_ring_size[idx] == -1 || _v_smallest_ring_size[idx] > cycle.size())
                _v_smallest_ring_size[idx] = cycle.size();

            _v_sssr_count[idx]++;
        }

        for (int j = edges.begin(); j != edges.end(); j = edges.next(j))
        {
            int idx = edges[j];

            if (_e_smallest_ring_size[idx] == -1 || _e_smallest_ring_size[idx] > cycle.size())
                _e_smallest_ring_size[idx] = cycle.size();
        }
    }

    for (int i = 0; i < _v_smallest_ring_size.size(); i++)
        if (_v_smallest_ring_size[i] == -1)
            _v_smallest_ring_size[i] = 0;

    for (int i = 0; i < _e_smallest_ring_size.size(); i++)
        if (_e_smallest_ring_size[i] == -1)
            _e_smallest_ring_size[i] = 0;

    _sssr_valid = true;
}

void Graph::_calculateSSSR()
{
    // Note: function was split into smaller functions to reduce stack usage
    QS_DEF(CycleBasis, basis);
    basis.create(*this);
    _calculateSSSRByCycleBasis(basis);
}

void Graph::_calculateComponents(const std::list<std::unordered_set<int>> external_neighbors)
{
    GraphDecomposer decomposer(*this);
    int i;

    decomposer.decompose(NULL, NULL, &external_neighbors);

    _component_numbers.clear_resize(vertexEnd());

    for (i = vertexBegin(); i != vertexEnd(); i = vertexNext(i))
        _component_numbers[i] = decomposer.getComponent(i);

    _components_count = decomposer.getComponentsCount();

    _component_vcount.clear_resize(_components_count);
    _component_ecount.clear_resize(_components_count);

    for (i = 0; i < _components_count; i++)
    {
        _component_vcount[i] = decomposer.getComponentVerticesCount(i);
        _component_ecount[i] = decomposer.getComponentEdgesCount(i);
    }

    _components_valid = true;
}

int Graph::vertexComponent(int v_idx)
{
    if (!_components_valid)
        _calculateComponents();

    return _component_numbers[v_idx];
}

int Graph::countComponents(const std::list<std::unordered_set<int>>& external_neighbors)
{
    if (!_components_valid)
        _calculateComponents(external_neighbors);

    return _components_count;
}

int Graph::countComponentEdges(int comp_idx, const std::list<std::unordered_set<int>>& external_neighbors)
{
    if (!_components_valid)
        _calculateComponents(external_neighbors);

    return _component_ecount[comp_idx];
}

int Graph::countComponentVertices(int comp_idx, const std::list<std::unordered_set<int>>& external_neighbors)
{
    if (!_components_valid)
        _calculateComponents(external_neighbors);

    return _component_vcount[comp_idx];
}

int Graph::countComponents()
{
    if (!_components_valid)
        _calculateComponents();

    return _components_count;
}

int Graph::countComponentEdges(int comp_idx)
{
    if (!_components_valid)
        _calculateComponents();

    return _component_ecount[comp_idx];
}

int Graph::countComponentVertices(int comp_idx)
{
    if (!_components_valid)
        _calculateComponents();

    return _component_vcount[comp_idx];
}

const Array<int>& Graph::getDecomposition()
{
    if (!_components_valid)
        _calculateComponents();

    return _component_numbers;
}

List<int>& Graph::sssrEdges(int idx)
{
    if (!_sssr_valid)
        _calculateSSSR();
    return _sssr_edges[idx];
}

List<int>& Graph::sssrVertices(int idx)
{
    if (!_sssr_valid)
        _calculateSSSR();
    return _sssr_vertices[idx];
}

int Graph::sssrCount()
{
    if (!_sssr_valid)
        _calculateSSSR();
    return _sssr_vertices.size();
}

void Graph::_cloneGraph_KeepIndices(const Graph& other)
{
    if (vertexCount() > 0 || edgeCount() > 0)
        throw Error("can not _clone_KeepIndices into a non-empty graph");

    int i, j, i_prev;
    int max_vertex_idx = -1;
    int max_edge_idx = -1;

    for (i = other.vertexBegin(); i != other.vertexEnd(); i = other.vertexNext(i))
        if (max_vertex_idx < i)
            max_vertex_idx = i;

    for (i = other.edgeBegin(); i != other.edgeEnd(); i = other.edgeNext(i))
        if (max_edge_idx < i)
            max_edge_idx = i;

    for (i = 0; i <= max_vertex_idx; i++)
        if (addVertex() != i)
            throw Error("_clone_KeepIndices: unexpected vertex index");

    i_prev = -1;

    for (i = other.vertexBegin(); i != other.vertexEnd(); i = other.vertexNext(i))
    {
        for (j = i_prev + 1; j < i; j++)
            removeVertex(j);
        i_prev = i;
    }

    if (vertexCount() != other.vertexCount())
        throw Error("_clone_KeepIndices: internal");

    for (i = 0; i <= max_edge_idx; i++)
        if (_edges.add() != i)
            throw Error("_clone_KeepIndices: unexpected edge index");

    i_prev = -1;

    for (i = other.edgeBegin(); i != other.edgeEnd(); i = other.edgeNext(i))
    {
        for (j = i_prev + 1; j < i; j++)
            _edges.remove(j);

        _edges[i].beg = other._edges[i].beg;
        _edges[i].end = other._edges[i].end;

        Vertex& vbeg = _vertices->at(_edges[i].beg);
        Vertex& vend = _vertices->at(_edges[i].end);

        int ve1_idx = vbeg.neighbors_list.add();
        int ve2_idx = vend.neighbors_list.add();

        VertexEdge& ve1 = vbeg.neighbors_list[ve1_idx];
        VertexEdge& ve2 = vend.neighbors_list[ve2_idx];

        ve1.v = _edges[i].end;
        ve2.v = _edges[i].beg;
        ve1.e = i;
        ve2.e = i;

        i_prev = i;
    }

    if (edgeCount() != other.edgeCount())
        throw Error("_clone_KeepIndices: internal");

    _topology_valid = false;
    _sssr_valid = false;
    _components_valid = false;
}

void Graph::_calculateSSSRAddEdgesAndVertices(const Array<int>& cycle, List<int>& edges, List<int>& vertices)
{
    int prev_beg = -1;
    int prev_end = -1;

    for (int j = 0; j < cycle.size(); j++)
    {
        const Edge& edge = getEdge(cycle[j]);

        edges.add(cycle[j]);

        if (j != cycle.size() - 1)
        {
            if (edge.beg != prev_beg && edge.beg != prev_end)
                vertices.add(edge.beg);
            if (edge.end != prev_beg && edge.end != prev_end)
                vertices.add(edge.end);
        }
        prev_beg = edge.beg;
        prev_end = edge.end;
    }
}

bool Graph::isTerminalVertex(int v_idx) const
{
    return getVertex(v_idx).degree() == 1;
}

bool Graph::isTerminalEdge(int e_idx) const
{
    const auto& edge = getEdge(e_idx);
    return isTerminalVertex(edge.beg) || isTerminalVertex(edge.end);
}
