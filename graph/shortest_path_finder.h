#ifndef __shortest_path_finder_h__
#define __shortest_path_finder_h__

#include "base_cpp/array.h"
#include "base_cpp/queue.h"

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

#endif
