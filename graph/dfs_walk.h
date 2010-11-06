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

#ifndef __dfs_walk_h__
#define __dfs_walk_h__

#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"
#include "graph.h"

namespace indigo {

class Graph;

class DfsWalk
{
public:
   struct SeqElem
   {
      int idx;           // index of vertex in _graph
      int parent_vertex; // parent vertex in DFS tree
      int parent_edge;   // edge to parent vertex
   };
   
   explicit DfsWalk (const Graph &graph);
   virtual ~DfsWalk ();
   
   void walk ();
   
   const Array<SeqElem> & getSequence () const;

   bool edgeClosingCycle (int e_idx) const;
   int  numBranches      (int v_idx) const;
   int  numOpeningCycles (int e_idx) const;
   
   // mapping[i] = index of appearance of graph's i-th vertex in the vertex sequence
   void calcMapping (Array<int> &mapping) const;
   
   int *ignored_vertices;
   int *vertex_ranks;
   
   DEF_ERROR("DFS walk");
protected:
   struct _VertexDesc
   {
      int dfs_state;     // 0 -- not on stack
                         // 1 -- on stack
                         // 2 -- removed from stack
      int parent_vertex; // parent vertex in DFS tree
      int parent_edge;   // edge to parent vertex
      int branches;      // how many DFS branches go out from this vertex
   };

   struct _EdgeDesc
   {
      int opening_cycles; // how many cycles are
                          // (i) starting with this edge
                          // and (ii) ending in this edge's first vertex
      int closing_cycle;  // 1 if this edge closes a cycle
   };

   static int _cmp (VertexEdge &ve1, VertexEdge &ve2, void *context);

   const Graph &_graph;
   
   TL_CP_DECL(Array<_VertexDesc>,    _vertices);
   TL_CP_DECL(Array<_EdgeDesc>,      _edges);
   
   TL_CP_DECL(Array<SeqElem>, _v_seq);

   
};
}

#endif
