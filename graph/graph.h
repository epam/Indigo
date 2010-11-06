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

#ifndef __graph_h__
#define __graph_h__

#include "base_cpp/array.h"
#include "base_cpp/list.h"
#include "base_cpp/obj_pool.h"
#include "graph/filter.h"

namespace indigo {
enum
{
   FILTER_EQ,
   FILTER_NEQ,
   FILTER_MORE
};

enum
{
   TOPOLOGY_RING = 1,
   TOPOLOGY_CHAIN = 2
};

struct VertexEdge
{
   VertexEdge () {}
   VertexEdge (int vertex, int edge) : v(vertex), e(edge) {}

   int v;
   int e;
};

class Vertex
{
public:
   Vertex (Pool<List<VertexEdge>::Elem> &pool) : neighbors(pool) {}
   ~Vertex () {}

   List<VertexEdge> neighbors;

   int neiBegin ()      const { return neighbors.begin(); }
   int neiEnd   ()      const { return neighbors.end(); }
   int neiNext  (int i) const { return neighbors.next(i); }

   int neiVertex (int i) const { return neighbors[i].v; }
   int neiEdge   (int i) const { return neighbors[i].e; }

   int findNeiVertex (int idx) const;
   int findNeiEdge   (int idx) const;

   int degree () const {return neighbors.size();}
private:
   Vertex (const Vertex &); // no implicit copy
};

struct Edge
{
   int beg;
   int end;

   int findOtherEnd (int i) const
   {
      if (i == beg)
         return end;
      if (i == end)
         return beg;
      return -1;
   }
};

class Graph
{
public:
   DEF_ERROR("graph");

   DLLEXPORT explicit Graph ();
   DLLEXPORT virtual ~Graph ();

   DLLEXPORT virtual void clear ();

   DLLEXPORT const Vertex & getVertex (int idx) const;

   DLLEXPORT const Edge & getEdge   (int idx) const;

   DLLEXPORT int vertexBegin ()      const {return _vertices->begin();}
   DLLEXPORT int vertexEnd   ()      const {return _vertices->end();}
   DLLEXPORT int vertexNext  (int i) const {return _vertices->next(i); }
   DLLEXPORT int vertexCount ()      const {return _vertices->size(); }

   DLLEXPORT int edgeBegin ()      const {return _edges.begin();}
   DLLEXPORT int edgeEnd   ()      const {return _edges.end();}
   DLLEXPORT int edgeNext  (int i) const {return _edges.next(i); }
   DLLEXPORT int edgeCount ()      const {return _edges.size(); }

   DLLEXPORT int addVertex ();
   DLLEXPORT int addEdge   (int beg, int end);

   DLLEXPORT int  findEdgeIndex (int beg, int end) const;
   DLLEXPORT bool haveEdge (int beg, int end) const; 

   DLLEXPORT void swapEdgeEnds (int edge_idx);
   DLLEXPORT void removeEdge (int idx);
   DLLEXPORT void removeVertex (int idx);
   DLLEXPORT void removeAllEdges ();

   DLLEXPORT bool findPath (int from, int where, Array<int> &path_out) const;

   DLLEXPORT void makeSubgraph (const Graph &other, const Array<int> &vertices, Array<int> *mapping);
   DLLEXPORT void makeSubgraph (const Graph &other, const Filter &filter, Array<int> *mapping_out, Array<int> *inv_mapping);
   DLLEXPORT void cloneGraph (const Graph &other, Array<int> *mapping);

   DLLEXPORT void mergeWith (const Graph &other, Array<int> *mapping);

   DLLEXPORT void makeEdgeSubgraph (const Graph &other, const Array<int> &vertices, const Array<int> &edges, Array<int> *v_mapping, Array<int> *e_mapping);

   DLLEXPORT int  getEdgeTopology (int idx);
   DLLEXPORT void setEdgeTopology (int idx, int topology);
   DLLEXPORT void validateEdgeTopologies ();

   DLLEXPORT static bool isConnected (const Graph &graph);
   DLLEXPORT static bool isChain_AssumingConnected (const Graph &graph);
   DLLEXPORT static bool isTree (const Graph &graph);
   DLLEXPORT static void filterVertices (const Graph &graph, const int *filter, int filter_type, int filter_value, Array<int> &result);
   DLLEXPORT static void filterEdges (const Graph &graph, const int *filter, int filter_type, int filter_value, Array<int> &result);
   DLLEXPORT static int  findMappedEdge (const Graph &graph, const Graph &mapped_graph, int edge_idx, const int *mapping);

   DLLEXPORT int vertexCountSSSR (int idx);
   DLLEXPORT int vertexSmallestRingSize (int idx);
   bool vertexInRing(int idx);

   DLLEXPORT int getComponentNumber (int v_idx);

protected:
   void _mergeWithSubgraph (const Graph &other, const Array<int> &vertices, const Array<int> *edges, Array<int> *mapping);

   Pool<List<VertexEdge>::Elem> *_neighbors_pool;
   ObjPool<Vertex>  *_vertices;
   Pool<Edge>       _edges;

   Array<int> _topology; // for each edge: TOPOLOGY_RING, TOPOLOGY_CHAIN, or -1 (not calculated)
   bool       _topology_valid;

   Array<int> _v_smallest_ring_size;
   Array<int> _v_sssr_count;
   bool        _sssr_valid;

   Array<int> _component_numbers;
   int        _components_valid;

   void _calculateTopology ();
   void _calculateSSSR ();
   void _calculateComponents ();
private:
   Graph (const Graph &); // no implicit copy
};

}
#endif
