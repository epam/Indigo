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

#include "base_cpp/properties_map.h"
#include "molecule/molecule.h"
#include "reaction/reaction.h"

class IndigoRdfData : public IndigoObject
{
public:
    IndigoRdfData(int type, Array<char>& data, int index, long long offset);
    IndigoRdfData(int type, Array<char>& data, PropertiesMap& properties, int index, long long offset);
    virtual ~IndigoRdfData();

    Array<char>& getRawData();
    //   virtual RedBlackStringObjMap< Array<char> > * getProperties () {return &_properties.getProperties();}
    virtual PropertiesMap& getProperties()
    {
        return _properties;
    }

    virtual int getIndex();
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
    virtual ~IndigoRdfMolecule();

    virtual Molecule& getMolecule();
    virtual BaseMolecule& getBaseMolecule();
    virtual const char* getName();
    virtual IndigoObject* clone();

protected:
    Molecule _mol;
};

class IndigoRdfReaction : public IndigoRdfData
{
public:
    IndigoRdfReaction(Array<char>& data, PropertiesMap& properties, int index, long long offset);
    virtual ~IndigoRdfReaction();

    virtual Reaction& getReaction();
    virtual BaseReaction& getBaseReaction();
    virtual const char* getName();
    virtual IndigoObject* clone();

protected:
    Reaction _rxn;
};

class IndigoSdfLoader : public IndigoObject
{
public:
    IndigoSdfLoader(Scanner& scanner);
    IndigoSdfLoader(const char* filename);
    virtual ~IndigoSdfLoader();

    virtual IndigoObject* next();
    virtual bool hasNext();

    IndigoObject* at(int index);

    long long tell();

    AutoPtr<SdfLoader> sdf_loader;

protected:
    AutoPtr<Scanner> _own_scanner;
};

class IndigoRdfLoader : public IndigoObject
{
public:
    IndigoRdfLoader(Scanner& scanner);
    IndigoRdfLoader(const char* filename);
    virtual ~IndigoRdfLoader();

    virtual IndigoObject* next();
    virtual bool hasNext();

    IndigoObject* at(int index);

    long long tell();

    AutoPtr<RdfLoader> rdf_loader;

protected:
    AutoPtr<Scanner> _own_scanner;
};

class IndigoSmilesMolecule : public IndigoRdfData
{
public:
    IndigoSmilesMolecule(Array<char>& smiles, int index, long long offset);
    virtual ~IndigoSmilesMolecule();

    virtual Molecule& getMolecule();
    virtual BaseMolecule& getBaseMolecule();
    virtual const char* getName();
    virtual IndigoObject* clone();

protected:
    Molecule _mol;
};

class IndigoSmilesReaction : public IndigoRdfData
{
public:
    IndigoSmilesReaction(Array<char>& data, int index, long long offset);
    virtual ~IndigoSmilesReaction();

    virtual Reaction& getReaction();
    virtual BaseReaction& getBaseReaction();
    virtual const char* getName();
    virtual IndigoObject* clone();

protected:
    Reaction _rxn;
};

class IndigoMultilineSmilesLoader : public IndigoObject
{
public:
    IndigoMultilineSmilesLoader(Scanner& scanner);
    IndigoMultilineSmilesLoader(const char* filename);
    virtual ~IndigoMultilineSmilesLoader();

    long long tell();

    virtual IndigoObject* next();
    virtual bool hasNext();

    IndigoObject* at(int index);
    int count();

protected:
    Scanner* _scanner;
    Array<char> _str;
    AutoPtr<Scanner> _own_scanner;

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
    virtual ~IndigoCmlMolecule();

    virtual Molecule& getMolecule();
    virtual BaseMolecule& getBaseMolecule();
    virtual const char* getName();
    virtual IndigoObject* clone();

    virtual const char* debugInfo();

protected:
    Molecule _mol;
};

class IndigoCmlReaction : public IndigoRdfData
{
public:
    IndigoCmlReaction(Array<char>& data_, int index, long long offset);
    virtual ~IndigoCmlReaction();

    virtual Reaction& getReaction();
    virtual BaseReaction& getBaseReaction();
    virtual const char* getName();
    virtual IndigoObject* clone();

    virtual const char* debugInfo();

protected:
    Reaction _rxn;
};

class IndigoMultipleCmlLoader : public IndigoObject
{
public:
    IndigoMultipleCmlLoader(Scanner& scanner);
    IndigoMultipleCmlLoader(const char* filename);
    virtual ~IndigoMultipleCmlLoader();

    virtual IndigoObject* next();
    virtual bool hasNext();

    IndigoObject* at(int index);

    long long tell();

    AutoPtr<MultipleCmlLoader> loader;

protected:
    AutoPtr<Scanner> _own_scanner;
};

namespace indigo
{
    class MultipleCdxLoader;
}

class IndigoCdxMolecule : public IndigoRdfData
{
public:
    IndigoCdxMolecule(Array<char>& data_, PropertiesMap& properties, int index, long long offset);
    virtual ~IndigoCdxMolecule();

    virtual Molecule& getMolecule();
    virtual BaseMolecule& getBaseMolecule();
    virtual const char* getName();
    virtual IndigoObject* clone();

    virtual const char* debugInfo();

protected:
    Molecule _mol;
};

class IndigoCdxReaction : public IndigoRdfData
{
public:
    IndigoCdxReaction(Array<char>& data_, PropertiesMap& properties, int index, long long offset);
    virtual ~IndigoCdxReaction();

    virtual Reaction& getReaction();
    virtual BaseReaction& getBaseReaction();
    virtual const char* getName();
    virtual IndigoObject* clone();

    virtual const char* debugInfo();

protected:
    Reaction _rxn;
};

class IndigoMultipleCdxLoader : public IndigoObject
{
public:
    IndigoMultipleCdxLoader(Scanner& scanner);
    IndigoMultipleCdxLoader(const char* filename);
    virtual ~IndigoMultipleCdxLoader();

    virtual IndigoObject* next();
    virtual bool hasNext();

    IndigoObject* at(int index);

    long long tell();

    AutoPtr<MultipleCdxLoader> loader;

protected:
    AutoPtr<Scanner> _own_scanner;
};

#endif
