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

#ifndef _max_common_submolecule
#define _max_common_submolecule
#include "base_cpp/scanner.h"
#include "graph/max_common_subgraph.h"
#include "molecule/molecule.h"
#include "molecule/molfile_loader.h"
#include "molecule/molfile_saver.h"
#include "time.h"

namespace indigo
{

    class DLLEXPORT MaxCommonSubmolecule : public MaxCommonSubgraph
    {
    public:
        MaxCommonSubmolecule(BaseMolecule& submol, BaseMolecule& supermol);

        static bool matchBonds(Graph& g1, Graph& g2, int i, int j, void* userdata);
        static bool matchAtoms(Graph& g1, Graph& g2, const int* core_sub, int i, int j, void* userdata);
    };

} // namespace indigo

#endif
