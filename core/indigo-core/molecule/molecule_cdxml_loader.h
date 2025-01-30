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
#include "common/utils/emf_utils.h"
#include "elements.h"
#include "molecule/base_molecule.h"
#include "molecule/meta_commons.h"
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
        CdxmlBond() : order(1), stereo(0), dir(0), display(0), display2(0), topology(0), reaction_center(0), swap_bond(false)
        {
        }
        AutoInt id;
        std::pair<AutoInt, AutoInt> be;
        AutoInt order;
        AutoInt stereo;
        AutoInt dir;
        AutoInt display;
        AutoInt display2;
        AutoInt topology;
        AutoInt reaction_center;
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

    struct CdxmlText
    {
        CdxmlText(const Vec3f& pos, const Vec2f& size, const std::string& text) : pos(pos), size(size), text(text)
        {
        }
        std::string text;
        Vec3f pos;
        Vec2f size;
    };

    class BaseCDXProperty
    {
    public:
        virtual ~BaseCDXProperty() = default;
        virtual bool hasContent() const = 0;
        virtual std::unique_ptr<BaseCDXProperty> copy() = 0;
        virtual std::unique_ptr<BaseCDXProperty> next() = 0;
        virtual std::string name() const = 0;
        virtual std::string value() const = 0;
    };

    static constexpr uint32_t tag_size = sizeof(uint16_t);
    static constexpr uint32_t id_size = sizeof(uint32_t);

    class CDXMLProperty : public BaseCDXProperty
    {
    public:
        DECL_ERROR;

        CDXMLProperty(const tinyxml2::XMLAttribute* attribute) : _attribute(attribute){};

        virtual bool hasContent() const override
        {
            return _attribute != nullptr;
        }

        virtual std::unique_ptr<BaseCDXProperty> copy() override
        {
            return std::make_unique<CDXMLProperty>(_attribute);
        }

        virtual std::unique_ptr<BaseCDXProperty> next() override
        {
            return std::make_unique<CDXMLProperty>(attribute()->Next());
        }

        virtual std::string name() const override
        {
            return attribute()->Name();
        }

        virtual std::string value() const override
        {
            return attribute()->Value();
        }

    protected:
        const tinyxml2::XMLAttribute* attribute() const
        {
            if (_attribute == nullptr)
                throw Error("Null property");
            return _attribute;
        }

    private:
        const tinyxml2::XMLAttribute* _attribute;
    };

    class CDXElement;

    class CDXProperty : public BaseCDXProperty
    {
    public:
        DECL_ERROR;
        CDXProperty(CDXElement* parent) : CDXProperty(parent, 0, nullptr)
        {
        }

        CDXProperty(CDXElement* parent, uint16_t tag, const uint8_t* data, uint32_t size = 0);

        bool hasContent() const override
        {
            return _data != nullptr;
        }

        std::unique_ptr<BaseCDXProperty> copy() override
        {
            return std::make_unique<CDXProperty>(_parent, _tag, _data, _size);
        }

        std::unique_ptr<BaseCDXProperty> next() override
        {
            return nextProp();
        }

        std::unique_ptr<CDXProperty> nextProp();

        std::string name() const override
        {
            auto it = KCDXPropToName.find(_tag);
            return it == KCDXPropToName.end() ? std::string{} : it->second.first;
        }

        std::string value() const override
        {
            auto it = KCDXPropToName.find(_tag);
            if (it != KCDXPropToName.end())
            {
                if (_size == 0)
                    return "";
                auto prop_type = it->second.second;
                return formatValue(prop_type);
            }

            std::stringstream ss;
            std::vector<uint8_t> val_dump(_data, _data + _size);
            ss << "raw value:" << std::hex;
            for (auto val : val_dump)
                ss << std::setw(2) << std::setfill('0') << (int)val << " ";
            return ss.str();
        }

        uint16_t tag()
        {
            return _tag;
        }

        // protected:
        std::string formatValue(ECDXType cdx_type) const;
        std::string parseCDXUINT16(uint16_t val) const;
        std::string parseCDXINT16(int16_t val) const;
        std::string parseCDXINT32(int32_t val) const;
        std::string parseCDXINT8(int8_t val) const;

        CDXElement* _parent;
        uint16_t _tag;
        const uint8_t* _data;
        uint32_t _size;
    };

    class CDXIdProperty : public CDXProperty
    {
    public:
        CDXIdProperty(CDXElement* parent, const uint8_t* data) : CDXProperty(parent, 0, data, id_size){};

        std::unique_ptr<BaseCDXProperty> copy() override
        {
            return std::make_unique<CDXIdProperty>(_parent, _data);
        }

        std::string name() const override
        {
            return "id";
        }
        std::string value() const override
        {
            return formatValue(ECDXType::CDXObjectID);
        }
    };

    class CDXStyleProperty : public CDXProperty
    {
    public:
        CDXStyleProperty(CDXElement* parent, const uint8_t* data, uint8_t prop_index)
            : CDXProperty(parent, 0xffff, data, sizeof(uint16_t)), _prop_index(prop_index){};

        std::unique_ptr<BaseCDXProperty> copy() override
        {
            return std::make_unique<CDXStyleProperty>(_parent, _data, _prop_index);
        }

        std::unique_ptr<BaseCDXProperty> next()
        {
            uint8_t next_index = _prop_index + 1;
            if (next_index < KStyleProperties.size())
                return std::make_unique<CDXStyleProperty>(_parent, _data + sizeof(uint16_t), next_index);
            else
                return std::make_unique<CDXProperty>(_parent);
        }

        std::string name() const override
        {
            return KStyleProperties[_prop_index];
        }

        std::string value() const override
        {
            uint16_t style_prop = *reinterpret_cast<const uint16_t*>(_data);
            if (_prop_index == kCDXMLStyleSizeIndex)
                style_prop /= kCDXMLSizeMultiplier;
            return parseCDXUINT16(style_prop);
        }

    protected:
        uint8_t _prop_index;
    };

    class BaseCDXElement
    {
    public:
        virtual ~BaseCDXElement() = default;
        virtual bool hasContent() = 0;
        virtual std::unique_ptr<BaseCDXElement> copy() = 0;
        virtual std::unique_ptr<BaseCDXProperty> firstProperty() = 0;
        virtual std::unique_ptr<BaseCDXProperty> findProperty(const std::string& name) = 0;
        virtual std::unique_ptr<BaseCDXElement> firstChildElement() = 0;
        virtual std::unique_ptr<BaseCDXElement> nextSiblingElement() = 0;
        virtual std::string name() = 0;
        virtual std::string value() = 0;
        virtual std::string getText() = 0;
    };

    class CDXMLElement : public BaseCDXElement
    {
    public:
        DECL_ERROR;

        CDXMLElement(const tinyxml2::XMLElement* xml) : _xml(xml){};

        bool hasContent() override
        {
            return _xml != nullptr;
        }

        std::unique_ptr<BaseCDXElement> copy() override
        {
            return std::make_unique<CDXMLElement>(_xml);
        }

        std::unique_ptr<BaseCDXProperty> firstProperty() override
        {
            return std::make_unique<CDXMLProperty>(xml()->FirstAttribute());
        };

        std::unique_ptr<BaseCDXProperty> findProperty(const std::string& name) override
        {
            return std::make_unique<CDXMLProperty>(xml()->FindAttribute(name.c_str()));
        }

        std::unique_ptr<BaseCDXElement> firstChildElement() override
        {
            return std::make_unique<CDXMLElement>(xml()->FirstChildElement());
        }

        std::unique_ptr<BaseCDXElement> nextSiblingElement() override
        {
            return std::make_unique<CDXMLElement>(xml()->NextSiblingElement());
        }

        std::string name() override
        {
            return std::string(xml()->Name());
        }

        std::string value() override
        {
            return std::string(xml()->Value());
        }

        std::string getText() override
        {
            std::string result;
            auto ptext = xml()->GetText();
            if (ptext)
                result = ptext;
            return result;
        }

    protected:
        const tinyxml2::XMLElement* xml()
        {
            if (_xml == nullptr)
                throw Error("Null element");
            return _xml;
        }

    private:
        const tinyxml2::XMLElement* _xml;
    };

    class CDXElement : public BaseCDXElement
    {
    public:
        DECL_ERROR;
        CDXElement() : CDXElement(0, nullptr)
        {
        }

        CDXElement(uint16_t tag, const uint8_t* data, uint32_t size = 0) : _tag(tag), _data(data), _data_size(size)
        {
        }

        CDXElement(const void* data, size_t size = 0) : _data_size(static_cast<uint32_t>(size))
        {
            _data = get_uint16_t(static_cast<const uint8_t*>(data), _tag);
        }

        bool hasContent() override
        {
            return _data != nullptr;
        }

        std::unique_ptr<BaseCDXElement> copy() override
        {
            return std::make_unique<CDXElement>(_tag, _data, _data_size);
        }

        std::unique_ptr<BaseCDXProperty> firstProperty() override
        {
            return std::make_unique<CDXIdProperty>(this, _data);
        }

        static const uint8_t* get_property_size(const uint8_t* data, uint32_t& size)
        {
            size = *reinterpret_cast<const uint16_t*>(data);
            data += sizeof(uint16_t);
            if (0xFFFF == size)
            {
                size = *reinterpret_cast<const uint32_t*>(data);
                data += sizeof(uint32_t);
            }
            return data;
        }

        static const uint8_t* get_uint16_t(const uint8_t* data, uint16_t& tag)
        {
            tag = *reinterpret_cast<const uint16_t*>(data);
            return data + sizeof(uint16_t);
        }

        std::unique_ptr<CDXProperty> getProperty(const uint8_t* data)
        {
            uint16_t tag;
            const uint8_t* ptr = get_uint16_t(data, tag);
            while (tag >= kCDXTag_Object || tag == kCDXProp_Text) // skip child objects
            {
                if (tag == kCDXProp_Text)
                {
                    uint32_t size;
                    ptr = get_property_size(ptr, size);
                    ptr += size;
                }
                else
                    ptr = skipObject(ptr);
                ptr = get_uint16_t(ptr, tag);
            }
            if (tag == 0) // End of object - return empty property
                return std::make_unique<CDXProperty>(this);
            uint32_t size;
            ptr = get_property_size(ptr, size);
            return std::make_unique<CDXProperty>(this, tag, ptr, size);
        }

        std::unique_ptr<BaseCDXProperty> findProperty(const std::string& name) override
        {
            auto first_prop = firstProperty();
            if (first_prop->name() == name)
                return first_prop;
            auto it = KCDXNameToProp.find(name);
            if (it != KCDXNameToProp.end())
                return findProperty(it->second.first);
            throw Error("Property %s not found", name.c_str());
        }

        std::unique_ptr<CDXProperty> findProperty(uint16_t tag)
        {
            for (auto prop = getProperty(_data + id_size); prop->hasContent(); prop = prop->nextProp())
                if (prop->tag() == tag)
                    return prop;
            return std::make_unique<CDXProperty>(this);
        }

        std::unique_ptr<BaseCDXElement> firstChildElement() override
        {
            return getChild(_data + id_size); // _data pointed to object id, object content just after id
        }

        std::unique_ptr<BaseCDXElement> nextSiblingElement() override
        {
            return next();
        }

        std::unique_ptr<CDXElement> next()
        {
            return getChild(skipObject(_data)); // return first object after this
        }

        std::unique_ptr<CDXElement> getChild(const uint8_t* ptr);

        std::string name() override
        {
            if (_tag == 0)
                return "CDXML";
            auto it = KCDXObjToName.find(_tag);
            if (it != KCDXObjToName.end())
                return it->second;
            return std::string{};
        }

        std::string value() override
        {
            return name();
        }

        std::string getText() override
        {
            switch (_tag)
            {
            case kCDXObj_Text: {
                for (auto child = getChild(_data + id_size); child->hasContent(); child = child->next())
                    if (child->tag() == kCDXProp_Text)
                        return child->getText();
                auto text_prop = findProperty(kCDXProp_Text);
                if (text_prop->hasContent())
                    return text_prop->value();
            }
            default:
                return name();
                break;
            }
            return std::string{};
        }

        uint16_t tag()
        {
            return _tag;
        }

    protected:
        static const uint8_t* skipProperty(const uint8_t* ptr)
        {
            uint32_t size = 0;
            ptr = get_property_size(ptr, size); // skip size
            ptr += size;                        // skip content
            return ptr;                         // points to the next property or object
        }

        static const uint8_t* skipObject(const uint8_t* ptr)
        {
            ptr += id_size; // skip tag and id
            uint16_t tag;
            while (tag = *reinterpret_cast<const uint16_t*>(ptr))
            {
                if (tag < kCDXTag_Object)
                    ptr = skipProperty(ptr + tag_size);
                else
                    ptr = skipObject(ptr + tag_size);
            }
            return ptr + tag_size; // skip terminating zero tag
        }

        uint16_t _tag;
        const uint8_t* _data;
        uint32_t _data_size;
    };

    class CDXTextElement : public CDXElement
    {
    public:
        CDXTextElement(uint16_t tag, const uint8_t* data, uint32_t size, uint16_t style_index) : CDXElement(tag, data, size), _style_index(style_index)
        {
            _style_count = *reinterpret_cast<const uint16_t*>(_data);
            _text_start = reinterpret_cast<const char*>(data);
            _text_len = size;
            if (_style_count > 0)
            {
                uint32_t styles_size = _style_count * sizeof(CDXTextStyle);
                if (styles_size < size + sizeof(_style_count)) // Some CDXString contains no style and no style-count property
                {
                    _text_start += styles_size + sizeof(_style_count);
                    _text_len -= styles_size + sizeof(_style_count);
                }
                else
                    _style_count = 0;
            }
            else
            {
                _text_start += sizeof(_style_count);
                _text_len -= sizeof(_style_count);
            }
        };

        std::unique_ptr<BaseCDXElement> copy() override
        {
            return std::make_unique<CDXTextElement>(_tag, _data, _data_size, _style_index);
        }

        std::unique_ptr<BaseCDXProperty> firstProperty() override
        {
            if (style_count() > 0 && _style_index < style_count())
            {
                // offset = sizeof(style_count)+sizeof(previous styles)+sizeof(text_offset)
                size_t offset = sizeof(uint16_t) + _style_index * sizeof(CDXTextStyle) + sizeof(uint16_t);
                return std::make_unique<CDXStyleProperty>(this, _data + offset, 0);
            }
            return std::make_unique<CDXProperty>(this);
        }

        std::unique_ptr<BaseCDXElement> firstChildElement() override
        {
            return std::make_unique<CDXElement>(); // no child objects in
        }

        uint16_t style_count()
        {
            return _style_count;
        }

        std::unique_ptr<BaseCDXElement> nextSiblingElement() override
        {
            if (style_count() > _style_index + 1)
                return std::make_unique<CDXTextElement>(_tag, _data, _data_size, _style_index + 1);
            return getChild(_data + _data_size); // return first object after this
        }

        std::string name() override
        {
            return "s";
        }

        std::string getText() override
        {
            long text_len = _text_len;
            const char* text_start = _text_start;

            if (_style_count > 0)
            {
                const CDXTextStyle* ptext_styles = reinterpret_cast<const CDXTextStyle*>(get_uint16_t(_data, _style_count));
                text_start += ptext_styles[_style_index].offset;

                if ((_style_index + 1) < _style_count)
                    text_len = ptext_styles[_style_index + 1].offset - ptext_styles[_style_index].offset;
                else
                    text_len -= ptext_styles[_style_index].offset;
            }
            return std::string(text_start, text_len);
        }

    protected:
        int _style_index;
        uint16_t _style_count;
        const char* _text_start;
        uint32_t _text_len;
    };

    class CDXReader
    {
    public:
        CDXReader(Scanner& scanner);
        virtual std::unique_ptr<BaseCDXElement> rootElement()
        {
            return std::make_unique<CDXElement>(_buffer.data(), _buffer.size());
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
        std::unique_ptr<BaseCDXElement> rootElement() override
        {
            return std::make_unique<CDXMLElement>(_xml.RootElement());
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
        struct ImageDescriptor
        {
            ImageDescriptor(EmbeddedImageObject::ImageFormat iformat, Rect2f& rc, const std::string& raw_data) : image_format(iformat), bbox(rc), data(raw_data)
            {
            }
            EmbeddedImageObject::ImageFormat image_format;
            Rect2f bbox;
            std::string data;
        };
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
        void loadMoleculeFromFragment(BaseMolecule& mol, BaseCDXElement& elem);

        static void applyDispatcher(BaseCDXProperty& prop, const std::unordered_map<std::string, std::function<void(const std::string&)>>& dispatcher);
        void parseCDXMLAttributes(BaseCDXProperty& prop);
        void parseBBox(const std::string& data, Rect2f& bbox);
        void parsePos(const std::string& data, Vec3f& bbox);
        void parseSeg(const std::string& data, Vec2f& v1, Vec2f& v2);
        void parseHex(const std::string& hex, std::string& binary);

        StereocentersOptions stereochemistry_options;
        bool ignore_bad_valence;
        Rect2f cdxml_bbox;
        AutoInt cdxml_bond_length;
        std::vector<CdxmlNode> nodes;
        std::vector<CdxmlBond> bonds;
        std::vector<CdxmlBracket> brackets;
        std::vector<CdxmlText> text_objects;

        static const int SCALE = 30;

    protected:
        void _initMolecule(BaseMolecule& mol);
        void _parseCollections(BaseMolecule& mol);
        void _checkFragmentConnection(int node_id, int bond_id);

        void _parseNode(CdxmlNode& node, BaseCDXElement& elem);
        void _addNode(CdxmlNode& node);

        void _parseBond(CdxmlBond& bond, BaseCDXProperty& prop);

        void _parseBracket(CdxmlBracket& bracket, BaseCDXProperty& prop);
        void _parseText(BaseCDXElement& elem, std::vector<CdxmlText>& text_parsed);
        void _parseLabel(BaseCDXElement& elem, std::string& label);

        void _parseGraphic(BaseCDXElement& elem);
        void _parseArrow(BaseCDXElement& elem);
        void _parseAltGroup(BaseCDXElement& elem);
        void _parseEmbeddedObject(BaseCDXElement& elem);

        int _addBond(Molecule& mol, const CdxmlBond& bond, int begin, int end);
        void _addAtomsAndBonds(BaseMolecule& mol, const std::vector<int>& atoms, const std::vector<CdxmlBond>& new_bonds);
        void _addBracket(BaseMolecule& mol, const CdxmlBracket& bracket);
        void _handleSGroup(SGroup& sgroup, const std::unordered_set<int>& atoms, BaseMolecule& bmol);
        void _processEnhancedStereo(BaseMolecule& mol);

        void _parseCDXMLPage(BaseCDXElement& elem);
        void _parseCDXMLElements(BaseCDXElement& elem, bool no_siblings = false, bool inside_fragment_node = false);
        void _parseFragmentAttributes(BaseCDXProperty& prop);
        void _gunzip(Scanner& scanner, Array<char>& dataBuf);
        std::string _inflate(const char* data, size_t dataLength);

        void _appendQueryAtom(const char* atom_label, std::unique_ptr<QueryMolecule::Atom>& atom);
        void _updateConnection(const CdxmlNode& node, int atom_idx);

        Molecule* _pmol;
        QueryMolecule* _pqmol;
        std::unordered_map<int, int> _id_to_atom_idx;
        std::unordered_map<int, std::size_t> _id_to_node_index;
        std::unordered_map<int, std::size_t> _id_to_bond_index;
        std::vector<int> _fragment_nodes;
        std::vector<Vec2f> _pluses;
        std::vector<ImageDescriptor> _images;
        std::unordered_map<int, std::pair<std::pair<Vec3f, Vec3f>, int>> _arrows;
        std::vector<std::pair<std::pair<Vec3f, Vec3f>, int>> _graphic_arrows;
        std::unordered_set<int> _retro_arrows_graph_id;
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
