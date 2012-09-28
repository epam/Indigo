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

#include "layout/molecule_layout_graph.h"

using namespace indigo;

MoleculeLayoutGraph::Cycle::Cycle () :
TL_CP_GET(_vertices),
TL_CP_GET(_edges)
{
   _vertices.clear();
   _edges.clear();
   _max_idx = 0;
}

MoleculeLayoutGraph::Cycle::Cycle (const List<int> &edges, const MoleculeLayoutGraph &graph) :
TL_CP_GET(_vertices),
TL_CP_GET(_edges)
{
   copy(edges, graph);
}

MoleculeLayoutGraph::Cycle::Cycle (const Array<int> &vertices, const Array<int> &edges) :
TL_CP_GET(_vertices),
TL_CP_GET(_edges)
{
   copy(vertices, edges);
}

void MoleculeLayoutGraph::Cycle::copy (const List<int> &edges, const MoleculeLayoutGraph &graph)
{
   int i = edges.begin();
   const Edge &edge1 = graph.getEdge(edges[i]);
   const Edge &edge2 = graph.getEdge(edges[edges.next(i)]);
   
   _vertices.clear();
   _edges.clear();
   
   if (edge1.beg == edge2.beg || edge1.beg == edge2.end)
      _vertices.push(edge1.end);
   else
      _vertices.push(edge1.beg);
   
   for ( ; i < edges.end(); i = edges.next(i))
   {
      const Edge &edge = graph.getEdge(edges[i]);
      
      if (_vertices.top() == edge.beg)
         _vertices.push(edge.end);
      else
         _vertices.push(edge.beg);
      
      _edges.push(edges[i]);
   }
   
   _vertices.pop();
   
   _max_idx = 0;

   for (int i = 0; i < _vertices.size(); i++)
      if (_vertices[i] > _max_idx)
         _max_idx = _vertices[i];
}

void MoleculeLayoutGraph::Cycle::copy (const Array<int> &vertices, const Array<int> &edges)
{
   _vertices.copy(vertices);
   _edges.copy(edges);
   _max_idx = 0;
   
   for (int i = 0; i < _vertices.size(); i++)
      if (_vertices[i] > _max_idx)
         _max_idx = _vertices[i];
}

void MoleculeLayoutGraph::Cycle::calcMorganCode (const MoleculeLayoutGraph &parent_graph)
{
   _morgan_code = 0;

   for (int i = 0; i < vertexCount(); i++)
      _morgan_code += parent_graph.getLayoutVertex(_vertices[i]).morgan_code;
}

void MoleculeLayoutGraph::Cycle::canonize ()
{
   // 1. v(0)<v(i), i=1,...,l-1 ; 2. v(1)< v(l-2) => unique representation of cycle
   int min_idx = 0, i;
   bool vert_invert = false;

   Cycle src_cycle(_vertices, _edges);

   for (i = 1; i < vertexCount(); i++)
      if (_vertices[i] < _vertices[min_idx])
         min_idx = i;

   int prev_idx = __max(0, min_idx - 1);
   int next_idx = __min(vertexCount() - 1, min_idx + 1);

   // rotate direction
   if (_vertices[prev_idx] < _vertices[next_idx])
      vert_invert = true;

   // rotate
   if (vert_invert)
   {
      for (i = 0; i < min_idx + 1; i++)
      {
         _vertices[i] = src_cycle._vertices[min_idx - i];
         _edges[i] = src_cycle._edges[i == min_idx ? vertexCount() - 1 : min_idx - i - 1];
      }
      for (     ; i < vertexCount(); i++)
      {
         _vertices[i] = src_cycle._vertices[min_idx - i + vertexCount()];
         _edges[i] = src_cycle._edges[min_idx - i + vertexCount() - 1];
      }
   } else
   {
      for (i = 0; i < vertexCount() - min_idx; i++)
      {
         _vertices[i] = src_cycle._vertices[min_idx + i];
         _edges[i] = src_cycle._edges[min_idx + i];
      }
      for (     ; i < vertexCount(); i++)
      {
         _vertices[i] = src_cycle._vertices[min_idx + i - vertexCount()];
         _edges[i] = src_cycle._edges[min_idx + i - vertexCount()];
      }
   }
}

bool MoleculeLayoutGraph::Cycle::contains (const Cycle &another) const
{
   if (vertexCount() < another.vertexCount())
      return false;

   QS_DEF(Array<int>, vertex_found);

   vertex_found.clear_resize(_max_idx + 1);
   vertex_found.zerofill();

   for (int i = 0; i < vertexCount(); i++)
      vertex_found[_vertices[i]] = 1;

   for (int i = 0; i < another.vertexCount(); i++)
      if (another._vertices[i] >= vertex_found.size() || vertex_found[another._vertices[i]] == 0)
         return false;

   return true;
}

// Cycle sorting callback
// Order by size: 6, 5, 7, 8, 4, 3, 9, 10, 11, ..
// If cycles has the same size then Morgan code in descending order (higher first)
int MoleculeLayoutGraph::Cycle::compare_cb (int &idx1, int &idx2, void *context)
{
   const ObjPool<Cycle> &cycles = *(const ObjPool<Cycle> *)context;

   int size_freq[] = {6, 5, 7, 8, 4, 3};
   int freq_idx1, freq_idx2;

   for (freq_idx1 = 0; freq_idx1 < NELEM(size_freq); freq_idx1++)
      if (cycles[idx1].vertexCount() == size_freq[freq_idx1])
         break;

   for (freq_idx2 = 0; freq_idx2 < NELEM(size_freq); freq_idx2++)
      if (cycles[idx2].vertexCount() == size_freq[freq_idx2])
         break;

   if (freq_idx1 != freq_idx2)
      return freq_idx1 - freq_idx2;

   if (freq_idx1 == NELEM(size_freq) && cycles[idx1].vertexCount() != cycles[idx2].vertexCount())
      return cycles[idx1].vertexCount() - cycles[idx2].vertexCount();

   return cycles[idx2].morganCode() - cycles[idx1].morganCode();
}
