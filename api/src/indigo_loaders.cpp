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

#include "indigo_loaders.h"
#include "indigo_io.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "molecule/sdf_loader.h"
#include "molecule/rdf_loader.h"
#include "molecule/molfile_loader.h"
#include "molecule/smiles_loader.h"
#include "reaction/rsmiles_loader.h"
#include "base_cpp/scanner.h"
#include "reaction/rxnfile_loader.h"
#include "molecule/sdf_loader.h"
#include "molecule/multiple_cml_loader.h"
#include "molecule/molecule_cml_loader.h"
#include "reaction/reaction_cml_loader.h"

IndigoSdfLoader::IndigoSdfLoader (Scanner &scanner) :
IndigoObject(SDF_LOADER)
{
   _own_scanner = 0;
   sdf_loader = 0;
   sdf_loader = new SdfLoader(scanner);
}

IndigoSdfLoader::IndigoSdfLoader (const char *filename) :
IndigoObject(SDF_LOADER)
{
   _own_scanner = 0;
   sdf_loader = 0;

   // AutoPtr guard in case of exception in SdfLoader (happens in case of empty file)
   AutoPtr<FileScanner> scanner(new FileScanner(indigoGetInstance().filename_encoding, filename));
   sdf_loader = new SdfLoader(*scanner.get());
   _own_scanner = scanner.release();
}

IndigoSdfLoader::~IndigoSdfLoader ()
{
   delete sdf_loader;
   if (_own_scanner != 0)
      delete _own_scanner;
}

IndigoRdfData::IndigoRdfData (int type, Array<char> &data, int index, int offset) :
IndigoObject(type)
{
   _loaded = false;
   _data.copy(data);

   _index = index;
   _offset = offset;
}

IndigoRdfData::IndigoRdfData (int type, Array<char> &data, RedBlackStringObjMap< Array<char> > &properties,
                              int index, int offset) :
IndigoObject(type)
{
   _loaded = false;
   _data.copy(data);

   _properties.copy(properties);

   _index = index;
   _offset = offset;
}

IndigoRdfData::~IndigoRdfData ()
{
}

Array<char> & IndigoRdfData::getRawData ()
{
   return _data;
}

int IndigoRdfData::tell ()
{
   return _offset;
}

int IndigoRdfData::getIndex ()
{
   return _index;
}

RedBlackStringObjMap< Array<char> > * IndigoRdfData::getProperties ()
{
   return &_properties;
}

IndigoRdfMolecule::IndigoRdfMolecule (Array<char> &data, RedBlackStringObjMap< Array<char> > &properties,
                                      int index, int offset) :
IndigoRdfData(RDF_MOLECULE, data, properties, index, offset)
{
}

Molecule & IndigoRdfMolecule::getMolecule ()
{
   if (!_loaded)
   {
      Indigo &self = indigoGetInstance();
      BufferScanner scanner(_data);
      MolfileLoader loader(scanner);

      loader.ignore_stereocenter_errors = self.ignore_stereochemistry_errors;
      loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;
      loader.skip_3d_chirality = self.skip_3d_chirality;
      loader.loadMolecule(_mol);
      _loaded = true;
   }

   return _mol;
}

BaseMolecule & IndigoRdfMolecule::getBaseMolecule ()
{
   return getMolecule();
}


const char * IndigoRdfMolecule::getName ()
{
   if (_loaded)
      return _mol.name.ptr();

   Indigo &self = indigoGetInstance();

   BufferScanner scanner(_data);
   scanner.readLine(self.tmp_string, true);
   return self.tmp_string.ptr();
}

IndigoObject * IndigoRdfMolecule::clone ()
{
   return IndigoMolecule::cloneFrom(*this);
}

IndigoRdfMolecule::~IndigoRdfMolecule ()
{
}

IndigoRdfReaction::IndigoRdfReaction (Array<char> &data, RedBlackStringObjMap< Array<char> > &properties,
                                      int index, int offset) :
IndigoRdfData(RDF_REACTION, data, properties, index, offset)
{
}

Reaction & IndigoRdfReaction::getReaction ()
{
   if (!_loaded)
   {
      Indigo &self = indigoGetInstance();
      BufferScanner scanner(_data);
      RxnfileLoader loader(scanner);

      loader.ignore_stereocenter_errors = self.ignore_stereochemistry_errors;
      loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;
      loader.loadReaction(_rxn);
      _loaded = true;
   }

   return _rxn;
}

BaseReaction & IndigoRdfReaction::getBaseReaction ()
{
   return getReaction();
}

const char * IndigoRdfReaction::getName ()
{
   if (_loaded)
      return _rxn.name.ptr();

   Indigo &self = indigoGetInstance();

   BufferScanner scanner(_data);
   scanner.readLine(self.tmp_string, true);
   if (strcmp(self.tmp_string.ptr(), "$RXN") != 0)
      throw IndigoError("IndigoRdfReaction::getName(): unexpected first line in the files with reactions."
         "'%s' has been found but '$RXN' has been expected.");
   // Read next line with the name
   scanner.readLine(self.tmp_string, true);
   return self.tmp_string.ptr();
}

IndigoObject * IndigoRdfReaction::clone ()
{
   return IndigoReaction::cloneFrom(*this);
}

IndigoRdfReaction::~IndigoRdfReaction ()
{
}

IndigoObject * IndigoSdfLoader::next ()
{
   if (sdf_loader->isEOF())
      return 0;

   int counter = sdf_loader->currentNumber();
   int offset = sdf_loader->tell();

   sdf_loader->readNext();

   return new IndigoRdfMolecule(sdf_loader->data, sdf_loader->properties,
                                counter, offset);
}

IndigoObject * IndigoSdfLoader::at (int index)
{
   sdf_loader->readAt(index);

   return new IndigoRdfMolecule(sdf_loader->data, sdf_loader->properties,
                                index, 0);
}

bool IndigoSdfLoader::hasNext ()
{
   return !sdf_loader->isEOF();
}

int IndigoSdfLoader::tell ()
{
   return sdf_loader->tell();
}

IndigoRdfLoader::IndigoRdfLoader (Scanner &scanner) :
IndigoObject(RDF_LOADER)
{
   _own_scanner = 0;
   rdf_loader = 0;

   rdf_loader = new RdfLoader(scanner);
}

IndigoRdfLoader::IndigoRdfLoader (const char *filename) :
IndigoObject(RDF_LOADER)
{
   _own_scanner = 0;
   rdf_loader = 0;
   
   _own_scanner = new FileScanner(indigoGetInstance().filename_encoding, filename);
   rdf_loader = new RdfLoader(*_own_scanner);
}

IndigoRdfLoader::~IndigoRdfLoader ()
{
   delete rdf_loader;
   if (_own_scanner != 0)
      delete _own_scanner;
}

IndigoObject * IndigoRdfLoader::next ()
{
   if (rdf_loader->isEOF())
      return 0;

   int counter = rdf_loader->currentNumber();
   int offset = rdf_loader->tell();

   rdf_loader->readNext();

   if (rdf_loader->isMolecule())
      return new IndigoRdfMolecule(rdf_loader->data, rdf_loader->properties,
                                   counter, offset);
   else
      return new IndigoRdfReaction(rdf_loader->data, rdf_loader->properties,
                                   counter, offset);
}

IndigoObject * IndigoRdfLoader::at (int index)
{
   rdf_loader->readAt(index);

   if (rdf_loader->isMolecule())
      return new IndigoRdfMolecule(rdf_loader->data, rdf_loader->properties, index, 0);
   else
      return new IndigoRdfReaction(rdf_loader->data, rdf_loader->properties, index, 0);

}


int IndigoRdfLoader::tell ()
{
   return rdf_loader->tell();
}

bool IndigoRdfLoader::hasNext ()
{
   return !rdf_loader->isEOF();
}

IndigoSmilesMolecule::IndigoSmilesMolecule (Array<char> &smiles, int index, int offset) :
IndigoRdfData(SMILES_MOLECULE, smiles, index, offset)
{
}

IndigoSmilesMolecule::~IndigoSmilesMolecule ()
{
}

Molecule & IndigoSmilesMolecule::getMolecule ()
{
   Indigo &self = indigoGetInstance();
   if (!_loaded)
   {
      BufferScanner scanner(_data);
      SmilesLoader loader(scanner);

      loader.ignore_stereochemistry_errors = self.ignore_stereochemistry_errors;
      loader.loadMolecule(_mol);
      _loaded = true;
   }
   return _mol;
}

BaseMolecule & IndigoSmilesMolecule::getBaseMolecule ()
{
   return getMolecule();
}

const char * IndigoSmilesMolecule::getName ()
{
   if (getMolecule().name.ptr() == 0)
      return "";
   return getMolecule().name.ptr();
}

IndigoObject * IndigoSmilesMolecule::clone ()
{
   return IndigoMolecule::cloneFrom(*this);
}

IndigoSmilesReaction::IndigoSmilesReaction (Array<char> &smiles, int index, int offset) :
IndigoRdfData(SMILES_REACTION, smiles, index, offset)
{
}

IndigoSmilesReaction::~IndigoSmilesReaction ()
{
}

Reaction & IndigoSmilesReaction::getReaction ()
{
   if (!_loaded)
   {
      BufferScanner scanner(_data);
      RSmilesLoader loader(scanner);

      loader.loadReaction(_rxn);
      _loaded = true;
   }
   return _rxn;
}

BaseReaction & IndigoSmilesReaction::getBaseReaction ()
{
   return getReaction();
}

const char * IndigoSmilesReaction::getName ()
{
   if (getReaction().name.ptr() == 0)
      return "";
   return getReaction().name.ptr();
}

IndigoObject * IndigoSmilesReaction::clone ()
{
   return IndigoReaction::cloneFrom(*this);
}

IndigoMultilineSmilesLoader::IndigoMultilineSmilesLoader (Scanner &scanner) :
IndigoObject(MULTILINE_SMILES_LOADER),
TL_CP_GET(_offsets)
{
   _own_scanner = false;
   _scanner = &scanner;

   _current_number = 0;
   _max_offset = 0;
   _offsets.clear();
}

IndigoMultilineSmilesLoader::IndigoMultilineSmilesLoader (const char *filename) :
IndigoObject(MULTILINE_SMILES_LOADER),
TL_CP_GET(_offsets)
{
   _scanner = 0;
   _scanner = new FileScanner(indigoGetInstance().filename_encoding, filename);
   _own_scanner = true;

   _current_number = 0;
   _max_offset = 0;
   _offsets.clear();
}


IndigoMultilineSmilesLoader::~IndigoMultilineSmilesLoader ()
{
   if (_own_scanner)
      delete _scanner;
}

void IndigoMultilineSmilesLoader::_advance ()
{
   _offsets.expand(_current_number + 1);
   _offsets[_current_number++] = _scanner->tell();
   _scanner->readLine(_str, false);

   if (_scanner->tell() > _max_offset)
      _max_offset = _scanner->tell();
}

IndigoObject * IndigoMultilineSmilesLoader::next ()
{
   if (_scanner->isEOF())
      return 0;

   int offset = _scanner->tell();
   int counter = _current_number;

   _advance();

   if (_str.find('>') == -1)
      return new IndigoSmilesMolecule(_str, counter, offset);
   else
      return new IndigoSmilesReaction(_str, counter, offset);
}

bool IndigoMultilineSmilesLoader::hasNext ()
{
   return !_scanner->isEOF();
}

int IndigoMultilineSmilesLoader::tell ()
{
   return _scanner->tell();
}

int IndigoMultilineSmilesLoader::count ()
{
   int offset = _scanner->tell();
   int cn = _current_number;

   if (offset != _max_offset)
   {
      _scanner->seek(_max_offset, SEEK_SET);
      _current_number = _offsets.size();
   }

   while (!_scanner->isEOF())
      _advance();

   int res = _current_number;

   if (res != cn)
   {
      _scanner->seek(offset, SEEK_SET);
      _current_number = cn;
   }

   return res;
}

IndigoObject * IndigoMultilineSmilesLoader::at (int index)
{
   if (index < _offsets.size())
   {
      _scanner->seek(_offsets[index], SEEK_SET);
      _current_number = index;
      return next();
   }
   _scanner->seek(_max_offset, SEEK_SET);
   _current_number = _offsets.size();
   while (index > _offsets.size())
      _advance();
   return next();
}

CEXPORT int indigoIterateSDF (int reader)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(reader);

      return self.addObject(new IndigoSdfLoader(IndigoScanner::get(obj)));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateRDF (int reader)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(reader);

      return self.addObject(new IndigoRdfLoader(IndigoScanner::get(obj)));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateSmiles (int reader)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(reader);

      return self.addObject(new IndigoMultilineSmilesLoader(IndigoScanner::get(obj)));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoTell (int handle)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(handle);

      if (obj.type == IndigoObject::SDF_LOADER)
         return ((IndigoSdfLoader &)obj).tell();
      if (obj.type == IndigoObject::RDF_LOADER)
         return ((IndigoRdfLoader &)obj).tell();
      if (obj.type == IndigoObject::MULTILINE_SMILES_LOADER)
         return ((IndigoMultilineSmilesLoader &)obj).tell();
      if (obj.type == IndigoObject::RDF_MOLECULE ||
          obj.type == IndigoObject::RDF_REACTION ||
          obj.type == IndigoObject::SMILES_MOLECULE ||
          obj.type == IndigoObject::SMILES_REACTION)
         return ((IndigoRdfData &)obj).tell();
      if (obj.type == IndigoObject::MULTIPLE_CML_LOADER)
         return ((IndigoMultipleCmlLoader &)obj).tell();
      if (obj.type == IndigoObject::CML_MOLECULE)
         return ((IndigoCmlMolecule &)obj).tell();
      if (obj.type == IndigoObject::CML_REACTION)
         return ((IndigoCmlReaction &)obj).tell();

      throw IndigoError("indigoTell(): not applicable to %s", obj.debugInfo());
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateSDFile (const char *filename)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoSdfLoader(filename));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateRDFile (const char *filename)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoRdfLoader(filename));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateSmilesFile (const char *filename)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoMultilineSmilesLoader(filename));
   }
   INDIGO_END(-1)
}


IndigoCmlMolecule::IndigoCmlMolecule (Array<char> &data, int index, int offset) :
IndigoRdfData(CML_MOLECULE, data, index, offset)
{
}

IndigoCmlMolecule::~IndigoCmlMolecule ()
{
}

Molecule & IndigoCmlMolecule::getMolecule ()
{
   if (!_loaded)
   {
      Indigo &self = indigoGetInstance();

      BufferScanner scanner(_data);
      MoleculeCmlLoader loader(scanner);
      loader.ignore_stereochemistry_errors = self.ignore_stereochemistry_errors;
      loader.loadMolecule(_mol);
      _loaded = true;
   }
   return _mol;
}

BaseMolecule & IndigoCmlMolecule::getBaseMolecule ()
{
   return getMolecule();
}

const char * IndigoCmlMolecule::getName ()
{
   return getMolecule().name.ptr();
}

IndigoObject * IndigoCmlMolecule::clone ()
{
   return IndigoMolecule::cloneFrom(*this);
}

const char * IndigoCmlMolecule::debugInfo ()
{
   return "<cml molecule>";
}

IndigoCmlReaction::IndigoCmlReaction (Array<char> &data, int index, int offset) :
IndigoRdfData(CML_REACTION, data, index, offset)
{
}

IndigoCmlReaction::~IndigoCmlReaction ()
{
}

Reaction & IndigoCmlReaction::getReaction ()
{
   if (!_loaded)
   {
      Indigo &self = indigoGetInstance();

      BufferScanner scanner(_data);
      ReactionCmlLoader loader(scanner);
      loader.ignore_stereochemistry_errors = self.ignore_stereochemistry_errors;
      loader.loadReaction(_rxn);
      _loaded = true;
   }
   return _rxn;
}

BaseReaction & IndigoCmlReaction::getBaseReaction ()
{
   return getReaction();
}

const char * IndigoCmlReaction::getName ()
{
   return getReaction().name.ptr();
}

IndigoObject * IndigoCmlReaction::clone ()
{
   return IndigoReaction::cloneFrom(*this);
}

const char * IndigoCmlReaction::debugInfo ()
{
   return "<cml reaction>";
}

IndigoMultipleCmlLoader::IndigoMultipleCmlLoader (Scanner &scanner) :
IndigoObject(MULTIPLE_CML_LOADER)
{
   _own_scanner = 0;
   loader = new MultipleCmlLoader(scanner);
}

IndigoMultipleCmlLoader::IndigoMultipleCmlLoader (const char *filename) :
IndigoObject(MULTIPLE_CML_LOADER)
{
   _own_scanner = new FileScanner(filename);
   loader = new MultipleCmlLoader(*_own_scanner);
}

IndigoMultipleCmlLoader::~IndigoMultipleCmlLoader ()
{
   delete loader;
   if (_own_scanner != 0)
      delete _own_scanner;
}

int IndigoMultipleCmlLoader::tell ()
{
   return loader->tell();
}

bool IndigoMultipleCmlLoader::hasNext ()
{
   return !loader->isEOF();
}

IndigoObject * IndigoMultipleCmlLoader::next ()
{
   if (!hasNext())
      return 0;

   int counter = loader->currentNumber();
   int offset = loader->tell();

   loader->readNext();

   if (loader->isReaction())
      return new IndigoCmlReaction(loader->data, counter, offset);
   else
      return new IndigoCmlMolecule(loader->data, counter, offset);
}

CEXPORT int indigoIterateCML (int reader)
{
   INDIGO_BEGIN
   {
      IndigoObject &obj = self.getObject(reader);

      return self.addObject(new IndigoMultipleCmlLoader(IndigoScanner::get(obj)));
   }
   INDIGO_END(-1)
}

CEXPORT int indigoIterateCMLFile (const char *filename)
{
   INDIGO_BEGIN
   {
      return self.addObject(new IndigoMultipleCmlLoader(filename));
   }
   INDIGO_END(-1)
}
