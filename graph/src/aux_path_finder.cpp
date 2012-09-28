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

#include "graph/aux_path_finder.h"
#include "graph/simple_cycle_basis.h"

using namespace indigo;

AuxPathFinder::AuxPathFinder (AuxiliaryGraph &graph, int max_size) : _graph(graph)
{
   _queue.setLength(max_size);
   _prev.clear_resize(max_size);
}

bool AuxPathFinder::find (Array<int>& vertices, Array<int>& edges, int u, int v)
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
