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

#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"

#include "base_cpp/tree.h"
#include "molecule/molecule_sgroups.h"

using namespace indigo;

static SGroup::SgType mappingForSgTypes[] = {
    {SGroup::SG_TYPE_GEN, "GEN"}, {SGroup::SG_TYPE_DAT, "DAT"}, {SGroup::SG_TYPE_SUP, "SUP"}, {SGroup::SG_TYPE_SRU, "SRU"}, {SGroup::SG_TYPE_MUL, "MUL"},
    {SGroup::SG_TYPE_MON, "MON"}, {SGroup::SG_TYPE_MER, "MER"}, {SGroup::SG_TYPE_COP, "COP"}, {SGroup::SG_TYPE_CRO, "CRO"}, {SGroup::SG_TYPE_MOD, "MOD"},
    {SGroup::SG_TYPE_GRA, "GRA"}, {SGroup::SG_TYPE_COM, "COM"}, {SGroup::SG_TYPE_MIX, "MIX"}, {SGroup::SG_TYPE_FOR, "FOR"}, {SGroup::SG_TYPE_ANY, "ANY"},
};

const char* SGroup::typeToString(int sg_type)
{
    for (int i = 0; i < NELEM(mappingForSgTypes); i++)
    {
        if (sg_type == mappingForSgTypes[i].int_type)
            return mappingForSgTypes[i].str_type;
    }
    return NULL;
}

int SGroup::getType(const char* sg_type)
{
    for (int i = 0; i < NELEM(mappingForSgTypes); i++)
    {
        if (strcasecmp(sg_type, mappingForSgTypes[i].str_type) == 0)
        {
            return mappingForSgTypes[i].int_type;
        }
    }
    return -1;
}

SGroup::SGroup()
{
    sgroup_type = SGroup::SG_TYPE_GEN;
    sgroup_subtype = 0;
    brk_style = 0;
    original_group = 0;
    parent_group = 0;
    parent_idx = -1;
    contracted = DisplayOption::Undefined;
}

SGroup::~SGroup()
{
}

DataSGroup::DataSGroup()
{
    sgroup_type = SGroup::SG_TYPE_DAT;
    detached = false;
    relative = false;
    display_units = false;
    dasp_pos = 1;
    num_chars = 0;
    tag = ' ';
}

DataSGroup::~DataSGroup()
{
}

constexpr char DataSGroup::mrv_implicit_h[];
constexpr char DataSGroup::impl_prefix[];

bool DataSGroup::isMrv_implicit()
{
    return name.size() == sizeof(mrv_implicit_h) && strncmp(name.ptr(), mrv_implicit_h, name.size()) == 0;
}

void DataSGroup::setMrv_implicit(int atom_idx, int hydrogens_count)
{
    atoms.push(atom_idx);
    std::string sdata = impl_prefix + std::to_string(hydrogens_count);
    data.readString(sdata.c_str(), true);
    name.readString(mrv_implicit_h, true);
    detached = true;
}

Superatom::Superatom()
{
    sgroup_type = SGroup::SG_TYPE_SUP;
    seqid = -1;
    attachment_points.clear();
    bond_connections.clear();
}

Superatom::~Superatom()
{
}

RepeatingUnit::RepeatingUnit()
{
    sgroup_type = SGroup::SG_TYPE_SRU;
    connectivity = 0;
}

RepeatingUnit::~RepeatingUnit()
{
}

MultipleGroup::MultipleGroup()
{
    sgroup_type = SGroup::SG_TYPE_MUL;
    multiplier = 1;
}

MultipleGroup::~MultipleGroup()
{
}

IMPL_ERROR(MoleculeSGroups, "molecule sgroups");

MoleculeSGroups::MoleculeSGroups()
{
    _sgroups.clear();
}

MoleculeSGroups::~MoleculeSGroups()
{
    _sgroups.clear();
}

void MoleculeSGroups::clear()
{
    _sgroups.clear();
}

void MoleculeSGroups::clear(int sg_type)
{
    for (int i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
    {
        if (_sgroups.at(i)->sgroup_type == sg_type)
            remove(i);
    }
}

void MoleculeSGroups::remove(int idx)
{
    _sgroups.remove(idx);
}

int MoleculeSGroups::begin()
{
    return _sgroups.begin();
}

int MoleculeSGroups::end()
{
    return _sgroups.end();
}

int MoleculeSGroups::next(int i)
{
    return _sgroups.next(i);
}

int MoleculeSGroups::addSGroup(const char* sg_type)
{
    int sgroup_type = SGroup::getType(sg_type);

    if (sgroup_type == -1)
        throw Error("Unknown SGroup type = %s", sg_type);

    return addSGroup(sgroup_type);
}

int MoleculeSGroups::addSGroup(int sg_type)
{
    int idx = -1;
    if (sg_type == SGroup::SG_TYPE_GEN)
    {
        idx = _sgroups.add(new SGroup());
    }
    else if (sg_type == SGroup::SG_TYPE_DAT)
    {
        idx = _sgroups.add(new DataSGroup());
    }
    else if (sg_type == SGroup::SG_TYPE_SUP)
    {
        idx = _sgroups.add(new Superatom());
    }
    else if (sg_type == SGroup::SG_TYPE_SRU)
    {
        idx = _sgroups.add(new RepeatingUnit());
    }
    else if (sg_type == SGroup::SG_TYPE_MUL)
    {
        idx = _sgroups.add(new MultipleGroup());
    }
    else
    {
        idx = _sgroups.add(new SGroup());
        if (idx != -1)
            _sgroups.at(idx)->sgroup_type = sg_type;
    }
    return idx;
}

SGroup& MoleculeSGroups::getSGroup(int idx)
{
    if (_sgroups.hasElement(idx))
        return *_sgroups.at(idx);

    throw Error("Sgroup with index %d is not found", idx);
}

bool MoleculeSGroups::hasSGroup(int idx)
{
    return _sgroups.hasElement(idx);
}

SGroup& MoleculeSGroups::getSGroup(int idx, int sg_type)
{
    int count = -1;
    for (int i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
    {
        if (_sgroups.at(i)->sgroup_type == sg_type)
        {
            count++;
            if (count == idx)
                return *_sgroups.at(i);
        }
    }
    throw Error("Sgroup index %d or type %d wrong", idx, sg_type);
}

int MoleculeSGroups::getSGroupCount()
{
    return _sgroups.size();
}

int MoleculeSGroups::getSGroupCount(int sg_type)
{
    int count = 0;
    for (int i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
    {
        if (_sgroups.at(i)->sgroup_type == sg_type)
            count++;
    }
    return count;
}

void MoleculeSGroups::buildTree(Tree& tree)
{
    for (auto i = begin(); i != end(); i = next(i))
    {
        SGroup& sgroup = getSGroup(i);
        tree.insert(i, sgroup.parent_idx);
    }
}

bool MoleculeSGroups::getParentAtoms(int idx, Array<int>& target)
{
    return getParentAtoms(getSGroup(idx), target);
}

bool MoleculeSGroups::getParentAtoms(SGroup& sgroup, Array<int>& target)
{
    if (sgroup.parent_idx < 0)
    {
        return false;
    }

    SGroup& parent = getSGroup(sgroup.parent_idx);
    getParentAtoms(parent, target);
    target.concat(parent.atoms);
    return true;
}

bool MoleculeSGroups::isPolimer()
{
    return getSGroupCount(SGroup::SG_TYPE_SRU) > 0;
}

void MoleculeSGroups::findSGroups(const char* property, const char* value, Array<int>& sgs)
{
    QS_DEF(Array<int>, s_indices);
    int s_property;
    int s_type;
    int s_int;

    sgs.clear();
    s_indices.clear();

    parseCondition(property, value, s_property, s_type, s_int, s_indices);

    if (s_type == PropertyTypes::PROPERTY_INT)
        findSGroups(s_property, s_int, sgs);
    else if (s_type == PropertyTypes::PROPERTY_STRING)
        findSGroups(s_property, value, sgs);
    else if (s_type == PropertyTypes::PROPERTY_INT_ARRAY)
        findSGroups(s_property, s_indices, sgs);
}

void MoleculeSGroups::parseCondition(const char* property, const char* value, int& s_property, int& s_type, int& s_int, Array<int>& s_indices)
{
    struct Mapping
    {
        const char* property;
        int sg_property;
        int property_type;
    };

    static Mapping mappingForProperties[] = {
        {"SG_TYPE", SGroup::SG_TYPE, PROPERTY_INT},
        {"SG_CLASS", SGroup::SG_CLASS, PROPERTY_STRING},
        {"SG_LABEL", SGroup::SG_LABEL, PROPERTY_STRING},
        {"SG_DISPLAY_OPTION", SGroup::SG_DISPLAY_OPTION, PROPERTY_INT},
        {"SG_BRACKET_STYLE", SGroup::SG_BRACKET_STYLE, PROPERTY_INT},
        {"SG_DATA", SGroup::SG_DATA, PROPERTY_STRING},
        {"SG_DATA_NAME", SGroup::SG_DATA_NAME, PROPERTY_STRING},
        {"SG_DATA_TYPE", SGroup::SG_DATA_TYPE, PROPERTY_STRING},
        {"SG_DATA_DESCRIPTION", SGroup::SG_DATA_DESCRIPTION, PROPERTY_STRING},
        {"SG_DATA_DISPLAY", SGroup::SG_DATA_DISPLAY, PROPERTY_STRING},
        {"SG_DATA_LOCATION", SGroup::SG_DATA_LOCATION, PROPERTY_STRING},
        {"SG_DATA_TAG", SGroup::SG_DATA_TAG, PROPERTY_STRING},
        {"SG_QUERY_CODE", SGroup::SG_QUERY_CODE, PROPERTY_STRING},
        {"SG_QUERY_OPER", SGroup::SG_QUERY_OPER, PROPERTY_STRING},
        {"SG_PARENT", SGroup::SG_PARENT, PROPERTY_INT},
        {"SG_CHILD", SGroup::SG_CHILD, PROPERTY_INT},
        {"SG_ATOMS", SGroup::SG_ATOMS, PROPERTY_INT_ARRAY},
        {"SG_BONDS", SGroup::SG_BONDS, PROPERTY_INT_ARRAY},
    };

    for (int i = 0; i < NELEM(mappingForProperties); i++)
    {
        if (strcasecmp(property, mappingForProperties[i].property) == 0)
        {
            int int_value = 0;
            if (strcasecmp(property, "SG_TYPE") == 0)
            {
                for (int j = 0; j < NELEM(mappingForSgTypes); j++)
                {
                    if (strcasecmp(value, mappingForSgTypes[j].str_type) == 0)
                    {
                        int_value = mappingForSgTypes[j].int_type;
                    }
                }
            }
            else if (value != NULL)
            {
                if (mappingForProperties[i].property_type == PROPERTY_INT)
                {
                    BufferScanner buf_scanner(value);
                    int_value = buf_scanner.readInt();
                }
                else if (mappingForProperties[i].property_type == PROPERTY_BOOL)
                {
                    if (strcasecmp(value, "true") == 0)
                        int_value = 1;
                    else if (strcasecmp(value, "false") == 0)
                        int_value = 0;
                    else
                    {
                        BufferScanner buf_scanner(value);
                        int_value = buf_scanner.readInt();
                    }
                }
                else if (mappingForProperties[i].property_type == PROPERTY_INT_ARRAY)
                {
                    BufferScanner buf_scanner(value);
                    while (!buf_scanner.isEOF())
                    {
                        s_indices.push(buf_scanner.readInt1());
                    }
                }
            }
            s_property = mappingForProperties[i].sg_property;
            s_type = mappingForProperties[i].property_type;
            s_int = int_value;

            return;
        }
    }

    throw Error("unsupported condition property: %s", property);
}

void MoleculeSGroups::findSGroups(int property, int value, Array<int>& sgs)
{
    int i;
    if (property == SGroup::SG_TYPE)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == value)
            {
                sgs.push(i);
            }
        }
    }
    else if (property == SGroup::SG_BRACKET_STYLE)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.brk_style == value)
            {
                sgs.push(i);
            }
        }
    }
    else if (property == SGroup::SG_DISPLAY_OPTION)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
            {
                Superatom& sup = (Superatom&)sg;
                if (sup.contracted == (DisplayOption)value)
                {
                    sgs.push(i);
                }
            }
        }
    }
    else if (property == SGroup::SG_PARENT)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.parent_group == value)
            {
                sgs.push(i);
            }
        }
    }
    else if (property == SGroup::SG_CHILD)
    {
        if (!_sgroups.hasElement(value))
            return;

        SGroup& sg = *_sgroups.at(value);
        if (sg.parent_group != 0)
        {
            int idx = _findSGroupById(sg.parent_group);
            if (idx != -1)
                sgs.push(idx);
        }
    }
    else
        throw Error("Unknown or incomaptible value Sgroup property: %d", property);
}

void MoleculeSGroups::findSGroups(int property, const char* str, Array<int>& sgs)
{
    int i;
    if (property == SGroup::SG_CLASS)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
            {
                Superatom& sa = (Superatom&)sg;
                BufferScanner sc(sa.sa_class);
                if (sc.findWordIgnoreCase(str))
                {
                    sgs.push(i);
                }
            }
        }
    }
    else if (property == SGroup::SG_LABEL)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
            {
                Superatom& sa = (Superatom&)sg;
                BufferScanner sc(sa.subscript);
                if (sc.findWordIgnoreCase(str))
                {
                    sgs.push(i);
                }
            }
            else if (sg.sgroup_type == SGroup::SG_TYPE_SRU)
            {
                RepeatingUnit& ru = (RepeatingUnit&)sg;
                BufferScanner sc(ru.subscript);
                if (sc.findWordIgnoreCase(str))
                {
                    sgs.push(i);
                }
            }
        }
    }
    else if (property == SGroup::SG_DATA)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dg = (DataSGroup&)sg;
                BufferScanner sc(dg.data);
                if (sc.findWordIgnoreCase(str))
                {
                    sgs.push(i);
                }
            }
        }
    }
    else if (property == SGroup::SG_DATA_NAME)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dg = (DataSGroup&)sg;
                BufferScanner sc(dg.name);
                if (sc.findWordIgnoreCase(str))
                {
                    sgs.push(i);
                }
            }
        }
    }
    else if (property == SGroup::SG_DATA_TYPE)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dg = (DataSGroup&)sg;
                BufferScanner sc(dg.type);
                if (sc.findWordIgnoreCase(str))
                {
                    sgs.push(i);
                }
            }
        }
    }
    else if (property == SGroup::SG_DATA_DESCRIPTION)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dg = (DataSGroup&)sg;
                BufferScanner sc(dg.description);
                if (sc.findWordIgnoreCase(str))
                {
                    sgs.push(i);
                }
            }
        }
    }
    else if (property == SGroup::SG_DATA_DISPLAY)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dg = (DataSGroup&)sg;
                if (((strcasecmp(str, "detached") == 0) && dg.detached) || ((strcasecmp(str, "attached") == 0) && !dg.detached))
                {
                    sgs.push(i);
                }
            }
        }
    }
    else if (property == SGroup::SG_DATA_LOCATION)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dg = (DataSGroup&)sg;
                if (((strcasecmp(str, "relative") == 0) && dg.relative) || ((strcasecmp(str, "absolute") == 0) && !dg.relative))
                {
                    sgs.push(i);
                }
            }
        }
    }
    else if (property == SGroup::SG_DATA_TAG)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dg = (DataSGroup&)sg;
                if ((strlen(str) == 1) && str[0] == dg.tag)
                {
                    sgs.push(i);
                }
            }
        }
    }
    else if (property == SGroup::SG_QUERY_CODE)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dg = (DataSGroup&)sg;
                BufferScanner sc(dg.querycode);
                if (sc.findWordIgnoreCase(str))
                {
                    sgs.push(i);
                }
            }
        }
    }
    else if (property == SGroup::SG_QUERY_OPER)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (sg.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dg = (DataSGroup&)sg;
                BufferScanner sc(dg.queryoper);
                if (sc.findWordIgnoreCase(str))
                {
                    sgs.push(i);
                }
            }
        }
    }
    else
        throw Error("Unknown or incomaptible value Sgroup property: %d", property);
}

void MoleculeSGroups::findSGroups(int property, Array<int>& indices, Array<int>& sgs)
{
    int i;
    if (property == SGroup::SG_ATOMS)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (_cmpIndices(sg.atoms, indices))
            {
                sgs.push(i);
            }
        }
    }
    else if (property == SGroup::SG_BONDS)
    {
        for (i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
        {
            SGroup& sg = *_sgroups.at(i);
            if (_cmpIndices(sg.bonds, indices))
            {
                sgs.push(i);
            }
        }
    }
    else
        throw Error("Unknown or incomaptible value Sgroup property: %d", property);
}

void MoleculeSGroups::registerUnfoldedHydrogen(int idx, int new_h_idx)
{
    for (int i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
    {
        SGroup& sg = *_sgroups.at(i);
        if (sg.atoms.find(idx) != -1)
        {
            sg.atoms.push(new_h_idx);
        }
    }
}

int MoleculeSGroups::_findSGroupById(int id)
{
    for (int i = _sgroups.begin(); i != _sgroups.end(); i = _sgroups.next(i))
    {
        SGroup& sg = *_sgroups.at(i);
        if (sg.original_group == id)
        {
            return i;
        }
    }
    return -1;
}

bool MoleculeSGroups::_cmpIndices(Array<int>& t_inds, Array<int>& q_inds)
{
    for (int i = 0; i < q_inds.size(); i++)
    {
        if (t_inds.find(q_inds[i]) == -1)
            return false;
    }
    return true;
}
