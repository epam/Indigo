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

#ifndef __molecule_tautomer_substructure_matcher__
#define __molecule_tautomer_substructure_matcher__

#include "base_cpp/obj.h"
#include "graph/embedding_enumerator.h"
#include "graph/embeddings_storage.h"
#include "molecule/molecule.h"
#include "molecule/molecule_layered_molecules.h"
#include "molecule/molecule_tautomer.h"
#include "molecule/molecule_tautomer_enumerator.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class DLLEXPORT MoleculeTautomerSubstructureMatcher
    {
    public:
        enum
        {
            AFFINE = 1,
            CONFORMATION = 2
        };

        typedef ObjArray<RedBlackStringMap<int>> FragmentMatchCache;

        MoleculeTautomerSubstructureMatcher(BaseMolecule& target, TautomerMethod method);
        ~MoleculeTautomerSubstructureMatcher();

        void setQuery(QueryMolecule& query);

        bool find();
        bool findNext();
        const int* getQueryMapping();
        const int* getTargetMapping();

        // Finding all embeddings and iterating them.
        // Substructure matcher can be used in 3 ways:
        // 1. Find first embedding
        // 2. Save all embeddings.
        // 3. Iterate over all embeddings.
        bool find_all_embeddings,   // false by default
            find_unique_embeddings, // true if to find only unique embeddings. false by default
            find_unique_by_edges,   // true if to find edges-unique embeddings. false by default
            save_for_iteration;     // true if to save embeddings to the embeddings storage. false by default

        const GraphEmbeddingsStorage& getEmbeddingsStorage() const;
        const Dbitset& getMask(int ind) const;
        void getTautomerFound(Molecule& mol, int enumInd, int tauInd) const;

        DECL_ERROR;

    protected:
        int _embedding_common(int* core_sub, int* core_super, Dbitset& mask);
        QueryMolecule* _query;
        TautomerEnumerator _tautomerEnumerator;

        Obj<EmbeddingEnumerator> _ee;

        // Because storage can be big it is not stored into TL_CP_***
        // It can be stored as TL_CP_*** if memory allocations will
        // be critical
        Obj<GraphEmbeddingsStorage> _embeddings_storage;
        ObjArray<Dbitset> _masks;

        CP_DECL;

        void _createEmbeddingsStorage();

        struct SubstructureSearchBreadcrumps
        {
            Dbitset mask;
            ObjArray<Dbitset> maskHistory;
            MoleculeTautomerSubstructureMatcher* self;
        };

        SubstructureSearchBreadcrumps _breadcrumps;
        bool _needAromatize;
        bool _allLayersFound;
        int _layerBeg;
        int _layerEnd;

        static bool _matchAtoms(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata);
        static bool _matchAtomsHyper(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata);
        static bool _matchBondsSubHyper(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);
        static void _edgeAddHyper(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);
        static void _vertexRemoveHyper(Graph& subgraph, int sub_idx, void* userdata);
        static int _preliminaryEmbeddingHyper(Graph& g1, Graph& g2, int* core1, int* core2, void* userdata);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
