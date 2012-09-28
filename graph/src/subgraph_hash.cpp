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

#include "graph/subgraph_hash.h"
#include "graph/graph.h"

using namespace indigo;

SubgraphHash::SubgraphHash (Graph &g) : _g(g)
{
   context = 0;
   max_iterations = _g.vertexEnd();
   cb_vertex_code = 0;
   cb_edge_code = 0;
   _different_codes_count = 0;
   calc_different_codes_count = false;
}

int SubgraphHash::_getVertexCode (int vertex)
{
   if (cb_vertex_code != 0)
      return cb_vertex_code(_g, vertex, context);
   else
      return 0;
}

int SubgraphHash::_getEdgeCode (int edge)
{
   if (cb_edge_code != 0)
      return cb_edge_code(_g, edge, context);
   else
      return 0;
}

dword SubgraphHash::getHash ()
{
   QS_DEF(Array<int>, vertices);
   QS_DEF(Array<int>, edges);
   int i;

   vertices.clear();
   edges.clear();

   for (i = _g.vertexBegin(); i != _g.vertexEnd(); i = _g.vertexNext(i))
      vertices.push(i);

   for (i = _g.edgeBegin(); i != _g.edgeEnd(); i = _g.edgeNext(i))
      edges.push(i);

   return getHash(vertices, edges);
}

dword SubgraphHash::getHash (const Array<int> &vertices, const Array<int> &edges)
{
   QS_DEF(Array<dword>, codes);
   QS_DEF(Array<dword>, oldcodes);
   int i, iter;

   codes.clear_resize(_g.vertexEnd());
   oldcodes.clear_resize(_g.vertexEnd());

   for (i = 0; i < vertices.size(); i++)
      codes[vertices[i]] = _getVertexCode(vertices[i]);

   for (iter = 0; iter < max_iterations; iter++)
   {
      for (i = 0; i < vertices.size(); i++)
         oldcodes[vertices[i]] = codes[vertices[i]];

      for (i = 0; i < edges.size(); i++)
      {
         int edge_index = edges[i];
         const Edge &edge = _g.getEdge(edge_index);
         int edge_rank = _getEdgeCode(edge_index);

         dword v1_code = oldcodes[edge.beg];
         dword v2_code = oldcodes[edge.end];

         codes[edge.beg] += v2_code * v2_code + (v2_code + 23) * (edge_rank + 1721);
         codes[edge.end] += v1_code * v1_code + (v1_code + 23) * (edge_rank + 1721);
      }
   }

   dword result = 0;
   
   for (i = 0; i < vertices.size(); i++)
   {
      dword code = codes[vertices[i]];
      
      result += code * (code + 6849) + 29;
   }

   if (calc_different_codes_count)
   {
      // Calculate number of different codes
      Array<dword> &code_was_used = oldcodes;
      for (i = 0; i < vertices.size(); i++)
         code_was_used[vertices[i]] = 0;

      _different_codes_count = 0;
      for (int i = 0; i < vertices.size(); i++)
      {
         if (code_was_used[vertices[i]])
            continue;
         _different_codes_count++;
         dword cur_code = codes[vertices[i]];
         for (int j = 0; j < vertices.size(); j++)
            if (codes[vertices[j]] == cur_code)
               code_was_used[vertices[j]] = 1;
      }
   }

   return result;
}

int SubgraphHash::getDifferentCodesCount ()
{
   return _different_codes_count;
}

int SubgraphHash::_cmp_int (int a, int b, void *context)
{
   return a - b;
}
