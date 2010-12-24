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
#include "molecule/molecule_automorphism_search.h"

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

CEXPORT int indigoResetStereo (int item)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);

      if (IndigoAtom::is(obj))
      {
         IndigoAtom &ia = IndigoAtom::cast(self.getObject(item));

         ia.mol->stereocenters.setType(ia.idx, 0, 0);
      }
      else if (IndigoBond::is(obj))
      {
         IndigoBond &ib = IndigoBond::cast(self.getObject(item));

         ib.mol->stereocenters.setBondDirection(ib.idx, 0);
      }
      else
         throw IndigoError("indigoResetStereo(): %s given", obj.debugInfo());
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

static int _resetSymmetricCisTrans (Molecule &mol)
{
   MoleculeAutomorphismSearch am;
   int i, sum = 0;

   am.detect_invalid_cistrans_bonds = true;
   am.process(mol);

   for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
   {
      if (mol.cis_trans.getParity(i) == 0)
         continue;

      if (am.invalidCisTransBond(i))
      {
         mol.cis_trans.setParity(i, 0);
         sum++;
      }
   }
   return sum;
}

CEXPORT int indigoResetSymmetricCisTrans (int handle)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      if (obj.isBaseMolecule())
         return _resetSymmetricCisTrans(obj.getMolecule());
      else if (obj.isBaseReaction())
      {
         Reaction &rxn = obj.getReaction();
         int i, sum = 0;

         for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
            sum += _resetSymmetricCisTrans(rxn.getMolecule(i));
         return sum;
      }
      throw IndigoError("only molecules and reactions have cis-trans");
   }
   INDIGO_END(-1)
}
