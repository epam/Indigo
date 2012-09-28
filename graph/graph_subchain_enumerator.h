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

#ifndef __chain_enumerator_h__
#define __chain_enumerator_h__

#include "base_cpp/tlscont.h"
#include "graph/graph.h"

namespace indigo {

class GraphSubchainEnumerator
{
public:
   enum
   {
      MODE_NO_DUPLICATE_VERTICES = 0,
      MODE_NO_BACKTURNS = 1,
      MODE_NO_CONSTRAINTS = 2
   };

   explicit GraphSubchainEnumerator (Graph &graph, int min_edges, int max_edges, int mode);
   virtual ~GraphSubchainEnumerator ();

   void * context;

   void (*cb_handle_chain) (Graph &graph, int size, const int *vertices, const int *edges, void *context);

   void processChains ();

protected:
   void _DFS (int from);

   Graph &_graph;
   int _max_edges;
   int _min_edges;
   int _mode;

   TL_CP_DECL(Array<int>, _vertex_states);
   TL_CP_DECL(Array<int>, _chain_vertices);
   TL_CP_DECL(Array<int>, _chain_edges);
};

}

#endif
