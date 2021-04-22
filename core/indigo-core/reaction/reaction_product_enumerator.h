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

#ifndef __reaction_product_enumerator__
#define __reaction_product_enumerator__

#include "base_cpp/reusable_obj_array.h"
#include "molecule/molecule.h"
#include "reaction/reaction.h"
#include "reaction/reaction_enumerator_state.h"

namespace indigo
{

    class ReactionProductEnumerator
    {
    public:
        DECL_ERROR;

        bool is_multistep_reaction; /* if true - all reactants in monomer take part in reaction, false - one */
        bool is_self_react;         /* if true - monomer's molecule can react with itself, false - can't */
        bool is_one_tube;           /* if true - all monomers are in one test-tube */
        int max_product_count;
        int max_deep_level;
        void* userdata;

        AromaticityOptions arom_options;

        ReactionProductEnumerator(QueryReaction& reaction);
        ~ReactionProductEnumerator()
        {
        }

        void addMonomer(int reactant_idx, Molecule& monomer);

        void clearReactantMonomers(int reactant_idx);

        Molecule& getMonomer(int reactant_idx, int index);

        Molecule& getMonomer(int mon_index);

        const QueryReaction& getReaction(void);

        int getMonomersCount(int reactant_idx);

        void buildProducts(void);

        // This callback should be used for validation and refining of the results of applying the pattern.
        // uncleaned_fragments: the molecule before applying the reaction (with aromatization and unfolded hydrogens)
        // product: the molecule after transformation (possibly broken), may be modified in callback
        // mapping: atom to atom mapping
        // result: true if the molecule shall be accepted, false otherwise
        bool (*refine_proc)(const Molecule& uncleaned_fragments, Molecule& product, Array<int>& mapping, void* userdata);

        // This callback provides the results of applying the pattern, one for each possible mapping.
        // product: the molecule after transformation
        // mapping: atom to atom mapping
        void (*product_proc)(Molecule& product, Array<int>& monomers_indices, Array<int>& mapping, void* userdata);

    private:
        bool _is_rg_exist;
        int _product_count;
        QueryReaction& _reaction;
        ReactionEnumeratorState::ReactionMonomers _reaction_monomers;
        CP_DECL;
        TL_CP_DECL(Array<int>, _product_aam_array);
        TL_CP_DECL(RedBlackStringMap<int>, _smiles_array);
        TL_CP_DECL(ObjArray<Array<int>>, _tubes_monomers);

        void _buildTubesGrid(void);
    };

} // namespace indigo

#endif /* __reaction_product_enumerator__ */
