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

#ifndef __indigo_scaffold__
#define __indigo_scaffold__

#include "indigo_internal.h"
#include "molecule/query_molecule.h"

class IndigoScaffold : public IndigoObject
{
public:
    IndigoScaffold();
    virtual ~IndigoScaffold();

    void extractScaffold();

    virtual QueryMolecule& getQueryMolecule();
    virtual BaseMolecule& getBaseMolecule();

    QueryMolecule max_scaffold;
    ObjArray<QueryMolecule> all_scaffolds;
};

#endif