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

#include "base_cpp/tlscont.h"
#include "graph/graph.h"
#include "graph/path_enumerator.h"

using namespace indigo;

PathEnumerator::PathEnumerator (Graph &graph, int begin, int end) :
_graph(graph),
_begin(begin),
_end(end)
{
   max_length = graph.vertexCount();
   context = 0;
   cb_check_vertex = 0;
   cb_check_edge = 0;
   cb_handle_path = 0;
}

PathEnumerator::~PathEnumerator ()
{
}

void PathEnumerator::process ()
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
   vertices.push(_begin);
   flags[_begin] = 1;
   visited_vertices.clear_resize(_graph.getVertex(_begin).neiEnd());
   visited_vertices.zerofill();

   // DFS all paths from given vertex
   while (vertices.size() > 0)
   {
      const Vertex &v_vertex = _graph.getVertex(vertices.top());
      bool no_push = true;
      
      if (vertices.size() < max_length)
      {
         for (int i = v_vertex.neiBegin(); i != v_vertex.neiEnd(); i = v_vertex.neiNext(i))
         {
            if (visited_vertices[cur_start_idx + i])
               continue;
            
            int u = v_vertex.neiVertex(i);
            int e = v_vertex.neiEdge(i);
            
            if (flags[u])
               continue;
            if (cb_check_vertex != 0 && !cb_check_vertex(_graph, u, context))
               continue;
            if (cb_check_edge != 0 && !cb_check_edge(_graph, e, context))
               continue;
            
            bool path = (vertices.size() > 1) && u == _end;
            
            vertices.push(u);
            edges.push(e);
            if (path)
            {
               if (cb_handle_path != 0 && !cb_handle_path(_graph, vertices, edges, context))
                  return;
               
               edges.pop();
               vertices.pop();
            }
            else
            {
               visited_vertices[cur_start_idx + i] = 1;
               flags[u] = 1;
               cur_start_idx += v_vertex.neiEnd();

               const Vertex &u_vertex = _graph.getVertex(u);
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
}
