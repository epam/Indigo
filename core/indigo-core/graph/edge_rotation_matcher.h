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

#ifndef __edge_rotation_matcher__
#define __edge_rotation_matcher__

#include "base_cpp/exception.h"

namespace indigo
{

    class Graph;
    struct Vec3f;

    class EdgeRotationMatcher
    {
    public:
        // takes mapping from subgraph to supergraph
        EdgeRotationMatcher(Graph& subgraph, Graph& supergraph, const int* mapping);

        void (*cb_get_xyz)(Graph& graph, int vertex_idx, Vec3f& pos);
        bool (*cb_can_rotate)(Graph& graph, int edge_idx);
        bool equalize_edges;

        bool match(float rsm_threshold, float eps);

        DECL_ERROR;

    protected:
        struct _DirEdge
        {
            int idx, beg, end;
        };

        Graph& _subgraph;
        Graph& _supergraph;
        const int* _mapping;

    private:
        EdgeRotationMatcher(const EdgeRotationMatcher&); // no implicit copy
    };

} // namespace indigo

#endif
