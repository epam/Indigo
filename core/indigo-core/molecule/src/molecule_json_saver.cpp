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

#include <iomanip>
#include <memory>
#include <set>
#include <sstream>

#include "layout/molecule_layout.h"

#include "molecule/molecule.h"
#include "molecule/molecule_cip_calculator.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/molecule_savers.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/monomer_commons.h"
#include "molecule/monomers_template_library.h"
#include "molecule/parse_utils.h"

#include "molecule/query_molecule.h"
#include "molecule/smiles_loader.h"
#include "molecule/smiles_saver.h"
#include "reaction/pathway_reaction.h"
#include "reaction/reaction_multistep_detector.h"

#include <base_cpp/scanner.h>

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(MoleculeJsonSaver, "molecule json saver");

void dumpAtoms(BaseMolecule& mol)
{
    for (auto i : mol.vertices())
    {
        Array<char> buff;
        mol.getAtomSymbol(i, buff);
        printf("%s,", buff.ptr());
    }
    printf("\n");
}

void printMappings(Array<int>& mapping)
{
    for (int i = 0; i < mapping.size(); ++i)
    {
        printf("%d ", mapping[i]);
    }
    printf("\n");
}

MoleculeJsonSaver::MoleculeJsonSaver(Output& output)
    : _output(output), _pmol(nullptr), _pqmol(nullptr), add_stereo_desc(false), pretty_json(false), use_native_precision(false), ket_version(KETVersion1),
      add_reaction_data(false)
{
}

MoleculeJsonSaver::MoleculeJsonSaver(Output& output, ReactionMultistepDetector& rmd) : MoleculeJsonSaver(output)
{
    _rmd = rmd;
}

void MoleculeJsonSaver::parseFormatMode(const char* version_str, KETVersion& version)
{
    auto version_data = split(version_str, '.');
    for (size_t i = 0; i < version_data.size(); ++i)
    {
        int val = std::stoi(version_data[i]);
        switch (static_cast<KETVersionIndex>(i))
        {
        case KETVersionIndex::EMajor:
            version.major = val;
            break;
        case KETVersionIndex::EMinor:
            version.minor = val;
            break;
        case KETVersionIndex::EPatch:
            version.patch = val;
            break;
        }
    }
}

void MoleculeJsonSaver::saveFormatMode(KETVersion& version, Array<char>& output)
{
    std::string ver;
    ver += std::to_string(version.major) + "." + std::to_string(version.minor) + "." + std::to_string(version.patch);
    output.readString(ver.c_str(), true);
}

void MoleculeJsonSaver::_checkSGroupIndices(BaseMolecule& mol, Array<int>& sgs_list)
{
    QS_DEF(Array<int>, orig_ids);
    QS_DEF(Array<int>, added_ids);
    QS_DEF(Array<int>, sgs_mapping);
    QS_DEF(Array<int>, sgs_changed);

    sgs_list.clear();
    orig_ids.clear();
    added_ids.clear();
    sgs_mapping.clear_resize(mol.sgroups.end());
    sgs_mapping.zerofill();
    sgs_changed.clear_resize(mol.sgroups.end());
    sgs_changed.zerofill();

    int iw = 1;
    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sgroup = mol.sgroups.getSGroup(i);
        if (sgroup.parent_group == 0)
        {
            sgs_mapping[i] = iw;
            iw++;
        }
    }
    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        if (sgs_mapping[i] == 0)
        {
            sgs_mapping[i] = iw;
            iw++;
        }
    }

    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sgroup = mol.sgroups.getSGroup(i);
        if (sgroup.original_group == 0)
        {
            sgroup.original_group = sgs_mapping[i];
        }
        else
        {
            for (int j = mol.sgroups.begin(); j != mol.sgroups.end(); j = mol.sgroups.next(j))
            {
                SGroup& sg = mol.sgroups.getSGroup(j);
                if (sg.parent_group == sgroup.original_group && sgs_changed[j] == 0)
                {
                    sg.parent_group = sgs_mapping[i];
                    sgs_changed[j] = 1;
                }
            }
            sgroup.original_group = sgs_mapping[i];
        }
        orig_ids.push(sgroup.original_group);
    }

    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sgroup = mol.sgroups.getSGroup(i);
        if (sgroup.parent_group == 0)
        {
            sgs_list.push(i);
            added_ids.push(sgroup.original_group);
        }
        else
        {
            if (orig_ids.find(sgroup.parent_group) == VALUE_UNKNOWN || sgroup.parent_group == sgroup.original_group)
            {
                sgroup.parent_group = 0;
                sgs_list.push(i);
                added_ids.push(sgroup.original_group);
            }
        }
    }

    for (;;)
    {
        for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            SGroup& sgroup = mol.sgroups.getSGroup(i);
            if (sgroup.parent_group == 0)
                continue;

            if (added_ids.find(sgroup.original_group) != VALUE_UNKNOWN)
                continue;

            if (added_ids.find(sgroup.parent_group) != VALUE_UNKNOWN)
            {
                sgs_list.push(i);
                added_ids.push(sgroup.original_group);
            }
        }
        if (sgs_list.size() == mol.countSGroups())
            break;
    }
}

void MoleculeJsonSaver::saveSGroups(BaseMolecule& mol, JsonWriter& writer)
{
    QS_DEF(Array<int>, sgs_sorted);
    _checkSGroupIndices(mol, sgs_sorted);
    int sGroupsCount = mol.countSGroups();
    bool componentDefined = false;
    if (mol.isQueryMolecule())
    {
        QueryMolecule& qmol = static_cast<QueryMolecule&>(mol);
        if (qmol.components.size() > 0 && qmol.components[0])
        {
            componentDefined = true;
            sGroupsCount++;
        }
    }

    if (sGroupsCount > 0)
    {
        writer.Key("sgroups");
        writer.StartArray();
        // int idx = 1;
        for (int i = 0; i < sgs_sorted.size(); i++)
        {
            int sg_idx = sgs_sorted[i];
            auto& sgrp = mol.sgroups.getSGroup(sg_idx);
            saveSGroup(sgrp, writer);
        }
        // save queryComponent
        if (mol.isQueryMolecule() && componentDefined)
        {
            QueryMolecule& qmol = static_cast<QueryMolecule&>(mol);
            writer.StartObject();
            writer.Key("type");
            writer.String("queryComponent");
            writer.Key("atoms");
            writer.StartArray();
            for (int i = 0; i < qmol.vertexCount(); i++)
            {
                if (qmol.components[i])
                {
                    writer.Int(i);
                }
            }
            writer.EndArray();
            writer.EndObject();
        }
        writer.EndArray();
    }
}

void MoleculeJsonSaver::saveSGroup(SGroup& sgroup, JsonWriter& writer)
{
    writer.StartObject();
    writer.Key("type");
    writer.String(SGroup::typeToString(sgroup.sgroup_type));
    writer.Key("atoms");
    if (sgroup.sgroup_type != SGroup::SG_TYPE_MUL)
    {
        writer.StartArray();
        for (int i = 0; i < sgroup.atoms.size(); i++)
            writer.Int(sgroup.atoms[i]);
        writer.EndArray();
    }

    switch (sgroup.sgroup_type)
    {
    case SGroup::SG_TYPE_GEN:
        break;
    case SGroup::SG_TYPE_DAT: {
        DataSGroup& dsg = (DataSGroup&)sgroup;
        auto name = dsg.name.ptr();
        auto data = dsg.data.ptr();
        if (name && strlen(name))
        {
            writer.Key("fieldName");
            writer.String(name);
        }
        if (data && strlen(data))
        {
            writer.Key("fieldData");
            writer.String(data);
        }
        auto field_type = dsg.description.ptr();
        if (field_type && strlen(field_type))
        {
            writer.Key("fieldType");
            writer.String(field_type);
        }
        auto query_code = dsg.querycode.ptr();
        if (query_code && strlen(query_code))
        {
            writer.Key("queryType");
            writer.String(query_code);
        }
        auto query_oper = dsg.queryoper.ptr();
        if (query_oper && strlen(query_oper))
        {
            writer.Key("queryOp");
            writer.String(query_oper);
        }

        writer.Key("x");
        writeFloat(writer, dsg.display_pos.x);
        writer.Key("y");
        writeFloat(writer, dsg.display_pos.y);

        if (!dsg.detached)
        {
            writer.Key("dataDetached");
            writer.Bool(false);
        }

        if (dsg.relative)
        {
            writer.Key("placement");
            writer.Bool(true);
        }

        if (dsg.display_units)
        {
            writer.Key("display");
            writer.Bool(true);
        }

        char tag = dsg.tag;
        if (tag != 0 && tag != ' ')
        {
            writer.Key("tag");
            std::string tag_s{tag};
            writer.String(tag_s.c_str());
        }

        if (dsg.num_chars > 0)
        {
            writer.Key("displayedChars");
            writer.Int(dsg.num_chars);
        }
    }
    break;
    case SGroup::SG_TYPE_SUP: {
        Superatom& sa = (Superatom&)sgroup;
        writer.Key("name");
        writer.String(sa.subscript.size() ? sa.subscript.ptr() : "");
        if (sa.contracted == DisplayOption::Expanded)
        {
            writer.Key("expanded");
            writer.Bool(true);
        }

        if (sa.sa_class.size())
        {
            writer.Key("class");
            writer.String(sa.sa_class.ptr());
        }

        if (sa.attachment_points.size())
        {
            writer.Key("attachmentPoints");
            writer.StartArray();
            for (int i = sa.attachment_points.begin(); i != sa.attachment_points.end(); i = sa.attachment_points.next(i))
            {
                writer.StartObject();
                auto& atp = sa.attachment_points[i];
                std::string atp_id_str(atp.apid.ptr());
                writer.Key("attachmentAtom");
                writer.Int(atp.aidx);
                if (atp.lvidx != VALUE_UNKNOWN)
                {
                    writer.Key("leavingAtom");
                    writer.Int(atp.lvidx);
                }
                if (atp_id_str.length() > 0)
                {
                    writer.Key("attachmentId");
                    writer.String(convertAPToHELM(atp_id_str).c_str());
                }
                writer.EndObject();
            }
            writer.EndArray();
        }
    }
    break;
    case SGroup::SG_TYPE_SRU: {
        RepeatingUnit& ru = (RepeatingUnit&)sgroup;
        if (ru.subscript.size())
        {
            writer.Key("subscript");
            writer.String(ru.subscript.ptr());
        }

        writer.Key("connectivity");
        switch (ru.connectivity)
        {
        case SGroup::HEAD_TO_TAIL:
            writer.String("HT");
            break;
        case SGroup::HEAD_TO_HEAD:
            writer.String("HH");
            break;
        default:
            writer.String("EU");
            break;
        }
    }
    break;
    case SGroup::SG_TYPE_MUL: {
        MultipleGroup& mg = (MultipleGroup&)sgroup;
        if (mg.parent_atoms.size())
        {
            writer.StartArray();
            for (int i = 0; i < mg.parent_atoms.size(); i++)
                writer.Int(mg.parent_atoms[i]);
            writer.EndArray();
        }
        writer.Key("mul");
        writer.Int(mg.multiplier);
    }
    break;
    case SGroup::SG_TYPE_MON:
        throw Error("SG_TYPE_MON not implemented in indigo yet");
        break;
    case SGroup::SG_TYPE_MER:
        throw Error("SG_TYPE_MER not implemented in indigo yet");
        break;
    case SGroup::SG_TYPE_COP: {
        CopolymerGroup& ru = (CopolymerGroup&)sgroup;
        if (ru.sgroup_subtype != 0)
        {
            writer.Key("subtype");
            if (ru.sgroup_subtype == SGroup::SG_SUBTYPE_ALT)
            {
                writer.String("ALT");
            }
            else if (ru.sgroup_subtype == SGroup::SG_SUBTYPE_RAN)
            {
                writer.String("RAN");
            }
            else if (ru.sgroup_subtype == SGroup::SG_SUBTYPE_BLO)
            {
                writer.String("BLO");
            }
        }

        writer.Key("connectivity");
        switch (ru.connectivity)
        {
        case SGroup::HEAD_TO_TAIL:
            writer.String("HT");
            break;
        case SGroup::HEAD_TO_HEAD:
            writer.String("HH");
            break;
        default:
            writer.String("EU");
            break;
        }
    }
    break;
    case SGroup::SG_TYPE_CRO:
        throw Error("SG_TYPE_CRO not implemented in indigo yet");
        break;
    case SGroup::SG_TYPE_MOD:
        throw Error("SG_TYPE_MOD not implemented in indigo yet");
        break;
    case SGroup::SG_TYPE_GRA:
        throw Error("SG_TYPE_GRA not implemented in indigo yet");
        break;
    case SGroup::SG_TYPE_COM:
        throw Error("SG_TYPE_COM not implemented in indigo yet");
        break;
    case SGroup::SG_TYPE_MIX:
        throw Error("SG_TYPE_MIX not implemented in indigo yet");
        break;
    case SGroup::SG_TYPE_FOR:
        throw Error("SG_TYPE_FOR not implemented in indigo yet");
        break;
    case SGroup::SG_TYPE_ANY:
        throw Error("SG_TYPE_ANY not implemented in indigo yet");
        break;
    default:
        break;
    }

    if (sgroup.bonds.size())
    {
        writer.Key("bonds");
        writer.StartArray();
        for (int i = 0; i < sgroup.bonds.size(); ++i)
            writer.Int(sgroup.bonds[i]);
        writer.EndArray();
    }

    writer.EndObject();
}

void MoleculeJsonSaver::saveBonds(BaseMolecule& mol, JsonWriter& writer)
{
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    if (mol.edgeCount() > 0)
    {
        for (auto i : mol.edges())
        {
            int direction = BOND_ZERO;
            bool negative = false;
            writer.StartObject();
            if (_pmol)
            {
                int bond_order = mol.getBondOrder(i);

                if (bond_order == BOND_ZERO && _pmol)
                {
                    bond_order = _BOND_COORDINATION;
                    const Edge& edge = mol.getEdge(i);
                    if ((_pmol->getAtomNumber(edge.beg) == ELEM_H) || (_pmol->getAtomNumber(edge.end) == ELEM_H))
                        bond_order = _BOND_HYDROGEN;
                }

                writer.Key("type");
                writer.Uint(bond_order);
            }
            else if (_pqmol)
            {
                QueryMolecule::Bond& qbond = _pqmol->getBond(i);
                int bond_order = QueryMolecule::getQueryBondType(qbond, direction, negative);
                if (bond_order < 0 || negative || direction == BOND_UP_OR_UNSPECIFIED || direction == BOND_DOWN_OR_UNSPECIFIED)
                {
                    std::string customQuery = QueryMolecule::getSmartsBondStr(&qbond);
                    writer.Key("customQuery");
                    writer.String(customQuery.c_str());
                    direction = BOND_ZERO; // clean up to not override stereo
                }
                else
                {
                    writer.Key("type");
                    writer.Uint(bond_order);
                    // convert direction to Biovia constants, to override stereo later
                    switch (direction)
                    {
                    case BOND_UP:
                        direction = BIOVIA_STEREO_UP;
                        break;
                    case BOND_DOWN:
                        direction = BIOVIA_STEREO_DOWN;
                        break;
                    default:
                        direction = BOND_ZERO; // clean up to not override stereo
                        break;
                    }
                }
            }

            int topology = VALUE_UNKNOWN;
            if (_pqmol)
            {
                _pqmol->getBond(i).sureValue(QueryMolecule::BOND_TOPOLOGY, topology);
                if (topology > 0)
                {
                    writer.Key("topology");
                    writer.Uint(topology);
                }
            }

            if (i < mol.reaction_bond_reacting_center.size())
            {
                int rcenter = mol.reaction_bond_reacting_center[i];
                if (rcenter)
                {
                    writer.Key("center");
                    writer.Int(rcenter);
                }
            }

            if (mol.isBondSelected(i))
            {
                writer.Key("selected");
                writer.Bool(true);
            }

            const Edge& e1 = mol.getEdge(i);
            writer.Key("atoms");
            writer.StartArray();
            writer.Int(e1.beg);
            writer.Int(e1.end);
            writer.EndArray();

            int stereo = mol.getBondDirection(i);
            if (mol.cis_trans.isIgnored(i))
                stereo = 3;
            else
                switch (stereo)
                {
                case BOND_UP:
                    stereo = BIOVIA_STEREO_UP;
                    break;
                case BOND_EITHER:
                    stereo = BIOVIA_STEREO_ETHER;
                    break;
                case BOND_DOWN:
                    stereo = BIOVIA_STEREO_DOWN;
                    break;
                default: {
                    stereo = BIOVIA_STEREO_NO;
                }
                break;
                }

            if (stereo || direction)
            {
                writer.Key("stereo");
                writer.Uint(direction ? direction : stereo); // If have stored direction - override calculated
            }

            auto cip = mol.getBondCIP(i);
            if (cip != CIPDesc::NONE)
            {
                auto cip_str = CIPToString(cip);
                if (cip_str.size())
                {
                    writer.Key("cip");
                    writer.String(cip_str.c_str());
                }
            }

            writer.EndObject();
        }
    }
}

void MoleculeJsonSaver::saveAttachmentPoint(BaseMolecule& mol, int atom_idx, JsonWriter& writer)
{
    int val = 0;
    for (int idx = 1; idx <= mol.attachmentPointCount(); idx++)
    {
        for (int j = 0; mol.getAttachmentPoint(idx, j) != VALUE_UNKNOWN; j++)
        {
            if (mol.getAttachmentPoint(idx, j) == atom_idx)
            {
                val |= 1 << (idx - 1);
                break;
            }
        }
    }

    if (val > 0)
    {
        writer.Key("attachmentPoints");
        writer.Int(val);
    }
}

void MoleculeJsonSaver::saveStereoCenter(BaseMolecule& mol, int atom_idx, JsonWriter& writer)
{
    writer.Key("pyramid");
    writer.StartArray();
    const int* pyramid = mol.stereocenters.getPyramid(atom_idx);
    for (int i = 0; i < 4; ++i)
    {
        int prm = pyramid[i];
        if (prm == VALUE_UNKNOWN && i == 3)
            prm = atom_idx;
        writer.Int(prm);
    }
    writer.EndArray();
}

void MoleculeJsonSaver::saveHighlights(BaseMolecule& mol, JsonWriter& writer)
{
    int ca = mol.countHighlightedAtoms();
    int cb = mol.countHighlightedBonds();
    if (ca || cb)
    {
        writer.Key("highlight");
        writer.StartArray();

        if (ca)
        {
            writer.Key("entityType");
            writer.String("atom");
            writer.StartObject();
            writer.Key("items");
            writer.StartArray();
            for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            {
                if (mol.isAtomHighlighted(i))
                {
                    writer.Int(i);
                }
            }
            writer.EndArray();
            writer.EndObject();
        }

        if (cb)
        {
            writer.Key("entityType");
            writer.String("bond");
            writer.StartObject();
            writer.Key("items");
            writer.StartArray();
            for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
            {
                if (mol.isBondHighlighted(i))
                {
                    writer.Int(i);
                }
            }
            writer.EndArray();
            writer.EndObject();
        }

        writer.EndArray();
    }
}

void saveNativeFloat(JsonWriter& writer, float f_value, int precision)
{
    std::string val;
    if (precision >= 0)
    {
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(precision) << f_value;
        val = oss.str();
    }
    else
    {
        val = std::to_string(f_value);
    }
    writer.RawValue(val.c_str(), val.length(), kStringType);
}

void MoleculeJsonSaver::writeFloat(JsonWriter& writer, float f_value)
{
    if (use_native_precision)
        saveNativeFloat(writer, f_value, native_precision);
    else
        writer.Double(f_value);
}

void MoleculeJsonSaver::writePos(JsonWriter& writer, const Vec3f& pos)
{
    writer.StartObject();
    writer.Key("x");
    writeFloat(writer, pos.x);
    writer.Key("y");
    writeFloat(writer, pos.y);
    writer.Key("z");
    writeFloat(writer, pos.z);
    writer.EndObject();
}

void MoleculeJsonSaver::saveAtoms(BaseMolecule& mol, JsonWriter& writer)
{
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    for (auto i : mol.vertices())
    {
        buf.clear();
        int anum = mol.getAtomNumber(i);
        int isotope = mol.getAtomIsotope(i);
        int radical = 0;

        if (!mol.isPseudoAtom(i) && !mol.isTemplateAtom(i) && !mol.isRSite(i))
            radical = mol.getAtomRadical(i);

        writer.StartObject();
        if (mol.attachmentPointCount())
            saveAttachmentPoint(mol, i, writer);
        QS_DEF(Array<int>, rg_list);
        int query_atom_type = QueryMolecule::QUERY_ATOM_UNKNOWN;
        bool needCustomQuery = false;
        std::map<int, std::unique_ptr<QueryMolecule::Atom>> query_atom_properties;
        bool is_rSite = mol.isRSite(i);
        if (is_rSite)
        {
            mol.getAllowedRGroups(i, rg_list);
            writer.Key("type");
            writer.String("rg-label");
            writer.Key("$refs");
            writer.StartArray();
            for (int j = 0; j < rg_list.size(); ++j)
            {
                buf.clear();
                out.printf("rg-%d", rg_list[j]);
                buf.push(0);
                writer.String(buf.ptr());
            }
            writer.EndArray();
        }
        else
        {
            bool is_qatom_list = false;
            std::vector<std::unique_ptr<QueryMolecule::Atom>> atoms;
            if (_pqmol)
            {
                query_atom_type = QueryMolecule::parseQueryAtomSmarts(*_pqmol, i, atoms, query_atom_properties);
                needCustomQuery = query_atom_type == QueryMolecule::QUERY_ATOM_UNKNOWN;
                if (query_atom_properties.count(QueryMolecule::ATOM_CHIRALITY) &&
                    (query_atom_properties[QueryMolecule::ATOM_CHIRALITY]->value_min != QueryMolecule::CHIRALITY_GENERAL ||
                     query_atom_properties[QueryMolecule::ATOM_CHIRALITY]->value_max & QueryMolecule::CHIRALITY_OR_UNSPECIFIED))
                    needCustomQuery = true;
            }

            if (mol.isPseudoAtom(i))
            {
                buf.readString(mol.getPseudoAtom(i), true);
            }
            else if (mol.isTemplateAtom(i))
            {
                buf.readString(mol.getTemplateAtom(i), true);
            }
            else if (anum != VALUE_UNKNOWN)
            {
                buf.readString(Element::toString(anum, isotope), true);
            }
            else if (_pqmol)
            {
                if (query_atom_type != QueryMolecule::QUERY_ATOM_UNKNOWN)
                {
                    if (query_atom_type == QueryMolecule::QUERY_ATOM_LIST || query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
                    {
                        is_qatom_list = true;
                        writer.Key("type");
                        writer.String("atom-list");
                        if (query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
                        {
                            writer.Key("notList");
                            writer.Bool(true);
                        }
                        writer.Key("elements");
                        writer.StartArray();
                        for (auto& atom : atoms)
                            if (atom->type == QueryMolecule::ATOM_NUMBER)
                                writer.String(Element::toString(atom->value_max));
                            else if (atom->type == QueryMolecule::ATOM_PSEUDO)
                                writer.String(atom->alias.ptr());
                            else
                                throw Error("Wrong atom type %d", atom->type);
                        writer.EndArray();
                    }
                    else if (query_atom_type == QueryMolecule::QUERY_ATOM_SINGLE)
                    {
                        anum = (*atoms.begin()).get()->value_max;
                        buf.readString(Element::toString(anum), true);
                        if (anum == ELEM_H && query_atom_properties.count(QueryMolecule::ATOM_ISOTOPE) > 0)
                        {
                            int h_isotope = query_atom_properties[QueryMolecule::ATOM_ISOTOPE]->value_min;
                            if (h_isotope == DEUTERIUM)
                            {
                                buf.clear();
                                buf.appendString("D", true);
                            }
                            else if (h_isotope == TRITIUM)
                            {
                                buf.clear();
                                buf.appendString("T", true);
                            }
                        }
                    }
                    else
                    {
                        if (query_atom_type == QueryMolecule::QUERY_ATOM_AH && _pqmol->isAlias(i))
                        {
                            buf.readString(_pqmol->getAlias(i), true);
                        }

                        if (buf.size() != 2 || buf[0] != '*')
                        {
                            buf.clear();
                            QueryMolecule::getQueryAtomLabel(query_atom_type, buf);
                        }
                    }
                }
                else // query_atom_type == QueryMolecule::QUERY_ATOM_UNKNOWN
                {
                    needCustomQuery = true;
                }
            }

            if (needCustomQuery)
            {
                writer.Key("label");
                writer.String("A"); // Set label any atom
            }
            else if (!is_qatom_list)
            {
                writer.Key("label");
                writer.String(buf.ptr());
            }

            if (mol.isAlias(i))
            {
                writer.Key("alias");
                writer.String(mol.getAlias(i));
            }
        }

        if (mol.isAtomSelected(i))
        {
            writer.Key("selected");
            writer.Bool(true);
        }

        Vec3f coord = mol.getAtomXyz(i);
        writer.Key("location");
        writer.StartArray();
        writeFloat(writer, coord.x);
        writeFloat(writer, coord.y);
        writeFloat(writer, coord.z);
        writer.EndArray();

        int charge = mol.getAtomCharge(i);
        int evalence = mol.getExplicitValence(i);
        int mapping = mol.reaction_atom_mapping[i];
        int inv_ret = mol.reaction_atom_inversion[i];
        bool ecflag = mol.reaction_atom_exact_change[i];
        int hcount = MoleculeSavers::getHCount(mol, i, anum, charge);

        if (_pqmol && !is_rSite) // No custom query for RSite
        {
            std::map<int, const char*> qprops{{QueryMolecule::ATOM_SSSR_RINGS, "ringMembership"},
                                              {QueryMolecule::ATOM_SMALLEST_RING_SIZE, "ringSize"},
                                              {QueryMolecule::ATOM_CONNECTIVITY, "connectivity"}};
            bool hasQueryProperties =
                query_atom_properties.count(QueryMolecule::ATOM_AROMATICITY) > 0 || query_atom_properties.count(QueryMolecule::ATOM_CHIRALITY) > 0 ||
                std::any_of(qprops.cbegin(), qprops.cend(), [&query_atom_properties](auto p) { return query_atom_properties.count(p.first) > 0; });
            if (needCustomQuery || hasQueryProperties)
            {
                writer.Key("queryProperties");
                writer.StartObject();
                if (needCustomQuery)
                {
                    QueryMolecule::Atom& atom = _pqmol->getAtom(i);
                    std::string customQuery = QueryMolecule::getSmartsAtomStr(&atom, _pqmol->original_format, false);
                    writer.Key("customQuery");
                    writer.String(customQuery.c_str());
                }
                else
                {
                    int value = VALUE_UNKNOWN;

                    if (query_atom_properties.count(QueryMolecule::ATOM_AROMATICITY))
                    {
                        value = query_atom_properties[QueryMolecule::ATOM_AROMATICITY]->value_min;
                        writer.Key("aromaticity");
                        if (value == ATOM_AROMATIC)
                            writer.String(ATOM_AROMATIC_STR);
                        else if (value == ATOM_ALIPHATIC)
                            writer.String(ATOM_ALIPHATIC_STR);
                        else
                            throw "Wrong aromaticity value";
                    }
                    if (query_atom_properties.count(QueryMolecule::ATOM_CHIRALITY))
                    {
                        // This is CHIRALITY_GENERAL without CHIRALITY_OR_UNSPECIFIED
                        writer.Key("chirality");
                        value = query_atom_properties[QueryMolecule::ATOM_CHIRALITY]->value_max;
                        if (value == QueryMolecule::CHIRALITY_CLOCKWISE)
                            writer.String("clockwise");
                        else if (value == QueryMolecule::CHIRALITY_ANTICLOCKWISE)
                            writer.String("anticlockwise");
                        else
                            throw Error("Wrong chirality value %d", value);
                    }
                    for (auto p : qprops)
                    {
                        if (query_atom_properties.count(p.first) > 0)
                        {
                            writer.Key(p.second);
                            writer.Uint(query_atom_properties[p.first]->value_min);
                        }
                    }
                    // 2do add hirality
                    //*/
                }
                writer.EndObject();
            }

            int subst = 0, rbc = 0;
            if (MoleculeSavers::getRingBondCountFlagValue(*_pqmol, i, rbc))
            {
                writer.Key("ringBondCount");
                writer.Int(rbc);
            }
            if (MoleculeSavers::getSubstitutionCountFlagValue(*_pqmol, i, subst))
            {
                writer.Key("substitutionCount");
                writer.Int(subst);
            }

            int unsat = 0;
            if (_pqmol->getAtom(i).sureValue(QueryMolecule::ATOM_UNSATURATION, unsat))
            {
                writer.Key("unsaturatedAtom");
                writer.Bool(true);
            }

            if (hcount == VALUE_UNKNOWN)
                hcount = 0;
            else
                hcount++;
            if (hcount > 0)
            {
                writer.Key("hCount");
                writer.Int(hcount);
            }
            if (query_atom_type >= 0 && query_atom_properties.count(QueryMolecule::ATOM_IMPLICIT_H) > 0)
            {
                writer.Key("implicitHCount");
                writer.Int(query_atom_properties[QueryMolecule::ATOM_IMPLICIT_H]->value_min);
            }
        }
        else if (_pmol)
        {
            if (Molecule::shouldWriteHCount(mol.asMolecule(), i) && hcount > 0)
            {
                writer.Key("implicitHCount");
                writer.Int(hcount);
            }
        }

        if (mapping)
        {
            writer.Key("mapping");
            writer.Int(mapping);
        }

        if ((mol.isQueryMolecule() && charge != CHARGE_UNKNOWN) || (!mol.isQueryMolecule() && charge != 0))
        {
            writer.Key("charge");
            writer.Int(charge);
        }

        // int total_bond_count = 0;
        if (evalence > 0)
        {
            writer.Key("explicitValence");
            writer.Int(evalence);
        }

        if (radical > 0)
        {
            writer.Key("radical");
            writer.Int(radical);
        }

        if (isotope > 0 && anum != ELEM_H)
        {
            writer.Key("isotope");
            writer.Int(isotope);
        }

        if (inv_ret > 0)
        {
            writer.Key("invRet");
            writer.Int(inv_ret);
        }

        if (ecflag)
        {
            writer.Key("exactChangeFlag");
            writer.Bool(ecflag);
        }

        int enh_stereo_type = mol.stereocenters.getType(i);
        if (enh_stereo_type > 1)
        {
            writer.Key("stereoLabel");
            switch (enh_stereo_type)
            {
            case MoleculeStereocenters::ATOM_ABS:
                writer.String("abs");
                break;
            case MoleculeStereocenters::ATOM_OR:
                writer.String((std::string("or") + std::to_string(mol.stereocenters.getGroup(i))).c_str());
                break;
            case MoleculeStereocenters::ATOM_AND:
                writer.String((std::string("&") + std::to_string(mol.stereocenters.getGroup(i))).c_str());
                break;
            default:
                throw Error("Unknows enhanced stereo type %d", enh_stereo_type);
                break;
            }
        }

        auto cip = mol.getAtomCIP(i);
        if (cip != CIPDesc::NONE)
        {
            auto cip_str = CIPToString(cip);
            if (cip_str.size())
            {
                writer.Key("cip");
                writer.String(cip_str.c_str());
            }
        }

        if (mol.isRSite(i) && !_checkAttPointOrder(mol, i))
        {
            const Vertex& vertex = mol.getVertex(i);
            writer.Key("attachmentOrder");
            writer.StartArray();
            for (int k = 0; k < vertex.degree(); k++)
            {
                writer.StartObject();
                writer.Key("attachmentAtom");
                writer.Int(mol.getRSiteAttachmentPointByOrder(i, k));
                writer.Key("attachmentId");
                writer.Int(k);
                writer.EndObject();
            }
            writer.EndArray();
        }

        if (mol.isTemplateAtom(i))
        {
            auto pclass = mol.getTemplateAtomClass(i);
            if (pclass && strlen(pclass))
            {
                writer.Key("class");
                if (strcasecmp(pclass, kMonomerClassLINKER) == 0)
                    writer.String(kMonomerClassCHEM);
                else
                    writer.String(pclass);
            }

            auto seqid = mol.getTemplateAtomSeqid(i);
            if (seqid != VALUE_UNKNOWN)
            {
                writer.Key("seqid");
                writer.Int(seqid);
            }

            if (mol.template_attachment_points.size())
            {
                if (mol.getTemplateAtomAttachmentPointsCount(i))
                {
                    writer.Key("attOrder");
                    writer.StartArray();
                    for (int j = mol.template_attachment_points.begin(); j != mol.template_attachment_points.end(); j = mol.template_attachment_points.next(j))
                    {
                        BaseMolecule::TemplateAttPoint& ap = mol.template_attachment_points.at(j);
                        if (ap.ap_occur_idx == i)
                        {
                            writer.StartObject();
                            writer.Key("index");
                            writer.Int(ap.ap_aidx);
                            writer.Key("id");
                            writer.String(ap.ap_id.ptr());
                            writer.EndObject();
                        }
                    }
                    writer.EndArray();
                }
            }
        }
        writer.EndObject();
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
