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

#include "molecule/molecule_tgroups.h"
#include "base_cpp/scanner.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"

using namespace indigo;

TGroup::TGroup() : unresolved(false)
{
}

TGroup::~TGroup()
{
}

void TGroup::clear()
{
    unresolved = false;
}

int TGroup::cmp(TGroup& tg1, TGroup& tg2, void* /*context*/)
{
    QS_DEF(Array<int>, lgrps)
    QS_DEF(Array<int>, bgrps)

    if (tg1.fragment.get() == 0)
        return -1;
    if (tg2.fragment.get() == 0)
        return 1;

    if (tg1.unresolved && !tg2.unresolved)
        return 1;
    else if (!tg1.unresolved && tg2.unresolved)
        return -1;

    lgrps.clear();
    bgrps.clear();

    tg1.fragment->sgroups.findSGroups(SGroup::SG_CLASS, "LGRP", lgrps);
    int lgrps1_count = lgrps.size();
    for (auto i = tg1.fragment->sgroups.begin(); i != tg1.fragment->sgroups.end(); i = tg1.fragment->sgroups.next(i))
    {
        if (lgrps.find(i) == -1)
            bgrps.push(i);
    }

    int non_hyd_count1 = 0;
    for (auto i = 0; i < bgrps.size(); i++)
    {
        SGroup& sg = tg1.fragment->sgroups.getSGroup(bgrps[i]);
        for (auto j = 0; j < sg.atoms.size(); j++)
            if (tg1.fragment->getAtomNumber(sg.atoms[j]) != ELEM_H)
                non_hyd_count1++;
    }

    lgrps.clear();
    bgrps.clear();

    tg2.fragment->sgroups.findSGroups(SGroup::SG_CLASS, "LGRP", lgrps);
    int lgrps2_count = lgrps.size();
    for (auto i = tg2.fragment->sgroups.begin(); i != tg2.fragment->sgroups.end(); i = tg2.fragment->sgroups.next(i))
    {
        if (lgrps.find(i) == -1)
            bgrps.push(i);
    }

    int non_hyd_count2 = 0;
    for (auto i = 0; i < bgrps.size(); i++)
    {
        SGroup& sg = tg2.fragment->sgroups.getSGroup(bgrps[i]);
        for (auto j = 0; j < sg.atoms.size(); j++)
            if (tg2.fragment->getAtomNumber(sg.atoms[j]) != ELEM_H)
                non_hyd_count2++;
    }

    if ((non_hyd_count2 - non_hyd_count1) != 0)
        return non_hyd_count2 - non_hyd_count1;

    if ((lgrps2_count - lgrps1_count) != 0)
        return lgrps2_count - lgrps1_count;

    if ((tg1.tgroup_alias.size() - tg2.tgroup_alias.size()) != 0)
        return tg1.tgroup_alias.size() - tg2.tgroup_alias.size();

    if ((tg2.tgroup_class.size() > 1) && strncmp(tg2.tgroup_class.ptr(), "AA", 2) == 0)
        return 1;
    else
        return -1;
}

void TGroup::copy(const TGroup& other)
{
    tgroup_class.copy(other.tgroup_class);
    tgroup_name.copy(other.tgroup_name);
    tgroup_full_name.copy(other.tgroup_full_name);
    tgroup_alias.copy(other.tgroup_alias);
    tgroup_text_id.copy(other.tgroup_text_id);
    tgroup_comment.copy(other.tgroup_comment);
    tgroup_natreplace.copy(other.tgroup_natreplace);
    tgroup_id = other.tgroup_id;
    unresolved = other.unresolved;
    idt_alias.copy(other.idt_alias);
    fragment.reset(other.fragment->neu());
    fragment->clone(*other.fragment.get(), 0, 0);
}

IMPL_ERROR(MoleculeTGroups, "molecule tgroups");

MoleculeTGroups::MoleculeTGroups()
{
    _tgroups.clear();
}

MoleculeTGroups::~MoleculeTGroups()
{
    _tgroups.clear();
}

void MoleculeTGroups::clear()
{
    _tgroups.clear();
}

int MoleculeTGroups::begin()
{
    return _tgroups.begin();
}

int MoleculeTGroups::end()
{
    return _tgroups.end();
}

int MoleculeTGroups::next(int i)
{
    return _tgroups.next(i);
}

void MoleculeTGroups::remove(int i)
{
    return _tgroups.remove(i);
}

int MoleculeTGroups::addTGroup()
{
    return _tgroups.add(new TGroup());
}

TGroup& MoleculeTGroups::getTGroup(int idx)
{
    return *_tgroups.at(idx);
}

void MoleculeTGroups::copyTGroupsFromMolecule(MoleculeTGroups& other)
{
    for (int i = other.begin(); i != other.end(); i = other.next(i))
    {
        TGroup& tgroup = other.getTGroup(i);
        int idx = addTGroup();
        getTGroup(idx).copy(tgroup);
    }
}

int MoleculeTGroups::getTGroupCount()
{
    return _tgroups.size();
}

int MoleculeTGroups::findTGroup(const char* name)
{
    for (int i = _tgroups.begin(); i != _tgroups.end(); i = _tgroups.next(i))
    {
        TGroup& tgroup = *_tgroups.at(i);
        if (tgroup.tgroup_name.size() > 0 && name != 0)
        {
            if (strncmp(tgroup.tgroup_name.ptr(), name, tgroup.tgroup_name.size()) == 0)
                return i;
        }
    }
    return -1;
}
