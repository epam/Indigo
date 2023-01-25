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

#ifndef __molecule_cdxml_saver_h__
#define __molecule_cdxml_saver_h__

#include <memory>

#include "base_cpp/properties_map.h"
#include "ket_commons.h"
#include "math/algebra.h"

typedef unsigned short int UINT16;
typedef int INT32;
typedef unsigned int UINT32;
#include "CDXCommons.h"

namespace tinyxml2
{
    class XMLElement;
    class XMLAttribute;
    class XMLDocument;
}

namespace indigo
{

    class BaseMolecule;
    class Output;

    class DLLEXPORT MoleculeCdxmlSaver
    {
        struct OutConnection
        {
            OutConnection(int uid, int b, int e) : id(uid), beg(b), end(e)
            {
            }
            int id;
            int beg;
            int end;
        };

    public:
        explicit MoleculeCdxmlSaver(Output& output, bool is_binary = false);

        ~MoleculeCdxmlSaver();

        void saveMolecule(BaseMolecule& mol);
        void addNodeToFragment(BaseMolecule& mol, tinyxml2::XMLElement* fragment, int atom_idx, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord,
                               Vec2f& node_pos);

        void addBondToFragment(BaseMolecule& mol, tinyxml2::XMLElement* fragment, int bond_idx);

        void addNodesToFragment(BaseMolecule& mol, tinyxml2::XMLElement* fragment, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord);
        void addFragmentNodes(BaseMolecule& mol, tinyxml2::XMLElement* fragment, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord);
        void addBondsToFragment(BaseMolecule& mol, tinyxml2::XMLElement* fragment);

        static const int SCALE = 30;
        static const int MAX_PAGE_HEIGHT = 64;
        const float PLUS_HALF_HEIGHT = 7.5 / 2;

        struct Bounds
        {
            Vec2f min, max;
        };

        void beginDocument(Bounds* bounds);
        void beginPage(Bounds* bounds);
        void addFontTable(const char* font);
        void addFontToTable(int id, const char* charset, const char* name);
        void addColorTable(const char* color);
        void addColorToTable(int id, int r, int g, int b);
        void saveMoleculeFragment(BaseMolecule& mol, const Vec2f& offset, float scale, int frag_id, int& id, std::vector<int>& ids);
        void saveMoleculeFragment(BaseMolecule& mol, const Vec2f& offset, float scale);

        void addMetaObject(const MetaObject& obj, int id);
        void addArrow(int id, int arrow_type, const Vec2f& beg, const Vec2f& end);

        void addText(const Vec2f& pos, const char* text);
        void addText(const Vec2f& pos, const char* text, const char* alignment);
        void addCustomText(const Vec2f& pos, const char* alignment, float line_height, const char* text);
        void addTitle(const Vec2f& pos, const char* text);
        void addElement(const char* element, int id, const Vec2f& p1, const Vec2f& p2, PropertiesMap& attrs);
        void addCustomElement(int id, Array<char>& name, PropertiesMap& attrs);
        void startCurrentElement(int id, Array<char>& name, PropertiesMap& attrs);
        void endCurrentElement();
        void endPage();
        void endDocument();
        void writeBinaryElement(tinyxml2::XMLElement* element);
        void writeBinaryAttributes(tinyxml2::XMLElement* pElement);
        void writeIrregularElement(tinyxml2::XMLElement* pElement, int16_t tag);

        void writeBinaryValue(const tinyxml2::XMLAttribute* pAttr, int16_t tag, ECDXType cdx_type);
        void writeBinaryTextValue(const tinyxml2::XMLElement* pTextElement);

        int getHydrogenCount(BaseMolecule& mol, int idx, int charge, int radical);

        float pageHeight() const;
        float textLineHeight() const;

        void addDefaultFontTable();
        void addDefaultColorTable();
        int getId();

        DECL_ERROR;

    protected:
        void _collectSuperatoms(BaseMolecule& mol);
        Output& _output;

        float _bond_length;
        int _pages_height;
        float _max_page_height;

        tinyxml2::XMLElement* _root;
        tinyxml2::XMLElement* _page;
        tinyxml2::XMLElement* _current;
        tinyxml2::XMLElement* _fonttable;
        tinyxml2::XMLElement* _colortable;

        std::unique_ptr<tinyxml2::XMLDocument> _doc;

    private:
        MoleculeCdxmlSaver(const MoleculeCdxmlSaver&); // no implicit copy
        std::unordered_set<int> _atoms_excluded;
        std::unordered_set<int> _bonds_excluded;
        std::unordered_set<int> _bonds_included;
        std::vector<OutConnection> _out_connections;

        std::vector<int> _atoms_ids;
        std::vector<int> _bonds_ids;
        std::map<int, std::vector<int>> _super_atoms;

        int _id;
        float _scale;
        bool _is_binary;
    };

} // namespace indigo

#endif
