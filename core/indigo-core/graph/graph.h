/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __graph_h__
#define __graph_h__

#include "base_cpp/array.h"
#include "base_cpp/list.h"
#include "base_cpp/non_copyable.h"
#include "base_cpp/obj_pool.h"
#include "base_cpp/ptr_array.h"
#include "base_cpp/ptr_reusable_pool.h"
#include "base_cpp/reusable.h"
#include "graph/filter.h"
#include "graph/graph_iterators.h"
#include <list>
#include <unordered_set>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    enum
    {
        FILTER_EQ,
        FILTER_NEQ,
        FILTER_MORE
    };

    enum
    {
        TOPOLOGY_ANY = -1,
        TOPOLOGY_RING = 1,
        TOPOLOGY_CHAIN = 2
    };

    struct VertexEdge
    {
        VertexEdge()
        {
        }
        VertexEdge(int vertex, int edge) : v(vertex), e(edge)
        {
        }

        int v;
        int e;
    };

    class DLLEXPORT Vertex : public Reusable
    {
    public:
        // The neighbor list starts unbound (no pool, no allocation): every vertex
        // is created through Graph::addVertex(), which binds it to the graph-wide
        // pool. Binding on construction would allocate a private pool per vertex
        // only to throw it away.
        Vertex() : neighbors_list(List<VertexEdge>::Unbound{})
        {
        }
        ~Vertex() override
        {
        }

        List<VertexEdge> neighbors_list;

        // Keeps the pool binding and the backing slots.
        void reuse() override
        {
            neighbors_list.clear();
        }

        // Binds the neighbor list onto the graph-wide element pool, so all
        // vertices of a graph share one. The pool applies this on every add(), so
        // a vertex is never handed out unbound.
        void reuse(Pool<List<VertexEdge>::Elem>& pool)
        {
            neighbors_list = List<VertexEdge>(pool);
        }

        NeighborsAuto neighbors() const;

        int neiBegin() const
        {
            return neighbors_list.begin();
        }
        int neiEnd() const
        {
            return neighbors_list.end();
        }
        int neiNext(int i) const
        {
            return neighbors_list.next(i);
        }

        int neiVertex(int i) const
        {
            return neighbors_list[i].v;
        }
        int neiEdge(int i) const
        {
            return neighbors_list[i].e;
        }

        int findNeiVertex(int idx) const;
        int findNeiEdge(int idx) const;

        int degree() const
        {
            return neighbors_list.size();
        }

    private:
        Vertex(const Vertex&); // no implicit copy
    };

    struct Edge
    {
        int beg;
        int end;

        int findOtherEnd(int i) const
        {
            if (i == beg)
                return end;
            if (i == end)
                return beg;
            return -1;
        }
    };

    class CycleBasis;

    class DLLEXPORT Graph : public NonCopyable, public Reusable
    {
    public:
        DECL_ERROR;

        explicit Graph();
        virtual ~Graph();

        VerticesAuto vertices() const;

        EdgesAuto edges();

        virtual void clear();
        virtual void changed();

        // Resetting a graph means emptying it. The virtual dispatch reaches the
        // most-derived clear(), so a pooled molecule is emptied completely.
        void reuse() override
        {
            clear();
        }

        const Vertex& getVertex(int idx) const;

        const Edge& getEdge(int idx) const;

        int vertexBegin() const
        {
            return _vertices->begin();
        }
        int vertexEnd() const
        {
            return _vertices->end();
        }
        int vertexNext(int i) const
        {
            return _vertices->next(i);
        }
        int vertexCount() const
        {
            return _vertices->size();
        }

        int edgeBegin() const
        {
            return _edges.begin();
        }
        int edgeEnd() const
        {
            return _edges.end();
        }
        int edgeNext(int i) const
        {
            return _edges.next(i);
        }
        int edgeCount() const
        {
            return _edges.size();
        }

        int addVertex();
        int addEdge(int beg, int end);

        int findEdgeIndex(int beg, int end) const;
        bool haveEdge(int beg, int end) const;
        bool hasEdge(int idx) const;
        bool hasVertex(int idx) const;
        int getEdgeEnd(int beg, int edge) const;

        void swapEdgeEnds(int edge_idx);
        void removeEdge(int idx);
        void removeVertex(int idx);
        void removeAllEdges();

        bool findPath(int from, int where, Array<int>& path_out) const;

        void makeSubgraph(const Graph& other, const Array<int>& vertices, Array<int>* vertex_mapping);
        void makeSubgraph(const Graph& other, const Array<int>& vertices, Array<int>* vertex_mapping, const Array<int>* edges, Array<int>* edge_mapping);
        void makeSubgraph(const Graph& other, const Filter& filter, Array<int>* mapping_out, Array<int>* inv_mapping);
        void cloneGraph(const Graph& other, Array<int>* mapping);

        void buildEdgeMapping(const Graph& other, Array<int>* mapping, Array<int>* edge_mapping);

        void mergeWith(const Graph& other, Array<int>* mapping);

        void makeEdgeSubgraph(const Graph& other, const Array<int>& vertices, const Array<int>& edges, Array<int>* v_mapping, Array<int>* e_mapping);

        int getEdgeTopology(int idx);
        void setEdgeTopology(int idx, int topology);
        void validateEdgeTopologies();

        static bool isConnected(Graph& graph);
        static bool isChain_AssumingConnected(const Graph& graph);
        static bool isTree(Graph& graph);
        static void filterVertices(const Graph& graph, const int* filter, int filter_type, int filter_value, Array<int>& result);
        static void filterEdges(const Graph& graph, const int* filter, int filter_type, int filter_value, Array<int>& result);
        static int findMappedEdge(const Graph& graph, const Graph& mapped_graph, int edge_idx, const int* mapping);

        int vertexCountSSSR(int idx);
        int vertexSmallestRingSize(int idx);
        bool vertexInRing(int idx);
        int edgeSmallestRingSize(int idx);

        List<int>& sssrEdges(int idx);
        List<int>& sssrVertices(int idx);
        int sssrCount();

        int vertexComponent(int v_idx);
        int countComponents();
        int countComponents(const std::list<std::unordered_set<int>>& external_neighbors);
        int countComponentVertices(int comp_idx);
        int countComponentVertices(int comp_idx, const std::list<std::unordered_set<int>>& external_neighbors);
        int countComponentEdges(int comp_idx);
        int countComponentEdges(int comp_idx, const std::list<std::unordered_set<int>>& external_neighbors);

        const Array<int>& getDecomposition();

        bool isTerminalVertex(int v_idx) const;
        bool isTerminalEdge(int e_idx) const;

    protected:
        void _mergeWithSubgraph(const Graph& other, const Array<int>& vertices, const Array<int>* edges, Array<int>* mapping, Array<int>* edge_mapping);

        Pool<List<VertexEdge>::Elem>* _neighbors_pool;
        PtrReusablePool<Vertex>* _vertices;
        Pool<Edge> _edges;

        Array<int> _topology; // for each edge: TOPOLOGY_RING, TOPOLOGY_CHAIN, or -1 (not calculated)
        bool _topology_valid;

        Array<int> _v_smallest_ring_size, _e_smallest_ring_size;
        Array<int> _v_sssr_count;
        Pool<List<int>::Elem>* _sssr_pool;
        PtrArray<List<int>> _sssr_vertices;
        PtrArray<List<int>> _sssr_edges;
        bool _sssr_valid;

        Array<int> _component_numbers;
        Array<int> _component_vcount;
        Array<int> _component_ecount;
        bool _components_valid;
        // Whether the cached decomposition was computed with external neighbours.
        // Without it the result would depend on the order of the calls: the two
        // countComponents() overloads share one cache, and getDecomposition()
        // computes without them.
        bool _components_used_external;
        int _components_count;

        void _calculateTopology();
        void _calculateSSSR();
        void _calculateSSSRInit();
        void _calculateSSSRByCycleBasis(CycleBasis& basis);
        void _calculateSSSRAddEdgesAndVertices(const Array<int>& cycle, List<int>& edges, List<int>& vertices);
        void _calculateComponents(const std::list<std::unordered_set<int>>& external_neighbors = {{}});
        // This is a bad hack for those who are too lazy to handle the mappings.
        // NEVER USE IT.
        void _cloneGraph_KeepIndices(const Graph& other);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
