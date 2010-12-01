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
#include "molecule/sdf_loader.h"
#include "molecule/rdf_loader.h"
#include "molecule/molfile_loader.h"
#include "molecule/smiles_loader.h"
#include "reaction/rsmiles_loader.h"
#include "base_cpp/scanner.h"
#include "reaction/rxnfile_loader.h"
#include "molecule/sdf_loader.h"

IndigoSdfLoader::IndigoSdfLoader (Scanner &scanner) :
IndigoObject(SDF_LOADER)
{
   _own_scanner = 0;
   _counter = 0;
   sdf_loader = 0;
   sdf_loader = new SdfLoader(scanner);
}

IndigoSdfLoader::IndigoSdfLoader (const char *filename) :
IndigoObject(SDF_LOADER)
{
   _own_scanner = 0;
   _counter = 0;
   sdf_loader = 0;

   _own_scanner = new FileScanner(indigoGetInstance().filename_encoding, filename);
   sdf_loader = new SdfLoader(*_own_scanner);
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

   for (int i = properties.begin(); i != properties.end(); i = properties.next(i))
      _properties.value(_properties.insert(properties.key(i))).copy(properties.value(i));

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
      loader.highlighting = &_highlighting;
      loader.loadMolecule(_mol);
      _loaded = true;
   }

   return _mol;
}

BaseMolecule & IndigoRdfMolecule::getBaseMolecule ()
{
   return getMolecule();
}

GraphHighlighting * IndigoRdfMolecule::getMoleculeHighlighting ()
{
   return &_highlighting;
}

const char * IndigoRdfMolecule::getName ()
{
   if (_loaded)
      return _mol.name.ptr();

   Indigo &self = indigoGetInstance();

   BufferScanner scanner(_data);
   scanner.readString(self.tmp_string, true);
   return self.tmp_string.ptr();
}

IndigoObject * IndigoRdfMolecule::clone ()
{
   AutoPtr<IndigoMolecule> molptr;
   molptr.reset(new IndigoMolecule());
   molptr->mol.clone(getMolecule(), 0, 0);
   molptr->copyProperties(_properties);
   return molptr.release();
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
      loader.highlighting = &_highlighting;
      loader.loadReaction(_rxn);
      _loaded = true;
   }

   return _rxn;
}

BaseReaction & IndigoRdfReaction::getBaseReaction ()
{
   return getReaction();
}

ReactionHighlighting * IndigoRdfReaction::getReactionHighlighting ()
{
   return &_highlighting;
}

const char * IndigoRdfReaction::getName ()
{
   if (_loaded)
      return _rxn.name.ptr();

   Indigo &self = indigoGetInstance();

   BufferScanner scanner(_data);
   scanner.readString(self.tmp_string, true);
   return self.tmp_string.ptr();
}

IndigoObject * IndigoRdfReaction::clone ()
{
   AutoPtr<IndigoReaction> rxnptr;
   rxnptr.reset(new IndigoReaction());
   rxnptr->rxn.clone(getReaction(), 0, 0);
   rxnptr->highlighting.init(rxnptr->rxn);
   rxnptr->copyProperties(_properties);
   return rxnptr.release();
}

IndigoRdfReaction::~IndigoRdfReaction ()
{
}

IndigoObject * IndigoSdfLoader::next ()
{
   if (sdf_loader->isEOF())
      return 0;

   int counter = _counter;
   int offset = sdf_loader->tell();

   sdf_loader->readNext();
   _counter++;

   return new IndigoRdfMolecule(sdf_loader->data, sdf_loader->properties,
                                counter, offset);
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
   _counter = 0;
   _own_scanner = 0;
   rdf_loader = 0;

   rdf_loader = new RdfLoader(scanner);
}

IndigoRdfLoader::IndigoRdfLoader (const char *filename) :
IndigoObject(RDF_LOADER)
{
   _counter = 0;
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

   int counter = _counter;
   int offset = rdf_loader->tell();

   rdf_loader->readNext();
   _counter++;

   if (rdf_loader->isMolecule())
      return new IndigoRdfMolecule(rdf_loader->data, rdf_loader->properties,
                                   counter, offset);
   else
      return new IndigoRdfReaction(rdf_loader->data, rdf_loader->properties,
                                   counter, offset);
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
   if (!_loaded)
   {
      BufferScanner scanner(_data);
      SmilesLoader loader(scanner);

      loader.highlighting = &_highlighting;
      loader.loadMolecule(_mol);
      _loaded = true;
   }
   return _mol;
}

BaseMolecule & IndigoSmilesMolecule::getBaseMolecule ()
{
   return getMolecule();
}

GraphHighlighting * IndigoSmilesMolecule::getMoleculeHighlighting ()
{
   return &_highlighting;
}

const char * IndigoSmilesMolecule::getName ()
{
   return getMolecule().name.ptr();
}

IndigoObject * IndigoSmilesMolecule::clone ()
{
   AutoPtr<IndigoMolecule> molptr;
   molptr.reset(new IndigoMolecule());
   molptr->mol.clone(getMolecule(), 0, 0);
   return molptr.release();
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

      loader.highlighting = &_highlighting;
      loader.loadReaction(_rxn);
      _loaded = true;
   }
   return _rxn;
}

BaseReaction & IndigoSmilesReaction::getBaseReaction ()
{
   return getReaction();
}

ReactionHighlighting * IndigoSmilesReaction::getReactionHighlighting ()
{
   return &_highlighting;
}

const char * IndigoSmilesReaction::getName ()
{
   return getReaction().name.ptr();
}

IndigoObject * IndigoSmilesReaction::clone ()
{
   AutoPtr<IndigoReaction> rxnptr;
   rxnptr.reset(new IndigoReaction());
   rxnptr->rxn.clone(getReaction(), 0, 0);
   rxnptr->highlighting.init(rxnptr->rxn);
   return rxnptr.release();
}

IndigoMultilineSmilesLoader::IndigoMultilineSmilesLoader (Scanner &scanner) :
IndigoObject(MULTILINE_SMILES_LOADER)
{
   _counter = 0;
   _own_scanner = false;
   _scanner = &scanner;
}

IndigoMultilineSmilesLoader::IndigoMultilineSmilesLoader (const char *filename) :
IndigoObject(MULTILINE_SMILES_LOADER)
{
   _counter = 0;
   _scanner = 0;
   _scanner = new FileScanner(indigoGetInstance().filename_encoding, filename);
   _own_scanner = true;
}


IndigoMultilineSmilesLoader::~IndigoMultilineSmilesLoader ()
{
   if (_own_scanner)
      delete _scanner;
}

IndigoObject * IndigoMultilineSmilesLoader::next ()
{
   if (_scanner->isEOF())
      return 0;

   int counter = _counter;
   int offset = _scanner->tell();

   _scanner->readString(_str, false);

   _counter++;

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
