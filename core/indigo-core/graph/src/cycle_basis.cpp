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

#include "graph/cycle_basis.h"
#include "base_cpp/tlscont.h"
#include "graph/biconnected_decomposer.h"
#include "graph/simple_cycle_basis.h"

using namespace indigo;

void CycleBasis::create(const Graph& graph)
{
    QS_DEF(Array<int>, mapping_out);

    // using biconnected decomposer since components will contain smallest cycles

    BiconnectedDecomposer bic_dec(graph);
    int comp_num = bic_dec.decompose();

    _cycles.clear();
    _cycleVertices.clear();

    QS_DEF(Graph, subgraph);

    Filter filter;
    for (int i = 0; i < comp_num; ++i)
    {
        bic_dec.getComponent(i, filter);

        // create subgraph and store mapping
        subgraph.makeSubgraph(graph, filter, &mapping_out, 0);

        if (subgraph.edgeCount() > 1)
        {
            SimpleCycleBasis simple_cycle(subgraph);

            // create cycles for each biconnected component
            simple_cycle.create();

            // create new cycle consider the mapping
            for (int k = 0; k < simple_cycle.getCyclesCount(); ++k)
            {
                const Array<int>& cycle = simple_cycle.getCycle(k);
                Array<int>& new_cycle = _cycles.push();
                for (int j = 0; j < cycle.size(); ++j)
                {
                    // cycle is edge list so we have to covert from subgraph edge list to graph edge list
                    int source = subgraph.getEdge(cycle[j]).beg;
                    int target = subgraph.getEdge(cycle[j]).end;
                    int edge_idx = graph.findEdgeIndex(mapping_out[source], mapping_out[target]);
                    _cycleVertices.find_or_insert(mapping_out[source]);
                    _cycleVertices.find_or_insert(mapping_out[target]);
                    new_cycle.push(edge_idx);
                }
            }
        }
    }
}

bool CycleBasis::containsVertex(int vertex) const
{
    return _cycleVertices.find(vertex);
}
