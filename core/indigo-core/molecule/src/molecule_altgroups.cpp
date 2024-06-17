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

#include "molecule/molecule_altgroups.h"
#include "molecule/query_molecule.h"
#include <memory>

using namespace indigo;

AltGroup::AltGroup()
{
}

AltGroup::~AltGroup()
{
}

void AltGroup::clear()
{
    occurrence.clear();
}

void AltGroup::copy(AltGroup& other)
{

}

bool AltGroup::occurrenceSatisfied(int value)
{
    if (occurrence.size() == 0)
        return true;

    for (int i = 0; i < occurrence.size(); i++)
        if (value >= (occurrence[i] >> 16) && value <= (occurrence[i] & 0xFFFF))
            return true;
    return false;
}

IMPL_ERROR(MoleculeAltGroups, "molecule AltGroups");

MoleculeAltGroups::MoleculeAltGroups()
{
}

MoleculeAltGroups::~MoleculeAltGroups()
{
}

void MoleculeAltGroups::copyAltGroupsFromMolecule(MoleculeAltGroups& other)
{
    /*int n_AltGroups = other.getAltGroupCount();

    for (int i = 1; i <= n_AltGroups; i++)
    {
        AltGroup& altgroup = other.getAltGroup(i);

        if (altgroup.fragments.size() > 0)
            getAltGroup(i).copy(altgroup);
    }*/
}

void MoleculeAltGroups::clear()
{
    _altgroups.clear();
}

AltGroup& MoleculeAltGroups::getAltGroup(int idx)
{
    if (_altgroups.size() < idx)
        _altgroups.resize(idx);

    return _altgroups[idx - 1];
}

int MoleculeAltGroups::getAltGroupCount() const
{
    return _altgroups.size();
}
