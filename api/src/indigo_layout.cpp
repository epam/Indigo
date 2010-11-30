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
#include "layout/reaction_layout.h"
#include "layout/molecule_layout.h"

CEXPORT int indigoLayout (int object)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(object);
      if (obj.isBaseMolecule()) {
         BaseMolecule &mol = obj.getBaseMolecule();
         MoleculeLayout ml(mol);
         ml.bond_length = 1.6f;
         ml.make();
         mol.stereocenters.markBonds();
      } else if (obj.isBaseReaction()) {
         BaseReaction &rxn = obj.getBaseReaction();
         ReactionLayout rl(rxn);
         rl.bond_length = 1.6f;
         rl.make();
         rxn.markStereocenterBonds();
      } else {
         throw IndigoError("The object provided is neither a molecule, nor a reaction");
      }
      return 0;
   }
   INDIGO_END(-1)
}