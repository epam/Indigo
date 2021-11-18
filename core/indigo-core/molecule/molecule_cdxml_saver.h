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
#include "math/algebra.h"

class TiXmlElement;
class TiXmlDocument;

namespace indigo
{

    class BaseMolecule;
    class Output;

    class DLLEXPORT MoleculeCdxmlSaver
    {
    public:
        explicit MoleculeCdxmlSaver(Output& output);

        ~MoleculeCdxmlSaver();

        void saveMolecule(BaseMolecule& mol);
        enum
        {
            BOND_LENGTH = 30
        };

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
        void saveMoleculeFragment(BaseMolecule& mol, const Vec2f& offset, double scale, int id, Array<int>& nodes_ids);
        void addText(const Vec2f& pos, const char* text);
        void addText(const Vec2f& pos, const char* text, const char* alignment);
        void addCustomText(const Vec2f& pos, const char* alignment, double line_height, const char* text);
        void addTitle(const Vec2f& pos, const char* text);
        void addGraphic(int id, const Vec2f& p1, const Vec2f& p2, PropertiesMap& attrs);
        void addCustomElement(int id, Array<char>& name, PropertiesMap& attrs);
        void startCurrentElement(int id, Array<char>& name, PropertiesMap& attrs);
        void endCurrentElement();
        void endPage();
        void endDocument();
        int getHydrogenCount(BaseMolecule& mol, int idx, int charge, int radical);

        double pageHeight() const;
        double textLineHeight() const;

        void addDefaultFontTable();
        void addDefaultColorTable();

        DECL_ERROR;

    protected:
        Output& _output;

        double _bond_length;
        int _pages_height;
        double _max_page_height;

        TiXmlElement* _root;
        TiXmlElement* _page;
        TiXmlElement* _current;
        TiXmlElement* _fonttable;
        TiXmlElement* _colortable;

        std::unique_ptr<TiXmlDocument> _doc;

    private:
        MoleculeCdxmlSaver(const MoleculeCdxmlSaver&); // no implicit copy
    };

} // namespace indigo

#endif
