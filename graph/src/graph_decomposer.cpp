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

#include "graph/graph_decomposer.h"
#include "graph/graph.h"
#include "graph/filter.h"

using namespace indigo;

IMPL_ERROR(GraphDecomposer, "Graph decomposer");

GraphDecomposer::GraphDecomposer (const Graph &graph) :
_graph(graph),
TL_CP_GET(_component_ids),
TL_CP_GET(_component_vertices_count),
TL_CP_GET(_component_edges_count)
{
   _component_vertices_count.clear();
   _component_edges_count.clear();
}

GraphDecomposer::~GraphDecomposer ()
{
}

int GraphDecomposer::decompose (const Filter *filter, const Filter *edge_filter)
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
      for (vertex_idx = _graph.vertexBegin(); vertex_idx != _graph.vertexEnd();
           vertex_idx = _graph.vertexNext(vertex_idx))
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
      while (top != bottom)
      {
         const Vertex &vertex = _graph.getVertex(queue[bottom]);

         _component_vertices_count.expandFill(n_comp + 1, 0);
         _component_edges_count.expandFill(n_comp + 1, 0);

         _component_ids[queue[bottom]] = n_comp;
         _component_vertices_count[n_comp]++;

         for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
         {
            int other = vertex.neiVertex(i);

            if (filter != 0 && !filter->valid(other))
               continue;
            if (edge_filter != 0 && !edge_filter->valid(vertex.neiEdge(i)))
               continue;

            if (_component_ids[other] == -1)
            {
               queue[top++] = other;
               _component_ids[other] = -2;
            }

            if (_component_ids[other] == -2)
               _component_edges_count[n_comp]++;
         }
         bottom++;
      }

      n_comp++;
   }

   return n_comp;
}

const Array<int> & GraphDecomposer::getDecomposition () const
{
   return _component_ids;
}

int GraphDecomposer::getComponent (int vertex) const
{
   return _component_ids[vertex];
}

int GraphDecomposer::getComponentsCount () const
{
   return n_comp;
}

int GraphDecomposer::getComponentVerticesCount (int component) const
{
   return _component_vertices_count[component];
}

int GraphDecomposer::getComponentEdgesCount (int component) const
{
   return _component_edges_count[component];
}
