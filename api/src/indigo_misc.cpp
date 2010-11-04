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
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_dearom.h"
#include "molecule/smiles_saver.h"
#include "reaction/rsmiles_saver.h"
#include "molecule/elements.h"

#define CHECKRGB(r, g, b) \
if (__min3(r, g, b) < 0 || __max3(r, g, b) > 1.0 + 1e-6) \
   throw IndigoError("Some of the color components are out of range [0..1]")

CEXPORT int indigoAromatize (int object)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(object);

      if (obj.isBaseMolecule())
         obj.getBaseMolecule().aromatize();
      else if (obj.isBaseReaction())
         obj.getBaseReaction().aromatize();
      else
         throw IndigoError("Only molecules and reactions can be aromatized");
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoDearomatize (int object)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(object);

      if (obj.isBaseMolecule()) {
         obj.getBaseMolecule().dearomatize();
      } else if (obj.isBaseReaction()) {
         obj.getBaseReaction().dearomatize();
      } else {
         throw IndigoError("Only molecules and reactions can be dearomatized");
      }      
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoSetOption (const char *name, const char *value)
{
   INDIGO_BEGIN
   {
      if (indigoGetOptionManager().callOptionHandler(name, value) == 0)
         throw IndigoError("Can't set property %s", name);
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoSetOptionInt (const char *name, int value)
{
   INDIGO_BEGIN
   {
      if (indigoGetOptionManager().callOptionHandlerInt(name, value) == 0)
         throw IndigoError("Can't set property %s", name);
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoSetOptionBool (const char *name, int value)
{
   INDIGO_BEGIN
   {
      if (indigoGetOptionManager().callOptionHandlerBool(name, value) == 0)
         throw IndigoError("Can't set property %s", name);
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoSetOptionFloat (const char *name, float value)
{
   INDIGO_BEGIN
   {
      if (indigoGetOptionManager().callOptionHandlerFloat(name, value) == 0)
         throw IndigoError("Can't set property %s", name);
   }
   INDIGO_END(0, -1)
}
CEXPORT int indigoSetOptionColor (const char *name, float r, float g, float b)
{
   INDIGO_BEGIN
   {
      if (indigoGetOptionManager().callOptionHandlerColor(name, r, g, b) == 0)
         throw IndigoError("Can't set property %s", name);
   }
   INDIGO_END(0, -1)
}
CEXPORT int indigoSetOptionXY (const char *name, int x, int y)
{
   INDIGO_BEGIN
   {
      if (indigoGetOptionManager().callOptionHandlerXY(name, x, y) == 0)
         throw IndigoError("Can't set property %s", name);
   }
   INDIGO_END(0, -1)
}


CEXPORT const char * indigoCheckBadValence (int handle)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      if (obj.isBaseMolecule())
      {
         BaseMolecule &bmol = obj.getBaseMolecule();

         if (bmol.isQueryMolecule())
            throw IndigoError("indigoCheckBadValence(): query molecules not allowed");

         Molecule &mol = bmol.asMolecule();

         try
         {
            int i;

            for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
               mol.getAtomValence(i);
         }
         catch (Exception &e)
         {
            self.tmp_string.readString(e.message(), true);
            return self.tmp_string.ptr();
         }
      }
      else if (obj.isBaseReaction())
      {
         BaseReaction &brxn = obj.getBaseReaction();

         if (brxn.isQueryReaction())
            throw IndigoError("indigoCheckBadValence(): query molecules not allowed");

         Reaction &rxn = brxn.asReaction();

         try
         {
            int i, j;

            for (j = rxn.begin(); j != rxn.end(); j = rxn.next(j))
            {
               Molecule &mol = rxn.getMolecule(j);
               
               for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
                  mol.getAtomValence(i);
            }
         }
         catch (Exception &e)
         {
            self.tmp_string.readString(e.message(), true);
            return self.tmp_string.ptr();
         }
      }
      else
         throw IndigoError("object %s is meither a molecule nor a reaction", obj.debugInfo());
      
      return "";
   }
   INDIGO_END(0, 0);
}

void _indigoCheckAmbiguousH (Molecule &mol)
{
   int i;

   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
      if (mol.getAtomAromaticity(i) == ATOM_AROMATIC)
      {
         int atom_number = mol.getAtomNumber(i);

         if (atom_number != ELEM_C && atom_number != ELEM_O)
            mol.getAtomTotalH(i);
      }
}

CEXPORT const char * indigoCheckAmbiguousH (int handle)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      if (obj.isBaseMolecule())
      {
         BaseMolecule &bmol = obj.getBaseMolecule();

         if (bmol.isQueryMolecule())
            throw IndigoError("indigoCheckAmbiguousH(): query molecules not allowed");

         Molecule &mol = bmol.asMolecule();

         try
         {
            _indigoCheckAmbiguousH(mol);
         }
         catch (Exception &e)
         {
            self.tmp_string.readString(e.message(), true);
            return self.tmp_string.ptr();
         }
      }
      else if (obj.isBaseReaction())
      {
         BaseReaction &brxn = obj.getBaseReaction();

         if (brxn.isQueryReaction())
            throw IndigoError("indigoCheckAmbiguousH(): query molecules not allowed");

         Reaction &rxn = brxn.asReaction();

         try
         {
            int j;

            for (j = rxn.begin(); j != rxn.end(); j = rxn.next(j))
               _indigoCheckAmbiguousH(rxn.getMolecule(j));
         }
         catch (Exception &e)
         {
            self.tmp_string.readString(e.message(), true);
            return self.tmp_string.ptr();
         }
      }
      else
         throw IndigoError("object %s is meither a molecule nor a reaction", obj.debugInfo());

      return "";
   }
   INDIGO_END(0, 0);
}

CEXPORT int indigoCisTransClear (int object)
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
   }
   INDIGO_END(0, -1)
}

CEXPORT int indigoStereocentersClear (int object)
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
   }
   INDIGO_END(0, -1)
}

CEXPORT const char * indigoSmiles (int item)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);

      ArrayOutput output(self.tmp_string);
      if (obj.isBaseMolecule())
      {
         BaseMolecule &mol = obj.getBaseMolecule();
         
         SmilesSaver saver(output);
         saver.highlighting = obj.getMoleculeHighlighting();
         
         if (mol.isQueryMolecule())
            saver.saveQueryMolecule(mol.asQueryMolecule());
         else
            saver.saveMolecule(mol.asMolecule());
      }
      else if (obj.isBaseReaction())
      {
         BaseReaction &rxn = obj.getBaseReaction();
         
         RSmilesSaver saver(output);
         saver.highlighting = obj.getReactionHighlighting();
         
         if (rxn.isQueryReaction())
            saver.saveQueryReaction(rxn.asQueryReaction());
         else
            saver.saveReaction(rxn.asReaction());
      }
      else
         throw IndigoError("%s can not be converted to SMILES", obj.debugInfo());

      self.tmp_string.push(0);
      return self.tmp_string.ptr();
   }
   INDIGO_END(0, 0);
}
