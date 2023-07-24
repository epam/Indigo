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

#include "graph/simple_cycle_basis.h"
#include "base_cpp/list.h"
#include "base_cpp/red_black.h"
#include "base_cpp/tlscont.h"
#include "graph/aux_path_finder.h"
#include "graph/graph.h"
#include "graph/shortest_path_finder.h"
#include "graph/spanning_tree.h"

using namespace indigo;

SimpleCycleBasis::SimpleCycleBasis(const Graph& graph) : _graph(graph), _isMinimized(false)
{
}

void SimpleCycleBasis::create()
{
    QS_DEF(Array<int>, vert_mapping);

    QS_DEF(ObjArray<Array<int>>, subgraph_cycles);

    subgraph_cycles.clear();

    Graph subgraph;

    subgraph.cloneGraph(_graph, &vert_mapping);

    _prepareSubgraph(subgraph);

    // The cycles just created are already minimal, so we can start minimizing at startIndex
    int startIndex = _cycles.size();

    // Now we perform a breadth first traversal and build a fundamental tree base
    // ("Kirchhoff base") of the remaining subgraph

    int current_vertex = subgraph.vertexBegin();

    // We build a spanning tree as a directed graph to easily find the parent of a
    // vertex in the tree. This means however that we have to create new Edge objects
    // for the tree and can't just use the Edge objects of the graph, since the
    // the edge in the graph might have a wrong or no direction.

    Graph spanning_tree;

    QS_DEF(RedBlackSet<int>, visited_edges);
    visited_edges.clear();

    // FIFO for the BFS
    QS_DEF(Array<int>, vertex_queue);
    vertex_queue.clear();

    // currentVertex is the root of the spanning tree

    int new_vertex = spanning_tree.addVertex();

    vertices_spanning_tree.emplace(current_vertex, new_vertex);
    spanning_tree_vertices.emplace(new_vertex, current_vertex);

    vertex_queue.push(current_vertex);

    // We need to remember the tree edges so we can add them at once to the
    // index list for the incidence matrix

    QS_DEF(Array<int>, tree_edges);
    tree_edges.clear();

    while (vertex_queue.size() > 0)
    {
        current_vertex = vertex_queue.pop();

        const Vertex& c_vertex = subgraph.getVertex(current_vertex);

        for (int i = c_vertex.neiBegin(); i < c_vertex.neiEnd(); i = c_vertex.neiNext(i))
        {
            // find a neighbour vertex of the current vertex
            int edge = c_vertex.neiEdge(i);

            if (!visited_edges.find(edge))
            {

                // mark edge as visited
                visited_edges.insert(edge);

                int next_vertex = subgraph.getEdge(edge).findOtherEnd(current_vertex);

                if (vertices_spanning_tree.find(next_vertex) == vertices_spanning_tree.end())
                {
                    // tree edge

                    tree_edges.push(edge);

                    int new_vertex = spanning_tree.addVertex();
                    vertices_spanning_tree.emplace(next_vertex, new_vertex);
                    spanning_tree_vertices.emplace(new_vertex, next_vertex);

                    // create a new (directed) Edge object (as explained above)
                    spanning_tree.addEdge(vertices_spanning_tree.at(current_vertex), new_vertex);

                    // add the next vertex to the BFS-FIFO
                    vertex_queue.push(next_vertex);
                }
                else
                {
                    // non-tree edge

                    // This edge defines a cycle together with the edges of the spanning tree
                    // along the path to the root of the tree. We create a new cycle containing
                    // these edges (not the tree edges, but the corresponding edges in the graph)

                    Array<int>& edges_of_cycle = subgraph_cycles.push();

                    // follow the path to the root of the tree

                    int vertex = current_vertex;

                    // get parent of vertex

                    int parent_vertex = -1;

                    while (_getParentVertex(spanning_tree, vertex, parent_vertex))
                    {
                        // add the corresponding edge to the cycle
                        edges_of_cycle.push(subgraph.findEdgeIndex(vertex, parent_vertex));
                        // go up the tree
                        vertex = parent_vertex;
                    }

                    // do the same thing for nextVertex
                    vertex = next_vertex;

                    while (_getParentVertex(spanning_tree, vertex, parent_vertex))
                    {
                        // add the corresponding edge to the cycle
                        edges_of_cycle.push(subgraph.findEdgeIndex(vertex, parent_vertex));
                        // go up the tree
                        vertex = parent_vertex;
                    }

                    // finally, add the non-tree edge to the cycle
                    edges_of_cycle.push(edge);

                    // add the edge to the index list for the incidence matrix
                    _edgeList.push(edge);
                }
            }
        }
    }

    // Add all the tree edges to the index list for the incidence matrix
    for (int i = 0; i < tree_edges.size(); ++i)
    {
        _edgeList.push(tree_edges[i]);
    }

    //   edgeIndexMap = createEdgeIndexMap(edgeList);

    // Now the index list is ordered: first the non-tree edges, then the tree edge.
    // Moreover, since the cycles and the corresponding non-tree edge have been added
    // to their lists in the same order, the incidence matrix is in upper triangular form.

    // Now we can minimize the cycles created from the tree base

    for (int i = 0; i < subgraph_cycles.size(); ++i)
    {
        Array<int>& cycle_edges = subgraph_cycles[i];
        Array<int>& new_cycle_edges = _cycles.push();
        for (int j = 0; j < cycle_edges.size(); ++j)
        {
            int edge_s = subgraph.getEdge(cycle_edges[j]).beg;
            int edge_t = subgraph.getEdge(cycle_edges[j]).end;
            new_cycle_edges.push(_graph.findEdgeIndex(vert_mapping[edge_s], vert_mapping[edge_t]));
        }
    }
    _createEdgeIndexMap();

    _minimize(startIndex);
}

bool SimpleCycleBasis::_getParentVertex(const Graph& graph, int vertex, int& parent_vertex)
{
    parent_vertex = -1;

    const Vertex& gv = graph.getVertex(vertices_spanning_tree.at(vertex));
    for (int i = gv.neiBegin(); i < gv.neiEnd(); i = gv.neiNext(i))
    {
        const Edge& edge = graph.getEdge(gv.neiEdge(i));
        if (edge.end == vertices_spanning_tree.at(vertex))
            parent_vertex = spanning_tree_vertices.at(edge.beg);
    }

    if (parent_vertex == -1)
    {
        return false;
    }
    return true;
}

void SimpleCycleBasis::_minimize(int startIndex)
{

    if (_isMinimized)
        return;

    // Implementation of "Algorithm 1" from [BGdV04]

    QS_DEF(ObjArray<Array<bool>>, a);
    a.clear();

    _getCycleEdgeIncidenceMatrix(a);

    for (int cur_cycle = startIndex; cur_cycle < _cycles.size(); ++cur_cycle)
    {
        // "Subroutine 2"

        // Construct kernel vector u
        Array<bool> u;
        u.resize(_edgeList.size());

        constructKernelVector(u, a, cur_cycle);

        // Construct auxiliary graph gu
        AuxiliaryGraph gu(_graph, u, _edgeIndexMap);

        AuxPathFinder path_finder(gu, _graph.vertexEnd() * 2);

        QS_DEF(ObjArray<Array<int>>, all_new_cycles);
        all_new_cycles.clear();

        for (int v = _graph.vertexBegin(); v < _graph.vertexEnd(); v = _graph.vertexNext(v))
        {

            // check if the vertex is incident to an edge with u[edge] == 1
            bool shouldSearchCycle = false;

            const Vertex& vertex = _graph.getVertex(v);

            for (int e = vertex.neiBegin(); e < vertex.neiEnd(); e = vertex.neiNext(e))
            {

                int edge = vertex.neiEdge(e);

                int edge_index = _getEdgeIndex(edge);
                if (u[edge_index])
                {
                    shouldSearchCycle = true;
                    break;
                }
            }

            if (shouldSearchCycle)
            {

                int auxVertex0 = gu.auxVertex0(v);
                int auxVertex1 = gu.auxVertex1(v);

                Array<int>& edges_of_new_cycle = all_new_cycles.push();
                Array<int> path_vertices;

                // Search for shortest path

                path_finder.find(path_vertices, edges_of_new_cycle, auxVertex0, auxVertex1);
            }
        }

        Array<int>& current_cycle = _cycles.at(cur_cycle);
        int shortest_cycle_size = current_cycle.size();

        int shortest_cycle = -1;
        for (int i = 0; i < all_new_cycles.size(); ++i)
        {
            int cycle_size = all_new_cycles[i].size();
            if (cycle_size > 0 && cycle_size < shortest_cycle_size)
            {
                shortest_cycle = i;
                shortest_cycle_size = cycle_size;
            }
        }

        if (shortest_cycle != -1)
        {
            current_cycle.clear();
            Array<int>& sh_cycle = all_new_cycles[shortest_cycle];
            for (int i = 0; i < sh_cycle.size(); ++i)
            {
                current_cycle.push(gu.edge(sh_cycle[i]));
            }
        }

        // insert the new cycle into the matrix
        for (int j = 1; j < _edgeList.size(); j++)
        {
            a[cur_cycle][j] = (current_cycle.find(_edgeList.at(j)) != -1);
        }

        // perform gaussian elimination on the inserted row
        for (int j = 0; j < cur_cycle; j++)
        {
            if (a[cur_cycle][j])
            {
                for (int k = 0; k < _edgeList.size(); k++)
                {
                    a[cur_cycle][k] = (a[cur_cycle][k] != a[j][k]);
                }
            }
        }
    }

    _isMinimized = true;
}

void SimpleCycleBasis::_getCycleEdgeIncidenceMatrix(ObjArray<Array<bool>>& result)
{
    for (int i = 0; i < _cycles.size(); ++i)
    {
        Array<bool>& new_array = result.push();
        new_array.resize(_edgeList.size());
        Array<int>& cycle = _cycles[i];
        for (int j = 0; j < _edgeList.size(); ++j)
        {
            result[i][j] = (cycle.find(_edgeList[j]) != -1);
        }
    }
}

void SimpleCycleBasis::constructKernelVector(Array<bool>& u, ObjArray<Array<bool>>& a, int i)
{
    for (int j = 0; j < u.size(); ++j)
    {
        u[j] = false;
    }

    // Construct kernel vector u by setting u[i] = true ...
    u[i] = true;

    // ... u[j] = 0 (false) for j > i (by initialization)...

    // ... and solving A u = 0

    for (int j = i - 1; j >= 0; j--)
    {
        u[j] = false;
        for (int k = i; k > j; k--)
        {
            u[j] = (u[j] != (a[j][k] && u[k]));
        }
    }
}

void SimpleCycleBasis::_createEdgeIndexMap()
{
    _edgeIndexMap.clear();
    for (int i = 0; i < _edgeList.size(); ++i)
    {
        _edgeIndexMap.emplace(_edgeList[i], i);
    }
}

int SimpleCycleBasis::_getEdgeIndex(int edge) const
{
    return _edgeIndexMap.at(edge);
}

void SimpleCycleBasis::_prepareSubgraph(Graph& subgraph)
{
    QS_DEF(Array<int>, path_vertices);
    path_vertices.clear();
    QS_DEF(RedBlackSet<int>, selected_edges);
    selected_edges.clear();
    QS_DEF(RedBlackSet<int>, remaining_edges);
    remaining_edges.clear();
    for (int i = subgraph.edgeBegin(); i < subgraph.edgeEnd(); i = subgraph.edgeNext(i))
    {
        remaining_edges.insert(i);
    }

    ShortestPathFinder path_finder(subgraph);

    while (remaining_edges.size() > 0)
    {

        int edge = remaining_edges.begin();

        int source = subgraph.getEdge(edge).beg;
        int target = subgraph.getEdge(edge).end;

        subgraph.removeEdge(edge);

        Array<int>& path_edges = _cycles.push();

        path_finder.find(path_vertices, path_edges, source, target);

        path_edges.push(edge);

        subgraph.addEdge(source, target);

        selected_edges.insert(edge);

        _edgeList.push(edge);

        for (int i = 0; i < path_edges.size(); ++i)
        {
            remaining_edges.remove_if_exists(path_edges[i]);
        }
    }

    for (int i = selected_edges.begin(); i < selected_edges.end(); i = selected_edges.next(i))
    {
        subgraph.removeEdge(selected_edges.key(i));
    }
}

int AuxiliaryGraph::_findOrCreateVertex(std::map<int, int>& vertexMap, int vertex)
{
    const auto it = vertexMap.find(vertex);
    if (it != vertexMap.end())
    {
        return it->second;
    }

    int newVertex = addVertex();
    vertexMap.emplace(vertex, newVertex);
    _auxVertexMap.emplace(newVertex, vertex);

    return newVertex;
}

int AuxiliaryGraph::auxVertex0(int vertex)
{
    return _findOrCreateVertex(_vertexMap0, vertex);
}

int AuxiliaryGraph::auxVertex1(int vertex)
{
    return _findOrCreateVertex(_vertexMap1, vertex);
}

const Vertex& AuxiliaryGraph::getVertexAndBuild(int auxVertex)
{
    const Vertex& vertex = _graph.getVertex(_auxVertexMap.at(auxVertex));

    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int edge = vertex.neiEdge(i);

        int vertex1 = _graph.getEdge(edge).beg;
        int vertex2 = _graph.getEdge(edge).end;

        int edge_idx = _edgeIndexMap.at(edge);

        if (_u[edge_idx])
        {
            int vertex1u = auxVertex0(vertex1);
            int vertex2u = auxVertex1(vertex2);
            int ex_aux_edge = findEdgeIndex(vertex1u, vertex2u);
            if (ex_aux_edge == -1)
            {
                int auxEdge = addEdge(vertex1u, vertex2u);
                _auxEdgeMap.emplace(auxEdge, edge);
            }

            vertex1u = auxVertex1(vertex1);
            vertex2u = auxVertex0(vertex2);

            ex_aux_edge = findEdgeIndex(vertex1u, vertex2u);
            if (ex_aux_edge == -1)
            {
                int auxEdge = addEdge(vertex1u, vertex2u);
                _auxEdgeMap.emplace(auxEdge, edge);
            }
        }
        else
        {
            int vertex1u = auxVertex0(vertex1);
            int vertex2u = auxVertex0(vertex2);
            int ex_aux_edge = findEdgeIndex(vertex1u, vertex2u);
            if (ex_aux_edge == -1)
            {
                int auxEdge = addEdge(vertex1u, vertex2u);
                _auxEdgeMap.emplace(auxEdge, edge);
            }

            vertex1u = auxVertex1(vertex1);
            vertex2u = auxVertex1(vertex2);
            ex_aux_edge = findEdgeIndex(vertex1u, vertex2u);
            if (ex_aux_edge == -1)
            {
                int auxEdge = addEdge(vertex1u, vertex2u);
                _auxEdgeMap.emplace(auxEdge, edge);
            }
        }
    }

    return getVertex(auxVertex);
}

int AuxiliaryGraph::edge(int auxEdge)
{
    return _auxEdgeMap.at(auxEdge);
}
