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

#include "IndigoChemicalEntity.h"
#include <IndigoSession.h>

#include <indigo-inchi.h>

using namespace indigo_cpp;

IndigoChemicalEntity::IndigoChemicalEntity(const int id, const IndigoSession& indigo) : IndigoObject(id, indigo)
{
}

void IndigoChemicalEntity::aromatize() const
{
    indigo.setSessionId();
    indigo._checkResult(indigoAromatize(id));
}

void IndigoChemicalEntity::dearomatize() const
{
    indigo.setSessionId();
    indigo._checkResult(indigoDearomatize(id));
}

void IndigoChemicalEntity::layout() const
{
    indigo.setSessionId();
    indigo._checkResult(indigoLayout(id));
}

void IndigoChemicalEntity::clean2d() const
{
    indigo.setSessionId();
    indigo._checkResult(indigoClean2d(id));
}

std::string IndigoChemicalEntity::smiles() const
{
    indigo.setSessionId();
    return indigo._checkResultString(indigoSmiles(id));
}

std::string IndigoChemicalEntity::cml() const
{
    indigo.setSessionId();
    return indigo._checkResultString(indigoCml(id));
}

std::string IndigoChemicalEntity::inchi() const
{
    indigo.setSessionId();
    return indigo._checkResultString(indigoInchiGetInchi(id));
}

//IndigoChemicalEntity::IndigoChemicalEntity(const IndigoChemicalEntity& other) : IndigoObject(-1, other.indigo)
//{
//    indigo.setSessionId();
//    const_cast<int&>(id) = -1;
//}

//IndigoChemicalEntity& IndigoChemicalEntity::operator=(const IndigoChemicalEntity& other)
//{
//    return <#initializer #>;
//}
//
//IndigoChemicalEntity IndigoChemicalEntity::clone() const
//{
//    indigo.setSessionId();
//    return {indigo._checkResultString(indigoClone(id)), indigo};
//}
