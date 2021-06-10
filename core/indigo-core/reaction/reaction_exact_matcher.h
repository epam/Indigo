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

#ifndef __reaction_exact_matcher__
#define __reaction_exact_matcher__

#include "base_cpp/exception.h"
#include "reaction/base_reaction_substructure_matcher.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Reaction;

    class DLLEXPORT ReactionExactMatcher : public BaseReactionSubstructureMatcher
    {
    public:
        enum
        {
            // start from 0x0100 not to conflict with MoleculExactMatcher
            CONDITION_AAM = 0x0100,              // atom-to-atom mapping values
            CONDITION_REACTING_CENTERS = 0x0200, // reacting centers
            CONDITION_ALL = 0x0300
        };

        ReactionExactMatcher(Reaction& query, Reaction& target);

        dword flags;

        DECL_ERROR;

    protected:
        Reaction& _query;
        Reaction& _target;

        static bool _match_atoms(BaseReaction& query_, Reaction& target, int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
                                 void* context);

        static bool _match_bonds(BaseReaction& query_, Reaction& target, int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
                                 AromaticityMatcher* am, void* context);

        static bool _prepare(BaseReaction& query, Reaction& target, void* context);

        static bool _prepare_ee(EmbeddingEnumerator& ee, BaseMolecule& submol, Molecule& supermol, void* context);
    };

} // namespace indigo

#endif
