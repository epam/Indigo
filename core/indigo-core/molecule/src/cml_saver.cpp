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

#include "molecule/cml_saver.h"

#include <cfloat>
#include <tinyxml2.h>

#include "base_cpp/locale_guard.h"
#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_savers.h"
#include "molecule/query_molecule.h"

using namespace indigo;
using namespace tinyxml2;

IMPL_ERROR(CmlSaver, "CML saver");

CmlSaver::CmlSaver(Output& output) : _output(output)
{
    skip_cml_tag = false;
}

void CmlSaver::saveMolecule(Molecule& mol)
{
    _saveMolecule(mol, false);
}

void CmlSaver::saveQueryMolecule(QueryMolecule& mol)
{
    _saveMolecule(mol, true);
}

void CmlSaver::_validate(BaseMolecule& bmol)
{
    std::string unresolved;
    if (bmol.getUnresolvedTemplatesList(bmol, unresolved))
        throw Error("%s cannot be written in CML format.", unresolved.c_str());
}

void CmlSaver::_saveMolecule(BaseMolecule& bmol, bool query)
{
    LocaleGuard locale_guard;
    _validate(bmol);
    auto* pmol = &bmol;
    std::unique_ptr<BaseMolecule> mol;
    if (bmol.tgroups.getTGroupCount())
    {
        mol.reset(bmol.neu());
        mol->clone(bmol);
        mol->transformTemplatesToSuperatoms();
        pmol = mol.get();
    }

    std::unique_ptr<XMLDocument> doc = std::make_unique<XMLDocument>();
    _doc = doc->GetDocument();
    _root = 0;
    XMLElement* elem = 0;

    if (!skip_cml_tag)
    {
        auto* decl = _doc->NewDeclaration();
        _doc->LinkEndChild(decl);
        _root = _doc->NewElement("cml");
        _doc->LinkEndChild(_root);
        elem = _root;
    }

    _addMoleculeElement(elem, *pmol, query);

    _addRgroups(elem, *pmol, query);

    XMLPrinter printer;
    _doc->Accept(&printer);
    _output.printf("%s", printer.CStr());
    doc.reset(nullptr);
}

void CmlSaver::_addMoleculeElement(XMLElement* elem, BaseMolecule& mol, bool query)
{
    int i;

    BaseMolecule* _mol = &mol;
    QueryMolecule* qmol = 0;

    if (query)
        qmol = (QueryMolecule*)(&mol);

    XMLElement* molecule = _doc->NewElement("molecule");
    if (elem == 0)
        _doc->LinkEndChild(molecule);
    else
        elem->LinkEndChild(molecule);

    if (_mol->name.ptr() != 0)
    {
        if (strchr(_mol->name.ptr(), '\"') != NULL)
            throw Error("can not save molecule with '\"' in title");
        molecule->SetAttribute("title", _mol->name.ptr());
    }

    bool have_xyz = BaseMolecule::hasCoord(*_mol);
    bool have_z = BaseMolecule::hasZCoord(*_mol);

    if (_mol->vertexCount() > 0)
    {
        XMLElement* atomarray = _doc->NewElement("atomArray");
        molecule->LinkEndChild(atomarray);

        for (i = _mol->vertexBegin(); i != _mol->vertexEnd(); i = _mol->vertexNext(i))
        {

            int atom_number = _mol->getAtomNumber(i);

            const char* atom_str = nullptr;

            if (_mol->isRSite(i))
                atom_str = "R";
            else if (_mol->isPseudoAtom(i))
                atom_str = _mol->getPseudoAtom(i);
            // else if (_mol->isTemplateAtom(i))
            //    atom_str = _mol->getTemplateAtom(i);
            else if (atom_number > 0)
                atom_str = Element::toString(atom_number);
            else if (qmol != 0)
            {
                QS_DEF(Array<int>, list);

                int query_atom_type;
                if ((query_atom_type = QueryMolecule::parseQueryAtom(*qmol, i, list)) != -1)
                {
                    if (query_atom_type == QueryMolecule::QUERY_ATOM_LIST || query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
                    {
                        atom_str = Element::toString(list[0]);
                    }
                    if (QueryMolecule::queryAtomIsSpecial(*qmol, i) || query_atom_type == QueryMolecule::QUERY_ATOM_A)
                    {
                        atom_str = Element::toString(ELEM_C);
                    }
                }
            }

            if (atom_str == nullptr)
            {
                throw Error("internal error: atom element was not set");
            }
            XMLElement* atom = _doc->NewElement("atom");
            atomarray->LinkEndChild(atom);

            QS_DEF(Array<char>, buf);
            ArrayOutput out(buf);
            out.printf("a%d", i);
            buf.push(0);
            atom->SetAttribute("id", buf.ptr());
            atom->SetAttribute("elementType", atom_str);

            if (qmol != 0)
            {
                QS_DEF(Array<int>, list);

                int query_atom_type;
                if ((query_atom_type = QueryMolecule::parseQueryAtom(*qmol, i, list)) != -1)
                {
                    if (query_atom_type == QueryMolecule::QUERY_ATOM_AH)
                        atom->SetAttribute("mrvPseudo", "AH");
                    else if (query_atom_type == QueryMolecule::QUERY_ATOM_QH)
                        atom->SetAttribute("mrvPseudo", "QH");
                    else if (query_atom_type == QueryMolecule::QUERY_ATOM_X)
                        atom->SetAttribute("mrvPseudo", "X");
                    else if (query_atom_type == QueryMolecule::QUERY_ATOM_XH)
                        atom->SetAttribute("mrvPseudo", "XH");
                    else if (query_atom_type == QueryMolecule::QUERY_ATOM_M)
                        atom->SetAttribute("mrvPseudo", "M");
                    else if (query_atom_type == QueryMolecule::QUERY_ATOM_MH)
                        atom->SetAttribute("mrvPseudo", "MH");
                }
            }

            if (_mol->getAtomIsotope(i) > 0)
            {
                atom->SetAttribute("isotope", _mol->getAtomIsotope(i));
                // for inchi-1 program which ignores "isotope" property (version 1.03)
                atom->SetAttribute("isotopeNumber", _mol->getAtomIsotope(i));
            }

            int charge = _mol->getAtomCharge(i);
            if ((mol.isQueryMolecule() && charge != CHARGE_UNKNOWN) || (!mol.isQueryMolecule() && charge != 0))
                atom->SetAttribute("formalCharge", _mol->getAtomCharge(i));

            if (!_mol->isRSite(i) && !_mol->isPseudoAtom(i))
            {
                if (_mol->getAtomRadical_NoThrow(i, 0) > 0)
                {
                    atom->SetAttribute("spinMultiplicity", _mol->getAtomRadical(i));
                    if (_mol->getAtomRadical_NoThrow(i, 0) == 1)
                        atom->SetAttribute("radical", "divalent1");
                    else if (_mol->getAtomRadical_NoThrow(i, 0) == 2)
                        atom->SetAttribute("radical", "monovalent");
                    else if (_mol->getAtomRadical_NoThrow(i, 0) == 3)
                        atom->SetAttribute("radical", "divalent3");
                }

                if (_mol->getExplicitValence(i) > 0)
                    atom->SetAttribute("mrvValence", _mol->getExplicitValence(i));

                if (qmol == 0)
                {
                    if (Molecule::shouldWriteHCount(mol.asMolecule(), i))
                    {
                        int hcount;

                        try
                        {
                            hcount = _mol->getAtomTotalH(i);
                        }
                        catch (Exception&)
                        {
                            hcount = -1;
                        }

                        if (hcount >= 0)
                            atom->SetAttribute("hydrogenCount", hcount);
                    }
                }

                if (mol.isAlias(i))
                {
                    atom->SetAttribute("mrvAlias", mol.getAlias(i));
                }
            }

            if (_mol->isRSite(i))
            {
                QS_DEF(Array<int>, rg_refs);

                _mol->getAllowedRGroups(i, rg_refs);

                QS_DEF(Array<char>, rbuf);
                ArrayOutput rout(rbuf);

                if (rg_refs.size() == 1)
                {
                    rout.printf("%d", rg_refs[0]);
                    rbuf.push(0);
                    atom->SetAttribute("rgroupRef", rbuf.ptr());
                }
            }

            if (qmol != 0)
            {
                QS_DEF(Array<char>, qbuf);
                ArrayOutput qout(qbuf);

                QS_DEF(Array<int>, list);

                int query_atom_type;
                if ((query_atom_type = QueryMolecule::parseQueryAtom(*qmol, i, list)) != -1)
                {
                    if (query_atom_type == QueryMolecule::QUERY_ATOM_LIST || query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
                    {
                        int k;

                        qout.writeString("L");

                        for (k = 0; k < list.size(); k++)
                        {
                            if (query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
                                qout.writeString("!");
                            else
                                qout.writeString(",");

                            qout.writeString(Element::toString(list[k]));
                        }
                        qout.writeString(": ");
                    }
                    else if (query_atom_type == QueryMolecule::QUERY_ATOM_A)
                        qout.writeString("A");
                    else if (query_atom_type == QueryMolecule::QUERY_ATOM_Q)
                        qout.writeString("Q");
                }

                int rbc;
                if (_getRingBondCountFlagValue(*qmol, i, rbc))
                {
                    if (rbc > 0)
                        qout.printf("rb%d;", rbc);
                    else if (rbc == -2)
                        qout.printf("rb*;");
                    else if (rbc == -1)
                        qout.printf("rb0;");
                }

                int subst;
                if (_getSubstitutionCountFlagValue(*qmol, i, subst))
                {
                    if (subst > 0)
                        qout.printf("s%d;", subst);
                    else if (subst == -2)
                        qout.printf("s*;");
                    else if (subst == -1)
                        qout.printf("s0;");
                }

                int unsat;
                if (qmol->getAtom(i).sureValue(QueryMolecule::ATOM_UNSATURATION, unsat))
                    qout.printf("u1;");

                int hcount = MoleculeSavers::getHCount(mol, i, atom_number, charge);

                if (hcount > 0)
                    qout.printf("H%d", hcount);
                else if (hcount == 0)
                    qout.printf("H0");

                if (qbuf.size() > 0)
                {
                    qbuf.push(0);
                    atom->SetAttribute("mrvQueryProps", qbuf.ptr());
                }
            }

            if (_mol->attachmentPointCount() > 0)
            {
                int val = 0;

                for (int idx = 1; idx <= _mol->attachmentPointCount(); idx++)
                {
                    for (int j = 0; _mol->getAttachmentPoint(idx, j) != -1; j++)
                    {
                        if (_mol->getAttachmentPoint(idx, j) == i)
                        {
                            val |= 1 << (idx - 1);
                            break;
                        }
                    }
                }

                if (val > 0)
                {
                    if (val == 3)
                        atom->SetAttribute("attachmentPoint", "both");
                    else
                        atom->SetAttribute("attachmentPoint", val);
                }
            }

            if (have_xyz)
            {
                Vec3f& pos = _mol->getAtomXyz(i);

                if (have_z)
                {
                    if (fabsf(pos.x) < FLT_MIN)
                        pos.x = 0.f;
                    if (fabsf(pos.y) < FLT_MIN)
                        pos.y = 0.f;
                    if (fabsf(pos.z) < FLT_MIN)
                        pos.z = 0.f;
                    atom->SetAttribute("x3", pos.x);
                    atom->SetAttribute("y3", pos.y);
                    atom->SetAttribute("z3", pos.z);
                }
                else
                {
                    if (fabsf(pos.x) < FLT_MIN)
                        pos.x = 0.f;
                    if (fabsf(pos.y) < FLT_MIN)
                        pos.y = 0.f;
                    atom->SetAttribute("x2", pos.x);
                    atom->SetAttribute("y2", pos.y);
                }
            }

            if (_mol->stereocenters.getType(i) > MoleculeStereocenters::ATOM_ANY && _mol->stereocenters.isTetrahydral(i))
            {
                XMLElement* atomparity = _doc->NewElement("atomParity");
                atom->LinkEndChild(atomparity);

                QS_DEF(Array<char>, sbuf);
                ArrayOutput sout(sbuf);
                const int* pyramid = _mol->stereocenters.getPyramid(i);
                if (pyramid[2] == -1)
                {
                    // The atomRefs4 attribute in the atomParity element specifies
                    // the four atoms involved in defining the stereochemistry.
                    // These atoms are typically ordered in a sequence
                    // that represents a directed path from the stereocenter to the fourth atom
                    // with the reference atom (the atom to which the stereochemistry is referenced) being included twice.
                    const Vertex& v = _mol->getVertex(i);
                    int j = v.neiVertex(i);
                    sout.printf("a%d a%d a%d a%d", pyramid[0], pyramid[1], i, j);
                }
                else if (pyramid[3] == -1)
                    sout.printf("a%d a%d a%d a%d", pyramid[0], pyramid[1], pyramid[2], i);
                else
                    sout.printf("a%d a%d a%d a%d", pyramid[0], pyramid[1], pyramid[2], pyramid[3]);
                sbuf.push(0);
                atomparity->SetAttribute("atomRefs4", sbuf.ptr());

                atomparity->LinkEndChild(_doc->NewText("1"));
            }

            if (_mol->reaction_atom_mapping[i] > 0)
            {
                atom->SetAttribute("mrvMap", _mol->reaction_atom_mapping[i]);
            }

            if (_mol->reaction_atom_inversion[i] > 0)
            {
                if (_mol->reaction_atom_inversion[i] == 1)
                    atom->SetAttribute("reactionStereo", "Inv");
                else if (_mol->reaction_atom_inversion[i] == 2)
                    atom->SetAttribute("reactionStereo", "Ret");
            }

            if (_mol->reaction_atom_exact_change[i] > 0)
            {
                atom->SetAttribute("exactChange", "1");
            }
        }

        int latest_ind = i;

        if (_mol->attachmentPointCount() > 0)
        {
            for (i = _mol->vertexBegin(); i != _mol->vertexEnd(); i = _mol->vertexNext(i))
            {
                int val = 0;

                for (int idx = 1; idx <= _mol->attachmentPointCount(); idx++)
                {
                    for (int j = 0; _mol->getAttachmentPoint(idx, j) != -1; j++)
                    {
                        if (_mol->getAttachmentPoint(idx, j) == i)
                        {
                            val |= 1 << (idx - 1);
                            break;
                        }
                    }
                }

                if (val > 0)
                {
                    XMLElement* atom = _doc->NewElement("atom");
                    atomarray->LinkEndChild(atom);

                    QS_DEF(Array<char>, buf);
                    ArrayOutput out(buf);
                    out.printf("a%d", latest_ind++);
                    buf.push(0);
                    atom->SetAttribute("id", buf.ptr());
                    atom->SetAttribute("elementType", "*");
                }
            }
        }
    }

    if (_mol->edgeCount() > 0)
    {
        XMLElement* bondarray = _doc->NewElement("bondArray");
        molecule->LinkEndChild(bondarray);

        for (i = _mol->edgeBegin(); i != _mol->edgeEnd(); i = _mol->edgeNext(i))
        {
            const Edge& edge = _mol->getEdge(i);

            XMLElement* bond = _doc->NewElement("bond");
            bondarray->LinkEndChild(bond);

            QS_DEF(Array<char>, buf);
            ArrayOutput out(buf);
            out.printf("a%d a%d", edge.beg, edge.end);
            buf.push(0);
            bond->SetAttribute("atomRefs2", buf.ptr());

            int order = _mol->getBondOrder(i);

            if (order == BOND_SINGLE || order == BOND_DOUBLE || order == BOND_TRIPLE)
                bond->SetAttribute("order", order);
            else if (order == BOND_AROMATIC)
                bond->SetAttribute("order", "A");

            if (qmol != 0)
            {
                if (order < 0)
                    bond->SetAttribute("order", 1);

                int qb = QueryMolecule::getQueryBondType(qmol->getBond(i));

                if (qb == _BOND_SINGLE_OR_DOUBLE)
                    bond->SetAttribute("queryType", "SD");
                else if (qb == _BOND_SINGLE_OR_AROMATIC)
                    bond->SetAttribute("queryType", "SA");
                else if (qb == _BOND_DOUBLE_OR_AROMATIC)
                    bond->SetAttribute("queryType", "DA");
                else if (qb == _BOND_ANY)
                    bond->SetAttribute("queryType", "Any");

                int indigo_topology = -1;
                if (qmol != 0)
                    qmol->getBond(i).sureValue(QueryMolecule::BOND_TOPOLOGY, indigo_topology);

                if (indigo_topology == TOPOLOGY_RING)
                    bond->SetAttribute("topology", "ring");
                else if (indigo_topology == TOPOLOGY_CHAIN)
                    bond->SetAttribute("topology", "chain");
            }

            int dir = _mol->getBondDirection(i);
            int parity = _mol->cis_trans.getParity(i);

            if (dir == BOND_UP || dir == BOND_DOWN)
            {
                XMLElement* bondstereo = _doc->NewElement("bondStereo");
                bond->LinkEndChild(bondstereo);
                bondstereo->LinkEndChild(_doc->NewText((dir == BOND_UP) ? "W" : "H"));
            }
            else if (parity != 0)
            {
                XMLElement* bondstereo = _doc->NewElement("bondStereo");
                bond->LinkEndChild(bondstereo);

                QS_DEF(Array<char>, pbuf);
                ArrayOutput pout(pbuf);

                const int* subst = _mol->cis_trans.getSubstituents(i);
                pout.printf("a%d a%d a%d a%d", subst[0], edge.beg, edge.end, subst[2]);
                pbuf.push(0);
                bondstereo->SetAttribute("atomRefs4", pbuf.ptr());
                bondstereo->LinkEndChild(_doc->NewText((parity == MoleculeCisTrans::CIS) ? "C" : "T"));
            }
            else if (_mol->cis_trans.isIgnored(i))
            {
                XMLElement* bondstereo = _doc->NewElement("bondStereo");
                bond->LinkEndChild(bondstereo);
                bondstereo->SetAttribute("convention", "MDL");
                bondstereo->SetAttribute("conventionValue", "3");
            }

            if (_mol->reaction_bond_reacting_center[i] != 0)
            {
                bond->SetAttribute("mrvReactingCenter", _mol->reaction_bond_reacting_center[i]);
            }
        }
    }

    if (_mol->countSGroups() > 0)
    {
        for (i = _mol->sgroups.begin(); i != _mol->sgroups.end(); i = _mol->sgroups.next(i))
        {
            SGroup& sgroup = _mol->sgroups.getSGroup(i);

            if (sgroup.parent_group == 0)
                _addSgroupElement(molecule, *_mol, sgroup);
        }
    }
}

void CmlSaver::_addSgroupElement(XMLElement* molecule, BaseMolecule& mol, SGroup& sgroup)
{
    XMLElement* sg = _doc->NewElement("molecule");
    molecule->LinkEndChild(sg);

    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    out.printf("sg%d", sgroup.original_group);
    buf.push(0);
    sg->SetAttribute("id", buf.ptr());

    if (sgroup.atoms.size() > 0)
    {
        QS_DEF(Array<char>, sbuf);
        ArrayOutput sout(sbuf);

        for (int j = 0; j < sgroup.atoms.size(); j++)
            sout.printf("a%d ", sgroup.atoms[j]);

        sbuf.pop();
        sbuf.push(0);

        sg->SetAttribute("atomRefs", sbuf.ptr());
    }

    if (sgroup.brackets.size() > 0)
    {
        XMLElement* brks = _doc->NewElement("MBracket");
        sg->LinkEndChild(brks);

        if (sgroup.brk_style == 0)
            brks->SetAttribute("type", "SQUARE");
        else
            brks->SetAttribute("type", "ROUND");

        for (int i = 0; i < sgroup.brackets.size(); i++)
        {
            XMLElement* pnt0 = _doc->NewElement("MPoint");
            brks->LinkEndChild(pnt0);
            pnt0->SetAttribute("x", sgroup.brackets[i][0].x);
            pnt0->SetAttribute("y", sgroup.brackets[i][0].y);

            XMLElement* pnt1 = _doc->NewElement("MPoint");
            brks->LinkEndChild(pnt1);
            pnt1->SetAttribute("x", sgroup.brackets[i][1].x);
            pnt1->SetAttribute("y", sgroup.brackets[i][1].y);
        }
    }

    if (sgroup.sgroup_type == SGroup::SG_TYPE_DAT)
    {
        sg->SetAttribute("role", "DataSgroup");

        DataSGroup& dsg = (DataSGroup&)sgroup;

        const char* name = dsg.name.ptr();
        if (name != 0 && strlen(name) > 0)
        {
            sg->SetAttribute("fieldName", name);
        }
        const char* desc = dsg.description.ptr();
        if (desc != 0 && strlen(desc) > 0)
        {
            sg->SetAttribute("fieldType", desc);
        }
        const char* querycode = dsg.querycode.ptr();
        if (querycode != 0 && strlen(querycode) > 0)
        {
            sg->SetAttribute("queryType", querycode);
        }
        const char* queryoper = dsg.queryoper.ptr();
        if (queryoper != 0 && strlen(queryoper) > 0)
        {
            sg->SetAttribute("queryOp", queryoper);
        }

        sg->SetAttribute("x", dsg.display_pos.x);
        sg->SetAttribute("y", dsg.display_pos.y);

        if (!dsg.detached)
        {
            sg->SetAttribute("dataDetached", "false");
        }

        if (dsg.relative)
        {
            sg->SetAttribute("placement", "Relative");
        }

        if (dsg.display_units)
        {
            sg->SetAttribute("unitsDisplayed", "Unit displayed");
        }

        char tag = dsg.tag;
        if (tag != 0 && tag != ' ')
        {
            sg->SetAttribute("tag", tag);
        }

        if (dsg.num_chars > 0)
        {
            sg->SetAttribute("displayedChars", dsg.num_chars);
        }

        if (dsg.data.size() > 0 && dsg.data[0] != 0)
        {
            sg->SetAttribute("fieldData", dsg.data.ptr());
        }

        MoleculeSGroups* sgroups = &mol.sgroups;

        for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            SGroup& sg_child = sgroups->getSGroup(i);

            if ((sg_child.parent_group != 0) && (sg_child.parent_group == sgroup.original_group))
                _addSgroupElement(sg, mol, sg_child);
        }
    }
    else if (sgroup.sgroup_type == SGroup::SG_TYPE_GEN)
    {
        sg->SetAttribute("role", "GenericSgroup");

        MoleculeSGroups* sgroups = &mol.sgroups;

        for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            SGroup& sg_child = sgroups->getSGroup(i);

            if ((sg_child.parent_group != 0) && (sg_child.parent_group == sgroup.original_group))
                _addSgroupElement(sg, mol, sg_child);
        }
    }
    else if (sgroup.sgroup_type == SGroup::SG_TYPE_SUP)
    {
        sg->SetAttribute("role", "SuperatomSgroup");

        Superatom& sup = (Superatom&)sgroup;

        const char* name = sup.subscript.ptr();
        if (name != 0 && strlen(name) > 0)
        {
            sg->SetAttribute("title", name);
        }

        MoleculeSGroups* sgroups = &mol.sgroups;

        for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            SGroup& sg_child = sgroups->getSGroup(i);

            if ((sg_child.parent_group != 0) && (sg_child.parent_group == sgroup.original_group))
                _addSgroupElement(sg, mol, sg_child);
        }
    }
    else if (sgroup.sgroup_type == SGroup::SG_TYPE_SRU)
    {
        sg->SetAttribute("role", "SruSgroup");

        RepeatingUnit& sru = (RepeatingUnit&)sgroup;

        const char* name = sru.subscript.ptr();
        if (name != 0 && strlen(name) > 0)
        {
            sg->SetAttribute("title", name);
        }

        if (sru.connectivity == SGroup::HEAD_TO_TAIL)
        {
            sg->SetAttribute("connect", "ht");
        }
        else if (sru.connectivity == SGroup::HEAD_TO_HEAD)
        {
            sg->SetAttribute("connect", "hh");
        }

        MoleculeSGroups* sgroups = &mol.sgroups;

        for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            SGroup& sg_child = sgroups->getSGroup(i);

            if ((sg_child.parent_group != 0) && (sg_child.parent_group == sgroup.original_group))
                _addSgroupElement(sg, mol, sg_child);
        }
    }
    else if (sgroup.sgroup_type == SGroup::SG_TYPE_MUL)
    {
        sg->SetAttribute("role", "MultipleSgroup");

        MultipleGroup& mul = (MultipleGroup&)sgroup;

        if (mul.multiplier > 0)
        {
            sg->SetAttribute("title", mul.multiplier);
        }

        if (mul.parent_atoms.size() > 0)
        {
            QS_DEF(Array<char>, pbuf);
            ArrayOutput pout(pbuf);

            for (int j = 0; j < mul.parent_atoms.size(); j++)
                pout.printf("a%d ", mul.parent_atoms[j]);

            pbuf.pop();
            pbuf.push(0);

            sg->SetAttribute("patoms", pbuf.ptr());
        }

        MoleculeSGroups* sgroups = &mol.sgroups;

        for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
        {
            SGroup& sg_child = sgroups->getSGroup(i);

            if ((sg_child.parent_group != 0) && (sg_child.parent_group == sgroup.original_group))
                _addSgroupElement(sg, mol, sg_child);
        }
    }
}

void CmlSaver::_addRgroups(XMLElement* elem, BaseMolecule& mol, bool query)
{
    if (mol.rgroups.getRGroupCount() > 0)
    {
        MoleculeRGroups& rgroups = mol.rgroups;
        int n_rgroups = rgroups.getRGroupCount();

        for (int i = 1; i <= n_rgroups; i++)
        {
            RGroup& rgroup = rgroups.getRGroup(i);

            if (rgroup.fragments.size() == 0)
                continue;

            XMLElement* rg = _doc->NewElement("Rgroup");
            if (elem == 0)
                _doc->LinkEndChild(rg);
            else
                elem->LinkEndChild(rg);

            rg->SetAttribute("rgroupID", i);

            if (rgroup.if_then > 0)
                rg->SetAttribute("thenR", rgroup.if_then);

            if (rgroup.rest_h > 0)
                rg->SetAttribute("restH", rgroup.rest_h);

            QS_DEF(Array<char>, buf);
            ArrayOutput out(buf);

            _writeOccurrenceRanges(out, rgroup.occurrence);

            if (buf.size() > 1)
                rg->SetAttribute("rlogicRange", buf.ptr());

            _addRgroupElement(rg, rgroup, query);
        }
    }
}

void CmlSaver::_addRgroupElement(XMLElement* elem, RGroup& rgroup, bool query)
{
    PtrPool<BaseMolecule>& frags = rgroup.fragments;

    for (int i = frags.begin(); i != frags.end(); i = frags.next(i))
    {
        BaseMolecule* fragment = frags[i];

        _addMoleculeElement(elem, *fragment, query);
    }
}

void CmlSaver::_writeOccurrenceRanges(Output& out, const Array<int>& occurrences)
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
    out.writeChar(0);
}

bool CmlSaver::_getRingBondCountFlagValue(QueryMolecule& qmol, int idx, int& value)
{
    QueryMolecule::Atom& atom = qmol.getAtom(idx);
    int rbc;
    if (atom.hasConstraint(QueryMolecule::ATOM_RING_BONDS))
    {
        if (atom.sureValue(QueryMolecule::ATOM_RING_BONDS, rbc))
        {
            value = rbc;
            if (value == 0)
                value = -1;
            return true;
        }
        int rbc_values[1] = {4};
        if (atom.sureValueBelongs(QueryMolecule::ATOM_RING_BONDS, rbc_values, 1))
        {
            value = 4;
            return true;
        }
    }
    else if (atom.sureValue(QueryMolecule::ATOM_RING_BONDS_AS_DRAWN, rbc))
    {
        value = -2;
        return true;
    }
    return false;
}

bool CmlSaver::_getSubstitutionCountFlagValue(QueryMolecule& qmol, int idx, int& value)
{
    QueryMolecule::Atom& atom = qmol.getAtom(idx);
    int v;
    if (atom.hasConstraint(QueryMolecule::ATOM_SUBSTITUENTS))
    {
        if (atom.sureValue(QueryMolecule::ATOM_SUBSTITUENTS, v))
        {
            value = v;
            if (value == 0)
                value = -1;
            return true;
        }
        int values[1] = {6};
        if (atom.sureValueBelongs(QueryMolecule::ATOM_SUBSTITUENTS, values, 1))
        {
            value = 6;
            return true;
        }
    }
    else if (atom.sureValue(QueryMolecule::ATOM_SUBSTITUENTS_AS_DRAWN, v))
    {
        value = -2;
        return true;
    }
    return false;
}
