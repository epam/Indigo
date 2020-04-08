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
        virtual ~RenderItemFragment();

        DECL_ERROR;

        virtual void estimateSize();
        virtual void setObjScale(float scale)
        {
            _scaleFactor = scale;
        }
        virtual void init();
        virtual void render(bool idle);
        virtual float getTotalBondLength();
        virtual float getTotalClosestAtomDistance();
        virtual int getBondCount();
        virtual int getAtomCount();

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
        Vec2f _min, _max;

        void _renderIdle();
    };

} // namespace indigo

#endif //__render_item_fragment_h__
