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
      QueryReaction &query_rxn = self.getObject(reaction).getQueryReaction();
      IndigoArray &monomers_object = IndigoArray::cast(self.getObject(monomers));

      ReactionTransformation rt;

      for (int i = 0; i < monomers_object.objects.size(); i++)
         rt.transform(monomers_object.objects[i]->getMolecule(), query_rxn);

      return 1;
   }
   INDIGO_END(-1)
}