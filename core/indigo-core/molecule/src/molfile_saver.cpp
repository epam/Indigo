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

#include "molecule/molfile_saver.h"

#include <ctime>
#include <map>
#include <sstream>

#include "base_cpp/locale_guard.h"
#include "base_cpp/output.h"
#include "math/algebra.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_cip_calculator.h"
#include "molecule/molecule_savers.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/molecule_stereocenters.h"
#include "molecule/query_molecule.h"

using namespace indigo;

IMPL_ERROR(MolfileSaver, "molfile saver");

CP_DEF(MolfileSaver);

MolfileSaver::MolfileSaver(Output& output) : _output(output), CP_INIT, TL_CP_GET(_atom_mapping), TL_CP_GET(_bond_mapping)
{
    mode = MODE_AUTO;
    no_chiral = false;
    chiral_flag = -1;
    skip_date = false;
    add_stereo_desc = false;
    add_implicit_h = true;
}
/*
 * Utility functions
 */
const float XYZ_EPSILON = 0.0001f;

void write_c(float c, std::stringstream& coords)
{
    int strip = (int)c;
    if (fabs(c - strip) < XYZ_EPSILON)
    {
        coords << strip << ".0";
    }
    else
    {
        coords << c;
    }
}
/*
 * Converts float to string
 */
void convert_xyz_to_string(Vec3f& xyz, std::stringstream& coords)
{
    coords.str("");
    write_c(xyz.x, coords);
    coords << " ";
    write_c(xyz.y, coords);
    coords << " ";
    write_c(xyz.z, coords);
}

void MolfileSaver::saveBaseMolecule(BaseMolecule& mol)
{
    _saveMolecule(mol, mol.isQueryMolecule());
}

void MolfileSaver::saveMolecule(Molecule& mol)
{
    _saveMolecule(mol, false);
}

void MolfileSaver::saveQueryMolecule(QueryMolecule& mol)
{
    _saveMolecule(mol, true);
}

void MolfileSaver::_saveMolecule(BaseMolecule& mol, bool query)
{
    LocaleGuard locale_guard;

    QueryMolecule* qmol = 0;

    if (query)
        qmol = (QueryMolecule*)(&mol);

    if (mode == MODE_2000)
        _v2000 = true;
    else if (mode == MODE_3000)
        _v2000 = false;
    else
    {
        // auto-detect the format: save to v3000 molfile only
        // if v2000 is not enough
        _v2000 = true;

        if (mol.hasHighlighting())
            _v2000 = false;
        else if (mol.stereocenters.haveEnhancedStereocenter())
            _v2000 = false;
        else if (mol.vertexCount() > 999 || mol.edgeCount() > 999)
            _v2000 = false;
    }

    bool rg2000 = (_v2000 && mol.rgroups.getRGroupCount() > 0);

    if (rg2000)
    {
        struct tm lt;
        if (skip_date)
            memset(&lt, 0, sizeof(lt));
        else
        {
            time_t tm = time(NULL);
            lt = *localtime(&tm);
        }
        _output.printfCR("$MDL  REV  1 %02d%02d%02d%02d%02d", lt.tm_mon + 1, lt.tm_mday, lt.tm_year % 100, lt.tm_hour, lt.tm_min);
        _output.writeStringCR("$MOL");
        _output.writeStringCR("$HDR");
    }

    _writeHeader(mol, _output, BaseMolecule::hasZCoord(mol));

    if (rg2000)
    {
        _output.writeStringCR("$END HDR");
        _output.writeStringCR("$CTAB");
    }

    MoleculeCIPCalculator mcc;
    mcc.updateCIPStereoDescriptors(mol, add_stereo_desc);

    if (_v2000)
    {
        _writeCtabHeader2000(_output, mol);
        _writeCtab2000(_output, mol, query);
    }
    else
    {
        _writeCtabHeader(_output);
        _writeCtab(_output, mol, query);
    }

    if (_v2000)
    {
        _writeRGroupIndices2000(_output, mol);
        _writeAttachmentValues2000(_output, mol);
    }

    if (rg2000)
    {
        int i, j;

        MoleculeRGroups& rgroups = mol.rgroups;
        int n_rgroups = rgroups.getRGroupCount();

        for (i = 1; i <= n_rgroups; i++)
        {
            RGroup& rgroup = rgroups.getRGroup(i);

            if (rgroup.fragments.size() == 0)
                continue;

            _output.printf("M  LOG  1 %3d %3d %3d  ", i, rgroup.if_then, rgroup.rest_h);

            QS_DEF(Array<char>, occ);
            ArrayOutput occ_out(occ);

            _writeOccurrenceRanges(occ_out, rgroup.occurrence);

            for (j = 0; j < 3 - occ.size(); j++)
                _output.writeChar(' ');

            _output.write(occ.ptr(), occ.size());
            _output.writeCR();
        }

        _output.writeStringCR("M  END");
        _output.writeStringCR("$END CTAB");

        for (i = 1; i <= n_rgroups; i++)
        {
            PtrPool<BaseMolecule>& frags = rgroups.getRGroup(i).fragments;

            if (frags.size() == 0)
                continue;

            _output.writeStringCR("$RGP");
            _output.printfCR("%4d", i);

            for (j = frags.begin(); j != frags.end(); j = frags.next(j))
            {
                BaseMolecule* fragment = frags[j];

                _output.writeStringCR("$CTAB");
                _writeCtabHeader2000(_output, *fragment);
                _writeCtab2000(_output, *fragment, query);
                _writeRGroupIndices2000(_output, *fragment);
                _writeAttachmentValues2000(_output, *fragment);

                _output.writeStringCR("M  END");
                _output.writeStringCR("$END CTAB");
            }
            _output.writeStringCR("$END RGP");
        }
        _output.writeStringCR("$END MOL");
    }
    else
        _output.writeStringCR("M  END");
}

void MolfileSaver::saveCtab3000(Molecule& mol)
{
    _writeCtab(_output, mol, false);
}

void MolfileSaver::saveQueryCtab3000(QueryMolecule& mol)
{
    _writeCtab(_output, mol, true);
}

int MolfileSaver::parseFormatMode(const char* mode)
{
    if (strcasecmp(mode, "2000") == 0)
        return MolfileSaver::MODE_2000;
    else if (strcasecmp(mode, "3000") == 0)
        return MolfileSaver::MODE_3000;
    else if (strcasecmp(mode, "auto") == 0)
        return MolfileSaver::MODE_AUTO;
    else
        throw Error("unknown format mode: %s, supported values: 2000, 3000, auto", mode);
}

void MolfileSaver::saveFormatMode(int mode, Array<char>& output)
{
    switch (mode)
    {
    case MolfileSaver::MODE_2000:
        output.readString("2000", true);
        break;
    case MolfileSaver::MODE_3000:
        output.readString("3000", true);
        break;
    case MolfileSaver::MODE_AUTO:
        output.readString("auto", true);
        break;
    default:
        throw Error("unknown format mode: %d", mode);
    }
}

void MolfileSaver::_writeHeader(BaseMolecule& mol, Output& output, bool zcoord)
{
    struct tm lt;
    if (skip_date)
        memset(&lt, 0, sizeof(lt));
    else
    {
        time_t tm = time(NULL);
        lt = *localtime(&tm);
    }

    const char* dim;

    if (zcoord)
        dim = "3D";
    else
        dim = "2D";

    if (mol.name.ptr() != 0)
        output.printfCR("%s", mol.name.ptr());
    else
        output.writeCR();
    output.printfCR("  -INDIGO-%02d%02d%02d%02d%02d%s", lt.tm_mon + 1, lt.tm_mday, lt.tm_year % 100, lt.tm_hour, lt.tm_min, dim);
    output.writeCR();
}

void MolfileSaver::_writeCtabHeader(Output& output)
{
    output.printfCR("%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d V3000", 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
}

void MolfileSaver::_writeCtabHeader2000(Output& output, BaseMolecule& mol)
{
    int chiral = 0;

    if (!no_chiral && mol.isChiral())
        chiral = 1;
    if (chiral_flag != -1)
        chiral = chiral_flag;

    output.printfCR("%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d%3d V2000", mol.vertexCount(), mol.edgeCount(), 0, 0, chiral, 0, 0, 0, 0, 0, 999);
}

void MolfileSaver::_writeAtomLabel(Output& output, int label)
{
    output.writeString(Element::toString(label));
}

void MolfileSaver::_writeMultiString(Output& output, const char* string, int len)
{
    int limit = 70;
    while (len > 0)
    {
        output.writeString("M  V30 ");

        if (len <= limit)
            limit = len;

        output.write(string, limit);
        if (len != limit)
            output.writeString("-");
        output.writeCR();
        len -= limit;
        string += limit;
    }
}

void MolfileSaver::_writeCtab(Output& output, BaseMolecule& mol, bool query)
{
    QueryMolecule* qmol = 0;

    if (query)
        qmol = (QueryMolecule*)(&mol);

    output.writeStringCR("M  V30 BEGIN CTAB");
    output.printfCR("M  V30 COUNTS %d %d %d 0 0", mol.vertexCount(), mol.edgeCount(), mol.countSGroups());
    output.writeStringCR("M  V30 BEGIN ATOM");

    int i;
    int iw = 1;
    QS_DEF(Array<char>, buf);

    _atom_mapping.clear_resize(mol.vertexEnd());
    _bond_mapping.clear_resize(mol.edgeEnd());

    for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i), iw++)
        _atom_mapping[i] = iw;

    std::stringstream coords;
    for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        int atom_number = mol.getAtomNumber(i);
        int isotope = mol.getAtomIsotope(i);
        ArrayOutput out(buf);

        out.printf("%d ", _atom_mapping[i]);
        QS_DEF(Array<int>, list);
        int query_atom_type;

        if (atom_number == ELEM_H && isotope == 2)
        {
            out.writeChar('D');
            isotope = 0;
        }
        else if (atom_number == ELEM_H && isotope == 3)
        {
            out.writeChar('T');
            isotope = 0;
        }
        else if (mol.isPseudoAtom(i))
            out.writeString(mol.getPseudoAtom(i));
        else if (mol.isTemplateAtom(i))
            out.writeString(mol.getTemplateAtom(i));
        else if (mol.isRSite(i))
            out.writeString("R#");
        else if (atom_number > 0)
        {
            _writeAtomLabel(out, atom_number);
        }
        else if (qmol != 0 && (query_atom_type = QueryMolecule::parseQueryAtom(*qmol, i, list)) != -1)
        {
            if (query_atom_type == QueryMolecule::QUERY_ATOM_A)
                out.writeChar('A');
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_Q)
                out.writeChar('Q');
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_X)
                out.writeChar('X');
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_M)
                out.writeChar('M');
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_AH)
                out.writeString("AH");
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_QH)
                out.writeString("QH");
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_XH)
                out.writeString("XH");
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_MH)
                out.writeString("MH");
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_LIST || query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
            {
                int k;

                if (query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
                    out.writeString("NOT");

                out.writeChar('[');
                for (k = 0; k < list.size(); k++)
                {
                    if (k > 0)
                        out.writeChar(',');
                    _writeAtomLabel(out, list[k]);
                }
                out.writeChar(']');
            }
        }
        else if (atom_number == -1)
            out.writeChar('A');
        else
            throw Error("molfile 3000: can not save atom %d because of unsupported "
                        "query feature",
                        i);

        int aam = 0, ecflag = 0, irflag = 0;

        aam = mol.reaction_atom_mapping[i];
        irflag = mol.reaction_atom_inversion[i];
        ecflag = mol.reaction_atom_exact_change[i];

        Vec3f& xyz = mol.getAtomXyz(i);
        int charge = mol.getAtomCharge(i);
        int radical = 0;
        int valence = mol.getExplicitValence(i);
        int stereo_parity = _getStereocenterParity(mol, i);

        if (!mol.isRSite(i) && !mol.isPseudoAtom(i) && !mol.isTemplateAtom(i))
            radical = mol.getAtomRadical_NoThrow(i, 0);

        /*
         * Trailing zeros workaround
         */
        convert_xyz_to_string(xyz, coords);

        out.printf(" %s %d", coords.str().c_str(), aam);

        if ((mol.isQueryMolecule() && charge != CHARGE_UNKNOWN) || (!mol.isQueryMolecule() && charge != 0))
            out.printf(" CHG=%d", charge);

        int hcount = MoleculeSavers::getHCount(mol, i, atom_number, charge);

        if (qmol != 0)
        {
            if (hcount > 0)
                out.printf(" HCOUNT=%d", hcount);
            else if (hcount == 0)
                out.printf(" HCOUNT=-1");
        }
        else if (add_implicit_h && Molecule::shouldWriteHCount(mol.asMolecule(), i) && hcount > 0)
        {
            int sg_idx;

            sg_idx = mol.sgroups.addSGroup(SGroup::SG_TYPE_DAT);
            DataSGroup& sgroup = (DataSGroup&)mol.sgroups.getSGroup(sg_idx);

            sgroup.atoms.push(i);

            QS_DEF(Array<char>, tmp_buf);
            ArrayOutput tmp_out(tmp_buf);
            tmp_out.printf("IMPL_H%d", hcount);
            tmp_buf.push(0);
            sgroup.data.readString(tmp_buf.ptr(), true);

            sgroup.name.readString("MRV_IMPLICIT_H", true);
            sgroup.detached = true;
        }

        if (radical > 0)
            out.printf(" RAD=%d", radical);
        if (stereo_parity > 0)
            out.printf(" CFG=%d", stereo_parity);
        if (isotope > 0)
            out.printf(" MASS=%d", isotope);
        if (valence > 0)
            out.printf(" VAL=%d", valence);
        if (valence == 0)
            out.printf(" VAL=-1");
        if (irflag > 0)
            out.printf(" INVRET=%d", irflag);
        if (ecflag > 0)
            out.printf(" EXACHG=%d", ecflag);

        if (mol.isRSite(i))
        {
            int k;

            QS_DEF(Array<int>, rg_list);
            mol.getAllowedRGroups(i, rg_list);

            if (rg_list.size() > 0)
            {
                out.printf(" RGROUPS=(%d", rg_list.size());
                for (k = 0; k < rg_list.size(); k++)
                    out.printf(" %d", rg_list[k]);
                out.writeChar(')');

                if (!_checkAttPointOrder(mol, i))
                {
                    const Vertex& vertex = mol.getVertex(i);

                    out.printf(" ATTCHORD=(%d", vertex.degree() * 2);
                    for (k = 0; k < vertex.degree(); k++)
                        out.printf(" %d %d", _atom_mapping[mol.getRSiteAttachmentPointByOrder(i, k)], k + 1);

                    out.writeChar(')');
                }
            }
        }

        if (mol.isTemplateAtom(i))
        {
            if (mol.getTemplateAtomClass(i) != 0 && strlen(mol.getTemplateAtomClass(i)) > 0)
                out.printf(" CLASS=%s", mol.getTemplateAtomClass(i));

            if (mol.getTemplateAtomSeqid(i) != -1)
                out.printf(" SEQID=%d", mol.getTemplateAtomSeqid(i));

            if (mol.template_attachment_points.size() > 0)
            {
                int ap_count = 0;
                for (int j = mol.template_attachment_points.begin(); j != mol.template_attachment_points.end(); j = mol.template_attachment_points.next(j))
                {
                    BaseMolecule::TemplateAttPoint& ap = mol.template_attachment_points.at(j);
                    if (ap.ap_occur_idx == i)
                        ap_count++;
                }
                if (ap_count > 0)
                {
                    out.printf(" ATTCHORD=(%d", ap_count * 2);
                    for (int j = mol.template_attachment_points.begin(); j != mol.template_attachment_points.end(); j = mol.template_attachment_points.next(j))
                    {
                        BaseMolecule::TemplateAttPoint& ap = mol.template_attachment_points.at(j);
                        if (ap.ap_occur_idx == i)
                        {
                            out.printf(" %d %s", _atom_mapping[ap.ap_aidx], ap.ap_id.ptr());
                        }
                    }
                    out.printf(")");
                }
            }
        }

        if (mol.attachmentPointCount() > 0)
        {
            int val = 0;

            for (int idx = 1; idx <= mol.attachmentPointCount(); idx++)
            {
                for (int j = 0; mol.getAttachmentPoint(idx, j) != -1; j++)
                    if (mol.getAttachmentPoint(idx, j) == i)
                    {
                        val |= 1 << (idx - 1);
                        break;
                    }
            }

            if (val > 0)
                out.printf(" ATTCHPT=%d", val == 3 ? -1 : val);
        }

        if (qmol != 0)
        {
            int unsat;
            if (qmol->getAtom(i).sureValue(QueryMolecule::ATOM_UNSATURATION, unsat))
                out.printf(" UNSAT=1");
            int subst;
            if (MoleculeSavers::getSubstitutionCountFlagValue(*qmol, i, subst))
                out.printf(" SUBST=%d", subst);
            int rbc;
            if (MoleculeSavers::getRingBondCountFlagValue(*qmol, i, rbc))
                out.printf(" RBCNT=%d", rbc);
        }

        _writeMultiString(output, buf.ptr(), buf.size());
    }

    output.writeStringCR("M  V30 END ATOM");
    output.writeStringCR("M  V30 BEGIN BOND");

    iw = 1;

    for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i), iw++)
    {
        const Edge& edge = mol.getEdge(i);
        int bond_order = mol.getBondOrder(i);
        ArrayOutput out(buf);

        _bond_mapping[i] = iw;

        if (bond_order < 0 && qmol != 0)
        {
            int qb = QueryMolecule::getQueryBondType(qmol->getBond(i));

            if (qb == QueryMolecule::QUERY_BOND_SINGLE_OR_DOUBLE)
                bond_order = 5;
            else if (qb == QueryMolecule::QUERY_BOND_SINGLE_OR_AROMATIC)
                bond_order = 6;
            else if (qb == QueryMolecule::QUERY_BOND_DOUBLE_OR_AROMATIC)
                bond_order = 7;
            else if (qb == QueryMolecule::QUERY_BOND_ANY)
                bond_order = 8;
        }

        if (bond_order < 0)
            throw Error("unrepresentable query bond");

        if (bond_order == BOND_ZERO)
        {
            bond_order = 9;
            if ((mol.getAtomNumber(edge.beg) == ELEM_H) || (mol.getAtomNumber(edge.end) == ELEM_H))
                bond_order = 10;
        }

        out.printf("%d %d %d %d", iw, bond_order, _atom_mapping[edge.beg], _atom_mapping[edge.end]);

        int direction = mol.getBondDirection(i);

        switch (direction)
        {
        case BOND_UP:
            out.printf(" CFG=1");
            break;
        case BOND_EITHER:
            out.printf(" CFG=2");
            break;
        case BOND_DOWN:
            out.printf(" CFG=3");
            break;
        case 0:
            if (mol.cis_trans.isIgnored(i))
                if (!_hasNeighborEitherBond(mol, i))
                    out.printf(" CFG=2");
            break;
        }

        int reacting_center = 0;
        reacting_center = mol.reaction_bond_reacting_center[i];

        if (reacting_center != 0)
            out.printf(" RXCTR=%d", reacting_center);

        int indigo_topology = -1;
        if (qmol != 0)
            qmol->getBond(i).sureValue(QueryMolecule::BOND_TOPOLOGY, indigo_topology);

        int topology = 0;
        if (indigo_topology == TOPOLOGY_RING)
            topology = 1;
        else if (indigo_topology == TOPOLOGY_CHAIN)
            topology = 2;

        if (topology != 0)
            out.printf(" TOPO=%d", topology);

        _writeMultiString(output, buf.ptr(), buf.size());
    }

    output.writeStringCR("M  V30 END BOND");

    MoleculeStereocenters& stereocenters = mol.stereocenters;

    if (stereocenters.begin() != stereocenters.end() || mol.hasHighlighting())
    {
        output.writeStringCR("M  V30 BEGIN COLLECTION");

        QS_DEF(Array<int>, processed);

        processed.clear_resize(mol.vertexEnd());
        processed.zerofill();

        for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        {
            if (processed[i])
                continue;

            ArrayOutput out(buf);
            int j, type = stereocenters.getType(i);

            if (type == MoleculeStereocenters::ATOM_ABS)
                out.writeString("MDLV30/STEABS ATOMS=(");
            else if (type == MoleculeStereocenters::ATOM_OR)
                out.printf("MDLV30/STEREL%d ATOMS=(", stereocenters.getGroup(i));
            else if (type == MoleculeStereocenters::ATOM_AND)
                out.printf("MDLV30/STERAC%d ATOMS=(", stereocenters.getGroup(i));
            else
                continue;

            QS_DEF(Array<int>, list);

            list.clear();
            list.push(i);

            for (j = mol.vertexNext(i); j < mol.vertexEnd(); j = mol.vertexNext(j))
                if (stereocenters.sameGroup(i, j))
                {
                    list.push(j);
                    processed[j] = 1;
                }

            out.printf("%d", list.size());
            for (j = 0; j < list.size(); j++)
                out.printf(" %d", _atom_mapping[list[j]]);
            out.writeChar(')');

            _writeMultiString(output, buf.ptr(), buf.size());
        }

        if (mol.hasHighlighting())
        {
            if (mol.countHighlightedBonds() > 0)
            {
                ArrayOutput out(buf);

                out.printf("MDLV30/HILITE BONDS=(%d", mol.countHighlightedBonds());

                for (i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
                    if (mol.isBondHighlighted(i))
                        out.printf(" %d", _bond_mapping[i]);
                out.writeChar(')');

                _writeMultiString(output, buf.ptr(), buf.size());
            }
            if (mol.countHighlightedAtoms() > 0)
            {
                ArrayOutput out(buf);
                out.printf("MDLV30/HILITE ATOMS=(%d", mol.countHighlightedAtoms());
                for (i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
                    if (mol.isAtomHighlighted(i))
                        out.printf(" %d", _atom_mapping[i]);
                out.writeChar(')');

                _writeMultiString(output, buf.ptr(), buf.size());
            }
        }
        if (mol.custom_collections.size() > 0)
        {
            for (i = mol.custom_collections.begin(); i != mol.custom_collections.end(); i = mol.custom_collections.next(i))
            {
                ArrayOutput out(buf);
                out.printf("%s", mol.custom_collections.at(i));
                _writeMultiString(output, buf.ptr(), buf.size());
            }
        }

        output.writeStringCR("M  V30 END COLLECTION");
    }

    QS_DEF(Array<int>, sgs_sorted);
    _checkSGroupIndices(mol, sgs_sorted);

    if (mol.countSGroups() > 0)
    {
        MoleculeSGroups* sgroups = &mol.sgroups;
        int idx = 1;

        output.writeStringCR("M  V30 BEGIN SGROUP");
        for (i = 0; i < sgs_sorted.size(); i++)
        {
            ArrayOutput out(buf);
            int sg_idx = sgs_sorted[i];
            SGroup& sgroup = sgroups->getSGroup(sg_idx);
            _writeGenericSGroup3000(sgroup, idx++, out);
            if (sgroup.sgroup_type == SGroup::SG_TYPE_GEN)
            {
                _writeMultiString(output, buf.ptr(), buf.size());
            }
            else if (sgroup.sgroup_type == SGroup::SG_TYPE_SUP)
            {
                Superatom& sup = (Superatom&)sgroup;
                if (sup.bond_connections.size() > 0)
                {
                    for (int j = 0; j < sup.bond_connections.size(); j++)
                    {
                        out.printf(" CSTATE=(4 %d %f %f %f)", _bond_mapping[sup.bond_connections[j].bond_idx], sup.bond_connections[j].bond_dir.x,
                                   sup.bond_connections[j].bond_dir.y, 0.f);
                    }
                }
                if (sup.subscript.size() > 1)
                {
                    if (sup.subscript.find(' ') > -1)
                        out.printf(" LABEL=\"%s\"", sup.subscript.ptr());
                    else
                        out.printf(" LABEL=%s", sup.subscript.ptr());
                }
                if (sup.sa_class.size() > 1)
                    out.printf(" CLASS=%s", sup.sa_class.ptr());
                if (sup.contracted == 0)
                    out.printf(" ESTATE=E");
                if (sup.attachment_points.size() > 0)
                {
                    for (int j = sup.attachment_points.begin(); j < sup.attachment_points.end(); j = sup.attachment_points.next(j))
                    {
                        int leave_idx = 0;
                        if (sup.attachment_points[j].lvidx > -1)
                            leave_idx = _atom_mapping[sup.attachment_points[j].lvidx];

                        out.printf(" SAP=(3 %d %d %s)", _atom_mapping[sup.attachment_points[j].aidx], leave_idx, sup.attachment_points[j].apid.ptr());
                    }
                }
                if (sup.seqid > 0)
                    out.printf(" SEQID=%d", sup.seqid);

                if (sup.sa_natreplace.size() > 1)
                    out.printf(" NATREPLACE=%s", sup.sa_natreplace.ptr());

                _writeMultiString(output, buf.ptr(), buf.size());
            }
            else if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& dsg = (DataSGroup&)sgroup;

                const char* name = dsg.name.ptr();
                if (name != 0 && strlen(name) > 0)
                {
                    out.writeString(" FIELDNAME=");
                    bool space_found = (strchr(name, ' ') != NULL);
                    if (space_found)
                        out.writeString("\"");
                    out.writeString(name);
                    if (space_found)
                        out.writeString("\"");
                }
                const char* desc = dsg.description.ptr();
                if (desc != 0 && strlen(desc) > 0)
                {
                    out.writeString(" FIELDINFO=");
                    bool space_found = (strchr(desc, ' ') != NULL);
                    if (space_found)
                        out.writeString("\"");
                    out.writeString(desc);
                    if (space_found)
                        out.writeString("\"");
                }
                const char* querycode = dsg.querycode.ptr();
                if (querycode != 0 && strlen(querycode) > 0)
                {
                    out.writeString(" QUERYTYPE=");
                    bool space_found = (strchr(querycode, ' ') != NULL);
                    if (space_found)
                        out.writeString("\"");
                    out.writeString(querycode);
                    if (space_found)
                        out.writeString("\"");
                }
                const char* queryoper = dsg.queryoper.ptr();
                if (queryoper != 0 && strlen(queryoper) > 0)
                {
                    out.writeString(" QUERYOP=");
                    bool space_found = (strchr(queryoper, ' ') != NULL);
                    if (space_found)
                        out.writeString("\"");
                    out.writeString(queryoper);
                    if (space_found)
                        out.writeString("\"");
                }

                out.printf(" FIELDDISP=\"");
                _writeDataSGroupDisplay(dsg, out);
                out.printf("\"");
                if (dsg.data.size() > 0 && dsg.data[0] != 0)
                {
                    // Split field data by new lines
                    int len = dsg.data.size();
                    char* data = dsg.data.ptr();
                    while (len > 0)
                    {
                        int j;
                        for (j = 0; j < len - 1; j++)
                            if (data[j] == '\n')
                                break;

                        out.printf(" FIELDDATA=\"%.*s\"", j, data);
                        if (data[j] == '\n')
                            j++;

                        data += j;
                        len -= j;

                        if (*data == 0)
                            break;
                    }
                }
                _writeMultiString(output, buf.ptr(), buf.size());
            }
            else if (sgroup.sgroup_type == SGroup::SG_TYPE_SRU)
            {
                RepeatingUnit& ru = (RepeatingUnit&)sgroup;
                if (ru.connectivity == SGroup::HEAD_TO_HEAD)
                    out.printf(" CONNECT=HH");
                else if (ru.connectivity == SGroup::HEAD_TO_TAIL)
                    out.printf(" CONNECT=HT");
                else
                    out.printf(" CONNECT=EU");
                if (ru.subscript.size() > 1)
                {
                    if (ru.subscript.find(' ') > -1)
                        out.printf(" LABEL=\"%s\"", ru.subscript.ptr());
                    else
                        out.printf(" LABEL=%s", ru.subscript.ptr());
                }
                _writeMultiString(output, buf.ptr(), buf.size());
            }
            else if (sgroup.sgroup_type == SGroup::SG_TYPE_MUL)
            {
                MultipleGroup& mg = (MultipleGroup&)sgroup;
                if (mg.parent_atoms.size() > 0)
                {
                    out.printf(" PATOMS=(%d", mg.parent_atoms.size());
                    int j;
                    for (j = 0; j < mg.parent_atoms.size(); j++)
                        out.printf(" %d", _atom_mapping[mg.parent_atoms[j]]);
                    out.printf(")");
                }
                out.printf(" MULT=%d", mg.multiplier);
                _writeMultiString(output, buf.ptr(), buf.size());
            }
            else
            {
                _writeMultiString(output, buf.ptr(), buf.size());
            }
        }
        output.writeStringCR("M  V30 END SGROUP");
    }

    output.writeStringCR("M  V30 END CTAB");

    int n_rgroups = mol.rgroups.getRGroupCount();
    for (i = 1; i <= n_rgroups; i++)
        if (mol.rgroups.getRGroup(i).fragments.size() > 0)
            _writeRGroup(output, mol, i);

    int n_tgroups = mol.tgroups.getTGroupCount();
    if (n_tgroups > 0)
    {
        output.writeStringCR("M  V30 BEGIN TEMPLATE");

        for (i = mol.tgroups.begin(); i != mol.tgroups.end(); i = mol.tgroups.next(i))
        {
            _writeTGroup(output, mol, i);
        }
        output.writeStringCR("M  V30 END TEMPLATE");
    }
}

void MolfileSaver::_writeGenericSGroup3000(SGroup& sgroup, int idx, Output& output)
{
    int i;

    output.printf("%d %s %d", sgroup.original_group, SGroup::typeToString(sgroup.sgroup_type), idx);

    if (sgroup.atoms.size() > 0)
    {
        output.printf(" ATOMS=(%d", sgroup.atoms.size());
        for (i = 0; i < sgroup.atoms.size(); i++)
            output.printf(" %d", _atom_mapping[sgroup.atoms[i]]);
        output.printf(")");
    }
    if (sgroup.bonds.size() > 0)
    {
        if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
            output.printf(" CBONDS=(%d", sgroup.bonds.size());
        else
            output.printf(" XBONDS=(%d", sgroup.bonds.size());
        for (i = 0; i < sgroup.bonds.size(); i++)
            output.printf(" %d", _bond_mapping[sgroup.bonds[i]]);
        output.printf(")");
    }
    if (sgroup.sgroup_subtype > 0)
    {
        if (sgroup.sgroup_subtype == SGroup::SG_SUBTYPE_ALT)
            output.printf(" SUBTYPE=ALT");
        else if (sgroup.sgroup_subtype == SGroup::SG_SUBTYPE_RAN)
            output.printf(" SUBTYPE=RAN");
        else if (sgroup.sgroup_subtype == SGroup::SG_SUBTYPE_BLO)
            output.printf(" SUBTYPE=BLO");
    }
    if (sgroup.parent_group > 0)
    {
        output.printf(" PARENT=%d", sgroup.parent_group);
    }
    for (i = 0; i < sgroup.brackets.size(); i++)
    {
        Vec2f* brackets = sgroup.brackets[i];
        output.printf(" BRKXYZ=(9 %f %f %f %f %f %f %f %f %f)", brackets[0].x, brackets[0].y, 0.f, brackets[1].x, brackets[1].y, 0.f, 0.f, 0.f, 0.f);
    }
    if (sgroup.brackets.size() > 0 && sgroup.brk_style > 0)
    {
        output.printf(" BRKTYP=PAREN");
    }
}

void MolfileSaver::_writeOccurrenceRanges(Output& out, const Array<int>& occurrences)
{
    for (int i = 0; i < occurrences.size(); i++)
    {
        int occurrence = occurrences[i];

        if ((occurrence & 0xFFFF) == 0xFFFF)
            out.printf(">%d", (occurrence >> 16) - 1);
        else if ((occurrence >> 16) == (occurrence & 0xFFFF))
            out.printf("%d", occurrence >> 16);
        else if ((occurrence >> 16) == 0)
            out.printf("<%d", (occurrence & 0xFFFF) + 1);
        else
            out.printf("%d-%d", occurrence >> 16, occurrence & 0xFFFF);

        if (i != occurrences.size() - 1)
            out.printf(",");
    }
}

void MolfileSaver::_writeRGroup(Output& output, BaseMolecule& mol, int rg_idx)
{
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    RGroup& rgroup = mol.rgroups.getRGroup(rg_idx);

    output.printfCR("M  V30 BEGIN RGROUP %d", rg_idx);

    out.printf("RLOGIC %d %d ", rgroup.if_then, rgroup.rest_h);

    _writeOccurrenceRanges(out, rgroup.occurrence);

    _writeMultiString(output, buf.ptr(), buf.size());

    PtrPool<BaseMolecule>& frags = rgroup.fragments;
    for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
        _writeCtab(output, *rgroup.fragments[j], mol.isQueryMolecule());

    output.writeStringCR("M  V30 END RGROUP");
}

void MolfileSaver::_writeTGroup(Output& output, BaseMolecule& mol, int tg_idx)
{
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    TGroup& tgroup = mol.tgroups.getTGroup(tg_idx);

    out.printf("TEMPLATE %d ", tgroup.tgroup_id);
    if (tgroup.tgroup_class.size() > 0)
        out.printf("%s/", tgroup.tgroup_class.ptr());
    if (tgroup.tgroup_name.size() > 0)
        out.printf("%s", tgroup.tgroup_name.ptr());
    if (tgroup.tgroup_alias.size() > 0)
        out.printf("/%s", tgroup.tgroup_alias.ptr());
    if (tgroup.tgroup_natreplace.size() > 0)
        out.printf(" NATREPLACE=%s", tgroup.tgroup_natreplace.ptr());
    if (tgroup.tgroup_comment.size() > 0)
        out.printf(" COMMENT=%s", tgroup.tgroup_comment.ptr());

    _writeMultiString(output, buf.ptr(), buf.size());

    _writeCtab(output, *tgroup.fragment.get(), mol.isQueryMolecule());
}

void MolfileSaver::_writeCtab2000(Output& output, BaseMolecule& mol, bool query)
{
    QueryMolecule* qmol = 0;

    if (query)
        qmol = (QueryMolecule*)(&mol);

    int i;
    QS_DEF(Array<int[2]>, radicals);
    QS_DEF(Array<int>, charges);
    QS_DEF(Array<int>, isotopes);
    QS_DEF(Array<int>, pseudoatoms);
    QS_DEF(Array<int>, atom_lists);
    QS_DEF(Array<int>, unsaturated);
    QS_DEF(Array<int[2]>, substitution_count);
    QS_DEF(Array<int[2]>, ring_bonds);

    _atom_mapping.clear_resize(mol.vertexEnd());
    _bond_mapping.clear_resize(mol.edgeEnd());

    radicals.clear();
    charges.clear();
    isotopes.clear();
    pseudoatoms.clear();
    atom_lists.clear();
    unsaturated.clear();
    substitution_count.clear();
    ring_bonds.clear();

    int iw = 1;

    for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i), iw++)
    {
        char label[3] = {' ', ' ', ' '};
        int valence = 0, radical = 0, charge = 0, stereo_parity = 0, hydrogens_count = 0, stereo_care = 0, aam = 0, irflag = 0, ecflag = 0;

        int atom_number = mol.getAtomNumber(i);
        int atom_isotope = mol.getAtomIsotope(i);
        int atom_charge = mol.getAtomCharge(i);
        int atom_radical = 0;

        _atom_mapping[i] = iw;

        if (!mol.isRSite(i) && !mol.isPseudoAtom(i))
            atom_radical = mol.getAtomRadical_NoThrow(i, 0);

        if (mol.isRSite(i))
        {
            label[0] = 'R';
            label[1] = '#';
        }
        else if (mol.isPseudoAtom(i))
        {
            const char* pseudo = mol.getPseudoAtom(i);

            if (strlen(pseudo) <= (size_t)3)
                memcpy(label, pseudo, std::min(strlen(pseudo), (size_t)3));
            else
            {
                label[0] = 'A';
                pseudoatoms.push(i);
            }
        }
        else if (mol.isTemplateAtom(i))
        {
            throw Error("internal: template atom is not supported in V2000 format");
        }
        else if (atom_number == -1)
        {
            if (qmol == 0)
                throw Error("internal: atom number = -1, but qmol == 0");

            QS_DEF(Array<int>, list);

            int query_atom_type = QueryMolecule::parseQueryAtom(*qmol, i, list);

            if (query_atom_type == QueryMolecule::QUERY_ATOM_A)
                label[0] = 'A';
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_Q)
                label[0] = 'Q';
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_X)
                label[0] = 'X';
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_M)
                label[0] = 'M';
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_AH)
            {
                label[0] = 'A';
                label[1] = 'H';
            }
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_QH)
            {
                label[0] = 'Q';
                label[1] = 'H';
            }
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_XH)
            {
                label[0] = 'X';
                label[1] = 'H';
            }
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_MH)
            {
                label[0] = 'M';
                label[1] = 'H';
            }
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_LIST || query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
            {
                label[0] = 'L';
                atom_lists.push(i);
            }
            else
                label[0] = 'A';
            // throw Error("error saving atom #%d: unsupported query atom", i);
        }
        else if (atom_number == ELEM_H && atom_isotope == 2)
            label[0] = 'D';
        else if (atom_number == ELEM_H && atom_isotope == 3)
            label[0] = 'T';
        else
        {
            const char* str = Element::toString(atom_number);

            label[0] = str[0];
            if (str[1] != 0)
            {
                label[1] = str[1];
                if (str[2] != 0)
                {
                    label[2] = str[2];
                }
            }

            if (atom_isotope > 0)
                isotopes.push(i);
        }

        aam = mol.reaction_atom_mapping[i];
        irflag = mol.reaction_atom_inversion[i];
        ecflag = mol.reaction_atom_exact_change[i];

        int explicit_valence = -1;

        if (!mol.isRSite(i) && !mol.isPseudoAtom(i))
            explicit_valence = mol.getExplicitValence(i);

        if (explicit_valence > 0 && explicit_valence <= 14)
            valence = explicit_valence;
        if (explicit_valence == 0)
            valence = 15;

        if (atom_charge != CHARGE_UNKNOWN && atom_charge >= -15 && atom_charge <= 15)
            charge = atom_charge;

        if (charge != 0)
            charges.push(i);

        if (atom_radical >= 0 && atom_radical <= 3)
            radical = atom_radical;

        if (radical != 0)
        {
            int* r = radicals.push();
            r[0] = i;
            r[1] = radical;
        }

        if (qmol != 0)
        {
            int unsat;
            if (qmol->getAtom(i).sureValue(QueryMolecule::ATOM_UNSATURATION, unsat))
                unsaturated.push(i);
            int rbc;
            if (MoleculeSavers::getRingBondCountFlagValue(*qmol, i, rbc))
            {
                int* r = ring_bonds.push();
                r[0] = i;
                r[1] = rbc;
            }
            int subst;
            if (MoleculeSavers::getSubstitutionCountFlagValue(*qmol, i, subst))
            {
                int* s = substitution_count.push();
                s[0] = i;
                s[1] = subst;
            }
        }

        hydrogens_count = MoleculeSavers::getHCount(mol, i, atom_number, atom_charge);

        if (qmol != 0)
        {
            if (hydrogens_count == -1)
                hydrogens_count = 0;
            else
                // molfile stores h+1
                hydrogens_count++;
        }
        else if (add_implicit_h && Molecule::shouldWriteHCount(mol.asMolecule(), i) && hydrogens_count > 0)
        {
            int sg_idx;

            sg_idx = mol.sgroups.addSGroup(SGroup::SG_TYPE_DAT);
            DataSGroup& sgroup = (DataSGroup&)mol.sgroups.getSGroup(sg_idx);

            sgroup.atoms.push(i);

            QS_DEF(Array<char>, tmp_buf);
            ArrayOutput tmp_out(tmp_buf);
            tmp_buf.clear();
            tmp_out.printf("IMPL_H%d", hydrogens_count);
            tmp_buf.push(0);
            sgroup.data.readString(tmp_buf.ptr(), true);

            sgroup.name.readString("MRV_IMPLICIT_H", true);
            sgroup.detached = true;

            hydrogens_count = 0;
        }
        else
        {
            hydrogens_count = 0;
        }

        stereo_parity = _getStereocenterParity(mol, i);

        Vec3f pos = mol.getAtomXyz(i);
        if (fabs(pos.x) < 1e-5f)
            pos.x = 0;
        if (fabs(pos.y) < 1e-5f)
            pos.y = 0;
        if (fabs(pos.z) < 1e-5f)
            pos.z = 0;

        output.printfCR("%10.4f%10.4f%10.4f %c%c%c%2d"
                        "%3d%3d%3d%3d%3d"
                        "%3d%3d%3d%3d%3d%3d",
                        pos.x, pos.y, pos.z, label[0], label[1], label[2], 0, 0, stereo_parity, hydrogens_count, stereo_care, valence, 0, 0, 0, aam, irflag,
                        ecflag);
    }

    iw = 1;

    for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
    {
        const Edge& edge = mol.getEdge(i);
        int bond_order = mol.getBondOrder(i);

        int indigo_topology = -1;

        if (bond_order < 0 && qmol != 0)
        {
            int qb = QueryMolecule::getQueryBondType(qmol->getBond(i));

            if (qb == QueryMolecule::QUERY_BOND_SINGLE_OR_DOUBLE)
                bond_order = 5;
            else if (qb == QueryMolecule::QUERY_BOND_SINGLE_OR_AROMATIC)
                bond_order = 6;
            else if (qb == QueryMolecule::QUERY_BOND_DOUBLE_OR_AROMATIC)
                bond_order = 7;
            else if (qb == QueryMolecule::QUERY_BOND_ANY)
                bond_order = 8;
        }

        if (bond_order < 0)
        {
            Array<char> buf;
            qmol->getBondDescription(i, buf);
            throw Error("unrepresentable query bond: %s", buf.ptr());
        }

        int stereo = 0;
        int reacting_center = 0;

        int direction = mol.getBondDirection(i);

        switch (direction)
        {
        case BOND_UP:
            stereo = 1;
            break;
        case BOND_DOWN:
            stereo = 6;
            break;
        case BOND_EITHER:
            stereo = 4;
            break;
        case 0:
            if (mol.cis_trans.isIgnored(i))
                stereo = 3;
            break;
        }

        if (stereo == 3)
        {
            // discard it if we have a neighbor "either" bond
            if (_hasNeighborEitherBond(mol, i))
                stereo = 0;
        }

        if (qmol != 0 && indigo_topology == -1)
            qmol->getBond(i).sureValue(QueryMolecule::BOND_TOPOLOGY, indigo_topology);

        int topology = 0;
        if (indigo_topology == TOPOLOGY_RING)
            topology = 1;
        else if (indigo_topology == TOPOLOGY_CHAIN)
            topology = 2;

        reacting_center = mol.reaction_bond_reacting_center[i];

        output.printfCR("%3d%3d%3d%3d%3d%3d%3d", _atom_mapping[edge.beg], _atom_mapping[edge.end], bond_order, stereo, 0, topology, reacting_center);
        _bond_mapping[i] = iw++;
    }

    if (charges.size() > 0)
    {
        int j = 0;
        while (j < charges.size())
        {
            output.printf("M  CHG%3d", std::min(charges.size(), j + 8) - j);
            for (i = j; i < std::min(charges.size(), j + 8); i++)
                output.printf(" %3d %3d", _atom_mapping[charges[i]], mol.getAtomCharge(charges[i]));
            output.writeCR();
            j += 8;
        }
    }

    if (radicals.size() > 0)
    {
        int j = 0;
        while (j < radicals.size())
        {
            output.printf("M  RAD%3d", std::min(radicals.size(), j + 8) - j);
            for (i = j; i < std::min(radicals.size(), j + 8); i++)
                output.printf(" %3d %3d", _atom_mapping[radicals[i][0]], radicals[i][1]);
            output.writeCR();
            j += 8;
        }
    }

    if (isotopes.size() > 0)
    {
        int j = 0;
        while (j < isotopes.size())
        {
            output.printf("M  ISO%3d", std::min(isotopes.size(), j + 8) - j);
            for (i = j; i < std::min(isotopes.size(), j + 8); i++)
                output.printf(" %3d %3d", _atom_mapping[isotopes[i]], mol.getAtomIsotope(isotopes[i]));
            output.writeCR();
            j += 8;
        }
    }

    if (unsaturated.size() > 0)
    {
        int j = 0;
        while (j < unsaturated.size())
        {
            output.printf("M  UNS%3d", std::min(unsaturated.size(), j + 8) - j);
            for (i = j; i < std::min(unsaturated.size(), j + 8); i++)
                output.printf(" %3d %3d", _atom_mapping[unsaturated[i]], 1);
            output.writeCR();
            j += 8;
        }
    }

    if (substitution_count.size() > 0)
    {
        int j = 0;
        while (j < substitution_count.size())
        {
            output.printf("M  SUB%3d", std::min(substitution_count.size(), j + 8) - j);
            for (i = j; i < std::min(substitution_count.size(), j + 8); i++)
                output.printf(" %3d %3d", _atom_mapping[substitution_count[i][0]], substitution_count[i][1]);
            output.writeCR();
            j += 8;
        }
    }

    if (ring_bonds.size() > 0)
    {
        int j = 0;
        while (j < ring_bonds.size())
        {
            output.printf("M  RBC%3d", std::min(ring_bonds.size(), j + 8) - j);
            for (i = j; i < std::min(ring_bonds.size(), j + 8); i++)
                output.printf(" %3d %3d", _atom_mapping[ring_bonds[i][0]], ring_bonds[i][1]);
            output.writeCR();
            j += 8;
        }
    }

    for (i = 0; i < atom_lists.size(); i++)
    {
        int atom_idx = atom_lists[i];
        QS_DEF(Array<int>, list);

        int query_atom_type = QueryMolecule::parseQueryAtom(*qmol, atom_idx, list);

        if (query_atom_type != QueryMolecule::QUERY_ATOM_LIST && query_atom_type != QueryMolecule::QUERY_ATOM_NOTLIST)
            throw Error("internal: atom list not recognized");

        if (list.size() < 1)
            throw Error("internal: atom list size is zero");

        output.printf("M  ALS %3d%3d %c ", _atom_mapping[atom_idx], list.size(), query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST ? 'T' : 'F');

        int j;

        for (j = 0; j < list.size(); j++)
        {
            char c1 = ' ', c2 = ' ';
            const char* str = Element::toString(list[j]);

            c1 = str[0];
            if (str[1] != 0)
                c2 = str[1];

            output.printf("%c%c  ", c1, c2);
        }
        output.writeCR();
    }

    for (i = 0; i < pseudoatoms.size(); i++)
    {
        output.printfCR("A  %3d", _atom_mapping[pseudoatoms[i]]);
        output.writeString(mol.getPseudoAtom(pseudoatoms[i]));
        output.writeCR();
    }

    for (i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sgroup = mol.sgroups.getSGroup(i);
        if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& dsg = (DataSGroup&)sgroup;
            if ((dsg.name.size() > 11) && (strncmp(dsg.name.ptr(), "INDIGO_ALIAS", 12) == 0) && (dsg.atoms.size() > 0) && dsg.data.size() > 0)
            {
                output.printfCR("A  %3d", _atom_mapping[dsg.atoms[0]]);
                output.writeString(dsg.data.ptr());
                output.writeCR();
            }
        }
    }

    QS_DEF(Array<int>, sgroup_ids);
    QS_DEF(Array<int>, child_ids);
    QS_DEF(Array<int>, parent_ids);

    sgroup_ids.clear();
    child_ids.clear();
    parent_ids.clear();

    for (i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sgroup = mol.sgroups.getSGroup(i);
        if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
        {
            DataSGroup& dsg = (DataSGroup&)sgroup;
            if ((dsg.name.size() > 11) && (strncmp(dsg.name.ptr(), "INDIGO_ALIAS", 12) == 0))
                continue;
        }
        sgroup_ids.push(i);
    }

    QS_DEF(Array<int>, sgs_sorted);
    _checkSGroupIndices(mol, sgs_sorted);

    if (sgroup_ids.size() > 0)
    {
        int j;
        for (j = 0; j < sgroup_ids.size(); j += 8)
        {
            output.printf("M  STY%3d", std::min(sgroup_ids.size(), j + 8) - j);
            for (i = j; i < std::min(sgroup_ids.size(), j + 8); i++)
            {
                SGroup* sgroup = &mol.sgroups.getSGroup(sgroup_ids[i]);
                output.printf(" %3d %s", sgroup->original_group, SGroup::typeToString(sgroup->sgroup_type));
            }
            output.writeCR();
        }
        for (j = 0; j < sgroup_ids.size(); j++)
        {
            SGroup* sgroup = &mol.sgroups.getSGroup(sgroup_ids[j]);
            if (sgroup->parent_group > 0)
            {
                child_ids.push(sgroup->original_group);
                parent_ids.push(sgroup->parent_group);
            }
        }
        for (j = 0; j < child_ids.size(); j += 8)
        {
            output.printf("M  SPL%3d", std::min(child_ids.size(), j + 8) - j);
            for (i = j; i < std::min(child_ids.size(), j + 8); i++)
            {
                output.printf(" %3d %3d", child_ids[i], parent_ids[i]);
            }
            output.writeCR();
        }
        for (j = 0; j < sgroup_ids.size(); j += 8)
        {
            output.printf("M  SLB%3d", std::min(sgroup_ids.size(), j + 8) - j);
            for (i = j; i < std::min(sgroup_ids.size(), j + 8); i++)
            {
                SGroup* sgroup = &mol.sgroups.getSGroup(sgroup_ids[i]);
                output.printf(" %3d %3d", sgroup->original_group, sgroup->original_group);
            }
            output.writeCR();
        }

        int sru_count = mol.sgroups.getSGroupCount(SGroup::SG_TYPE_SRU);
        for (j = 0; j < sru_count; j += 8)
        {
            output.printf("M  SCN%3d", std::min(sru_count, j + 8) - j);
            for (i = j; i < std::min(sru_count, j + 8); i++)
            {
                RepeatingUnit* ru = (RepeatingUnit*)&mol.sgroups.getSGroup(i, SGroup::SG_TYPE_SRU);

                output.printf(" %3d ", ru->original_group);

                if (ru->connectivity == SGroup::HEAD_TO_HEAD)
                    output.printf("HH ");
                else if (ru->connectivity == SGroup::HEAD_TO_TAIL)
                    output.printf("HT ");
                else
                    output.printf("EU ");
            }
            output.writeCR();
        }

        for (i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            SGroup& sgroup = mol.sgroups.getSGroup(i);

            if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& datasgroup = (DataSGroup&)sgroup;
                if ((datasgroup.name.size() > 11) && (strncmp(datasgroup.name.ptr(), "INDIGO_ALIAS", 12) == 0))
                    continue;
            }

            for (j = 0; j < sgroup.atoms.size(); j += 8)
            {
                int k;
                output.printf("M  SAL %3d%3d", sgroup.original_group, std::min(sgroup.atoms.size(), j + 8) - j);
                for (k = j; k < std::min(sgroup.atoms.size(), j + 8); k++)
                    output.printf(" %3d", _atom_mapping[sgroup.atoms[k]]);
                output.writeCR();
            }
            for (j = 0; j < sgroup.bonds.size(); j += 8)
            {
                int k;
                output.printf("M  SBL %3d%3d", sgroup.original_group, std::min(sgroup.bonds.size(), j + 8) - j);
                for (k = j; k < std::min(sgroup.bonds.size(), j + 8); k++)
                    output.printf(" %3d", _bond_mapping[sgroup.bonds[k]]);
                output.writeCR();
            }

            if (sgroup.sgroup_type == SGroup::SG_TYPE_SUP)
            {
                Superatom& superatom = (Superatom&)sgroup;

                if (superatom.subscript.size() > 1)
                {
                    if (superatom.subscript.find(' ') > -1)
                        output.printfCR("M  SMT %3d \"%s\"", superatom.original_group, superatom.subscript.ptr());
                    else
                        output.printfCR("M  SMT %3d %s", superatom.original_group, superatom.subscript.ptr());
                }
                if (superatom.sa_class.size() > 1)
                    output.printfCR("M  SCL %3d %s", superatom.original_group, superatom.sa_class.ptr());
                if (superatom.bond_connections.size() > 0)
                {
                    for (j = 0; j < superatom.bond_connections.size(); j++)
                    {
                        output.printfCR("M  SBV %3d %3d %9.4f %9.4f", superatom.original_group, _bond_mapping[superatom.bond_connections[j].bond_idx],
                                        superatom.bond_connections[j].bond_dir.x, superatom.bond_connections[j].bond_dir.y);
                    }
                }
                if (superatom.contracted == 0)
                {
                    output.printfCR("M  SDS EXP  1 %3d", superatom.original_group);
                }
                if (superatom.attachment_points.size() > 0)
                {
                    bool next_line = true;
                    int nrem = superatom.attachment_points.size();
                    int k = 0;
                    for (int j = superatom.attachment_points.begin(); j < superatom.attachment_points.end(); j = superatom.attachment_points.next(j))
                    {
                        if (next_line)
                        {
                            output.printf("M  SAP %3d%3d", superatom.original_group, std::min(nrem, 6));
                            next_line = false;
                        }

                        int leave_idx = 0;
                        if (superatom.attachment_points[j].lvidx > -1)
                            leave_idx = _atom_mapping[superatom.attachment_points[j].lvidx];
                        output.printf(" %3d %3d %c%c", _atom_mapping[superatom.attachment_points[j].aidx], leave_idx, superatom.attachment_points[j].apid[0],
                                      superatom.attachment_points[j].apid[1]);
                        k++;
                        nrem--;
                        if ((k == 6) || (nrem == 0))
                        {
                            output.writeCR();
                            next_line = true;
                            k = 0;
                        }
                    }
                }
            }
            else if (sgroup.sgroup_type == SGroup::SG_TYPE_SRU)
            {
                RepeatingUnit& sru = (RepeatingUnit&)sgroup;

                if (sru.subscript.size() > 1)
                {
                    if (sru.subscript.find(' ') > -1)
                        output.printfCR("M  SMT %3d \"%s\"", sru.original_group, sru.subscript.ptr());
                    else
                        output.printfCR("M  SMT %3d %s", sru.original_group, sru.subscript.ptr());
                }
            }
            else if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
            {
                DataSGroup& datasgroup = (DataSGroup&)sgroup;

                if ((datasgroup.name.size() > 11) && (strncmp(datasgroup.name.ptr(), "INDIGO_ALIAS", 12) == 0))
                    continue;

                output.printf("M  SDT %3d ", datasgroup.original_group);

                _writeFormattedString(output, datasgroup.name, 30);

                _writeFormattedString(output, datasgroup.type, 2);

                _writeFormattedString(output, datasgroup.description, 20);

                _writeFormattedString(output, datasgroup.querycode, 2);

                _writeFormattedString(output, datasgroup.queryoper, 15);

                output.writeCR();

                output.printf("M  SDD %3d ", datasgroup.original_group);
                _writeDataSGroupDisplay(datasgroup, output);
                output.writeCR();

                int k = datasgroup.data.size();
                if (k > 0 && datasgroup.data.top() == 0)
                    k--; // Exclude terminating zero

                char* ptr = datasgroup.data.ptr();
                while (k > 0)
                {
                    int j;
                    for (j = 0; j < 69 && j < k; j++)
                        if (ptr[j] == '\n')
                            break;

                    // Print ptr[0..i]
                    output.writeString("M  ");
                    if (j != 69 || j == k)
                        output.writeString("SED ");
                    else
                        output.writeString("SCD ");
                    output.printf("%3d ", datasgroup.original_group);

                    output.write(ptr, j);
                    if (ptr[j] == '\n')
                        j++;

                    ptr += j;
                    k -= j;
                    output.writeCR();
                }
            }
            else if (sgroup.sgroup_type == SGroup::SG_TYPE_MUL)
            {
                MultipleGroup& mg = (MultipleGroup&)sgroup;

                for (j = 0; j < mg.parent_atoms.size(); j += 8)
                {
                    int k;
                    output.printf("M  SPA %3d%3d", mg.original_group, std::min(mg.parent_atoms.size(), j + 8) - j);
                    for (k = j; k < std::min(mg.parent_atoms.size(), j + 8); k++)
                        output.printf(" %3d", _atom_mapping[mg.parent_atoms[k]]);
                    output.writeCR();
                }

                output.printf("M  SMT %3d %d\n", mg.original_group, mg.multiplier);
            }
            for (j = 0; j < sgroup.brackets.size(); j++)
            {
                output.printf("M  SDI %3d  4 %9.4f %9.4f %9.4f %9.4f\n", sgroup.original_group, sgroup.brackets[j][0].x, sgroup.brackets[j][0].y,
                              sgroup.brackets[j][1].x, sgroup.brackets[j][1].y);
            }
            if (sgroup.brackets.size() > 0 && sgroup.brk_style > 0)
            {
                output.printf("M  SBT  1 %3d %3d\n", sgroup.original_group, sgroup.brk_style);
            }
            if (sgroup.sgroup_subtype > 0)
            {
                if (sgroup.sgroup_subtype == SGroup::SG_SUBTYPE_ALT)
                    output.printf("M  SST  1 %3d ALT\n", sgroup.original_group);
                else if (sgroup.sgroup_subtype == SGroup::SG_SUBTYPE_RAN)
                    output.printf("M  SST  1 %3d RAN\n", sgroup.original_group);
                else if (sgroup.sgroup_subtype == SGroup::SG_SUBTYPE_BLO)
                    output.printf("M  SST  1 %3d BLO\n", sgroup.original_group);
            }
        }
    }
}

void MolfileSaver::_writeFormattedString(Output& output, Array<char>& str, int length)
{
    int k = length;
    if ((str.size() > 1) && (str.size() <= length))
    {
        output.printf("%s", str.ptr());
        k -= str.size() - 1;
        while (k-- > 0)
            output.writeChar(' ');
    }
    else if (str.size() > 1)
    {
        for (k = 0; k < length; k++)
        {
            if (str[k] != 0)
                output.writeChar(str[k]);
            else
                output.writeChar(' ');
        }
    }
    else
        while (k-- > 0)
            output.writeChar(' ');
}

void MolfileSaver::_checkSGroupIndices(BaseMolecule& mol, Array<int>& sgs_list)
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

int MolfileSaver::_getStereocenterParity(BaseMolecule& mol, int idx)
{
    int type = mol.stereocenters.getType(idx);
    if (type == 0)
        return 0;
    if (type == MoleculeStereocenters::ATOM_ANY)
        return 3;

    // Reference from "CTfile Formats. Appendix A: Stereo Notes":
    // Number the atoms surrounding the stereo center with 1, 2, 3, and 4 in
    // order of increasing atom number (position in the atom block) (a hydrogen
    // atom should be considered the highest numbered atom, in this case atom 4).
    // View the center from a position such that the bond connecting the
    // highest-numbered atom (4) projects behind the plane formed by atoms 1, 2, and 3.

    int pyramid[4];
    memcpy(pyramid, mol.stereocenters.getPyramid(idx), sizeof(pyramid));
    if (pyramid[3] == -1)
    {
        if (mol.isQueryMolecule())
        {
            if (mol.getAtomNumber(idx) == -1)
                // This atom is not a pure atom
                // There are no implicit hydrogens for query molecules
                return 0;
        }

        // Assign implicit hydrogen the highest index
        pyramid[3] = mol.vertexEnd();
    }
    else
    {
        // Replace pure hydrogen atom with the highest value
        for (int i = 0; i < 4; i++)
        {
            int p = pyramid[i];
            if (mol.getAtomNumber(p) == ELEM_H)
            {
                bool pure_hydrogen = (mol.getAtomIsotope(p) == 0);
                if (!pure_hydrogen && mol.isQueryMolecule())
                    pure_hydrogen = !mol.asQueryMolecule().getAtom(p).hasConstraint(QueryMolecule::ATOM_ISOTOPE);
                if (pure_hydrogen)
                {
                    pyramid[i] = mol.vertexEnd();
                    break;
                }
            }
        }
    }

    if (MoleculeStereocenters::isPyramidMappingRigid(pyramid))
        return 1; // odd parity
    return 2;     // even parity
}

void MolfileSaver::_writeRGroupIndices2000(Output& output, BaseMolecule& mol)
{
    int i, j;

    QS_DEF(Array<int>, atom_ids);
    QS_DEF(Array<int>, rg_ids);

    atom_ids.clear();
    rg_ids.clear();

    for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (!mol.isRSite(i))
            continue;

        QS_DEF(Array<int>, rg_list);

        mol.getAllowedRGroups(i, rg_list);

        for (j = 0; j < rg_list.size(); j++)
        {
            atom_ids.push(_atom_mapping[i]);
            rg_ids.push(rg_list[j]);
        }
    }

    if (atom_ids.size() > 0)
    {
        output.printf("M  RGP%3d", atom_ids.size());
        for (i = 0; i < atom_ids.size(); i++)
            output.printf(" %3d %3d", atom_ids[i], rg_ids[i]);
        output.writeCR();
    }

    for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (!mol.isRSite(i))
            continue;

        if (!_checkAttPointOrder(mol, i))
        {
            const Vertex& vertex = mol.getVertex(i);
            int k;

            output.printf("M  AAL %3d%3d", _atom_mapping[i], vertex.degree());
            for (k = 0; k < vertex.degree(); k++)
                output.printf(" %3d %3d", _atom_mapping[mol.getRSiteAttachmentPointByOrder(i, k)], k + 1);

            output.writeCR();
        }
    }
}

void MolfileSaver::_writeAttachmentValues2000(Output& output, BaseMolecule& fragment)
{
    if (fragment.attachmentPointCount() == 0)
        return;

    std::map<int, int> orders;
    int i;

    for (i = 1; i <= fragment.attachmentPointCount(); i++)
    {
        int j = 0;
        int idx;

        while ((idx = fragment.getAttachmentPoint(i, j++)) != -1)
        {
            auto it = orders.find(_atom_mapping[idx]);
            int* val = it != orders.end() ? &(it->second) : nullptr;

            if (!val)
                orders.emplace(_atom_mapping[idx], 1 << (i - 1));
            else
                *val |= 1 << (i - 1);
        }
    }

    output.printf("M  APO%3d", orders.size());

    for (auto it = orders.begin(); it != orders.end(); it++)
        output.printf(" %3d %3d", it->first, it->second);

    output.writeCR();
}

bool MolfileSaver::_checkAttPointOrder(BaseMolecule& mol, int rsite)
{
    const Vertex& vertex = mol.getVertex(rsite);
    int i;

    for (i = 0; i < vertex.degree() - 1; i++)
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

void MolfileSaver::_writeDataSGroupDisplay(DataSGroup& datasgroup, Output& out)
{
    out.printf("%10.4f%10.4f    %c%c%c", datasgroup.display_pos.x, datasgroup.display_pos.y, datasgroup.detached ? 'D' : 'A', datasgroup.relative ? 'R' : 'A',
               datasgroup.display_units ? 'U' : ' ');
    if (datasgroup.num_chars == 0)
        out.printf("   ALL  1    %c  %1d  ", datasgroup.tag, datasgroup.dasp_pos);
    else
        out.printf("   %3d  1    %c  %1d  ", datasgroup.num_chars, datasgroup.tag, datasgroup.dasp_pos);
}

bool MolfileSaver::_hasNeighborEitherBond(BaseMolecule& mol, int edge_idx)
{
    const Edge& edge = mol.getEdge(edge_idx);
    const Vertex& beg = mol.getVertex(edge.beg);
    const Vertex& end = mol.getVertex(edge.end);
    int k;

    for (k = beg.neiBegin(); k != beg.neiEnd(); k = beg.neiNext(k))
        if (mol.getBondDirection2(edge.beg, beg.neiVertex(k)) == BOND_EITHER)
            return true;

    for (k = end.neiBegin(); k != end.neiEnd(); k = end.neiNext(k))
        if (mol.getBondDirection2(edge.end, end.neiVertex(k)) == BOND_EITHER)
            return true;
    return false;
}
