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

#include <codecvt>
#include <tinyxml2.h>

#include "base_cpp/locale_guard.h"
#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cdxml_loader.h"
#include "molecule/molecule_cdxml_saver.h"
#include "molecule/query_molecule.h"

using namespace indigo;
using namespace tinyxml2;

IMPL_ERROR(MoleculeCdxmlSaver, "molecule CDXML saver");

int MoleculeCdxmlSaver::getId()
{
    return _id;
}

MoleculeCdxmlSaver::MoleculeCdxmlSaver(Output& output) : _output(output)
{
    _bond_length = SCALE;
    _max_page_height = MAX_PAGE_HEIGHT;
    _pages_height = 1;
    _id = 0;
}

MoleculeCdxmlSaver::~MoleculeCdxmlSaver()
{
}

float MoleculeCdxmlSaver::pageHeight() const
{
    return _max_page_height;
}

float MoleculeCdxmlSaver::textLineHeight() const
{
    return 12.75f / _bond_length;
}

void MoleculeCdxmlSaver::beginDocument(Bounds* bounds)
{
    _doc = std::make_unique<XMLDocument>();
    _root = _doc->NewElement("CDXML");

    _doc->LinkEndChild(_doc->NewDeclaration());
    XMLUnknown* doctype = _doc->NewUnknown(R"(!DOCTYPE CDXML SYSTEM "http://www.cambridgesoft.com/xml/cdxml.dtd")");
    _doc->LinkEndChild(doctype);

    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    out.printf("%f", _bond_length);
    buf.push(0);

    _root->SetAttribute("BondLength", buf.ptr());
    _root->SetAttribute("LabelFont", "3");
    _root->SetAttribute("CaptionFont", "4");

    _doc->LinkEndChild(_root);

    if (bounds != NULL)
    {
        // Generate MacPrintInfo according to the size
        // http://www.cambridgesoft.com/services/documentation/sdk/chemdraw/cdx/properties/MacPrintInfo.htm

        int dpi_logical = 72;
        int dpi_print = 600;

        float x_inch = bounds->max.x * _bond_length / dpi_logical + 1;
        float y_inch = bounds->max.y * _bond_length / dpi_logical + 1;

        int width = (int)(x_inch * dpi_print);
        int height = (int)(y_inch * dpi_print);

        // Add 1 to compensate margins = 36 points = 0.5 inches
        int max_height = (int)((_max_page_height * _bond_length / dpi_logical + 1) * dpi_print);
        if (height > max_height)
        {
            _pages_height = (int)ceil((float)height / max_height);
            height = max_height;
        }

        int mac_print_info[60] = {0};
        mac_print_info[0] = 3; // magic number
        mac_print_info[2] = dpi_print;
        mac_print_info[3] = dpi_print;

        mac_print_info[6] = height;
        mac_print_info[7] = width;

        mac_print_info[10] = height;
        mac_print_info[11] = width;

        mac_print_info[12] = 871; // magic number

        mac_print_info[13] = height / 5; // magic scaling coeffient
        mac_print_info[14] = width / 5;

        mac_print_info[24] = 100; // horizontal scale, in percent
        mac_print_info[25] = 100; // Vertical scale, in percent

        _root->SetAttribute("PrintMargins", "36 36 36 36");

        buf.clear();
        for (int i = 0; i < NELEM(mac_print_info); i++)
        {
            out.printf("%04hx", (unsigned short)mac_print_info[i]);
        }
        buf.push(0);
        _root->SetAttribute("MacPrintInfo", buf.ptr());
    }
    _current = _root;
}

void MoleculeCdxmlSaver::beginPage(Bounds* bounds)
{
    _page = _doc->NewElement("page");
    _root->LinkEndChild(_page);
    _page->SetAttribute("HeightPages", _pages_height);
    _page->SetAttribute("WidthPages", 1);
    _current = _page;
}

void MoleculeCdxmlSaver::addFontTable(const char* font)
{
    if (font != NULL && strlen(font) > 0)
    {
        _fonttable = _doc->NewElement("fonttable");
        _root->LinkEndChild(_fonttable);

        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        buf.readString(&font[1], false);
        buf.remove(buf.size() - 1);
        buf.push(0);
        XMLUnknown* f = _doc->NewUnknown(buf.ptr());
        _fonttable->LinkEndChild(f);
    }
}

void MoleculeCdxmlSaver::addFontToTable(int id, const char* charset, const char* name)
{
    XMLElement* font = _doc->NewElement("font");
    _fonttable->LinkEndChild(font);
    if (id > 0)
        font->SetAttribute("id", id);
    font->SetAttribute("charset", charset);
    font->SetAttribute("name", name);
}

void MoleculeCdxmlSaver::addColorTable(const char* color)
{
    if (color != NULL && strlen(color) > 0)
    {
        _colortable = _doc->NewElement("colortable");
        _root->LinkEndChild(_colortable);

        addColorToTable(-1, 1, 1, 1);
        addColorToTable(-1, 0, 0, 0);
        addColorToTable(-1, 1, 0, 0);
        addColorToTable(-1, 1, 1, 0);
        addColorToTable(-1, 0, 1, 0);
        addColorToTable(-1, 0, 1, 1);
        addColorToTable(-1, 0, 0, 1);
        addColorToTable(-1, 1, 0, 1);

        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        buf.readString(&color[1], false);
        buf.remove(buf.size() - 1);
        buf.push(0);

        XMLUnknown* c = _doc->NewUnknown(buf.ptr());
        _colortable->LinkEndChild(c);
    }
}

void MoleculeCdxmlSaver::addColorToTable(int id, int r, int g, int b)
{
    XMLElement* color = _doc->NewElement("color");
    _colortable->LinkEndChild(color);
    if (id > 0)
        color->SetAttribute("id", id);
    color->SetAttribute("r", r);
    color->SetAttribute("g", g);
    color->SetAttribute("b", b);
}

void MoleculeCdxmlSaver::addDefaultFontTable()
{
    Array<char> name;
    PropertiesMap attrs;

    name.clear();
    attrs.clear();

    name.readString("fonttable", true);
    startCurrentElement(++_id, name, attrs);

    name.readString("font", true);
    attrs.insert("charset", "utf-8");
    attrs.insert("name", "Arial");
    addCustomElement(++_id, name, attrs);

    attrs.clear();
    attrs.insert("charset", "utf-8");
    attrs.insert("name", "Times New Roman");
    addCustomElement(++_id, name, attrs);

    endCurrentElement();
}

void MoleculeCdxmlSaver::addDefaultColorTable()
{
    Array<char> color;
    ArrayOutput color_out(color);

    color_out.printf(R"(<color r="0.5" g="0.5" b="0.5"/>)");
    color.push(0);

    addColorTable(color.ptr());
}

void MoleculeCdxmlSaver::addNodeToFragment(BaseMolecule& mol, XMLElement* fragment, int atom_idx, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord,
                                           Vec2f& node_pos)
{
    Vec3f pos3 = mol.getAtomXyz(atom_idx);
    Vec2f pos(pos3.x, pos3.y);

    pos.add(offset);
    if (atom_idx == mol.vertexBegin())
        min_coord = max_coord = pos;
    else
    {
        min_coord.min(pos);
        max_coord.max(pos);
    }

    pos.scale(_scale);
    node_pos.set(pos.x, -pos.y);

    int atom_number = mol.getAtomNumber(atom_idx);
    int charge = mol.getAtomCharge(atom_idx);
    int radical = 0;
    int hcount = -1;

    XMLElement* node = _doc->NewElement("n");
    fragment->LinkEndChild(node);

    node->SetAttribute("id", _atoms_ids[atom_idx]);

    if (mol.isRSite(atom_idx))
    {
        node->SetAttribute("NodeType", "GenericNickname");
        node->SetAttribute("GenericNickname", "A");

        if ((charge != 0) && (charge != CHARGE_UNKNOWN))
            node->SetAttribute("Charge", charge);
    }
    else if (mol.isPseudoAtom(atom_idx))
    {
        node->SetAttribute("NodeType", "GenericNickname");
        node->SetAttribute("GenericNickname", mol.getPseudoAtom(atom_idx));

        if ((charge != 0) && (charge != CHARGE_UNKNOWN))
            node->SetAttribute("Charge", charge);
    }
    else if (atom_number > 0)
    {
        if (atom_number != ELEM_C)
            node->SetAttribute("Element", atom_number);

        if ((charge != 0) && (charge != CHARGE_UNKNOWN))
            node->SetAttribute("Charge", charge);

        if (mol.getAtomIsotope(atom_idx) > 0)
            node->SetAttribute("Isotope", mol.getAtomIsotope(atom_idx));

        radical = mol.getAtomRadical_NoThrow(atom_idx, 0);
        if (radical > 0)
        {
            const char* radical_str = NULL;
            if (radical == RADICAL_DOUBLET)
                radical_str = "Doublet";
            else if (radical == RADICAL_SINGLET)
                radical_str = "Singlet";
            else if (radical == RADICAL_TRIPLET)
                radical_str = "Triplet";
            else
                throw Error("Radical type %d is not supported", radical);

            node->SetAttribute("Radical", radical_str);
        }

        if ((atom_number != ELEM_C) && (atom_number != ELEM_H))
        {
            try
            {
                hcount = getHydrogenCount(mol, atom_idx, charge, radical);
            }
            catch (Exception&)
            {
                hcount = -1;
            }

            if (hcount >= 0)
                node->SetAttribute("NumHydrogens", hcount);
        }
    }
    else if (atom_number < 0)
    {
        QS_DEF(Array<int>, list);
        int query_atom_type;
        if (mol.isQueryMolecule() && (query_atom_type = QueryMolecule::parseQueryAtom(mol.asQueryMolecule(), atom_idx, list)) != -1)
        {
            if (query_atom_type == QueryMolecule::QUERY_ATOM_A)
            {
                node->SetAttribute("NodeType", "GenericNickname");
                node->SetAttribute("GenericNickname", "A");
            }
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_Q)
            {
                node->SetAttribute("NodeType", "GenericNickname");
                node->SetAttribute("GenericNickname", "Q");
            }
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_X)
            {
                node->SetAttribute("NodeType", "GenericNickname");
                node->SetAttribute("GenericNickname", "X");
            }
            else if (query_atom_type == QueryMolecule::QUERY_ATOM_LIST || query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
            {
                int k;

                QS_DEF(Array<char>, buf);
                ArrayOutput out(buf);

                if (query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
                    out.writeString("NOT ");

                for (k = 0; k < list.size(); k++)
                {
                    out.printf("%d ", list[k]);
                }
                buf.pop();
                buf.push(0);

                node->SetAttribute("NodeType", "ElementList");
                node->SetAttribute("ElementList", buf.ptr());
            }
        }
    }

    if (mol.have_xyz)
    {
        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        out.printf("%f %f", pos.x, -pos.y);
        buf.push(0);
        node->SetAttribute("p", buf.ptr());
    }

    int enh_stereo_type = mol.stereocenters.getType(atom_idx);
    if (enh_stereo_type > MoleculeStereocenters::ATOM_ANY)
    {
        int enh_stereo_grp = mol.stereocenters.getGroup(atom_idx);

        node->SetAttribute("Geometry", "Tetrahedral");

        const int* pyramid = mol.stereocenters.getPyramid(atom_idx);
        // 0 means atom absence
        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        for (int i = 0; i < 4; ++i)
        {
            if (i)
                out.printf(" ");
            out.printf("%d", _atoms_ids[pyramid[i] < 0 ? 0 : pyramid[i]]);
        }

        buf.push(0);
        node->SetAttribute("BondOrdering", buf.ptr());
        switch (enh_stereo_type)
        {
        case MoleculeStereocenters::ATOM_ABS:
            node->SetAttribute("EnhancedStereoType", "Absolute");
            break;
        case MoleculeStereocenters::ATOM_OR:
            node->SetAttribute("EnhancedStereoType", "Or");
            break;
        case MoleculeStereocenters::ATOM_AND:
            node->SetAttribute("EnhancedStereoType", "And");
            break;
        default:
            throw Error("Unknows enhanced stereo type %d", enh_stereo_type);
            break;
        }
        if (enh_stereo_grp > 0)
            node->SetAttribute("EnhancedStereoGroupNum", enh_stereo_grp);
    }

    if (mol.getVertex(atom_idx).degree() == 0 && atom_number == ELEM_C && charge == 0 && radical == 0)
    {
        XMLElement* t = _doc->NewElement("t");
        node->LinkEndChild(t);

        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        out.printf("%f %f", pos.x, -pos.y);
        buf.push(0);
        t->SetAttribute("p", buf.ptr());
        t->SetAttribute("Justification", "Center");

        XMLElement* s = _doc->NewElement("s");
        t->LinkEndChild(s);
        s->SetAttribute("font", 3);
        s->SetAttribute("size", 10);
        s->SetAttribute("face", 96);

        XMLText* txt = _doc->NewText("CH4");
        s->LinkEndChild(txt);
    }
    else if (mol.isRSite(atom_idx))
    {
        XMLElement* t = _doc->NewElement("t");
        node->LinkEndChild(t);

        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        out.printf("%f %f", pos.x, -pos.y);
        buf.push(0);
        t->SetAttribute("p", buf.ptr());
        t->SetAttribute("LabelJustification", "Left");

        XMLElement* s = _doc->NewElement("s");
        t->LinkEndChild(s);
        s->SetAttribute("font", 3);
        s->SetAttribute("size", 10);
        s->SetAttribute("face", 96);

        out.clear();
        //			out.printf("A");
        mol.getAtomSymbol(atom_idx, buf);
        /*
         * Skip charge since Chemdraw is pure. May be in future it will be fixed by Chemdraw
         */
        /*if (charge != 0) {
            if (charge > 0) {
                out.printf("+%d", charge);
            }
            else {
                out.printf("-%d", charge);
            }
        }*/
        buf.push(0);

        XMLText* txt = _doc->NewText(buf.ptr());
        s->LinkEndChild(txt);
    }
    else if (mol.isPseudoAtom(atom_idx))
    {
        XMLElement* t = _doc->NewElement("t");
        node->LinkEndChild(t);

        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        out.printf("%f %f", pos.x, -pos.y);
        buf.push(0);
        t->SetAttribute("p", buf.ptr());
        t->SetAttribute("LabelJustification", "Left");

        XMLElement* s = _doc->NewElement("s");
        t->LinkEndChild(s);
        s->SetAttribute("font", 3);
        s->SetAttribute("size", 10);
        s->SetAttribute("face", 96);

        out.clear();

        out.printf("%s", mol.getPseudoAtom(atom_idx));
        /*
         * Skip charge since Chemdraw is pure. May be in future it will be fixed by Chemdraw
         */
        /*if (charge != 0) {
            if (charge > 0) {
                out.printf("+%d", charge);
            }
            else {
                out.printf("-%d", charge);
            }
        }*/
        buf.push(0);
        XMLText* txt = _doc->NewText(buf.ptr());
        s->LinkEndChild(txt);
    }
    else if (atom_number > 0 && atom_number != ELEM_C)
    {
        XMLElement* t = _doc->NewElement("t");
        node->LinkEndChild(t);

        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        out.printf("%f %f", pos.x, -pos.y);
        buf.push(0);
        t->SetAttribute("p", buf.ptr());
        t->SetAttribute("LabelJustification", "Left");

        XMLElement* s = _doc->NewElement("s");
        t->LinkEndChild(s);
        s->SetAttribute("font", 3);
        s->SetAttribute("size", 10);
        s->SetAttribute("face", 96);

        out.clear();
        mol.getAtomSymbol(atom_idx, buf);
        if (hcount > 0)
        {
            buf.pop();
            buf.push('H');
        }

        buf.push(0);
        XMLText* txt = _doc->NewText(buf.ptr());
        s->LinkEndChild(txt);
        if (hcount > 1)
        {
            XMLElement* s = _doc->NewElement("s");
            t->LinkEndChild(s);
            s->SetAttribute("font", 3);
            s->SetAttribute("size", 10);
            s->SetAttribute("face", 32);

            out.clear();
            out.printf("%d", hcount);
            buf.push(0);
            XMLText* txt = _doc->NewText(buf.ptr());
            s->LinkEndChild(txt);
        }
    }
    else if (atom_number < 0 && mol.isQueryMolecule())
    {
        XMLElement* t = _doc->NewElement("t");
        node->LinkEndChild(t);

        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        out.printf("%f %f", pos.x, -pos.y);
        buf.push(0);
        t->SetAttribute("p", buf.ptr());
        t->SetAttribute("LabelJustification", "Left");

        XMLElement* s = _doc->NewElement("s");
        t->LinkEndChild(s);
        s->SetAttribute("font", 3);
        s->SetAttribute("size", 10);
        s->SetAttribute("face", 96);

        QS_DEF(Array<int>, list);
        int query_atom_type;

        out.clear();

        if (mol.isQueryMolecule() && (query_atom_type = QueryMolecule::parseQueryAtom(mol.asQueryMolecule(), atom_idx, list)) != -1)
        {
            if (query_atom_type == QueryMolecule::QUERY_ATOM_LIST || query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
            {
                int k;

                if (query_atom_type == QueryMolecule::QUERY_ATOM_NOTLIST)
                    out.writeString("NOT ");

                for (k = 0; k < list.size(); k++)
                {
                    if (k > 0)
                        out.writeChar(',');
                    out.writeString(Element::toString(list[k]));
                }
                buf.push(0);
            }
            else
                mol.getAtomSymbol(atom_idx, buf);
        }

        XMLText* txt = _doc->NewText(buf.ptr());
        s->LinkEndChild(txt);
    }
}

void MoleculeCdxmlSaver::_collectSuperatoms(BaseMolecule& mol)
{
    _atoms_excluded.clear();
    _bonds_excluded.clear();
    _bonds_included.clear();
    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sgroup = mol.sgroups.getSGroup(i);
        if (sgroup.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            _super_atoms.emplace(i, std::vector<int>{});
            auto& atoms_list = _super_atoms.at(i);
            Superatom& sa = (Superatom&)sgroup;
            for (int j = 0; j < sa.atoms.size(); ++j)
            {
                _atoms_excluded.insert(sa.atoms[j]);
                atoms_list.push_back(sa.atoms[j]);
            }
        }
    }

    if (_atoms_excluded.size())
        for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
        {
            auto& edge = mol.getEdge(i);
            int v_count = 0;
            if (_atoms_excluded.find(edge.beg) != _atoms_excluded.end())
                v_count++;
            if (_atoms_excluded.find(edge.end) != _atoms_excluded.end())
                v_count++;

            if (v_count)
                _bonds_excluded.insert(i);

            if (v_count == 2) // 2 means that both bond's atoms belongs to superatom
                _bonds_included.insert(i);
        }
}

void MoleculeCdxmlSaver::addBondToFragment(BaseMolecule& mol, tinyxml2::XMLElement* fragment, int bond_idx)
{
    const Edge& edge = mol.getEdge(bond_idx);

    XMLElement* bond = _doc->NewElement("b");
    fragment->LinkEndChild(bond);
    bond->SetAttribute("id", _bonds_ids[bond_idx]);
    bond->SetAttribute("B", _atoms_ids[edge.beg]);
    bond->SetAttribute("E", _atoms_ids[edge.end]);

    int order = mol.getBondOrder(bond_idx);

    if (order == BOND_DOUBLE || order == BOND_TRIPLE)
        bond->SetAttribute("Order", order);
    else if (order == BOND_AROMATIC)
    {
        bond->SetAttribute("Order", "1.5");
        bond->SetAttribute("Display", "Dash");
        bond->SetAttribute("Display2", "Dash");
    }
    else
        ; // Do not write single bond order

    int dir = mol.getBondDirection(bond_idx);
    int parity = mol.cis_trans.getParity(bond_idx);

    if (mol.have_xyz && (dir == BOND_UP || dir == BOND_DOWN))
    {
        bond->SetAttribute("Display", (dir == BOND_UP) ? "WedgeBegin" : "WedgedHashBegin");
    }
    else if (!mol.have_xyz && parity != 0)
    {
        const int* subst = mol.cis_trans.getSubstituents(bond_idx);

        int s1, s2, s3, s4;
        s1 = _atoms_ids[subst[0]], s2 = _atoms_ids[subst[1]];
        s3 = _atoms_ids[subst[2]], s4 = _atoms_ids[subst[3]];
        if (parity == MoleculeCisTrans::TRANS)
        {
            std::swap(s3, s4);
        }
        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        out.printf("%d %d %d %d", s1, s2, s3, s4);
        buf.push(0);
        bond->SetAttribute("BondCircularOrdering", buf.ptr());
    }
}

void MoleculeCdxmlSaver::addBondsToFragment(BaseMolecule& mol, tinyxml2::XMLElement* fragment)
{
    for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        if (_bonds_excluded.find(i) == _bonds_excluded.end())
            addBondToFragment(mol, fragment, i);
    }
}

void MoleculeCdxmlSaver::addNodesToFragment(BaseMolecule& mol, XMLElement* fragment, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord)
{
    Vec2f dummy_pos;
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (_atoms_excluded.find(i) == _atoms_excluded.end()) // skip atoms from superatoms
            addNodeToFragment(mol, fragment, i, offset, min_coord, max_coord, dummy_pos);
    }
}

void MoleculeCdxmlSaver::addFragmentNodes(BaseMolecule& mol, tinyxml2::XMLElement* fragment, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord)
{
    for (auto& kvp : _super_atoms)
    {
        std::vector<std::pair<int, int>> ext_connections;
        std::vector<int> connection_order, bond_ordering;
        std::vector<int> int_connections;
        XMLElement* node = _doc->NewElement("n");
        fragment->LinkEndChild(node);
        node->SetAttribute("id", ++_id);
        int fragment_node_id = _id;
        node->SetAttribute("NodeType", "Fragment");
        XMLElement* super_fragment = _doc->NewElement("fragment");
        super_fragment->SetAttribute("id", ++_id);
        node->LinkEndChild(super_fragment);
        for (auto atom_idx : kvp.second)
        {
            Vec2f pos;
            addNodeToFragment(mol, super_fragment, atom_idx, offset, min_coord, max_coord, pos);
            auto& vx = mol.getVertex(atom_idx);
            for (auto nei_idx = vx.neiBegin(); nei_idx != vx.neiEnd(); nei_idx = vx.neiNext(nei_idx))
            {
                int nei_atom_idx = vx.neiVertex(nei_idx);
                int nei_edge_idx = vx.neiEdge(nei_idx);
                if (_atoms_excluded.find(nei_atom_idx) == _atoms_excluded.end())
                {
                    // external neighbor found
                    XMLElement* connection_node = _doc->NewElement("n");
                    super_fragment->LinkEndChild(connection_node);
                    connection_node->SetAttribute("id", ++_id);
                    connection_node->SetAttribute("NodeType", "ExternalConnectionPoint");
                    ext_connections.emplace_back(_id, _atoms_ids[atom_idx]);
                    connection_order.push_back(_id);
                    bond_ordering.push_back(++_id);
                    _out_connections.emplace_back(_id, _atoms_ids[nei_atom_idx], fragment_node_id);
                }

                if (_bonds_included.find(nei_edge_idx) != _bonds_included.end())
                    int_connections.push_back(nei_edge_idx);
            }
        }

        for (int edge_idx : int_connections)
            addBondToFragment(mol, super_fragment, edge_idx);

        for (const auto& ext_bond : ext_connections)
        {
            XMLElement* bond = _doc->NewElement("b");
            super_fragment->LinkEndChild(bond);
            bond->SetAttribute("id", ++_id);
            bond->SetAttribute("B", ext_bond.first);
            bond->SetAttribute("E", ext_bond.second);
        }

        if (connection_order.size() > 1)
        {
            std::string order;
            for (int i = 0; i < connection_order.size(); ++i)
            {
                if (i)
                    order += " ";
                order += std::to_string(connection_order[i]);
            }
            super_fragment->SetAttribute("ConnectionOrder", order.c_str());
        }

        if (bond_ordering.size() > 1)
        {
            std::string order;
            for (int i = 0; i < bond_ordering.size(); ++i)
            {
                if (i)
                    order += " ";
                order += std::to_string(bond_ordering[i]);
            }
            node->SetAttribute("BondOrdering", order.c_str());
        }

        auto& sa = (Superatom&)mol.sgroups.getSGroup(kvp.first);
        XMLElement* t = _doc->NewElement("t");
        node->LinkEndChild(t);
        t->SetAttribute("LabelJustification", "Left");
        t->SetAttribute("LabelAlignment", "Above");
        XMLElement* s = _doc->NewElement("s");
        t->LinkEndChild(s);
        XMLText* txt = _doc->NewText(sa.subscript.ptr());
        s->LinkEndChild(txt);
    }
}

void MoleculeCdxmlSaver::saveMoleculeFragment(BaseMolecule& mol, const Vec2f& offset, float scale)
{
    std::vector<int> ids;
    int id = 0;
    saveMoleculeFragment(mol, offset, scale, -1, id, ids);
}

void MoleculeCdxmlSaver::saveMoleculeFragment(BaseMolecule& mol, const Vec2f& offset, float structure_scale, int frag_id, int& id, std::vector<int>& ids)
{
    _atoms_ids.clear();
    _bonds_ids.clear();
    _super_atoms.clear();
    _atoms_excluded.clear();
    _bonds_excluded.clear();
    _bonds_included.clear();
    _out_connections.clear();

    _scale = structure_scale * _bond_length;

    LocaleGuard locale_guard;

    XMLElement* parent = _current;
    XMLElement* fragment = _doc->NewElement("fragment");
    _current->LinkEndChild(fragment);
    _current = fragment;

    if (frag_id > 0)
    {
        fragment->SetAttribute("id", frag_id);
        _id = id;
    }
    else
        fragment->SetAttribute("id", ++_id);

    if (ids.size())
    {
        _atoms_ids = ids;
        if (_atoms_ids.back() > _id)
            _id = _atoms_ids.back();
    }
    else
        for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
            _atoms_ids.push_back(++_id);

    for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
        _bonds_ids.push_back(++_id);

    Vec2f min_coord, max_coord;

    _collectSuperatoms(mol);
    addFragmentNodes(mol, fragment, offset, min_coord, max_coord);
    addNodesToFragment(mol, fragment, offset, min_coord, max_coord);
    addBondsToFragment(mol, fragment);

    for (const auto& out_bond : _out_connections)
    {
        XMLElement* bond = _doc->NewElement("b");
        fragment->LinkEndChild(bond);
        bond->SetAttribute("id", out_bond.id);
        bond->SetAttribute("B", out_bond.beg);
        bond->SetAttribute("E", out_bond.end);
    }

    if (mol.isChiral())
    {
        Vec2f chiral_pos(max_coord.x, max_coord.y);
        Vec2f bbox(_scale * chiral_pos.x, -_scale * chiral_pos.y);

        XMLElement* graphic = _doc->NewElement("graphic");
        fragment->LinkEndChild(graphic);

        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        out.printf("%f %f %f %f", bbox.x, bbox.y, bbox.x, bbox.y);
        buf.push(0);
        graphic->SetAttribute("BoundingBox", buf.ptr());
        graphic->SetAttribute("GraphicType", "Symbol");
        graphic->SetAttribute("SymbolType", "Absolute");
        graphic->SetAttribute("FrameType", "None");

        _current = graphic;
        addText(chiral_pos, "Chiral");
        _current = fragment;
    }

    for (int i = 0; i < mol.meta().metaData().size(); ++i)
        addMetaObject(*mol.meta().metaData()[i], ++_id);

    _current = parent;
    id = _id;
}

void MoleculeCdxmlSaver::addArrow(int id, int arrow_type, const Vec2f& beg, const Vec2f& end)
{
    PropertiesMap attrs;
    attrs.insert("FillType", "None");
    attrs.insert("ArrowheadType", "Solid");
    attrs.insert("HeadSize", "2250");
    attrs.insert("ArrowheadWidth", "563");
    switch (arrow_type)
    {
    case KETReactionArrow::EOpenAngle:
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadCenterSize", "25");
        break;
    case KETReactionArrow::EFilledTriangle:
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadCenterSize", "2250");
        break;

    case KETReactionArrow::EFilledBow:
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadCenterSize", "1125");
        break;

    case KETReactionArrow::EDashedOpenAngle:
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadCenterSize", "25");
        attrs.insert("LineType", "Dashed");
        break;

    case KETReactionArrow::EFailed:
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadCenterSize", "1125");
        attrs.insert("NoGo", "Cross");
        break;

    case KETReactionArrow::EBothEndsFilledTriangle:
        attrs.insert("ArrowheadCenterSize", "2250");
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadTail", "Full");
        break;

    case KETReactionArrow::EEquilibriumFilledHalfBow:
        attrs.insert("ArrowheadHead", "HalfLeft");
        attrs.insert("ArrowheadTail", "HalfLeft");
        attrs.insert("ArrowheadCenterSize", "1125");
        attrs.insert("ArrowShaftSpacing", "300");
        break;

    case KETReactionArrow::EEquilibriumFilledTriangle:
        attrs.insert("ArrowheadHead", "HalfLeft");
        attrs.insert("ArrowheadTail", "HalfLeft");
        attrs.insert("ArrowheadCenterSize", "2250");
        attrs.insert("ArrowShaftSpacing", "300");
        break;

    case KETReactionArrow::EEquilibriumOpenAngle:
        attrs.insert("ArrowheadHead", "HalfLeft");
        attrs.insert("ArrowheadTail", "HalfLeft");
        attrs.insert("ArrowheadCenterSize", "25");
        attrs.insert("ArrowShaftSpacing", "300");
        break;

    case KETReactionArrow::EUnbalancedEquilibriumFilledHalfBow:
        break;

    case KETReactionArrow::EUnbalancedEquilibriumLargeFilledHalfBow:
        break;

    case KETReactionArrow::EUnbalancedEquilibriumOpenHalfAngle:
        break;

    case KETReactionArrow::EUnbalancedEquilibriumFilledHalfTriangle:
        break;

    case KETReactionArrow::EEllipticalArcFilledBow:
        break;

    case KETReactionArrow::EEllipticalArcFilledTriangle:
        break;

    case KETReactionArrow::EEllipticalArcOpenAngle:
        break;

    case KETReactionArrow::EEllipticalArcOpenHalfAngle:
        break;

    default:
        break;
    }

    Vec3f ar_beg(beg.x, -beg.y, 0);
    Vec3f ar_end(end.x, -end.y, 0);
    ar_beg.scale(_bond_length);
    ar_end.scale(_bond_length);

    attrs.insert("Head3D", std::to_string(ar_end.x) + " " + std::to_string(ar_end.y) + " " + std::to_string(ar_end.z));
    attrs.insert("Tail3D", std::to_string(ar_beg.x) + " " + std::to_string(ar_beg.y) + " " + std::to_string(ar_beg.z));
    addElement("arrow", id, end, beg, attrs);
}

void MoleculeCdxmlSaver::addMetaObject(const MetaObject& obj, int id)
{
    PropertiesMap attrs;
    attrs.clear();
    switch (obj._class_id)
    {
    case KETReactionArrow::CID: {
        KETReactionArrow& ar = (KETReactionArrow&)(obj);
        addArrow(id, ar._arrow_type, ar._begin, ar._end);
    }
    break;
    case KETReactionPlus::CID: {
        KETReactionPlus& rp = (KETReactionPlus&)(obj);
        attrs.insert("GraphicType", "Symbol");
        attrs.insert("SymbolType", "Plus");
        Vec2f v1(rp._pos.x, rp._pos.y - PLUS_HALF_HEIGHT / _bond_length);
        Vec2f v2(rp._pos.x, rp._pos.y + PLUS_HALF_HEIGHT / _bond_length);
        addElement("graphic", id, v1, v2, attrs);
    }
    break;
    case KETSimpleObject::CID: {
        KETSimpleObject& simple_obj = (KETSimpleObject&)obj;
        Rect2f bbox(simple_obj._coordinates.first, simple_obj._coordinates.second);
        switch (simple_obj._mode)
        {
        case KETSimpleObject::EKETEllipse: {
            auto ecenter = bbox.center();
            Vec2f maj_axis, min_axis;
            if (bbox.width() > bbox.height())
            {
                maj_axis.copy(bbox.rightMiddle());
                min_axis.copy(bbox.topMiddle());
            }
            else
            {
                maj_axis.copy(bbox.topMiddle());
                min_axis.copy(bbox.rightMiddle());
            }
            ecenter.scale(_bond_length);
            min_axis.scale(_bond_length);
            maj_axis.scale(_bond_length);
            ecenter.y = -ecenter.y;
            min_axis.y = -min_axis.y;
            maj_axis.y = -maj_axis.y;
            Rect2f bbox_new(ecenter, bbox.rightTop());
            bbox.copy(bbox_new);
            attrs.insert("Center3D", std::to_string(ecenter.x) + " " + std::to_string(ecenter.y));
            attrs.insert("MajorAxisEnd3D", std::to_string(maj_axis.x) + " " + std::to_string(maj_axis.y));
            attrs.insert("MinorAxisEnd3D", std::to_string(min_axis.x) + " " + std::to_string(min_axis.y));
            attrs.insert("GraphicType", "Oval");
        }
        break;
        case KETSimpleObject::EKETRectangle:
            attrs.insert("GraphicType", "Rectangle");
            break;
        case KETSimpleObject::EKETLine:
            attrs.insert("GraphicType", "Line");
            break;
        }
        addElement("graphic", id, bbox.leftBottom(), bbox.rightTop(), attrs);
    }
    break;
    case KETTextObject::CID: {
        const KETTextObject& ko = static_cast<const KETTextObject&>(obj);
        double text_offset_y = 0;
        int font_size = 13;
        CDXMLFontStyle font_face(0);
        for (auto& text_item : ko._block)
        {
            int first_index = -1;
            int second_index = -1;
            double text_offset_x = 0;
            FONT_STYLE_SET current_styles;
            Vec2f text_origin(ko._pos.x, ko._pos.y);
            std::string pos_str = std::to_string(_bond_length * text_origin.x) + " " + std::to_string(-_bond_length * text_origin.y);
            XMLElement* t = _doc->NewElement("t");
            _current->LinkEndChild(t);
            t->SetAttribute("id", id);
            t->SetAttribute("p", pos_str.c_str());
            t->SetAttribute("Justification", "Left");
            t->SetAttribute("InterpretChemically", "no");
            for (auto& kvp : text_item.styles)
            {
                if (first_index == -1)
                {
                    first_index = kvp.first;
                    current_styles = kvp.second;
                    continue;
                }
                second_index = kvp.first;

                std::wstring_convert<std::codecvt_utf8<wchar_t>> utf82w;
                std::wstring_convert<std::codecvt_utf8<wchar_t>> w2utf8;

                auto sub_text = w2utf8.to_bytes(utf82w.from_bytes(text_item.text).substr(first_index, second_index - first_index));
                for (const auto& text_style : current_styles)
                {
                    switch (text_style.first)
                    {
                    case KETTextObject::EPlain:
                        break;
                    case KETTextObject::EBold:
                        font_face.is_bold = text_style.second;
                        break;
                    case KETTextObject::EItalic:
                        font_face.is_italic = text_style.second;
                        break;
                    case KETTextObject::ESuperScript:
                        font_face.is_superscript = text_style.second;
                        break;
                    case KETTextObject::ESubScript:
                        font_face.is_subscript = text_style.second;
                        break;
                    default:
                        font_size = text_style.second ? text_style.first : 13;
                        break;
                    }
                }

                XMLElement* s = _doc->NewElement("s");
                t->LinkEndChild(s);
                s->SetAttribute("font", 4);
                s->SetAttribute("size", font_size);
                s->SetAttribute("face", font_face.face);
                XMLText* txt = _doc->NewText(sub_text.c_str());
                s->LinkEndChild(txt);
                current_styles = kvp.second;
                first_index = second_index;
            }
        }
    }
    break;
    }
}

void MoleculeCdxmlSaver::addText(const Vec2f& pos, const char* text)
{
    addText(pos, text, "Center");
}

void MoleculeCdxmlSaver::addText(const Vec2f& pos, const char* text, const char* alignment)
{
    QS_DEF(Array<char>, buf);
    buf.readString(text, false);
    if (buf.size() < 1)
        return;
    buf.clear();

    XMLElement* t = _doc->NewElement("t");
    _current->LinkEndChild(t);

    ArrayOutput out(buf);
    out.printf("%f %f", _bond_length * pos.x, -_bond_length * pos.y);
    buf.push(0);
    t->SetAttribute("p", buf.ptr());
    t->SetAttribute("Justification", alignment);
    t->SetAttribute("InterpretChemically", "no");

    XMLElement* s = _doc->NewElement("s");
    t->LinkEndChild(s);
    s->SetAttribute("font", 3);
    s->SetAttribute("size", 10);
    s->SetAttribute("face", 96);
    XMLText* txt = _doc->NewText(text);
    s->LinkEndChild(txt);
}

void MoleculeCdxmlSaver::addTitle(const Vec2f& pos, const char* text)
{
    QS_DEF(Array<char>, buf);
    buf.readString(text, false);
    if (buf.size() < 1)
        return;
    buf.clear();

    XMLElement* t = _doc->NewElement("t");
    _current->LinkEndChild(t);

    ArrayOutput out(buf);
    out.printf("%f %f", _bond_length * pos.x, -_bond_length * pos.y);
    buf.push(0);
    t->SetAttribute("p", buf.ptr());
    t->SetAttribute("Justification", "Center");
    t->SetAttribute("InterpretChemically", "no");

    XMLElement* s = _doc->NewElement("s");
    t->LinkEndChild(s);
    s->SetAttribute("font", 4);
    s->SetAttribute("size", 18);
    s->SetAttribute("face", 1);
    XMLText* txt = _doc->NewText(text);
    s->LinkEndChild(txt);
}

void MoleculeCdxmlSaver::addElement(const char* element, int id, const Vec2f& p1, const Vec2f& p2, PropertiesMap& attrs)
{
    XMLElement* g = _doc->NewElement(element);
    _current->LinkEndChild(g);

    if (id > 0)
        g->SetAttribute("id", id);

    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    out.printf("%f %f %f %f", _bond_length * p1.x, -_bond_length * p1.y, _bond_length * p2.x, -_bond_length * p2.y);
    buf.push(0);

    g->SetAttribute("BoundingBox", buf.ptr());

    for (auto i : attrs.elements())
    {
        g->SetAttribute(attrs.key(i), attrs.value(i));
    }
}

void MoleculeCdxmlSaver::addCustomElement(int id, Array<char>& name, PropertiesMap& attrs)
{
    XMLElement* e = _doc->NewElement(name.ptr());
    _current->LinkEndChild(e);

    if (id > 0)
        e->SetAttribute("id", id);

    for (auto i : attrs.elements())
    {
        e->SetAttribute(attrs.key(i), attrs.value(i));
    }
}

void MoleculeCdxmlSaver::startCurrentElement(int id, Array<char>& name, PropertiesMap& attrs)
{
    XMLElement* e = _doc->NewElement(name.ptr());
    _current->LinkEndChild(e);
    _current = e;

    if (id > 0)
        e->SetAttribute("id", id);

    for (auto i : attrs.elements())
    {
        e->SetAttribute(attrs.key(i), attrs.value(i));
    }
}

void MoleculeCdxmlSaver::endCurrentElement()
{
    XMLNode* node = _current->Parent();
    _current = (XMLElement*)node;
}

void MoleculeCdxmlSaver::addCustomText(const Vec2f& pos, const char* alignment, float line_height, const char* text)
{
    XMLElement* t = _doc->NewElement("t");
    _current->LinkEndChild(t);

    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    out.printf("%f %f", _bond_length * pos.x, -_bond_length * pos.y);
    buf.push(0);
    t->SetAttribute("p", buf.ptr());
    t->SetAttribute("Justification", alignment);

    out.clear();
    out.printf("%f", line_height);
    buf.push(0);
    t->SetAttribute("LineHeight", buf.ptr());

    buf.readString(text, false);
    if (buf.size() > 1)
    {
        buf.remove(buf.size() - 1);
        buf.remove(0);
        buf.push(0);

        XMLUnknown* s = _doc->NewUnknown(buf.ptr());
        t->LinkEndChild(s);
    }
}

void MoleculeCdxmlSaver::endPage()
{
    _current = _root;
}

void MoleculeCdxmlSaver::endDocument()
{
    XMLPrinter printer;
    _doc->Accept(&printer);
    _output.printf("%s", printer.CStr());
    _doc.reset(nullptr);
    //   _doc = 0;
}

int MoleculeCdxmlSaver::getHydrogenCount(BaseMolecule& mol, int idx, int charge, int radical)
{
    int h;
    int val, chg, rad;

    if (!mol.isQueryMolecule())
        h = mol.asMolecule().getImplicitH(idx);
    else if (mol.isQueryMolecule())
    {
        int number = mol.getAtomNumber(idx);

        if (number == -1)
            return -1;

        int conn = mol.asQueryMolecule()._calcAtomConnectivity(idx);

        if (conn == -1)
            return -1;

        if (charge == CHARGE_UNKNOWN)
            chg = 0;
        else
            chg = charge;

        if (radical == -1)
            rad = 0;
        else
            rad = radical;

        int explicit_val = mol.getExplicitValence(idx);

        if (explicit_val != -1)
            h = explicit_val - Element::calcValenceMinusHyd(number, chg, rad, conn);
        else
            Element::calcValence(number, chg, rad, conn, val, h, false);
    }
    return h;
}

void MoleculeCdxmlSaver::saveMolecule(BaseMolecule& mol)
{
    Vec3f min_coord, max_coord;

    _id = 0;

    if (mol.have_xyz)
    {
        for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        {
            Vec3f& pos = mol.getAtomXyz(i);
            if (i == mol.vertexBegin())
                min_coord = max_coord = pos;
            else
            {
                min_coord.min(pos);
                max_coord.max(pos);
            }
        }
        // Add margins
        max_coord.add(Vec3f(1, 1, 1));
        min_coord.sub(Vec3f(1, 1, 1));
    }
    else
    {
        min_coord.set(0, 0, 0);
        max_coord.set(0, 0, 0);
    }

    beginDocument(NULL);
    addDefaultFontTable();
    addDefaultColorTable();
    beginPage(NULL);

    Vec2f offset(-min_coord.x, -max_coord.y);

    saveMoleculeFragment(mol, offset, 1);
    endPage();
    endDocument();
}
