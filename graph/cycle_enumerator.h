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

#ifndef __cycle_enumerator_h__
#define __cycle_enumerator_h__

#include "base_cpp/array.h"

namespace indigo
{

    class Graph;
    class SpanningTree;
    class Filter;

    class CycleEnumerator
    {
    public:
        explicit CycleEnumerator(Graph& graph);
        ~CycleEnumerator();

        int min_length;
        int max_length;
        void* context;

        Filter* vfilter;

        bool (*cb_check_vertex)(Graph& graph, int v_idx, void* context);
        bool (*cb_handle_cycle)(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context);

        bool process();

    protected:
        bool _pathFinder(const SpanningTree& spt, int ext_v1, int ext_v2, int ext_e);
        Graph& _graph;

    private:
        CycleEnumerator(const CycleEnumerator&); // no implicit copy
    };

} // namespace indigo
#endif
