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

#ifndef __render_item_aux_h__
#define __render_item_aux_h__

#include "molecule/ket_commons.h"
#include "render_item.h"

namespace indigo
{

    class RenderItemAuxiliary : public RenderItemBase
    {
    public:
        enum AUX_TYPE
        {
            AUX_NOT_INITIALIZED = -1,
            AUX_COMMENT = 0,
            AUX_TITLE,
            AUX_RXN_PLUS,
            AUX_RXN_ARROW,
            AUX_RGROUP_LABEL,
            AUX_RGROUP_IFTHEN,
            AUX_META
        };

        RenderItemAuxiliary(RenderItemFactory& factory);
        ~RenderItemAuxiliary() override;
        DECL_ERROR;

        void init() override;

        void estimateSize() override
        {
            _renderIdle();
        }

        void setObjScale(float scale) override
        {
            scaleFactor = scale;
        }

        void scale(Vec2f& vec)
        {
            vec.x -= min.x;
            vec *= scaleFactor;
            vec.y = max.y - vec.y;
        }

        void fillKETStyle(TextItem& ti, const FONT_STYLE_SET& style_set);

        void render(bool idle) override;
        float getTotalBondLength() override
        {
            return 0.0f;
        }
        float getTotalClosestAtomDistance() override
        {
            return 0.0f;
        }
        int getBondCount() override
        {
            return 0;
        }
        int getAtomCount() override
        {
            return 0;
        }

        AUX_TYPE type;
        Array<char> text;
        BaseMolecule* mol;
        MetaDataStorage* meta;
        int rLabelIdx;
        float arrowLength;
        float scaleFactor;
        Vec2f offset;
        bool hasOffset;

    private:
        void _drawRGroupLabel(bool idle);
        void _drawRIfThen(bool idle);
        void _drawText(bool idle);
        void _drawText(TextItem& ti, bool idle);
        void _drawMeta(bool idle);
        void _drawPlus();
        void _drawArrow();
        void _drawArrow(const KETReactionArrow& ar);
        void _renderIdle();
        void _renderSimpleObject(const KETSimpleObject& simple);
        float _getMaxHeight(const KETTextObject::KETTextLine& tl);
    };

} // namespace indigo

#endif //__render_item_aux_h__
