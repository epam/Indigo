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

#ifndef __biconnected_decomposer_h__
#define __biconnected_decomposer_h__

#include "base_cpp/ptr_array.h"
#include "base_cpp/tlscont.h"
#include "graph/graph.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT BiconnectedDecomposer
    {
    public:
        explicit BiconnectedDecomposer(const Graph& graph);
        virtual ~BiconnectedDecomposer();

        // returns the amount of biconnected components
        int decompose();

        int componentsCount();

        bool isArticulationPoint(int idx) const;
        void getComponent(int idx, Filter& filter) const;
        const Array<int>& getIncomingComponents(int idx) const;
        int getIncomingCount(int idx) const;
        void getVertexComponents(int idx, Array<int>& components) const;

        DECL_ERROR;

    protected:
        void _biconnect(int v, int u);

        bool _pushToStack(Array<int>& dfs_stack, int v);
        void _processIfNotPushed(Array<int>& dfs_stack, int w);

        const Graph& _graph;
        CP_DECL;
        TL_CP_DECL(PtrArray<Array<int>>, _components); // masks for components
        TL_CP_DECL(Array<int>, _dfs_order);
        TL_CP_DECL(Array<int>, _lowest_order);
        TL_CP_DECL(PtrArray<Array<int>>, _component_lists);
        TL_CP_DECL(Array<Array<int>*>, _component_ids); // list of components for articulation point
        TL_CP_DECL(Array<Edge>, _edges_stack);
        int _cur_order;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
