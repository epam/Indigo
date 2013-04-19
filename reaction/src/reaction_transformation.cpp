/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
 *
 * This file is part of Indigo toolkit.
 *
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 *
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "reaction/reaction_transformation.h"
#include "reaction/reaction_enumerator_state.h"
#include "layout/molecule_layout.h"
#include "molecule/elements.h"

using namespace indigo;

IMPL_ERROR(ReactionTransformation, "Reaction transformation");

ReactionTransformation::ReactionTransformation( void ) : TL_CP_GET(_merged_reaction), TL_CP_GET(_cur_monomer)
{
   _merged_reaction.clear();
   _cur_monomer.clear();
}

bool ReactionTransformation::transform( Molecule &molecule, QueryReaction &reaction )
{
   _generateMergedReaction(reaction);
   
   int reactant_idx = _merged_reaction.reactantBegin();
   int product_idx = _merged_reaction.productBegin();

   bool has_coord = BaseMolecule::hasCoord(molecule);

   QS_DEF(QueryMolecule, cur_full_product);
   cur_full_product.clear();
   cur_full_product.clone(_merged_reaction.getQueryMolecule(product_idx), NULL, NULL);
   Array<int> &cur_cur_monomer_aam_array = _merged_reaction.getAAMArray(product_idx);
   QS_DEF(RedBlackStringMap<int>, cur_smiles_array);
   cur_smiles_array.clear();
   QS_DEF(ReactionEnumeratorState::ReactionMonomers, cur_reaction_monomers);
   cur_reaction_monomers.clear();
   cur_reaction_monomers.addMonomer(reactant_idx, molecule);
   QS_DEF(ObjArray< Array<int> >, cur_tubes_monomers);
   cur_tubes_monomers.clear();

   int product_count = 0;

   ReactionEnumeratorContext context;
   context.arom_options = arom_options;

   ReactionEnumeratorState re_state(context, _merged_reaction, cur_full_product, 
      cur_cur_monomer_aam_array, cur_smiles_array, cur_reaction_monomers, 
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

   while (re_state.performSingleTransformation(_cur_monomer, forbidden_atoms, original_hydrogens))
      ;

   molecule.clone(_cur_monomer, NULL, NULL);

   if (has_coord)
   {
      MoleculeLayout ml(molecule);
      ml.make();
      molecule.stereocenters.markBonds();
   }

   return true;
}

bool ReactionTransformation::transform(ReusableObjArray<Molecule> &molecules, QueryReaction &reaction)
{
   for (int i = 0; i < molecules.size(); i++)
      if (!transform(molecules[i], reaction))
         return false;

   return true;
}

void ReactionTransformation::_product_proc( Molecule &product, Array<int> &monomers_indices, 
                                            void *userdata )
{
   ReactionTransformation *rt = (ReactionTransformation *)userdata;

   rt->_cur_monomer.clone(product, NULL, NULL);

   return;
}

void ReactionTransformation::_mergeReactionComponents( QueryReaction &reaction, int mol_type, 
                                                       QueryMolecule &merged_molecule, 
                                                       Array<int> &merged_aam)
{
   merged_molecule.clear();
   merged_aam.clear();

   for (int i = reaction.begin(); i < reaction.end(); i = reaction.next(i))
   {
      if (reaction.getSideType(i) != mol_type)
         continue;

      QueryMolecule &molecule_i = reaction.getQueryMolecule(i);

      merged_aam.concat(reaction.getAAMArray(i));

      merged_molecule.mergeWithMolecule(molecule_i, NULL, NULL);
   }
}

void ReactionTransformation::_generateMergedReaction( QueryReaction &reaction )
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

   QueryMolecule &reactant = _merged_reaction.getQueryMolecule(reactant_idx);
   QueryMolecule &product = _merged_reaction.getQueryMolecule(product_idx);

   reactant.clone(merged_reactant, NULL, NULL);
   product.clone(merged_cur_monomer, NULL, NULL);

   Array<int> &r_aam = _merged_reaction.getAAMArray(reactant_idx);
   r_aam.clear();
   r_aam.concat(reactant_aam);

   Array<int> &p_aam = _merged_reaction.getAAMArray(product_idx);
   p_aam.clear();
   p_aam.concat(product_aam);
}
