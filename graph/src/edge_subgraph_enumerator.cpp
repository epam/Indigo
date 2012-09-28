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

#include "math/algebra.h"

#include "graph/spanning_tree.h"
#include "graph/edge_subgraph_enumerator.h"

using namespace indigo;

IMPL_ERROR(EdgeSubgraphEnumerator, "edge subgraph enumerator");

EdgeSubgraphEnumerator::EdgeSubgraphEnumerator (Graph &graph) :
_graph(graph),
TL_CP_GET(_subgraph),
TL_CP_GET(_mapping),
TL_CP_GET(_inv_mapping),
TL_CP_GET(_edge_mapping),
TL_CP_GET(_inv_edge_mapping),
TL_CP_GET(_pool),
TL_CP_GET(_adjacent_edges)
{
   min_edges = 1;
   max_edges = graph.edgeCount();
   cb_subgraph = 0;
   userdata = 0;
}

int EdgeSubgraphEnumerator::_fCIS ()
{
   int j;

   int min = _graph.edgeEnd();

   for (j = _subgraph.edgeBegin(); j < _subgraph.edgeEnd(); j = _subgraph.edgeNext(j))
   {
      const Edge &edge = _subgraph.getEdge(j);
      bool good = false;

      // 'hanging' edge?
      if (_subgraph.getVertex(edge.beg).degree() == 1 ||
          _subgraph.getVertex(edge.end).degree() == 1)
         good = true;
      else if (_subgraph.getEdgeTopology(j) == TOPOLOGY_RING)
         good = true;
      
      if (good && min > _edge_mapping[j])
         min = _edge_mapping[j];
   }

   return min;
}

EdgeSubgraphEnumerator::_Enumerator::_Enumerator (EdgeSubgraphEnumerator &context) :
_context(context),
_graph(context._graph),
_subgraph(context._subgraph),
_adjacent_edges_added(context._pool)
{
   _added_vertex = -1;
   _added_edge = -1;
}

EdgeSubgraphEnumerator::_Enumerator::_Enumerator (const EdgeSubgraphEnumerator::_Enumerator &other) :
_context(other._context),
_graph(other._context._graph),
_subgraph(other._context._subgraph),
_adjacent_edges_added(other._context._pool)
{
   _added_vertex = -1;
   _added_edge = -1;
}

void EdgeSubgraphEnumerator::_Enumerator::process ()
{
   if (_subgraph.edgeCount() >= _context.max_edges)
      throw Error("subgraph exceeds max_edges");

   int i, j;

   // find adjacent edges
   for (j = _subgraph.edgeBegin(); j < _subgraph.edgeEnd(); j = _subgraph.edgeNext(j))
   {
      const Edge &edge = _subgraph.getEdge(j);
      int vbeg_idx = _context._mapping[edge.beg];
      int vend_idx = _context._mapping[edge.end];
      const Vertex &vbeg = _graph.getVertex(vbeg_idx);
      const Vertex &vend = _graph.getVertex(vend_idx);

      for (i = vbeg.neiBegin(); i != vbeg.neiEnd(); i = vbeg.neiNext(i))
      {
         int edge_idx = vbeg.neiEdge(i);
         if (!_context._adjacent_edges[edge_idx] && _context._inv_edge_mapping[edge_idx] < 0)
            _addAdjacentEdge(edge_idx);
      }
      for (i = vend.neiBegin(); i != vend.neiEnd(); i = vend.neiNext(i))
      {
         int edge_idx = vend.neiEdge(i);
         if (!_context._adjacent_edges[edge_idx] && _context._inv_edge_mapping[edge_idx] < 0)
            _addAdjacentEdge(edge_idx);
      }
   }

   for (j = _graph.edgeBegin(); j < _graph.edgeEnd(); j = _graph.edgeNext(j))
   {
		if (!_context._adjacent_edges[j])
         continue;

      if (_context._inv_edge_mapping[j] >= 0)
         throw Error("internal error: edge mapped");

      _Enumerator next(*this);
 
      next._addEdgeToSubgraph(j);

      // reverse traverse
      if (j == _context._fCIS())
      {
         if (_subgraph.edgeCount() >= _context.min_edges && _subgraph.edgeCount() <= _context.max_edges)
         {
            if (_context.cb_subgraph != 0)
               _context.cb_subgraph(_graph, _context._inv_mapping.ptr(), _context._inv_edge_mapping.ptr(), _context.userdata);
         }
         if (_subgraph.edgeCount() < _context.max_edges)
            next.process();
      }

      next._removeAddedEdge();
   }

   _removeAdjacentEdges();
}

void EdgeSubgraphEnumerator::_Enumerator::_addEdgeToSubgraph (int edge_idx)
{
   const Edge &edge = _graph.getEdge(edge_idx);

   int beg = _context._inv_mapping[edge.beg];
   int end = _context._inv_mapping[edge.end];

   if (beg == -1 && end == -1)
      throw Error("internal error: beg == -1 && end == -1");

   if (beg == -1)
   {
      beg = _added_vertex = _subgraph.addVertex();
      _context._mapping[_added_vertex] = edge.beg;
      _context._inv_mapping[edge.beg] = _added_vertex;
   }
   else if (end == -1)
   {
      end = _added_vertex = _subgraph.addVertex();
      _context._mapping[_added_vertex] = edge.end;
      _context._inv_mapping[edge.end] = _added_vertex;
   }

   _added_edge = _subgraph.addEdge(beg, end);

   _context._edge_mapping[_added_edge] = edge_idx;
   _context._inv_edge_mapping[edge_idx] = _added_edge;
   _context._adjacent_edges[edge_idx] = 0;
}

void EdgeSubgraphEnumerator::_Enumerator::_removeAddedEdge ()
{
   if (_added_edge >= 0)
   {
      int idx = _context._edge_mapping[_added_edge];

      _subgraph.removeEdge(_added_edge);
      _context._edge_mapping[_added_edge] = -1;
      _context._inv_edge_mapping[idx] = -1;
      _context._adjacent_edges[idx] = 1;
   }

   if (_added_vertex >= 0)
   {
      int idx = _context._mapping[_added_vertex];

      _subgraph.removeVertex(_added_vertex);
      _context._inv_mapping[idx] = -1;
      _context._mapping[_added_vertex] = -1;
   }
}

void EdgeSubgraphEnumerator::_Enumerator::_addAdjacentEdge (int edge_idx)
{
   _context._adjacent_edges[edge_idx] = 1;
   _adjacent_edges_added.add(edge_idx);
}

void EdgeSubgraphEnumerator::_Enumerator::_removeAdjacentEdges ()
{
   int i;

   for (i = _adjacent_edges_added.begin(); i != _adjacent_edges_added.end();
        i = _adjacent_edges_added.next(i))
       _context._adjacent_edges[_adjacent_edges_added.at(i)] = 0;

   _adjacent_edges_added.clear();
}

void EdgeSubgraphEnumerator::process ()
{
   int i;

   _subgraph.clear();

   _mapping.clear_resize(_graph.vertexCount());
   _inv_mapping.clear_resize(_graph.vertexEnd());
   _edge_mapping.clear_resize(_graph.edgeCount());
   _inv_edge_mapping.clear_resize(_graph.edgeEnd());

   _adjacent_edges.clear_resize(_graph.edgeEnd());
   _adjacent_edges.zerofill();

   for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
      _inv_mapping[i] = -1;

   for (i = _graph.edgeBegin(); i < _graph.edgeEnd(); i = _graph.edgeNext(i))
      _inv_edge_mapping[i] = -1;

   for (i = _graph.edgeBegin(); i < _graph.edgeEnd(); i = _graph.edgeNext(i))
   {
      const Edge edge = _graph.getEdge(i);

      int idx1 = _subgraph.addVertex();
      int idx2 = _subgraph.addVertex();
      int idx = _subgraph.addEdge(idx1, idx2);

      _mapping[idx1] = edge.beg;
      _inv_mapping[edge.beg] = idx1;
      _mapping[idx2] = edge.end;
      _inv_mapping[edge.end] = idx2;

      _edge_mapping[idx] = i;
      _inv_edge_mapping[i] = idx;

      if (1 >= min_edges && 1 <= max_edges)
      {
         if (cb_subgraph != 0)
            cb_subgraph(_graph, _inv_mapping.ptr(), _inv_edge_mapping.ptr(), userdata);
      }

      if (max_edges > 1)
      {
         _Enumerator enumerator(*this);

         enumerator.process();
      }

      _subgraph.removeEdge(idx);
      _subgraph.removeVertex(idx1);
      _subgraph.removeVertex(idx2);

      _mapping[idx1] = -1;
      _mapping[idx2] = -1;
      _inv_mapping[edge.beg] = -1;
      _inv_mapping[edge.end] = -1;
      _edge_mapping[idx] = -1;
      _inv_edge_mapping[i] = -1;
   }
}
