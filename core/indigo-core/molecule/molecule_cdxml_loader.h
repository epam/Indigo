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
#include "molecule/CDXConstants.h"

const int KCDXMLChemicalFontStyle = 96;

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
        enum class EnhancedStereoType
        {
            UNSPECIFIED,
            NONE,
            ABSOLUTE,
            OR,
            AND
        };

        CdxmlNode() : element(ELEM_C), type(kCDXNodeType_Element), enchanced_stereo(EnhancedStereoType::UNSPECIFIED) // Carbon by default
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
        bool is_not_list;
        std::vector<AutoInt> element_list;
        std::unordered_map<int, int> bond_id_to_connection_idx;
        std::unordered_map<int, int> node_id_to_connection_idx;
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

    inline std::vector<std::string> split(const std::string& str, char delim)
    {
        std::vector<std::string> strings;
        size_t start;
        size_t end = 0;
        while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
        {
            end = str.find(delim, start);
            strings.push_back(str.substr(start, end - start));
        }
        return strings;
    }

    class CDXProperty
    {
    public:
        DECL_ERROR;
        CDXProperty(const void* data, int size = 0) : _data(data), _size(size)
        {
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
            return _size ? CDXProperty(0, 0) : CDXProperty(attribute().Next());
        }

        std::string name()
        {
            return _size ? std::string() : std::string( attribute().Name() );
        }

        std::string value()
        {
            return _size ? std::string() : std::string(attribute().Value());
        }

        bool hasContent()
        {
            return _data;
        }

    protected:
        const void* _data;
        int _size;
    };

    class CDXElement
    {
    public:
        DECL_ERROR;
        CDXElement(const void* data, int size = 0) : _data(data), _size(size)
        {
        }

        CDXProperty firstProperty()
        {
            return _size ? CDXProperty(0, 0) : CDXProperty(xml().FirstAttribute());
        }

        CDXProperty findProperty( const std::string name )
        {
            return _size ? CDXProperty(0, 0) : CDXProperty(xml().FindAttribute(name.c_str()));
        }

        CDXElement firstChildElement()
        {
            return _size ? CDXElement(0, 0) : CDXElement(xml().FirstChildElement());
        }

        CDXElement nextSiblingElement()
        {
            return _size ? CDXElement(0, 0) : CDXElement(xml().NextSiblingElement());
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
            return _size ? std::string() : std::string(xml().Name());
        }

        std::string value()
        {
            return _size ? std::string() : std::string(xml().Value());
        }

        std::string getText()
        {
            return _size ? std::string() : std::string(xml().GetText());
        }

    protected:
        const void* _data;
        int _size;
    };

    class CDXReader
    {
    public:
        CDXReader(Scanner& scanner);
        virtual CDXElement rootElement()
        {
            return CDXElement(_buffer.data(), _buffer.size());
        }

        virtual void process()
        {
        }

        Scanner& scanner()
        {
            return _scanner;
        }

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

        MoleculeCdxmlLoader(Scanner& scanner);

        void loadMolecule(BaseMolecule& mol);
        void loadMoleculeFromFragment(BaseMolecule& mol, CDXElement elem);

        static void applyDispatcher(CDXProperty prop,
                                    const std::unordered_map<std::string, std::function<void(const std::string&)>>& dispatcher);
        void parseCDXMLAttributes(CDXProperty prop);
        void parseBBox(const std::string& data, Rect2f& bbox);
        void parsePos(const std::string& data, Vec3f& bbox);

        StereocentersOptions stereochemistry_options;
        bool ignore_bad_valence;
        bool _has_bounding_box;
        Rect2f cdxml_bbox;
        AutoInt cdxml_bond_length;
        std::vector<CdxmlNode> nodes;
        std::vector<CdxmlBond> bonds;
        std::vector<CdxmlBracket> brackets;
        std::vector<std::pair<Vec3f, std::string>> text_objects;

        static const int SCALE = 30;

    protected:
        const tinyxml2::XMLNode* _fragment;
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

        void _addAtomsAndBonds(BaseMolecule& mol, const std::vector<int>& atoms, const std::vector<CdxmlBond>& bonds);
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
        std::unordered_map<int, int> _id_to_node_index;
        std::unordered_map<int, int> _id_to_bond_index;
        std::vector<int> _fragment_nodes;
        std::vector<Vec2f> _pluses;
        std::vector<std::pair<std::pair<Vec3f, Vec3f>, int>> _arrows;
        float _bond_length;
        std::vector<EnhancedStereoCenter> _stereo_centers;
        CDXMLReader _cdx_reader;

    private:
        MoleculeCdxmlLoader(const MoleculeCdxmlLoader&); // no implicit copy
    };

} // namespace indigo

#endif
