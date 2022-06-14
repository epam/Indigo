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

#ifndef __render_item_molecule_h__
#define __render_item_molecule_h__

#include "render_item_aux.h"
#include "render_item_fragment.h"
#include "render_item_hline.h"

namespace indigo
{

    class RenderItemMolecule : public RenderItemContainer
    {
    public:
        RenderItemMolecule(RenderItemFactory& factory);
        ~RenderItemMolecule() override
        {
        }

        DECL_ERROR;

        void init() override;
        void estimateSize() override;
        void render(bool idle) override;
        void estimateSizeWithMeta();
        void initWithMetaData();
        void renderWithMeta(bool idle);

        BaseMolecule* mol;
        int refAtom;
        Vec2f refAtomPos;

    private:
        int _meta;
        int _core;
        int _mol;
        int _getRIfThenCount();
    };

} // namespace indigo

#endif //__render_item_molecule_h__
