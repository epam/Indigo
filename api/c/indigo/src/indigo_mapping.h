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

#ifndef __indigo_mapping__
#define __indigo_mapping__

#include "indigo_internal.h"

class IndigoMapping : public IndigoObject
{
public:
    IndigoMapping(BaseMolecule& from, BaseMolecule& to);
    ~IndigoMapping() override;
    static IndigoMapping& cast(IndigoObject& obj);

    BaseMolecule& from;
    BaseMolecule& to;
    Array<int> mapping;

    IndigoObject* clone() override;

protected:
};

class IndigoReactionMapping : public IndigoObject
{
public:
    IndigoReactionMapping(BaseReaction& from, BaseReaction& to);
    ~IndigoReactionMapping() override;

    BaseReaction& from;
    BaseReaction& to;

    Array<int> mol_mapping;
    ObjArray<Array<int>> mappings;

    IndigoObject* clone() override;
};

#endif
