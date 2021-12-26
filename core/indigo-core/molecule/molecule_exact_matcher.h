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

#ifndef __molecule_exact_matcher__
#define __molecule_exact_matcher__

#include "base_cpp/obj.h"
#include "base_cpp/scanner.h"
#include "graph/embedding_enumerator.h"
#include "graph/filter.h"
#include "graph/graph_affine_matcher.h"
#include "graph/graph_decomposer.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cis_trans.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/molecule_substructure_matcher.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Molecule;

    class DLLEXPORT MoleculeExactMatcher
    {
    public:
        enum
        {
            // Conditions
            CONDITION_NONE = 0x0000,
            CONDITION_ELECTRONS = 0x0001, // bond types, atom charges, valences, radicals must match
            CONDITION_ISOTOPE = 0x0002,   // atom isotopes must match
            CONDITION_STEREO = 0x0004,    // tetrahedral and cis-trans configurations must match
            CONDITION_FRAGMENTS = 0x0008, // query fragments count must be equal to target fragments count
            CONDITION_ALL = 0x000F,       // all but 3D
            CONDITION_3D = 0x0010         // atom positions must match up to affine+scale transformation
        };

        MoleculeExactMatcher(BaseMolecule& query, BaseMolecule& target);

        bool find();

        const int* getQueryMapping();
        const int* getTargetMapping();
        void ignoreTargetAtom(int idx);
        void ignoreQueryAtom(int idx);

        static void parseConditions(const char* params, int& flags, float& rms_threshold);

        static bool matchAtoms(BaseMolecule& query, BaseMolecule& target, int sub_idx, int super_idx, int flags);
        static bool matchBonds(BaseMolecule& query, BaseMolecule& target, int sub_idx, int super_idx, int flags);

        int flags;
        float rms_threshold; // for affine match

        bool needCoords();

        DECL_ERROR;

    protected:
        BaseMolecule& _query;
        BaseMolecule& _target;
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
        MoleculeExactMatcher(const MoleculeExactMatcher&);
    };

} // namespace indigo

#endif
