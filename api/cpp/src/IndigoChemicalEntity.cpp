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
#include "IndigoSession.h"

#include <indigo-inchi.h>

using namespace indigo_cpp;

IndigoChemicalEntity::IndigoChemicalEntity(const int id, IndigoSessionPtr session) : IndigoObject(id, std::move(session))
{
}

void IndigoChemicalEntity::aromatize()
{
    session->setSessionId();
    session->_checkResult(indigoAromatize(id));
}

void IndigoChemicalEntity::dearomatize()
{
    session->setSessionId();
    session->_checkResult(indigoDearomatize(id));
}

void IndigoChemicalEntity::layout()
{
    session->setSessionId();
    session->_checkResult(indigoLayout(id));
}

void IndigoChemicalEntity::clean2d()
{
    session->setSessionId();
    session->_checkResult(indigoClean2d(id));
}

std::string IndigoChemicalEntity::smiles() const
{
    session->setSessionId();
    return session->_checkResultString(indigoSmiles(id));
}

std::string IndigoChemicalEntity::cml() const
{
    session->setSessionId();
    return session->_checkResultString(indigoCml(id));
}

std::string IndigoChemicalEntity::inchi() const
{
    session->setSessionId();
    return session->_checkResultString(indigoInchiGetInchi(id));
}
