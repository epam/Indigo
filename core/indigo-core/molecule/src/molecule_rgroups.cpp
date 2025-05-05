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
#include "base_cpp/output.h"
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

void RGroup::readOccurrence(const char* str)
{
    int beg = -1, end = -1;
    int add_beg = 0, add_end = 0;

    while (*str != 0)
    {
        if (*str == '>')
        {
            end = UINT16_MAX;
            add_beg = 1;
        }
        else if (*str == '<')
        {
            beg = 0;
            add_end = -1;
        }
        else if (isdigit(*str))
        {
            sscanf(str, "%d", beg == -1 ? &beg : &end);
            while (isdigit(*str))
                str++;
            continue;
        }
        else if (*str == ',')
        {
            if (end == -1)
                end = beg;
            else
                beg += add_beg, end += add_end;
            pushRange(beg, end);
            beg = end = -1;
            add_beg = add_end = 0;
        }
        str++;
    }

    if (beg == -1 && end == -1)
        return;

    if (end == -1)
        end = beg;
    else
        beg += add_beg, end += add_end;
    pushRange(beg, end);
}

void RGroup::writeOccurrence(Output& output)
{
    for (int i = 0; i < occurrence.size(); i++)
    {
        int end = occurrence[i];
        int begin = end >> std::numeric_limits<uint16_t>::digits;
        end = end & UINT16_MAX;

        if (end == UINT16_MAX)
            output.printf(">%d", begin - 1);
        else if (begin == end)
            output.printf("%d", begin);
        else if (begin == 0)
            output.printf("<%d", end + 1);
        else
            output.printf("%d-%d", begin, end);

        if (i != occurrence.size() - 1)
            output.printf(",");
    }
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
