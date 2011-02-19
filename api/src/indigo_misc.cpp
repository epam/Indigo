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
#include "indigo_molecule.h"
#include "molecule/sdf_loader.h"
#include "molecule/rdf_loader.h"
#include "indigo_array.h"
#include "molecule/icm_saver.h"
#include "molecule/icm_loader.h"
#include "reaction/icr_saver.h"
#include "reaction/icr_loader.h"
#include "indigo_reaction.h"
#include "indigo_mapping.h"
#include "indigo_match.h"
#include "molecule/molecule_cml_saver.h"

#include <time.h>

#define CHECKRGB(r, g, b) \
if (__min3(r, g, b) < 0 || __max3(r, g, b) > 1.0 + 1e-6) \
   throw IndigoError("Some of the color components are out of range [0..1]")

CEXPORT int indigoAromatize (int object)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(object);

      bool ret;
      if (obj.isBaseMolecule())
         ret = obj.getBaseMolecule().aromatize();
      else if (obj.isBaseReaction())
         ret = obj.getBaseReaction().aromatize();
      else
         throw IndigoError("Only molecules and reactions can be aromatized");
      if (ret)
         return 1;
      return 0;
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
            {
               if (mol.isPseudoAtom(i) || mol.isRSite(i))
                  continue;
               mol.getAtomValence(i);
               mol.getImplicitH(i);
            }
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
            throw IndigoError("indigoCheckBadValence(): query reactions not allowed");

         Reaction &rxn = brxn.asReaction();

         try
         {
            int i, j;

            for (j = rxn.begin(); j != rxn.end(); j = rxn.next(j))
            {
               Molecule &mol = rxn.getMolecule(j);
               
               for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
               {
                  if (mol.isPseudoAtom(i) || mol.isRSite(i))
                     continue;
                  mol.getAtomValence(i);
                  mol.getImplicitH(i);
               }
            }
         }
         catch (Exception &e)
         {
            self.tmp_string.readString(e.message(), true);
            return self.tmp_string.ptr();
         }
      }
      else
         throw IndigoError("object %s is neither a molecule nor a reaction", obj.debugInfo());
      
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
         self.initMolfileSaver(saver);
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
         self.initRxnfileSaver(saver);
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
      else if (obj.type == IndigoObject::CML_MOLECULE)
      {
         IndigoCmlMolecule &cml = (IndigoCmlMolecule &)obj;
         self.tmp_string.copy(cml.data);
      }
      else if (obj.type == IndigoObject::PROPERTY)
         self.tmp_string.copy(((IndigoProperty &)obj).getValue());
      else if (obj.type == IndigoObject::DATA_SGROUP)
      {
         self.tmp_string.copy(((IndigoDataSGroup &)obj).get().data);
      }
      else
         throw IndigoError("%s does not have raw data", obj.debugInfo());
      self.tmp_string.push(0);
      return self.tmp_string.ptr();
   }
   INDIGO_END(0)
}

CEXPORT int indigoRemove (int item)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);

      obj.remove();
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSmilesAppend (int output, int item)
{
   const char *smiles = indigoSmiles(item);

   if (smiles == 0)
      return -1;

   INDIGO_BEGIN
   {
      Output &out = IndigoOutput::get(self.getObject(output));

      out.writeStringCR(smiles);
      out.flush();
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoRdfHeader (int output)
{
   INDIGO_BEGIN
   {
      Output &out = IndigoOutput::get(self.getObject(output));

      out.printfCR("$RDFILE 1");
      struct tm lt;
      if (self.molfile_saving_skip_date)
         memset(&lt, 0, sizeof(lt));
      else
      {
         time_t tm = time(NULL);
         lt = *localtime(&tm);
      }
      out.printfCR("$DATM    %02d/%02d/%02d %02d:%02d",
               lt.tm_mon + 1, lt.tm_mday, lt.tm_year % 100, lt.tm_hour, lt.tm_min);
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoRdfAppend (int output, int item)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);
      Output &out = IndigoOutput::get(self.getObject(output));

      if (obj.isBaseMolecule())
      {
         out.writeStringCR("$MFMT");
         MolfileSaver saver(out);
         self.initMolfileSaver(saver);
         saver.saveBaseMolecule(obj.getBaseMolecule());
      }
      else if (obj.isBaseReaction())
      {
         out.writeStringCR("$RFMT");
         RxnfileSaver saver(out);
         self.initRxnfileSaver(saver);
         saver.saveBaseReaction(obj.getBaseReaction());
      }
      else
         throw IndigoError("%s can not be saved to RDF", obj.debugInfo());

      RedBlackStringObjMap< Array<char> > *props = obj.getProperties();
      
      if (props != 0)
      {
         int i;

         for (i = props->begin(); i != props->end(); i = props->next(i))
            out.printf("$DTYPE %s\n$DATUM %s\n", props->key(i), props->value(i).ptr());
      }

      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCmlHeader (int output)
{
   INDIGO_BEGIN
   {
      Output &out = IndigoOutput::get(self.getObject(output));

      out.printf("<?xml version=\"1.0\" ?>\n");
      out.printf("<cml>\n");
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCmlFooter (int output)
{
   INDIGO_BEGIN
   {
      Output &out = IndigoOutput::get(self.getObject(output));

      out.printf("</cml>\n");
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCmlAppend (int output, int item)
{
   INDIGO_BEGIN
   {
      Output &out = IndigoOutput::get(self.getObject(output));
      IndigoObject &obj = self.getObject(item);

      if (obj.isBaseMolecule())
      {
         MoleculeCmlSaver saver(out);
         saver.skip_cml_tag = true;
         saver.saveMolecule(obj.getMolecule());
      }
      else
         throw IndigoError("%s can not be saved to CML", obj.debugInfo());

      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoAt (int item, int index)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);
      if (obj.type == IndigoObject::SDF_LOADER)
      {
         IndigoObject * newobj = ((IndigoSdfLoader &)obj).at(index);
         if (newobj == 0)
            return 0;
         return self.addObject(newobj);
      }
      if (obj.type == IndigoObject::RDF_LOADER)
      {
         IndigoObject * newobj = ((IndigoRdfLoader &)obj).at(index);
         if (newobj == 0)
            return 0;
         return self.addObject(newobj);
      }
      else if (obj.type == IndigoObject::MULTILINE_SMILES_LOADER)
      {
         IndigoObject * newobj = ((IndigoMultilineSmilesLoader &)obj).at(index);
         if (newobj == 0)
            return 0;
         return self.addObject(newobj);
      }
      else if (IndigoArray::is(obj))
      {
         IndigoArray &arr = IndigoArray::cast(obj);

         return self.addObject(new IndigoArrayElement(arr, index));
      }
      else
         throw IndigoError("indigoAt(): not accepting %s", obj.debugInfo());
   }
   INDIGO_END(-1);
}

CEXPORT int indigoCount (int item)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);

      if (IndigoArray::is(obj))
         return IndigoArray::cast(obj).objects.size();

      if (obj.type == IndigoObject::SDF_LOADER)
         return ((IndigoSdfLoader &)obj).sdf_loader->count();

      if (obj.type == IndigoObject::RDF_LOADER)
         return ((IndigoRdfLoader &)obj).rdf_loader->count();

      if (obj.type == IndigoObject::MULTILINE_SMILES_LOADER)
         return ((IndigoMultilineSmilesLoader &)obj).count();

      throw IndigoError("indigoCount(): can not handle %s", obj.debugInfo());
   }
   INDIGO_END(-1);
}

CEXPORT int indigoSerialize (int item, char **buf, int *size)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);
      ArrayOutput out(self.tmp_string);

      if (obj.isBaseMolecule())
      {
         Molecule &mol = obj.getMolecule();

         IcmSaver saver(out);
         saver.save_xyz = mol.have_xyz;
         saver.saveMolecule(mol);
      }
      else if (obj.isBaseReaction())
      {
         Reaction &rxn = obj.getReaction();
         IcrSaver saver(out);
         saver.saveReaction(rxn);
      }

      *buf = self.tmp_string.ptr();
      *size = self.tmp_string.size();
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoUnserialize (char *buf, int size)
{
   INDIGO_BEGIN
   {
      if (size > 3 && memcmp(buf, "ICM", 3) == 0)
      {
         BufferScanner scanner(buf, size);
         IcmLoader loader(scanner);
         AutoPtr<IndigoMolecule> im(new IndigoMolecule());
         loader.loadMolecule(im->mol);
         im->highlighting.init(im->mol);
         return self.addObject(im.release());
      }
      else if (size > 3 && memcmp(buf, "ICR", 3) == 0)
      {
         BufferScanner scanner(buf, size);
         IcrLoader loader(scanner);
         AutoPtr<IndigoReaction> ir(new IndigoReaction());
         loader.loadReaction(ir->rxn);
         ir->highlighting.init(ir->rxn);
         return self.addObject(ir.release());
      }
      else
         throw IndigoError("indigoUnserialize(): format not recognized");
   }
   INDIGO_END(-1)
}

CEXPORT int indigoMapAtom (int handle, int atom)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      if (obj.type == IndigoObject::MOLECULE_SUBSTRUCTURE_MATCH)
      {
         IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

         IndigoMoleculeSubstructureMatch &match = (IndigoMoleculeSubstructureMatch &)obj;
         match.query.getAtom(ia.idx); // will throw an exception if the atom index is invalid
         int idx = match.query_atom_mapping[ia.idx];
         if (idx < 0)
            return 0;

         return self.addObject(new IndigoAtom(match.target, idx));
      }
      if (obj.type == IndigoObject::MAPPING)
      {
         IndigoAtom &ia = IndigoAtom::cast(self.getObject(atom));

         IndigoMapping &mapping = (IndigoMapping &)obj;

         int mapped = mapping.mapping[ia.idx];
         if (mapped < 0)
            return 0;
         return self.addObject(new IndigoAtom(mapping.to, mapped));
      }

      throw IndigoError("indigoMapAtom(): not applicable to %s", obj.debugInfo());
   }
   INDIGO_END(-1)
}


CEXPORT int indigoClear (int item)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);

      if (IndigoArray::is(obj))
      {
         IndigoArray &array = IndigoArray::cast(obj);

         array.objects.clear();
      }
      else if (obj.isBaseMolecule())
         obj.getBaseMolecule().clear();
      else if (obj.isBaseReaction())
         obj.getBaseReaction().clear();
      else
         throw IndigoError("indigoClear(): do not know how to clear %s", obj.debugInfo());
      return 1;
   }
   INDIGO_END(-1);
}
