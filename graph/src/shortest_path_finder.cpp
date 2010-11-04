#include "graph/graph.h"
#include "graph/shortest_path_finder.h"

ShortestPathFinder::ShortestPathFinder (const Graph &graph) : _graph(graph)
{                                       
   cb_check_vertex = 0;
   cb_check_edge = 0;
   check_vertex_context = 0;
   check_edge_context = 0;
   queue.setLength(_graph.vertexEnd());
   prev.clear_resize(_graph.vertexEnd());
}  

bool ShortestPathFinder::find (Array<int>& vertices, Array<int>& edges, int u, int v)
{         
   // init
   queue.clear();
   prev.fffill();
   vertices.clear();
   edges.clear();

   // push initial vertex
   queue.push(v);
   prev[v] = u;

   while (!queue.isEmpty())
   {
      // pop vertex
      int w = queue.pop();

      const Vertex& vert = _graph.getVertex(w);
      for (int i = vert.neiBegin(); i < vert.neiEnd(); i = vert.neiNext(i))
      {
         int e = vert.neiEdge(i);
         if (cb_check_edge != 0 && !cb_check_edge(_graph, e, check_edge_context))
            continue; 
         int n = vert.neiVertex(i);
         if (cb_check_vertex != 0 && !cb_check_vertex(_graph, n, check_vertex_context))
            continue;
         if (prev[n] >= 0)
            continue; // vertex is already done

         if (n == u)
         {
            // shortest path found. mark and return
            prev[u] = w;
            for (int j = u; j != v; j = prev[j])
            {  
               vertices.push(j);
               edges.push(_graph.findEdgeIndex(j, prev[j]));
            }
            vertices.push(v);
            return true;
         }

         queue.push(n);
         prev[n] = w;
      }
   }     
   return false;
}
