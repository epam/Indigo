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

#include "graph/graph_fast_access.h"

#include "graph/graph.h"

using namespace indigo;

void GraphFastAccess::setGraph(Graph& g)
{
    _g = &g;

    _vertices.clear();

    _vertices_nei.resize(g.vertexEnd());
    _vertices_nei.fffill();

    _nei_vertices_data.clear();
    _nei_edges_data.clear();
}

int* GraphFastAccess::prepareVertices(int& count)
{
    count = _vertices.size();
    if (count != 0)
        return _vertices.ptr();

    for (int v = _g->vertexBegin(); v != _g->vertexEnd(); v = _g->vertexNext(v))
        _vertices.push(v);

    count = _vertices.size();
    return _vertices.ptr();
}

int GraphFastAccess::getVertex(int idx)
{
    return _vertices.ptr()[idx];
}

int GraphFastAccess::vertexCount()
{
    return _vertices.size();
}

// Prepare both nei vertices and edges list. Runs faster then
// preparing them one by one.
void GraphFastAccess::prepareVertexNeiVerticesAndEdges(int v)
{
    if (_vertices_nei[v].v_begin != -1 && _vertices_nei[v].e_begin != -1)
        return;

    _vertices_nei[v].v_begin = _nei_vertices_data.size();
    _vertices_nei[v].e_begin = _nei_edges_data.size();

    const Vertex& vertex = _g->getVertex(v);
    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        _nei_vertices_data.push(vertex.neiVertex(i));
        _nei_edges_data.push(vertex.neiEdge(i));
    }

    _vertices_nei[v].v_count = _nei_vertices_data.size() - _vertices_nei[v].v_begin;
    _vertices_nei[v].e_count = _nei_edges_data.size() - _vertices_nei[v].e_begin;
}

// Returns nei vertices and nei edges for specified vertex
// Numeration is coherent
int* GraphFastAccess::getVertexNeiVertices(int v, int& count)
{
    int offset = prepareVertexNeiVertices(v, count);
    return _nei_vertices_data.ptr() + offset;
}

int* GraphFastAccess::getVertexNeiEdges(int v, int& count)
{
    if (_vertices_nei[v].e_begin == -1)
        prepareVertexNeiVerticesAndEdges(v);

    count = _vertices_nei[v].e_count;
    return _nei_edges_data.ptr() + _vertices_nei[v].e_begin;
}

int GraphFastAccess::findEdgeIndex(int v1, int v2)
{
    int count;
    int* vertices = getVertexNeiVertices(v1, count);
    int* edges = getVertexNeiEdges(v1, count);
    for (int i = 0; i < count; i++)
        if (vertices[i] == v2)
            return edges[i];
    return -1;
}

void GraphFastAccess::prepareEdges()
{
    _edges.clear_resize(_g->edgeEnd());
    for (int e = _g->edgeBegin(); e != _g->edgeEnd(); e = _g->edgeNext(e))
        _edges[e] = _g->getEdge(e);
}

const Edge& GraphFastAccess::getEdge(int e)
{
    return _edges[e];
}

const Edge* GraphFastAccess::getEdges()
{
    return _edges.ptr();
}

int GraphFastAccess::prepareVertexNeiVertices(int v, int& count)
{
    getVertexNeiEdges(v, count);
    return _vertices_nei.ptr()[v].v_begin;
}

int GraphFastAccess::getVertexNeiVertiex(int v_id, int index)
{
    return _nei_vertices_data.ptr()[v_id + index];
}
