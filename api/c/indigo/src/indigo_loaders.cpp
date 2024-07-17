/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "indigo_loaders.h"
#include "base_cpp/scanner.h"
#include "indigo_io.h"
#include "indigo_molecule.h"
#include "indigo_reaction.h"
#include "molecule/cml_loader.h"
#include "molecule/molecule_cdxml_loader.h"
#include "molecule/molfile_loader.h"
#include "molecule/multiple_cdx_loader.h"
#include "molecule/multiple_cml_loader.h"
#include "molecule/rdf_loader.h"
#include "molecule/sdf_loader.h"
#include "molecule/smiles_loader.h"
#include "reaction/reaction_cdxml_loader.h"
#include "reaction/reaction_cml_loader.h"
#include "reaction/reaction_json_loader.h"
#include "reaction/rsmiles_loader.h"
#include "reaction/rxnfile_loader.h"

#include <limits>

using namespace rapidjson;

IndigoJSONMolecule::IndigoJSONMolecule(Document& ket, MonomerTemplateLibrary& library) : IndigoObject(JSON_MOLECULE), _loader(ket), _loaded(false)
{
}

Molecule& IndigoJSONMolecule::getMolecule()
{
    if (!_loaded)
    {
        _loader.loadMolecule(_mol);
        _loaded = true;
    }
    return _mol;
}

BaseMolecule& IndigoJSONMolecule::getBaseMolecule()
{
    return getMolecule();
}

IndigoObject* IndigoJSONMolecule::clone()
{
    return IndigoMolecule::cloneFrom(*this);
}

const char* IndigoJSONMolecule::getName()
{
    if (getMolecule().name.ptr() == 0)
        return "";
    return getMolecule().name.ptr();
}

IndigoJSONMolecule::~IndigoJSONMolecule()
{
}

IndigoSdfLoader::IndigoSdfLoader(Scanner& scanner) : IndigoObject(SDF_LOADER)
{
    sdf_loader = std::make_unique<SdfLoader>(scanner);
}

IndigoSdfLoader::IndigoSdfLoader(const char* filename) : IndigoObject(SDF_LOADER)
{
    // AutoPtr guard in case of exception in SdfLoader (happens in case of empty file)
    _own_scanner = std::make_unique<FileScanner>(indigoGetInstance().filename_encoding, filename);
    sdf_loader = std::make_unique<SdfLoader>(*_own_scanner);
}

IndigoSdfLoader::~IndigoSdfLoader()
{
}

IndigoRdfData::IndigoRdfData(int type, Array<char>& data, int index, long long offset) : IndigoObject(type)
{
    _loaded = false;
    _data.copy(data);

    _index = index;
    _offset = offset;
}

IndigoRdfData::IndigoRdfData(int type, Array<char>& data, PropertiesMap& properties, int index, long long offset) : IndigoObject(type)
{
    _loaded = false;
    _data.copy(data);

    _properties.copy(properties);

    _index = index;
    _offset = offset;
}

IndigoRdfData::~IndigoRdfData()
{
}

Array<char>& IndigoRdfData::getRawData()
{
    return _data;
}

long long IndigoRdfData::tell()
{
    return _offset;
}

int IndigoRdfData::getIndex()
{
    return _index;
}

IndigoRdfMolecule::IndigoRdfMolecule(Array<char>& data, PropertiesMap& properties, int index, long long offset)
    : IndigoRdfData(RDF_MOLECULE, data, properties, index, offset)
{
}

Molecule& IndigoRdfMolecule::getMolecule()
{
    if (!_loaded)
    {
        Indigo& self = indigoGetInstance();
        BufferScanner scanner(_data);
        MolfileLoader loader(scanner);

        loader.stereochemistry_options = self.stereochemistry_options;
        loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;
        loader.skip_3d_chirality = self.skip_3d_chirality;
        loader.ignore_noncritical_query_features = self.ignore_noncritical_query_features;
        loader.ignore_no_chiral_flag = self.ignore_no_chiral_flag;
        loader.treat_stereo_as = self.treat_stereo_as;
        loader.ignore_bad_valence = self.ignore_bad_valence;
        loader.loadMolecule(_mol);
        _loaded = true;
    }

    return _mol;
}

BaseMolecule& IndigoRdfMolecule::getBaseMolecule()
{
    return getMolecule();
}

const char* IndigoRdfMolecule::getName()
{
    if (_loaded)
        return _mol.name.ptr();

    Indigo& self = indigoGetInstance();

    BufferScanner scanner(_data);
    auto& tmp = self.getThreadTmpData();
    scanner.readLine(tmp.string, true);
    return tmp.string.ptr();
}

IndigoObject* IndigoRdfMolecule::clone()
{
    return IndigoMolecule::cloneFrom(*this);
}

IndigoRdfMolecule::~IndigoRdfMolecule()
{
}

IndigoRdfReaction::IndigoRdfReaction(Array<char>& data, PropertiesMap& properties, int index, long long offset)
    : IndigoRdfData(RDF_REACTION, data, properties, index, offset)
{
}

Reaction& IndigoRdfReaction::getReaction()
{
    if (!_loaded)
    {
        Indigo& self = indigoGetInstance();
        BufferScanner scanner(_data);
        RxnfileLoader loader(scanner);

        loader.stereochemistry_options = self.stereochemistry_options;
        loader.treat_x_as_pseudoatom = self.treat_x_as_pseudoatom;
        loader.ignore_noncritical_query_features = self.ignore_noncritical_query_features;
        loader.ignore_no_chiral_flag = self.ignore_no_chiral_flag;
        loader.treat_stereo_as = self.treat_stereo_as;
        loader.ignore_bad_valence = self.ignore_bad_valence;
        loader.loadReaction(_rxn);
        _loaded = true;
    }

    return _rxn;
}

BaseReaction& IndigoRdfReaction::getBaseReaction()
{
    return getReaction();
}

const char* IndigoRdfReaction::getName()
{
    if (_loaded)
        return _rxn.name.ptr();

    Indigo& self = indigoGetInstance();

    BufferScanner scanner(_data);
    auto& tmp = self.getThreadTmpData();
    scanner.readLine(tmp.string, true);
    if (strcmp(tmp.string.ptr(), "$RXN") != 0 && strcmp(tmp.string.ptr(), "$RXN V3000") != 0)
    {
        throw IndigoError("IndigoRdfReaction::getName(): unexpected first line in the files with reactions."
                          "'%s' has been found but '$RXN' or '$RXN V3000' has been expected.",
                          tmp.string.ptr());
    }
    // Read next line with the name
    scanner.readLine(tmp.string, true);
    return tmp.string.ptr();
}

IndigoObject* IndigoRdfReaction::clone()
{
    return IndigoReaction::cloneFrom(*this);
}

IndigoRdfReaction::~IndigoRdfReaction()
{
}

IndigoObject* IndigoSdfLoader::next()
{
    if (sdf_loader->isEOF())
        return 0;

    int counter = sdf_loader->currentNumber();
    long long offset = sdf_loader->tell();

    sdf_loader->readNext();

    return new IndigoRdfMolecule(sdf_loader->data, sdf_loader->properties, counter, offset);
}

IndigoObject* IndigoSdfLoader::at(int index)
{
    sdf_loader->readAt(index);

    return new IndigoRdfMolecule(sdf_loader->data, sdf_loader->properties, index, 0LL);
}

bool IndigoSdfLoader::hasNext()
{
    return !sdf_loader->isEOF();
}

long long IndigoSdfLoader::tell()
{
    return sdf_loader->tell();
}

IndigoRdfLoader::IndigoRdfLoader(Scanner& scanner) : IndigoObject(RDF_LOADER)
{
    rdf_loader = std::make_unique<RdfLoader>(scanner);
}

IndigoRdfLoader::IndigoRdfLoader(const char* filename) : IndigoObject(RDF_LOADER)
{
    _own_scanner = std::make_unique<FileScanner>(indigoGetInstance().filename_encoding, filename);
    rdf_loader = std::make_unique<RdfLoader>(*_own_scanner);
}

IndigoRdfLoader::~IndigoRdfLoader()
{
}

IndigoObject* IndigoRdfLoader::next()
{
    if (rdf_loader->isEOF())
        return 0;

    int counter = rdf_loader->currentNumber();
    long long offset = rdf_loader->tell();

    rdf_loader->readNext();

    if (rdf_loader->isMolecule())
        return new IndigoRdfMolecule(rdf_loader->data, rdf_loader->properties, counter, offset);
    else
        return new IndigoRdfReaction(rdf_loader->data, rdf_loader->properties, counter, offset);
}

IndigoObject* IndigoRdfLoader::at(int index)
{
    rdf_loader->readAt(index);

    if (rdf_loader->isMolecule())
        return new IndigoRdfMolecule(rdf_loader->data, rdf_loader->properties, index, 0LL);
    else
        return new IndigoRdfReaction(rdf_loader->data, rdf_loader->properties, index, 0LL);
}

long long IndigoRdfLoader::tell()
{
    return rdf_loader->tell();
}

bool IndigoRdfLoader::hasNext()
{
    return !rdf_loader->isEOF();
}

IndigoSmilesMolecule::IndigoSmilesMolecule(Array<char>& smiles, int index, long long offset) : IndigoRdfData(SMILES_MOLECULE, smiles, index, offset)
{
}

IndigoSmilesMolecule::~IndigoSmilesMolecule()
{
}

Molecule& IndigoSmilesMolecule::getMolecule()
{
    Indigo& self = indigoGetInstance();
    if (!_loaded)
    {
        BufferScanner scanner(_data);
        SmilesLoader loader(scanner);

        loader.stereochemistry_options = self.stereochemistry_options;
        loader.ignore_bad_valence = self.ignore_bad_valence;
        loader.ignore_no_chiral_flag = self.ignore_no_chiral_flag;

        loader.loadMolecule(_mol);
        _loaded = true;
    }
    return _mol;
}

BaseMolecule& IndigoSmilesMolecule::getBaseMolecule()
{
    return getMolecule();
}

const char* IndigoSmilesMolecule::getName()
{
    if (getMolecule().name.ptr() == 0)
        return "";
    return getMolecule().name.ptr();
}

IndigoObject* IndigoSmilesMolecule::clone()
{
    return IndigoMolecule::cloneFrom(*this);
}

IndigoSmilesReaction::IndigoSmilesReaction(Array<char>& smiles, int index, long long offset) : IndigoRdfData(SMILES_REACTION, smiles, index, offset)
{
}

IndigoSmilesReaction::~IndigoSmilesReaction()
{
}

Reaction& IndigoSmilesReaction::getReaction()
{
    Indigo& self = indigoGetInstance();
    if (!_loaded)
    {
        BufferScanner scanner(_data);
        RSmilesLoader loader(scanner);

        loader.stereochemistry_options = self.stereochemistry_options;
        loader.ignore_bad_valence = self.ignore_bad_valence;

        loader.loadReaction(_rxn);
        _loaded = true;
    }
    return _rxn;
}

BaseReaction& IndigoSmilesReaction::getBaseReaction()
{
    return getReaction();
}

const char* IndigoSmilesReaction::getName()
{
    if (getReaction().name.ptr() == 0)
        return "";
    return getReaction().name.ptr();
}

IndigoObject* IndigoSmilesReaction::clone()
{
    return IndigoReaction::cloneFrom(*this);
}

CP_DEF(IndigoMultilineSmilesLoader);

IndigoMultilineSmilesLoader::IndigoMultilineSmilesLoader(Scanner& scanner) : IndigoObject(MULTILINE_SMILES_LOADER), CP_INIT, TL_CP_GET(_offsets)
{
    _scanner = &scanner;

    _current_number = 0;
    _max_offset = 0LL;
    _offsets.clear();
}

IndigoMultilineSmilesLoader::IndigoMultilineSmilesLoader(const char* filename) : IndigoObject(MULTILINE_SMILES_LOADER), CP_INIT, TL_CP_GET(_offsets)
{
    _own_scanner = std::make_unique<FileScanner>(indigoGetInstance().filename_encoding, filename);
    _scanner = _own_scanner.get();

    _current_number = 0;
    _max_offset = 0LL;
    _offsets.clear();
}

IndigoMultilineSmilesLoader::~IndigoMultilineSmilesLoader()
{
}

void IndigoMultilineSmilesLoader::_advance()
{
    _offsets.expand(_current_number + 1);
    _offsets[_current_number++] = _scanner->tell();
    _scanner->readLine(_str, false);

    if (_scanner->tell() > _max_offset)
        _max_offset = _scanner->tell();
}

IndigoObject* IndigoMultilineSmilesLoader::next()
{
    if (_scanner->isEOF())
        return 0;

    long long offset = _scanner->tell();
    int counter = _current_number;

    _advance();

    if (_str.find('>') == -1)
        return new IndigoSmilesMolecule(_str, counter, offset);
    else
        return new IndigoSmilesReaction(_str, counter, offset);
}

bool IndigoMultilineSmilesLoader::hasNext()
{
    return !_scanner->isEOF();
}

long long IndigoMultilineSmilesLoader::tell()
{
    return _scanner->tell();
}

int IndigoMultilineSmilesLoader::count()
{
    long long offset = _scanner->tell();
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

IndigoObject* IndigoMultilineSmilesLoader::at(int index)
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

CEXPORT int indigoIterateSDF(int reader)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(reader);

        return self.addObject(new IndigoSdfLoader(IndigoScanner::get(obj)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateRDF(int reader)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(reader);

        return self.addObject(new IndigoRdfLoader(IndigoScanner::get(obj)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateSmiles(int reader)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(reader);

        return self.addObject(new IndigoMultilineSmilesLoader(IndigoScanner::get(obj)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoTell(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);
        long long size = 0LL;

        const int max_int = std::numeric_limits<int>::max();

        if (obj.type == IndigoObject::SDF_LOADER)
            size = ((IndigoSdfLoader&)obj).tell();
        else if (obj.type == IndigoObject::RDF_LOADER)
            size = ((IndigoRdfLoader&)obj).tell();
        else if (obj.type == IndigoObject::MULTILINE_SMILES_LOADER)
            size = ((IndigoMultilineSmilesLoader&)obj).tell();
        else if (obj.type == IndigoObject::RDF_MOLECULE || obj.type == IndigoObject::RDF_REACTION || obj.type == IndigoObject::SMILES_MOLECULE ||
                 obj.type == IndigoObject::SMILES_REACTION)
            size = ((IndigoRdfData&)obj).tell();
        else if (obj.type == IndigoObject::MULTIPLE_CML_LOADER)
            size = ((IndigoMultipleCmlLoader&)obj).tell();
        else if (obj.type == IndigoObject::CML_MOLECULE)
            size = ((IndigoCmlMolecule&)obj).tell();
        else if (obj.type == IndigoObject::CML_REACTION)
            size = ((IndigoCmlReaction&)obj).tell();
        else if (obj.type == IndigoObject::MULTIPLE_CDX_LOADER)
            size = ((IndigoMultipleCdxLoader&)obj).tell();
        else if (obj.type == IndigoObject::CDX_MOLECULE)
            size = ((IndigoCdxMolecule&)obj).tell();
        else if (obj.type == IndigoObject::CDX_REACTION)
            size = ((IndigoCdxReaction&)obj).tell();
        else
            throw IndigoError("indigoTell(): not applicable to %s", obj.debugInfo());

        if (size > max_int)
            throw IndigoError("indigoTell(): file size exceeds %d bytes. Please use indigoTell64() instead", max_int);
        else
            return static_cast<int>(size);
    }
    INDIGO_END(-1);
}

CEXPORT long long indigoTell64(int handle)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(handle);

        if (obj.type == IndigoObject::SDF_LOADER)
            return ((IndigoSdfLoader&)obj).tell();
        if (obj.type == IndigoObject::RDF_LOADER)
            return ((IndigoRdfLoader&)obj).tell();
        if (obj.type == IndigoObject::MULTILINE_SMILES_LOADER)
            return ((IndigoMultilineSmilesLoader&)obj).tell();
        if (obj.type == IndigoObject::RDF_MOLECULE || obj.type == IndigoObject::RDF_REACTION || obj.type == IndigoObject::SMILES_MOLECULE ||
            obj.type == IndigoObject::SMILES_REACTION)
            return ((IndigoRdfData&)obj).tell();
        if (obj.type == IndigoObject::MULTIPLE_CML_LOADER)
            return ((IndigoMultipleCmlLoader&)obj).tell();
        if (obj.type == IndigoObject::CML_MOLECULE)
            return ((IndigoCmlMolecule&)obj).tell();
        if (obj.type == IndigoObject::CML_REACTION)
            return ((IndigoCmlReaction&)obj).tell();
        if (obj.type == IndigoObject::MULTIPLE_CDX_LOADER)
            return ((IndigoMultipleCdxLoader&)obj).tell();
        if (obj.type == IndigoObject::CDX_MOLECULE)
            return ((IndigoCdxMolecule&)obj).tell();
        if (obj.type == IndigoObject::CDX_REACTION)
            return ((IndigoCdxReaction&)obj).tell();

        throw IndigoError("indigoTell64(): not applicable to %s", obj.debugInfo());
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateSDFile(const char* filename)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoSdfLoader(filename));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateRDFile(const char* filename)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoRdfLoader(filename));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateSmilesFile(const char* filename)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoMultilineSmilesLoader(filename));
    }
    INDIGO_END(-1);
}

IndigoCmlMolecule::IndigoCmlMolecule(Array<char>& data, int index, long long offset) : IndigoRdfData(CML_MOLECULE, data, index, offset)
{
}

IndigoCmlMolecule::~IndigoCmlMolecule()
{
}

Molecule& IndigoCmlMolecule::getMolecule()
{
    if (!_loaded)
    {
        Indigo& self = indigoGetInstance();

        BufferScanner scanner(_data);
        CmlLoader loader(scanner);
        loader.stereochemistry_options = self.stereochemistry_options;
        loader.ignore_bad_valence = self.ignore_bad_valence;
        loader.loadMolecule(_mol);
        _loaded = true;
    }
    return _mol;
}

BaseMolecule& IndigoCmlMolecule::getBaseMolecule()
{
    return getMolecule();
}

const char* IndigoCmlMolecule::getName()
{
    return getMolecule().name.ptr();
}

IndigoObject* IndigoCmlMolecule::clone()
{
    return IndigoMolecule::cloneFrom(*this);
}

const char* IndigoCmlMolecule::debugInfo() const
{
    return "<cml molecule>";
}

IndigoCmlReaction::IndigoCmlReaction(Array<char>& data, int index, long long offset) : IndigoRdfData(CML_REACTION, data, index, offset)
{
}

IndigoCmlReaction::~IndigoCmlReaction()
{
}

Reaction& IndigoCmlReaction::getReaction()
{
    if (!_loaded)
    {
        Indigo& self = indigoGetInstance();

        BufferScanner scanner(_data);
        ReactionCmlLoader loader(scanner);
        loader.stereochemistry_options = self.stereochemistry_options;
        loader.ignore_bad_valence = self.ignore_bad_valence;
        loader.loadReaction(_rxn);
        _loaded = true;
    }
    return _rxn;
}

BaseReaction& IndigoCmlReaction::getBaseReaction()
{
    return getReaction();
}

const char* IndigoCmlReaction::getName()
{
    return getReaction().name.ptr();
}

IndigoObject* IndigoCmlReaction::clone()
{
    return IndigoReaction::cloneFrom(*this);
}

const char* IndigoCmlReaction::debugInfo() const
{
    return "<cml reaction>";
}

IndigoMultipleCmlLoader::IndigoMultipleCmlLoader(Scanner& scanner) : IndigoObject(MULTIPLE_CML_LOADER)
{
    _own_scanner = 0;
    loader = std::make_unique<MultipleCmlLoader>(scanner);
}

IndigoMultipleCmlLoader::IndigoMultipleCmlLoader(const char* filename) : IndigoObject(MULTIPLE_CML_LOADER)
{
    _own_scanner = std::make_unique<FileScanner>(filename);
    loader = std::make_unique<MultipleCmlLoader>(*_own_scanner);
}

IndigoMultipleCmlLoader::~IndigoMultipleCmlLoader()
{
}

long long IndigoMultipleCmlLoader::tell()
{
    return loader->tell();
}

bool IndigoMultipleCmlLoader::hasNext()
{
    return !loader->isEOF();
}

IndigoObject* IndigoMultipleCmlLoader::next()
{
    if (!hasNext())
        return 0;

    int counter = loader->currentNumber();
    long long offset = loader->tell();

    loader->readNext();

    if (loader->isReaction())
        return new IndigoCmlReaction(loader->data, counter, offset);
    else
        return new IndigoCmlMolecule(loader->data, counter, offset);
}

CEXPORT int indigoIterateCML(int reader)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(reader);

        return self.addObject(new IndigoMultipleCmlLoader(IndigoScanner::get(obj)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateCMLFile(const char* filename)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoMultipleCmlLoader(filename));
    }
    INDIGO_END(-1);
}

IndigoCdxMolecule::IndigoCdxMolecule(Array<char>& data, PropertiesMap& properties, int index, long long offset)
    : IndigoRdfData(CDX_MOLECULE, data, properties, index, offset)
{
}

IndigoCdxMolecule::~IndigoCdxMolecule()
{
}

Molecule& IndigoCdxMolecule::getMolecule()
{
    if (!_loaded)
    {
        Indigo& self = indigoGetInstance();

        BufferScanner scanner(_data);
        MoleculeCdxmlLoader loader(scanner, true, true);
        loader.stereochemistry_options = self.stereochemistry_options;
        loader.ignore_bad_valence = self.ignore_bad_valence;
        loader.loadMolecule(_mol);
        _loaded = true;
    }
    return _mol;
}

BaseMolecule& IndigoCdxMolecule::getBaseMolecule()
{
    return getMolecule();
}

const char* IndigoCdxMolecule::getName()
{
    return getMolecule().name.ptr();
}

IndigoObject* IndigoCdxMolecule::clone()
{
    return IndigoMolecule::cloneFrom(*this);
}

const char* IndigoCdxMolecule::debugInfo() const
{
    return "<cdx molecule>";
}

IndigoCdxReaction::IndigoCdxReaction(Array<char>& data, PropertiesMap& properties, int index, long long offset)
    : IndigoRdfData(CDX_REACTION, data, properties, index, offset)
{
}

IndigoCdxReaction::~IndigoCdxReaction()
{
}

Reaction& IndigoCdxReaction::getReaction()
{
    if (!_loaded)
    {
        Indigo& self = indigoGetInstance();

        BufferScanner scanner(_data);
        ReactionCdxmlLoader loader(scanner, true);
        loader.stereochemistry_options = self.stereochemistry_options;
        loader.ignore_bad_valence = self.ignore_bad_valence;
        loader.loadReaction(_rxn);
        _loaded = true;
    }
    return _rxn;
}

BaseReaction& IndigoCdxReaction::getBaseReaction()
{
    return getReaction();
}

const char* IndigoCdxReaction::getName()
{
    return getReaction().name.ptr();
}

IndigoObject* IndigoCdxReaction::clone()
{
    return IndigoReaction::cloneFrom(*this);
}

const char* IndigoCdxReaction::debugInfo() const
{
    return "<cdx reaction>";
}

IndigoMultipleCdxLoader::IndigoMultipleCdxLoader(Scanner& scanner) : IndigoObject(MULTIPLE_CDX_LOADER)
{
    _own_scanner = 0;
    loader = std::make_unique<MultipleCdxLoader>(scanner);
}

IndigoMultipleCdxLoader::IndigoMultipleCdxLoader(const char* filename) : IndigoObject(MULTIPLE_CDX_LOADER)
{
    _own_scanner = std::make_unique<FileScanner>(filename);
    loader = std::make_unique<MultipleCdxLoader>(*_own_scanner);
}

IndigoMultipleCdxLoader::~IndigoMultipleCdxLoader()
{
}

long long IndigoMultipleCdxLoader::tell()
{
    return loader->tell();
}

bool IndigoMultipleCdxLoader::hasNext()
{
    return !loader->isEOF();
}

IndigoObject* IndigoMultipleCdxLoader::next()
{
    if (!hasNext())
        return 0;

    int counter = loader->currentNumber();
    long long offset = loader->tell();

    loader->readNext();

    if (loader->isReaction())
        return new IndigoCdxReaction(loader->data, loader->properties, counter, offset);
    else
        return new IndigoCdxMolecule(loader->data, loader->properties, counter, offset);
}

IndigoObject* IndigoMultipleCdxLoader::at(int index)
{
    loader->readAt(index);

    if (loader->isReaction())
        return new IndigoCdxReaction(loader->data, loader->properties, index, 0);
    else
        return new IndigoCdxMolecule(loader->data, loader->properties, index, 0);
}

CEXPORT int indigoIterateCDX(int reader)
{
    INDIGO_BEGIN
    {
        IndigoObject& obj = self.getObject(reader);

        return self.addObject(new IndigoMultipleCdxLoader(IndigoScanner::get(obj)));
    }
    INDIGO_END(-1);
}

CEXPORT int indigoIterateCDXFile(const char* filename)
{
    INDIGO_BEGIN
    {
        return self.addObject(new IndigoMultipleCdxLoader(filename));
    }
    INDIGO_END(-1);
}
