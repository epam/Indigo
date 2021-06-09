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
#include <vector>

#include "layout/molecule_layout.h"
#include "molecule/molecule.h"
#include "molecule/molecule_json_saver.h"
#include "molecule/query_molecule.h"

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(MoleculeJsonSaver, "molecule json saver");

MoleculeJsonSaver::MoleculeJsonSaver(Output& output) : _output(output)
{
}

void MoleculeJsonSaver::_checkSGroupIndices(BaseMolecule& mol, ArrayNew<int>& sgs_list)
{
    QS_DEF(ArrayNew<int>, orig_ids);
    QS_DEF(ArrayNew<int>, added_ids);
    QS_DEF(ArrayNew<int>, sgs_mapping);
    QS_DEF(ArrayNew<int>, sgs_changed);

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

void MoleculeJsonSaver::saveSGroups(BaseMolecule& mol, rapidjson::Writer<rapidjson::StringBuffer>& writer)
{
    QS_DEF(ArrayNew<int>, sgs_sorted);
    _checkSGroupIndices(mol, sgs_sorted);

    if (mol.countSGroups() > 0)
    {
        writer.Key("sgroups");
        writer.StartArray();
        int idx = 1;
        for (int i = 0; i < sgs_sorted.size(); i++)
        {
            int sg_idx = sgs_sorted[i];
            saveSGroup(mol.sgroups.getSGroup(sg_idx), writer);
        }
        writer.EndArray();
    }
}

void indigo::MoleculeJsonSaver::saveSGroup(SGroup& sgroup, rapidjson::Writer<rapidjson::StringBuffer>& writer)
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
        if (name && strlen(name))
        {
            writer.Key("fieldName");
            writer.String(name);
        }
        auto data = dsg.data.ptr();
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

void MoleculeJsonSaver::saveBonds(BaseMolecule& mol, rapidjson::Writer<rapidjson::StringBuffer>& writer)
{
    QS_DEF(ArrayChar, buf);
    ArrayOutput out(buf);
    if (mol.edgeCount() > 0)
    {
        writer.Key("bonds");
        writer.StartArray();
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
                    /*	int parity = mol.cis_trans.getParity(i);
                        if (parity)
                        {
                            if (parity == MoleculeCisTrans::CIS)
                                stereo = 7;
                            if (parity == MoleculeCisTrans::TRANS)
                                stereo = 8;
                            const int* subst = mol.cis_trans.getSubstituents(i);
                            writer.Key("subs");
                            writer.StartArray();
                            writer.Int(subst[0]);
                            writer.Int(e1.beg);
                            writer.Int(e1.end);
                            writer.Int(subst[2]);
                            writer.EndArray();
                        }*/
                }
                break;
                }

            if (stereo)
            {
                writer.Key("stereo");
                writer.Uint(stereo);
            }
            writer.EndObject();
        }
        writer.EndArray();
    }
}

void MoleculeJsonSaver::saveAttachmentPoint(BaseMolecule& mol, int atom_idx, rapidjson::Writer<rapidjson::StringBuffer>& writer)
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

void MoleculeJsonSaver::saveStereoCenter(BaseMolecule& mol, int atom_idx, rapidjson::Writer<rapidjson::StringBuffer>& writer)
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

void indigo::MoleculeJsonSaver::saveHighlights(BaseMolecule& mol, rapidjson::Writer<rapidjson::StringBuffer>& writer)
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

void indigo::MoleculeJsonSaver::saveSelection(BaseMolecule& mol, rapidjson::Writer<rapidjson::StringBuffer>& writer)
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

void MoleculeJsonSaver::saveAtoms(BaseMolecule& mol, Writer<StringBuffer>& writer)
{
    QS_DEF(ArrayChar, buf);
    ArrayOutput out(buf);
    if (mol.vertexCount() > 0)
    {
        for (auto i : mol.vertices())
        {
            int anum = mol.getAtomNumber(i);
            int isotope = mol.getAtomIsotope(i);
            writer.StartObject();
            if (mol.attachmentPointCount())
                saveAttachmentPoint(mol, i, writer);
            QS_DEF(ArrayNew<int>, rg_list);
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
                if (!mol.isPseudoAtom(i))
                {
                    radical = mol.getAtomRadical(i);
                }
                mol.getAtomSymbol(i, buf);
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
                writer.Key("label");
                writer.String(buf.ptr());
            }
            if (BaseMolecule::hasCoord(mol))
            {
                const Vec3f& coord = mol.getAtomXyz(i);
                writer.Key("location");
                writer.StartArray();
                writer.Double(coord.x);
                writer.Double(coord.y);
                writer.Double(coord.z);
                writer.EndArray();
            }
            int charge = mol.getAtomCharge(i);
            int evalence = mol.getExplicitValence(i);
            int mapping = mol.reaction_atom_mapping[i];
            if (mapping)
            {
                writer.Key("mapping");
                writer.Int(mapping);
            }
            if (charge)
            {
                writer.Key("charge");
                writer.Int(charge);
            }
            if (evalence > 0)
            {
                writer.Key("explicitValence");
                writer.Int(evalence);
            }
            if (radical)
            {
                writer.Key("radical");
                writer.Int(radical);
            }

            if (isotope && anum != ELEM_H)
            {
                writer.Key("isotope");
                writer.Int(isotope);
            }
            writer.EndObject();
        }
    }
}

void MoleculeJsonSaver::saveRGroup(PtrPool<BaseMolecule>& fragments, int rgnum, rapidjson::Writer<rapidjson::StringBuffer>& writer)
{
    QS_DEF(ArrayChar, buf);
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

    writer.Key("atoms");
    writer.StartArray();
    for (int j = fragments.begin(); j != fragments.end(); j = fragments.next(j))
        saveAtoms(*fragments[j], writer);
    writer.EndArray();

    writer.Key("bonds");
    writer.StartArray();
    for (int j = fragments.begin(); j != fragments.end(); j = fragments.next(j))
        saveBonds(*fragments[j], writer);
    writer.EndArray();
    writer.EndObject();
}

void MoleculeJsonSaver::saveMolecule(BaseMolecule& bmol, Writer<StringBuffer>& writer)
{
    // bool have_z = BaseMolecule::hasZCoord(*_mol);
    int chiral = bmol.getChiralFlag();
    std::unique_ptr<BaseMolecule> mol;
    _pmol = nullptr;
    _pqmol = nullptr;
    if (bmol.isQueryMolecule())
    {
        mol.reset(new QueryMolecule());
        _pqmol = (QueryMolecule*)mol.get();
    }
    else
    {
        mol.reset(new Molecule());
        _pmol = (Molecule*)mol.get();
    }
    mol->clone_KeepIndices(bmol);
    if (!BaseMolecule::hasCoord(*mol))
    {
        MoleculeLayout ml(*mol, false);
        ml.layout_orientation = UNCPECIFIED;
        ml.make();
    }
    BaseMolecule::collapse(*mol);
    QS_DEF(ArrayChar, buf);
    ArrayOutput out(buf);
    std::set<int> rgrp_full_list;
    writer.StartObject();

    writer.Key("root");
    writer.StartObject();
    writer.Key("nodes");
    writer.StartArray();

    writer.StartObject();
    writer.Key("$ref");
    writer.String("mol0");
    writer.EndObject();

    int n_rgroups = mol->rgroups.getRGroupCount();
    for (int i = 1; i <= n_rgroups; ++i)
    {
        buf.clear();
        out.printf("rg%d", i);
        buf.push(0);
        writer.StartObject();
        writer.Key("$ref");
        writer.String(buf.ptr());
        writer.EndObject();
    }

    writer.EndArray();  // nodes
    writer.EndObject(); // root

    writer.Key("mol0");
    writer.StartObject();
    writer.Key("type");
    writer.String("molecule");
    if (chiral)
    {
        writer.Key("chiral");
        writer.Int(chiral);
    }
    writer.Key("atoms");
    writer.StartArray();
    saveAtoms(*mol, writer);
    writer.EndArray();

    saveBonds(*mol, writer);
    saveSGroups(bmol, writer);
    saveHighlights(*mol, writer);
    saveSelection(*mol, writer);

    writer.EndObject(); // mol0

    for (int i = 1; i <= n_rgroups; i++)
    {
        auto& rgrp = mol->rgroups.getRGroup(i);
        if (rgrp.fragments.size())
            saveRGroup(rgrp.fragments, i, writer);
    }

    writer.EndObject();
}

void MoleculeJsonSaver::saveMolecule(BaseMolecule& bmol)
{
    StringBuffer s;
    Writer<StringBuffer> writer(s);
    saveMolecule(bmol, writer);
    std::stringstream result;
    result << s.GetString();
    _output.printf("%s", result.str().c_str());
}
