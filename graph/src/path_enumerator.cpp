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

#include "base_cpp/tlscont.h"
#include "graph/graph.h"
#include "graph/path_enumerator.h"

PathEnumerator::PathEnumerator (Graph &graph, int begin, int end) :
_graph(graph),
_begin(begin),
_end(end),
TL_CP_GET(vertices),
TL_CP_GET(edges),
TL_CP_GET(flags)
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
   flags.clear_resize(_graph.vertexEnd());
   flags.zerofill();

   vertices.clear();
   edges.clear();

   vertices.push(_begin);
   flags[_begin] = 1;

	_pathFinder();
}    

bool PathEnumerator::_pathFinder ()
{
   if (vertices.size() > max_length)
      return true;

   const Vertex &v_vertex = _graph.getVertex(vertices.top());

   for (int i = v_vertex.neiBegin(); i != v_vertex.neiEnd(); i = v_vertex.neiNext(i))
   {
      int u = v_vertex.neiVertex(i);
      int e = v_vertex.neiEdge(i);
      bool path = (vertices.size() > 2) && u == _end;

      _graph.getEdge(e);

      if (flags[u] != 0)
         continue;
      if (cb_check_vertex != 0 && !cb_check_vertex(_graph, u, context))
         continue;
      if (cb_check_edge != 0 && !cb_check_edge(_graph, e, context))
         continue;

      vertices.push(u);
      edges.push(e);
      if (path)
      {
         if (cb_handle_path != 0 && !cb_handle_path(_graph, vertices, edges, context))
            return false;
      }
      else
      {
         flags[u] = 1;
         if (!_pathFinder())
            return false;
         flags[u] = 0;
      }
      edges.pop();
      vertices.pop();
   }

   return true;
}
