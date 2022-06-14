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

#ifndef __render_item_reaction_h__
#define __render_item_reaction_h__

#include "render_item_aux.h"
#include "render_item_fragment.h"
#include "render_item_hline.h"

namespace indigo
{

    class RenderItemReaction : public RenderItemContainer
    {
    public:
        RenderItemReaction(RenderItemFactory& factory);
        ~RenderItemReaction() override
        {
        }

        DECL_ERROR;

        void init() override;
        void estimateSize() override;
        void render(bool idle) override;

        void initWithMeta();
        void estimateSizeWithMeta();
        void renderWithMeta(bool idle);

        BaseReaction* rxn;
        float hSpace, catalystOffset;

    private:
        int _addFragment(int id);
        int _addPlus();
        int _reactantLine, _catalystLineUpper, _catalystLineLower, _productLine, _arrow, _meta;
        bool _splitCatalysts;
        float _arrowWidth;
    };

} // namespace indigo

#endif //__render_item_reaction_h__
