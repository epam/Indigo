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

#include "graph/path_enumerator.h"
#include "base_cpp/cancellation_handler.h"
#include "base_cpp/tlscont.h"
#include "graph/graph.h"

using namespace indigo;

IMPL_TIMEOUT_EXCEPTION(PathEnumerator, "path enumerator");

PathEnumerator::PathEnumerator(Graph& graph, int begin, int end) : _graph(graph), _begin(begin), _end(end)
{
    max_length = graph.vertexCount();
    context = 0;
    cb_check_vertex = 0;
    cb_check_edge = 0;
    cb_handle_path = 0;
}

PathEnumerator::~PathEnumerator()
{
}

void PathEnumerator::process()
{
    QS_DEF(Array<int>, vertices);
    QS_DEF(Array<int>, edges);
    QS_DEF(Array<bool>, flags);
    QS_DEF(Array<int>, index);
    QS_DEF(Array<bool>, can_achieve_to_end);

    vertices.clear();
    edges.clear();
    flags.clear_resize(_graph.vertexEnd());
    flags.zerofill();
    vertices.push(_begin);
    edges.push(-1); // fictitious edge
    flags[_begin] = true;
    index.clear_resize(_graph.vertexEnd());
    index.zerofill();
    can_achieve_to_end.clear_resize(_graph.vertexEnd());
    can_achieve_to_end.zerofill();
    can_achieve_to_end[_end] = true;
    index[_begin] = _graph.getVertex(_begin).neiBegin();

    while (vertices.size() > 0)
    {
        int current_v_number = vertices.top();
        const Vertex& current_v = _graph.getVertex(current_v_number);
        int& current_index = index[current_v_number];
        if (current_v_number != _end && current_index != current_v.neiEnd())
        {
            int u = current_v.neiVertex(current_index);
            int e = current_v.neiEdge(current_index);
            if (can_achieve_to_end[u])
            {
            }
            {
                if (!flags[u] && (cb_check_vertex == 0 || cb_check_vertex(_graph, u, context)))
                {

                    vertices.push(u);
                    edges.push(e);
                    flags[u] = true;
                    index[u] = _graph.getVertex(u).neiBegin();
                }
            }

            current_index = current_v.neiNext(current_index);
        }
        else
        {
            if (can_achieve_to_end[vertices.top()] && vertices.size() > 1)
            {
                if (vertices.size() > 2 || vertices[1] != _end)
                    cb_check_edge(_graph, edges.top(), context);
                vertices.top() = -1;
                vertices.pop();
                can_achieve_to_end[vertices.top()] = true;
            }
            else
            {
                vertices.top() = -1;
                vertices.pop();
            }
            edges.pop();
        }
    }
}