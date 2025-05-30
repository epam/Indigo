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

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <memory>

#include "base_cpp/properties_map.h"
#include "math/algebra.h"
#include "meta_commons.h"

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

        struct SuperatomDesc
        {
            SuperatomDesc(int uid) : id(uid)
            {
            }
            int id;
            std::vector<int> atoms;
            std::vector<int> bonds;
        };

    public:
        static std::string boundingBoxToString(const Rect2f& bbox);

        explicit MoleculeCdxmlSaver(Output& output, bool is_binary = false);

        ~MoleculeCdxmlSaver();

        void saveMolecule(BaseMolecule& bmol);
        void deleteNamelessSGroups(BaseMolecule& bmol);
        void addNodeToFragment(BaseMolecule& mol, tinyxml2::XMLElement* fragment, int atom_idx, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord,
                               Vec2f& node_pos);

        void addBondToFragment(BaseMolecule& mol, tinyxml2::XMLElement* fragment, int bond_idx);

        void addNodesToFragment(BaseMolecule& mol, tinyxml2::XMLElement* fragment, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord);
        void addFragmentNodes(BaseMolecule& mol, tinyxml2::XMLElement* fragment, const Vec2f& offset, Vec2f& min_coord, Vec2f& max_coord);
        void addBondsToFragment(BaseMolecule& mol, tinyxml2::XMLElement* fragment);

        static const int SCALE = 30;
        static const int MAX_PAGE_HEIGHT = 64;
        const float PLUS_HALF_HEIGHT = 7.5 / 2;
        const float RETRO_ARROW_DELTA_X = 0.5;

        struct Bounds
        {
            Vec2f min, max;
        };

        void beginDocument(Bounds* bounds);
        void beginPage(Bounds* bounds);
        void addFontTable(const char* font);
        void addFontToTable(int id, const char* charset, const char* name);
        void addColorTable(const char* color);
        void addColorToTable(int id, float r, float g, float b);
        void saveMoleculeFragment(BaseMolecule& bmol, const Vec2f& offset, float scale, int frag_id, int& id, std::map<int, int>& atom_ids);
        void saveMoleculeFragment(BaseMolecule& bmol, const Vec2f& offset, float scale);
        void saveRGroup(PtrPool<BaseMolecule>& fragments, const Vec2f& offset, int rgnum);

        void addMetaObject(const MetaObject& obj, int id, const Vec2f& offset);
        void addArrow(int id, int arrow_type, const Vec2f& beg, const Vec2f& end);
        void addRetrosynteticArrow(int graphic_obj_id, int arrow_id, const Vec2f& arrow_beg, const Vec2f& arrow_end);
        void addImage(int id, const EmbeddedImageObject& image);

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
        int _getAttachmentPoint(BaseMolecule& mol, int atom_idx);
        void _validate(BaseMolecule& bmol);

        tinyxml2::XMLElement* create_text(tinyxml2::XMLElement* parent, float x, float y, const char* label_justification);
        void add_style_str(tinyxml2::XMLElement* parent, int font, int size, int face, const char* str);
        void add_charge(tinyxml2::XMLElement* parent, int font, int size, int charge);

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
        std::unordered_map<int, int> _superatoms_atoms;
        std::unordered_map<int, int> _superatoms_bonds;
        std::list<OutConnection> _out_connections;

        std::map<int, int> _atoms_ids;
        std::map<int, int> _bonds_ids;
        std::map<int, SuperatomDesc> _superatoms;
        std::unordered_map<uint32_t, int> _color_table_map;
        std::vector<uint32_t> _color_table;

        int _id;
        float _scale;
        bool _is_binary;
    };

} // namespace indigo

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
