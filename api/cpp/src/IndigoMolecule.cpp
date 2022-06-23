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

#include "IndigoMolecule.h"

#include <indigo.h>

#include "IndigoSession.h"

using namespace indigo_cpp;

IndigoMolecule::IndigoMolecule(const int id, IndigoSessionPtr session) : IndigoBaseMolecule(id, std::move(session))
{
}

IndigoMolecule::IndigoMolecule(const IndigoMolecule& other) : IndigoBaseMolecule(other)
{
}

double IndigoMolecule::molecularWeight() const
{
    session()->setSessionId();
    return session()->_checkResultFloat(indigoMolecularWeight(id()));
}

double IndigoMolecule::tpsa(const bool includeSP) const
{
    session()->setSessionId();
    return session()->_checkResultFloat(indigoTPSA(id(), static_cast<int>(includeSP)));
}

int IndigoMolecule::numRotatableBonds() const
{
    session()->setSessionId();
    return session()->_checkResult(indigoNumRotatableBonds(id()));
}

int IndigoMolecule::numHydrogenBondAcceptors() const
{
    session()->setSessionId();
    return session()->_checkResult(indigoNumHydrogenBondAcceptors(id()));
}

int IndigoMolecule::numHydrogenBondDonors() const
{
    session()->setSessionId();
    return session()->_checkResult(indigoNumHydrogenBondDonors(id()));
}

double IndigoMolecule::logP() const
{
    session()->setSessionId();
    return session()->_checkResultFloat(indigoLogP(id()));
}

double IndigoMolecule::molarRefractivity() const
{
    session()->setSessionId();
    return session()->_checkResultFloat(indigoMolarRefractivity(id()));
}

int64_t IndigoMolecule::hash() const
{
    session()->setSessionId();
    return session()->_checkResult(indigoHash(id()));
}
