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

#ifndef __graph_h__
#define __graph_h__

#include "base_cpp/array.h"
#include "base_cpp/list.h"
#include "base_cpp/obj_pool.h"
#include "base_cpp/obj_array.h"
#include "graph/filter.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

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

class DLLEXPORT Vertex
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

class CycleBasis;

class DLLEXPORT Graph
{
public:
   DECL_ERROR;

   explicit Graph ();
   virtual ~Graph ();

   virtual void clear ();

   const Vertex & getVertex (int idx) const;

   const Edge & getEdge   (int idx) const;

   int vertexBegin ()      const {return _vertices->begin();}
   int vertexEnd   ()      const {return _vertices->end();}
   int vertexNext  (int i) const {return _vertices->next(i); }
   int vertexCount ()      const {return _vertices->size(); }

   int edgeBegin ()      const {return _edges.begin();}
   int edgeEnd   ()      const {return _edges.end();}
   int edgeNext  (int i) const {return _edges.next(i); }
   int edgeCount ()      const {return _edges.size(); }

   int addVertex ();
   int addEdge   (int beg, int end);

   int  findEdgeIndex (int beg, int end) const;
   bool haveEdge (int beg, int end) const;
   bool hasEdge (int idx) const;
   bool hasVertex(int idx) const;
   int  getEdgeEnd (int beg, int edge) const;

   void swapEdgeEnds (int edge_idx);
   void removeEdge (int idx);
   void removeVertex (int idx);
   void removeAllEdges ();

   bool findPath (int from, int where, Array<int> &path_out) const;

   void makeSubgraph (const Graph &other, const Array<int> &vertices, Array<int> *mapping);
   void makeSubgraph (const Graph &other, const Filter &filter, Array<int> *mapping_out, Array<int> *inv_mapping);
   void cloneGraph (const Graph &other, Array<int> *mapping);

   void buildEdgeMapping (const Graph &other, Array<int> *mapping, Array<int> *edge_mapping);

   void mergeWith (const Graph &other, Array<int> *mapping);

   void makeEdgeSubgraph (const Graph &other, const Array<int> &vertices, const Array<int> &edges, Array<int> *v_mapping, Array<int> *e_mapping);

   int  getEdgeTopology (int idx);
   void setEdgeTopology (int idx, int topology);
   void validateEdgeTopologies ();

   static bool isConnected (const Graph &graph);
   static bool isChain_AssumingConnected (const Graph &graph);
   static bool isTree (const Graph &graph);
   static void filterVertices (const Graph &graph, const int *filter, int filter_type, int filter_value, Array<int> &result);
   static void filterEdges (const Graph &graph, const int *filter, int filter_type, int filter_value, Array<int> &result);
   static int  findMappedEdge (const Graph &graph, const Graph &mapped_graph, int edge_idx, const int *mapping);

   int vertexCountSSSR (int idx);
   int vertexSmallestRingSize (int idx);
   bool vertexInRing(int idx);

   List<int> & sssrEdges (int idx);
   List<int> & sssrVertices (int idx);
   int sssrCount ();

   int vertexComponent (int v_idx);
   int countComponents ();
   int countComponentVertices (int comp_idx);
   int countComponentEdges (int comp_idx);
   const Array<int> & getDecomposition ();

protected:
   void _mergeWithSubgraph (const Graph &other, const Array<int> &vertices, const Array<int> *edges,
           Array<int> *mapping, Array<int> *edge_mapping);

   Pool<List<VertexEdge>::Elem> *_neighbors_pool;
   ObjPool<Vertex>  *_vertices;
   Pool<Edge>       _edges;

   Array<int> _topology; // for each edge: TOPOLOGY_RING, TOPOLOGY_CHAIN, or -1 (not calculated)
   bool       _topology_valid;

   Array<int> _v_smallest_ring_size;
   Array<int> _v_sssr_count;
   Pool<List<int>::Elem> *_sssr_pool;
   ObjArray< List<int> > _sssr_vertices;
   ObjArray< List<int> > _sssr_edges;
   bool       _sssr_valid;

   Array<int> _component_numbers;
   Array<int> _component_vcount;
   Array<int> _component_ecount;
   int        _components_valid;
   int        _components_count;

   void _calculateTopology ();
   void _calculateSSSR ();
   void _calculateSSSRInit ();
   void _calculateSSSRByCycleBasis (CycleBasis &basis);
   void _calculateSSSRAddEdgesAndVertices (const Array<int> &cycle, List<int> &edges, List<int> &vertices);
   void _calculateComponents ();

   // This is a bad hack for those who are too lazy to handle the mappings.
   // NEVER USE IT.
   void _cloneGraph_KeepIndices (const Graph &other);

private:
   Graph (const Graph &); // no implicit copy
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
