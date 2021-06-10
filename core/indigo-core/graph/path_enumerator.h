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

#ifndef __path_enumerator_h__
#define __path_enumerator_h__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

namespace indigo
{
    class Graph;

    class PathEnumerator
    {
    public:
        explicit PathEnumerator(Graph& graph, int begin, int end);
        ~PathEnumerator();

        int max_length;
        void* context;

        bool (*cb_check_vertex)(Graph& graph, int v_idx, void* context);
        bool (*cb_check_edge)(Graph& graph, int e_idx, void* context);
        bool (*cb_handle_path)(Graph& graph, const Array<int>& vertices, const Array<int>& edges, void* context);

        void process();

        DECL_TIMEOUT_EXCEPTION;

    protected:
        bool _pathFinder();

        Graph& _graph;
        int _begin;
        int _end;

    private:
        PathEnumerator(const PathEnumerator&); // no implicit copy
    };

} // namespace indigo

#endif
