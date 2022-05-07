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

#ifndef __render_item_h__
#define __render_item_h__

#include "render_context.h"

namespace indigo
{

    class RenderItemFactory;

    class RenderItemBase
    {
    public:
        RenderItemBase(RenderItemFactory& factory);

        virtual ~RenderItemBase()
        {
        }

        DECL_ERROR;

        virtual void estimateSize() = 0;
        virtual void setObjScale(float scale) = 0;
        virtual void init() = 0;
        virtual void render(bool idle) = 0;
        virtual float getTotalBondLength() = 0;
        virtual float getTotalClosestAtomDistance() = 0;
        virtual int getBondCount() = 0;
        virtual int getAtomCount() = 0;

        Vec2f size;
        Vec2f origin;
        float referenceY;
        Vec2f min;
        Vec2f max;

    protected:
        RenderItemFactory& _factory;
        RenderContext& _rc;
        const RenderSettings& _settings;
        const RenderOptions& _opt;
    };

} // namespace indigo

#endif //__render_item_h__
