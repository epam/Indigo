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

#include "indigo_molecule.h"
#include "reaction/reaction.h"

CEXPORT int indigoStereocenterType (int atom)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

      switch (ia.mol->stereocenters.getType(ia.idx))
      {
         case MoleculeStereocenters::ATOM_ABS: return INDIGO_ABS;
         case MoleculeStereocenters::ATOM_OR: return INDIGO_OR;
         case MoleculeStereocenters::ATOM_AND: return INDIGO_AND;
         case MoleculeStereocenters::ATOM_ANY: return INDIGO_EITHER;
         default: return 0;
      }
      return 0;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoCountStereocenters (int molecule)
{
   INDIGO_BEGIN
   {
      BaseMolecule &mol = self.getObject(molecule).getBaseMolecule();

      return mol.stereocenters.size();
   }
   INDIGO_END(-1)
}

CEXPORT int indigoBondStereo (int bond)
{
   INDIGO_BEGIN
   {
      IndigoBond &ib = IndigoBond::cast(self.getObject(bond));
      BaseMolecule &mol = *ib.mol;

      int dir = mol.stereocenters.getBondDirection(ib.idx);

      if (dir == MoleculeStereocenters::BOND_UP)
         return INDIGO_UP;
      if (dir == MoleculeStereocenters::BOND_DOWN)
         return INDIGO_DOWN;
      if (dir == MoleculeStereocenters::BOND_EITHER)
         return INDIGO_EITHER;

      int parity = mol.cis_trans.getParity(ib.idx);

      if (parity == MoleculeCisTrans::CIS)
         return INDIGO_CIS;
      if (parity == MoleculeCisTrans::TRANS)
         return INDIGO_TRANS;
      return 0;
   }
   INDIGO_END(-1);
}

CEXPORT int indigoInvertStereo (int item)
{
   INDIGO_BEGIN
   {
      IndigoAtom &ia = IndigoAtom::cast(self.getObject(item));

      int k, *pyramid = ia.mol->stereocenters.getPyramid(ia.idx);
      if (pyramid == 0)
         throw IndigoError("indigoInvertStereo: not a stereoatom");
      __swap(pyramid[0], pyramid[1], k);
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoClearStereocenters (int object)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(object);

      if (obj.isBaseMolecule())
         obj.getBaseMolecule().stereocenters.clear();
      else if (obj.isBaseReaction())
      {
         BaseReaction &rxn = obj.getBaseReaction();
         int i;

         for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
            rxn.getBaseMolecule(i).stereocenters.clear();
      }
      else
         throw IndigoError("only molecules and reactions have stereocenters");
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoClearCisTrans (int object)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(object);

      if (obj.isBaseMolecule())
         obj.getBaseMolecule().cis_trans.clear();
      else if (obj.isBaseReaction())
      {
         BaseReaction &rxn = obj.getBaseReaction();
         int i;

         for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
            rxn.getBaseMolecule(i).cis_trans.clear();
      }
      else
         throw IndigoError("only molecules and reactions have cis-trans");
      return 1;
   }
   INDIGO_END(-1)
}
