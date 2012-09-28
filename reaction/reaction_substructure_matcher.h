/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

#ifndef __reaction_substructure_matcher__
#define __reaction_substructure_matcher__

#include "graph/embedding_enumerator.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_substructure_matcher.h"
#include "base_cpp/auto_ptr.h"
#include "base_cpp/obj.h"
#include "reaction/base_reaction_substructure_matcher.h"

namespace indigo {

class QueryReaction;

class ReactionSubstructureMatcher : public BaseReactionSubstructureMatcher
{
public:
   ReactionSubstructureMatcher (Reaction &target);
   bool use_daylight_aam_mode;

   DECL_ERROR;

protected:

   TL_CP_DECL(ObjArray<MoleculeSubstructureMatcher::FragmentMatchCache>, _fmcaches);

   virtual bool _checkAAM ();
   
   static bool _match_atoms (BaseReaction &query_, Reaction &target,
                      int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
                      void *context);

   static bool _match_bonds (BaseReaction &query_, Reaction &target,
                      int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
                      AromaticityMatcher *am, void *context);

   static void _remove_atom (BaseMolecule &submol, int sub_idx, AromaticityMatcher *am);

   static void _add_bond (BaseMolecule &submol, Molecule &supermol,
                         int sub_idx, int super_idx, AromaticityMatcher *am);

   static bool _prepare (BaseReaction &query_, Reaction &target, void *context);
   static bool _prepare_ee (EmbeddingEnumerator &ee,
                         BaseMolecule &submol, Molecule &supermol, void *context);
};

}

#endif
