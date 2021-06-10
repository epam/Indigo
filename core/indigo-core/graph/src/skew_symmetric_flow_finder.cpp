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

#include "graph/skew_symmetric_flow_finder.h"
#include "graph/skew_symmetric_network.h"

using namespace indigo;

IMPL_ERROR(SkewSymmetricFlowFinder, "SkewSymmetricFlowFinder");

CP_DEF(SkewSymmetricFlowFinder);

SkewSymmetricFlowFinder::SkewSymmetricFlowFinder(const SkewSymmetricNetwork& network)
    : CP_INIT, TL_CP_GET(_arc_values), TL_CP_GET(_arc_sym), TL_CP_GET(_edge_used_dir), TL_CP_GET(_vertex_is_used), _network(network)
{
    _init();
}

void SkewSymmetricFlowFinder::process()
{
    QS_DEF(Array<int>, path);
    while (_findAugmentatingPath(path))
        _increaseFlowByPath(path);
}

int SkewSymmetricFlowFinder::getArcValue(int arc) const
{
    return _arc_values[arc];
}

void SkewSymmetricFlowFinder::_init()
{
    int n_edges = _network.g().edgeEnd();
    _arc_sym.clear_resize(n_edges);
    _arc_values.clear_resize(n_edges);
    _edge_used_dir.clear_resize(n_edges);

    for (int i = 0; i < n_edges; i++)
    {
        _arc_sym[i] = 0;
        _arc_values[i] = 0;
        _edge_used_dir[i] = 0;
    }

    for (int e = _network.g().edgeBegin(); e != _network.g().edgeEnd(); e = _network.g().edgeNext(e))
    {
        _arc_sym[e] = _network.getSymmetricArc(e);
    }

    _vertex_is_used.clear_resize(_network.g().vertexEnd());
    for (int i = 0; i < _vertex_is_used.size(); i++)
        _vertex_is_used[i] = 0;

    _network_sink = _network.getSymmetricVertex(_network.getSource());
}

bool SkewSymmetricFlowFinder::_findAugmentatingPath(Array<int>& vertices)
{
    for (int i = 0; i < _vertex_is_used.size(); i++)
        _vertex_is_used[i] = 0;
    for (int i = 0; i < _edge_used_dir.size(); i++)
        _edge_used_dir[i] = 0;

    vertices.clear();
    vertices.push(_network.getSource());
    return _findAugmentatingPathRec(vertices);
}

bool SkewSymmetricFlowFinder::_findAugmentatingPathRec(Array<int>& vertices)
{
    // This is trivial implementation that takes exponential time in worth case
    // There is fast polynomial algorithm bud trimming algorithm that
    // will be implemented in future

    int from = vertices.top();
    if (from == _network_sink)
        return true;

    _vertex_is_used[from] = 1;
    const Vertex& v = _network.g().getVertex(from);
    for (int i = v.neiBegin(); i != v.neiEnd(); i = v.neiNext(i))
    {
        int edge = v.neiEdge(i);
        int edge_sym = _network.getSymmetricArc(edge);

        int nei_vertex = v.neiVertex(i);
        if (_vertex_is_used[nei_vertex])
            continue;

        if (_isEdgeAugmentating(edge, from, _edge_used_dir[edge_sym]))
        {
            vertices.push(v.neiVertex(i));

            if (_network.getArcType(edge, from) == ARC_OUT)
                _edge_used_dir[edge] = 1;
            else
                _edge_used_dir[edge] = -1;

            bool ret = _findAugmentatingPathRec(vertices);
            if (ret)
                return true;
            _edge_used_dir[edge] = 0;
            vertices.pop();
        }
    }
    _vertex_is_used[from] = 0;
    return false;
}

int SkewSymmetricFlowFinder::_getResidualCapacity(int edge, int from)
{
    if (_network.getArcType(edge, from) == ARC_OUT)
    {
        // Outgoing arc
        return _arc_values[edge];
    }
    else
    {
        // Incomming arc
        return _network.getArcCapacity(edge) - _arc_values[edge];
    }
}

bool SkewSymmetricFlowFinder::_isEdgeAugmentating(int edge, int from, int sym_used_dir)
{
    int residual_capacity = _getResidualCapacity(edge, from);
    int delta = _network.getArcCapacity(edge) - residual_capacity;

    int edge_dir_cur;
    if (_network.getArcType(edge, from) == ARC_OUT)
        edge_dir_cur = 1;
    else
        edge_dir_cur = -1;

    if (edge_dir_cur * sym_used_dir == -1)
        return 0; // This edge will be changed by 0

    if (sym_used_dir != 0 && delta > 1)
        return true;
    if (sym_used_dir == 0 && delta > 0)
        return true;
    return false;
}

void SkewSymmetricFlowFinder::_increaseFlowByPath(Array<int>& vertices)
{
    int delta = -1;
    // Find path maximum delta
    for (int i = 1; i < vertices.size(); i++)
    {
        int from = vertices[i - 1];
        int to = vertices[i];
        int edge = _network.g().findEdgeIndex(from, to);
        int edge_sym = _network.getSymmetricArc(edge);

        int cur_delta = _network.getArcCapacity(edge) - _getResidualCapacity(edge, from);

        if (_edge_used_dir[edge_sym] != 0)
        {
            if (_edge_used_dir[edge_sym] * _edge_used_dir[edge] == 1)
                cur_delta = cur_delta / 2;
            else
                continue; // Delta for this edge is zero
        }

        if (delta == -1 || delta > cur_delta)
            delta = cur_delta;
    }
    if (delta == 0)
        throw Error("algorithm error: delta should be positive");

    // Update network flow
    for (int i = 1; i < vertices.size(); i++)
    {
        int from = vertices[i - 1];
        int to = vertices[i];
        int edge = _network.g().findEdgeIndex(from, to);
        int edge_sym = _arc_sym[edge];

        int cur_delta = delta;

        if (_edge_used_dir[edge_sym] != 0)
        {
            if (_edge_used_dir[edge_sym] * _edge_used_dir[edge] == -1)
                continue; // Delta for this edge is zero
        }

        if (_edge_used_dir[edge] == -1)
            cur_delta = -cur_delta;

        _arc_values[edge] += cur_delta;
        _arc_values[edge_sym] += cur_delta;
    }

    // For debug purposes
    _dbgCheckConsistency();
}

void SkewSymmetricFlowFinder::_dbgCheckConsistency()
{
    // Check that all arc values are in bounds
    const Graph& g = _network.g();
    for (int e = g.edgeBegin(); e != g.edgeEnd(); e = g.edgeNext(e))
    {
        if (_arc_values[e] < 0 || _arc_values[e] > _network.getArcCapacity(e))
            throw Error("arc values are not in bounds");

        int se = _arc_sym[e];
        if (_arc_values[e] != _arc_values[se])
            throw Error("symmetry arc values are not the same");
    }

    // Check divergence
    int source_div = -1, sink_div = -1;
    for (int v = g.vertexBegin(); v != g.vertexEnd(); v = g.vertexNext(v))
    {
        int div = 0;
        const Vertex& vert = g.getVertex(v);
        for (int nei = vert.neiBegin(); nei != vert.neiEnd(); nei = vert.neiNext(nei))
        {
            int e = vert.neiEdge(nei);
            if (_network.getArcType(e, v) == ARC_OUT)
                div += _arc_values[e];
            else
                div -= _arc_values[e];
        }

        if (v == _network_sink)
            sink_div = div;
        else if (v == _network.getSource())
            source_div = div;
        else if (div != 0)
            throw Error("internal vertex divergence must be zero");
    }

    if (source_div + sink_div != 0)
        throw Error("source and sink Divergence must be zero in sum");
}
