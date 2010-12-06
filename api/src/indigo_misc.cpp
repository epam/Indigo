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
#include "indigo_properties.h"
#include "indigo_io.h"
#include "indigo_loaders.h"
#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_dearom.h"
#include "molecule/smiles_saver.h"
#include "reaction/rsmiles_saver.h"
#include "molecule/elements.h"
#include "molecule/molfile_saver.h"
#include "reaction/rxnfile_saver.h"

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
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoDearomatize (int object)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(object);

      if (obj.isBaseMolecule())
         obj.getBaseMolecule().dearomatize();
      else if (obj.isBaseReaction())
         obj.getBaseReaction().dearomatize();
      else
         throw IndigoError("Only molecules and reactions can be dearomatized");
      return 1;
   }
   INDIGO_END(-1)
}

#define INDIGO_SET_OPTION(SUFFIX, TYPE)                                  \
  CEXPORT int indigoSetOption##SUFFIX (const char *name, TYPE value)     \
  {                                                                      \
     INDIGO_BEGIN                                                        \
     {                                                                   \
        indigoGetOptionManager().callOptionHandler##SUFFIX(name, value); \
        return 1;                                                        \
     }                                                                   \
     INDIGO_END(-1)                                                      \
  }

INDIGO_SET_OPTION(, const char *)
INDIGO_SET_OPTION(Int, int)
INDIGO_SET_OPTION(Bool, int)
INDIGO_SET_OPTION(Float, float)

CEXPORT int indigoSetOptionColor (const char *name, float r, float g, float b)
{
   INDIGO_BEGIN
   {
      indigoGetOptionManager().callOptionHandlerColor(name, r, g, b);
      return 1;
   }
   INDIGO_END(-1)
}
CEXPORT int indigoSetOptionXY (const char *name, int x, int y)
{
   INDIGO_BEGIN
   {
      indigoGetOptionManager().callOptionHandlerXY(name, x, y);
      return 1;
   }
   INDIGO_END(-1)
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
   INDIGO_END(0);
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
   INDIGO_END(0);
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
   INDIGO_END(0);
}

CEXPORT int indigoSaveMDLCT (int item, int output)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);
      QS_DEF(Array<char>, buf);
      ArrayOutput out(buf);

      if (obj.isBaseMolecule())
      {
         BaseMolecule &mol = obj.getBaseMolecule();

         MolfileSaver saver(out);
         saver.mode = self.molfile_saving_mode;
         saver.highlighting = obj.getMoleculeHighlighting();
         if (mol.isQueryMolecule())
            saver.saveQueryMolecule(mol.asQueryMolecule());
         else
            saver.saveMolecule(mol.asMolecule());
      }
      else if (obj.isBaseReaction())
      {
         BaseReaction &rxn = obj.getBaseReaction();
         RxnfileSaver saver(out);

         saver.molfile_saving_mode = self.molfile_saving_mode;
         saver.highlighting = obj.getReactionHighlighting();
         if (rxn.isQueryReaction())
            saver.saveQueryReaction(rxn.asQueryReaction());
         else
            saver.saveReaction(rxn.asReaction());
      }

      Output &out2 = IndigoOutput::get(self.getObject(output));

      BufferScanner scanner(buf);
      QS_DEF(Array<char>, line);

      while (!scanner.isEOF())
      {
         scanner.readString(line, false);
         if (line.size() > 255)
            throw IndigoError("indigoSaveMDLCT: line too big (%d)", line.size());
         out2.writeChar(line.size());
         out2.writeArray(line);
      }
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoUnfoldHydrogens (int item)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);

      if (obj.isBaseMolecule())
      {
         QS_DEF(Array<int>, markers);
         obj.getMolecule().unfoldHydrogens(&markers, -1);

         GraphHighlighting *hl = obj.getMoleculeHighlighting();
         if (hl != 0)
            hl->nondestructiveUpdate();
      }
      else if (obj.isBaseReaction())
      {
         int i;
         Reaction &rxn = obj.getReaction();

         for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
            rxn.getMolecule(i).unfoldHydrogens(0, -1);

         ReactionHighlighting *hl = obj.getReactionHighlighting();
         if (hl != 0)
         {
            for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
               hl->getGraphHighlighting(i).nondestructiveUpdate();
         }
      }
      else
         throw IndigoError("indigoUnfoldHydrogens(): %s given", obj.debugInfo());

      return 1;
   }
   INDIGO_END(-1)
}

static void _removeHydrogens (Molecule &mol, GraphHighlighting *hl)
{
   QS_DEF(Array<int>, to_remove);
   int i;

   to_remove.clear();
   for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
   {
      if (mol.getAtomNumber(i) == ELEM_H && mol.getAtomIsotope(i) == 0)
         to_remove.push(i);
   }

   if (to_remove.size() > 0)
      mol.removeAtoms(to_remove);

   if (hl != 0)
      for (i = 0; i < to_remove.size(); i++)
         hl->removeVertex(to_remove[i]);
}

CEXPORT int indigoFoldHydrogens (int item)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);

      if (obj.isBaseMolecule())
         _removeHydrogens(obj.getMolecule(), obj.getMoleculeHighlighting());
      else if (obj.isBaseReaction())
      {
         int i;
         Reaction &rxn = obj.getReaction();
         ReactionHighlighting *hl = obj.getReactionHighlighting();

         for (i = rxn.begin(); i != rxn.end(); i = rxn.next(i))
            _removeHydrogens(rxn.getMolecule(i), hl == 0 ? 0 : &hl->getGraphHighlighting(i));
      }
      else
         throw IndigoError("indigoFoldHydrogens(): %s given", obj.debugInfo());

      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSetName (int handle, const char *name)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      if (obj.isBaseMolecule())
         obj.getBaseMolecule().name.readString(name, true);
      else if (obj.isBaseReaction())
         obj.getBaseReaction().name.readString(name, true);
      else
         throw IndigoError("The object provided is neither a molecule, nor a reaction");
      return 1;
   }
   INDIGO_END(-1);
}

CEXPORT const char * indigoName (int handle)
{
   INDIGO_BEGIN
   {
      return self.getObject(handle).getName();
   }
   INDIGO_END(0);
}

CEXPORT const char * indigoRawData (int handler)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handler);

      if (obj.type == IndigoObject::RDF_MOLECULE ||
          obj.type == IndigoObject::RDF_REACTION ||
          obj.type == IndigoObject::SMILES_MOLECULE ||
          obj.type == IndigoObject::SMILES_REACTION)
      {
         IndigoRdfData &data = (IndigoRdfData &)obj;

         self.tmp_string.copy(data.getRawData());
      }
      else if (obj.type == IndigoObject::PROPERTY)
         self.tmp_string.copy(((IndigoProperty &)obj).getValue());
      else
         throw IndigoError("%s does not have raw data", obj.debugInfo());
      self.tmp_string.push(0);
      return self.tmp_string.ptr();
   }
   INDIGO_END(0)
}
