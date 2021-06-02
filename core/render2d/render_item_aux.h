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

#include "render_item.h"

namespace indigo
{

    class RenderItemAuxiliary : public RenderItemBase
    {
    public:
        enum AUX_TYPE
        {
            AUX_COMMENT = 0,
            AUX_TITLE,
            AUX_RXN_PLUS,
            AUX_RXN_ARROW,
            AUX_RGROUP_LABEL,
            AUX_RGROUP_IFTHEN
        };

        RenderItemAuxiliary(RenderItemFactory& factory);
        virtual ~RenderItemAuxiliary();
        DECL_ERROR;

        virtual void estimateSize()
        {
            _renderIdle();
        }
        virtual void setObjScale(float scale)
        {
        }
        virtual void init()
        {
        }
        virtual void render(bool idle);
        virtual float getTotalBondLength()
        {
            return 0.0f;
        }
        virtual float getTotalClosestAtomDistance()
        {
            return 0.0f;
        }
        virtual int getBondCount()
        {
            return 0;
        }
        virtual int getAtomCount()
        {
            return 0;
        }

        AUX_TYPE type;
        ArrayChar text;
        BaseMolecule* mol;
        int rLabelIdx;
        float arrowLength;

    private:
        void _drawRGroupLabel(bool idle);
        void _drawRIfThen(bool idle);
        void _drawText(bool idle);
        void _drawPlus();
        void _drawArrow();
        void _renderIdle();
    };

} // namespace indigo

#endif //__render_item_aux_h__
