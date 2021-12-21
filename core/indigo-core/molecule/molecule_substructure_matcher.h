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

#ifndef __molecule_substructure_matcher__
#define __molecule_substructure_matcher__

#include "base_cpp/obj.h"
#include "graph/embedding_enumerator.h"
#include "graph/embeddings_storage.h"
#include "molecule/molecule.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_pi_systems_matcher.h"
#include "molecule/query_molecule.h"
#include <memory>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Molecule;
    class AromaticityMatcher;
    struct Vec3f;
    class GraphVertexEquivalence;
    class MoleculeAtomNeighbourhoodCounters;
    class MoleculePiSystemsMatcher;

    class DLLEXPORT MoleculeSubstructureMatcher
    {
    public:
        enum
        {
            AFFINE = 1,
            CONFORMATION = 2
        };

        typedef ObjArray<RedBlackStringMap<int>> FragmentMatchCache;

        MoleculeSubstructureMatcher(BaseMolecule& target);
        ~MoleculeSubstructureMatcher();

        void setQuery(QueryMolecule& query);
        QueryMolecule& getQuery();

        // Set vertex neibourhood counters for effective matching
        void setNeiCounters(const MoleculeAtomNeighbourhoodCounters* query_counters, const MoleculeAtomNeighbourhoodCounters* target_counters);

        // Property indicating that first atom in the query should be ignored because
        // it will be used later. For example, it is fixed during fragment matching
        bool not_ignore_first_atom;

        bool use_aromaticity_matcher;
        bool use_pi_systems_matcher;
        GraphVertexEquivalence* vertex_equivalence_handler;

        AromaticityOptions arom_options;

        FragmentMatchCache* fmcache;

        bool highlight;

        bool disable_unfolding_implicit_h;
        bool disable_folding_query_h;
        bool restore_unfolded_h;

        int match_3d;        // 0 or AFFINE or CONFORMATION
        float rms_threshold; // for AFFINE and CONFORMATION

        void ignoreQueryAtom(int idx);
        void ignoreTargetAtom(int idx);
        bool fix(int query_atom_idx, int target_atom_idx);

        // for finding the first embedding
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
        // Embedding callback. Returns true to continue enumeration.
        bool (*cb_embedding)(Graph& sub, Graph& super, const int* core1, const int* core2, void* context);
        void* cb_embedding_context;

        const GraphEmbeddingsStorage& getEmbeddingsStorage() const;

        static bool needCoords(int match_3d, QueryMolecule& query);

        static void removeAtom(Graph& subgraph, int sub_idx, AromaticityMatcher* am);

        static void addBond(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, AromaticityMatcher* am);

        static void markIgnoredHydrogens(BaseMolecule& mol, int* arr, int value_keep, int value_ignore);
        static void markIgnoredQueryHydrogens(QueryMolecule& mol, int* arr, int value_keep, int value_ignore);

        static void getAtomPos(Graph& graph, int vertex_idx, Vec3f& pos);

        // Flags for matchQueryAtom and matchQueryBond (by default all flags should be set)
        enum
        {
            // When some flags are not set it means that checking should be done without
            // such conditions but as precise as possible because such conditions will be
            // checked later.
            // For example, MATCH_ATOM_CHARGE isn't set.
            // It means that
            // (1) match should return true if it does so for some charge with
            //     MATCH_ATOM_CHARGE set to true.
            // (2) if for every charge match returns false, then without MATCH_ATOM_CHARGE
            //     match should return false.
            // It it not easy to implement point (2) exactly, but match algorithm should
            // always satisfy point (1). So it have to satisfy point (2) as precise as
            // possible.
            MATCH_ATOM_CHARGE = 0x01,
            MATCH_ATOM_VALENCE = 0x02,
            MATCH_BOND_TYPE = 0x04,

            // To satisfy point (2) (not precisely) following flag is introduced.
            // It shows what value should be returned if condition is disabled.
            // When 'not' operation is applied then such flag is inverted. So
            // points (1) is satisfied and point (2) generally satisfied too (not always).
            MATCH_DISABLED_AS_TRUE = 0x1000
        };

        static bool matchQueryAtom(QueryMolecule::Atom* query, BaseMolecule& target, int super_idx, FragmentMatchCache* fmcache, dword flags);

        static bool matchQueryBond(QueryMolecule::Bond* query, BaseMolecule& target, int sub_idx, int super_idx, AromaticityMatcher* am, dword flags);

        static void makeTransposition(BaseMolecule& mol, Array<int>& transposition);

        DECL_ERROR;

        static bool shouldUnfoldTargetHydrogens(QueryMolecule& query, bool find_all_embeddings);

    protected:
        struct MarkushContext
        {
            explicit MarkushContext(QueryMolecule& query_, BaseMolecule& target_);

            CP_DECL;
            TL_CP_DECL(QueryMolecule, query);
            TL_CP_DECL(Array<int>, query_marking);
            TL_CP_DECL(Array<int>, sites);
            int depth;
        };

        static bool _matchAtoms(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata);

        static bool _matchBonds(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);

        static void _removeAtom(Graph& subgraph, int sub_idx, void* userdata);

        static void _addBond(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);

        static int _embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata);

        int _embedding_common(int* core_sub, int* core_super);
        int _embedding_markush(int* core_sub, int* core_super);

        static bool _canUseEquivalenceHeuristic(QueryMolecule& query);
        static bool _isSingleBond(Graph& graph, int edge_idx);

        static bool _shouldUnfoldTargetHydrogens(QueryMolecule& query, bool is_fragment, bool disable_folding_query_h);
        static bool _shouldUnfoldTargetHydrogens_A(QueryMolecule::Atom* atom, bool is_fragment, bool disable_folding_query_h);

        static int _countSubstituents(Molecule& mol, int idx);

        bool _checkRGroupConditions();
        bool _attachRGroupAndContinue(int* core1, int* core2, QueryMolecule* fragment, bool two_attachment_points, int att_idx1, int att_idx2, int rgroup_idx,
                                      bool rest_h);

        void _removeUnfoldedHydrogens();

        BaseMolecule& _target;
        QueryMolecule* _query;

        const MoleculeAtomNeighbourhoodCounters *_query_nei_counters, *_target_nei_counters;

        Obj<EmbeddingEnumerator> _ee;

        std::unique_ptr<MarkushContext> _markush;

        // Because storage can be big it is not stored into TL_CP_***
        // It can be stored as TL_CP_*** if memory allocations will
        // be critical
        Obj<GraphEmbeddingsStorage> _embeddings_storage;

        Obj<Molecule3dConstraintsChecker> _3d_constraints_checker;
        Obj<AromaticityMatcher> _am;
        Obj<MoleculePiSystemsMatcher> _pi_systems_matcher;

        bool _h_unfold; // implicit target hydrogens unfolded

        CP_DECL;
        TL_CP_DECL(Array<int>, _3d_constrained_atoms);
        TL_CP_DECL(Array<int>, _unfolded_target_h);
        TL_CP_DECL(Array<int>, _used_target_h);

        static int _compare_degree_asc(BaseMolecule& mol, int i1, int i2);
        static int _compare_frequency_base(BaseMolecule& mol, int i1, int i2);
        static int _compare_frequency_asc(BaseMolecule& mol, int i1, int i2);
        static int _compare_in_loop(BaseMolecule& mol, int i1, int i2);
        static int _compare(int& i1, int& i2, void* context);

        void _createEmbeddingsStorage();
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
