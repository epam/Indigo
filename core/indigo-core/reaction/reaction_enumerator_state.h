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

#ifndef __reaction_enumerator_state__
#define __reaction_enumerator_state__

#include "base_cpp/obj.h"
#include "base_cpp/red_black.h"
#include "base_cpp/reusable_obj_array.h"
#include "graph/embedding_enumerator.h"
#include "molecule/molecule.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/query_molecule.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"

namespace indigo
{

    class ReactionEnumeratorContext
    {
    public:
        AromaticityOptions arom_options;
    };

    class ReactionEnumeratorState
    {
    public:
        DECL_ERROR;

        class ReactionMonomers
        {
        public:
            DECL_ERROR;

            CP_DECL;
            TL_CP_DECL(PtrArray<Molecule>, _monomers);
            TL_CP_DECL(Array<int>, _reactant_indexes);
            TL_CP_DECL(Array<int>, _deep_levels);
            TL_CP_DECL(Array<int>, _tube_indexes);

            ReactionMonomers();

            int size();

            void clear();

            Molecule& getMonomer(int reactant_idx, int index);

            Molecule& getMonomer(int mon_index);

            void addMonomer(int reactant_idx, Molecule& monomer, int deep_level = 0, int tube_idx = -1);

            void removeMonomer(int idx);
        };

        bool (*refine_proc)(const Molecule& uncleaned_fragments, Molecule& product, Array<int>& mapping, void* userdata);
        void (*product_proc)(Molecule& product, Array<int>& monomers_indices, Array<int>& mapping, void* userdata);

        void* userdata;
        bool is_multistep_reaction;
        bool is_self_react;
        bool is_one_tube;
        bool is_same_keeping;
        bool is_transform;

        int max_deep_level;
        int max_product_count;
        int max_reuse_count;

        ReactionEnumeratorState(ReactionEnumeratorContext& context, QueryReaction& cur_reaction, QueryMolecule& cur_full_product,
                                Array<int>& cur_product_aam_array, RedBlackStringMap<int>& cur_smiles_array, ReactionMonomers& cur_reaction_monomers,
                                int& cur_product_coint, ObjArray<Array<int>>& cur_tubes_monomers);

        ReactionEnumeratorState(ReactionEnumeratorState& cur_rpe_state);

        int buildProduct(void);

        bool performSingleTransformation(Molecule& molecule, Array<int>& mapping, Array<int>& forbidden_atoms, Array<int>& original_hydrogens,
                                         bool& need_layout);

    private:
        ReactionEnumeratorContext& _context;

        QueryReaction& _reaction;
        int _reactant_idx;

        int _is_simple_transform;

        int& _product_count;

        ObjArray<Array<int>>& _tubes_monomers;
        Array<int>& _product_aam_array;
        RedBlackStringMap<int>& _smiles_array;
        ReactionMonomers& _reaction_monomers;

        CP_DECL;
        TL_CP_DECL(Array<int>, _fragments_aam_array);
        TL_CP_DECL(QueryMolecule, _full_product);
        TL_CP_DECL(Array<int>, _product_monomers);
        TL_CP_DECL(Array<int>, _mapping);
        TL_CP_DECL(Molecule, _fragments);
        TL_CP_DECL(Array<int>, _is_needless_atom);
        TL_CP_DECL(Array<int>, _is_needless_bond);
        TL_CP_DECL(Array<int>, _bonds_mapping_sub);
        TL_CP_DECL(Array<int>, _bonds_mapping_super);
        TL_CP_DECL(ObjArray<Array<int>>, _att_points);
        TL_CP_DECL(MoleculeSubstructureMatcher::FragmentMatchCache, _fmcache);
        TL_CP_DECL(Array<int>, _monomer_forbidden_atoms);
        TL_CP_DECL(Array<int>, _product_forbidden_atoms);

        TL_CP_DECL(Array<int>, _original_hydrogens);

        AromaticityMatcher* _am;
        EmbeddingEnumerator* _ee;
        int _tube_idx;
        int _deep_level;
        bool _is_frag_search;
        bool _is_rg_exist;

        int _findCurTube(void);

        bool _isMonomerFromCurTube(int monomer_idx);

        static void _foldHydrogens(BaseMolecule& molecule, Array<int>* atoms_to_keep = 0, Array<int>* original_hydrogens = 0, Array<int>* mol_mapping = 0);

        void _productProcess(void);

        bool _checkForSimplicity()
        {
            if (_reaction.reactantsCount() != 1 || _reaction.productsCount() != 1)
                return false;

            QueryMolecule& reactant = _reaction.getQueryMolecule(_reaction.reactantBegin());
            QueryMolecule& product = _reaction.getQueryMolecule(_reaction.productBegin());

            if ((reactant.vertexCount() != product.vertexCount()) || (reactant.edgeCount() != product.edgeCount()))
                return false;

            Array<int>& reactant_aam = _reaction.getAAMArray(_reaction.reactantBegin());
            Array<int>& product_aam = _reaction.getAAMArray(_reaction.productBegin());

            Array<int> aam_mapping;
            aam_mapping.resize(reactant.vertexEnd());
            aam_mapping.fffill();

            for (int i = reactant.vertexBegin(); i != reactant.vertexEnd(); i = reactant.vertexNext(i))
            {
                if (reactant_aam[i] == 0)
                    return false;

                int product_idx = product_aam.find(reactant_aam[i]);

                if (product_idx == -1)
                    return false;

                aam_mapping[i] = product_idx;
            }

            for (int i = reactant.edgeBegin(); i != reactant.edgeEnd(); i = reactant.edgeNext(i))
            {
                const Edge& edge = reactant.getEdge(i);

                int product_beg = aam_mapping[edge.beg];
                int product_end = aam_mapping[edge.end];

                if (product_beg == -1 || product_end == -1)
                    return false;

                if (product.findEdgeIndex(product_beg, product_end) == -1)
                    return false;

                if (!MoleculeCisTrans::isGeomStereoBond(reactant, i, NULL, false))
                    continue;

                int ct_sign = MoleculeCisTrans::getMappingParitySign(reactant, product, i, aam_mapping.ptr());

                if (ct_sign <= 0)
                    return false;
            }

            if (!MoleculeStereocenters::checkSub(reactant, product, aam_mapping.ptr(), false))
                return false;

            return true;
        }

        bool _nextMatchProcess(EmbeddingEnumerator& ee, const QueryMolecule& reactant, const Molecule& monomer);

        int _calcMaxHCnt(QueryMolecule& molecule);

        bool _startEmbeddingEnumerator(Molecule& monomer);

        void _changeQueryNode(QueryMolecule& ee_reactant, int change_atom_idx);

        void _findFragAtoms(Array<byte>& unfrag_mon_atoms, QueryMolecule& submolecule, Molecule& fragment, int* core_sub, int* core_super);

        void _cleanFragments(void);

        void _findR2PMapping(QueryMolecule& reactant, Array<int>& mapping);

        void _invertStereocenters(Molecule& molecule, int edge_idx);

        void _cistransUpdate(QueryMolecule& submolecule, Molecule& supermolecule, int* frag_mapping, const Array<int>& rp_mapping, int* core_sub);

        QueryMolecule::Atom* _getReactantAtom(int atom_aam);

        void _buildMolProduct(QueryMolecule& product, Molecule& mol_product, Molecule& uncleaned_fragments, Array<int>& all_forbidden_atoms,
                              Array<int>& mapping_out);

        void _stereocentersUpdate(QueryMolecule& submolecule, Molecule& supermolecule, const Array<int>& rp_mapping, int* core_sub, int* core_super);

        void _findFragments2ProductMapping(Array<int>& f2p_mapping);

        void _completeCisTrans(Molecule& product, Molecule& uncleaned_fragments, Array<int>& frags_mapping);

        bool _checkValence(Molecule& mol, int atom_idx);

        bool _attachFragments(Molecule& ready_product_out, Array<int>& ucfrag_mapping);

        bool _checkFragment(QueryMolecule& submolecule, Molecule& monomer, Array<byte>& unfrag_mon_atoms, int* core_sub);

        void _checkFragmentNecessity(Array<int>& is_needless_att_point);

        bool _addFragment(Molecule& fragment, QueryMolecule& submolecule, Array<int>& rp_mapping, const Array<int>& sub_rg_atoms, int* core_sub,
                          int* core_super);

        static bool _matchVertexCallback(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata);

        static bool _matchEdgeCallback(Graph& subgraph, Graph& supergraph, int self_idx, int other_idx, void* userdata);

        static bool _allowManyToOneCallback(Graph& subgraph, int sub_idx, void* userdata);

        static void _removeAtomCallback(Graph& subgraph, int sub_idx, void* userdata);

        static void _addBondCallback(Graph& subgraph, Graph& supergraph, int self_idx, int other_idx, void* userdata);

        static bool _checkForNeverUsed(ReactionEnumeratorState* rpe_state, Molecule& supermolecule);

        static int _embeddingCallback(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata);
    };

} // namespace indigo

#endif /* __reaction_enumerator_state__ */
