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

#ifndef __reaction_substructure_matcher__
#define __reaction_substructure_matcher__

#include "base_cpp/obj.h"
#include "graph/embedding_enumerator.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_substructure_matcher.h"
#include "reaction/base_reaction_substructure_matcher.h"
#include <memory>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class QueryReaction;

    class DLLEXPORT ReactionSubstructureMatcher : public BaseReactionSubstructureMatcher
    {
    public:
        ReactionSubstructureMatcher(Reaction& target);
        bool use_daylight_aam_mode;

        DECL_ERROR;

    protected:
        TL_CP_DECL(ObjArray<MoleculeSubstructureMatcher::FragmentMatchCache>, _fmcaches);

        bool _checkAAM() override;

        static bool _match_atoms(BaseReaction& query_, Reaction& target, int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
                                 void* context);

        static bool _match_bonds(BaseReaction& query_, Reaction& target, int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
                                 AromaticityMatcher* am, void* context);

        static void _remove_atom(BaseMolecule& submol, int sub_idx, AromaticityMatcher* am);

        static void _add_bond(BaseMolecule& submol, Molecule& supermol, int sub_idx, int super_idx, AromaticityMatcher* am);

        static bool _prepare(BaseReaction& query_, Reaction& target, void* context);
        static bool _prepare_ee(EmbeddingEnumerator& ee, BaseMolecule& submol, Molecule& supermol, void* context);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
