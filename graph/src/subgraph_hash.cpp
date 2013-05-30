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

#include "graph/subgraph_hash.h"
#include "graph/graph.h"

using namespace indigo;

CP_DEF(SubgraphHash);

SubgraphHash::SubgraphHash (Graph &g) : _g(g),
   CP_INIT,
   TL_CP_GET(_codes),
   TL_CP_GET(_oldcodes),
   TL_CP_GET(_gf)
{
   max_iterations = _g.vertexEnd();
   vertex_codes = 0;
   edge_codes = 0;
   _different_codes_count = 0;
   calc_different_codes_count = false;

   _codes.clear_resize(_g.vertexEnd());
   _oldcodes.clear_resize(_g.vertexEnd());

   _gf.setGraph(g);
   _gf.prepareEdges();
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
   int i, iter;

   dword *codes_ptr = _codes.ptr();
   dword *oldcodes_ptr = _oldcodes.ptr();

   if (vertex_codes == 0 || edge_codes == 0)
      throw Exception("SubgraphHash: vertex_codes and edge_codes are not set");

   const int *vc = vertex_codes->ptr();
   const int *ec = edge_codes->ptr();

   const int *v = vertices.ptr();
   const int *e = edges.ptr();
   for (i = 0; i < vertices.size(); i++)
      codes_ptr[v[i]] = vc[v[i]];

   const Edge *graph_edges = _gf.getEdges();

   for (iter = 0; iter < max_iterations; iter++)
   {
      for (i = 0; i < vertices.size(); i++)
         oldcodes_ptr[v[i]] = codes_ptr[v[i]];
      for (i = 0; i < edges.size(); i++)
      {
         int edge_index = e[i];
         const Edge &edge = graph_edges[edge_index];

         int edge_rank = ec[edge_index];
         dword v1_code = oldcodes_ptr[edge.beg];
         dword v2_code = oldcodes_ptr[edge.end];

         codes_ptr[edge.beg] += v2_code * v2_code + (v2_code + 23) * (edge_rank + 1721);
         codes_ptr[edge.end] += v1_code * v1_code + (v1_code + 23) * (edge_rank + 1721);
      }
   }

   dword result = 0;
   
   for (i = 0; i < vertices.size(); i++)
   {
      dword code = codes_ptr[v[i]];
      
      result += code * (code + 6849) + 29;
   }

   if (calc_different_codes_count)
   {
      // Calculate number of different codes
      Array<dword> &code_was_used = _oldcodes;
      dword *code_was_used_ptr = code_was_used.ptr();

      for (i = 0; i < vertices.size(); i++)
         code_was_used_ptr[v[i]] = 0;

      _different_codes_count = 0;
      for (int i = 0; i < vertices.size(); i++)
      {
         if (code_was_used[v[i]])
            continue;
         _different_codes_count++;
         dword cur_code = codes_ptr[v[i]];
         for (int j = 0; j < vertices.size(); j++)
            if (codes_ptr[v[j]] == cur_code)
               code_was_used_ptr[v[j]] = 1;
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
