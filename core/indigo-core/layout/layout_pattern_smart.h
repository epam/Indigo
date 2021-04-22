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

#ifndef __layout_pattern_smart_h__
#define __layout_pattern_smart_h__

#include "base_c/defs.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class MoleculeLayoutGraphSmart;
    class Graph;

    class DLLEXPORT PatternLayoutFinder
    {
    public:
        static bool tryToFindPattern(MoleculeLayoutGraphSmart& layout_graph);

    private:
        static void _initPatterns();
        static bool _matchPatternBond(Graph& subgraph, Graph& supergraph, int self_idx, int other_idx, void* userdata);
        static bool _matchPatternAtom(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
