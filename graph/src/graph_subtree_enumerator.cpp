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

#include "graph/graph_subtree_enumerator.h"

using namespace indigo;

GraphSubtreeEnumerator::GraphSubtreeEnumerator (Graph &graph) :
_graph(graph),
TL_CP_GET(_subtree),
TL_CP_GET(_vertices),
TL_CP_GET(_edges),
TL_CP_GET(_v_mapping),
TL_CP_GET(_e_mapping),
TL_CP_GET(_inv_e_mapping),
TL_CP_GET(_pool),
TL_CP_GET(_dfs_front)
{
   min_vertices = 1;
   max_vertices = graph.vertexCount();
   callback = 0;
   callback2 = 0;
   context = 0;
   handle_maximal = false;
   maximal_critera_value_callback = 0;
   vfilter = 0;
}

GraphSubtreeEnumerator::~GraphSubtreeEnumerator ()
{
   _dfs_front.clear(); // to avoid data race
}

void GraphSubtreeEnumerator::process ()
{
   int i;

   _dfs_front.clear();
   _dfs_front.push(_pool);

   _subtree.clear();
   _v_mapping.clear_resize(_graph.vertexEnd());
   _e_mapping.clear_resize(_graph.edgeEnd());
   _inv_e_mapping.clear_resize(_graph.edgeEnd());

   for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
      _v_mapping[i] = -1;
   for (i = _graph.edgeBegin(); i < _graph.edgeEnd(); i = _graph.edgeNext(i))
      _e_mapping[i] = -1;

   int root_idx = _subtree.addVertex();

   _edges.clear();
   _vertices.clear();

   for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
   {
      if (vfilter != 0 && !vfilter->valid(i))
         continue;

      _v_mapping[i] = root_idx;
      _vertices.push(i);

      int cur_maximal_criteria_value = 0;
      if (handle_maximal && maximal_critera_value_callback != 0)
         cur_maximal_criteria_value =
            maximal_critera_value_callback(_graph, _v_mapping.ptr(), _e_mapping.ptr(), context);

      _reverseSearch(i, cur_maximal_criteria_value);

      _v_mapping[i] = -1;
      _vertices.pop();
   }
}

void GraphSubtreeEnumerator::_updateDfsFront (int v_idx)
{
   int nvertices = _subtree.vertexCount();

   while (_dfs_front.size() < nvertices + 1)
      _dfs_front.push(_pool);

   List<VertexEdge> &prev_front = _dfs_front[nvertices - 1];
   List<VertexEdge> &front = _dfs_front[nvertices];

   front.clear();

   for (int i = prev_front.begin(); i != prev_front.end(); i = prev_front.next(i))
   {
      VertexEdge xedge = prev_front[i];

      if (xedge.v != v_idx)
         front.add(xedge);
   }

   const Vertex &vertex = _graph.getVertex(v_idx);

   for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
   {
      int nei = vertex.neiVertex(i);
      int edge = vertex.neiEdge(i);

      if (vfilter != 0 && !vfilter->valid(nei))
         continue;
      
      if (_v_mapping[nei] == -1)
      {
         VertexEdge xe(nei, edge);

         front.add(xe);
      }
   }
}

void GraphSubtreeEnumerator::_reverseSearch (int v_idx, int cur_maximal_criteria_value)
{
   _updateDfsFront(v_idx);

   int nvertices = _subtree.vertexCount();
   bool has_supergraph = false;

   bool maximal_by_criteria = true;

   if (nvertices < max_vertices)
   {
      // Additional check to reduce number of checked "new_e_idx == _fCIS()" in the inner loop
      // new_e_idx can be equal to _fCIS() only if edge number is maximal that means that
      // it is greater than previous maximum or 2nd statistic. And if new_e_idx is not attached 
      // to the previous maximum edge then previous maximum persists and we need to compare only
      // with m1. Otherwise we have to compare with m2.
      int m1, m2, max_vertex;
      _leafMaxEdgeValues(m1, m2, max_vertex);

      for (int i = _dfs_front[nvertices].begin();
           i != _dfs_front[nvertices].end(); i = _dfs_front[nvertices].next(i))
      {
         VertexEdge xe = _dfs_front[nvertices][i];
         // Edge number should be greater then m2
         if (xe.e < m2)
            continue;

         int parent = _graph.getEdge(xe.e).beg + _graph.getEdge(xe.e).end - xe.v;
         int subtree_parent = _v_mapping[parent];
         // If edge is attached to the previous maximum then it replaces it and
         // edge number should be greater then m1
         if (subtree_parent != max_vertex && xe.e < m1)
            continue;

         int new_v_idx = _subtree.addVertex();

         int new_e_idx = _subtree.addEdge(new_v_idx, subtree_parent);

         _vertices.push(xe.v);
         _edges.push(xe.e);

         _v_mapping[xe.v] = new_v_idx;
         _e_mapping[xe.e] = new_e_idx;
         _inv_e_mapping[new_e_idx] = xe.e;

         int descedant_maximal_criteria_value = 0;
         if (handle_maximal && maximal_critera_value_callback != 0)
            descedant_maximal_criteria_value =
               maximal_critera_value_callback(_graph, _v_mapping.ptr(), _e_mapping.ptr(), context);

         if (descedant_maximal_criteria_value == cur_maximal_criteria_value)
            maximal_by_criteria = false;

         if ((nvertices > 1 || v_idx < xe.v) && new_e_idx == _fCIS())
            _reverseSearch(xe.v, descedant_maximal_criteria_value);

         has_supergraph = true;

         _v_mapping[xe.v] = -1;
         _e_mapping[xe.e] = -1;

         _vertices.pop();
         _edges.pop();

         _inv_e_mapping[new_e_idx] = -1;
         _subtree.removeVertex(new_v_idx);
      }
   }

   if (nvertices >= min_vertices && nvertices <= max_vertices && (callback != 0 || callback2 != 0))
   {
      if (handle_maximal)
      {
         if (has_supergraph && !maximal_by_criteria)
            return; // This subgraph isn't maximal
      }

      if (callback)
         callback(_graph, _v_mapping.ptr(), _e_mapping.ptr(), context);
      if (callback2)
         callback2(_graph, _vertices, _edges, context);
   }
}

void GraphSubtreeEnumerator::_leafMaxEdgeValues (int &m1, int &m2, int &max_vertex)
{
   m1 = -1;
   m2 = -1;
   max_vertex = -1;

   for (int i = _subtree.vertexBegin(); i != _subtree.vertexEnd(); i = _subtree.vertexNext(i))
   {
      const Vertex &vertex = _subtree.getVertex(i);
      if (vertex.degree() == 1)
      {
         int nei_first = vertex.neiBegin();
         int edge_idx = vertex.neiEdge(nei_first);
         int value = _inv_e_mapping[edge_idx];

         if (value > m1)
         {
            m2 = m1;
            m1 = value;
            max_vertex = i;
         }
         else if (value > m2)
         {
            m2 = value;
         }
      }
   }
}


// The index of edge whose removal won't disconnect the _subtree
// (the edge corresponding to the maximal _graph edge is selected)
int GraphSubtreeEnumerator::_fCIS ()
{
   int max_edge = -1;
   int max_edge_value = -1;

   for (int i = _subtree.vertexBegin(); i != _subtree.vertexEnd(); i = _subtree.vertexNext(i))
   {
      const Vertex &vertex = _subtree.getVertex(i);
      if (vertex.degree() == 1)
      {
         int edge_idx = vertex.neiEdge(vertex.neiBegin());
         int value = _inv_e_mapping[edge_idx];

         if (value > max_edge_value)
         {
            max_edge_value = value;
            max_edge = edge_idx;
         }
      }
   }

   return max_edge;
}
