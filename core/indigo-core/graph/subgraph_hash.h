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

#ifndef __subgraph_hash__
#define __subgraph_hash__

#include "base_cpp/array.h"
#include "base_cpp/tlscont.h"
#include "graph/graph_fast_access.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Graph;

    class DLLEXPORT SubgraphHash
    {
    public:
        SubgraphHash(Graph& g);

        int max_iterations;
        bool calc_different_codes_count;

        dword getHash();
        dword getHash(const Array<int>& vertices, const Array<int>& edges);

        int getDifferentCodesCount();

        const Array<int>*vertex_codes, *edge_codes;

    private:
        Graph& _g;
        int _different_codes_count;

        CP_DECL;
        TL_CP_DECL(Array<dword>, _codes);
        TL_CP_DECL(Array<dword>, _oldcodes);
        TL_CP_DECL(GraphFastAccess, _gf);

        TL_CP_DECL(Array<int>, _default_vertex_codes);
        TL_CP_DECL(Array<int>, _default_edge_codes);
    };

} // namespace indigo

#endif // __subgraph_hash__
