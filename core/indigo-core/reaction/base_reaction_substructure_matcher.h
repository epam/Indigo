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

#ifndef __base_reaction__substructure_matcher__
#define __base_reaction__substructure_matcher__

#include "base_cpp/obj.h"
#include "base_cpp/red_black.h"
#include "base_cpp/tlscont.h"
#include "graph/embedding_enumerator.h"
#include "molecule/molecule_arom_match.h"
#include <map>
#include <memory>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Reaction;
    class ReactionAtomNeighbourhoodCounters;
    class BaseReaction;
    class Graph;
    class BaseMolecule;
    class Molecule;

    typedef std::map<int, int> StdIntMap;

    class DLLEXPORT BaseReactionSubstructureMatcher
    {
    protected:
        class _Matcher;

    public:
        BaseReactionSubstructureMatcher(Reaction& target);

        void setQuery(BaseReaction& query);
        void setNeiCounters(const ReactionAtomNeighbourhoodCounters* query_counters, const ReactionAtomNeighbourhoodCounters* target_counters);

        bool highlight;
        bool use_aromaticity_matcher;

        AromaticityOptions arom_options;

        bool find();

        int getTargetMoleculeIndex(int query_molecule_idx);
        const int* getQueryMoleculeMapping(int query_mol_idx);

        DECL_ERROR;

        bool (*match_atoms)(BaseReaction& query_, Reaction& target, int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx, void* context);

        bool (*match_bonds)(BaseReaction& query_, Reaction& target, int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
                            AromaticityMatcher* am, void* context);

        void (*remove_atom)(BaseMolecule& submol, int sub_idx, AromaticityMatcher* am);

        void (*add_bond)(BaseMolecule& submol, Molecule& supermol, int sub_idx, int super_idx, AromaticityMatcher* am);

        bool (*prepare)(BaseReaction& query_, Reaction& target, void* context);

        bool (*prepare_ee)(EmbeddingEnumerator& ee, BaseMolecule& submol, Molecule& supermol, void* context);

        void* context;

    protected:
        void _initMap(BaseReaction& reaction, int side, std::map<int, int>& aam_map);
        virtual bool _checkAAM();
        void _highlight();
        bool _match_stereo;

        enum
        {
            _FIRST_SIDE,
            _SECOND_SIDE,
            _SECOND_SIDE_REST,
            _CONTINUE,
            _NO_WAY,
            _RETURN
        };

        class _Matcher
        {
        public:
            _Matcher(BaseReactionSubstructureMatcher& context);
            _Matcher(const _Matcher& other);

            int nextPair();
            void setMode(int mode)
            {
                _mode = mode;
            }
            int getMode()
            {
                return _mode;
            }
            bool addPair(int mol1_idx, int mol2_idx, const Array<int>& core1, const Array<int>& core2, bool from_first_side);
            void restore();

            bool match_stereo;

            int _current_molecule_1, _current_molecule_2;

            CP_DECL;
            TL_CP_DECL(Array<int>, _current_core_1);
            TL_CP_DECL(Array<int>, _current_core_2);

        protected:
            int _nextPair();

            bool _initEnumerator(BaseMolecule& mol_1, Molecule& mol_2);

            static int _embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata);
            static bool _matchAtoms(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata);
            static bool _matchBonds(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);
            static void _removeAtom(Graph& subgraph, int sub_idx, void* userdata);
            static void _addBond(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata);

            BaseReactionSubstructureMatcher& _context;
            std::unique_ptr<AromaticityMatcher> _am;

            Obj<EmbeddingEnumerator> _enumerator;
            int _mode;
            int _selected_molecule_1;
            int _selected_molecule_2;
            TL_CP_DECL(Array<int>, _mapped_aams);
        };

        Reaction& _target;

        CP_DECL;
        TL_CP_DECL(PtrArray<_Matcher>, _matchers);
        TL_CP_DECL(StdIntMap, _aam_to_second_side_1);
        TL_CP_DECL(StdIntMap, _aam_to_second_side_2);
        TL_CP_DECL(Array<int>, _molecule_core_1);
        TL_CP_DECL(Array<int>, _molecule_core_2);
        TL_CP_DECL(StdIntMap, _aam_core_first_side);

        int _first_side;
        int _second_side;

        BaseReaction* _query;

        const ReactionAtomNeighbourhoodCounters *_query_nei_counters, *_target_nei_counters;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
