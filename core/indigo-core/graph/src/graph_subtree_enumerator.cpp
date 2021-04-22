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

#include "graph/graph_subtree_enumerator.h"

#include "graph/graph_fast_access.h"

using namespace indigo;

CP_DEF(GraphSubtreeEnumerator);

GraphSubtreeEnumerator::GraphSubtreeEnumerator(Graph& graph)
    : _graph(graph), CP_INIT, TL_CP_GET(_front), TL_CP_GET(_vertices), TL_CP_GET(_edges), TL_CP_GET(_v_processed)
{
    min_vertices = 1;
    max_vertices = graph.vertexCount();
    callback = 0;
    context = 0;
    handle_maximal = false;
    maximal_critera_value_callback = 0;
    vfilter = 0;
}

GraphSubtreeEnumerator::~GraphSubtreeEnumerator()
{
}

void GraphSubtreeEnumerator::process()
{
    _edges.clear();
    _vertices.clear();

    _v_processed.clear_resize(_graph.vertexEnd());
    _v_processed.zerofill();

    _front.clear_resize(1);

    _m1.e = _m2.e = -1;
    _m1.v = _m2.v = -1;

    if (vfilter != 0)
        for (int i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
        {
            if (!vfilter->valid(i))
                _v_processed[i] = 1;
        }

    for (int i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
    {
        if (_v_processed[i] == 1)
            continue;

        _vertices.push(i);
        _v_processed[i] = 1;

        int cur_maximal_criteria_value = 0;
        if (handle_maximal && maximal_critera_value_callback != 0)
            cur_maximal_criteria_value = maximal_critera_value_callback(_graph, _vertices, _edges, context);

        _front[0].reset();
        _front[0].v = i;

        _reverseSearch(0, cur_maximal_criteria_value);

        _v_processed[i] = 0;
        _vertices.pop();
    }
}

void GraphSubtreeEnumerator::_reverseSearch(int front_idx, int cur_maximal_criteria_value)
{
    bool maximal_by_criteria = true;
    bool has_supergraph = false;

    int nvertices = _vertices.size();
    if (nvertices < max_vertices)
    {
        // Save front state
        int front_size = _front.size();
        VertexEdgeParent front_prev_value = _front[front_idx];
        _front[front_idx].reset();

        // Update front
        int v = front_prev_value.v;
        const Vertex& vertex = _graph.getVertex(v);
        for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
        {
            int nei_v = vertex.neiVertex(i);
            if (_v_processed[nei_v] == 1)
                continue;

            VertexEdgeParent& added = _front.push();
            added.v = nei_v;
            added.e = vertex.neiEdge(i);
            added.parent = v;
        }
        // Check if we can reuse front_idx front index
        int new_front_size = _front.size();
        if (front_size < new_front_size)
        {
            _front[front_idx] = _front.top();
            _front.pop();
            new_front_size--;
        }

        VertexEdge m1_prev = _m1, m2_prev = _m2;

        for (int i = 0; i < new_front_size; i++)
        {
            VertexEdgeParent cur = _front[i];
            if (cur.v == -1 || _v_processed[cur.v] == 1)
                continue;

            if (nvertices == 1 && cur.v < v)
                continue;

            // Check if edge/vertex follows fCIS maximality rule in the reverse search
            if (cur.parent == _m1.v)
            {
                if (cur.e < _m2.e)
                    continue;
                _m1.v = cur.v;
                _m1.e = cur.e;
            }
            else
            {
                if (cur.e < _m1.e)
                    continue;
                _m2 = _m1;
                _m1.v = cur.v;
                _m1.e = cur.e;
            }

            if (nvertices == 1)
            {
                _m2.e = _m1.e;
                _m2.v = v;
            }

            // Add this edge
            _vertices.push(cur.v);
            _v_processed[cur.v] = 1;
            _edges.push(cur.e);

            int descedant_maximal_criteria_value = 0;
            if (handle_maximal && maximal_critera_value_callback != 0)
                descedant_maximal_criteria_value = maximal_critera_value_callback(_graph, _vertices, _edges, context);

            if (descedant_maximal_criteria_value == cur_maximal_criteria_value)
                maximal_by_criteria = false;

            _reverseSearch(i, descedant_maximal_criteria_value);
            has_supergraph = true;

            _m1 = m1_prev;
            _m2 = m2_prev;

            _edges.pop();
            _v_processed[cur.v] = 0;
            _vertices.pop();
        }

        // Restore front
        _front.resize(front_size);
        _front[front_idx] = front_prev_value;
    }

    if (nvertices >= min_vertices && nvertices <= max_vertices && callback != 0)
    {
        if (handle_maximal)
        {
            if (has_supergraph && !maximal_by_criteria)
                return; // This subgraph isn't maximal
        }

        if (callback)
            callback(_graph, _vertices, _edges, context);
    }
}
