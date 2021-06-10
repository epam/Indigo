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

#ifndef __molecule_exact_substructure_matcher__
#define __molecule_exact_substructure_matcher__

#include "base_cpp/obj.h"
#include "graph/embedding_enumerator.h"
#include "graph/graph_decomposer.h"

namespace indigo
{

    class Molecule;

    class MoleculeExactSubstructureMatcher
    {
    public:
        MoleculeExactSubstructureMatcher(Molecule& query, Molecule& target);

        bool find();
        bool find_withHydrogens();
        bool findNext();

        const int* getQueryMapping();
        void ignoreTargetAtom(int idx);
        void ignoreQueryAtom(int idx);

        dword flags;

        DECL_ERROR;

    protected:
        Molecule& _query;
        Molecule& _target;
        EmbeddingEnumerator _ee;
        Obj<GraphDecomposer> _query_decomposer;
        Obj<GraphDecomposer> _target_decomposer;

        struct _MatchToken
        {
            bool compare(const char* text) const;

            const char* t_text;
            int t_flag;
        };

        static bool _matchAtoms(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata);
        static bool _matchBonds(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);
        static int _embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata);

        void _collectConnectedComponentsInfo();

    private:
        MoleculeExactSubstructureMatcher(const MoleculeExactSubstructureMatcher&);
    };

} // namespace indigo

#endif
