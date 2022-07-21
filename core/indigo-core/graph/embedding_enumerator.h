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

#ifndef __embedding_enumerator__
#define __embedding_enumerator__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include "base_cpp/list.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/red_black.h"
#include "base_cpp/tlscont.h"
#include "graph/graph_fast_access.h"

namespace indigo
{

    class Graph;
    class GraphVertexEquivalence;
    class CancellationHandler;

    class DLLEXPORT EmbeddingEnumerator
    {
    public:
        enum
        {
            UNMAPPED = -1,
            TERM_OUT = -2,
            IGNORE = -3
        };

        bool allow_many_to_one;

        EmbeddingEnumerator(Graph& supergraph);

        ~EmbeddingEnumerator();

        // when cb_embedding returns zero, enumeration stops
        int (*cb_embedding)(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata);

        bool (*cb_match_vertex)(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata);
        bool (*cb_match_edge)(Graph& subgraph, Graph& supergraph, int self_idx, int other_idx, void* userdata);

        void (*cb_vertex_remove)(Graph& subgraph, int sub_idx, void* userdata);
        void (*cb_edge_add)(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);
        void (*cb_vertex_add)(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);
        bool (*cb_allow_many_to_one)(Graph& subgraph, int sub_idx, void* userdata);

        void* userdata;

        void setSubgraph(Graph& subgraph);

        void ignoreSubgraphVertex(int idx);
        void ignoreSupergraphVertex(int idx);

        int countUnmappedSubgraphVertices();
        int countUnmappedSupergraphVertices();

        int countUnmappedSubgraphEdges();
        int countUnmappedSupergraphEdges();

        void setEquivalenceHandler(GraphVertexEquivalence* equivalence_handler);

        bool fix(int node1, int node2);
        bool unsafeFix(int node1, int node2);

        // returns 0 if cb_embedding returned 0, 1 otherwise
        int process();
        void processStart();
        bool processNext();

        const int* getSubgraphMapping();

        const int* getSupergraphMapping();

        // Update internal structures to fit all target vertices that might be added
        void validate();

        DECL_ERROR;
        DECL_TIMEOUT_EXCEPTION;

    protected:
        // digit 1 relates to the subgraph, 2 relates to the supergraph.

        Graph* _g1;
        Graph* _g2;

        GraphVertexEquivalence* _equivalence_handler;

        CP_DECL;

        TL_CP_DECL(Array<int>, _core_1);
        TL_CP_DECL(Array<int>, _core_2);

        TL_CP_DECL(Array<int>, _term2);
        TL_CP_DECL(Array<int>, _unterm2);

        TL_CP_DECL(Pool<RedBlackSet<int>::Node>, _s_pool);

        TL_CP_DECL(GraphFastAccess, _g1_fast);
        TL_CP_DECL(GraphFastAccess, _g2_fast);

        void _terminatePreviousMatch();

        //
        // Query nodes sequence calculation
        //

        struct _QuertMatchState
        {
            _QuertMatchState(int atom_index, int parent_index, int t1_len) : atom_index(atom_index), parent_index(parent_index), t1_len(t1_len)
            {
            }
            int atom_index, parent_index, t1_len;
        };
        TL_CP_DECL(Array<_QuertMatchState>, _query_match_state);

        void _fixNode1(int node1, int node2);
        int _getNextNode1();
        int _t1_len_pre;

        enum
        {
            _RETURN0 = 0,
            _NOWAY = 1,
            _ADD_PAIR = 2
        };

        class _Enumerator
        {
        public:
            _Enumerator(EmbeddingEnumerator& context);
            _Enumerator(const _Enumerator& other);

            bool fix(int node1, int node2, bool safe);
            void setUseEquivalence(bool use);
            void reset();

            int nextPair();
            void addPair(int node1, int node2);
            void restore();

            void initForFirstSearch(int t1_len);

            int _current_node1, _current_node2;

        protected:
            EmbeddingEnumerator& _context;

            bool _use_equivalence;
            RedBlackSet<int> _mapped_orbit_ids;
            int _term2_begin;
            int _unterm2_begin;

            int _core_len;
            int _t1_len, _t2_len;
            int _selected_node1, _selected_node2;
            int _node1_prev_value, _node2_prev_value;

            int _current_node1_idx, _current_node2_idx;
            int _current_node2_parent;
            int _current_node2_nei_index;

            bool _checkNode1(int node1);
            bool _checkNode2(int node2, int for_node1);
            bool _checkPair(int node1, int node2);

            void _initState();

            void _addPairNode2(int node1, int node2);
            void _fixPair(int node1, int node2);
        };

        TL_CP_DECL(ObjArray<_Enumerator>, _enumerators);

        int _cancellation_check_number;
        std::shared_ptr<CancellationHandler> _cancellation_handler;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
