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

#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"

#include "graph/graph.h"
#include "graph/ring_canonizer.h"

bool RingCanonizer::_makeCode (const Graph &ring, int start_vertex, int start_edge,
                               Array<int> &codes, Array<int> &indices)
{
   int v_idx = start_vertex;
   int e_idx = start_edge;

   int length = 0;
   bool bigger = false;

   indices.clear();

   do
   {
      const Vertex &vertex = ring.getVertex(v_idx);

      int j = vertex.neiBegin();
      if (vertex.neiEdge(j) != e_idx)
      {
         j = vertex.neiNext(j);
         if (vertex.neiEdge(j) != e_idx)
            throw Error("internal error: can not find edge %d", e_idx);
      }

      int vcode = cb_vertex_rank(ring, v_idx);
      int ecode = 0;

      if (cb_edge_rank != 0)
         ecode = cb_edge_rank(ring, e_idx);

      if (codes.size() <= length)
      {
         codes.push(vcode);
         bigger = true;
      }
      else
      {
         if (!bigger)
         {
            if (codes[length] > vcode)
               return false;

            if (codes[length] < vcode)
               bigger = true;
         }
         if (bigger)
            codes[length] = vcode;
      }

      indices.push(v_idx);
      length++;

      if (codes.size() <= length)
      {
         codes.push(ecode);
         bigger = true;
      }
      else
      {
         if (!bigger)
         {
            if (codes[length] > ecode)
               return false;

            if (codes[length] < ecode)
               bigger = true;
         }
         if (bigger)
            codes[length] = ecode;
      }

      indices.push(e_idx);
      length++;

      v_idx = vertex.neiVertex(j);

      const Vertex &next_vertex = ring.getVertex(v_idx);

      j = next_vertex.neiBegin();
      if (next_vertex.neiEdge(j) == e_idx)
         e_idx = next_vertex.neiEdge(next_vertex.neiNext(j));
      else
         e_idx = next_vertex.neiEdge(j);

   } while (v_idx != start_vertex);

   if (bigger)
      while (codes.size() != length)
         codes.pop();

   return bigger;
}

void RingCanonizer::perform (const Graph &ring, Output &output)
{
   int i;
   int maximal_rank = -1;
   QS_DEF(Array<int>, codes);
   QS_DEF(Array<int>, indices1);
   QS_DEF(Array<int>, indices2);

   codes.clear();

   if (ring.vertexCount() < 1)
      return;

   Array<int> *indices_cur = &indices1,
              *indices_temp = &indices2, *tmp;

   if (cb_vertex_rank != 0)
      for (i = ring.vertexBegin(); i != ring.vertexEnd(); i = ring.vertexNext(i))
      {
         int rank = cb_vertex_rank(ring, i);

         if (rank > maximal_rank)
            maximal_rank = rank;
      }

   for (i = ring.vertexBegin(); i != ring.vertexEnd(); i = ring.vertexNext(i))
   {
      if (cb_vertex_rank != 0 && cb_vertex_rank(ring, i) != maximal_rank)
         continue;

      const Vertex &vertex = ring.getVertex(i);

      if (_makeCode(ring, i, vertex.neiEdge(vertex.neiBegin()), codes, *indices_temp))
         __swap(indices_cur, indices_temp, tmp);
      if (_makeCode(ring, i, vertex.neiEdge(vertex.neiNext(vertex.neiBegin())), codes, *indices_temp))
         __swap(indices_cur, indices_temp, tmp);
   }

   for (i = 0; i < indices_cur->size(); i += 2)
   {
      if (cb_vertex_code != 0)
         cb_vertex_code(ring, indices_cur->at(i), output);
      if (cb_edge_code != 0)
         cb_edge_code(ring, indices_cur->at(i + 1), output);
   }
   output.writeChar(0);
}
