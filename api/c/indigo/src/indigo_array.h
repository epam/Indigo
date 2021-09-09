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

#ifndef __indigo_array__
#define __indigo_array__

#include "indigo_internal.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

class DLLEXPORT IndigoArray : public IndigoObject
{
public:
    IndigoArray();

    ~IndigoArray() override;

    IndigoObject* clone() override;

    static bool is(IndigoObject& obj);
    static IndigoArray& cast(IndigoObject& obj);

    PtrArray<IndigoObject> objects;
};

class DLLEXPORT IndigoArrayElement : public IndigoObject
{
public:
    IndigoArrayElement(IndigoArray& arr, int idx_);
    ~IndigoArrayElement() override;

    IndigoObject& get();

    BaseMolecule& getBaseMolecule() override;
    Molecule& getMolecule() override;
    QueryMolecule& getQueryMolecule() override;

    MonomersProperties& getMonomersProperties() override;
    BaseReaction& getBaseReaction() override;
    Reaction& getReaction() override;

    IndigoObject* clone() override;

    const char* getName() override;

    int getIndex() override;

    IndigoArray* array;
    int idx;
};

class IndigoArrayIter : public IndigoObject
{
public:
    IndigoArrayIter(IndigoArray& arr);
    ~IndigoArrayIter() override;

    IndigoObject* next() override;
    bool hasNext() override;

protected:
    IndigoArray* _arr;
    int _idx;
};

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
