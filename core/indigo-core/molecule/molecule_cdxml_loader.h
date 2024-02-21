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

#ifndef __cdxml_loader__
#define __cdxml_loader__

#include <functional>
#include <regex>
#include <sstream>
#include <string>
#include <tinyxml2.h>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "elements.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule_stereocenter_options.h"
#include "molecule/query_molecule.h"

typedef unsigned short int UINT16;
typedef int INT32;
typedef unsigned int UINT32;
#include "CDXCommons.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4201)
#endif

namespace tinyxml2
{
    class XMLHandle;
    class XMLElement;
    class XMLNode;
    class XMLAttribute;
}

namespace indigo
{
    class Scanner;
    class Molecule;
    class QueryMolecule;

    class AutoInt
    {
    public:
        AutoInt() : val(0){};

        AutoInt(int v) : val(v){};

        AutoInt(const std::string& v) : val(std::stoi(v))
        {
        }

        operator int() const
        {
            return val;
        }

        operator std::string() const
        {
            return std::to_string(val);
        }

    private:
        int val;
    };

    union CDXMLFontStyle {
        CDXMLFontStyle(unsigned int val) : face(val)
        {
        }
        struct
        {
            unsigned int is_bold : 1;
            unsigned int is_italic : 1;
            unsigned int is_underline : 1;
            unsigned int is_outline : 1;
            unsigned int is_shadow : 1;
            unsigned int is_subscript : 1;
            unsigned int is_superscript : 1;
        };
        unsigned int face;
    };

    struct _ExtConnection
    {
        int bond_id;
        int point_id;
        int atom_idx;
    };

    struct CdxmlKetTextStyle
    {
        std::size_t offset;
        std::size_t size;
        std::list<std::string> styles;
    };

    struct CdxmlKetTextLine
    {
        std::string text;
        std::list<CdxmlKetTextStyle> text_styles;
    };

    struct CdxmlNode
    {
        CdxmlNode()
            : element(ELEM_C), type(kCDXNodeType_Element), enchanced_stereo(EnhancedStereoType::UNSPECIFIED), is_not_list(false),
              has_fragment(false) // Carbon by default
        {
        }

        AutoInt id;
        std::string label;
        AutoInt element;
        Vec3f pos;
        int type;
        AutoInt isotope;
        AutoInt charge;
        AutoInt radical;
        AutoInt valence;
        AutoInt hydrogens;
        AutoInt stereo;
        EnhancedStereoType enchanced_stereo;
        AutoInt enhanced_stereo_group;
        AutoInt index;
        AutoInt geometry;
        AutoInt alt_group_id;
        AutoInt rg_index;

        bool is_not_list;
        bool has_fragment;
        std::vector<AutoInt> element_list;
        std::unordered_map<int, std::size_t> bond_id_to_connection_idx;
        std::unordered_map<int, std::size_t> node_id_to_connection_idx;
        std::vector<_ExtConnection> connections;
        std::vector<int> ext_connections;
        std::vector<int> inner_nodes;
    };

    struct CdxmlBond
    {
        CdxmlBond() : order(1), stereo(0), dir(0), swap_bond(false)
        {
        }
        AutoInt id;
        std::pair<AutoInt, AutoInt> be;
        AutoInt order;
        AutoInt stereo;
        AutoInt dir;
        bool swap_bond;
    };

    struct CdxmlBracket
    {
        CdxmlBracket() : repeat_pattern(RepeatingUnit::HEAD_TO_TAIL), usage(kCDXBracketUsage_Generic), is_superatom(false)
        {
        }
        std::vector<AutoInt> bracketed_list;
        int usage;
        AutoInt repeat_count;
        int repeat_pattern;
        std::string label;
        bool is_superatom;
    };

    class CDXProperty
    {
    public:
        DECL_ERROR;

        static constexpr int tag_size = sizeof(uint16_t);
        static constexpr int id_size = sizeof(uint32_t);
        static constexpr int tag_id_size = tag_size + id_size;

        CDXProperty() : CDXProperty(nullptr)
        {
        }

        CDXProperty(const void* data, const void* data_limit = nullptr, uint32_t size = 0, bool is_object = false, int style_index = -1, int style_prop = -1)
            : _data(data), _data_limit(data_limit), _size(size), _is_object(is_object), _style_index(style_index), _style_prop(style_prop)
        {
        }

        static const uint8_t* get_size(const void* data, uint32_t& size)
        {
            const uint16_t* p16 = reinterpret_cast<const uint16_t*>(data);
            const uint8_t* p8 = reinterpret_cast<const uint8_t*>(data);
            size = *p16;
            p8 += sizeof(uint16_t);
            if (0xFFFF == size)
            {
                size = *reinterpret_cast<const uint32_t*>(p8);
                p8 += sizeof(uint32_t);
            }
            return p8;
        }

        static const uint8_t* get_tag_and_size(const void* data, uint16_t& tag, uint32_t& size)
        {
            const uint16_t* p16 = reinterpret_cast<const uint16_t*>(data);
            tag = *p16;
            return get_size(p16 + 1, size);
        }

        const tinyxml2::XMLAttribute& attribute()
        {
            if (_size)
                throw Error("Binary data");
            if (!_data)
                throw Error("Null property");
            auto res = (tinyxml2::XMLAttribute*)_data;
            return (tinyxml2::XMLAttribute&)*res;
        }

        CDXProperty next()
        {
            return _size || _is_object ? getNextProp() : CDXProperty(attribute().Next());
        }

        CDXProperty getNextProp();

        std::string name()
        {
            if (_is_object)
                return "id";

            if (_style_prop >= 0)
                return KStyleProperties[_style_prop];

            return _size ? getName() : std::string(attribute().Name());
        }

        std::string getName()
        {
            auto ptag = (uint16_t*)_data;
            auto it = KCDXPropToName.find(*ptag);
            return it == KCDXPropToName.end() ? std::string{} : it->second.first;
        }

        int tag()
        {
            auto ptag = (uint16_t*)_data;
            return *ptag;
        }

        std::string value()
        {
            if (_is_object)
                return formatValue(reinterpret_cast<const uint8_t*>(_data) + tag_size, id_size, 0, ECDXType::CDXObjectID);
            return _size ? getValue() : std::string(attribute().Value());
        }

        std::string getValue()
        {
            if (_style_prop >= 0 && _style_index >= 0)
            {
                auto tsp = (CDXTextStyleProperty*)_data;
                if (tsp->style_count)
                {
                    auto& style = tsp->styles[_style_index];
                    auto pstyle_prop = (uint16_t*)(&style.font_index + _style_prop);
                    uint16_t style_prop = *pstyle_prop;
                    if (_style_prop == kCDXMLStyleSizeIndex)
                        style_prop /= kCDXMLSizeMultiplier;
                    return formatValue((uint8_t*)&style_prop, sizeof(uint16_t), 0, ECDXType::CDXUINT16);
                }
                else
                    return std::string();
            }
            uint16_t tag = 0;
            uint32_t sz = 0;
            auto ptr = get_tag_and_size(_data, tag, sz);
            auto it = KCDXPropToName.find(tag);
            if (it != KCDXPropToName.end())
            {
                if (sz)
                {
                    auto prop_type = it->second.second;
                    return formatValue(ptr, sz, tag, prop_type);
                }
                else
                    return "";
            }

            std::stringstream ss;
            std::vector<uint8_t> val_dump(ptr, ptr + sz);
            ss << "raw value:" << std::hex;
            for (auto val : val_dump)
                ss << std::setw(2) << std::setfill('0') << (int)val << " ";
            return ss.str();
        }

        std::string formatValue(const uint8_t* ptr, uint16_t value_size, uint16_t tag, ECDXType cdx_type)
        {
            std::string result;
            switch (cdx_type)
            {
            case ECDXType::CDXPoint2D:
            case ECDXType::CDXRectangle: {
                auto ptr32 = (int32_t*)ptr;
                std::stringstream ss;
                ss << std::setprecision(2) << std::fixed;
                for (uint16_t i = 0; i < value_size / sizeof(int32_t); ++i)
                {
                    if (i)
                        ss << " ";
                    double val = ptr32[i ^ 1];
                    val = round(val * 100 / (1 << 16)) / 100;
                    ss << val;
                }
                result = ss.str();
            }
            break;

            case ECDXType::CDXPoint3D: {
                auto ptr32 = (int32_t*)ptr;
                std::stringstream ss;
                ss << std::setprecision(2) << std::fixed;
                for (uint16_t i = 0; i < value_size / sizeof(int32_t); ++i)
                {
                    if (i)
                        ss << " ";
                    double val = ptr32[i];
                    val = round(val * 100 / (1 << 16)) / 100;
                    ss << val;
                }
                result = ss.str();
            }
            break;

            case ECDXType::CDXCoordinate: {
                auto ptr32 = (int32_t*)ptr;
                std::stringstream ss;
                double val = *ptr32;
                val = round(val * 100 / (1 << 16)) / 100;
                ss << std::setprecision(2) << std::fixed << val;
                result = ss.str();
            }
            break;

            case ECDXType::CDXUINT16: {
                auto ptr16 = (uint16_t*)ptr;
                result = parseCDXUINT16(*ptr16, tag);
            }
            break;
            case ECDXType::CDXINT16: {
                int16_t val16 = value_size == sizeof(int8_t) ? *((int8_t*)ptr) : *((int16_t*)ptr); // ChemDraw 8.0 bug fix
                result = parseCDXINT16(val16, tag);
            }
            break;
            case ECDXType::CDXUINT8:
            case ECDXType::CDXINT8: {
                result = parseCDXINT8(*ptr, tag);
            }
            break;
            case ECDXType::CDXINT32: {
                auto ptr32 = (uint32_t*)ptr;
                result = parseCDXINT32(*ptr32, tag);
            }
            break;
            case ECDXType::CDXObjectID:
            case ECDXType::CDXUINT32: {
                auto ptr32 = (uint32_t*)ptr;
                result = std::to_string(*ptr32);
            }
            break;
            case ECDXType::CDXObjectIDArray: {
                auto ptr32 = (uint32_t*)ptr;
                for (uint16_t i = 0; i < value_size / sizeof(uint32_t); ++i)
                {
                    if (i)
                        result += " ";
                    result += std::to_string(ptr32[i]);
                }
            }
            break;
            case ECDXType::CDXString: {
                // get raw string.
                auto ptr16 = (uint16_t*)ptr;
                int offset = (*ptr16) * sizeof(CDXTextStyle) + sizeof(uint16_t);
                value_size -= offset;
                if (value_size > 0)
                {
                    return std::string((char*)(ptr + offset), value_size);
                }
                return std::string();
            }
            break;

            case ECDXType::CDXFLOAT64: {
                auto pflt = (double*)ptr;
                result = std::to_string(*pflt);
            }
            break;

            case ECDXType::CDXBooleanImplied: {
                result = "yes";
            }
            break;

            case ECDXType::CDXBoolean: {
                result = *ptr ? "yes" : "no";
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
                std::vector<uint8_t> val_dump(ptr, ptr + value_size);
                for (auto val : val_dump)
                    ss << std::setw(2) << std::setfill('0') << (int)val;
                return ss.str();
            }
            break;
            case ECDXType::CDXINT16ListWithCounts: {
                auto pcount = (int16_t*)ptr;
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

        std::string parseCDXUINT16(uint16_t val, uint16_t /*tag*/)
        {
            return std::to_string(val);
        }

        std::string parseCDXINT16(int16_t val, int16_t tag)
        {
            switch (tag)
            {
            case kCDXProp_Bond_Order: {
                return kBondOrderIntToStr.at(val);
            }
            case kCDXProp_Node_Type: {
                return KNodeTypeIntToName.at(val);
            }
            break;
            case kCDXProp_Bond_Display: {
                return kCDXProp_Bond_DisplayIdToStr.at((CDXBondDisplay)val);
            }
            break;
            case kCDXProp_BondSpacing: {
                val /= 10;
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
            default:
                break;
            }
            return std::to_string(val);
        }

        std::string parseCDXINT32(int32_t val, uint16_t tag)
        {
            switch (tag)
            {
            case kCDXProp_ChainAngle: {
                std::stringstream ss;
                ss << std::setprecision(2) << std::fixed << double(val) / (1 << 16);
                return ss.str();
            }
            default:
                break;
            }
            return std::to_string(val);
        }

        std::string parseCDXINT8(int8_t val, uint16_t tag)
        {
            switch (tag)
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
            default:
                break;
            }
            return std::to_string(val);
        }

        bool hasContent()
        {
            return _data || _is_object;
        }

    protected:
        const void* _data;
        const void* _data_limit;
        uint32_t _size;
        bool _is_object;
        int _style_index;
        int _style_prop;
    };

    class CDXElement
    {
    public:
        DECL_ERROR;
        CDXElement() : CDXElement(nullptr)
        {
        }

        CDXElement(const void* data, uint32_t size = 0, int style_index = -1) : _data(data), _size(size), _style_index(style_index)
        {
        }

        CDXProperty firstProperty()
        {
            return _size ? firstBinaryProperty() : CDXProperty(xml().FirstAttribute());
        }

        CDXProperty firstBinaryProperty()
        {
            if (_data && _size)
            {
                auto ptr = reinterpret_cast<const uint8_t*>(_data);
                auto ptr16 = reinterpret_cast<const uint16_t*>(_data);

                if (ptr16[0] >= kCDXTag_Object) // if object tag
                    return CDXProperty(_data, ptr + _size, _size, true, _style_index, _style_index < 0 ? -1 : 0);
                else // property tag
                {
                    uint32_t sz = 0;
                    const uint8_t* pdata = CDXProperty::get_size(ptr16 + 1, sz);
                    return CDXProperty(_data, ptr + _size, sz + static_cast<uint32_t>(pdata - ptr), false, _style_index,
                                       _style_index < 0 ? -1 : 0); // total chunk size = property size + tag + size
                }
            }
            return CDXProperty();
        }

        static const uint8_t* skipProperty(const uint8_t* ptr)
        {
            ptr += sizeof(uint16_t); // skip tag
            uint32_t size = 0;
            ptr = CDXProperty::get_size(ptr, size); // skip size
            ptr += size;                            // skip content
            return ptr;                             // points to the next property or object
        }

        static uint8_t* skipObject(uint8_t* ptr)
        {
            ptr += sizeof(uint16_t) + sizeof(uint32_t); // skip tag and id
            auto ptr16 = (uint16_t*)ptr;
            while (*ptr16)
            {
                if (*ptr16 < kCDXTag_Object)
                {
                    ptr16 = (uint16_t*)skipProperty((uint8_t*)ptr16);
                }
                else
                {
                    ptr16 = (uint16_t*)skipObject((uint8_t*)ptr16);
                }
            }
            return (uint8_t*)++ptr16; // skip terminating zero
        }

        CDXProperty findProperty(const std::string& name)
        {
            return _size ? findBinaryProperty(name) : CDXProperty(xml().FindAttribute(name.c_str()));
        }

        CDXProperty findBinaryProperty(const std::string& name)
        {
            auto prop = firstBinaryProperty();
            if (prop.name() == name)
                return prop;
            auto it = KCDXNameToProp.find(name);
            if (it != KCDXNameToProp.end())
                return findBinaryProperty(it->second.first);
            throw Error("Property %s not found", name.c_str());
            return CDXProperty();
        }

        CDXProperty findBinaryProperty(int16_t tag)
        {
            for (auto prop = firstBinaryProperty(); prop.hasContent(); prop = prop.getNextProp())
            {
                if (prop.tag() == tag)
                    return prop;
            }
            return CDXProperty();
        }

        CDXElement firstChildElement()
        {
            return _size ? firstChildBinaryElement() : CDXElement(xml().FirstChildElement());
        }

        CDXElement firstChildBinaryElement()
        {
            if (_data && _size)
            {
                auto ptr = (uint8_t*)_data;
                auto ptr16 = (uint16_t*)ptr;
                auto tag = *ptr16;
                if (tag >= kCDXTag_Object)
                    ptr16 = (uint16_t*)(ptr + sizeof(uint16_t) + sizeof(uint32_t)); // fall down into the cdx object

                while (*ptr16 && *ptr16 < kCDXTag_Object)
                {
                    if (*ptr16 == kCDXProp_Text)
                    {
                        ptr = (uint8_t*)ptr16;
                        auto sz = ptr16[1];
                        return CDXElement(ptr, sz + sizeof(uint16_t) * 2, 0); // simulated style object
                    }
                    ptr16 = (uint16_t*)skipProperty((uint8_t*)ptr16);
                }

                if (*ptr16)
                {
                    ptr = (uint8_t*)ptr16;
                    auto sz = skipObject(ptr) - ptr;
                    return CDXElement(ptr, static_cast<int>(sz));
                }
            }
            return CDXElement();
        }

        CDXElement nextSiblingElement()
        {
            return _size ? nextSiblingBinaryElement() : CDXElement(xml().NextSiblingElement());
        }

        CDXElement nextSiblingBinaryElement()
        {
            auto ptr = (uint8_t*)_data;
            auto ptr16 = (uint16_t*)ptr;
            if (*ptr16 == kCDXProp_Text)
            {
                auto tsp = (CDXTextStyleProperty*)(ptr16);
                if (tsp->style_count > _style_index + 1)
                    return CDXElement(_data, _size, _style_index + 1);
                else
                    return CDXElement();
            }

            ptr += _size;
            ptr16 = (uint16_t*)ptr;
            while (*ptr16 && *ptr16 < kCDXTag_Object)
            {
                ptr16 = (uint16_t*)skipProperty((uint8_t*)ptr16);
            }
            // ptr16 points to zero or object
            if (*ptr16)
            {
                ptr = (uint8_t*)ptr16;
                auto sz = skipObject(ptr) - ptr;
                return CDXElement(ptr, static_cast<int>(sz));
            }
            return CDXElement();
        }

        const tinyxml2::XMLElement& xml()
        {
            if (_size)
                throw Error("Binary data");
            if (!_data)
                throw Error("Null element");
            auto res = (tinyxml2::XMLElement*)_data;
            return (tinyxml2::XMLElement&)*res;
        }

        bool hasContent()
        {
            return _data;
        }

        std::string name()
        {
            return _size ? getBinaryName() : std::string(xml().Name());
        }

        std::string value()
        {
            return _size ? getBinaryValue() : std::string(xml().Value());
        }

        std::string getBinaryName() const
        {
            auto ptag = (uint16_t*)_data;
            if (*ptag < kCDXTag_Object && _style_index < 0)
                return "CDXML";
            switch (*ptag)
            {
            case kCDXProp_Text: // property tag as am object tag. special case for style object.
                return "s";
                break;
            default: {
                auto it = KCDXObjToName.find(*ptag);
                if (it != KCDXObjToName.end())
                    return it->second;
            }
            }
            return std::string{};
        }

        std::string getBinaryValue() const
        {
            return getBinaryName();
        }

        std::string getText()
        {
            std::string result;
            if (_size)
                result = getBinaryText();
            else
            {
                auto ptext = xml().GetText();
                if (ptext)
                    result = ptext;
            }
            return result;
        }

        std::string getBinaryText()
        {
            auto ptag = (uint16_t*)_data;
            switch (*ptag)
            {
            case kCDXObj_Text: {
                auto text_prop = findBinaryProperty(kCDXProp_Text);
                if (text_prop.hasContent())
                    return text_prop.getValue();
            }
            case kCDXProp_Text: {
                auto ptr = (char*)_data;
                ptr += sizeof(uint16_t); // skip tag
                uint32_t sz = 0;
                ptr = (char*)CDXProperty::get_size(ptr, sz); // skip size
                auto pstyles = (uint16_t*)ptr;
                ptr += sizeof(uint16_t) + *pstyles * sizeof(CDXTextStyle);
                auto ptext_style = (CDXTextStyle*)(pstyles + 1);
                ptr += ptext_style[_style_index].offset;
                if (static_cast<uint32_t>(ptr - (char*)_data) < _size)
                {
                    if ((_style_index + 1) < *pstyles)
                        return std::string(ptr, ptext_style[_style_index + 1].offset - ptext_style[_style_index].offset);
                    else
                        return std::string(ptr, _size - (ptr - (char*)_data));
                }
            }
            break;
            default:
                return getBinaryName();
                break;
            }
            return std::string{};
        }

    protected:
        const void* _data;
        uint32_t _size;
        int _style_index;
    };

    class CDXReader
    {
    public:
        CDXReader(Scanner& scanner);
        virtual CDXElement rootElement()
        {
            return CDXElement(_buffer.data(), static_cast<int>(_buffer.size()));
        }

        virtual void process()
        {
        }

        Scanner& scanner()
        {
            return _scanner;
        }

        virtual ~CDXReader(){};

    protected:
        std::string _buffer;
        Scanner& _scanner;
    };

    class CDXMLReader : public CDXReader
    {
    public:
        DECL_ERROR;

        CDXMLReader(Scanner& scanner) : CDXReader(scanner)
        {
        }

        void process() override
        {
            _xml.Parse(_buffer.c_str());
            if (_xml.Error())
                throw Error("XML parsing error: %s", _xml.ErrorStr());
        }
        CDXElement rootElement() override
        {
            return CDXElement{_xml.RootElement()};
        }

        ~CDXMLReader()
        {
        }

    private:
        tinyxml2::XMLDocument _xml;
    };

    class MoleculeCdxmlLoader
    {
    public:
        struct EnhancedStereoCenter
        {
            EnhancedStereoCenter(int atom, int type_id, int group_num) : atom_idx(atom), type(type_id), group(group_num)
            {
            }
            int atom_idx;
            int type;
            int group;
        };

        DECL_ERROR;

        MoleculeCdxmlLoader(Scanner& scanner, bool is_binary = false, bool is_fragment = false);

        void loadMolecule(BaseMolecule& mol, bool load_arrows = false);
        void loadMoleculeFromFragment(BaseMolecule& mol, CDXElement elem);

        static void applyDispatcher(CDXProperty prop, const std::unordered_map<std::string, std::function<void(const std::string&)>>& dispatcher);
        void parseCDXMLAttributes(CDXProperty prop);
        void parseBBox(const std::string& data, Rect2f& bbox);
        void parsePos(const std::string& data, Vec3f& bbox);
        void parseSeg(const std::string& data, Vec2f& v1, Vec2f& v2);

        StereocentersOptions stereochemistry_options;
        bool ignore_bad_valence;
        Rect2f cdxml_bbox;
        AutoInt cdxml_bond_length;
        std::vector<CdxmlNode> nodes;
        std::vector<CdxmlBond> bonds;
        std::vector<CdxmlBracket> brackets;
        std::vector<std::pair<Vec3f, std::string>> text_objects;

        static const int SCALE = 30;

    protected:
        void _initMolecule(BaseMolecule& mol);
        void _parseCollections(BaseMolecule& mol);
        void _checkFragmentConnection(int node_id, int bond_id);

        void _parseNode(CdxmlNode& node, CDXElement elem);
        void _addNode(CdxmlNode& node);

        void _parseBond(CdxmlBond& bond, CDXProperty prop);
        void _addBond(CdxmlBond& node);

        void _parseBracket(CdxmlBracket& bracket, CDXProperty prop);
        void _parseText(CDXElement elem, std::vector<std::pair<Vec3f, std::string>>& text_parsed);
        void _parseLabel(CDXElement elem, std::string& label);

        void _parseGraphic(CDXElement elem);
        void _parseArrow(CDXElement elem);
        void _parseAltGroup(CDXElement elem);

        void _addAtomsAndBonds(BaseMolecule& mol, const std::vector<int>& atoms, const std::vector<CdxmlBond>& new_bonds);
        void _addBracket(BaseMolecule& mol, const CdxmlBracket& bracket);
        void _handleSGroup(SGroup& sgroup, const std::unordered_set<int>& atoms, BaseMolecule& bmol);
        void _processEnhancedStereo(BaseMolecule& mol);

        void _parseCDXMLPage(CDXElement elem);
        void _parseCDXMLElements(CDXElement elem, bool no_siblings = false, bool inside_fragment_node = false);
        void _parseFragmentAttributes(CDXProperty prop);

        void _appendQueryAtom(const char* atom_label, std::unique_ptr<QueryMolecule::Atom>& atom);
        void _updateConnection(const CdxmlNode& node, int atom_idx);

        Molecule* _pmol;
        QueryMolecule* _pqmol;
        std::unordered_map<int, int> _id_to_atom_idx;
        std::unordered_map<int, std::size_t> _id_to_node_index;
        std::unordered_map<int, std::size_t> _id_to_bond_index;
        std::vector<int> _fragment_nodes;
        std::vector<Vec2f> _pluses;
        std::vector<std::pair<std::pair<Vec3f, Vec3f>, int>> _arrows;
        std::vector<std::pair<std::pair<Vec3f, Vec3f>, int>> _graphic_arrows;
        std::vector<std::pair<std::pair<Vec2f, Vec2f>, int>> _primitives;

        std::vector<EnhancedStereoCenter> _stereo_centers;
        Scanner& _scanner;
        bool _is_binary;
        bool _is_fragment;
        bool _has_bounding_box;
        bool _has_scheme;

    private:
        MoleculeCdxmlLoader(const MoleculeCdxmlLoader&); // no implicit copy
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
