/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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
#include "base_cpp/cancellation_handler.h"
#include "graph/graph.h"
#include "graph/path_enumerator.h"

using namespace indigo;

IMPL_TIMEOUT_EXCEPTION(PathEnumerator, "path enumerator");

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

void PathEnumerator::process1() {
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

   while (vertices.size() > 0) {
      int current_v_number = vertices.top();
      const Vertex &current_v = _graph.getVertex(current_v_number);
      int& current_index = index[current_v_number];
      if (current_v_number != _end && current_index != current_v.neiEnd()) {
         int u = current_v.neiVertex(current_index);
         int e = current_v.neiEdge(current_index);
         if (can_achieve_to_end[u]) {

            //cb_check_edge_norm(_graph, e, context);
            //can_achieve_to_end[current_v_number] = true;
         }
         {
            if (!flags[u] &&
               (cb_check_vertex == 0 || cb_check_vertex(_graph, u, context)) &&
               (cb_check_edge == 0 || cb_check_edge(_graph, current_v.neiEdge(current_index), context))) {

               vertices.push(u);
               edges.push(e);
               flags[u] = true;
               index[u] = _graph.getVertex(u).neiBegin();
            }

         }

         current_index = current_v.neiNext(current_index);
      }
      else {
         if (can_achieve_to_end[vertices.top()] && vertices.size() > 1) {
            if (vertices.size() > 2 || vertices[1] != _end)
               cb_check_edge_norm(_graph, edges.top(), context);
            vertices.top() = -1;
            vertices.pop();
            can_achieve_to_end[vertices.top()] = true;
         }
         else {
            vertices.top() = -1;
            vertices.pop();
         }
         edges.pop();
      }
   }
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

   // Cancellation handler variables
   int iteration = 0;
   CancellationHandler *cancellation_handler = getCancellationHandler();

   // DFS all paths from given vertex
   while (vertices.size() > 0)
   {
      // Check cancellation each 1000th iteration
      if (cancellation_handler != NULL && iteration % 1000 == 0)
      {
         if (cancellation_handler->isCancelled())
            throw TimeoutException("%s", cancellation_handler->cancelledRequestMessage());
      }
      iteration++;

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
