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

#include "graph/graph_decomposer.h"
#include "graph/filter.h"
#include "graph/graph.h"

using namespace indigo;

IMPL_ERROR(GraphDecomposer, "Graph decomposer");

CP_DEF(GraphDecomposer);

GraphDecomposer::GraphDecomposer(const Graph& graph)
    : _graph(graph), CP_INIT, TL_CP_GET(_component_ids), TL_CP_GET(_component_vertices_count), TL_CP_GET(_component_edges_count)
{
    _component_vertices_count.clear();
    _component_edges_count.clear();
}

GraphDecomposer::~GraphDecomposer()
{
}

int GraphDecomposer::decompose(const Filter* filter, const Filter* edge_filter, const std::list<std::unordered_set<int>>* ext_neighbours)
{
    if (_graph.vertexCount() < 1)
    {
        n_comp = 0;
        return 0;
    }

    QS_DEF(Array<int>, queue);

    _component_ids.clear_resize(_graph.vertexEnd());
    _component_ids.fffill();
    queue.clear_resize(_graph.vertexEnd());

    n_comp = 0;
    int vertex_idx;

    while (1)
    {
        for (vertex_idx = _graph.vertexBegin(); vertex_idx != _graph.vertexEnd(); vertex_idx = _graph.vertexNext(vertex_idx))
        {
            if (filter != 0 && !filter->valid(vertex_idx))
                continue;

            if (_component_ids[vertex_idx] == -1)
                break;
        }

        if (vertex_idx == _graph.vertexEnd())
            break;

        // BFS
        int top = 1, bottom = 0;

        queue[0] = vertex_idx;
        while (top != bottom) // while queue not empty
        {
            auto v_bottom_id = queue[bottom];
            const Vertex& vertex = _graph.getVertex(v_bottom_id);

            _component_vertices_count.expandFill(n_comp + 1, 0);
            _component_edges_count.expandFill(n_comp + 1, 0);

            _component_ids[v_bottom_id] = n_comp;
            _component_vertices_count[n_comp]++;

            for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
            {
                int other = vertex.neiVertex(i);

                if (filter != 0 && !filter->valid(other))
                    continue;
                if (edge_filter != 0 && !edge_filter->valid(vertex.neiEdge(i)))
                    continue;

                if (_component_ids[other] == -1) // if other unused
                {
                    queue[top++] = other;       // queue.push(other)
                    _component_ids[other] = -2; // mark as in_queue
                }

                if (_component_ids[other] == -2)
                    _component_edges_count[n_comp]++;
            }

            if (ext_neighbours)
            {
                for (const auto& neighbors_group : *ext_neighbours)
                {
                    if (neighbors_group.find(v_bottom_id) != neighbors_group.end())
                    {
                        for (auto other : neighbors_group)
                        {
                            if (other != v_bottom_id)
                            {
                                if (filter != 0 && !filter->valid(other))
                                    continue;
                                if (edge_filter != 0 && !edge_filter->valid(other))
                                    continue;

                                if (_component_ids[other] == -1)
                                {
                                    queue[top++] = other;
                                    _component_ids[other] = -2;
                                }
                                if (_component_ids[other] == -2)
                                    _component_edges_count[n_comp]++;
                            }
                        }
                    }
                }
            }
            bottom++; // queue.pop()
        }

        n_comp++;
    }

    return n_comp;
}

const Array<int>& GraphDecomposer::getDecomposition() const
{
    return _component_ids;
}

int GraphDecomposer::getComponent(int vertex) const
{
    return _component_ids[vertex];
}

int GraphDecomposer::getComponentsCount() const
{
    return n_comp;
}

int GraphDecomposer::getComponentVerticesCount(int component) const
{
    return _component_vertices_count[component];
}

int GraphDecomposer::getComponentEdgesCount(int component) const
{
    return _component_edges_count[component];
}
