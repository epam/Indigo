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

#include "molecule/molecule_rgroups.h"
#include "molecule/query_molecule.h"
#include <memory>

using namespace indigo;

RGroup::RGroup() : if_then(0), rest_h(0)
{
}

RGroup::~RGroup()
{
}

void RGroup::clear()
{
    if_then = 0;
    rest_h = 0;
    occurrence.clear();
    fragments.clear();
}

void RGroup::copy(RGroup& other)
{
    if_then = other.if_then;
    rest_h = other.rest_h;
    occurrence.copy(other.occurrence);
    fragments.clear();

    PtrPool<BaseMolecule>& frags = other.fragments;
    for (int i = frags.begin(); i != frags.end(); i = frags.next(i))
    {
        std::unique_ptr<BaseMolecule> new_fragment(frags[i]->neu());

        new_fragment->clone(*frags[i], 0, 0);
        fragments.add(new_fragment.release());
    }
}

bool RGroup::occurrenceSatisfied(int value)
{
    if (occurrence.size() == 0)
        return true;

    for (int i = 0; i < occurrence.size(); i++)
        if (value >= (occurrence[i] >> 16) && value <= (occurrence[i] & 0xFFFF))
            return true;
    return false;
}

IMPL_ERROR(MoleculeRGroups, "molecule rgroups");

MoleculeRGroups::MoleculeRGroups()
{
}

MoleculeRGroups::~MoleculeRGroups()
{
}

void MoleculeRGroups::copyRGroupsFromMolecule(MoleculeRGroups& other)
{
    int n_rgroups = other.getRGroupCount();

    for (int i = 1; i <= n_rgroups; i++)
    {
        RGroup& rgroup = other.getRGroup(i);

        if (rgroup.fragments.size() > 0)
            getRGroup(i).copy(rgroup);
    }
}

void MoleculeRGroups::clear()
{
    _rgroups.clear();
}

RGroup& MoleculeRGroups::getRGroup(int idx)
{
    if (_rgroups.size() < idx)
        _rgroups.resize(idx);

    return _rgroups[idx - 1];
}

int MoleculeRGroups::getRGroupCount() const
{
    return _rgroups.size();
}
