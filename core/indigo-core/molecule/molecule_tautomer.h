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

#ifndef __molecule_tautomer_h__
#define __molecule_tautomer_h__

#include "base_cpp/array.h"
#include "base_cpp/obj.h"
#include "base_cpp/tlscont.h"
#include "graph/graph_decomposer.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule.h"
#include "molecule/molecule_dearom.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Molecule;
    class AromaticityMatcher;
    class Dearomatizer;
    class DearomatizationMatcher;
    struct Atom;

    typedef enum
    {
        BASIC,
        INCHI,
        RSMARTS
    } TautomerMethod;

    // #define TRACE_TAUTOMER_MATCHING

    struct TautomerRule
    {
        bool check(BaseMolecule& molecule, int first, int last, char other_arom_first, char other_arom_last) const;

        Array<int> list1;
        Array<int> list2;
        int aromaticity1;
        int aromaticity2;

        static bool atomInAromaticRing(BaseMolecule& mol, int atom_idx);
    };

    struct TautomerSearchContext
    {
        explicit TautomerSearchContext(BaseMolecule& g1_, BaseMolecule& g2_, GraphDecomposer& decomposer1_, GraphDecomposer& decomposer2_,
                                       const PtrArray<TautomerRule>& rules_list_, const AromaticityOptions& arom_options);
        virtual ~TautomerSearchContext();

        BaseMolecule& g1;
        BaseMolecule& g2;

        GraphDecomposer& decomposer1;
        GraphDecomposer& decomposer2;

        // amount of metal bonds
        CP_DECL;
        TL_CP_DECL(Array<int>, h_rep_count_1);
        TL_CP_DECL(Array<int>, h_rep_count_2);

        const PtrArray<TautomerRule>& rules_list;

        bool force_hydrogens;
        bool ring_chain;
        int rules;
        TautomerMethod method;
        bool substructure;
        bool (*cb_check_rules)(TautomerSearchContext& context, int first1, int first2, int last1, int last2);

        int max_chains;

        AromaticityOptions arom_options;

        TL_CP_DECL(DearomatizationsStorage, dearomatizations);

        TL_CP_DECL(Array<int>, core_1);
        TL_CP_DECL(Array<int>, core_2);

        int initial_g1_vertexend;

        TL_CP_DECL(Array<int>, chains_2);

        TL_CP_DECL(Array<int>, edges_1);
        TL_CP_DECL(Array<int>, edges_2);
        TL_CP_DECL(Array<int>, edge_types_2);

        TL_CP_DECL(Array<int>, n1);
        TL_CP_DECL(Array<int>, n2);

        Obj<Dearomatizer> dearomatizer;
        Obj<DearomatizationMatcher> dearomatizationMatcher;
    };

    class TautomerMatcher
    {
    public:
        explicit TautomerMatcher(TautomerSearchContext& context);
        explicit TautomerMatcher(TautomerSearchContext& context, int start_path_number, int n_chains);
        virtual ~TautomerMatcher();

        bool findMatch();
        void addPair(int n1, int n2, int arom_bond_idx2, int bond_type2);
        bool nextPair(int& n1, int& n2, int& h_diff, int prev_n1, int prev_n2);
        bool isFeasiblePair(int n1, int n2, int& h_diff);
        void restore();

        static bool matchAtomsTau(BaseMolecule& g1, BaseMolecule& g2, int n1, int n2);
        static bool matchBondsTau(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);
        static bool matchBondsTauSub(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);

        static bool fixBondsNotInChains(TautomerSearchContext& context, const int* core1, const int* core2);

    private:
        bool _checkInterPathBonds();

        static int _remainderEmbedding(Graph& g1, Graph& g2, int* core1, int* core2, void* userdata);
        static int _preliminaryEmbedding(Graph& g1, Graph& g2, int* core1, int* core2, void* userdata);

        static bool _matchAtoms(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata);
        static bool _matchAtomsEx(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata);

        struct MatchData
        {
            MatchData(TautomerSearchContext& context_) : context(context_)
            {
            }

            TautomerSearchContext& context;
            int start_path_number;
        } _d;

        int _n1;
        int _n2;
        int _bond_idx2;
        int _n_chains;
    };

    class TautomerChainFinder
    {
    public:
        explicit TautomerChainFinder(TautomerSearchContext& context, int h_difference, int start_path_number, int n_chains);
        explicit TautomerChainFinder(TautomerChainFinder& other);
        virtual ~TautomerChainFinder();

        bool enumeratePaths();
        void addPair(int n1, int n2, bool is_zero_bond, int arom_bond_idx2, int bond_type2);
        void restore();

        bool nextPair(int& n1, int& n2, int& e1, int& e2, int prev_e1, int prev_e2);
        int isFeasiblePair(int n1, int n2, bool& zero_bond, int& arom_bond_idx2, int& bond_type2);

    private:
        TautomerSearchContext& _context;

        int _prev_n1;
        int _prev_n2;
        int _bond_idx2;

        int _path_length;

        int _h_difference;

        bool _is_zero_bond_present;

        int _path_number;

        int _start_idx1;
        int _start_idx2;

        int _n_chains;
    };

    class TautomerChainChecker
    {
    public:
        explicit TautomerChainChecker(TautomerSearchContext& context, const Array<int>& core1, const Array<int>& core2, int start_path_number);
        explicit TautomerChainChecker(TautomerChainChecker& other);
        virtual ~TautomerChainChecker();

        bool check();
        void addPair(int n1, int n2);
        void restore();

        bool nextStartingPair(int& n1, int& n2);
        bool isFeasibleStartingPair(int n1, int n2, int& h_diff);

        bool nextPair(int& n1, int& n2, int& e1, int& e2);
        int isFeasiblePair(int n1, int n2, TautomerChainChecker& next1, TautomerChainChecker& next2);

        bool releaseChain();
        void restoreChain();

        DECL_ERROR;

    private:
        bool _checkInterPathBonds();

        static bool _matchAromBonds(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);
        static void _removeAtom(Graph& subgraph, int sub_idx, void* userdata);
        static void _addBond(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);
        static int _embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata);
        bool _matchAromatizedQuery();

        TautomerSearchContext& _context;

        int _path_length;
        int _h_difference;

        // at the end of chain
        int _final_path_length;
        int _final_h_difference;

        bool _is_zero_bond_present;
        bool _is_query_bond_present;
        bool _is_non_aromatic_bond_present;

        int _path_number;

        const Array<int>& _core_1;
        const Array<int>& _core_2;

        int _tau_bonds_to_match;

        int _prev_n1;
        int _prev_n2;

        int _bond_idx1;
        int _bond_idx2;
        int _bond_type2;

        int _start_idx1;
        int _start_idx2;
    };

    class DLLEXPORT TautomerSuperStructure : public Molecule
    {
    public:
        enum
        {
            NONE,
            TAUTOMER,
            ORIGINAL
        };

        TautomerSuperStructure(Molecule& mol);
        ~TautomerSuperStructure() override;

        void clear() override;

        int getBondOrder(int idx) const override;
        int getBondTopology(int idx) override;
        bool possibleBondOrder(int idx, int order) override;

        int getAtomTotalH(int idx) override;

        bool isZeroedBond(int idx);

        const int* getMapping();
        const Array<int>& getInvMapping();

        int getSubgraphType(const Array<int>& vertices, const Array<int>& edges);

    protected:
        void _findMinDistance(int source, int maxDist, Array<int>& dest, int* result);

        void _collectAtomProperties(void);
        void _getDoubleBondsCount(int i, int& double_count, int& arom_count);
        bool _isAcceptingHeteroatom(int idx);
        bool _isEmittingHeteroatom(int idx);
        int _hetroatomsCount(int idx);

        int _getBondOrder(int idx);
        int _getBondTopology(int idx);

        bool _inside_ctor;

        CP_DECL;
        TL_CP_DECL(Array<int>, _atomsEmitBond);
        TL_CP_DECL(Array<int>, _atomsAcceptBond);
        TL_CP_DECL(Array<bool>, _isBondAttachedArray);
        TL_CP_DECL(Array<int>, _mapping);
        TL_CP_DECL(Array<int>, _inv_mapping);
        TL_CP_DECL(Array<int>, _edge_mapping);
        TL_CP_DECL(Array<int>, _total_h);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
