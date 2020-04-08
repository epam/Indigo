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

#include "graph/cycle_enumerator.h"
#include "graph/spanning_tree.h"

using namespace indigo;

CycleEnumerator::CycleEnumerator(Graph& graph) : _graph(graph)
{
    min_length = 0;
    max_length = graph.vertexCount();
    context = 0;
    cb_check_vertex = 0;
    cb_handle_cycle = 0;
    vfilter = 0;
}

CycleEnumerator::~CycleEnumerator()
{
}

bool CycleEnumerator::process()
{
    int i;
    SpanningTree spt(_graph, vfilter);

    for (i = 0; i < spt.getEdgesNum(); i++)
    {
        const SpanningTree::ExtEdge& ext_edge = spt.getExtEdge(i);

        int v = ext_edge.ext_beg_idx;
        int w = ext_edge.ext_end_idx;

        if (cb_check_vertex == 0 || (cb_check_vertex(_graph, v, context) && cb_check_vertex(_graph, w, context)))
        {
            if (!_pathFinder(spt, v, w, ext_edge.ext_edge_idx))
                return true;
        }

        spt.addEdge(ext_edge.beg_idx, ext_edge.end_idx, ext_edge.ext_edge_idx);
    }

    return false;
}

bool CycleEnumerator::_pathFinder(const SpanningTree& spt, int ext_v1, int ext_v2, int ext_e)
{

    QS_DEF(Array<int>, vertices);
    QS_DEF(Array<int>, edges);
    QS_DEF(Array<int>, flags);
    QS_DEF(Array<int>, visited_vertices);
    int cur_start_idx = 0;

    vertices.clear();
    edges.clear();
    flags.clear_resize(_graph.vertexEnd());
    flags.zerofill();

    vertices.push(ext_v1);
    vertices.push(ext_v2);
    flags[ext_v1] = 1;
    flags[ext_v2] = 1;
    edges.push(ext_e);
    visited_vertices.clear_resize(spt.getVertexFromExtIdx(ext_v2).neiEnd());
    visited_vertices.zerofill();

    // DFS all cycles with given edge
    while (vertices.size() > 1)
    {
        const Vertex& v_vertex = spt.getVertexFromExtIdx(vertices.top());
        bool no_push = true;

        if (vertices.size() <= max_length)
        {
            for (int i = v_vertex.neiBegin(); i != v_vertex.neiEnd(); i = v_vertex.neiNext(i))
            {
                if (visited_vertices[cur_start_idx + i])
                    continue;
                visited_vertices[cur_start_idx + i] = 1;

                int u = spt.getExtVertexIndex(v_vertex.neiVertex(i));
                int e = spt.getExtEdgeIndex(v_vertex.neiEdge(i));

                bool cycle = (vertices.size() > 2) && u == vertices[0];

                if (!cycle)
                {
                    if (flags[u])
                        continue;
                    if (cb_check_vertex != 0 && !cb_check_vertex(_graph, u, context))
                        continue;
                }

                if (cycle)
                {
                    if (min_length != 0 && vertices.size() < min_length)
                        continue;
                    edges.push(e);
                    if (cb_handle_cycle != 0 && !cb_handle_cycle(_graph, vertices, edges, context))
                        return false;
                    edges.pop();
                }
                else
                {
                    edges.push(e);
                    vertices.push(u);
                    flags[u] = 1;

                    cur_start_idx += v_vertex.neiEnd();

                    const Vertex& u_vertex = spt.getVertexFromExtIdx(u);
                    visited_vertices.expand(cur_start_idx + u_vertex.neiEnd());
                    memset(&visited_vertices[cur_start_idx], 0, u_vertex.neiEnd() * sizeof(int));

                    no_push = false;
                    break;
                }
            }
        }

        if (no_push)
        {
            if (edges.size() > 0)
                edges.pop();
            flags[vertices.pop()] = 0;
            cur_start_idx -= v_vertex.neiEnd();
        }
    }

    return true;
}
