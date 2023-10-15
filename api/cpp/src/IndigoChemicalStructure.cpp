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

#include "IndigoChemicalStructure.h"
#include "IndigoSession.h"

#include <indigo-inchi.h>

using namespace indigo_cpp;

IndigoChemicalStructure::IndigoChemicalStructure(const int id, IndigoSessionPtr session) : IndigoCopyableObject(id, std::move(session))
{
}

IndigoChemicalStructure::IndigoChemicalStructure(const IndigoChemicalStructure& other) : IndigoCopyableObject(other)
{
}

void IndigoChemicalStructure::aromatize()
{
    session()->setSessionId();
    session()->_checkResult(indigoAromatize(id()));
}

void IndigoChemicalStructure::dearomatize()
{
    session()->setSessionId();
    session()->_checkResult(indigoDearomatize(id()));
}

void IndigoChemicalStructure::layout()
{
    session()->setSessionId();
    session()->_checkResult(indigoLayout(id()));
}

void IndigoChemicalStructure::clean2d()
{
    session()->setSessionId();
    session()->_checkResult(indigoClean2d(id()));
}

std::string IndigoChemicalStructure::smiles() const
{
    session()->setSessionId();
    return session()->_checkResultString(indigoSmiles(id()));
}

std::string IndigoChemicalStructure::smarts() const
{
    session()->setSessionId();
    return session()->_checkResultString(indigoSmarts(id()));
}

std::string IndigoChemicalStructure::canonicalSmiles() const
{
    session()->setSessionId();
    return session()->_checkResultString(indigoCanonicalSmiles(id()));
}

std::string IndigoChemicalStructure::cml() const
{
    session()->setSessionId();
    return session()->_checkResultString(indigoCml(id()));
}

std::string IndigoChemicalStructure::inchi() const
{
    session()->setSessionId();
    return session()->_checkResultString(indigoInchiGetInchi(id()));
}

std::string IndigoChemicalStructure::rawData() const
{
    session()->setSessionId();
    return session()->_checkResultString(indigoRawData(id()));
}

std::string IndigoChemicalStructure::name() const
{
    session()->setSessionId();
    return session()->_checkResultString(indigoName(id()));
}
