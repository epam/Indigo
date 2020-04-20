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

#include "graph/graph_subchain_enumerator.h"

using namespace indigo;

CP_DEF(GraphSubchainEnumerator);

GraphSubchainEnumerator::GraphSubchainEnumerator(Graph& graph, int min_edges, int max_edges, int mode)
    : _graph(graph), _max_edges(max_edges), _min_edges(min_edges), _mode(mode), CP_INIT, TL_CP_GET(_vertex_states), TL_CP_GET(_chain_vertices),
      TL_CP_GET(_chain_edges)
{
    context = 0;
    cb_handle_chain = 0;

    if (_mode == MODE_NO_DUPLICATE_VERTICES)
    {
        _vertex_states.clear_resize(_graph.vertexEnd());
        _vertex_states.zerofill();
    }

    _chain_vertices.clear();
    _chain_edges.clear();
}

GraphSubchainEnumerator::~GraphSubchainEnumerator()
{
}

void GraphSubchainEnumerator::processChains()
{
    int i;

    for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
    {
        _chain_vertices.push(i);

        if (_mode == MODE_NO_DUPLICATE_VERTICES)
            _vertex_states[i] = 1;

        _DFS(i);

        if (_mode == MODE_NO_DUPLICATE_VERTICES)
            _vertex_states[i] = 0;

        _chain_vertices.pop();
    }
}

void GraphSubchainEnumerator::_DFS(int from)
{
    int i;

    const Vertex& v = _graph.getVertex(from);

    for (i = v.neiBegin(); i != v.neiEnd(); i = v.neiNext(i))
    {
        int nei_idx = v.neiVertex(i);
        int edge_idx = v.neiEdge(i);

        if (_mode == MODE_NO_DUPLICATE_VERTICES)
        {
            if (_vertex_states[nei_idx] == 1)
                continue;

            _vertex_states[nei_idx] = 1;
        }
        else if (_mode == MODE_NO_BACKTURNS)
        {
            if (_chain_edges.size() > 0 && _chain_edges.top() == edge_idx)
                continue;
        }

        _chain_vertices.push(nei_idx);
        _chain_edges.push(edge_idx);

        if (_chain_edges.size() >= _min_edges && _chain_edges.size() <= _max_edges)
            if (cb_handle_chain != 0)
                cb_handle_chain(_graph, _chain_edges.size(), _chain_vertices.ptr(), _chain_edges.ptr(), context);

        if (_chain_edges.size() < _max_edges)
            _DFS(nei_idx);

        _chain_edges.pop();
        _chain_vertices.pop();

        if (_mode == MODE_NO_DUPLICATE_VERTICES)
            _vertex_states[nei_idx] = 0;
    }
}
