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

#ifndef __chain_enumerator_h__
#define __chain_enumerator_h__

#include "base_cpp/tlscont.h"
#include "graph/graph.h"

namespace indigo
{
    class GraphSubchainEnumerator
    {
    public:
        enum
        {
            MODE_NO_DUPLICATE_VERTICES = 0,
            MODE_NO_BACKTURNS = 1,
            MODE_NO_CONSTRAINTS = 2
        };

        explicit GraphSubchainEnumerator(Graph& graph, int min_edges, int max_edges, int mode);
        virtual ~GraphSubchainEnumerator();

        void* context;

        void (*cb_handle_chain)(Graph& graph, int size, const int* vertices, const int* edges, void* context);

        void processChains();

    protected:
        void _DFS(int from);

        Graph& _graph;
        int _max_edges;
        int _min_edges;
        int _mode;

        CP_DECL;
        TL_CP_DECL(Array<int>, _vertex_states);
        TL_CP_DECL(Array<int>, _chain_vertices);
        TL_CP_DECL(Array<int>, _chain_edges);
    };

} // namespace indigo

#endif
