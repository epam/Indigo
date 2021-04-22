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

#ifndef __graph_affine_matcher__
#define __graph_affine_matcher__

#include "base_cpp/array.h"

namespace indigo
{

    class Graph;
    struct Vec3f;

    class GraphAffineMatcher
    {
    public:
        // takes mapping from subgraph to supergraph
        GraphAffineMatcher(Graph& subgraph, Graph& supergraph, const int* mapping);

        bool match(float rms_threshold);

        void (*cb_get_xyz)(Graph& graph, int vertex_idx, Vec3f& pos);

        const Array<int>* fixed_vertices;

        DECL_ERROR;

    protected:
        Graph& _subgraph;
        Graph& _supergraph;
        const int* _mapping;

    private:
        GraphAffineMatcher(const GraphAffineMatcher&); // guess what? tip: look at any other class
    };

} // namespace indigo

#endif
