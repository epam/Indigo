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

#include "indigo_savers.h"

#include "indigo_io.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/auto_ptr.h"
#include "molecule/molecule_cml_saver.h"
#include "molecule/molfile_saver.h"
#include "molecule/smiles_saver.h"
#include "reaction/rxnfile_saver.h"
#include "reaction/rsmiles_saver.h"
#include "reaction/reaction_cml_saver.h"

#include <time.h>

//
// IndigoSaver
//

IndigoSaver::IndigoSaver (Output &output) : IndigoObject(IndigoObject::SAVER),
   _output(output)
{
   _closed = false;
   _own_output = 0;
}

IndigoSaver::~IndigoSaver ()
{
   close();
}

void IndigoSaver::acquireOutput (Output *output)
{
   if (_own_output)
      delete _own_output;
   _own_output = output;
}

void IndigoSaver::close ()
{
   if (_closed)
      return;
   _appendFooter();

   delete _own_output;
   _own_output = 0;

   _closed = true;
}

IndigoSaver* IndigoSaver::create (Output &output, const char *type)
{
   AutoPtr<IndigoSaver> saver;
   if (strcasecmp(type, "sdf") == 0)
      saver = new IndigoSdfSaver(output);
   else if (strcasecmp(type, "smiles") == 0 || strcasecmp(type, "smi") == 0)
      saver = new IndigoSmilesSaver(output);
   else if (strcasecmp(type, "cml") == 0)
      saver =  new IndigoCmlSaver(output);
   else if (strcasecmp(type, "rdf") == 0)
      saver =  new IndigoRdfSaver(output);
   else
      throw IndigoError("unsupported saver type: '%s'. Supported formats are sdf, smiles, cml, rdf", type);

   saver->_appendHeader();
   return saver.release();
}

void IndigoSaver::appendObject (IndigoObject &object)
{
   if (_closed)
      throw IndigoError("save %s has already been closed", debugInfo());
   _append(object);
}

//
// IndigoSDFSaver
//

void IndigoSdfSaver::appendMolfile (Output &out, IndigoObject &obj)
{
   Indigo &indigo = indigoGetInstance();

   MolfileSaver saver(out);
   indigo.initMolfileSaver(saver);
   saver.saveBaseMolecule(obj.getBaseMolecule());
}

void IndigoSdfSaver::append (Output &out, IndigoObject &obj)
{
   appendMolfile(out, obj);

   RedBlackStringObjMap< Array<char> > *props = obj.getProperties();
   if (props != 0)
   {
      int i;

      for (i = props->begin(); i != props->end(); i = props->next(i))
         out.printf(">  <%s>\n%s\n\n", props->key(i), props->value(i).ptr());
   }

   out.printfCR("$$$$");
   out.flush();
}

const char * IndigoSdfSaver::debugInfo () 
{
   return "<SDF saver>";
}

void IndigoSdfSaver::_append (IndigoObject &object)
{
   append(_output, object);
}

CEXPORT int indigoSdfAppend (int output, int molecule)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(molecule);
      Output &out = IndigoOutput::get(self.getObject(output));
      IndigoSdfSaver::append(out, obj);
      return 1;
   }
   INDIGO_END(-1)
}

//
// IndigoSmilesSaver
//

void IndigoSmilesSaver::generateSmiles (IndigoObject &obj, Array<char> &out_buffer)
{
   ArrayOutput output(out_buffer);
   if (IndigoBaseMolecule::is(obj))
   {
      BaseMolecule &mol = obj.getBaseMolecule();
         
      SmilesSaver saver(output);
         
      if (mol.isQueryMolecule())
         saver.saveQueryMolecule(mol.asQueryMolecule());
      else
         saver.saveMolecule(mol.asMolecule());
   }
   else if (IndigoBaseReaction::is(obj))
   {
      BaseReaction &rxn = obj.getBaseReaction();
         
      RSmilesSaver saver(output);
         
      if (rxn.isQueryReaction())
         saver.saveQueryReaction(rxn.asQueryReaction());
      else
         saver.saveReaction(rxn.asReaction());
   }
   else
      throw IndigoError("%s can not be converted to SMILES", obj.debugInfo());
   out_buffer.push(0);
}

void IndigoSmilesSaver::append (Output &output, IndigoObject &object)
{
   QS_DEF(Array<char>, tmp_buffer);
   IndigoSmilesSaver::generateSmiles(object, tmp_buffer);
   output.writeString(tmp_buffer.ptr());

   Indigo &indigo = indigoGetInstance();
   if (indigo.smiles_saving_write_name)
   {
      output.writeString(" ");
      output.writeString(object.getName());
   }
   output.writeCR();
   output.flush();
}

const char * IndigoSmilesSaver::debugInfo () 
{
   return "<smiles saver>";
}

void IndigoSmilesSaver::_append (IndigoObject &object)
{
   append(_output, object);
}

CEXPORT int indigoSmilesAppend (int output, int item)
{
   INDIGO_BEGIN
   {
      Output &out = IndigoOutput::get(self.getObject(output));
      IndigoObject &obj = self.getObject(item);
      IndigoSmilesSaver::append(out, obj);
      out.flush();
      return 1;
   }
   INDIGO_END(-1)
}

//
// IndigoCMLSaver
// 

void IndigoCmlSaver::append (Output &out, IndigoObject &obj)
{
   if (IndigoBaseMolecule::is(obj))
   {
      MoleculeCmlSaver saver(out);
      saver.skip_cml_tag = true;
      saver.saveMolecule(obj.getMolecule());
   }
   else if (IndigoBaseReaction::is(obj))
   {
      ReactionCmlSaver saver(out);
      saver.skip_cml_tag = true;
      saver.saveReaction(obj.getReaction());
   }
   else
      throw IndigoError("%s can not be saved to CML", obj.debugInfo());
}

void IndigoCmlSaver::appendHeader (Output &out)
{
   out.printf("<?xml version=\"1.0\" ?>\n");
   out.printf("<cml>\n");
}

void IndigoCmlSaver::appendFooter (Output &out)
{
   out.printf("</cml>\n");
}

const char * IndigoCmlSaver::debugInfo () 
{
   return "<CML saver>";
}

void IndigoCmlSaver::_append (IndigoObject &object)
{
   append(_output, object);
}

void IndigoCmlSaver::_appendHeader ()
{
   appendHeader(_output);
}

void IndigoCmlSaver::_appendFooter ()
{
   appendFooter(_output);
}

CEXPORT int indigoCmlHeader (int output)
{
   INDIGO_BEGIN
   {
      Output &out = IndigoOutput::get(self.getObject(output));
      IndigoCmlSaver::appendHeader(out);
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoCmlFooter (int output)
{
   INDIGO_BEGIN
   {
      Output &out = IndigoOutput::get(self.getObject(output));
      IndigoCmlSaver::appendFooter(out);
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
      IndigoCmlSaver::append(out, obj);
      return 1;
   }
   INDIGO_END(-1)
}

//
// IndigoRDFSaver
//

void IndigoRdfSaver::appendRXN (Output &out, IndigoObject &obj)
{
   Indigo &indigo = indigoGetInstance();

   RxnfileSaver saver(out);
   indigo.initRxnfileSaver(saver);
   saver.saveBaseReaction(obj.getBaseReaction());
}

void IndigoRdfSaver::append (Output &out, IndigoObject &obj)
{
   if (IndigoBaseMolecule::is(obj))
   {
      out.writeStringCR("$MFMT");
      IndigoSdfSaver::appendMolfile(out, obj);
   }
   else if (IndigoBaseReaction::is(obj))
   {
      out.writeStringCR("$RFMT");
      IndigoRdfSaver::appendRXN(out, obj);
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
}

void IndigoRdfSaver::appendHeader (Output &out)
{
   Indigo &indigo = indigoGetInstance();

   out.printfCR("$RDFILE 1");
   struct tm lt;
   if (indigo.molfile_saving_skip_date)
      memset(&lt, 0, sizeof(lt));
   else
   {
      time_t tm = time(NULL);
      lt = *localtime(&tm);
   }
   out.printfCR("$DATM    %02d/%02d/%02d %02d:%02d",
            lt.tm_mon + 1, lt.tm_mday, lt.tm_year % 100, lt.tm_hour, lt.tm_min);
}

const char * IndigoRdfSaver::debugInfo () 
{
   return "<smiles saver>";
}

void IndigoRdfSaver::_append (IndigoObject &object)
{
   append(_output, object);
}

void IndigoRdfSaver::_appendHeader ()
{
   appendHeader(_output);
}

CEXPORT int indigoRdfHeader (int output)
{
   INDIGO_BEGIN
   {
      Output &out = IndigoOutput::get(self.getObject(output));
      IndigoRdfSaver::appendHeader(out);
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
      IndigoRdfSaver::append(out, obj);
      return 1;
   }
   INDIGO_END(-1)
}

//
// Saving functions
//
CEXPORT int indigoCreateSaver (int output, const char *format)
{
   INDIGO_BEGIN
   {
      Output &out = IndigoOutput::get(self.getObject(output));
      return self.addObject(IndigoSaver::create(out, format));
   }
   INDIGO_END(-1)
}


CEXPORT int indigoCreateFileSaver (const char *filename, const char *format)
{
   INDIGO_BEGIN
   {
      AutoPtr<FileOutput> output(new FileOutput(self.filename_encoding, filename));
      AutoPtr<IndigoSaver> saver(IndigoSaver::create(output.ref(), format));
      saver->acquireOutput(output.release());
      return self.addObject(saver.release());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSaveMolfile (int molecule, int output)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(molecule);
      Output &out = IndigoOutput::get(self.getObject(output));

      IndigoSdfSaver::appendMolfile(out, obj);
      out.flush();
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSaveCml (int item, int output)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);
      Output &out = IndigoOutput::get(self.getObject(output));

      if (IndigoBaseMolecule::is(obj))
      {
         Molecule &mol = obj.getMolecule();
         MoleculeCmlSaver saver(out);
         
         saver.saveMolecule(mol);
         out.flush();
         return 1;
      }
      if (IndigoBaseReaction::is(obj))
      {
         Reaction &rxn = obj.getReaction();
         ReactionCmlSaver saver(out);

         saver.saveReaction(rxn);
         out.flush();
         return 1;
      }
      throw IndigoError("indigoSaveCml(): expected molecule or reaction, got %s", obj.debugInfo());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSaveMDLCT (int item, int output)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(item);
      QS_DEF(Array<char>, buf);
      ArrayOutput out(buf);

      if (IndigoBaseMolecule::is(obj))
         IndigoSdfSaver::appendMolfile(out, obj);
      else if (IndigoBaseReaction::is(obj))
         IndigoRdfSaver::appendRXN(out, obj);

      Output &out2 = IndigoOutput::get(self.getObject(output));

      BufferScanner scanner(buf);
      QS_DEF(Array<char>, line);

      while (!scanner.isEOF())
      {
         scanner.readLine(line, false);
         if (line.size() > 255)
            throw IndigoError("indigoSaveMDLCT: line too big (%d)", line.size());
         out2.writeChar(line.size());
         out2.writeArray(line);
      }
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoSaveRxnfile (int reaction, int output)
{
   INDIGO_BEGIN
   {
      BaseReaction &rxn = self.getObject(reaction).getBaseReaction();
      Output &out = IndigoOutput::get(self.getObject(output));
      
      RxnfileSaver saver(out);
      self.initRxnfileSaver(saver);
      if (rxn.isQueryReaction())
         saver.saveQueryReaction(rxn.asQueryReaction());
      else
         saver.saveReaction(rxn.asReaction());
      out.flush();
      return 1;
   }
   INDIGO_END(-1)
}

CEXPORT int indigoAppend (int saver_id, int object)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(object);
      IndigoObject &saver_obj = self.getObject(saver_id);
      if (saver_obj.type != IndigoObject::SAVER)
         throw IndigoError("indigoAppend() is only applicable to saver objects. %s object was passed as a saver", 
            saver_obj.debugInfo());
      IndigoSaver &saver = (IndigoSaver &)saver_obj;
      saver.appendObject(obj);
      return 1;
   }
   INDIGO_END(-1)
}
