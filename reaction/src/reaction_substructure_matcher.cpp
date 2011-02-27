/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_3d_constraints.h"
#include "molecule/molecule_substructure_matcher.h"
#include "reaction/reaction_substructure_matcher.h"
#include "reaction/reaction_neighborhood_counters.h"
#include "molecule/molecule_neighbourhood_counters.h"

using namespace indigo;

ReactionSubstructureMatcher::ReactionSubstructureMatcher (Reaction &target) :
BaseReactionSubstructureMatcher(target)
{
   match_atoms = _match_atoms;
   match_bonds = _match_bonds;
   add_bond = _add_bond;
   remove_atom = _remove_atom;
   prepare_ee = _prepare_ee;
}

bool ReactionSubstructureMatcher::_match_atoms (BaseReaction &query_, Reaction &target,
           int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
           void *context)
{
   QueryReaction &query = query_.asQueryReaction();
   QueryMolecule &submol = query.getQueryMolecule(sub_mol_idx);
   Molecule &supermol = target.getMolecule(super_mol_idx);

   if (!MoleculeSubstructureMatcher::matchQueryAtom(&submol.getAtom(sub_atom_idx),
      supermol, super_atom_idx, 0, 0xFFFFFFFFUL))
      return false;

   if (submol.stereocenters.getType(sub_atom_idx) > supermol.stereocenters.getType(super_atom_idx))
      return false;

   if (query.getExactChange(sub_mol_idx, sub_atom_idx) == 1)
   {
      const Vertex &can_vertex = submol.getVertex(sub_atom_idx);
      int ch_flag;
      int bonds_changes[RC_TOTAL] = {0};
      int i;

      for (i = can_vertex.neiBegin(); i != can_vertex.neiEnd(); i = can_vertex.neiNext(i))
      {
         ch_flag = query.getReactingCenter(sub_atom_idx, can_vertex.neiEdge(i));

         if (ch_flag == RC_NOT_CENTER)
            ch_flag = RC_UNCHANGED;

         if (ch_flag > 0)
            bonds_changes[ch_flag]++;
      }

      const Vertex &pat_vertex = supermol.getVertex(super_atom_idx);

      for (i = pat_vertex.neiBegin(); i != pat_vertex.neiEnd(); i = pat_vertex.neiNext(i))
      {

         ch_flag = target.getReactingCenter(super_mol_idx, pat_vertex.neiEdge(i));
         if (ch_flag > 0)
            bonds_changes[ch_flag]--;
      }

      int n_centers = bonds_changes[RC_CENTER];
      bonds_changes[RC_CENTER] = 0;

      if (n_centers < 0)
         return false;

      for (int i = 0; i < NELEM(bonds_changes); i++)
      {
         if ((ch_flag = bonds_changes[i]) > 0)
            return false;
         else if (ch_flag < 0)
         {
            if ((n_centers += ch_flag) < 0)
               return false;
         }
      }

      if (n_centers != 0)
         return false;
   }


   return true;
}

bool ReactionSubstructureMatcher::_match_bonds (BaseReaction &query_, Reaction &target,
           int sub_mol_idx, int sub_bond_idx, int super_mol_idx, int super_bond_idx,
           AromaticityMatcher *am, void *context)
{
   QueryReaction &query = query_.asQueryReaction();
   QueryMolecule &submol = query.getQueryMolecule(sub_mol_idx);
   Molecule &supermol = target.getMolecule(super_mol_idx);

   if (!MoleculeSubstructureMatcher::matchQueryBond(&submol.getBond(sub_bond_idx),
            supermol, sub_bond_idx, super_bond_idx, am, 0xFFFFFFFFUL))
      return false;

   int sub_change = query.getReactingCenter(sub_mol_idx, sub_bond_idx);
   int super_change = target.getReactingCenter(super_mol_idx, super_bond_idx);

   if (super_change == RC_UNMARKED)
      return true;

   // super_change == (RC_UNCHANGED + RC_ORDER_CHANGED) is for changed aromatics
   if (sub_change == RC_NOT_CENTER || sub_change == RC_UNCHANGED)
      return super_change == 0 || super_change == RC_UNCHANGED || super_change == (RC_UNCHANGED + RC_ORDER_CHANGED);

   if (sub_change == RC_CENTER)
      return super_change != 0 && super_change != RC_UNCHANGED && super_change != RC_NOT_CENTER;

   if ((sub_change & super_change) != sub_change)
      return false;

   return true;
}


void ReactionSubstructureMatcher::_remove_atom (BaseMolecule &submol, int sub_idx,
                                                AromaticityMatcher *am)
{
   MoleculeSubstructureMatcher::removeAtom(submol, sub_idx, am);
}

void ReactionSubstructureMatcher::_add_bond (BaseMolecule &submol, Molecule &supermol,
                                             int sub_idx, int super_idx, AromaticityMatcher *am)
{
   MoleculeSubstructureMatcher::addBond(submol, supermol, sub_idx, super_idx, am);
}

bool ReactionSubstructureMatcher::_prepare_ee (EmbeddingEnumerator &ee,
        BaseMolecule &submol, Molecule &supermol, void *context)
{
   // mark hydrogens to ignore
   QS_DEF(Array<int>, ignored);

   ignored.clear_resize(submol.vertexEnd());

   MoleculeSubstructureMatcher::markIgnoredQueryHydrogens(submol.asQueryMolecule(), ignored.ptr(), 0, 1);

   for (int i = submol.vertexBegin(); i != submol.vertexEnd(); i = submol.vertexNext(i))
      if (ignored[i])
         ee.ignoreSubgraphVertex(i);

   return true;
}
