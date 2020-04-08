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

#ifndef __render_item_hline_h__
#define __render_item_hline_h__

#include "render_item_container.h"

namespace indigo
{

    class RenderItemHLine : public RenderItemContainer
    {
    public:
        RenderItemHLine(RenderItemFactory& factory);
        virtual ~RenderItemHLine()
        {
        }

        DECL_ERROR;

        virtual void init();
        virtual void estimateSize();
        virtual void render(bool idle);

        float hSpace;
    };

} // namespace indigo

#endif //__render_item_hline_h__
