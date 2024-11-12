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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4315) // TODO: Fix CDXFont allign issue
#endif

#include "molecule/molecule_cdxml_saver.h"
#include "base_cpp/locale_guard.h"
#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cdxml_loader.h"
#include "molecule/parse_utils.h"
#include "molecule/query_molecule.h"
#include "utils/image_not_supported.h"

#include <codecvt>
#include <fstream>
#include <streambuf>
#include <tinyxml2.h>

using namespace indigo;
using namespace tinyxml2;

tinyxml2::XMLElement* MoleculeCdxmlSaver::create_text(tinyxml2::XMLElement* parent, float x, float y, const char* label_justification)
{
    XMLElement* t = _doc->NewElement("t");
    parent->LinkEndChild(t);
    Array<char> buf;
    ArrayOutput out(buf);
    out.printf("%f %f", x, y);
    buf.push(0);
    t->SetAttribute("p", buf.ptr());
    if (label_justification)
        t->SetAttribute("LabelJustification", label_justification);
    return t;
}

void MoleculeCdxmlSaver::add_style_str(tinyxml2::XMLElement* parent, int font, int size, int face, const char* str)
{
    XMLElement* s = _doc->NewElement("s");
    parent->LinkEndChild(s);
    s->SetAttribute("font", font);
    s->SetAttribute("size", size);
    s->SetAttribute("face", face);
    XMLText* txt = _doc->NewText(str);
    s->LinkEndChild(txt);
}

void MoleculeCdxmlSaver::add_charge(tinyxml2::XMLElement* parent, int font, int size, int charge)
{
    if (charge == 0 || charge == CHARGE_UNKNOWN)
        return;
    if (charge > 0)
    {
        if (charge > 1)
            add_style_str(parent, font, size, 64, std::to_string(charge).c_str());
        add_style_str(parent, font, size, 96, "+");
    }
    else if (charge < 0)
    {
        if (charge < -1)
            add_style_str(parent, font, size, 64, std::to_string(-charge).c_str());
        add_style_str(parent, font, size, 96, "-");
    }
}

void MoleculeCdxmlSaver::writeBinaryTextValue(const tinyxml2::XMLElement* pTextElement)
{
    if (std::string(pTextElement->Name()) != "t")
        throw Error("not a text element");
    std::vector<CDXTextStyle> styled_strings;
    std::string text;
    CDXTextStyle ts;
    for (auto pStyleElem = pTextElement->FirstChildElement(); pStyleElem; pStyleElem = pStyleElem->NextSiblingElement())
    {
        if (std::string(pStyleElem->Name()) == "s")
        {
            for (auto pAttr = pStyleElem->FirstAttribute(); pAttr; pAttr = pAttr->Next())
            {
                std::string attr_name = pAttr->Name();
                if (attr_name == "font")
                    ts.font_index = static_cast<uint16_t>(pAttr->IntValue());
                else if (attr_name == "size")
                    ts.font_size = static_cast<uint16_t>(pAttr->FloatValue() * kCDXMLSizeMultiplier);
                else if (attr_name == "color")
                    ts.font_color = static_cast<uint16_t>(pAttr->IntValue());
                else if (attr_name == "face")
                    ts.font_face = static_cast<uint16_t>(pAttr->IntValue());
            }
            ts.offset = static_cast<uint16_t>(text.size());
            styled_strings.push_back(ts);
            auto ptext = pStyleElem->GetText();
            if (ptext)
                text += ptext;
        }
    }

    _output.writeBinaryUInt16(kCDXProp_Text);
    if (text.size())
    {
        _output.writeBinaryUInt16(static_cast<uint16_t>(styled_strings.size() * sizeof(CDXTextStyle) + sizeof(uint16_t) + text.size()));
        _output.writeBinaryUInt16(static_cast<uint16_t>(styled_strings.size()));
        for (const auto& ss : styled_strings)
        {
            _output.writeBinaryUInt16(ss.offset);
            _output.writeBinaryUInt16(ss.font_index);
            _output.writeBinaryUInt16(ss.font_face);
            _output.writeBinaryUInt16(ss.font_size);
            _output.writeBinaryUInt16(ss.font_color);
        }
        _output.write(text.c_str(), static_cast<int>(text.size()));
    }
    else
        _output.writeBinaryUInt16(0);
}

void MoleculeCdxmlSaver::writeBinaryValue(const XMLAttribute* pAttr, int16_t tag, ECDXType cdx_type)
{
    _output.writeBinaryUInt16(tag);
    switch (cdx_type)
    {
    case ECDXType::CDXString: {
        std::string val = pAttr->Value();
        uint16_t styles = 0;
        _output.writeBinaryUInt16(static_cast<uint16_t>(val.size() + sizeof(styles)));
        _output.writeBinaryUInt16(styles);
        _output.write(val.data(), static_cast<int>(val.size()));
    }
    break;

    case ECDXType::CDXUINT8:
    case ECDXType::CDXINT8: {
        int8_t val;
        switch (tag)
        {
        case kCDXProp_CaptionJustification:
        case kCDXProp_Justification:
        case kCDXProp_LabelJustification:
            val = kTextJustificationStrToInt.at(pAttr->Value());
            break;
        case kCDXProp_LabelAlignment:
        case kCDXProp_Node_LabelDisplay:
            val = kLabelAlignmentStrToInt.at(pAttr->Value());
            break;
        case kCDXProp_Atom_Radical:
            val = static_cast<int8_t>(kRadicalStrToId.at(pAttr->Value()));
            break;
        case kCDXProp_Bond_CIPStereochemistry:
            val = kCIPBondStereochemistryIndexToChar.at(pAttr->Value()[0]);
            break;
        case kCDXProp_Atom_CIPStereochemistry:
            val = kCIPStereochemistryCharToIndex.at(pAttr->Value()[0]);
            break;
        case kCDXProp_Arrow_Type:
            val = static_cast<int8_t>(kCDXProp_Arrow_TypeStrToID.at(pAttr->Value()));
            break;
        case kCDXProp_Atom_Geometry:
            val = static_cast<int8_t>(KGeometryTypeNameToInt.at(pAttr->Value()));
            break;
        case kCDXProp_Atom_EnhancedStereoType:
            val = static_cast<int8_t>(kCDXEnhancedStereoStrToID.at(pAttr->Value()));
            break;
        default:
            val = static_cast<int8_t>(pAttr->IntValue());
            break;
        }
        _output.writeBinaryUInt16(sizeof(val));
        _output.writeByte(val);
    }
    break;

    case ECDXType::CDXINT16:
    case ECDXType::CDXUINT16: {
        int16_t val = static_cast<int16_t>(pAttr->IntValue());
        switch (tag)
        {
        case kCDXProp_Node_Type:
            val = static_cast<int16_t>(KNodeTypeNameToInt.at(pAttr->Value()));
            break;
        case kCDXProp_Graphic_Type:
            val = static_cast<int16_t>(kCDXPropGraphicTypeStrToID.at(pAttr->Value()));
            break;
        case kCDXProp_Rectangle_Type: {
            auto vecs = split(pAttr->Value(), ' ');
            for (auto str_val : vecs)
                val |= kRectangleTypeStrToInt.at(str_val);
        }
        break;
        case kCDXProp_BondSpacing:
            val *= kBondSpacingMultiplier;
            break;
        case kCDXProp_Line_Type:
            val = static_cast<int16_t>(kLineTypeStrToInt.at(pAttr->Value()));
            break;
        case kCDXProp_Arrow_Type:
            val = static_cast<int16_t>(kCDXProp_Arrow_TypeStrToID.at(pAttr->Value()));
            break;
        case kCDXProp_Arrow_ArrowHead_Head:
        case kCDXProp_Arrow_ArrowHead_Tail:
            val = kCDXProp_Arrow_ArrowHeadStrToInt.at(pAttr->Value());
            break;
        case kCDXProp_Arrowhead_Type: {
            auto it = kCDXProp_Arrow_ArrowHeadTypeStrToInt.find(pAttr->Value());
            if (it != kCDXProp_Arrow_ArrowHeadTypeStrToInt.end())
                val = it->second;
            break;
        }
        case kCDXProp_Symbol_Type: {
            auto it = kCDXPropSymbolTypeStrToID.find(pAttr->Value());
            if (it != kCDXPropSymbolTypeStrToID.end())
                val = it->second;
            break;
        }
        case kCDXProp_Bond_Display:
        case kCDXProp_Bond_Display2:
            val = static_cast<int16_t>(kCDXProp_Bond_DisplayStrToID.at(pAttr->Value()));
            break;
        case kCDXProp_Bond_Order: {
            auto vals = split(pAttr->Value(), ' ');
            val = 0;
            for (auto& value : vals)
            {
                val |= static_cast<int16_t>(kBondOrderStrToId.at(value));
            }
            if (val == 0)
                val = -1;
        }
        break;
        }

        _output.writeBinaryUInt16(sizeof(val));
        _output.writeBinaryUInt16(val);
    }
    break;

    case ECDXType::CDXINT32:
    case ECDXType::CDXUINT32: {
        int32_t val = pAttr->IntValue();
        if (tag == kCDXProp_ChainAngle)
            val <<= 16;
        _output.writeBinaryUInt16(sizeof(val));
        _output.writeBinaryInt(val);
    }
    break;

    case ECDXType::CDXPoint3D:
    case ECDXType::CDXPoint2D:
    case ECDXType::CDXRectangle: {
        std::string values = pAttr->Value();
        auto vec_strs = split(values, ' ');
        if (vec_strs.size() % 2 == 0)
        {
            for (size_t i = 0; i < vec_strs.size(); i += 2)
                std::swap(vec_strs[i], vec_strs[i + 1]);
        }

        _output.writeBinaryUInt16(static_cast<uint16_t>(sizeof(int32_t) * vec_strs.size()));

        for (const auto& v : vec_strs)
        {
            int32_t coord = static_cast<int32_t>(std::stof(v) * (1 << 16));
            _output.writeBinaryInt(coord);
        }
    }
    break;

    case ECDXType::CDXCoordinate: {
        int32_t coord = static_cast<int32_t>(pAttr->FloatValue() * (1 << 16));
        _output.writeBinaryUInt16(sizeof(coord));
        _output.writeBinaryInt(coord);
    }
    break;

    case ECDXType::CDXBooleanImplied:
    case ECDXType::CDXBoolean: {
        uint8_t val = std::string(pAttr->Value()) == "yes" ? 1 : 0;
        _output.writeBinaryUInt16(sizeof(val));
        _output.writeByte(val);
    }
    break;

    case ECDXType::CDXObjectID: {
        uint32_t val = pAttr->IntValue();
        _output.writeBinaryUInt16(sizeof(val));
        _output.writeBinaryInt(val);
    }
    break;
    case ECDXType::CDXUnformatted: {
        std::string values = pAttr->Value();
        std::vector<uint8_t> bytes_vector;
        for (size_t i = 0; i < values.size(); i += 2)
        {
            uint32_t val;
            std::string hex_str = values.substr(i, 2);
            std::stringstream converter;
            converter << std::hex << hex_str;
            converter >> val;
            bytes_vector.push_back(static_cast<uint8_t>(val));
        }
        _output.writeBinaryUInt16(static_cast<uint16_t>(bytes_vector.size()));
        _output.write(bytes_vector.data(), static_cast<int>(bytes_vector.size()));
    }
    break;

    case ECDXType::CDXObjectIDArray: {
        std::string values = pAttr->Value();
        auto vals = split(values, ' ');
        _output.writeBinaryUInt16(static_cast<uint16_t>(vals.size() * sizeof(int32_t)));
        for (const auto& val : vals)
            _output.writeBinaryInt(std::stoi(val));
    }
    break;

    case ECDXType::CDXDate:
    case ECDXType::CDXRepresentsProperty:
    case ECDXType::CDXFontTable:
    case ECDXType::CDXColorTable:
    case ECDXType::CDXElementList:
    case ECDXType::CDXFormula:
    case ECDXType::CDXObjectIDArrayWithCounts:
    case ECDXType::CDXGenericList:
    case ECDXType::CDXFLOAT64:
    case ECDXType::CDXINT16ListWithCounts:
    case ECDXType::CDXCurvePoints:
    case ECDXType::CDXCurvePoints3D:
    case ECDXType::CDXvaries:
    case ECDXType::CDXFontStyle:
        throw Error("Unsupported type: %d", cdx_type);
        break;
    default:
        break;
    }
}

IMPL_ERROR(MoleculeCdxmlSaver, "molecule CDXML saver");

int MoleculeCdxmlSaver::getId()
{
    return _id;
}

MoleculeCdxmlSaver::MoleculeCdxmlSaver(Output& output, bool is_binary) : _output(output), _is_binary(is_binary)
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
            _pages_height = (int)round((float)height / max_height);
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

        mac_print_info[13] = height / 5; // magic scaling coefficient
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

void MoleculeCdxmlSaver::beginPage(Bounds* /* bounds */)
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

int MoleculeCdxmlSaver::_getAttachmentPoint(BaseMolecule& mol, int atom_idx)
{
    int val = 0;
    if (mol.attachmentPointCount() != 0)
    {
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
    }
    return val;
}

void MoleculeCdxmlSaver::addNodeToFragment(BaseMolecule& mol, XMLElement* fragment, int atom_idx, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord,
                                           Vec2f& node_pos)
{
    Vec3f pos3 = mol.getAtomXyz(atom_idx);
    Vec2f pos(pos3.x, pos3.y);

    bool have_z = BaseMolecule::hasZCoord(mol);
    if (have_z)
    {
        pos3.x += offset.x;
        pos3.y += offset.y;
        pos3.scale(_scale);
    }

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

        if (_getAttachmentPoint(mol, atom_idx))
        {
            node->SetAttribute("NodeType", "ExternalConnectionPoint");
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
        if (have_z)
        {
            QS_DEF(Array<char>, buf);
            ArrayOutput out(buf);
            out.printf("%f %f %f", pos3.x, -pos3.y, -pos3.z);
            buf.push(0);
            node->SetAttribute("xyz", buf.ptr());
        }
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
            node->SetAttribute("EnhancedStereoGroupNum", enh_stereo_grp);
            break;
        case MoleculeStereocenters::ATOM_AND:
            node->SetAttribute("EnhancedStereoType", "And");
            node->SetAttribute("EnhancedStereoGroupNum", enh_stereo_grp);
            break;
        default:
            throw Error("Unknows enhanced stereo type %d", enh_stereo_type);
            break;
        }
    }

    if (mol.getVertex(atom_idx).degree() == 0 && atom_number == ELEM_C && charge == 0 && radical == 0)
    {
        XMLElement* t = create_text(node, pos.x, -pos.y, nullptr);
        t->SetAttribute("Justification", "Center");
        add_style_str(t, 3, 10, 96, "CH4");
        add_charge(t, 3, 10, charge);
    }
    else if (mol.isRSite(atom_idx))
    {
        XMLElement* t = create_text(node, pos.x, -pos.y, "Left");
        QS_DEF(Array<char>, buf);
        mol.getAtomSymbol(atom_idx, buf);
        buf.push(0);
        add_style_str(t, 3, 10, 96, buf.ptr());
        add_charge(t, 3, 10, charge);
    }
    else if (mol.isPseudoAtom(atom_idx))
    {
        XMLElement* t = create_text(node, pos.x, -pos.y, "Left");
        add_style_str(t, 3, 10, 96, mol.getPseudoAtom(atom_idx));
        add_charge(t, 3, 10, charge);
    }
    else if (atom_number > 0 && atom_number != ELEM_C)
    {
        XMLElement* t = create_text(node, pos.x, -pos.y, "Left");

        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);
        mol.getAtomSymbol(atom_idx, buf);
        if (hcount > 0)
        {
            buf.pop();
            buf.push('H');
        }
        buf.push(0);
        add_style_str(t, 3, 10, 96, buf.ptr());

        if (hcount > 1)
        {
            out.clear();
            out.printf("%d", hcount);
            buf.push(0);
            add_style_str(t, 3, 10, 32, buf.ptr());
        }
        add_charge(t, 3, 10, charge);
    }
    else if (atom_number < 0 && mol.isQueryMolecule())
    {
        XMLElement* t = create_text(node, pos.x, -pos.y, "Left");

        QS_DEF(Array<char>, buf);
        ArrayOutput out(buf);

        QS_DEF(Array<int>, list);
        int query_atom_type;

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

        add_style_str(t, 3, 10, 96, buf.ptr());
    }
}

void MoleculeCdxmlSaver::_collectSuperatoms(BaseMolecule& mol)
{
    for (int i = mol.sgroups.begin(); i != mol.sgroups.end(); i = mol.sgroups.next(i))
    {
        SGroup& sgroup = mol.sgroups.getSGroup(i);
        if (sgroup.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            _superatoms.emplace(i, ++_id);
            auto& atoms_list = _superatoms.at(i).atoms;

            Superatom& sa = (Superatom&)sgroup;
            // collect atoms of all the superatoms
            for (int j = 0; j < sa.atoms.size(); ++j)
            {
                _superatoms_atoms.emplace(sa.atoms[j], i);
                atoms_list.push_back(sa.atoms[j]);
            }
        }
    }

    if (_superatoms_atoms.size())
        for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
        {
            auto& edge = mol.getEdge(i);
            auto beg_it = _superatoms_atoms.find(edge.beg);
            auto end_it = _superatoms_atoms.find(edge.end);

            if (beg_it != _superatoms_atoms.end() && end_it != _superatoms_atoms.end() && beg_it->second == end_it->second)
                _superatoms.at(beg_it->second).bonds.push_back(i);

            if (beg_it != _superatoms_atoms.end())
                _superatoms_bonds.emplace(i, beg_it->second);
            else if (end_it != _superatoms_atoms.end())
                _superatoms_bonds.emplace(i, end_it->second);
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
    if (order < 0 && mol.isQueryMolecule())
        order = QueryMolecule::getQueryBondType(mol.asQueryMolecule().getBond(bond_idx));

    int dir = mol.getBondDirection(bond_idx);

    if (mol.cis_trans.isIgnored(bond_idx))
    {
        order = BOND_DOUBLE;
        dir = BOND_EITHER;
    }

    if (order == BOND_DOUBLE || order == BOND_TRIPLE)
        bond->SetAttribute("Order", order);
    else if (order == BOND_AROMATIC)
    {
        bond->SetAttribute("Order", kBondOrderIntToStr.at(kCDXBondOrder_OneHalf).c_str());
        bond->SetAttribute("Display", kCDXProp_Bond_DisplayIdToStr.at(kCDXBondDisplay_Dash).c_str());
        bond->SetAttribute("Display2", kCDXProp_Bond_DisplayIdToStr.at(kCDXBondDisplay_Dash).c_str());
    }
    else if (order == _BOND_SINGLE_OR_DOUBLE)
    {
        bond->SetAttribute("Order", (kBondOrderIntToStr.at(kCDXBondOrder_Single) + " " + kBondOrderIntToStr.at(kCDXBondOrder_Double)).c_str());
    }
    else if (order == _BOND_SINGLE_OR_AROMATIC)
    {
        bond->SetAttribute("Order", (kBondOrderIntToStr.at(kCDXBondOrder_Single) + " " + kBondOrderIntToStr.at(kCDXBondOrder_OneHalf)).c_str());
    }
    else if (order == _BOND_DOUBLE_OR_AROMATIC)
    {
        bond->SetAttribute("Order", (kBondOrderIntToStr.at(kCDXBondOrder_Double) + " " + kBondOrderIntToStr.at(kCDXBondOrder_OneHalf)).c_str());
    }
    else if (order == _BOND_ANY)
    {
        bond->SetAttribute("Order", kBondOrderIntToStr.at(kCDXBondOrder_Any).c_str());
    }
    else if (order == _BOND_COORDINATION)
    {
        bond->SetAttribute("Order", kBondOrderIntToStr.at(kCDXBondOrder_Dative).c_str());
    }
    else if (order == _BOND_HYDROGEN)
    {
        bond->SetAttribute("Order", kBondOrderIntToStr.at(kCDXBondOrder_Hydrogen).c_str());
    }
    else
        ; // Do not write single bond order

    int parity = mol.cis_trans.getParity(bond_idx);

    if (mol.have_xyz && (dir == BOND_UP || dir == BOND_DOWN))
    {
        bond->SetAttribute("Display", kCDXProp_Bond_DisplayIdToStr.at((dir == BOND_UP) ? kCDXBondDisplay_WedgeBegin : kCDXBondDisplay_WedgedHashBegin).c_str());
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
    else if (dir == BOND_EITHER)
    {
        bond->SetAttribute("Display", kCDXProp_Bond_DisplayIdToStr.at(kCDXBondDisplay_Wavy).c_str());
    }

    if (mol.reaction_bond_reacting_center[bond_idx] != RC_UNMARKED)
    {
        auto it = reaction_center_to_bond_rxn_participation.find(mol.reaction_bond_reacting_center[bond_idx]);
        if (it != reaction_center_to_bond_rxn_participation.end())
            bond->SetAttribute("RxnParticipation", it->second);
    }

    if (mol.isQueryMolecule())
    {
        int topology = VALUE_UNKNOWN;
        mol.asQueryMolecule().getBond(bond_idx).sureValue(QueryMolecule::BOND_TOPOLOGY, topology);
        if (topology > 0)
            bond->SetAttribute("Topology", topology_to_cdx_topology.at(topology));
    }
}

void MoleculeCdxmlSaver::addBondsToFragment(BaseMolecule& mol, tinyxml2::XMLElement* fragment)
{
    for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        // skip bonds from superatoms
        if (_superatoms_bonds.find(i) == _superatoms_bonds.end())
            addBondToFragment(mol, fragment, i);
    }
}

void MoleculeCdxmlSaver::addNodesToFragment(BaseMolecule& mol, XMLElement* fragment, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord)
{
    Vec2f dummy_pos;
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        // skip atoms from superatoms
        if (_superatoms_atoms.find(i) == _superatoms_atoms.end())
            addNodeToFragment(mol, fragment, i, offset, min_coord, max_coord, dummy_pos);
    }
}

void MoleculeCdxmlSaver::addFragmentNodes(BaseMolecule& mol, tinyxml2::XMLElement* fragment, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord)
{
    // iterate over superatoms
    std::unordered_map<std::pair<int, int>, int, pair_int_hash> outer_bond_ids;

    for (auto& kvp : _superatoms)
    {
        std::vector<std::pair<int, int>> ext_connections;
        std::vector<int> connection_order, bond_ordering;
        XMLElement* node = _doc->NewElement("n");
        fragment->LinkEndChild(node);
        int fragment_node_id = kvp.second.id;
        node->SetAttribute("id", fragment_node_id);
        node->SetAttribute("NodeType", "Fragment");
        XMLElement* super_fragment = _doc->NewElement("fragment");
        super_fragment->SetAttribute("id", ++_id);
        node->LinkEndChild(super_fragment);
        // iterate atoms in the superatom
        for (auto atom_idx : kvp.second.atoms)
        {
            Vec2f pos;
            addNodeToFragment(mol, super_fragment, atom_idx, offset, min_coord, max_coord, pos);
            auto& vx = mol.getVertex(atom_idx);
            // iterate neighbors
            for (auto nei_idx = vx.neiBegin(); nei_idx != vx.neiEnd(); nei_idx = vx.neiNext(nei_idx))
            {
                int nei_atom_idx = vx.neiVertex(nei_idx);
                int nei_edge_idx = vx.neiEdge(nei_idx);
                auto ex_atom_it = _superatoms_atoms.find(nei_atom_idx);
                // if atom is not in the superatom
                if (ex_atom_it == _superatoms_atoms.end() || ex_atom_it->second != kvp.first)
                {
                    // external neighbor found
                    XMLElement* connection_node = _doc->NewElement("n");
                    super_fragment->LinkEndChild(connection_node);
                    connection_node->SetAttribute("id", ++_id);
                    connection_node->SetAttribute("NodeType", "ExternalConnectionPoint");
                    ext_connections.emplace_back(_id, _atoms_ids[atom_idx]);
                    connection_order.push_back(_id);
                    // if this connection already exists, _id should be reused for bond_ordering
                    auto out_bond = std::make_pair(std::min(nei_atom_idx, atom_idx), std::max(nei_atom_idx, atom_idx));
                    auto outer_bond_it = outer_bond_ids.find(out_bond);
                    if (outer_bond_it == outer_bond_ids.end())
                    {
                        bond_ordering.push_back(++_id);
                        if (ex_atom_it != _superatoms_atoms.end() && ex_atom_it->second != kvp.first)
                            _out_connections.emplace_back(_id, _superatoms.at(ex_atom_it->second).id, fragment_node_id);
                        else
                            _out_connections.emplace_back(_id, _atoms_ids[nei_atom_idx], fragment_node_id);
                        outer_bond_ids.emplace(out_bond, _id);
                    }
                    else
                        bond_ordering.push_back(outer_bond_it->second);
                }
            }
        }

        for (int edge_idx : kvp.second.bonds)
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
            for (size_t i = 0; i < connection_order.size(); ++i)
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
            for (size_t i = 0; i < bond_ordering.size(); ++i)
            {
                if (i)
                    order += " ";
                order += std::to_string(bond_ordering[i]);
            }
            node->SetAttribute("BondOrdering", order.c_str());
        }

        auto& sa = (Superatom&)mol.sgroups.getSGroup(kvp.first);
        if (sa.subscript.size())
        {
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
}

void MoleculeCdxmlSaver::saveMoleculeFragment(BaseMolecule& bmol, const Vec2f& offset, float scale)
{
    std::map<int, int> atom_ids;
    int id = 0;
    saveMoleculeFragment(bmol, offset, scale, -1, id, atom_ids);
}

void MoleculeCdxmlSaver::saveRGroup(PtrPool<BaseMolecule>& fragments, const Vec2f& offset, int rgnum)
{
    // XMLElement* parent = _current;
    XMLElement* fragment = _doc->NewElement("altgroup");
    _current->LinkEndChild(fragment);
    _current = fragment;
    fragment->SetAttribute("id", ++_id);
    Vec2f rmin, rmax;
    int valence = 0;
    for (int i = fragments.begin(); i != fragments.end(); i = fragments.next(i))
    {
        Vec2f min_coord, max_coord;
        fragments[i]->getBoundingBox(min_coord, max_coord);
        if (i == fragments.begin())
        {
            rmin.copy(min_coord);
            rmax.copy(max_coord);
        }
        else
        {
            rmin.min(min_coord);
            rmax.max(max_coord);
        }
        saveMoleculeFragment(*fragments[i], offset, 1);
        valence += fragments[i]->attachmentPointCount();
    }
    std::string rg_name("R");
    rg_name += std::to_string(rgnum);
    rmin.add(offset);
    rmax.add(offset);
    Vec2f text_origin(rmin.x, rmax.y);
    addText(text_origin, rg_name.c_str(), nullptr);
    rmin.x *= _scale;
    rmax.x *= _scale;
    rmax.y *= -_scale;
    rmin.y *= -_scale;
    auto gframe = std::to_string(rmin.x) + " " + std::to_string(rmin.y) + " " + std::to_string(rmax.x) + " " + std::to_string(rmax.y);
    fragment->SetAttribute("Valence", valence);
}

void MoleculeCdxmlSaver::saveMoleculeFragment(BaseMolecule& bmol, const Vec2f& offset, float structure_scale, int frag_id, int& id,
                                              std::map<int, int>& atom_ids)
{
    std::unique_ptr<BaseMolecule> mol(bmol.neu());
    mol->clone(bmol);
    deleteNamelessSGroups(*mol);
    mol->transformTemplatesToSuperatoms();
    _atoms_ids.clear();
    _bonds_ids.clear();
    _superatoms.clear();
    _superatoms_atoms.clear();
    _superatoms_bonds.clear();
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

    if (atom_ids.size())
    {
        _atoms_ids = atom_ids;
        auto back_it = std::prev(_atoms_ids.end());
        if (back_it->second > _id)
            _id = back_it->second;
    }
    else
        for (int i = mol->vertexBegin(); i != mol->vertexEnd(); i = mol->vertexNext(i))
            _atoms_ids.emplace(i, ++_id);

    for (int i = mol->edgeBegin(); i != mol->edgeEnd(); i = mol->edgeNext(i))
    {
        _bonds_ids.emplace(i, ++_id);
    }

    Vec2f min_coord, max_coord;

    _collectSuperatoms(*mol);
    addFragmentNodes(*mol, fragment, offset, min_coord, max_coord);
    addNodesToFragment(*mol, fragment, offset, min_coord, max_coord);
    addBondsToFragment(*mol, fragment);

    for (const auto& out_bond : _out_connections)
    {
        XMLElement* bond = _doc->NewElement("b");
        fragment->LinkEndChild(bond);
        bond->SetAttribute("id", out_bond.id);
        bond->SetAttribute("B", out_bond.beg);
        bond->SetAttribute("E", out_bond.end);
    }

    if (mol->isChiral())
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

    _current = parent;
    id = _id;
}

void MoleculeCdxmlSaver::addRetrosynteticArrow(int graphic_obj_id, int arrow_id, const Vec2f& arrow_beg, const Vec2f& arrow_end)
{
    PropertiesMap attrs;
    attrs.insert("FillType", "None");
    attrs.insert("ArrowheadHead", "Full");
    attrs.insert("ArrowheadType", "Angle");
    attrs.insert("HeadSize", "600");
    attrs.insert("ArrowheadCenterSize", "600");
    attrs.insert("ArrowheadWidth", "150");
    attrs.insert("ArrowShaftSpacing", "600");

    Vec3f ar_beg(arrow_beg.x, -arrow_beg.y, 0);
    Vec3f ar_end(arrow_end.x, -arrow_end.y, 0);
    ar_beg.scale(_bond_length);
    ar_end.scale(_bond_length);

    attrs.insert("Head3D", std::to_string(ar_end.x) + " " + std::to_string(ar_end.y) + " " + std::to_string(ar_end.z));
    attrs.insert("Tail3D", std::to_string(ar_beg.x) + " " + std::to_string(ar_beg.y) + " " + std::to_string(ar_beg.z));

    addElement("arrow", arrow_id, arrow_end, arrow_beg, attrs);

    attrs.clear();

    attrs.insert("SupersededBy", std::to_string(arrow_id));
    attrs.insert("GraphicType", "Line");
    attrs.insert("ArrowType", "RetroSynthetic");
    attrs.insert("HeadSize", "600");

    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    out.printf("%f %f %f %f", ar_end.x, ar_end.y, ar_beg.x, ar_beg.y);
    buf.push(0);

    attrs.insert("BoundingBox", buf.ptr());

    QS_DEF(Array<char>, name);
    name.clear();
    name.readString("graphic", true);

    addCustomElement(graphic_obj_id, name, attrs);
}

std::string stringToHex(const std::string& input)
{
    std::stringstream hexStream;
    hexStream << std::hex << std::setfill('0');

    for (uint8_t val : input)
        hexStream << std::setw(2) << static_cast<int>(val);

    return hexStream.str();
}

void MoleculeCdxmlSaver::addImage(int id, const EmbeddedImageObject& image)
{
    PropertiesMap attrs;
    Array<char> emb_object;
    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    const auto& bbox = image.getBoundingBox();
    out.printf("%f %f %f %f", _bond_length * bbox.left(), -_bond_length * bbox.bottom(), _bond_length * bbox.right(), -_bond_length * bbox.top());
    buf.push(0);
    attrs.insert("BoundingBox", buf.ptr());
    if (image.getFormat() == EmbeddedImageObject::EKETPNG)
        attrs.insert("PNG", stringToHex(image.getData()));
    else if (image.getFormat() == EmbeddedImageObject::EKETSVG)
        attrs.insert("PNG", IMAGE_NOT_SUPPORTED_PNG);
    else
        throw Error("MoleculeCdxmlSaver::addImage: Unknown image format");
    emb_object.readString("embeddedobject", true);
    addCustomElement(id, emb_object, attrs);
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
    case ReactionArrowObject::EOpenAngle:
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadCenterSize", "25");
        break;
    case ReactionArrowObject::EFilledTriangle:
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadCenterSize", "2250");
        break;

    case ReactionArrowObject::EFilledBow:
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadCenterSize", "1125");
        break;

    case ReactionArrowObject::EDashedOpenAngle:
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadCenterSize", "25");
        attrs.insert("LineType", "Dashed");
        break;

    case ReactionArrowObject::EFailed:
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadCenterSize", "1125");
        attrs.insert("NoGo", "Cross");
        break;

    case ReactionArrowObject::EBothEndsFilledTriangle:
        attrs.insert("ArrowheadCenterSize", "2250");
        attrs.insert("ArrowheadHead", "Full");
        attrs.insert("ArrowheadTail", "Full");
        break;

    case ReactionArrowObject::EEquilibriumFilledHalfBow:
        attrs.insert("ArrowheadHead", "HalfLeft");
        attrs.insert("ArrowheadTail", "HalfLeft");
        attrs.insert("ArrowheadCenterSize", "1125");
        attrs.insert("ArrowShaftSpacing", "300");
        break;

    case ReactionArrowObject::EEquilibriumFilledTriangle:
        attrs.insert("ArrowheadHead", "HalfLeft");
        attrs.insert("ArrowheadTail", "HalfLeft");
        attrs.insert("ArrowheadCenterSize", "2250");
        attrs.insert("ArrowShaftSpacing", "300");
        break;

    case ReactionArrowObject::EEquilibriumOpenAngle:
        attrs.insert("ArrowheadHead", "HalfLeft");
        attrs.insert("ArrowheadTail", "HalfLeft");
        attrs.insert("ArrowheadCenterSize", "25");
        attrs.insert("ArrowShaftSpacing", "300");
        break;

    case ReactionArrowObject::EUnbalancedEquilibriumFilledHalfBow:
        break;

    case ReactionArrowObject::EUnbalancedEquilibriumLargeFilledHalfBow:
        break;

    case ReactionArrowObject::EUnbalancedEquilibriumOpenHalfAngle:
        break;

    case ReactionArrowObject::EUnbalancedEquilibriumFilledHalfTriangle:
        break;

    case ReactionArrowObject::EEllipticalArcFilledBow:
        break;

    case ReactionArrowObject::EEllipticalArcFilledTriangle:
        break;

    case ReactionArrowObject::EEllipticalArcOpenAngle:
        break;

    case ReactionArrowObject::EEllipticalArcOpenHalfAngle:
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

void MoleculeCdxmlSaver::addMetaObject(const MetaObject& obj, int id, const Vec2f& offset)
{
    PropertiesMap attrs;
    attrs.clear();
    auto& cp_obj = *(obj.clone());
    cp_obj.offset(offset);

    switch (cp_obj._class_id)
    {
    case ReactionArrowObject::CID: {
        ReactionArrowObject& ar = (ReactionArrowObject&)(cp_obj);
        addArrow(id, ar.getArrowType(), ar.getTail(), ar.getHead());
    }
    break;
    case ReactionPlusObject::CID: {
        ReactionPlusObject& rp = (ReactionPlusObject&)(cp_obj);
        attrs.insert("GraphicType", "Symbol");
        attrs.insert("SymbolType", "Plus");
        Vec2f v1(rp.getPos().x, rp.getPos().y - PLUS_HALF_HEIGHT / _bond_length);
        Vec2f v2(rp.getPos().x, rp.getPos().y + PLUS_HALF_HEIGHT / _bond_length);
        addElement("graphic", id, v1, v2, attrs);
    }
    break;
    case SimpleGraphicsObject::CID: {
        SimpleGraphicsObject& simple_obj = (SimpleGraphicsObject&)cp_obj;
        Rect2f bbox(simple_obj._coordinates.first, simple_obj._coordinates.second);
        switch (simple_obj._mode)
        {
        case SimpleGraphicsObject::EEllipse: {
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
        case SimpleGraphicsObject::ERectangle:
            attrs.insert("GraphicType", "Rectangle");
            break;
        case SimpleGraphicsObject::ELine:
            attrs.insert("GraphicType", "Line");
            break;
        }
        addElement("graphic", id, bbox.leftBottom(), bbox.rightTop(), attrs);
    }
    break;
    case SimpleTextObject::CID: {
        const SimpleTextObject& ko = static_cast<const SimpleTextObject&>(cp_obj);
        // double text_offset_y = 0;
        int font_size = static_cast<int>(KDefaultFontSize);
        CDXMLFontStyle font_face(0);
        for (auto& text_item : ko._block)
        {
            bool is_first_index = true;
            size_t first_index = 0;
            size_t second_index = 0;
            // double text_offset_x = 0;
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
                if (is_first_index)
                {
                    first_index = kvp.first;
                    current_styles = kvp.second;
                    is_first_index = false;
                    continue;
                }
                second_index = kvp.first;

                // std::wstring_convert<std::codecvt_utf8<wchar_t>> utf82w;
                // std::wstring_convert<std::codecvt_utf8<wchar_t>> w2utf8;

                // auto sub_text = w2utf8.to_bytes(utf82w.from_bytes(text_item.text).substr(first_index, second_index - first_index));
                auto sub_text = text_item.text.substr(first_index, second_index - first_index);
                for (const auto& text_style : current_styles)
                {
                    switch (text_style.first)
                    {
                    case SimpleTextObject::EPlain:
                        break;
                    case SimpleTextObject::EBold:
                        font_face.is_bold = text_style.second;
                        break;
                    case SimpleTextObject::EItalic:
                        font_face.is_italic = text_style.second;
                        break;
                    case SimpleTextObject::ESuperScript:
                        font_face.is_superscript = text_style.second;
                        break;
                    case SimpleTextObject::ESubScript:
                        font_face.is_subscript = text_style.second;
                        break;
                    default:
                        font_size = text_style.second ? text_style.first : static_cast<int>(KDefaultFontSize);
                        break;
                    }
                }

                XMLElement* s = _doc->NewElement("s");
                t->LinkEndChild(s);
                s->SetAttribute("font", 4);
                s->SetAttribute("size", font_size / kCDXMLFonsSizeMultiplier);
                s->SetAttribute("face", font_face.face);
                if (font_face.is_superscript)
                {
                    s->SetAttribute("face", KCDXMLFontStyleSuperscript);
                }
                if (font_face.is_subscript)
                    s->SetAttribute("face", KCDXMLFontStyleSubscript);
                XMLText* txt = _doc->NewText(sub_text.c_str());
                s->LinkEndChild(txt);
                current_styles = kvp.second;
                first_index = second_index;
            }
        }
    }
    break;
    case EmbeddedImageObject::CID: {
        const EmbeddedImageObject& image = static_cast<const EmbeddedImageObject&>(cp_obj);
        addImage(id, image);
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
    if (alignment)
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

void MoleculeCdxmlSaver::writeBinaryAttributes(tinyxml2::XMLElement* pElement)
{
    for (auto pAttr = pElement->FirstAttribute(); pAttr; pAttr = pAttr->Next())
    {
        if (pAttr->Name() == std::string("id"))
            continue;
        auto prop_it = KCDXNameToProp.find(pAttr->Name());
        if (prop_it != KCDXNameToProp.end())
        {
            writeBinaryValue(pAttr, prop_it->second.first, prop_it->second.second);
        }
        else
        {
            if (std::string("NeedsClean") != pAttr->Name())
            {
                throw Error("Undefined property: %s\n", pAttr->Name());
            }
        }
    }
}

void MoleculeCdxmlSaver::writeIrregularElement(tinyxml2::XMLElement* pElement, int16_t tag)
{
    switch (tag)
    {
    case kCDXProp_FontTable: {
        std::vector<CDXFont> font_table;

        uint16_t total_size = sizeof(uint16_t) * 2; // platform type + fonts counter
        for (auto pElem = pElement->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement())
        {
            if (std::string(pElem->Name()) == "font")
            {
                CDXFont font;
                for (auto pAttr = pElem->FirstAttribute(); pAttr; pAttr = pAttr->Next())
                {
                    std::string attr_name = pAttr->Name();
                    if (attr_name == "id")
                        font.font_id = static_cast<uint16_t>(pAttr->IntValue());
                    else if (attr_name == "charset")
                    {
                        font.char_set = static_cast<uint16_t>(kCharsetStrToInt.at(pAttr->Value()));
                    }
                    else if (attr_name == "name")
                        font.name = pAttr->Value();
                }
                font_table.push_back(font);
                total_size += static_cast<uint16_t>(sizeof(uint16_t) * 3 + font.name.size()); // font id + charset + name length + name
            }
        }

        _output.writeBinaryUInt16(total_size);
        _output.writeBinaryUInt16(0); // platform type
        _output.writeBinaryUInt16(static_cast<uint16_t>(font_table.size()));

        for (const auto& ft : font_table)
        {
            _output.writeBinaryUInt16(ft.font_id);
            _output.writeBinaryUInt16(ft.char_set);
            _output.writeBinaryUInt16(static_cast<uint16_t>(ft.name.size()));
            _output.write(ft.name.c_str(), static_cast<int>(ft.name.size()));
        }
    }
    break;
    case kCDXProp_ColorTable: {
        std::vector<CDXColor> color_table;
        for (auto pElem = pElement->FirstChildElement(); pElem; pElem = pElem->NextSiblingElement())
        {
            float r = 0, g = 0, b = 0;
            for (auto pAttr = pElem->FirstAttribute(); pAttr; pAttr = pAttr->Next())
            {
                const char* pcol = pAttr->Name();
                switch (*pcol)
                {
                case 'r':
                    r = pAttr->FloatValue();
                    break;
                case 'g':
                    g = pAttr->FloatValue();
                    break;
                case 'b':
                    b = pAttr->FloatValue();
                    break;
                }
            }
            color_table.emplace_back(r, g, b);
        }
        _output.writeBinaryUInt16(static_cast<uint16_t>(color_table.size() * sizeof(CDXColor) + sizeof(uint16_t)));
        _output.writeBinaryUInt16(static_cast<uint16_t>(color_table.size()));
        for (const auto& rgb : color_table)
        {
            _output.write(&rgb, sizeof(rgb));
        }
    }
    break;
    case kCDXProp_RepresentsProperty: {
        AutoInt rp_obj = 0;
        uint16_t rp_tag = 0;
        for (auto pAttr = pElement->FirstAttribute(); pAttr; pAttr = pAttr->Next())
        {
            std::string rp_attr = pAttr->Name();
            if (rp_attr == "attribute")
            {
                auto prop_it = KCDXNameToProp.find(pAttr->Value());
                if (prop_it != KCDXNameToProp.end())
                    rp_tag = prop_it->second.first;
            }
            else if (rp_attr == "object")
                rp_obj = std::string(pAttr->Value());
        }
        if (rp_tag && rp_obj)
        {
            _output.writeBinaryUInt16(sizeof(rp_obj) + sizeof(rp_tag));
            _output.writeBinaryInt(rp_obj);
            _output.writeBinaryUInt16(rp_tag);
        }
        else
            _output.writeBinaryUInt16(0);
    }
    break;
    default:
        throw Error("Unexpected irregular property: %x", tag);
        break;
    }
}

void MoleculeCdxmlSaver::writeBinaryElement(tinyxml2::XMLElement* element)
{
    std::string objname = element->Value();
    int id = 0, tag = 0;
    // bool is_object = false;
    if (objname != "CDXML")
    {
        auto it = KCDXNameToObjID.find(objname);
        if (it != KCDXNameToObjID.end())
        {
            tag = it->second;
            _output.writeBinaryUInt16(static_cast<uint16_t>(tag));
            if (tag < kCDXTag_Object)
            {
                writeIrregularElement(element, static_cast<uint16_t>(tag));
                return;
            }
        }
        else
            throw Error("Unknown object: %s", objname.c_str());
        auto id_attribute = element->FindAttribute("id");
        if (id_attribute)
            id = id_attribute->IntValue();
        _output.writeBinaryInt(id);
    }
    else
        tag = -1;

    if (!tag && KCDXNameToProp.find(objname) == KCDXNameToProp.end())
        throw Error("undefined object: %s", objname.c_str());

    writeBinaryAttributes(element); // save attributes
    if (tag == kCDXObj_Text)        // at this point all text attributes are saved. Need to handle only - 's' entries
    {
        writeBinaryTextValue(element);
    }
    else
        for (auto elem = element->FirstChildElement(); elem; elem = elem->NextSiblingElement())
            writeBinaryElement(elem);

    _output.writeBinaryUInt16(0);
}

void MoleculeCdxmlSaver::endDocument()
{
    if (_is_binary)
    {
        _output.writeString(kCDX_HeaderString);
        _output.writeBinaryInt(kCDXMagicNumber);
        _output.write(kCDXReserved, sizeof(kCDXReserved));
        _output.writeBinaryWord(0); // TODO: change to kCDXObj_Document); // First object is document
        _output.writeBinaryInt(0);  // Document ID 4 bytes.
        auto cdxml = _doc->FirstChildElement();
        writeBinaryElement(cdxml);
        _output.writeBinaryUInt16(0);
    }
    else
    {
        XMLPrinter printer;
        _doc->Accept(&printer);
        _output.printf("%s", printer.CStr());
    }
    _doc.reset(nullptr);
}

int MoleculeCdxmlSaver::getHydrogenCount(BaseMolecule& mol, int idx, int charge, int radical)
{
    int h = 0;
    int val = 0, chg = 0, rad = 0;

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

void MoleculeCdxmlSaver::_validate(BaseMolecule& bmol)
{
    std::string unresolved;
    if (bmol.getUnresolvedTemplatesList(bmol, unresolved))
        throw Error("%s cannot be written in CDXML/CDX format.", unresolved.c_str());
}

void MoleculeCdxmlSaver::saveMolecule(BaseMolecule& bmol)
{
    _validate(bmol);
    Vec2f min_coord, max_coord;

    _id = 0;

    if (bmol.have_xyz)
    {
        bmol.getBoundingBox(min_coord, max_coord);
    }

    for (int i = 0; i < bmol.meta().metaData().size(); ++i)
    {
        Rect2f bb;
        auto& mo = *bmol.meta().metaData()[i];
        mo.getBoundingBox(bb);
        min_coord.min(bb.leftBottom());
        max_coord.max(bb.rightTop());
    }

    beginDocument(NULL);
    addDefaultFontTable();
    addDefaultColorTable();
    beginPage(NULL);

    Vec2f offset(-min_coord.x, -max_coord.y);
    saveMoleculeFragment(bmol, offset, 1);
    for (int i = 1; i <= bmol.rgroups.getRGroupCount(); i++)
    {
        auto& rgrp = bmol.rgroups.getRGroup(i);
        if (rgrp.fragments.size())
            saveRGroup(rgrp.fragments, offset, i);
    }

    for (int i = 0; i < bmol.meta().metaData().size(); ++i)
    {
        auto& mo = *bmol.meta().metaData()[i];
        addMetaObject(*bmol.meta().metaData()[i], ++_id, offset);
    }

    QS_DEF(Array<char>, buf);
    ArrayOutput out(buf);
    out.printf("%f %f %f %f", _scale * min_coord.x, -_scale * min_coord.y, _scale * max_coord.x, -_scale * max_coord.y);
    buf.push(0);
    _page->SetAttribute("BoundingBox", buf.ptr());
    endPage();
    endDocument();
}

void MoleculeCdxmlSaver::deleteNamelessSGroups(BaseMolecule& bmol)
{
    for (int j = bmol.sgroups.begin(); j != bmol.sgroups.end(); j = bmol.sgroups.next(j))
    {
        SGroup& sg = bmol.sgroups.getSGroup(j);
        if (sg.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            auto& sa = static_cast<Superatom&>(sg);
            if (sa.subscript.size() == 0 || std::string(sa.subscript.ptr()).size() == 0)
                bmol.sgroups.remove(j);
        }
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
