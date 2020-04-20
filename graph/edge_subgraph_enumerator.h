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

#ifndef __edge_subgraph_enumerator__
#define __edge_subgraph_enumerator__

#include "base_cpp/tlscont.h"
#include "graph/graph.h"

namespace indigo
{

    class EdgeSubgraphEnumerator
    {
    public:
        explicit EdgeSubgraphEnumerator(Graph& graph);

        int min_edges;
        int max_edges;

        void process();

        void (*cb_subgraph)(Graph& graph, const int* vertices, const int* edges, void* userdata);

        void* userdata;

        DECL_ERROR;

    protected:
        int _fCIS();

        Graph& _graph;

        CP_DECL;
        TL_CP_DECL(Graph, _subgraph);

        TL_CP_DECL(Array<int>, _mapping);          // subgraph -> graph
        TL_CP_DECL(Array<int>, _inv_mapping);      // graph -> subgraph
        TL_CP_DECL(Array<int>, _edge_mapping);     // subgraph -> graph
        TL_CP_DECL(Array<int>, _inv_edge_mapping); // graph -> subgraph

        TL_CP_DECL(Pool<List<int>::Elem>, _pool);
        TL_CP_DECL(Array<int>, _adjacent_edges);

        class _Enumerator
        {
        public:
            _Enumerator(EdgeSubgraphEnumerator& context);
            _Enumerator(const _Enumerator& other);

            void process();

        protected:
            EdgeSubgraphEnumerator& _context;

            Graph& _graph;
            Graph& _subgraph;

            void _addEdgeToSubgraph(int edge_idx);
            void _removeAddedEdge();
            void _addAdjacentEdge(int edge_idx);
            void _removeAdjacentEdges();

            int _added_vertex;
            int _added_edge;
            List<int> _adjacent_edges_added;
        };
    };

} // namespace indigo

#endif
