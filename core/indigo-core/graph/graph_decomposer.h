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

#ifndef __graph_decomposer__
#define __graph_decomposer__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"

#include <list>
#include <unordered_set>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Graph;
    class Filter;

    class DLLEXPORT GraphDecomposer
    {
    public:
        GraphDecomposer(const Graph& graph);
        ~GraphDecomposer();

        // returns the amount of connected components
        int decompose(const Filter* filter = NULL, const Filter* edge_filter = NULL, const std::list<std::unordered_set<int>>* ext_neighbours = NULL);

        const Array<int>& getDecomposition() const;

        int getComponent(int vertex) const;
        int getComponentsCount() const;

        int getComponentVerticesCount(int component) const;
        int getComponentEdgesCount(int component) const;

        DECL_ERROR;

    protected:
        const Graph& _graph;
        int n_comp;

        CP_DECL;
        TL_CP_DECL(Array<int>, _component_ids);
        TL_CP_DECL(Array<int>, _component_vertices_count);
        TL_CP_DECL(Array<int>, _component_edges_count);

    private:
        GraphDecomposer(const GraphDecomposer&);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
