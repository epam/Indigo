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

#ifndef __shortest_path_finder_h__
#define __shortest_path_finder_h__

#include "base_cpp/array.h"
#include "base_cpp/queue.h"

namespace indigo {

class Graph;
class ShortestPathFinder {
public:
   explicit ShortestPathFinder (const Graph &graph);

   void *check_vertex_context;   
   bool (*cb_check_vertex)(const Graph &graph, int v_idx, void *context);
   void *check_edge_context;   
   bool (*cb_check_edge)(const Graph &graph, int e_idx, void *context);

   bool find (Array<int>& vertices, Array<int>& edges, int u, int v);
private:   
   Queue<int> queue;
   Array<int> prev;
   const Graph &_graph;
};

}

#endif
