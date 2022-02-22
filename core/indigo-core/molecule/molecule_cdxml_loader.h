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

    struct _ExtConnection
    {
        int bond_id;
        int point_id;
        int atom_idx;
    };

    struct CdxmlNode
    {
        CdxmlNode() : element(ELEM_C), type(kCDXNodeType_Element) // Carbon by default
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
        AutoInt enchanced_stereo;
        AutoInt stereo_group;
        AutoInt index;
        bool is_not_list;
        std::vector<AutoInt> element_list;
        std::unordered_map<int, int> bond_id_to_connection_idx;
        std::unordered_map<int, int> node_id_to_connection_idx;
        std::vector<_ExtConnection> connections;
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
        CdxmlBracket() : repeat_pattern(RepeatingUnit::HEAD_TO_TAIL)
        {
        }
        std::vector<AutoInt> bracketed_list;
        int usage;
        AutoInt repeat_count;
        int repeat_pattern;
        std::string sru_label;
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

    class MoleculeCdxmlLoader
    {
    public:
        DECL_ERROR;

        MoleculeCdxmlLoader(Scanner& scanner);

        void loadMolecule(BaseMolecule& mol);

        StereocentersOptions stereochemistry_options;
        bool ignore_bad_valence;
        bool _has_bounding_box;
        Rect2f _cdxml_bbox;
        AutoInt _cdxml_bond_length;
        std::vector<CdxmlNode> _nodes;
        std::vector<CdxmlBond> _bonds;
        std::vector<CdxmlBracket> _brackets;

    protected:
        Scanner* _scanner;
        const tinyxml2::XMLNode* _fragment;
        void _parseCDXMLAttributes(const tinyxml2::XMLAttribute* pAttr);
        void _parseNode(CdxmlNode& node, tinyxml2::XMLElement* pElem);
        void _addNode(CdxmlNode& node);

        void _parseBond(CdxmlBond& bond, const tinyxml2::XMLAttribute* pAttr);
        void _addBond(CdxmlBond& node);

        void _parseBracket(CdxmlBracket& bracket, const tinyxml2::XMLAttribute* pAttr);

        void _applyDispatcher(const tinyxml2::XMLAttribute* pAttr, const std::unordered_map<std::string, std::function<void(std::string&)>>& dispatcher);
        void _addAtomsAndBonds(BaseMolecule& mol, const std::vector<int>& atoms, const std::vector<CdxmlBond>& bonds);
        void _addBracket(BaseMolecule& mol, const CdxmlBracket& bracket);
        void _handleSGroup(SGroup& sgroup, const std::unordered_set<int>& atoms, BaseMolecule& bmol);

        void _parseCDXMLPage(tinyxml2::XMLElement* pElem);
        void _parseCDXMLFragment(tinyxml2::XMLElement* pElem);
        void _parseFragmentAttributes(const tinyxml2::XMLAttribute* pAttr);

        void _appendQueryAtom(const char* atom_label, std::unique_ptr<QueryMolecule::Atom>& atom);
        void _updateConnection(const CdxmlNode& node, int atom_idx);

        Molecule* _pmol;
        QueryMolecule* _pqmol;
        std::unordered_map<int, int> _id_to_atom_idx;
        std::unordered_map<int, int> _id_to_node_index;
        std::unordered_map<int, int> _id_to_bond_index;
        std::vector<int> _fragment_nodes;

    private:
        MoleculeCdxmlLoader(const MoleculeCdxmlLoader&); // no implicit copy
    };

} // namespace indigo

#endif
