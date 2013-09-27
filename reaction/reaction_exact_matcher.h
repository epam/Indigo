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

#ifndef __reaction_exact_matcher__
#define __reaction_exact_matcher__

#include "base_cpp/exception.h"
#include "reaction/base_reaction_substructure_matcher.h"

namespace indigo {

class Reaction;

class ReactionExactMatcher : public BaseReactionSubstructureMatcher
{
public:

   enum
   {
      // start from 0x0100 not to conflict with MoleculExactMatcher
      CONDITION_AAM              = 0x0100, // atom-to-atom mapping values
      CONDITION_REACTING_CENTERS = 0x0200, // reacting centers
      CONDITION_ALL              = 0x0300
   };


   ReactionExactMatcher (Reaction &query, Reaction &target);

   dword flags;

   DECL_ERROR;

protected:
   Reaction &_query;
   Reaction &_target;

   static bool _match_atoms (BaseReaction &query_, Reaction &target,
                      int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
                      void *context);

   static bool _match_bonds (BaseReaction &query_, Reaction &target,
                      int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
                      AromaticityMatcher *am, void *context);

   static bool _prepare (BaseReaction &query, Reaction &target, void *context);

   static bool _prepare_ee (EmbeddingEnumerator &ee, BaseMolecule &submol, Molecule &supermol, void *context);

};

}

#endif
