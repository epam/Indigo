/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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
#include "base_cpp/cancellation_handler.h"
#include "layout/reaction_layout.h"
#include "layout/molecule_layout.h"
#include "reaction/base_reaction.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "layout/molecule_cleaner_2d.h"
#include <vector>

CEXPORT int indigoLayout (int object)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(object);
      int i;

      if (IndigoBaseMolecule::is(obj)) {
         BaseMolecule *mol = &obj.getBaseMolecule();
         Filter f;
         if (obj.type == IndigoObject::SUBMOLECULE) {
            IndigoSubmolecule &submol = (IndigoSubmolecule &)obj;
            mol = &submol.getOriginalMolecule();
            f.initNone(mol->vertexEnd());
            for (int i = 0; i < submol.vertices.size(); i++) {
               f.unhide(submol.vertices[i]);
            }
         }
         MoleculeLayout ml(*mol, self.smart_layout);
         
         if (obj.type == IndigoObject::SUBMOLECULE) {
            ml.filter = &f;
         }
         
         ml.max_iterations = self.layout_max_iterations;
         ml.bond_length = 1.6f;
         ml.layout_orientation = (layout_orientation_value) self.layout_orientation;

         TimeoutCancellationHandler cancellation(self.cancellation_timeout);
         ml.setCancellationHandler(&cancellation);

         ml.make();
      
         if (obj.type != IndigoObject::SUBMOLECULE)
         {
            // Not for submolecule yet
            mol->clearBondDirections();
            try
            {
               mol->stereocenters.markBonds();
               mol->allene_stereo.markBonds();
            } catch (Exception e) {}
            for (i = 1; i <= mol->rgroups.getRGroupCount(); i++)
            {
               RGroup &rgp = mol->rgroups.getRGroup(i);

               for (int j = rgp.fragments.begin(); j != rgp.fragments.end();
                        j = rgp.fragments.next(j))
               {
                  rgp.fragments[j]->clearBondDirections();
                  try
                  {
                     rgp.fragments[j]->stereocenters.markBonds();
                     rgp.fragments[j]->allene_stereo.markBonds();
                  } catch (Exception e) {}
               }
            }
         }
      } else if (IndigoBaseReaction::is(obj)) {
         BaseReaction &rxn = obj.getBaseReaction();
         ReactionLayout rl(rxn, self.smart_layout);
         rl.max_iterations = self.layout_max_iterations;
         rl.layout_orientation = (layout_orientation_value) self.layout_orientation;
         rl.bond_length = 1.6f;
         rl.horizontal_interval_factor = self.layout_horintervalfactor;

         rl.make();
         try
         {
            rxn.markStereocenterBonds();
         } catch (Exception e) {}
     
      } else {
         throw IndigoError("The object provided is neither a molecule, nor a reaction");
      }
      return 0;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoClean2d(int object)
{
    INDIGO_BEGIN
    {
        IndigoObject &obj = self.getObject(object);
        
        if (IndigoBaseMolecule::is(obj)) {
            if (obj.type == IndigoObject::SUBMOLECULE) {
               IndigoSubmolecule &submol = (IndigoSubmolecule &)obj;
               BaseMolecule &orig_mol = submol.getOriginalMolecule();
               MoleculeCleaner2d cleaner2d1(orig_mol, false, submol.vertices);
               cleaner2d1.do_clean(false);
               return 0;
            }
            BaseMolecule &mol = obj.getBaseMolecule();
			MoleculeCleaner2d::clean(mol);
        } else {
			if (IndigoBaseReaction::is(obj)) {
				BaseReaction &rxn = obj.getBaseReaction();
				for (int i = rxn.begin(); i < rxn.end(); i = rxn.next(i))
				{
					MoleculeCleaner2d::clean(rxn.getBaseMolecule(i));
				}

			}
            else
				throw IndigoError("Clean2d can be executed only for molecules but %s was provided", obj.debugInfo());
        }
        
        return 0;
    }
    INDIGO_END(-1)
}
