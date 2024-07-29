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

#include <algorithm>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/ket_commons.h"
#include "molecule/molecule.h"
#include "molecule/molecule_cdxml_loader.h"
#include "molecule/molecule_scaffold_detection.h"
#include "molecule/parse_utils.h"
#include "reaction/reaction.h"

using namespace indigo;
using namespace tinyxml2;
using namespace rapidjson;

bool is_fragment(CdxmlNode& node)
{
    return node.has_fragment || node.type == kCDXNodeType_Nickname || node.type == kCDXNodeType_Fragment;
}

/*/
static float readFloat(const char* point_str)
{
    float res = 0;
    if (point_str != 0)
    {
        BufferScanner strscan(point_str);
        res = strscan.readFloat();
    }
    return res;
}
//*/

IMPL_ERROR(MoleculeCdxmlLoader, "CDXML loader");
IMPL_ERROR(CDXMLReader, "CDXML reader");
IMPL_ERROR(CDXElement, "CDX element");
IMPL_ERROR(CDXProperty, "CDX property");
IMPL_ERROR(CDXMLElement, "CDXML element");
IMPL_ERROR(CDXMLProperty, "CDXML property");

CDXProperty::CDXProperty(CDXElement* parent, uint16_t tag, const uint8_t* data, uint32_t size) : _parent(parent), _tag(tag), _data(data), _size(size)
{
}

std::unique_ptr<CDXProperty> CDXProperty::nextProp()
{
    return _parent->getProperty(_data + _size);
}

static inline double unitsToPoints(double units)
{
    return round(units * 100 / kCDXUnitsPerPoint) / 100;
}

std::string CDXProperty::formatValue(ECDXType cdx_type) const
{
    std::string result;
    switch (cdx_type)
    {
    case ECDXType::CDXPoint2D:
    case ECDXType::CDXRectangle: {
        auto ptr32 = (int32_t*)_data;
        std::stringstream ss;
        ss << std::setprecision(2) << std::fixed;
        for (uint16_t i = 0; i < _size / sizeof(int32_t); ++i)
        {
            if (i)
                ss << " ";
            double val = ptr32[i ^ 1];
            val = unitsToPoints(val);
            ss << val;
        }
        result = ss.str();
    }
    break;

    case ECDXType::CDXPoint3D: {
        auto ptr32 = (int32_t*)_data;
        std::stringstream ss;
        ss << std::setprecision(2) << std::fixed;
        for (uint16_t i = 0; i < _size / sizeof(int32_t); ++i)
        {
            if (i)
                ss << " ";
            double val = ptr32[i];
            val = unitsToPoints(val);
            ss << val;
        }
        result = ss.str();
    }
    break;

    case ECDXType::CDXCoordinate: {
        auto ptr32 = (int32_t*)_data;
        std::stringstream ss;
        double val = *ptr32;
        val = unitsToPoints(val);
        ss << std::setprecision(2) << std::fixed << val;
        result = ss.str();
    }
    break;

    case ECDXType::CDXUINT16: {
        auto ptr16 = (uint16_t*)_data;
        result = parseCDXUINT16(*ptr16);
    }
    break;
    case ECDXType::CDXINT16: {
        int16_t val16 = _size == sizeof(int8_t) ? *((int8_t*)_data) : *((int16_t*)_data); // ChemDraw 8.0 bug fix
        result = parseCDXINT16(val16);
    }
    break;
    case ECDXType::CDXUINT8:
    case ECDXType::CDXINT8: {
        result = parseCDXINT8(*_data);
    }
    break;
    case ECDXType::CDXINT32: {
        auto ptr32 = (uint32_t*)_data;
        result = parseCDXINT32(*ptr32);
    }
    break;
    case ECDXType::CDXObjectID:
    case ECDXType::CDXUINT32: {
        auto ptr32 = (uint32_t*)_data;
        result = std::to_string(*ptr32);
    }
    break;
    case ECDXType::CDXObjectIDArray: {
        auto ptr32 = (uint32_t*)_data;
        for (uint16_t i = 0; i < _size / sizeof(uint32_t); ++i)
        {
            if (i)
                result += " ";
            result += std::to_string(ptr32[i]);
        }
    }
    break;
    case ECDXType::CDXString: {
        // get raw string.
        auto ptr16 = (uint16_t*)_data;
        int offset = (*ptr16) * sizeof(CDXTextStyle) + sizeof(uint16_t);
        uint32_t value_size = _size - offset;
        if (value_size > 0)
        {
            return std::string((char*)(_data + offset), value_size);
        }
        return std::string();
    }
    break;

    case ECDXType::CDXFLOAT64: {
        auto pflt = (double*)_data;
        result = std::to_string(*pflt);
    }
    break;

    case ECDXType::CDXBooleanImplied: {
        result = "yes";
    }
    break;

    case ECDXType::CDXBoolean: {
        result = *_data ? "yes" : "no";
    }
    break;

    case ECDXType::CDXColorTable:
        result = "ColorTable not implemented";
        break;

    case ECDXType::CDXFontTable:
        result = "FontTable not implemented";
        break;

    case ECDXType::CDXFontStyle:
        result = "FontStyle not implemented";
        break;

    case ECDXType::CDXUnformatted: {
        std::stringstream ss;
        ss << std::hex;
        std::vector<uint8_t> val_dump(_data, _data + _size);
        for (auto val : val_dump)
            ss << std::setw(2) << std::setfill('0') << (int)val;
        return ss.str();
    }
    break;
    case ECDXType::CDXINT16ListWithCounts: {
        auto pcount = (int16_t*)_data;
        auto ptr16 = pcount + 1;
        for (int i = 0; i < *pcount; ++i)
        {
            if (i)
                result += " ";
            result += std::to_string(ptr16[i]);
        }
    }
    break;
    default:
        throw Error("undefined property type: %d", cdx_type);
        break;
    }
    return result;
}

std::string CDXProperty::parseCDXUINT16(uint16_t val) const
{
    return std::to_string(val);
}

std::string CDXProperty::parseCDXINT16(int16_t val) const
{
    switch (_tag)
    {
    case kCDXProp_Bond_Order: {
        if (auto it = kBondOrderIntToStr.find(val); it != kBondOrderIntToStr.end())
            return kBondOrderIntToStr.at(val);
        std::string res;
        for (int i = 0; i < std::numeric_limits<uint16_t>::digits; i++)
        {
            uint16_t order_bit = 1 << i;
            if (order_bit & val)
            {
                if (res.length() > 0)
                    res += " ";
                res += kBondOrderIntToStr.at(order_bit);
            }
        }
        return res;
    }
    case kCDXProp_Node_Type: {
        return KNodeTypeIntToName.at(val);
    }
    break;
    case kCDXProp_Bond_Display:
    case kCDXProp_Bond_Display2: {
        return kCDXProp_Bond_DisplayIdToStr.at((CDXBondDisplay)val);
    }
    break;
    case kCDXProp_BondSpacing: {
        val /= kCDXBondSpacingMultiplier;
    }
    break;
    case kCDXProp_Graphic_Type: {
        return kCDXPropGraphicTypeIDToStr.at((CDXGraphicType)val);
    }
    break;
    case kCDXProp_Symbol_Type: {
        return kCDXPropSymbolTypeIDToStr.at((CDXSymbolType)val);
    }
    break;
    case kCDXProp_Arrow_Type: {
        return kCDXProp_Arrow_TypeIDToStr.at((CDXArrowType)val);
    }
    break;
    case kCDXProp_Arrowhead_Type: {
        auto it = kCDXProp_Arrow_ArrowHeadTypeIntToStr.find((CDXArrowheadType)val);
        if (it != kCDXProp_Arrow_ArrowHeadTypeIntToStr.end())
            return it->second;
        break;
    }
    default:
        break;
    }
    return std::to_string(val);
}

std::string CDXProperty::parseCDXINT32(int32_t val) const
{
    switch (_tag)
    {
    case kCDXProp_ChainAngle: {
        std::stringstream ss;
        ss << std::setprecision(2) << std::fixed << double(val) / kCDXAngleMultiplier;
        return ss.str();
    }
    default:
        break;
    }
    return std::to_string(val);
}

std::string CDXProperty::parseCDXINT8(int8_t val) const
{
    switch (_tag)
    {
    case kCDXProp_Atom_EnhancedStereoType:
        return kCDXEnhancedStereoIDToStr.at(val);
        break;
    case kCDXProp_Bond_CIPStereochemistry:
        return std::string{kCIPBondStereochemistryIndexToChar[val]};
        break;
    case kCDXProp_Atom_CIPStereochemistry:
        return std::string{kCIPStereochemistryIndexToChar[val]};
        break;
    case kCDXProp_Bracket_Usage:
        return std::string{kBracketUsageIntToName.at(val)};
        break;
    case kCDXProp_Justification:
    case kCDXProp_LabelJustification:
    case kCDXProp_CaptionJustification:
        return std::string(kTextJustificationIntToStr.at(val));
    case kCDXProp_Node_LabelDisplay:
    case kCDXProp_LabelAlignment:
        return std::string(kLabelAlignmentIntTostr.at(val));
    case kCDXProp_Atom_Geometry:
        return std::string(KGeometryTypeIntToName.at(val));
    case kCDXProp_Bond_RestrictRxnParticipation:
        return kBondReactionParticipationIntToName.at(val);
    case kCDXProp_Bond_RestrictTopology:
        return kBondTopologyIntToName.at(val);
    default:
        break;
    }
    return std::to_string(val);
}

std::unique_ptr<CDXElement> CDXElement::getChild(const uint8_t* ptr)
{
    uint16_t tag;
    ptr = get_uint16_t(ptr, tag);
    while (tag > 0 && tag < kCDXTag_Object && tag != kCDXProp_Text)
    {
        ptr = skipProperty(ptr);
        ptr = get_uint16_t(ptr, tag);
    }
    if (tag == 0)
        return std::make_unique<CDXElement>();
    uint32_t new_size = _data_size - static_cast<uint32_t>(ptr - _data);
    if (tag == kCDXProp_Text)
    {
        ptr = get_property_size(ptr, new_size);
        return std::make_unique<CDXTextElement>(tag, ptr, new_size, 0);
    }
    return std::make_unique<CDXElement>(tag, ptr, new_size);
}

CDXReader::CDXReader(Scanner& scanner) : _scanner(scanner)
{
    scanner.readAll(_buffer);
}

MoleculeCdxmlLoader::MoleculeCdxmlLoader(Scanner& scanner, bool is_binary, bool is_fragment)
    : _scanner(scanner), _is_binary(is_binary), _is_fragment(is_fragment), _has_bounding_box(false), _pmol(nullptr), _pqmol(nullptr), ignore_bad_valence(false)
{
}

void MoleculeCdxmlLoader::_initMolecule(BaseMolecule& mol)
{
    mol.clear();
    nodes.clear();
    bonds.clear();
    _arrows.clear();
    _graphic_arrows.clear();
    _primitives.clear();
    _id_to_atom_idx.clear();
    _id_to_node_index.clear();
    _id_to_bond_index.clear();
    _fragment_nodes.clear();
    text_objects.clear();
    _pluses.clear();
    brackets.clear();
    _pmol = NULL;
    _pqmol = NULL;
    if (mol.isQueryMolecule())
    {
        _pqmol = &mol.asQueryMolecule();
    }
    else
    {
        _pmol = &mol.asMolecule();
        _pmol->setIgnoreBadValenceFlag(ignore_bad_valence);
    }
}

void MoleculeCdxmlLoader::loadMolecule(BaseMolecule& mol, bool load_arrows)
{
    _initMolecule(mol);
    _has_scheme = false;
    std::unique_ptr<CDXReader> cdx_reader = _is_binary ? std::make_unique<CDXReader>(_scanner) : std::make_unique<CDXMLReader>(_scanner);
    cdx_reader->process();
    auto root = cdx_reader->rootElement();

    if (_is_fragment)
    {
        loadMoleculeFromFragment(mol, *root);
        return;
    }

    parseCDXMLAttributes(*root->firstProperty());
    _parseCDXMLPage(*root);

    _parseCollections(mol);
    int arrows_count = mol.meta().getMetaCount(KETReactionArrow::CID);
    if (arrows_count && !load_arrows && _has_scheme)
        throw Error("Not a molecule. Found %d arrows.", arrows_count);
}

void MoleculeCdxmlLoader::_checkFragmentConnection(int node_id, int bond_id)
{
    auto& fn = nodes[_id_to_node_index.at(node_id)];
    if (fn.ext_connections.size())
    {
        if (is_fragment(fn) && fn.ext_connections.size() == 1)
        {
            fn.bond_id_to_connection_idx.emplace(bond_id, fn.connections.size());
            int pid = fn.ext_connections.back();
            fn.node_id_to_connection_idx.emplace(pid, fn.connections.size());
            fn.connections.push_back(_ExtConnection{bond_id, pid, -1});
        }
        else
            throw Error("Unsupported node connectivity for bond id: %d", bond_id);
    }
}

void MoleculeCdxmlLoader::_parseCollections(BaseMolecule& mol)
{
    std::vector<int> atoms;
    for (auto& node : nodes)
    {
        int node_idx = static_cast<int>(_id_to_node_index.at(node.id));
        switch (node.type)
        {
        case kCDXNodeType_NamedAlternativeGroup:
        case kCDXNodeType_Element:
        case kCDXNodeType_ElementList:
        case kCDXNodeType_GenericNickname:
            atoms.push_back(node_idx);
            break;
        case kCDXNodeType_Unspecified:
            if (node.has_fragment)
                _fragment_nodes.push_back(node_idx);
            else
                atoms.push_back(node_idx);
            break;
        case kCDXNodeType_ExternalConnectionPoint: {
            if (_fragment_nodes.size())
            {
                auto& fn = nodes[_fragment_nodes.back()];
                if (fn.connections.size() == 0)
                {
                    fn.ext_connections.push_back(node.id);
                }
            }
            else
            {
                // handle free external connection. attachment point?
            }
        }
        break;
        case kCDXNodeType_AnonymousAlternativeGroup:
        case kCDXNodeType_Nickname:
        case kCDXNodeType_Fragment:
            _fragment_nodes.push_back(node_idx);
            break;
        default:
            break;
        }
    }

    for (const auto& bond : bonds)
    {
        _checkFragmentConnection(bond.be.first, bond.id);
        _checkFragmentConnection(bond.be.second, bond.id);
    }

    _addAtomsAndBonds(mol, atoms, bonds);

    _processEnhancedStereo(mol);

    for (auto& brk : brackets)
        _addBracket(mol, brk);

    for (const auto& to : text_objects)
        mol.meta().addMetaObject(new KETTextObject(to.first, to.second));

    for (const auto& plus : _pluses)
        mol.meta().addMetaObject(new KETReactionPlus(plus));

    // CDX contains draphic arrow wich id dublicate arrow/
    // Search arrows for arrow with coords same as in grapic arrow and if found - remove tis arrow gecause graphic arrow contains more specific type
    for (const auto& g_arrow : _graphic_arrows)
    {
        const auto& g_arr_info = g_arrow.first;
        Vec2f p1(g_arr_info.first.x, g_arr_info.first.y);
        Vec2f p2(g_arr_info.second.x, g_arr_info.second.y);
        for (auto it = _arrows.begin(); it != _arrows.end(); it++)
        {
            const auto& arr_info = (*it).first;
            Vec2f ap1(arr_info.first.x, arr_info.first.y);
            Vec2f ap2(arr_info.second.x, arr_info.second.y);
            if (fabsf(p1.x - ap1.x) < EPSILON && fabsf(p1.y - ap1.y) < EPSILON && fabsf(p2.x - ap2.x) < EPSILON && fabsf(p2.y - ap2.y) < EPSILON)
            {
                _arrows.erase(it);
                break;
            }
        }
        mol.meta().addMetaObject(new KETReactionArrow(g_arrow.second, p1, p2));
    }

    for (const auto& arrow : _arrows)
    {
        const auto& arr_info = arrow.first;
        Vec2f v1(arr_info.first.x, arr_info.first.y);
        Vec2f v2(arr_info.second.x, arr_info.second.y);
        mol.meta().addMetaObject(new KETReactionArrow(arrow.second, v1, v2));
    }

    for (const auto& prim : _primitives)
    {
        if (prim.second == kCDXGraphicType_Rectangle)
            mol.meta().addMetaObject(new KETSimpleObject(KETSimpleObject::EKETRectangle, prim.first));
    }
}

void MoleculeCdxmlLoader::_processEnhancedStereo(BaseMolecule& mol)
{
    std::vector<int> ignore_cistrans(mol.edgeCount());
    std::vector<int> sensible_bond_directions(mol.edgeCount());
    for (int i = 0; i < mol.edgeCount(); i++)
        if (mol.getBondDirection(i) == BOND_EITHER)
        {
            if (MoleculeCisTrans::isGeomStereoBond(mol, i, 0, true))
            {
                ignore_cistrans[i] = true;
                sensible_bond_directions[i] = true;
            }
            else
            {
                int k;
                const Vertex& v = mol.getVertex(mol.getEdge(i).beg);

                for (k = v.neiBegin(); k != v.neiEnd(); k = v.neiNext(k))
                {
                    if (MoleculeCisTrans::isGeomStereoBond(mol, v.neiEdge(k), 0, true))
                    {
                        ignore_cistrans[v.neiEdge(k)] = true;
                        sensible_bond_directions[i] = true;
                        break;
                    }
                }
            }
        }
        else if (mol.cis_trans.isIgnored(i))
            ignore_cistrans[i] = true;

    mol.buildFromBondsStereocenters(stereochemistry_options, sensible_bond_directions.data());
    mol.buildFromBondsAlleneStereo(stereochemistry_options.ignore_errors, sensible_bond_directions.data());

    if (!mol.getChiralFlag())
        for (int i : mol.vertices())
        {
            int type = mol.stereocenters.getType(i);
            if (type == MoleculeStereocenters::ATOM_ABS)
                mol.stereocenters.setType(i, MoleculeStereocenters::ATOM_AND, 1);
        }

    mol.buildCisTrans(ignore_cistrans.data());
    mol.have_xyz = true;
    if (mol.stereocenters.size() == 0)
    {
        mol.buildFrom3dCoordinatesStereocenters(stereochemistry_options);
    }

    for (const auto& sc : _stereo_centers)
    {
        if (mol.stereocenters.getType(sc.atom_idx) == 0)
        {
            if (!stereochemistry_options.ignore_errors)
                throw Error("stereo type specified for atom #%d, but the bond "
                            "directions does not say that it is a stereocenter",
                            sc.atom_idx);
            mol.addStereocentersIgnoreBad(sc.atom_idx, sc.type, sc.group, false); // add non-valid stereocenters
        }
        else
            mol.stereocenters.setType(sc.atom_idx, sc.type, sc.group);
    }
}

void MoleculeCdxmlLoader::loadMoleculeFromFragment(BaseMolecule& mol, BaseCDXElement& elem)
{
    _initMolecule(mol);
    _parseCDXMLElements(elem, true);
    _parseCollections(mol);
}

void MoleculeCdxmlLoader::parseCDXMLAttributes(BaseCDXProperty& prop)
{
    auto cdxml_bbox_lambda = [this](const std::string& data) {
        std::vector<std::string> coords = split(data, ' ');
        if (coords.size() == 4)
        {
            this->_has_bounding_box = true;
            this->cdxml_bbox = Rect2f(Vec2f(std::stof(coords[0]), std::stof(coords[1])), Vec2f(std::stof(coords[2]), std::stof(coords[3])));
        }
        else
            throw Error("Not enought coordinates for atom position");
    };

    auto& bond_length = cdxml_bond_length;
    auto cdxml_bond_length_lambda = [&bond_length](const std::string& data) { bond_length = data; };
    std::unordered_map<std::string, std::function<void(const std::string&)>> cdxml_dispatcher = {{"BoundingBox", cdxml_bbox_lambda},
                                                                                                 {"BondLength", cdxml_bond_length_lambda}};
    applyDispatcher(prop, cdxml_dispatcher);
}

void MoleculeCdxmlLoader::_parseCDXMLPage(BaseCDXElement& elem)
{
    for (auto page_elem = elem.firstChildElement(); page_elem->hasContent(); page_elem = page_elem->nextSiblingElement())
    {
        if (page_elem->value() == "page")
        {
            auto cdxml_elem = page_elem->firstChildElement();
            _parseCDXMLElements(*cdxml_elem);
            for (; cdxml_elem->hasContent(); cdxml_elem = cdxml_elem->nextSiblingElement())
            {
                if (cdxml_elem->value() == "scheme")
                    _has_scheme = true;
            }
        }
    }
}

void MoleculeCdxmlLoader::_parseCDXMLElements(BaseCDXElement& first_elem, bool no_siblings, bool inside_fragment_node)
{
    int fragment_start_idx = -1;

    auto node_lambda = [this](BaseCDXElement& elem) {
        CdxmlNode node;
        this->_parseNode(node, elem);
        _addNode(node);
        if (node.has_fragment)
        {
            auto inner_idx_start = nodes.size();
            this->_parseCDXMLElements(*elem.firstChildElement(), false, true);
            auto inner_idx_end = nodes.size();
            CdxmlNode& fragment_node = nodes[inner_idx_start - 1];
            for (auto i = inner_idx_start; i < inner_idx_end; ++i)
            {
                auto it = std::upper_bound(fragment_node.inner_nodes.cbegin(), fragment_node.inner_nodes.cend(), fragment_node.id,
                                           [](int a, int b) { return a > b; });
                if (nodes[i].pos.x == 0 && nodes[i].pos.y == 0 && nodes[i].pos.z == 0) // if no coord - copy from parent
                    nodes[i].pos = fragment_node.pos;
                fragment_node.inner_nodes.insert(it, nodes[i].id);
            }
        }
    };

    auto bond_lambda = [this](BaseCDXElement& elem) {
        CdxmlBond bond;
        this->_parseBond(bond, *elem.firstProperty());
        this->bonds.push_back(bond);
        this->_id_to_bond_index.emplace(bond.id, bonds.size() - 1);
    };

    auto fragment_lambda = [this, &fragment_start_idx](BaseCDXElement& elem) {
        fragment_start_idx = static_cast<int>(nodes.size());
        this->_parseFragmentAttributes(*elem.firstProperty());
        this->_parseCDXMLElements(*elem.firstChildElement());
    };

    auto group_lambda = [this](BaseCDXElement& elem) { this->_parseCDXMLElements(*elem.firstChildElement()); };

    auto bracketed_lambda = [this](BaseCDXElement& elem) {
        CdxmlBracket bracket;
        this->_parseBracket(bracket, *elem.firstProperty());
        this->brackets.push_back(bracket);
    };

    auto text_lambda = [this, &fragment_start_idx, inside_fragment_node](BaseCDXElement& elem) {
        if (fragment_start_idx >= 0 && inside_fragment_node)
        {
            CdxmlBracket bracket;
            bracket.is_superatom = true;
            for (size_t node_idx = fragment_start_idx; node_idx < this->nodes.size(); ++node_idx)
            {
                auto& node = this->nodes[node_idx];
                if (node.type == kCDXNodeType_Element || node.type == kCDXNodeType_ElementList)
                {
                    bracket.bracketed_list.push_back(node.id);
                }
            }
            this->_parseLabel(elem, bracket.label);
            this->brackets.push_back(bracket);
        }
        else
            this->_parseText(elem, this->text_objects);
    };

    auto graphic_lambda = [this](BaseCDXElement& elem) { this->_parseGraphic(elem); };

    auto arrow_lambda = [this](BaseCDXElement& elem) { this->_parseArrow(elem); };

    auto altgroup_lambda = [this](BaseCDXElement& elem) { this->_parseAltGroup(elem); };

    std::unordered_map<std::string, std::function<void(BaseCDXElement & elem)>> cdxml_dispatcher = {
        {"n", node_lambda},          {"b", bond_lambda},      {"fragment", fragment_lambda}, {"group", group_lambda}, {"bracketedgroup", bracketed_lambda},
        {"graphic", graphic_lambda}, {"arrow", arrow_lambda}, {"altgroup", altgroup_lambda}};

    for (auto pelem = first_elem.copy(); pelem->hasContent(); pelem = pelem->nextSiblingElement())
    {
        auto it = cdxml_dispatcher.find(pelem->value());
        if (it != cdxml_dispatcher.end())
        {
            it->second(*pelem);
        }
        else
        {
        }
        if (no_siblings)
            break;
    }
    // Text elements should be processed after all others because it behavior depends on fragment lambda
    for (auto pelem = first_elem.copy(); pelem->hasContent(); pelem = pelem->nextSiblingElement())
    {
        if (pelem->value() == "t")
            text_lambda(*pelem);
        if (no_siblings)
            break;
    }
}

void MoleculeCdxmlLoader::_updateConnection(const CdxmlNode& node, int atom_idx)
{
    for (auto fidx : _fragment_nodes)
    {
        auto& frag_node = nodes[fidx];
        auto fit = frag_node.node_id_to_connection_idx.find(node.id);
        if (fit != frag_node.node_id_to_connection_idx.end())
        {
            auto& conn = frag_node.connections[fit->second];
            conn.atom_idx = atom_idx;
        }
    }
}

int MoleculeCdxmlLoader::_addBond(Molecule& mol, const CdxmlBond& bond, int begin, int end)
{
    // bond topology is allowed only for queries but queries not supported for now
    // if (bond.topology > 0 && _pqmol == nullptr)
    //     throw Error("bond topology is allowed only for queries");
    int order = bond.order;
    int direction = bond.dir;

    if (order == BOND_AROMATIC && bond.display == kCDXBondDisplay_Solid && bond.display2 == kCDXBondDisplay_Dash) // tautomeric bond
    {
        order = _BOND_SINGLE_OR_DOUBLE;
    }

    int bond_idx = _pmol != nullptr ? _pmol->addBond_Silent(begin, end, order)
                                    : _pqmol->addBond(begin, end, QueryMolecule::createQueryMoleculeBond(order, bond.topology, direction));
    if (order == BOND_DOUBLE && direction == BOND_EITHER)
        mol.cis_trans.ignore(bond_idx);
    else if (direction > 0 && order == BOND_SINGLE)
        mol.setBondDirection(bond_idx, direction);
    if (bond.reaction_center > 0)
        mol.reaction_bond_reacting_center[bond_idx] = bond.reaction_center;

    return bond_idx;
}

void MoleculeCdxmlLoader::_addAtomsAndBonds(BaseMolecule& mol, const std::vector<int>& atoms, const std::vector<CdxmlBond>& new_bonds)
{
    _id_to_atom_idx.clear();
    mol.reaction_atom_mapping.clear_resize(static_cast<int>(atoms.size()));
    mol.reaction_atom_mapping.zerofill();
    mol.reaction_atom_inversion.clear_resize(static_cast<int>(atoms.size()));
    mol.reaction_atom_inversion.zerofill();
    mol.reaction_atom_exact_change.clear_resize(static_cast<int>(atoms.size()));
    mol.reaction_atom_exact_change.zerofill();

    for (auto atom_idx : atoms)
    {
        auto& atom = nodes[atom_idx];
        if (_pmol)
        {
            atom_idx = _pmol->addAtom(atom.element);

            if (atom.type == kCDXNodeType_NamedAlternativeGroup)
                mol.allowRGroupOnRSite(atom_idx, atom.rg_index);

            _id_to_atom_idx.emplace(atom.id, atom_idx);
            mol.setAtomXyz(atom_idx, atom.pos);
            _pmol->setAtomCharge_Silent(atom_idx, atom.charge);
            if (atom.valence)
                _pmol->setExplicitValence(atom_idx, atom.valence);
            _pmol->setAtomRadical(atom_idx, atom.radical);
            _pmol->setAtomIsotope(atom_idx, atom.isotope);
            if (atom.type == kCDXNodeType_GenericNickname || atom.element == ELEM_PSEUDO)
                _pmol->setPseudoAtom(atom_idx, atom.label.c_str());
            switch (atom.enchanced_stereo)
            {
            case EnhancedStereoType::ABSOLUTE:
                _stereo_centers.emplace_back(atom_idx, MoleculeStereocenters::ATOM_ABS, 1);
                break;
            case EnhancedStereoType::AND:
                if (atom.enhanced_stereo_group)
                    _stereo_centers.emplace_back(atom_idx, MoleculeStereocenters::ATOM_AND, atom.enhanced_stereo_group);
                break;
            case EnhancedStereoType::OR:
                if (atom.enhanced_stereo_group)
                    _stereo_centers.emplace_back(atom_idx, MoleculeStereocenters::ATOM_OR, atom.enhanced_stereo_group);
                break;
            default:
                break;
            }
        }
        else
        {
            throw Error("Queries not supported");
        }
    }

    for (const auto& bond : new_bonds)
    {
        int bond_idx;
        if (_pmol)
        {
            auto bond_first_it = _id_to_atom_idx.find(bond.be.first);
            auto bond_second_it = _id_to_atom_idx.find(bond.be.second);
            auto& fn = nodes[_id_to_node_index.at(bond.be.first)];
            auto& sn = nodes[_id_to_node_index.at(bond.be.second)];
            if (bond_first_it != _id_to_atom_idx.end() && bond_second_it != _id_to_atom_idx.end())
            {
                if (bond.swap_bond)
                    bond_idx = _addBond(*_pmol, bond, bond_second_it->second, bond_first_it->second);
                else
                    bond_idx = _addBond(*_pmol, bond, bond_first_it->second, bond_second_it->second);
            }
            else if (fn.type == kCDXNodeType_ExternalConnectionPoint && bond_second_it != _id_to_atom_idx.end())
            {
                _updateConnection(fn, bond_second_it->second);
            }
            else if (sn.type == kCDXNodeType_ExternalConnectionPoint && bond_first_it != _id_to_atom_idx.end())
            {
                _updateConnection(sn, bond_first_it->second);
            }
            else if (is_fragment(fn) && bond_second_it != _id_to_atom_idx.end())
            {
                auto bit_beg = fn.bond_id_to_connection_idx.find(bond.id);
                int a1 = -1;
                int a2 = bond_second_it->second;

                if (bit_beg == fn.bond_id_to_connection_idx.end())
                {
                    if (fn.inner_nodes.size())
                    {
                        auto cp_id = fn.inner_nodes.back();
                        auto it = _id_to_atom_idx.find(cp_id);
                        if (it != _id_to_atom_idx.end())
                        {
                            a1 = it->second;
                        }
                        else
                            throw Error("unable to connect node %d", a1);
                    }
                    else
                        throw Error("orphaned node %d", a1);
                }
                else
                {
                    a1 = fn.connections[bit_beg->second].atom_idx;
                }

                if (a1 >= 0 && a2 >= 0)
                {
                    std::ignore = _addBond(*_pmol, bond, a1, a2);
                }
            }
            else if (is_fragment(sn) && bond_first_it != _id_to_atom_idx.end())
            {
                auto bit_beg = sn.bond_id_to_connection_idx.find(bond.id);
                int a1 = bond_first_it->second;
                int a2 = -1;
                if (bit_beg == sn.bond_id_to_connection_idx.end())
                {
                    if (sn.inner_nodes.size())
                    {
                        auto cp_id = sn.inner_nodes.front();
                        auto it = _id_to_atom_idx.find(cp_id);
                        if (it != _id_to_atom_idx.end())
                        {
                            a2 = it->second;
                        }
                        else
                            throw Error("unable to connect node %d", a1);
                    }
                    else
                        throw Error("orphaned node %d", a1);
                }
                else
                    a2 = sn.connections[bit_beg->second].atom_idx;
                if (a1 >= 0 && a2 >= 0)
                {
                    std::ignore = _addBond(*_pmol, bond, a1, a2);
                }
            }
            else if (is_fragment(fn) && is_fragment(sn))
            {
                auto bit_beg = fn.bond_id_to_connection_idx.find(bond.id);
                auto bit_end = sn.bond_id_to_connection_idx.find(bond.id);
                if (bit_beg != fn.bond_id_to_connection_idx.end() && bit_end != sn.bond_id_to_connection_idx.end())
                {
                    int a1 = fn.connections[bit_beg->second].atom_idx;
                    int a2 = sn.connections[bit_end->second].atom_idx;

                    std::ignore = _addBond(*_pmol, bond, a1, a2);
                }
            }
            else
            {
                throw Error("orphaned node!!!");
            }
        }
    }
}

void MoleculeCdxmlLoader::_addBracket(BaseMolecule& mol, const CdxmlBracket& bracket)
{
    static const std::unordered_map<int, int> implemeted_brackets = {
        {kCDXBracketUsage_SRU, SGroup::SG_TYPE_SRU}, {kCDXBracketUsage_MultipleGroup, SGroup::SG_TYPE_MUL}, {kCDXBracketUsage_Generic, SGroup::SG_TYPE_GEN}};

    auto it = implemeted_brackets.find(bracket.usage);
    if (it != implemeted_brackets.end())
    {
        int grp_idx = mol.sgroups.addSGroup(bracket.is_superatom ? SGroup::SG_TYPE_SUP : it->second);
        SGroup& sgroup = mol.sgroups.getSGroup(grp_idx);
        std::unordered_set<int> sgroup_atoms;
        for (auto atom_id : bracket.bracketed_list)
        {
            int atom_idx = _id_to_atom_idx.at(atom_id);
            sgroup.atoms.push(atom_idx);
            sgroup_atoms.insert(atom_idx);
            if (bracket.usage == kCDXBracketUsage_MultipleGroup)
            {
                MultipleGroup& mg = (MultipleGroup&)sgroup;
                if (mg.multiplier)
                    mg.parent_atoms.push(atom_idx);
            }
        }

        // add brackets
        Vec2f* p = sgroup.brackets.push();
        p[0].set(0, 0);
        p[1].set(0, 0);
        p = sgroup.brackets.push();
        p[0].set(0, 0);
        p[1].set(0, 0);
        // sgroup.brk_style
        if (bracket.is_superatom)
        {
            Superatom& sa = (Superatom&)sgroup;
            sa.contracted = DisplayOption::Contracted;
            sa.subscript.readString(bracket.label.c_str(), true);
        }
        else
            switch (bracket.usage)
            {
            case kCDXBracketUsage_SRU: {
                RepeatingUnit& ru = (RepeatingUnit&)sgroup;
                ru.connectivity = bracket.repeat_pattern;
                ru.subscript.readString(bracket.label.c_str(), true);
            }
            break;
            case kCDXBracketUsage_MultipleGroup: {
                MultipleGroup& mg = (MultipleGroup&)sgroup;
                if (bracket.repeat_count)
                {
                    mg.multiplier = bracket.repeat_count;
                }
            }
            break;
            case kCDXBracketUsage_Generic:
                break;
            default:
                break;
            }
        _handleSGroup(sgroup, sgroup_atoms, mol);
    }
}

void MoleculeCdxmlLoader::_handleSGroup(SGroup& sgroup, const std::unordered_set<int>& atoms, BaseMolecule& bmol)
{
    int start = -1;
    int start_neighbor = -1;
    int end = -1;
    int end_neighbor = -1;
    int end_bond = -1, start_bond = -1;
    QS_DEF(Array<int>, xbonds);

    for (auto j : bmol.edges())
    {
        if (!bmol.hasEdge(j))
            continue;

        const Edge& edge = bmol.getEdge(j);
        auto itbeg = atoms.find(edge.beg);
        auto itend = atoms.find(edge.end);

        if (itbeg == atoms.end() && itend == atoms.end())
            continue;
        if (itbeg != atoms.end() && itend != atoms.end())
            continue;
        else
        {
            // bond going out of the sgroup
            xbonds.push(j);
            if (start_bond == -1)
            {
                start_bond = j;
                start = itbeg != atoms.end() ? *itbeg : *itend;
            }
            else if (end_bond == -1)
            {
                end_bond = j;
                end = itbeg != atoms.end() ? *itbeg : *itend;
            }
        }
    }

    if (sgroup.sgroup_type == SGroup::SG_TYPE_SUP && start >= 0)
    {
        Superatom& sa = static_cast<Superatom&>(sgroup);
        sa.attachment_points.add(start);
        if (end >= 0)
            sa.attachment_points.add(end);
    }
    else if (sgroup.sgroup_type == SGroup::SG_TYPE_MUL)
    {
        QS_DEF(Array<int>, mapping);
        std::unique_ptr<BaseMolecule> rep(bmol.neu());
        rep->makeSubmolecule(bmol, sgroup.atoms, &mapping, 0);

        rep->sgroups.clear(SGroup::SG_TYPE_SRU);
        rep->sgroups.clear(SGroup::SG_TYPE_MUL);

        int rep_start = mapping[start];
        int rep_end = mapping[end];
        MultipleGroup& mg = (MultipleGroup&)sgroup;
        if (mg.multiplier > 1)
        {
            int start_order = start_bond > 0 ? bmol.getBondOrder(start_bond) : -1;
            int end_order = end_bond > 0 ? bmol.getBondOrder(end_bond) : -1;
            for (int j = 0; j < mg.multiplier - 1; j++)
            {
                bmol.mergeWithMolecule(*rep, &mapping, 0);
                int k;
                for (k = rep->vertexBegin(); k != rep->vertexEnd(); k = rep->vertexNext(k))
                    sgroup.atoms.push(mapping[k]);
                if (rep_end >= 0 && end_bond >= 0)
                {
                    int external = bmol.getEdge(end_bond).findOtherEnd(end);
                    bmol.removeBond(end_bond);
                    if (_pmol != 0)
                    {
                        _pmol->addBond(end, mapping[rep_start], start_order);
                        end_bond = _pmol->addBond(mapping[rep_end], external, end_order);
                    }
                    else
                    {
                        _pqmol->addBond(end, mapping[rep_start], new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, start_order));
                        end_bond = _pqmol->addBond(mapping[rep_end], external, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, end_order));
                    }
                    end = mapping[rep_end];
                }
            }
        }
    }
}

void MoleculeCdxmlLoader::_parseFragmentAttributes(BaseCDXProperty& prop)
{
    for (auto prop_ptr = prop.copy(); prop_ptr->hasContent(); prop_ptr = prop_ptr->next())
    {
        // it means that we are inside of NodeType=Fragment
        // let's check it
        if (nodes.size() && is_fragment(nodes.back()))
        {
            if (std::string(prop_ptr->name()) == "ConnectionOrder")
            {
                auto& fn = nodes.back();
                auto vec_str = split(prop_ptr->value(), ' ');
                if (fn.connections.size() == vec_str.size())
                {
                    for (size_t i = 0; i < vec_str.size(); ++i)
                    {
                        auto pid = std::stoi(vec_str[i]);
                        fn.connections[i].point_id = pid;
                        fn.node_id_to_connection_idx.emplace(pid, i);
                    }
                }
                else
                    throw Error("BondOrdering and ConnectionOrder sizes are not equal");
            }
        }
    }
}

void MoleculeCdxmlLoader::applyDispatcher(BaseCDXProperty& prop, const std::unordered_map<std::string, std::function<void(const std::string&)>>& dispatcher)
{
    for (auto ptr = prop.copy(); ptr->hasContent(); ptr = ptr->next())
    {
        auto it = dispatcher.find(ptr->name());
        if (it != dispatcher.end())
        {
            std::string str_arg(ptr->value());
            it->second(str_arg);
        }
    }
}

void MoleculeCdxmlLoader::_parseNode(CdxmlNode& node, BaseCDXElement& elem)
{
    // Atom parsing lambdas definition
    auto id_lambda = [&node](const std::string& data) { node.id = data; };
    auto hydrogens_lambda = [&node](const std::string& data) { node.hydrogens = data; };
    auto charge_lambda = [&node](const std::string& data) { node.charge = data; };
    auto element_lambda = [&node](const std::string& data) { node.element = data; };
    auto isotope_lambda = [&node](const std::string& data) { node.isotope = data; };
    auto radical_lambda = [&node](const std::string& data) {
        auto rd_it = kRadicalStrToId.find(data);
        if (rd_it != kRadicalStrToId.end())
            node.radical = rd_it->second;
    };
    auto label_lambda = [&node](const std::string& data) { node.label = data; };

    auto bond_ordering_lambda = [&node](const std::string& data) {
        auto vec_str = split(data, ' ');
        for (auto& str : vec_str)
        {
            auto bid = std::stoi(str);
            node.connections.push_back(_ExtConnection{bid, 0, -1});
            node.bond_id_to_connection_idx.emplace(bid, node.connections.size() - 1);
        }
    };

    auto pos_lambda = [&node, this](const std::string& data) { this->parsePos(data, node.pos); };

    auto stereo_lambda = [&node](const std::string& data) { node.stereo = kCIPStereochemistryCharToIndex.at(data.front()); };

    auto node_type_lambda = [&node](const std::string& data) {
        node.type = KNodeTypeNameToInt.at(data);
        if (node.type == kCDXNodeType_NamedAlternativeGroup)
            node.element = ELEM_RSITE;
    };

    auto element_list_lambda = [&node](const std::string& data) {
        std::vector<std::string> elements = split(data, ' ');
        if (elements.size() && elements.front().compare("NOT") == 0)
        {
            elements.erase(elements.begin());
            node.is_not_list = true;
        }
        node.element_list.assign(elements.begin(), elements.end());
    };

    auto geometry_lambda = [&node](const std::string& data) { node.geometry = KGeometryTypeNameToInt.at(data); };

    auto enhanced_stereo_type_lambda = [&node](const std::string& data) { node.enchanced_stereo = kCDXEnhancedStereoStrToID.at(data); };

    auto enhanced_stereo_group_lambda = [&node](const std::string& data) { node.enhanced_stereo_group = data; };

    auto alt_group_id_lambda = [&node](const std::string& data) { node.alt_group_id = data; };

    std::unordered_map<std::string, std::function<void(const std::string&)>> node_dispatcher = {{"id", id_lambda},
                                                                                                {"p", pos_lambda},
                                                                                                {"xyz", pos_lambda},
                                                                                                {"NumHydrogens", hydrogens_lambda},
                                                                                                {"Charge", charge_lambda},
                                                                                                {"Isotope", isotope_lambda},
                                                                                                {"Radical", radical_lambda},
                                                                                                {"AS", stereo_lambda},
                                                                                                {"NodeType", node_type_lambda},
                                                                                                {"Element", element_lambda},
                                                                                                {"GenericNickname", label_lambda},
                                                                                                {"ElementList", element_list_lambda},
                                                                                                {"BondOrdering", bond_ordering_lambda},
                                                                                                {"Geometry", geometry_lambda},
                                                                                                {"EnhancedStereoType", enhanced_stereo_type_lambda},
                                                                                                {"EnhancedStereoGroupNum", enhanced_stereo_group_lambda},
                                                                                                {"AltGroupID", alt_group_id_lambda}};
    applyDispatcher(*elem.firstProperty().get(), node_dispatcher);
    for (auto child_elem = elem.firstChildElement(); child_elem->hasContent(); child_elem = child_elem->nextSiblingElement())
    {
        if (child_elem->name() == "t")
        {
            std::string label;
            _parseLabel(*child_elem, label);
            if (label.size() > 1 && label.find("R") == 0)
            {
                try
                {
                    node.rg_index = label.substr(1);
                    node.type = kCDXNodeType_NamedAlternativeGroup;
                    node.element = ELEM_RSITE;
                }
                catch (const std::exception&)
                {
                    // not a R-Group
                }
            }
            if (node.element == ELEM_C) // overridable
            {
                auto element = Element::fromString2(label.c_str());
                if (element > 0)
                    node.element = element;
                else if (node.label.empty())
                {
                    node.label = label;
                    node.element = ELEM_PSEUDO;
                }
            }
        }
        else if (child_elem->name() == "fragment")
        {
            node.has_fragment = true;
        }
    }
}

void MoleculeCdxmlLoader::_addNode(CdxmlNode& node)
{
    nodes.push_back(node);
    _id_to_node_index.emplace(node.id, nodes.size() - 1);
}

void MoleculeCdxmlLoader::_parseBond(CdxmlBond& bond, BaseCDXProperty& prop)
{
    auto id_lambda = [&bond](const std::string& data) { bond.id = data; };
    auto bond_begin_lambda = [&bond](const std::string& data) { bond.be.first = data; };
    auto bond_end_lambda = [&bond](const std::string& data) { bond.be.second = data; };
    auto bond_order_lambda = [&bond](const std::string& data) {
        uint16_t bond_order = 0;
        for (auto order : split(data, ' '))
        {
            bond_order |= kBondOrderStrToId.at(order);
        }
        static const std::unordered_map<uint16_t, int> order_map = {
            {kCDXBondOrder_Single, BOND_SINGLE},
            {kCDXBondOrder_Double, BOND_DOUBLE},
            {kCDXBondOrder_Triple, BOND_TRIPLE},
            {kCDXBondOrder_OneHalf, BOND_AROMATIC},
            {kCDXBondOrder_Dative, _BOND_COORDINATION},
            {kCDXBondOrder_Hydrogen, _BOND_HYDROGEN},
            {kCDXBondOrder_SingleOrDouble, _BOND_SINGLE_OR_DOUBLE},
            {kCDXBondOrder_SingleOrAromatic, _BOND_SINGLE_OR_AROMATIC},
            {kCDXBondOrder_DoubleOrAromatic, _BOND_DOUBLE_OR_AROMATIC},
            {kCDXBondOrder_Any, _BOND_ANY},
        };
        if (auto it = order_map.find(bond_order); it != order_map.end())
            bond.order = order_map.at(bond_order);
        else
            bond.order = BOND_SINGLE; // Unsupported bond type fall down to single
    };

    auto stereo_lambda = [&bond](const std::string& data) { bond.stereo = kCIPBondStereochemistryCharToIndex.at(data.front()); };

    auto bond_display_lambda = [&bond](const std::string& data) {
        auto dislay_it = kCDXProp_Bond_DisplayStrToID.find(data);
        if (dislay_it != kCDXProp_Bond_DisplayStrToID.end())
        {
            bond.display = dislay_it->second;
        }
        auto disp_it = display_to_direction.find(bond.display);
        if (disp_it != display_to_direction.end())
        {
            auto& dir = disp_it->second;
            bond.dir = dir.first;
            bond.swap_bond = dir.second;
        }
    };

    auto bond_display2_lambda = [&bond](const std::string& data) {
        auto disp_it = kCDXProp_Bond_DisplayStrToID.find(data);
        if (disp_it != kCDXProp_Bond_DisplayStrToID.end())
        {
            bond.display2 = disp_it->second;
        }
    };

    auto reaction_center_lambda = [&bond](const std::string& data) {
        uint8_t rxn_participation = kBondReactionParticipationNameToInt.at(data);
        auto it = bond_rxn_participation_to_reaction_center.find(rxn_participation);
        if (it != bond_rxn_participation_to_reaction_center.end())
            bond.reaction_center = it->second;
    };

    auto topology_lambda = [&bond](const std::string& data) {
        uint8_t topology = kBondTopologyNameToInt.at(data);
        bond.topology = cdx_topology_to_topology.at(topology);
    };

    std::unordered_map<std::string, std::function<void(const std::string&)>> bond_dispatcher = {{"id", id_lambda},
                                                                                                {"B", bond_begin_lambda},
                                                                                                {"E", bond_end_lambda},
                                                                                                {"Order", bond_order_lambda},
                                                                                                {"Display", bond_display_lambda},
                                                                                                {"Display2", bond_display2_lambda},
                                                                                                {"BS", stereo_lambda},
                                                                                                {"RxnParticipation", reaction_center_lambda},
                                                                                                {"Topology", topology_lambda}};

    applyDispatcher(prop, bond_dispatcher);
}

void MoleculeCdxmlLoader::parsePos(const std::string& data, Vec3f& pos)
{
    std::vector<std::string> coords = split(data, ' ');
    if (coords.size() >= 2)
    {
        pos.x = std::stof(coords[0]);
        pos.y = std::stof(coords[1]);
        if (coords.size() > 2)
        {
            pos.z = std::stof(coords[2]);
            pos.z /= -SCALE;
        }
        if (this->_has_bounding_box)
        {
            pos.x -= this->cdxml_bbox.left();
            pos.y -= this->cdxml_bbox.bottom();
        }
        pos.x /= SCALE;
        pos.y /= -SCALE;
    }
    else
        throw Error("Not enought coordinates");
}

void MoleculeCdxmlLoader::parseBBox(const std::string& data, Rect2f& bbox)
{
    Vec2f v1, v2;
    parseSeg(data, v1, v2);
    bbox = Rect2f(v1, v2);
}

void MoleculeCdxmlLoader::parseSeg(const std::string& data, Vec2f& v1, Vec2f& v2)
{
    std::vector<std::string> coords = split(data, ' ');
    if (coords.size() == 4)
    {
        v1.set(std::stof(coords[0]), std::stof(coords[1]));
        v2.set(std::stof(coords[2]), std::stof(coords[3]));
        if (this->_has_bounding_box)
        {
            v1.sub(this->cdxml_bbox.leftBottom());
            v2.sub(this->cdxml_bbox.leftBottom());
        }
        v1.x /= SCALE;
        v2.x /= SCALE;
        v1.y /= -SCALE;
        v2.y /= -SCALE;
    }
    else
        throw Error("Not enought coordinates for text bounding box");
}

void MoleculeCdxmlLoader::_parseAltGroup(BaseCDXElement& elem)
{
    std::vector<AutoInt> r_labels;
    std::vector<std::unique_ptr<BaseCDXElement>> r_fragments;

    std::pair<Vec2f, Vec2f> bbox, text_frame, group_frame;
    auto bbox_lambda = [&bbox, this](const std::string& data) { this->parseSeg(data, bbox.first, bbox.second); };
    auto text_frame_lambda = [&text_frame, this](const std::string& data) { this->parseSeg(data, text_frame.first, text_frame.second); };
    auto group_frame_lambda = [&group_frame, this](const std::string& data) { this->parseSeg(data, group_frame.first, group_frame.second); };

    std::unordered_map<std::string, std::function<void(const std::string&)>> altgroup_dispatcher = {
        {"BoundingBox", bbox_lambda}, {"TextFrame", text_frame_lambda}, {"GroupFrame", group_frame_lambda}};

    applyDispatcher(*elem.firstProperty().get(), altgroup_dispatcher);

    for (auto r_elem = elem.firstChildElement(); r_elem->hasContent(); r_elem = r_elem->nextSiblingElement())
    {
        auto el_name = r_elem->name();
        if (el_name == "fragment")
            r_fragments.emplace_back(r_elem->copy());
        else if (el_name == "t")
        {
            std::string rl;
            _parseLabel(*r_elem, rl);
            if (rl.find("R") == 0)
                r_labels.push_back(rl.substr(1));
        }
    }

    if (r_labels.size())
    {
        // TODO: check if there are some fragments inside of group_frame_lambda and put them into r_fragments
        if (r_fragments.size())
        {
            MoleculeCdxmlLoader alt_loader(_scanner, _is_binary);
            BaseMolecule& mol = _pmol ? *(BaseMolecule*)_pmol : *(BaseMolecule*)_pqmol;
            std::unique_ptr<BaseMolecule> fragment(mol.neu());
            alt_loader.stereochemistry_options = stereochemistry_options;
            alt_loader.loadMoleculeFromFragment(*fragment, *r_fragments.front());
            MoleculeRGroups& rgroups = mol.rgroups;
            RGroup& rgroup = rgroups.getRGroup(r_labels.front());
            rgroup.fragments.add(fragment.release());
        }
    }
}

void MoleculeCdxmlLoader::_parseGraphic(BaseCDXElement& elem)
{
    AutoInt superseded_id = 0;
    auto superseded_lambda = [&superseded_id](const std::string& data) { superseded_id = data; };

    std::pair<Vec2f, Vec2f> graph_bbox;

    auto graphic_bbox_lambda = [&graph_bbox, this](const std::string& data) { this->parseSeg(data, graph_bbox.first, graph_bbox.second); };

    CDXGraphicType graphic_type = kCDXGraphicType_Undefined;
    auto graphic_type_lambda = [&graphic_type](const std::string& data) { graphic_type = kCDXPropGraphicTypeStrToID.at(data); };

    CDXSymbolType symbol_type = kCDXSymbolType_LonePair;
    auto symbol_type_lambda = [&symbol_type](const std::string& data) { symbol_type = kCDXPropSymbolTypeStrToID.at(data); };

    CDXArrowType arrow_type = kCDXArrowType_FullHead;
    auto arrow_type_lambda = [&arrow_type](const std::string& data) { arrow_type = kCDXProp_Arrow_TypeStrToID.at(data); };

    AutoInt head_size = 0;
    auto head_size_lambda = [&head_size](const std::string& data) { head_size = data; };

    std::unordered_map<std::string, std::function<void(const std::string&)>> graphic_dispatcher = {
        {"SupersededBy", superseded_lambda}, {"BoundingBox", graphic_bbox_lambda}, {"GraphicType", graphic_type_lambda},
        {"SymbolType", symbol_type_lambda},  {"ArrowType", arrow_type_lambda},     {"HeadSize", head_size_lambda}};

    applyDispatcher(*elem.firstProperty().get(), graphic_dispatcher);

    switch (graphic_type)
    {
    case kCDXGraphicType_Undefined:
        break;
    case kCDXGraphicType_Line: {
        if (arrow_type && superseded_id == 0)
        {
            auto head = graph_bbox.first;
            auto tail = graph_bbox.second;
            int ar_type = ReactionComponent::ARROW_BASIC;
            switch (arrow_type)
            {
            case kCDXArrowType_Resonance:
                ar_type = ReactionComponent::ARROW_BOTH_ENDS_FILLED_TRIANGLE;
                break;
            case kCDXArrowType_Equilibrium:
                ar_type = ReactionComponent::ARROW_EQUILIBRIUM_FILLED_HALF_BOW;
                break;
            default:
                break;
            }
            _graphic_arrows.push_back(std::make_pair(std::make_pair(Vec3f(tail.x, tail.y, 0), Vec3f(head.x, head.y, 0)), ar_type));
        }
    }
    break;
    case kCDXGraphicType_Oval:
    case kCDXGraphicType_Arc:
    case kCDXGraphicType_Rectangle:
        _primitives.push_back(std::make_pair(graph_bbox, graphic_type));
        break;
    case kCDXGraphicType_Orbital:
        break;
    case kCDXGraphicType_Bracket:
        break;
    case kCDXGraphicType_Symbol: {
        if (symbol_type == kCDXSymbolType_Plus)
        {
            Rect2f bbox(graph_bbox.first, graph_bbox.second);
            _pluses.emplace_back(bbox.center());
        }
    }
    break;
    default:
        break;
    }
}

void MoleculeCdxmlLoader::_parseArrow(BaseCDXElement& elem)
{
    Rect2f text_bbox;
    auto arrow_bbox_lambda = [&text_bbox, this](const std::string& data) { this->parseBBox(data, text_bbox); };
    Vec3f begin_pos;
    auto arrow_begin_lambda = [&begin_pos, this](const std::string& data) { this->parsePos(data, begin_pos); };
    Vec3f end_pos;
    auto arrow_end_lambda = [&end_pos, this](const std::string& data) { this->parsePos(data, end_pos); };
    std::string fill_type;
    auto fill_type_lambda = [&fill_type](const std::string& data) { fill_type = data; };
    std::string arrow_head;
    auto arrow_head_lambda = [&arrow_head](const std::string& data) { arrow_head = data; };
    std::string head_type;
    auto head_type_lambda = [&head_type](const std::string& data) { head_type = data; };
    std::unordered_map<std::string, std::function<void(const std::string&)>> arrow_dispatcher = {
        {"BoundingBox", arrow_bbox_lambda},  {"FillType", fill_type_lambda}, {"ArrowheadHead", arrow_head_lambda},
        {"ArrowheadType", head_type_lambda}, {"Head3D", arrow_end_lambda},   {"Tail3D", arrow_begin_lambda}};

    applyDispatcher(*elem.firstProperty().get(), arrow_dispatcher);
    _arrows.push_back(std::make_pair(std::make_pair(begin_pos, end_pos), 2));
}

void MoleculeCdxmlLoader::_parseLabel(BaseCDXElement& elem, std::string& label)
{
    label.clear();
    for (auto text_style = elem.firstChildElement(); text_style->hasContent(); text_style = text_style->nextSiblingElement())
    {
        std::string text_element = text_style->value();
        if (text_element == "s")
        {
            auto txt = text_style->getText();
            if (!is_valid_utf8(txt))
                txt = latin1_to_utf8(txt);
            label += txt;
        }
    }
}

void MoleculeCdxmlLoader::_parseText(BaseCDXElement& elem, std::vector<std::pair<Vec3f, std::string>>& text_parsed)
{
    Vec3f text_pos;
    auto text_coordinates_lambda = [&text_pos, this](const std::string& data) { this->parsePos(data, text_pos); };

    Rect2f text_bbox;
    auto text_bbox_lambda = [&text_bbox, this](const std::string& data) { this->parseBBox(data, text_bbox); };

    std::string label_justification, label_alignment;
    auto label_justification_lambda = [&label_justification, this](const std::string& data) { label_justification = data; };
    auto label_justification_alignment_lambda = [&label_alignment, this](const std::string& data) { label_alignment = data; };

    std::unordered_map<std::string, std::function<void(const std::string&)>> text_dispatcher = {{"p", text_coordinates_lambda},
                                                                                                {"xyz", text_coordinates_lambda},
                                                                                                {"BoundingBox", text_bbox_lambda},
                                                                                                {"LabelJustification", label_justification_lambda},
                                                                                                {"LabelAlignment", label_justification_alignment_lambda}};

    AutoInt font_id, font_color_id, font_face;
    float font_size;

    auto style_font_lambda = [&font_id](const std::string& data) { font_id = data; };
    auto style_size_lambda = [&font_size](const std::string& data) { font_size = round(std::stof(data) * kCDXMLFonsSizeMultiplier); };
    auto style_color_lambda = [&font_color_id](const std::string& data) { font_color_id = data; };
    auto style_face_lambda = [&font_face](const std::string& data) { font_face = data; };

    std::unordered_map<std::string, std::function<void(const std::string&)>> style_dispatcher = {
        {"font", style_font_lambda}, {"size", style_size_lambda}, {"color", style_color_lambda}, {"face", style_face_lambda}};

    applyDispatcher(*elem.firstProperty().get(), text_dispatcher);

    StringBuffer s;
    Writer<StringBuffer> writer(s);
    writer.StartObject();
    writer.Key("blocks");
    writer.StartArray();

    std::list<CdxmlKetTextLine> ket_text_lines;
    ket_text_lines.emplace_back();
    for (auto text_style = elem.firstChildElement(); text_style->hasContent(); text_style = text_style->nextSiblingElement())
    {
        std::string text_element = text_style->name();
        auto& ket_text_line = ket_text_lines.back();
        if (text_element == "s")
        {
            std::string label_part = text_style->getText();
            if (label_part == "+")
            {
                _pluses.push_back(text_bbox.center());
                return;
            }

            ket_text_line.text_styles.emplace_back();
            auto& ket_text_style = ket_text_line.text_styles.back();

            auto initial_size = label_part.size();
            label_part.erase(std::remove_if(label_part.begin(), label_part.end(), [](auto ch) { return (ch == '\n' || ch == '\r'); }), label_part.end());
            if (initial_size > label_part.size()) // line break
                ket_text_lines.emplace_back();

            ket_text_style.offset = ket_text_line.text.size();
            ket_text_style.size = label_part.size();
            ket_text_line.text += label_part;

            font_face = 0;
            font_size = 0.0;
            applyDispatcher(*text_style->firstProperty().get(), style_dispatcher);

            CDXMLFontStyle fs(font_face);
            if (font_face == KCDXMLChemicalFontStyle)
            {
                // special case
            }
            else
            {
                if (fs.is_bold)
                    ket_text_style.styles.push_back(KETFontBoldStr);
                if (fs.is_italic)
                    ket_text_style.styles.push_back(KETFontItalicStr);
                if (fs.is_superscript)
                    ket_text_style.styles.push_back(KETFontSuperscriptStr);
                if (fs.is_subscript)
                    ket_text_style.styles.push_back(KETFontSubscriptStr);
            }
            if (font_size > 0 && (int)font_size != KETDefaultFontSize)
                ket_text_style.styles.push_back(std::string(KETFontCustomSizeStr) + "_" + std::to_string((int)ceil(font_size)) + "px");
        }
    }

    for (const auto& ket_text_line : ket_text_lines)
    {
        writer.StartObject();
        writer.Key("text");
        writer.String(ket_text_line.text.c_str());
        writer.Key("inlineStyleRanges");
        writer.StartArray();
        for (const auto& ts : ket_text_line.text_styles)
        {
            for (const auto& style_str : ts.styles)
            {
                writer.StartObject();
                writer.Key("offset");
                writer.Int(static_cast<int>(ts.offset));
                writer.Key("length");
                writer.Int(static_cast<int>(ts.size));
                writer.Key("style");
                writer.String(style_str.c_str());
                writer.EndObject();
            }
        }
        writer.EndArray();
        writer.Key("entityRanges");
        writer.StartArray();
        writer.EndArray();
        writer.Key("data");
        writer.StartObject();
        writer.EndObject();
        writer.EndObject();
    }

    writer.EndArray();
    writer.Key("entityMap");
    writer.StartObject();
    writer.EndObject();

    writer.EndObject();

    Vec3f tpos(text_pos);
    if (text_bbox.width() > 0 && text_bbox.height() > 0)
        tpos.set(text_bbox.center().x, text_bbox.center().y, 0);

    std::string txt = s.GetString();
    if (!is_valid_utf8(txt))
        txt = latin1_to_utf8(txt);

    text_parsed.emplace_back(tpos, txt.c_str());
}

void MoleculeCdxmlLoader::_parseBracket(CdxmlBracket& bracket, BaseCDXProperty& prop)
{
    auto bracketed_ids_lambda = [&bracket](const std::string& data) {
        std::vector<std::string> vec_str = split(data, ' ');
        bracket.bracketed_list.assign(vec_str.begin(), vec_str.end());
    };
    auto bracket_usage_lambda = [&bracket](const std::string& data) { bracket.usage = kBracketUsageNameToInt.at(data); };

    auto repeat_count_lambda = [&bracket](const std::string& data) { bracket.repeat_count = data; };
    auto repeat_pattern_lambda = [&bracket](const std::string& data) {
        static const std::unordered_map<std::string, int> rep_map = {
            {"HeadToTail", RepeatingUnit::HEAD_TO_TAIL}, {"HeadToHead", RepeatingUnit::HEAD_TO_HEAD}, {"EitherUnknown", RepeatingUnit::EITHER}};
        bracket.repeat_pattern = rep_map.at(data);
    };

    auto sru_label_lambda = [&bracket](const std::string& data) { bracket.label = data; };

    std::unordered_map<std::string, std::function<void(const std::string&)>> bracket_dispatcher = {{"BracketedObjectIDs", bracketed_ids_lambda},
                                                                                                   {"BracketUsage", bracket_usage_lambda},
                                                                                                   {"RepeatCount", repeat_count_lambda},
                                                                                                   {"PolymerRepeatPattern", repeat_pattern_lambda},
                                                                                                   {"SRULabel", sru_label_lambda}};

    applyDispatcher(prop, bracket_dispatcher);
}

void MoleculeCdxmlLoader::_appendQueryAtom(const char* atom_label, std::unique_ptr<QueryMolecule::Atom>& atom)
{
    int atom_number = Element::fromString2(atom_label);
    std::unique_ptr<QueryMolecule::Atom> cur_atom;
    if (atom_number != -1)
        cur_atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_NUMBER, atom_number);
    else
        cur_atom = std::make_unique<QueryMolecule::Atom>(QueryMolecule::ATOM_PSEUDO, atom_label);

    if (atom.get() == 0)
        atom.reset(cur_atom.release());
    else
        atom.reset(QueryMolecule::Atom::oder(atom.release(), cur_atom.release()));
}
