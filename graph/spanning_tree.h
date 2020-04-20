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

#ifndef __spanning_tree_h__
#define __spanning_tree_h__

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "graph/graph.h"

namespace indigo
{

    class Filter;

    class SpanningTree
    {
    public:
        struct ExtEdge
        {
            int beg_idx;
            int end_idx;
            int ext_beg_idx;
            int ext_end_idx;
            int ext_edge_idx;
        };

        explicit SpanningTree(Graph& graph, const Filter* vertex_filter, const Filter* edge_filter = 0);

        inline int getEdgesNum()
        {
            return _edges_list.size();
        }

        inline const ExtEdge& getExtEdge(int i)
        {
            return _edges_list[i];
        }

        void addEdge(int beg, int end, int ext_index);

        inline const Vertex& getVertexFromExtIdx(int ext_idx) const
        {
            return _tree.getVertex(_inv_mapping[ext_idx]);
        }

        inline int getExtVertexIndex(int v_idx) const
        {
            return _mapping[v_idx];
        }

        inline int getExtEdgeIndex(int e_idx) const
        {
            return _edge_mapping[e_idx];
        }

        void markAllEdgesInCycles(int* marks_out, int value);

        DECL_ERROR;

    protected:
        struct StackElem
        {
            const Vertex* vertex;
            int vertex_idx;
            int nei_idx;
            int parent_idx;
        };

        void _build();

        const Graph& _graph;
        const Filter* _vertex_filter;
        const Filter* _edge_filter;

        // these members made static for saving time of memory allocations
        CP_DECL;
        TL_CP_DECL(Array<ExtEdge>, _edges_list);
        TL_CP_DECL(Array<int>, _depth_counters);
        TL_CP_DECL(Graph, _tree);
        TL_CP_DECL(Array<int>, _mapping);
        TL_CP_DECL(Array<int>, _inv_mapping);
        TL_CP_DECL(Array<int>, _edge_mapping);
        TL_CP_DECL(Array<StackElem>, _stack);

        int _current_depth;
    };

} // namespace indigo

#endif
