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

#ifndef __indigo_loaders__
#define __indigo_loaders__

#include "indigo_internal.h"

#include <rapidjson/document.h>

#include "base_cpp/properties_map.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_loader.h"
#include "molecule/query_molecule.h"
#include "reaction/reaction.h"

class IndigoRdfData : public IndigoObject
{
public:
    IndigoRdfData(int type, Array<char>& data, int index, long long offset);
    IndigoRdfData(int type, Array<char>& data, PropertiesMap& properties, int index, long long offset);
    ~IndigoRdfData() override;

    Array<char>& getRawData();
    //   RedBlackStringObjMap< Array<char> > * getProperties () override {return &_properties.getProperties();}
    PropertiesMap& getProperties() override
    {
        return _properties;
    }

    int getIndex() override;
    long long tell();

protected:
    Array<char> _data;

    PropertiesMap _properties;
    bool _loaded;
    int _index;
    long long _offset;
};

class IndigoRdfMolecule : public IndigoRdfData
{
public:
    IndigoRdfMolecule(Array<char>& data, PropertiesMap& properties, int index, long long offset);
    ~IndigoRdfMolecule() override;

    Molecule& getMolecule() override;
    BaseMolecule& getBaseMolecule() override;
    const char* getName() override;
    IndigoObject* clone() override;

protected:
    Molecule _mol;
};

class IndigoRdfReaction : public IndigoRdfData
{
public:
    IndigoRdfReaction(Array<char>& data, PropertiesMap& properties, int index, long long offset);
    ~IndigoRdfReaction() override;

    Reaction& getReaction() override;
    BaseReaction& getBaseReaction() override;
    const char* getName() override;
    IndigoObject* clone() override;

protected:
    Reaction _rxn;
};

class IndigoSdfLoader : public IndigoObject
{
public:
    IndigoSdfLoader(Scanner& scanner);
    IndigoSdfLoader(const char* filename);
    ~IndigoSdfLoader() override;
    IndigoObject* next() override;
    bool hasNext() override;
    IndigoObject* at(int index);
    long long tell();
    std::unique_ptr<SdfLoader> sdf_loader;

protected:
    std::unique_ptr<Scanner> _own_scanner;
};

/*
class IndigoJSONLoader : public IndigoObject
{
public:
    IndigoJSONLoader(Scanner& scanner);
    IndigoJSONLoader(const char* filename);
    ~IndigoJSONLoader() override;
    IndigoObject* next() override;
    bool hasNext() override;
    IndigoObject* at(int index);
    std::unique_ptr<JSONLoader> json_loader;

protected:
    std::unique_ptr<Scanner> _own_scanner;
};*/

class IndigoRdfLoader : public IndigoObject
{
public:
    IndigoRdfLoader(Scanner& scanner);
    IndigoRdfLoader(const char* filename);
    ~IndigoRdfLoader() override;

    IndigoObject* next() override;
    bool hasNext() override;

    IndigoObject* at(int index);

    long long tell();

    std::unique_ptr<RdfLoader> rdf_loader;

protected:
    std::unique_ptr<Scanner> _own_scanner;
};

class IndigoJSONMolecule : public IndigoObject
{
public:
    IndigoJSONMolecule(rapidjson::Document& ket);
    ~IndigoJSONMolecule() override;
    Molecule& getMolecule() override;
    BaseMolecule& getBaseMolecule() override;
    const char* getName() override;
    IndigoObject* clone() override;

protected:
    Molecule _mol;
    MoleculeJsonLoader _loader;
    bool _loaded;
};

class IndigoSmilesMolecule : public IndigoRdfData
{
public:
    IndigoSmilesMolecule(Array<char>& smiles, int index, long long offset);
    ~IndigoSmilesMolecule() override;

    Molecule& getMolecule() override;
    BaseMolecule& getBaseMolecule() override;
    const char* getName() override;
    IndigoObject* clone() override;

protected:
    Molecule _mol;
};

class IndigoSmilesReaction : public IndigoRdfData
{
public:
    IndigoSmilesReaction(Array<char>& data, int index, long long offset);
    ~IndigoSmilesReaction() override;

    Reaction& getReaction() override;
    BaseReaction& getBaseReaction() override;
    const char* getName() override;
    IndigoObject* clone() override;

protected:
    Reaction _rxn;
};

class IndigoMultilineSmilesLoader : public IndigoObject
{
public:
    IndigoMultilineSmilesLoader(Scanner& scanner);
    IndigoMultilineSmilesLoader(const char* filename);
    ~IndigoMultilineSmilesLoader() override;

    long long tell();

    IndigoObject* next() override;
    bool hasNext() override;

    IndigoObject* at(int index);
    int count();

protected:
    Scanner* _scanner;
    Array<char> _str;
    std::unique_ptr<Scanner> _own_scanner;

    void _advance();

    CP_DECL;
    TL_CP_DECL(Array<long long>, _offsets);
    int _current_number;
    long long _max_offset;
};

namespace indigo
{
    class MultipleCmlLoader;
}

class IndigoCmlMolecule : public IndigoRdfData
{
public:
    IndigoCmlMolecule(Array<char>& data_, int index, long long offset);
    ~IndigoCmlMolecule() override;

    Molecule& getMolecule() override;
    BaseMolecule& getBaseMolecule() override;
    const char* getName() override;
    IndigoObject* clone() override;

    const char* debugInfo() const override;

protected:
    Molecule _mol;
};

class IndigoCmlReaction : public IndigoRdfData
{
public:
    IndigoCmlReaction(Array<char>& data_, int index, long long offset);
    ~IndigoCmlReaction() override;

    Reaction& getReaction() override;
    BaseReaction& getBaseReaction() override;
    const char* getName() override;
    IndigoObject* clone() override;

    const char* debugInfo() const override;

protected:
    Reaction _rxn;
};

class IndigoMultipleCmlLoader : public IndigoObject
{
public:
    IndigoMultipleCmlLoader(Scanner& scanner);
    IndigoMultipleCmlLoader(const char* filename);
    ~IndigoMultipleCmlLoader() override;

    IndigoObject* next() override;
    bool hasNext() override;

    IndigoObject* at(int index);

    long long tell();

    std::unique_ptr<MultipleCmlLoader> loader;

protected:
    std::unique_ptr<Scanner> _own_scanner;
};

namespace indigo
{
    class MultipleCdxLoader;
}

class IndigoCdxMolecule : public IndigoRdfData
{
public:
    IndigoCdxMolecule(Array<char>& data_, PropertiesMap& properties, int index, long long offset);
    ~IndigoCdxMolecule() override;

    Molecule& getMolecule() override;
    BaseMolecule& getBaseMolecule() override;
    const char* getName() override;
    IndigoObject* clone() override;

    const char* debugInfo() const override;

protected:
    Molecule _mol;
};

class IndigoCdxReaction : public IndigoRdfData
{
public:
    IndigoCdxReaction(Array<char>& data_, PropertiesMap& properties, int index, long long offset);
    ~IndigoCdxReaction() override;

    Reaction& getReaction() override;
    BaseReaction& getBaseReaction() override;
    const char* getName() override;
    IndigoObject* clone() override;

    const char* debugInfo() const override;

protected:
    Reaction _rxn;
};

class IndigoMultipleCdxLoader : public IndigoObject
{
public:
    IndigoMultipleCdxLoader(Scanner& scanner);
    IndigoMultipleCdxLoader(const char* filename);
    ~IndigoMultipleCdxLoader() override;

    IndigoObject* next() override;
    bool hasNext() override;

    IndigoObject* at(int index);

    long long tell();

    std::unique_ptr<MultipleCdxLoader> loader;

protected:
    std::unique_ptr<Scanner> _own_scanner;
};

#endif
