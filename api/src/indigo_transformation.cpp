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

#include "indigo_molecule.h"
#include "indigo_internal.h"
#include "indigo_reaction.h"
#include "indigo_array.h"
#include "layout/molecule_layout.h"
#include "reaction/reaction_transformation.h" 

CEXPORT int indigoTransform (int reaction, int monomers)
{
   INDIGO_BEGIN
   {
      IndigoObject &monomers_object = self.getObject(monomers);
      QueryReaction &query_rxn = self.getObject(reaction).getQueryReaction();

      ReactionTransformation rt;

      if (monomers_object.type == IndigoObject::MOLECULE)
      {
         Molecule &mol = monomers_object.getMolecule();
         rt.transform(mol, query_rxn);
      }
      else if (monomers_object.type == IndigoObject::ARRAY)
      {
         IndigoArray &monomers_array = IndigoArray::cast(self.getObject(monomers));

      
         for (int i = 0; i < monomers_array.objects.size(); i++)
            rt.transform(monomers_array.objects[i]->getMolecule(), query_rxn);
      }
      else
         throw IndigoError("%s is not a molecule or array of molecules", self.getObject(monomers).debugInfo());

      return 1;
   }
   INDIGO_END(-1)
}