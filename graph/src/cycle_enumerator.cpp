/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#include "graph/cycle_enumerator.h"
#include "graph/spanning_tree.h"

using namespace indigo;

CycleEnumerator::CycleEnumerator (Graph &graph) :
_graph(graph)
{
   max_length = graph.vertexCount();
   context = 0;
   cb_check_vertex = 0;
   cb_handle_cycle = 0;
   vfilter = 0;
}

CycleEnumerator::~CycleEnumerator ()
{
}

bool CycleEnumerator::process ()
{
   int i;
   QS_DEF(Array<int>, vertices);
   QS_DEF(Array<int>, edges);
   QS_DEF(Array<int>, flags);

   flags.clear_resize(_graph.vertexEnd());
   flags.zerofill();

   vertices.clear();
   edges.clear();

   SpanningTree spt(_graph, vfilter);

   for (i = 0; i < spt.getEdgesNum(); i++)
   {
      const SpanningTree::ExtEdge &ext_edge = spt.getExtEdge(i);

      int v = ext_edge.ext_beg_idx;
      int w = ext_edge.ext_end_idx;

		if (cb_check_vertex == 0 ||
          (cb_check_vertex(_graph, v, context) && cb_check_vertex(_graph, w, context)))
      {
         while (vertices.size() > 0)
         {
            int v = vertices.pop();
            flags[v] = 0;
         }

         vertices.push(v);
         vertices.push(w);
         flags[v] = 1;
         flags[w] = 1;

         edges.push(ext_edge.ext_edge_idx);

		   if (!_pathFinder(spt, vertices, edges, flags))
            return true;

         vertices.pop();
         vertices.pop();
         edges.pop();
         flags[v] = 0;
         flags[w] = 0;
      }

      spt.addEdge(ext_edge.beg_idx, ext_edge.end_idx, ext_edge.ext_edge_idx);
   }

   return false;
}    

bool CycleEnumerator::_pathFinder (const SpanningTree &spt, Array<int> &vertices,
                                   Array<int> &edges, Array<int> &flags)
{
   if (vertices.size() > max_length)
      return true;

   const Vertex &v_vertex = spt.getVertexFromExtIdx(vertices.top());

   for (int i = v_vertex.neiBegin(); i != v_vertex.neiEnd(); i = v_vertex.neiNext(i))
   {
      int u = spt.getExtVertexIndex(v_vertex.neiVertex(i));
      int e = spt.getExtEdgeIndex(v_vertex.neiEdge(i));
      int cycle = (vertices.size() > 2) && u == vertices[0];

      _graph.getEdge(e);
      if (!cycle)
      {
         if (flags[u] != 0)
            continue;
         if (cb_check_vertex != 0 && !cb_check_vertex(_graph, u, context))
            continue;
      }

      if (cycle)
      {
         edges.push(e);
         if (cb_handle_cycle != 0 && !cb_handle_cycle(_graph, vertices, edges, context))
            return false;
         edges.pop();
      }
      else
      {
         vertices.push(u);
         edges.push(e);
         flags[u] = 1;
         if (!_pathFinder(spt, vertices, edges, flags))
            return false;
         flags[u] = 0;
         vertices.pop();
         edges.pop();
      }
   }

   return true;
}
