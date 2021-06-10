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

#ifndef __render_cdxml_h__
#define __render_cdxml_h__
#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"

namespace indigo
{

    class RenderParams;
    class Molecule;
    class Reaction;
    struct Vec2f;

    class RenderCdxmlContext
    {
    public:
        enum
        {
            ALIGNMENT_LEFT,
            ALIGNMENT_RIGHT
        };
        class PropertyData
        {
        public:
            Array<char> propertyName;
            Array<char> propertyValue;
            PropertyData(){};

        private:
            PropertyData(PropertyData&);
        };

        bool enabled;
        int keyAlignment;
        float propertyFontSize;
        Array<char> titleFont;
        Array<char> titleFace;
        Array<char> fonttable;
        Array<char> colortable;
        Array<char> propertyNameCaption;
        Array<char> propertyValueCaption;
        ObjArray<PropertyData> property_data;

        void clear()
        {
            enabled = false;
            keyAlignment = ALIGNMENT_LEFT;
            propertyFontSize = 12.0f;
            titleFont.clear();
            titleFace.clear();
            fonttable.clear();
            colortable.clear();
            propertyNameCaption.clear();
            propertyValueCaption.clear();
            property_data.clear();
        }

        RenderCdxmlContext() : enabled(false)
        {
            clear();
        };

    private:
        RenderCdxmlContext(RenderCdxmlContext&);
    };

    class RenderParamCdxmlInterface
    {
    public:
        static void render(RenderParams& params);
        static void _renderMols(RenderParams& params);
        static void _renderRxns(RenderParams& params);
    };

} // namespace indigo

#endif // __render_cdxml_h__
