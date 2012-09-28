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

#ifndef __spanning_tree_h__
#define __spanning_tree_h__

#include "base_cpp/array.h"
#include "graph/graph.h"
#include "base_cpp/tlscont.h"

namespace indigo {

class Filter;

class SpanningTree
{
public:
   struct ExtEdge
   {
      int beg_idx;
      int end_idx;
      int ext_beg_idx;
      int ext_end_idx;
      int ext_edge_idx;
   };

   explicit SpanningTree (Graph &graph, const Filter *vertex_filter, const Filter *edge_filter = 0);

   inline int            getEdgesNum ()
   {
      return _edges_list.size();
   }

   inline const ExtEdge & getExtEdge (int i)
   {
      return _edges_list[i];
   }

   void addEdge (int beg, int end, int ext_index);

   inline const Vertex &getVertexFromExtIdx (int ext_idx) const
   {
      return _tree.getVertex(_inv_mapping[ext_idx]);
   }

   inline int getExtVertexIndex (int v_idx) const 
   {
      return _mapping[v_idx]; 
   }

   inline int getExtEdgeIndex (int e_idx) const 
   {
      return _edge_mapping[e_idx]; 
   }

   void markAllEdgesInCycles (int *marks_out, int value);

   DECL_ERROR;
protected:
   struct StackElem
   {
      const Vertex *vertex;
      int vertex_idx;
      int nei_idx;
      int parent_idx;
   };

   void _build ();

   const Graph &_graph;
   const Filter *_vertex_filter;
   const Filter *_edge_filter;

   // these members made static for saving time of memory allocations
   TL_CP_DECL(Array<ExtEdge>, _edges_list);
   TL_CP_DECL(Array<int>, _depth_counters);
   TL_CP_DECL(Graph, _tree);
   TL_CP_DECL(Array<int>, _mapping);
   TL_CP_DECL(Array<int>, _inv_mapping);
   TL_CP_DECL(Array<int>, _edge_mapping);
   TL_CP_DECL(Array<StackElem>, _stack);

   int         _current_depth;
};

}

#endif
