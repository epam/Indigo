/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "graph/simple_cycle_basis.h"
#include "graph/cycle_basis.h"
#include "graph/biconnected_decomposer.h"
#include "base_cpp/tlscont.h"

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
   for (int i = 0; i < comp_num; ++i) {
      bic_dec.getComponent(i, filter);
      
      // create subgraph and store mapping
      subgraph.makeSubgraph(graph, filter, &mapping_out, 0);

      if (subgraph.edgeCount() > 1) {
         SimpleCycleBasis simple_cycle(subgraph);

         // create cycles for each biconnected component
         simple_cycle.create();

         // create new cycle consider the mapping
         for (int k = 0; k < simple_cycle.getCyclesCount(); ++k) {
            const Array<int>& cycle = simple_cycle.getCycle(k);
            Array<int>& new_cycle = _cycles.push();
            for(int j = 0; j < cycle.size(); ++j) {
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

bool CycleBasis::containsVertex(int vertex) const {
   return _cycleVertices.find(vertex);
}
