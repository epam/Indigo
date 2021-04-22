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

#include "graph/aux_path_finder.h"
#include "graph/simple_cycle_basis.h"

using namespace indigo;

AuxPathFinder::AuxPathFinder(AuxiliaryGraph& graph, int max_size) : _graph(graph)
{
    _queue.setLength(max_size);
    _prev.clear_resize(max_size);
}

bool AuxPathFinder::find(Array<int>& vertices, Array<int>& edges, int u, int v)
{
    // init
    _queue.clear();
    _prev.fffill();
    vertices.clear();
    edges.clear();

    // push initial vertex
    _queue.push(v);
    _prev[v] = u;

    while (!_queue.isEmpty())
    {
        // pop vertex
        int w = _queue.pop();

        const Vertex& vert = _graph.getVertexAndBuild(w);
        for (int i = vert.neiBegin(); i < vert.neiEnd(); i = vert.neiNext(i))
        {
            int n = vert.neiVertex(i);
            if (_prev[n] >= 0)
                continue; // vertex is already done

            if (n == u)
            {
                // shortest path found. mark and return
                _prev[u] = w;
                for (int j = u; j != v; j = _prev[j])
                {
                    vertices.push(j);
                    edges.push(_graph.findEdgeIndex(j, _prev[j]));
                }
                vertices.push(v);
                return true;
            }

            _queue.push(n);
            _prev[n] = w;
        }
    }
    return false;
}
