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

#ifndef __indigo_tautomer_enumerator__
#define __indigo_tautomer_enumerator__

#include "base_cpp/properties_map.h"
#include "indigo_internal.h"
#include "molecule/molecule_tautomer_enumerator.h"

class IndigoMoleculeTautomer : public IndigoObject
{
public:
    IndigoMoleculeTautomer(TautomerEnumerator& enumerator, int index);
    virtual ~IndigoMoleculeTautomer();

    virtual int getIndex();

    virtual Molecule& getMolecule();
    virtual IndigoObject* clone();

    virtual const char* debugInfo();

    virtual PropertiesMap& getProperties()
    {
        return _properties;
    }

private:
    Molecule _molInstance;
    int _index;
    indigo::PropertiesMap _properties;
};

class IndigoTautomerIter : public IndigoObject
{
public:
    IndigoTautomerIter(Molecule& molecule, TautomerMethod method);
    virtual ~IndigoTautomerIter();

    virtual int getIndex();

    virtual IndigoObject* next();
    virtual bool hasNext();

    virtual const char* debugInfo();

protected:
    TautomerEnumerator _enumerator;
    int _currentPosition;
    bool _complete;
};

#endif /* __indigo_tautomer_enumerator__ */
