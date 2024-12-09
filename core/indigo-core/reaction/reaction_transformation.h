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

#ifndef __reaction_transformation__
#define __reaction_transformation__

#include "base_cpp/cancellation_handler.h"
#include "base_cpp/reusable_obj_array.h"
#include "graph/embedding_enumerator.h"
#include "molecule/molecule.h"
#include "molecule/molecule_arom.h"
#include "molecule/query_molecule.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_enumerator_state.h"

namespace indigo
{
    class ReactionTransformation // : public ReactionEnumeratorState
    {
    public:
        DECL_ERROR;

        ReactionTransformation(void);

        bool transform(Molecule& molecule, QueryReaction& reaction, Array<int>* mapping = 0);

        bool transform(ReusableObjArray<Molecule>& molecules, QueryReaction& reaction, ReusableObjArray<Array<int>>* mapping_array = 0);

        AromaticityOptions arom_options;

        bool layout_flag;

        bool smart_layout;
        LAYOUT_ORIENTATION layout_orientation;

        CancellationHandler* cancellation;

    private:
        CP_DECL;
        TL_CP_DECL(QueryReaction, _merged_reaction);
        TL_CP_DECL(Molecule, _cur_monomer);
        TL_CP_DECL(Array<int>, _mapping);

        static void _product_proc(Molecule& product, Array<int>& monomers_indices, Array<int>& mapping, void* userdata);

        void _mergeReactionComponents(QueryReaction& reaction, int mol_type, QueryMolecule& merged_molecule, Array<int>& merged_aam);

        void _generateMergedReaction(QueryReaction& reaction);
    };
} // namespace indigo

#endif /*__reaction_transformation__*/
