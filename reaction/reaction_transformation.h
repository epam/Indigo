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

#ifndef __reaction_transformation__
#define __reaction_transformation__

#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "molecule/molecule_arom.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction_enumerator_state.h"
#include "graph/embedding_enumerator.h"
#include "base_cpp/reusable_obj_array.h"

namespace indigo
{
   class ReactionTransformation// : public ReactionEnumeratorState
   {
   public:
      DECL_ERROR;

      ReactionTransformation( void );

      bool transform(Molecule &molecule, QueryReaction &reaction);
      
      bool transform(ReusableObjArray<Molecule> &molecules, QueryReaction &reaction);

      AromaticityOptions arom_options;

   private:
      TL_CP_DECL(QueryReaction, _merged_reaction);
      TL_CP_DECL(Molecule, _cur_monomer);

      static void _product_proc( Molecule &product, Array<int> &monomers_indices, 
                                 void *userdata );
   
      void _mergeReactionComponents( QueryReaction &reaction, int mol_type, 
                                     QueryMolecule &merged_molecule, Array<int> &merged_aam);
   
      void _generateMergedReaction( QueryReaction &reaction );
   };
}

#endif /*__reaction_transformation__*/
