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

#include "graph/spanning_tree.h"
#include "base_cpp/tlscont.h"
#include "graph/filter.h"

using namespace indigo;

IMPL_ERROR(SpanningTree, "spanning tree");

SpanningTree::SpanningTree (Graph &graph, const Filter *vertex_filter, const Filter *edge_filter) : _graph(graph),
TL_CP_GET(_edges_list),
TL_CP_GET(_depth_counters),
TL_CP_GET(_tree),
TL_CP_GET(_mapping),
TL_CP_GET(_inv_mapping),
TL_CP_GET(_edge_mapping),
TL_CP_GET(_stack)
{
   int i;

   _vertex_filter = vertex_filter;
   _edge_filter = edge_filter;

   _tree.clear();
   _edges_list.clear();
   _mapping.clear_resize(_graph.vertexCount());
   _edge_mapping.clear_resize(_graph.edgeCount());
   _inv_mapping.clear_resize(_graph.vertexEnd());

   for (i = _graph.vertexBegin(); i < _graph.vertexEnd(); i = _graph.vertexNext(i))
   {
      if (vertex_filter != 0 && !vertex_filter->valid(i))
         continue;
      int idx = _tree.addVertex();

      _mapping[idx] = i;
      _inv_mapping[i] = idx;
   }

   _depth_counters.clear_resize(_tree.vertexEnd());
   _depth_counters.zerofill();
   _current_depth = 0;

   int start = 0;

   _stack.clear();

   while (1)
   {
      for (; start < _tree.vertexEnd(); start = _tree.vertexNext(start))
      {
         if (vertex_filter != 0 && !vertex_filter->valid(_mapping[start]))
            continue;
         if (_depth_counters[start] == 0)
            break;
      }

      if (start == _tree.vertexEnd())
         break;

      StackElem & elem = _stack.push();
      elem.vertex = &_graph.getVertex(_mapping[start]);
      elem.nei_idx = elem.vertex->neiBegin();
      elem.vertex_idx = start;
      elem.parent_idx = -1;
      _depth_counters[start] = ++_current_depth;
      _build();
   }
}

void SpanningTree::_build ()
{
   while (_stack.size() > 0)
   {
      StackElem &elem = _stack.top();

      int v = elem.vertex_idx;
      int i = elem.nei_idx;

      if (i < elem.vertex->neiEnd())
      {
         elem.nei_idx = elem.vertex->neiNext(i);

         int nei_v = elem.vertex->neiVertex(i);
         if (_vertex_filter != 0 && !_vertex_filter->valid(nei_v))
            continue;

         if (_edge_filter != 0)
         {
            int nei_edge = elem.vertex->neiEdge(i);
            if (!_edge_filter->valid(nei_edge))
               continue;
         }

         int w = _inv_mapping[elem.vertex->neiVertex(i)];

         if (_depth_counters[w] == 0)
         {
            int idx = _tree.addEdge(v, w);

            _edge_mapping[idx] = elem.vertex->neiEdge(i);

            StackElem &newelem = _stack.push();

            _depth_counters[w] = ++_current_depth;
            newelem.parent_idx = v;
            newelem.vertex_idx = w;
            newelem.vertex = &_graph.getVertex(_mapping[w]);
            newelem.nei_idx = newelem.vertex->neiBegin();
         }
         else if  (w != elem.parent_idx && _depth_counters[w] < _depth_counters[v])
         {
            ExtEdge edge;

            edge.beg_idx = v;
            edge.end_idx = w;
            edge.ext_beg_idx = _mapping[v];
            edge.ext_end_idx = _mapping[w];
            edge.ext_edge_idx = elem.vertex->neiEdge(i);
            _edges_list.push(edge);
         }
      }
      else
         _stack.pop();
   }
}

void SpanningTree::addEdge (int beg, int end, int ext_index)
{
   int idx = _tree.addEdge(beg, end);
   _edge_mapping[idx] = ext_index;
}

void SpanningTree::markAllEdgesInCycles (int *marks_out, int value)
{
   int i, j;

   QS_DEF(Array<int>, path);

   for (i = 0; i < _edges_list.size(); i++)
   {
      const ExtEdge &ext_edge = _edges_list[i];

      if (!_tree.findPath(ext_edge.beg_idx, ext_edge.end_idx, path))
         throw Error("markAllEdgesInCycles(): no path");

      for (j = 0; j < path.size(); j++)
         marks_out[_edge_mapping[path[j]]] = value;

      marks_out[ext_edge.ext_edge_idx] = value;
   }
}
