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

#ifndef __render_item_fragment_h__
#define __render_item_fragment_h__

#include "render_item.h"

namespace indigo
{

    class RenderItemFactory;

    class RenderItemFragment : public RenderItemBase
    {
    public:
        RenderItemFragment(RenderItemFactory& factory);
        ~RenderItemFragment() override;

        DECL_ERROR;

        void estimateSize() override;
        void setObjScale(float scale) override
        {
            _scaleFactor = scale;
        }
        void init() override;
        void render(bool idle) override;
        float getTotalBondLength() override;
        float getTotalClosestAtomDistance() override;
        int getBondCount() override;
        int getAtomCount() override;

        BaseMolecule* mol;
        bool isRFragment;
        Array<int>* aam;
        Array<int>* reactingCenters;
        Array<int>* inversionArray;
        Array<int>* exactChangeArray;
        int refAtom;
        Vec2f refAtomPos;

    private:
        float _scaleFactor;

        void _renderIdle();
    };

} // namespace indigo

#endif //__render_item_fragment_h__
