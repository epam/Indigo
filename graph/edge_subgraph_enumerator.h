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

#ifndef __edge_subgraph_enumerator__
#define __edge_subgraph_enumerator__

#include "graph/graph.h"
#include "base_cpp/tlscont.h"

namespace indigo
{

class EdgeSubgraphEnumerator
{
public:
   explicit EdgeSubgraphEnumerator (Graph &graph);

   int min_edges;
   int max_edges;

   void process ();

   void (*cb_subgraph) (Graph &graph, const int *vertices, const int *edges, void *userdata);

   void *userdata;

   DECL_ERROR;

protected:
   int  _fCIS ();

   Graph &_graph;

   TL_CP_DECL(Graph,      _subgraph);

   TL_CP_DECL(Array<int>, _mapping);          // subgraph -> graph
   TL_CP_DECL(Array<int>, _inv_mapping);      // graph -> subgraph
   TL_CP_DECL(Array<int>, _edge_mapping);     // subgraph -> graph
   TL_CP_DECL(Array<int>, _inv_edge_mapping); // graph -> subgraph

   TL_CP_DECL(Pool<List<int>::Elem>, _pool);
   TL_CP_DECL(Array<int>, _adjacent_edges);


   class _Enumerator
   {
   public:
      _Enumerator (EdgeSubgraphEnumerator &context);
      _Enumerator (const _Enumerator &other);

      void process ();

   protected:

      EdgeSubgraphEnumerator &_context;

      Graph &_graph;
      Graph &_subgraph;

      void _addEdgeToSubgraph (int edge_idx);
      void _removeAddedEdge   ();
      void _addAdjacentEdge (int edge_idx);
      void _removeAdjacentEdges ();

      int       _added_vertex;
      int       _added_edge;
      List<int> _adjacent_edges_added;
   };
};

}

#endif
