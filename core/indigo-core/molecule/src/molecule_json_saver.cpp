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
#include "molecule/ket_commons.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cip_calculator.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/molecule_savers.h"
#include "molecule/query_molecule.h"

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

MoleculeJsonSaver::MoleculeJsonSaver(Output& output) : _output(output), _pmol(nullptr), _pqmol(nullptr), add_stereo_desc(false), pretty_json(false)
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
            if (orig_ids.find(sgroup.parent_group) == -1 || sgroup.parent_group == sgroup.original_group)
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

            if (added_ids.find(sgroup.original_group) != -1)
                continue;

            if (added_ids.find(sgroup.parent_group) != -1)
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
    if (mol.countSGroups() > 0)
    {
        writer.Key("sgroups");
        writer.StartArray();
        int idx = 1;
        for (int i = 0; i < sgs_sorted.size(); i++)
        {
            int sg_idx = sgs_sorted[i];
            auto& sgrp = mol.sgroups.getSGroup(sg_idx);
            saveSGroup(sgrp, writer);
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
        writer.Double(dsg.display_pos.x);
        writer.Key("y");
        writer.Double(dsg.display_pos.y);

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
        writer.String(sa.subscript.ptr());
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
                std::string atp_id_str(atp.apid.ptr(), atp.apid.size());
                writer.Key("attachmentAtom");
                writer.Int(atp.aidx);
                if (atp.lvidx != -1)
                {
                    writer.Key("leavingAtom");
                    writer.Int(atp.lvidx);
                }
                if (atp_id_str.length() > 0)
                {
                    writer.Key("attachmentId");
                    writer.String(atp_id_str.c_str());
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
            writer.StartObject();
            writer.Key("type");
            int bond_order = mol.getBondOrder(i);
            if (bond_order < 0 && _pqmol)
            {
                int qb = QueryMolecule::getQueryBondType(_pqmol->getBond(i));
                if (qb == QueryMolecule::QUERY_BOND_SINGLE_OR_DOUBLE)
                    bond_order = 5;
                else if (qb == QueryMolecule::QUERY_BOND_SINGLE_OR_AROMATIC)
                    bond_order = 6;
                else if (qb == QueryMolecule::QUERY_BOND_DOUBLE_OR_AROMATIC)
                    bond_order = 7;
                else if (qb == QueryMolecule::QUERY_BOND_ANY)
                    bond_order = 8;
                if (bond_order < 0)
                    throw Error("Invalid query bond");
            }

            if (bond_order == BOND_ZERO && _pmol)
            {
                bond_order = 9;
                const Edge& edge = mol.getEdge(i);
                if ((_pmol->getAtomNumber(edge.beg) == ELEM_H) || (_pmol->getAtomNumber(edge.end) == ELEM_H))
                    bond_order = 10;
            }

            writer.Uint(bond_order);

            int topology = -1;
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
                    writer.Uint(rcenter);
                }
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
                    stereo = 1;
                    break;
                case BOND_EITHER:
                    stereo = 4;
                    break;
                case BOND_DOWN:
                    stereo = 6;
                    break;
                default: {
                    stereo = 0;
                }
                break;
                }

            if (stereo)
            {
                writer.Key("stereo");
                writer.Uint(stereo);
            }

            auto cip = mol.getBondCIP(i);
            if (cip != CIPDesc::NONE)
            {
                auto cip_it = KCIPToString.find((int)cip);
                if (cip_it != KCIPToString.end())
                {
                    writer.Key("cip");
                    writer.String(cip_it->second.c_str());
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
        for (int j = 0; mol.getAttachmentPoint(idx, j) != -1; j++)
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
        if (prm == -1 && i == 3)
            prm = atom_idx;
        writer.Int(prm);
    }
    writer.EndArray();
}

void MoleculeJsonSaver::saveHighlights(BaseMolecule& mol, JsonWriter& writer)
{
    int ca = mol.countSelectedAtoms();
    int cb = mol.countSelectedBonds();
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

void MoleculeJsonSaver::saveSelection(BaseMolecule& mol, JsonWriter& writer)
{
    int ca = mol.countSelectedAtoms();
    int cb = mol.countSelectedBonds();
    if (ca || cb)
    {
        writer.Key("selection");
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
                if (mol.isAtomSelected(i))
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
                if (mol.isBondSelected(i))
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
        if (mol.isRSite(i))
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
            int query_atom_type = -1;
            bool is_qatom_list = false;
            QS_DEF(Array<int>, qatom_list);
            if (mol.isPseudoAtom(i))
            {
                buf.readString(mol.getPseudoAtom(i), true);
            }
            else if (mol.isTemplateAtom(i))
            {
                buf.readString(mol.getTemplateAtom(i), true);
            }
            else if (anum != -1)
            {
                buf.readString(Element::toString(anum), true);
                radical = mol.getAtomRadical(i);
                if (anum == ELEM_H)
                {
                    if (isotope == 2)
                    {
                        buf.clear();
                        buf.appendString("D", true);
                    }
                    if (isotope == 3)
                    {
                        buf.clear();
                        buf.appendString("T", true);
                    }
                }
            }
            else if (_pqmol && (query_atom_type = QueryMolecule::parseQueryAtom(*_pqmol, i, qatom_list)) != -1)
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
                    for (int k = 0; k < qatom_list.size(); k++)
                        writer.String(Element::toString(qatom_list[k]));
                    writer.EndArray();
                }
                else
                    QueryMolecule::getQueryAtomLabel(query_atom_type, buf);
            }

            if (!is_qatom_list)
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

        const Vec3f& coord = mol.getAtomXyz(i);
        writer.Key("location");
        writer.StartArray();
        writer.Double(coord.x);
        writer.Double(coord.y);
        writer.Double(coord.z);
        writer.EndArray();

        int charge = mol.getAtomCharge(i);
        int evalence = mol.getExplicitValence(i);
        int mapping = mol.reaction_atom_mapping[i];
        int inv_ret = mol.reaction_atom_inversion[i];
        bool ecflag = mol.reaction_atom_exact_change[i];
        int hcount = MoleculeSavers::getHCount(mol, i, anum, charge);

        if (_pqmol)
        {
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

            if (hcount == -1)
                hcount = 0;
            else
                hcount++;
            if (hcount > 0)
            {
                writer.Key("hCount");
                writer.Int(hcount);
            }
        }
        else
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
            auto cip_it = KCIPToString.find((int)cip);
            if (cip_it != KCIPToString.end())
            {
                writer.Key("cip");
                writer.String(cip_it->second.c_str());
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
            if (seqid != -1)
            {
                writer.Key("seqid");
                writer.Int(seqid);
            }

            if (mol.template_attachment_points.size())
            {
                int ap_count = 0;
                for (int j = mol.template_attachment_points.begin(); j != mol.template_attachment_points.end(); j = mol.template_attachment_points.next(j))
                {
                    BaseMolecule::TemplateAttPoint& ap = mol.template_attachment_points.at(j);
                    if (ap.ap_occur_idx == i)
                        ap_count++;
                }
                if (ap_count)
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

void MoleculeJsonSaver::saveTGroup(TGroup& tg, JsonWriter& writer)
{
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    buf.clear();
    out.printf("tg%d", tg.tgroup_id);
    buf.push(0);
    writer.Key(buf.ptr());
    writer.StartObject();
    writer.Key("type");
    writer.String("tgroup");
    writer.Key("name");
    writer.String(tg.tgroup_name.ptr());
    if (tg.tgroup_class.size())
    {
        writer.Key("class");
        writer.String(tg.tgroup_class.ptr());
    }
    if (tg.tgroup_alias.size())
    {
        writer.Key("alias");
        writer.String(tg.tgroup_alias.ptr());
    }
    if (tg.tgroup_natreplace.size())
    {
        writer.Key("natreplace");
        writer.String(tg.tgroup_natreplace.ptr());
    }
    if (tg.tgroup_comment.size())
    {
        writer.Key("comment");
        writer.String(tg.tgroup_comment.ptr());
    }
    saveFragment(*tg.fragment, writer);
    writer.EndObject();
}

void MoleculeJsonSaver::saveRGroup(PtrPool<BaseMolecule>& fragments, int rgnum, JsonWriter& writer)
{
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);

    buf.clear();
    out.printf("rg%d", rgnum);
    buf.push(0);

    writer.Key(buf.ptr());
    writer.StartObject();
    writer.Key("rlogic");
    writer.StartObject();
    writer.Key("number");
    writer.Int(rgnum);
    writer.EndObject(); // rlogic
    writer.Key("type");
    writer.String("rgroup");

    bool fmode = fragments.size() > 1;
    if (fmode)
    {
        writer.Key("fragments");
        writer.StartArray();
    }

    for (int i = fragments.begin(); i != fragments.end(); i = fragments.next(i))
    {
        if (fmode)
            writer.StartObject();
        saveFragment(*fragments[i], writer);
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

        if (cur == -1 || next == -1)
            return true; // here we treat "undefined" as "ok"

        if (cur > next)
            return false;
    }

    return true;
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

    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    writer.StartObject();

    writer.Key("root");
    writer.StartObject();
    writer.Key("nodes");
    writer.StartArray();

    std::list<std::unordered_set<int>> s_neighbors;
    getSGroupAtoms(*mol, s_neighbors);
    for (int idx = 0; idx < mol->countComponents(s_neighbors); ++idx)
    {
        writer.StartObject();
        writer.Key("$ref");
        std::string mol_node = std::string("mol") + std::to_string(idx);
        writer.String(mol_node.c_str());
        writer.EndObject();
    }

    saveMetaData(writer, mol->meta());

    int n_rgroups = mol->rgroups.getRGroupCount();
    for (int i = 1; i <= n_rgroups; ++i)
    {
        RGroup& rgroup = mol->rgroups.getRGroup(i);
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

    for (int i = mol->tgroups.begin(); i != mol->tgroups.end(); i = mol->tgroups.next(i))
    {
        TGroup& tg = mol->tgroups.getTGroup(i);
        buf.clear();
        out.printf("tg%d", i);
        buf.push(0);
        writer.StartObject();
        writer.Key("$ref");
        writer.String(buf.ptr());
        writer.EndObject();
    }

    writer.EndArray();  // nodes
    writer.EndObject(); // root

    for (int idx = 0; idx < mol->countComponents(s_neighbors); idx++)
    {
        _pmol = nullptr;
        _pqmol = nullptr;
        Filter filt(mol->getDecomposition().ptr(), Filter::EQ, idx);
        std::unique_ptr<BaseMolecule> component(mol->neu());
        component->makeSubmolecule(*mol, filt, NULL, NULL);

        if (component->isQueryMolecule())
            _pqmol = &component->asQueryMolecule();
        else
            _pmol = &component->asMolecule();

        if (_pmol)
            _pmol->setIgnoreBadValenceFlag(true);

        if (component->vertexCount())
        {
            std::string mol_node = std::string("mol") + std::to_string(idx);
            writer.Key(mol_node.c_str());
            writer.StartObject();
            writer.Key("type");
            writer.String("molecule");
            saveFragment(*component, writer);
            // TODO: the code below needs refactoring
            Vec3f flag_pos;
            if (bmol.getStereoFlagPosition(idx, flag_pos))
            {
                writer.Key("stereoFlagPosition");
                writer.StartObject();
                writer.Key("x");
                writer.Double(flag_pos.x);
                writer.Key("y");
                writer.Double(flag_pos.y);
                writer.Key("z");
                writer.Double(flag_pos.z);
                writer.EndObject();
            }
            writer.EndObject();
        }
    }

    for (int i = 1; i <= n_rgroups; i++)
    {
        auto& rgrp = mol->rgroups.getRGroup(i);
        if (rgrp.fragments.size())
            saveRGroup(rgrp.fragments, i, writer);
    }

    for (int i = mol->tgroups.begin(); i != mol->tgroups.end(); i = mol->tgroups.next(i))
    {
        TGroup& tg = mol->tgroups.getTGroup(i);
        saveTGroup(tg, writer);
    }

    writer.EndObject();
}

void MoleculeJsonSaver::saveFragment(BaseMolecule& fragment, JsonWriter& writer)
{
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
    saveSelection(fragment, writer);
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
        {ReactionComponent::ARROW_BOTH_ENDS_FILLED_TRIANGLE, "unbalanced-equilibrium-filled-half-triangle"}};

    const auto& meta_objects = meta.metaData();
    for (int meta_index = 0; meta_index < meta_objects.size(); ++meta_index)
    {
        auto pobj = meta_objects[meta_index];
        switch (pobj->_class_id)
        {
        case KETReactionArrow::CID: {
            KETReactionArrow& ar = (KETReactionArrow&)(*pobj);
            writer.StartObject();
            writer.Key("type");
            writer.String("arrow");
            writer.Key("data");
            writer.StartObject();
            // arrow mode
            writer.Key("mode");
            std::string arrow_mode = "open-angle";
            auto at_it = _arrow_type2string.find(ar._arrow_type);
            if (at_it != _arrow_type2string.end())
                arrow_mode = at_it->second.c_str();
            writer.String(arrow_mode.c_str());

            // arrow coordinates
            writer.Key("pos");
            writer.StartArray();
            writer.StartObject();
            writer.Key("x");
            writer.Double(ar._begin.x);
            writer.Key("y");
            writer.Double(ar._begin.y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            writer.StartObject();
            writer.Key("x");
            writer.Double(ar._end.x);
            writer.Key("y");
            writer.Double(ar._end.y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            writer.EndArray();  // arrow coordinates
            writer.EndObject(); // end data
            writer.EndObject(); // end node
        }
        break;
        case KETReactionPlus::CID: {
            KETReactionPlus& rp = (KETReactionPlus&)(*pobj);
            writer.StartObject();
            writer.Key("type");
            writer.String("plus");
            writer.Key("location");
            writer.StartArray();
            writer.Double(rp._pos.x);
            writer.Double(rp._pos.y);
            writer.Double(0);
            writer.EndArray();
            writer.EndObject();
        }
        break;
        case KETSimpleObject::CID: {
            auto simple_obj = (KETSimpleObject*)pobj;
            writer.StartObject();
            writer.Key("type");
            writer.String("simpleObject");
            writer.Key("data");
            writer.StartObject();
            writer.Key("mode");
            switch (simple_obj->_mode)
            {
            case KETSimpleObject::EKETEllipse:
                writer.String("ellipse");
                break;
            case KETSimpleObject::EKETRectangle:
                writer.String("rectangle");
                break;
            case KETSimpleObject::EKETLine:
                writer.String("line");
                break;
            }
            writer.Key("pos");
            writer.StartArray();

            auto& coords = simple_obj->_coordinates;

            // point1
            writer.StartObject();
            writer.Key("x");
            writer.Double(coords.first.x);
            writer.Key("y");
            writer.Double(coords.first.y);
            writer.Key("z");
            writer.Double(0);
            writer.EndObject();

            // point2
            writer.StartObject();
            writer.Key("x");
            writer.Double(coords.second.x);
            writer.Key("y");
            writer.Double(coords.second.y);
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
        case KETTextObject::CID: {
            auto simple_obj = (KETTextObject*)pobj;
            writer.StartObject();
            writer.Key("type");
            writer.String("text");
            writer.Key("data");
            writer.StartObject();
            writer.Key("content");
            writer.String(simple_obj->_content.c_str());
            writer.Key("position");
            writer.StartObject();
            writer.Key("x");
            writer.Double(simple_obj->_pos.x);
            writer.Key("y");
            writer.Double(simple_obj->_pos.y);
            writer.Key("z");
            writer.Double(simple_obj->_pos.z);
            writer.EndObject(); // end position
            writer.EndObject(); // end data
            writer.EndObject(); // end node
            break;
        }
        }
    }
}
