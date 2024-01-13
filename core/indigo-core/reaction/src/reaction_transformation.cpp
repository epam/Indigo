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

#include "reaction/reaction_transformation.h"
#include "layout/molecule_layout.h"
#include "molecule/elements.h"
#include "reaction/reaction_enumerator_state.h"

using namespace indigo;

IMPL_ERROR(ReactionTransformation, "Reaction transformation");

CP_DEF(ReactionTransformation);

ReactionTransformation::ReactionTransformation(void) : CP_INIT, TL_CP_GET(_merged_reaction), TL_CP_GET(_cur_monomer), TL_CP_GET(_mapping)
{
    _merged_reaction.clear();
    _cur_monomer.clear();
    _mapping.clear();
    layout_flag = true;
    cancellation = 0;
    smart_layout = false;
}

bool ReactionTransformation::transform(Molecule& molecule, QueryReaction& reaction, Array<int>* mapping)
{
    _generateMergedReaction(reaction);

    int reactant_idx = _merged_reaction.reactantBegin();
    int product_idx = _merged_reaction.productBegin();

    bool has_coord = BaseMolecule::hasCoord(molecule);

    QS_DEF(QueryMolecule, cur_full_product);
    cur_full_product.clear();
    cur_full_product.clone(_merged_reaction.getQueryMolecule(product_idx), NULL, NULL);
    Array<int>& cur_cur_monomer_aam_array = _merged_reaction.getAAMArray(product_idx);
    QS_DEF(RedBlackStringMap<int>, cur_smiles_array);
    cur_smiles_array.clear();
    QS_DEF(ReactionEnumeratorState::ReactionMonomers, cur_reaction_monomers);
    cur_reaction_monomers.clear();
    cur_reaction_monomers.addMonomer(reactant_idx, molecule);
    QS_DEF(ObjArray<Array<int>>, cur_tubes_monomers);
    cur_tubes_monomers.clear();

    int product_count = 0;

    ReactionEnumeratorContext context;
    context.arom_options = arom_options;

    ReactionEnumeratorState re_state(context, _merged_reaction, cur_full_product, cur_cur_monomer_aam_array, cur_smiles_array, cur_reaction_monomers,
                                     product_count, cur_tubes_monomers);

    re_state.is_multistep_reaction = false;
    re_state.is_one_tube = false;
    re_state.is_same_keeping = true;
    re_state.is_self_react = false;
    re_state.is_transform = true;
    re_state.userdata = this;
    re_state.product_proc = _product_proc;

    _cur_monomer.clone(molecule, NULL, NULL);

    QS_DEF(Array<int>, forbidden_atoms);
    forbidden_atoms.clear_resize(_cur_monomer.vertexEnd());
    forbidden_atoms.zerofill();

    QS_DEF(Array<int>, original_hydrogens);
    original_hydrogens.clear();
    for (int i = _cur_monomer.vertexBegin(); i != _cur_monomer.vertexEnd(); i = _cur_monomer.vertexNext(i))
    {
        if (_cur_monomer.getAtomNumber(i) == ELEM_H)
            original_hydrogens.push(i);
    }

    bool need_layout = false;

    bool transformed_flag = false;
    while (re_state.performSingleTransformation(_cur_monomer, _mapping, forbidden_atoms, original_hydrogens, need_layout))
        transformed_flag = true;

    molecule.clone(_cur_monomer, NULL, NULL);

    if (has_coord)
    {
        if (need_layout)
        {
            if (layout_flag)
            {
                MoleculeLayout ml(molecule, smart_layout);
                ml.layout_orientation = layout_orientation;
                ml.setCancellationHandler(cancellation);
                ml.make();
            }
            else
                molecule.clearXyz();
        }
        else
            molecule.markBondsStereocenters();
    }

    mapping->copy(_mapping);

    return transformed_flag;
}

bool ReactionTransformation::transform(ReusableObjArray<Molecule>& molecules, QueryReaction& reaction, ReusableObjArray<Array<int>>* mapping_array)
{
    for (int i = 0; i < molecules.size(); i++)
    {
        Array<int>* mapping = 0;
        if (mapping_array != 0)
            mapping = &(mapping_array->at(i));

        if (!transform(molecules[i], reaction, mapping))
            return false;
    }

    return true;
}

void ReactionTransformation::_product_proc(Molecule& product, Array<int>& /*monomers_indices*/, Array<int>& mapping, void* userdata)
{
    ReactionTransformation* rt = (ReactionTransformation*)userdata;

    rt->_mapping.copy(mapping);

    rt->_cur_monomer.clone(product, NULL, NULL);

    return;
}

void ReactionTransformation::_mergeReactionComponents(QueryReaction& reaction, int mol_type, QueryMolecule& merged_molecule, Array<int>& merged_aam)
{
    merged_molecule.clear();
    merged_aam.clear();

    for (int i = reaction.begin(); i < reaction.end(); i = reaction.next(i))
    {
        if (reaction.getSideType(i) != mol_type)
            continue;

        QueryMolecule& molecule_i = reaction.getQueryMolecule(i);

        merged_aam.concat(reaction.getAAMArray(i));

        merged_molecule.mergeWithMolecule(molecule_i, NULL, 0);
    }
}

void ReactionTransformation::_generateMergedReaction(QueryReaction& reaction)
{
    QS_DEF(QueryMolecule, merged_reactant);
    merged_reactant.clear();

    QS_DEF(Array<int>, reactant_aam);
    reactant_aam.clear();

    QS_DEF(QueryMolecule, merged_cur_monomer);
    merged_cur_monomer.clear();

    QS_DEF(Array<int>, product_aam);
    product_aam.clear();

    // Reactants merging
    _mergeReactionComponents(reaction, BaseReaction::REACTANT, merged_reactant, reactant_aam);

    // Products merging
    _mergeReactionComponents(reaction, BaseReaction::PRODUCT, merged_cur_monomer, product_aam);

    _merged_reaction.clear();

    int reactant_idx = _merged_reaction.addReactant();
    int product_idx = _merged_reaction.addProduct();

    QueryMolecule& reactant = _merged_reaction.getQueryMolecule(reactant_idx);
    QueryMolecule& product = _merged_reaction.getQueryMolecule(product_idx);

    reactant.clone(merged_reactant, NULL, NULL);
    product.clone(merged_cur_monomer, NULL, NULL);

    Array<int>& r_aam = _merged_reaction.getAAMArray(reactant_idx);
    r_aam.clear();
    r_aam.concat(reactant_aam);

    Array<int>& p_aam = _merged_reaction.getAAMArray(product_idx);
    p_aam.clear();
    p_aam.concat(product_aam);
}
