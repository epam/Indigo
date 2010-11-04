/****************************************************************************
 * Copyright (C) 2010 GGA Software Services LLC
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

#include "indigo_internal.h"
#include "reaction/reaction_substructure_matcher.h"
#include "molecule/molecule_exact_matcher.h"
#include "reaction/reaction_exact_matcher.h"

CEXPORT int indigoExactMatch (int handler1, int handler2)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj1 = self.getObject(handler1);
      IndigoObject &obj2 = self.getObject(handler2);

      if (obj1.isBaseMolecule())
      {
         Molecule &mol1 = obj1.getMolecule();
         Molecule &mol2 = obj2.getMolecule();

         MoleculeExactMatcher matcher(mol1, mol2);
         matcher.flags = MoleculeExactMatcher::CONDITION_ALL;
         if (!matcher.find())
            return 0;
      }
      else if (obj1.isBaseReaction())
      {
         Reaction &rxn1 = obj1.getReaction();
         Reaction &rxn2 = obj2.getReaction();

         ReactionExactMatcher matcher(rxn1, rxn2);
         matcher.flags = MoleculeExactMatcher::CONDITION_ALL;
         if (!matcher.find())
            return 0;
      }

      return 1;
   }
   INDIGO_END(0, -1);
}
