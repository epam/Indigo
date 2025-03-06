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

#include <memory>
#include <set>

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

void mergeMappings(Array<int>& dest, Array<int>& src)
{
    for (int i = 0; i < dest.size(); ++i)
    {
        int atom_idx = dest[i];
        if (atom_idx > -1 && atom_idx < src.size())
            dest[i] = src[atom_idx];
        else
            dest[i] = -1;
    }
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
    : _output(output), _pmol(nullptr), _pqmol(nullptr), add_stereo_desc(false), pretty_json(false), use_native_precision(false)
{
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
    case SGroup::SG_TYPE_COP:
        throw Error("SG_TYPE_COP not implemented in indigo yet");
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
static void saveNativeFloat(JsonWriter& writer, float f_value)
{
    std::string val = std::to_string(f_value);
    writer.RawValue(val.c_str(), val.length(), kStringType);
}

void MoleculeJsonSaver::writeFloat(JsonWriter& writer, float f_value)
{
    if (use_native_precision)
        saveNativeFloat(writer, f_value);
    else
        writer.Double(f_value);
}

void indigo::MoleculeJsonSaver::writePos(JsonWriter& writer, const Vec3f& pos)
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
        writer.StartObject();
        if (mol.attachmentPointCount())
            saveAttachmentPoint(mol, i, writer);
        QS_DEF(Array<int>, rg_list);
        int radical = 0;
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
                radical = mol.getAtomRadical(i);
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

std::string MoleculeJsonSaver::monomerId(const TGroup& tg)
{
    std::string name;
    std::string monomer_class;
    if (tg.tgroup_text_id.ptr())
        return tg.tgroup_text_id.ptr();
    if (tg.tgroup_name.ptr())
        name = tg.tgroup_name.ptr();
    if (tg.tgroup_class.ptr())
        monomer_class = tg.tgroup_class.ptr();
    if (name.size())
        name = monomerNameByAlias(monomer_class, name) + "_" + std::to_string(tg.tgroup_id);
    else
        name = std::string("#") + std::to_string(tg.tgroup_id);
    return name;
}

std::string MoleculeJsonSaver::monomerHELMClass(const std::string& class_name)
{
    if (isAminoAcidClass(class_name))
        return kMonomerClassPEPTIDE;
    if (isNucleicClass(class_name))
        return kMonomerClassRNA;
    return kMonomerClassCHEM;
}

std::string MoleculeJsonSaver::monomerKETClass(const std::string& class_name)
{
    auto mclass = class_name;
    if (class_name == kMonomerClassAA)
        return kMonomerClassAminoAcid;

    if (mclass == kMonomerClassdAA)
        return kMonomerClassDAminoAcid;

    if (mclass == kMonomerClassRNA || mclass == kMonomerClassDNA || mclass.find(kMonomerClassMOD) == 0 || mclass.find(kMonomerClassXLINK) == 0)
        return mclass;

    for (auto it = mclass.begin(); it < mclass.end(); ++it)
        *it = static_cast<char>(it > mclass.begin() ? std::tolower(*it) : std::toupper(*it));

    return mclass;
}

void MoleculeJsonSaver::saveMonomerTemplate(TGroup& tg, JsonWriter& writer)
{
    std::string template_id("monomerTemplate-");
    std::string tg_id(monomerId(tg));
    std::string template_class(monomerKETClass(tg.tgroup_class.ptr()));
    std::string helm_class(monomerHELMClass(tg.tgroup_class.ptr()));
    template_id += tg_id;
    writer.Key(template_id.c_str());
    writer.StartObject();
    writer.Key("type");
    writer.String("monomerTemplate");
    writer.Key("id");
    writer.String(tg_id.c_str());
    if (tg.tgroup_class.size())
    {
        writer.Key("class");
        writer.String(template_class.c_str());
        writer.Key("classHELM");
        writer.String(helm_class.c_str());
    }

    writer.Key("alias");
    writer.String(monomerAlias(tg).c_str());

    if (tg.tgroup_name.size())
    {
        writer.Key("name");
        writer.String(tg.tgroup_name.ptr());
    }

    if (tg.tgroup_full_name.size())
    {
        writer.Key("fullName");
        writer.String(tg.tgroup_full_name.ptr());
    }

    std::string natreplace;
    if (tg.tgroup_natreplace.size() == 0)
    {
        auto alias = monomerAlias(tg);
        if (isBasicAminoAcid(template_class, alias))
        {
            natreplace = alias;
        }
        else if (tg.tgroup_name.size() > 0)
        {
            std::string name = tg.tgroup_name.ptr();
            alias = monomerAliasByName(tg.tgroup_class.ptr(), name);
            if (alias.size() > 0 && alias.size() != name.size())
                natreplace = alias;
        }
    }
    else
        natreplace = tg.tgroup_natreplace.ptr();

    if (natreplace.size())
    {
        auto analog = extractMonomerName(natreplace);
        auto nat_alias = monomerAliasByName(tg.tgroup_class.ptr(), analog);
        writer.Key("naturalAnalogShort");
        writer.String(nat_alias.c_str());
        if (analog.size() > 1)
        {
            writer.Key("naturalAnalog");
            writer.String(analog.c_str());
        }
    }

    if (tg.tgroup_comment.size())
    {
        writer.Key("comment");
        writer.String(tg.tgroup_comment.ptr());
    }

    if (tg.unresolved)
    {
        writer.Key("unresolved");
        writer.Bool(tg.unresolved);

        if (tg.idt_alias.size()) // Save IDT alias only for unresolved
        {
            writer.Key("idtAliases");
            writer.StartObject();
            writer.Key("base");
            writer.String(tg.idt_alias.ptr());
            writer.Key("modifications");
            writer.StartObject();
            writer.Key("endpoint5");
            writer.String(tg.idt_alias.ptr());
            writer.Key("internal");
            writer.String(tg.idt_alias.ptr());
            writer.Key("endpoint3");
            writer.String(tg.idt_alias.ptr());
            writer.EndObject();
            writer.EndObject();
        }
    }

    saveMonomerAttachmentPoints(tg, writer);
    saveFragment(*tg.fragment, writer);
    writer.EndObject();
}

void MoleculeJsonSaver::saveAmbiguousMonomerTemplate(TGroup& tg, JsonWriter& writer)
{
    std::string template_id("ambiguousMonomerTemplate-");
    std::string tg_id(monomerId(tg));
    std::string template_class(monomerKETClass(tg.tgroup_class.ptr()));
    std::string helm_class(monomerHELMClass(tg.tgroup_class.ptr()));
    template_id += tg_id;
    writer.Key(template_id.c_str());
    writer.StartObject();
    writer.Key("type");
    writer.String("ambiguousMonomerTemplate");
    writer.Key("subtype");
    writer.String(tg.mixture ? "mixture" : "alternatives");
    writer.Key("id");
    writer.String(tg_id.c_str());
    writer.Key("alias");
    writer.String(tg.tgroup_alias.ptr());
    writer.Key("options");
    writer.StartArray();
    const char* num_name = tg.mixture ? "ratio" : "probability";
    for (int i = 0; i < tg.aliases.size(); i++)
    {
        writer.StartObject();
        writer.Key("templateId");
        writer.String(tg.aliases[i].ptr());
        writer.EndObject();
        if (tg.ratios[i] >= 0)
        {
            writer.Key(num_name);
            saveNativeFloat(writer, tg.ratios[i]);
        }
    }
    writer.EndArray();
    writer.EndObject();
}

void MoleculeJsonSaver::saveSuperatomAttachmentPoints(Superatom& sa, JsonWriter& writer)
{
    std::map<std::string, int> sorted_attachment_points;
    if (sa.attachment_points.size())
    {
        for (int i = sa.attachment_points.begin(); i != sa.attachment_points.end(); i = sa.attachment_points.next(i))
        {
            auto& atp = sa.attachment_points[i];
            std::string atp_id_str(atp.apid.ptr());
            if (atp_id_str.size())
                sorted_attachment_points.insert(std::make_pair(atp_id_str, i));
        }

        if (sorted_attachment_points.size())
        {
            writer.Key("attachmentPoints");
            writer.StartArray();
            int order = 0;
            for (const auto& kvp : sorted_attachment_points)
            {
                writer.StartObject();
                auto& atp = sa.attachment_points[kvp.second];
                std::string atp_id_str(atp.apid.ptr());
                if (!isAttachmentPointsInOrder(order++, atp_id_str))
                {
                    if (atp_id_str.size())
                    {
                        writer.Key("id");
                        writer.String(atp_id_str.c_str());
                    }
                    writer.Key("type");
                    if (atp_id_str == kLeftAttachmentPoint || atp_id_str == kAttachmentPointR1)
                        writer.String("left");
                    else if (atp_id_str == kRightAttachmentPoint || atp_id_str == kAttachmentPointR2)
                        writer.String("right");
                    else
                        writer.String("side");
                    writer.Key("label");
                    writer.String(convertAPToHELM(atp_id_str).c_str());
                }
                writer.Key("attachmentAtom");
                writer.Int(atp.aidx);
                if (atp.lvidx >= 0)
                {
                    writer.Key("leavingGroup");
                    writer.StartObject();
                    writer.Key("atoms");
                    writer.StartArray();
                    writer.Int(atp.lvidx);
                    writer.EndArray();
                    writer.EndObject(); // leavingGroup
                }
                writer.EndObject(); // attachmentAtom
            }
            writer.EndArray();
        }
    }
}

void MoleculeJsonSaver::saveMonomerAttachmentPoints(TGroup& tg, JsonWriter& writer)
{
    auto& sgroups = tg.fragment->sgroups;
    for (int j = sgroups.begin(); j != sgroups.end(); j = sgroups.next(j))
    {
        SGroup& sg = sgroups.getSGroup(j);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            saveSuperatomAttachmentPoints((Superatom&)sg, writer);
            sgroups.remove(j);
        }
    }
}
void MoleculeJsonSaver::saveRGroup(RGroup& rgroup, int rgnum, JsonWriter& writer)
{
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);

    if (rgroup.fragments.size() == 0 && rgroup.occurrence.size() == 0 && rgroup.if_then <= 0 && !rgroup.rest_h)
        return;

    buf.clear();
    out.printf("rg%d", rgnum);
    buf.push(0);

    writer.Key(buf.ptr());
    writer.StartObject();
    writer.Key("rlogic");
    writer.StartObject();
    writer.Key("number");
    writer.Int(rgnum);
    if (rgroup.occurrence.size() > 0)
    {
        buf.clear();
        rgroup.writeOccurrence(out);
        out.writeChar(0);
        writer.Key("range");
        writer.String(buf.ptr());
    }
    if (rgroup.if_then > 0)
    {
        writer.Key("ifthen");
        writer.Int(rgroup.if_then);
    }
    if (rgroup.rest_h)
    {
        writer.Key("resth");
        writer.Bool(rgroup.rest_h);
    }
    writer.EndObject(); // rlogic
    writer.Key("type");
    writer.String("rgroup");

    bool fmode = rgroup.fragments.size() > 1;
    if (fmode)
    {
        writer.Key("fragments");
        writer.StartArray();
    }

    for (int i = rgroup.fragments.begin(); i != rgroup.fragments.end(); i = rgroup.fragments.next(i))
    {
        if (fmode)
            writer.StartObject();
        saveFragment(*rgroup.fragments[i], writer);
        if (fmode)
            writer.EndObject();
    }

    if (fmode)
        writer.EndArray();

    writer.EndObject();
}

bool MoleculeJsonSaver::_checkAttPointOrder(BaseMolecule& mol, int rsite)
{
    const Vertex& vertex = mol.getVertex(rsite);
    for (int i = 0; i < vertex.degree() - 1; i++)
    {
        int cur = mol.getRSiteAttachmentPointByOrder(rsite, i);
        int next = mol.getRSiteAttachmentPointByOrder(rsite, i + 1);

        if (cur == VALUE_UNKNOWN || next == VALUE_UNKNOWN)
            return true; // here we treat "undefined" as "ok"

        if (cur > next)
            return false;
    }

    return true;
}

int MoleculeJsonSaver::getMonomerNumber(int mon_idx)
{
    auto mon_it = _monomers_enum.find(mon_idx);
    if (mon_it != _monomers_enum.end())
        return mon_it->second;
    else
        throw Error("Monomer index: %d not found", mon_idx);
    return -1;
}

void MoleculeJsonSaver::saveEndpoint(BaseMolecule& mol, const std::string& ep, int beg_idx, int end_idx, JsonWriter& writer, bool hydrogen)
{
    writer.Key(ep.c_str());
    writer.StartObject();
    if (mol.isTemplateAtom(beg_idx))
    {
        writer.Key("monomerId");
        writer.String((std::string("monomer") + std::to_string(getMonomerNumber(beg_idx))).c_str());
        auto conn_it = _monomer_connections.find(std::make_pair(beg_idx, end_idx));
        if (conn_it != _monomer_connections.end())
        {
            writer.Key("attachmentPointId");
            writer.String(convertAPToHELM(conn_it->second).c_str());
        }
        else if (!hydrogen) // Hydrogen connection has no attachment point
            throw Error("Attachment point not found!!!");
    }
    else
    {
        auto atom_mol_it = _atom_to_mol_id.find(beg_idx);
        if (atom_mol_it != _atom_to_mol_id.end())
        {
            int mol_id = atom_mol_it->second;
            writer.Key("moleculeId");
            writer.String((std::string("mol") + std::to_string(mol_id)).c_str());
            writer.Key("atomId");
            writer.String(std::to_string(_mappings[mol_id][beg_idx]).c_str());
        }
        else
            throw Error("Atom %d not found", beg_idx);
    }
    writer.EndObject();
}

void MoleculeJsonSaver::saveMoleculeReference(int mol_id, JsonWriter& writer)
{
    writer.StartObject();
    writer.Key("$ref");
    std::string mol_node = std::string("mol") + std::to_string(mol_id);
    writer.String(mol_node.c_str());
    writer.EndObject();
    auto& mapping = _mappings[mol_id];
    // printf("mol id:%d\n", mol_id);
    for (auto atom_idx = 0; atom_idx < mapping.size(); ++atom_idx)
    {
        if (mapping[atom_idx] > -1)
        {
            // printf("%d ", atom_idx);
            _atom_to_mol_id.emplace(atom_idx, mol_id);
        }
    }
    // printf("\n");
}

void MoleculeJsonSaver::saveRoot(BaseMolecule& mol, JsonWriter& writer)
{
    _no_template_molecules.clear();
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    writer.StartObject();
    writer.Key("root");
    writer.StartObject();
    writer.Key("nodes");
    writer.StartArray();

    getSGroupAtoms(mol, _s_neighbors);
    // save mol references
    // int mol_id = 0;
    for (int idx = 0; idx < mol.countComponents(_s_neighbors); ++idx)
    {
        Filter filt(mol.getDecomposition().ptr(), Filter::EQ, idx);
        std::unique_ptr<BaseMolecule> component(mol.neu());
        Array<int> mapping, inv_mapping;
        component->makeSubmolecule(mol, filt, &mapping, &inv_mapping);
        if (!component->countTemplateAtoms())
        {
            _no_template_molecules.emplace_back(std::move(component));
            _mappings.push().copy(inv_mapping);
            saveMoleculeReference((int)_no_template_molecules.size() - 1, writer);
        }
        else
        {
            // collect non-template atoms
            Array<int> vertices;
            for (int atom_idx = component->vertexBegin(); atom_idx != component->vertexEnd(); atom_idx = component->vertexNext(atom_idx))
            {
                if (!component->isTemplateAtom(atom_idx))
                    vertices.push(atom_idx);
            }

            if (vertices.size())
            {
                Array<int> sub_mapping;
                std::unique_ptr<BaseMolecule> sub_mol(component->neu());
                sub_mol->makeSubmolecule(*component, vertices, &sub_mapping);
                mergeMappings(inv_mapping, sub_mapping);
                for (int sub_idx = 0; sub_idx < sub_mol->countComponents(); ++sub_idx)
                {
                    Array<int> sub_comp_mapping, mapping_cp, inv_sub_comp_mapping;
                    mapping_cp.copy(inv_mapping);
                    Filter filter(sub_mol->getDecomposition().ptr(), Filter::EQ, sub_idx);
                    std::unique_ptr<BaseMolecule> sub_mol_component(sub_mol->neu());
                    sub_mol_component->makeSubmolecule(*sub_mol, filter, &sub_comp_mapping, &inv_sub_comp_mapping);
                    _no_template_molecules.emplace_back(std::move(sub_mol_component));
                    mergeMappings(mapping_cp, inv_sub_comp_mapping);
                    _mappings.push().copy(mapping_cp);
                    saveMoleculeReference(static_cast<int>(_no_template_molecules.size()) - 1, writer);
                }
            }
        }
    }

    // save meta data
    saveMetaData(writer, mol.meta());

    // save rgroups
    for (int i = 1; i <= mol.rgroups.getRGroupCount(); ++i)
    {
        RGroup& rgroup = mol.rgroups.getRGroup(i);
        if (rgroup.fragments.size() == 0)
            continue;

        buf.clear();
        out.printf("rg%d", i);
        buf.push(0);
        writer.StartObject();
        writer.Key("$ref");
        writer.String(buf.ptr());
        writer.EndObject();
    }

    int mon_idx = 0;

    // save references to monomer's instances
    for (auto i : mol.vertices())
    {
        if (mol.isTemplateAtom(i))
        {
            writer.StartObject();
            writer.Key("$ref");
            writer.String((std::string("monomer") + std::to_string(mon_idx)).c_str());
            writer.EndObject();
            _monomers_enum.emplace(i, mon_idx++);
        }
    }

    // save references to monomer shapes
    for (int shape_idx = 0; shape_idx < mol.monomer_shapes.size(); ++shape_idx)
    {
        writer.StartObject();
        writer.Key("$ref");
        writer.String((KetMonomerShape::ref_prefix + std::to_string(shape_idx)).c_str());
        writer.EndObject();
    }

    writer.EndArray(); // nodes

    // save connections and templates
    if (mol.tgroups.getTGroupCount())
    {
        // collect attachment points into unordered map <key, val>. key - pair of from and destination atom. val - attachment point name.
        _monomer_connections.clear();
        for (int i = mol.template_attachment_points.begin(); i != mol.template_attachment_points.end(); i = mol.template_attachment_points.next(i))
        {
            auto& sap = mol.template_attachment_points.at(i);
            _monomer_connections.emplace(std::make_pair(sap.ap_occur_idx, sap.ap_aidx), sap.ap_id.ptr());
        }

        // save connections
        writer.Key("connections");
        writer.StartArray();
        for (auto i : mol.edges())
        {
            auto& e = mol.getEdge(i);
            if (mol.isTemplateAtom(e.beg) || mol.isTemplateAtom(e.end))
            {
                // save connections between templates or atoms
                writer.StartObject();
                writer.Key("connectionType");
                bool hydrogen = mol.getBondOrder(i) == _BOND_HYDROGEN;
                writer.String(hydrogen ? "hydrogen" : "single");
                // save endpoints
                saveEndpoint(mol, "endpoint1", e.beg, e.end, writer, hydrogen);
                saveEndpoint(mol, "endpoint2", e.end, e.beg, writer, hydrogen);
                writer.EndObject(); // connection
            }
        }
        writer.EndArray(); // connections
        writer.Key("templates");
        writer.StartArray();

        for (int i = mol.tgroups.begin(); i != mol.tgroups.end(); i = mol.tgroups.next(i))
        {
            TGroup& tg = mol.tgroups.getTGroup(i);
            auto template_name = std::string(tg.ambiguous ? "ambiguousMonomerTemplate-" : "monomerTemplate-") + monomerId(tg);
            writer.StartObject();
            writer.Key("$ref");
            writer.String(template_name.c_str());
            writer.EndObject();
        }

        writer.EndArray(); // templates
    }
    writer.EndObject(); // root
}

void MoleculeJsonSaver::saveMolecule(BaseMolecule& bmol, JsonWriter& writer)
{
    if (add_stereo_desc)
        bmol.addCIP();

    std::unique_ptr<BaseMolecule> mol(bmol.neu());
    mol->clone_KeepIndices(bmol);

    if (!BaseMolecule::hasCoord(*mol))
    {
        MoleculeLayout ml(*mol, false);
        ml.layout_orientation = UNCPECIFIED;
        ml.make();
    }
    BaseMolecule::collapse(*mol);

    mol->getTemplatesMap(_templates);

    // save root elements
    saveRoot(*mol, writer);

    // save monomers
    if (mol->tgroups.getTGroupCount())
        for (auto i : mol->vertices())
        {
            if (mol->isTemplateAtom(i))
            {
                int mon_id = getMonomerNumber(i);
                writer.Key((std::string("monomer") + std::to_string(mon_id)).c_str());
                writer.StartObject();
                writer.Key("type");
                int temp_idx = mol->getTemplateAtomTemplateIndex(i);
                writer.String(temp_idx > -1 && bmol.tgroups.getTGroup(temp_idx).ambiguous ? "ambiguousMonomer" : "monomer");
                writer.Key("id");
                writer.String(std::to_string(mon_id).c_str());
                auto seqid = mol->getTemplateAtomSeqid(i);
                if (seqid != VALUE_UNKNOWN)
                {
                    writer.Key("seqid");
                    writer.Int(seqid);
                }
                // location
                writer.Key("position");
                const auto& pos = mol->getAtomXyz(i);
                writer.StartObject();
                writer.Key("x");
                writeFloat(writer, pos.x);
                writer.Key("y");
                writeFloat(writer, pos.y);
                writer.EndObject(); // pos

                auto display = mol->getTemplateAtomDisplayOption(i);
                if (display != DisplayOption::Undefined)
                {
                    writer.Key("expanded");
                    writer.Bool(display == DisplayOption::Expanded);
                }

                // find template
                writer.Key("alias");
                auto alias = mol->getTemplateAtom(i);
                writer.String(alias);
                auto mon_class = mol->getTemplateAtomClass(i);
                if (temp_idx > -1)
                {
                    auto& tg = bmol.tgroups.getTGroup(temp_idx);
                    writer.Key("templateId");
                    writer.String(monomerId(tg).c_str());
                }
                else
                {
                    auto tg_ref = findTemplateInMap(alias, mon_class, _templates);
                    if (tg_ref.has_value())
                    {
                        writer.Key("templateId");
                        writer.String(monomerId(tg_ref.value().get()).c_str());
                    }
                }
                writer.EndObject(); // monomer
            }
            else
            {
                //
            }
        }

    // save templates
    for (int i = mol->tgroups.begin(); i != mol->tgroups.end(); i = mol->tgroups.next(i))
    {
        TGroup& tg = mol->tgroups.getTGroup(i);
        if (tg.ambiguous)
            saveAmbiguousMonomerTemplate(tg, writer);
        else
            saveMonomerTemplate(tg, writer);
    }

    // save molecules
    for (int i = 0; i < static_cast<int>(_no_template_molecules.size()); ++i)
    {
        auto& component = _no_template_molecules[i];
        if (component->vertexCount())
        {
            std::string mol_node = std::string("mol") + std::to_string(i);
            writer.Key(mol_node.c_str());
            writer.StartObject();
            writer.Key("type");
            writer.String("molecule");
            saveFragment(*component, writer);
            // TODO: the code below needs refactoring
            Vec3f flag_pos;
            if (bmol.getStereoFlagPosition(i, flag_pos))
            {
                writer.Key("stereoFlagPosition");
                writer.StartObject();
                writer.Key("x");
                writeFloat(writer, flag_pos.x);
                writer.Key("y");
                writeFloat(writer, flag_pos.y);
                writer.Key("z");
                writeFloat(writer, flag_pos.z);
                writer.EndObject();
            }
            writer.EndObject();
        }
    }

    // save R-Groups
    for (int i = 1; i <= mol->rgroups.getRGroupCount(); i++)
    {
        saveRGroup(mol->rgroups.getRGroup(i), i, writer);
    }

    // save monomer shapes
    for (int shape_idx = 0; shape_idx < mol->monomer_shapes.size(); ++shape_idx)
    {
        auto& monomer_shape = *mol->monomer_shapes[shape_idx];
        writer.Key((KetMonomerShape::ref_prefix + std::to_string(shape_idx)).c_str());
        writer.StartObject();
        writer.Key("type");
        writer.String("monomerShape");
        writer.Key("id");
        writer.String(monomer_shape.id());
        writer.Key("collapsed");
        writer.Bool(monomer_shape.collapsed());
        writer.Key("shape");
        writer.String(KetMonomerShape::shapeTypeToStr(monomer_shape.shape()).c_str());
        writer.Key("position");
        Vec2f pos = monomer_shape.position();
        writer.StartObject();
        writer.Key("x");
        saveNativeFloat(writer, pos.x);
        writer.Key("y");
        saveNativeFloat(writer, pos.y);
        writer.EndObject();
        writer.Key("monomers");
        writer.StartArray();
        for (auto& monomer_id : monomer_shape.monomers())
            writer.String(monomer_id);
        writer.EndArray();
        writer.EndObject();
    }

    writer.EndObject();
}

void MoleculeJsonSaver::saveFragment(BaseMolecule& fragment, JsonWriter& writer)
{
    _pmol = nullptr;
    _pqmol = nullptr;
    if (fragment.isQueryMolecule())
        _pqmol = &fragment.asQueryMolecule();
    else
        _pmol = &fragment.asMolecule();

    if (_pmol)
        _pmol->setIgnoreBadValenceFlag(true);

    writer.Key("atoms");
    writer.StartArray();
    saveAtoms(fragment, writer);
    writer.EndArray();

    writer.Key("bonds");
    writer.StartArray();
    saveBonds(fragment, writer);
    writer.EndArray();

    saveSGroups(fragment, writer);
    saveHighlights(fragment, writer);
    if (fragment.properties().size())
    {
        auto& props = fragment.properties().value(0);
        writer.Key("properties");
        writer.StartArray();
        for (auto it = props.elements().begin(); it != props.elements().end(); ++it)
        {
            writer.StartObject();
            writer.Key("key");
            writer.String(props.key(*it));
            writer.Key("value");
            writer.String(props.value(*it));
            writer.EndObject();
        }
        writer.EndArray();
    }
}

void MoleculeJsonSaver::saveMolecule(BaseMolecule& bmol)
{
    StringBuffer s;
    JsonWriter writer(pretty_json);
    writer.Reset(s);
    saveMolecule(bmol, writer);
    std::stringstream result;
    result << s.GetString();
    _output.printf("%s", result.str().c_str());
}

void MoleculeJsonSaver::saveMetaData(JsonWriter& writer, MetaDataStorage& meta)
{
    static const std::unordered_map<int, std::string> _arrow_type2string = {
        {ReactionComponent::ARROW_BASIC, "open-angle"},
        {ReactionComponent::ARROW_FILLED_TRIANGLE, "filled-triangle"},
        {ReactionComponent::ARROW_FILLED_BOW, "filled-bow"},
        {ReactionComponent::ARROW_DASHED, "dashed-open-angle"},
        {ReactionComponent::ARROW_FAILED, "failed"},
        {ReactionComponent::ARROW_BOTH_ENDS_FILLED_TRIANGLE, "both-ends-filled-triangle"},
        {ReactionComponent::ARROW_EQUILIBRIUM_FILLED_HALF_BOW, "equilibrium-filled-half-bow"},
        {ReactionComponent::ARROW_EQUILIBRIUM_FILLED_TRIANGLE, "equilibrium-filled-triangle"},
        {ReactionComponent::ARROW_EQUILIBRIUM_OPEN_ANGLE, "equilibrium-open-angle"},
        {ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_FILLED_HALF_BOW, "unbalanced-equilibrium-filled-half-bow"},
        {ReactionComponent::ARROW_UNBALANCED_EQUILIBRIUM_LARGE_FILLED_HALF_BOW, "unbalanced-equilibrium-large-filled-half-bow"},
        {ReactionComponent::ARROW_BOTH_ENDS_FILLED_TRIANGLE, "unbalanced-equilibrium-filled-half-triangle"},
        {ReactionComponent::ARROW_RETROSYNTHETIC, "retrosynthetic"}};

    const auto& meta_objects = meta.metaData();
    for (int meta_index = 0; meta_index < meta_objects.size(); ++meta_index)
    {
        auto pobj = meta_objects[meta_index];
        switch (pobj->_class_id)
        {
        case ReactionArrowObject::CID: {
            ReactionArrowObject& ar = (ReactionArrowObject&)(*pobj);
            writer.StartObject();
            writer.Key("type");
            writer.String("arrow");
            writer.Key("data");
            writer.StartObject();
            // arrow mode
            writer.Key("mode");
            std::string arrow_mode = "open-angle";
            auto at_it = _arrow_type2string.find(ar.getArrowType());
            if (at_it != _arrow_type2string.end())
                arrow_mode = at_it->second.c_str();
            writer.String(arrow_mode.c_str());

            // arrow coordinates
            writer.Key("pos");
            writer.StartArray();
            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, ar.getTail().x);
            writer.Key("y");
            writeFloat(writer, ar.getTail().y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, ar.getHead().x);
            writer.Key("y");
            writeFloat(writer, ar.getHead().y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            writer.EndArray();  // arrow coordinates
            writer.EndObject(); // end data
            writer.EndObject(); // end node
        }
        break;
        case ReactionMultitailArrowObject::CID: {
            ReactionMultitailArrowObject& ar = (ReactionMultitailArrowObject&)(*pobj);
            writer.StartObject();
            writer.Key("type");
            writer.String("multi-tailed-arrow");
            writer.Key("data");
            writer.StartObject();

            writer.Key("head");
            writer.StartObject();
            writer.Key("position");
            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, ar.getHead().x);
            writer.Key("y");
            writeFloat(writer, ar.getHead().y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();
            writer.EndObject();

            writer.Key("spine");
            writer.StartObject();
            writer.Key("pos");
            writer.StartArray();

            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, ar.getSpineBegin().x);
            writer.Key("y");
            writeFloat(writer, ar.getSpineBegin().y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, ar.getSpineEnd().x);
            writer.Key("y");
            writeFloat(writer, ar.getSpineEnd().y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            writer.EndArray();
            writer.EndObject();

            writer.Key("tails");
            writer.StartObject();
            writer.Key("pos");
            writer.StartArray();

            for (auto& t : ar.getTails())
            {
                writer.StartObject();
                writer.Key("x");
                writeFloat(writer, t.x);
                writer.Key("y");
                writeFloat(writer, t.y);
                writer.Key("z");
                writer.Double(0);
                writer.EndObject();
            }

            writer.EndArray();
            writer.EndObject();

            writer.Key("zOrder");
            writer.Int(0);

            writer.EndObject();
            writer.EndObject();
        }
        break;
        case ReactionPlusObject::CID: {
            ReactionPlusObject& rp = (ReactionPlusObject&)(*pobj);
            writer.StartObject();
            writer.Key("type");
            writer.String("plus");
            writer.Key("location");
            writer.StartArray();
            writeFloat(writer, rp.getPos().x);
            writeFloat(writer, rp.getPos().y);
            writer.Double(0);
            writer.EndArray();
            writer.EndObject();
        }
        break;
        case SimpleGraphicsObject::CID: {
            auto simple_obj = (SimpleGraphicsObject*)pobj;
            writer.StartObject();
            writer.Key("type");
            writer.String("simpleObject");
            writer.Key("data");
            writer.StartObject();
            writer.Key("mode");
            switch (simple_obj->_mode)
            {
            case SimpleGraphicsObject::EEllipse:
                writer.String("ellipse");
                break;
            case SimpleGraphicsObject::ERectangle:
                writer.String("rectangle");
                break;
            case SimpleGraphicsObject::ELine:
                writer.String("line");
                break;
            }
            writer.Key("pos");
            writer.StartArray();

            auto& coords = simple_obj->_coordinates;

            // point1
            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, coords.first.x);
            writer.Key("y");
            writeFloat(writer, coords.first.y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            // point2
            writer.StartObject();
            writer.Key("x");
            writeFloat(writer, coords.second.x);
            writer.Key("y");
            writeFloat(writer, coords.second.y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            writer.EndArray();

            // end data
            writer.EndObject();
            // end node
            writer.EndObject();
            break;
        }
        case SimpleTextObject::CID: {
            auto simple_obj = (SimpleTextObject*)pobj;
            writer.StartObject();
            writer.Key("type");
            writer.String("text");
            writer.Key("data");
            writer.StartObject();
            writer.Key("content");
            writer.String(simple_obj->_content.c_str());
            writer.Key("position");
            writePos(writer, simple_obj->_pos);

            writer.Key("pos");
            writer.StartArray();
            Vec2f pos_bbox(simple_obj->_pos.x, simple_obj->_pos.y);
            writePos(writer, pos_bbox);
            pos_bbox.y -= simple_obj->_size.y;
            writePos(writer, pos_bbox);
            pos_bbox.x += simple_obj->_size.x;
            writePos(writer, pos_bbox);
            pos_bbox.y += simple_obj->_size.y;
            writePos(writer, pos_bbox);
            writer.EndArray();
            writer.EndObject(); // end data
            writer.EndObject(); // end node
            break;
        }
        case EmbeddedImageObject::CID: {
            auto image_obj = static_cast<const EmbeddedImageObject*>(pobj);
            auto& bbox = image_obj->getBoundingBox();
            writer.StartObject(); // start node
            writer.Key("type");
            writer.String("image");
            writer.Key("format");
            switch (image_obj->getFormat())
            {
            case EmbeddedImageObject::EKETPNG:
                writer.String(KImagePNG);
                break;
            case EmbeddedImageObject::EKETSVG:
                writer.String(KImageSVG);
                break;
            default:
                throw Exception("Bad image format: %d", image_obj->getFormat());
            }

            writer.Key("boundingBox");

            writer.StartObject(); // start bbox
            writer.Key("x");
            writeFloat(writer, bbox.left());
            writer.Key("y");
            writeFloat(writer, bbox.top());
            writer.Key("z");
            writer.Double(0);

            writer.Key("width");
            writeFloat(writer, bbox.width());
            writer.Key("height");
            writeFloat(writer, bbox.height());
            writer.EndObject(); // end bbox

            writer.Key("data");
            writer.String(image_obj->getBase64().c_str());
            writer.EndObject(); // end node
            break;
        }
        }
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif