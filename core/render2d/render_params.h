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

#ifndef __render_params_h__
#define __render_params_h__

#include "render_common.h"
#include <memory>

namespace indigo
{

    class BaseMolecule;
    class Reaction;
    class Scanner;
    class Output;
    class RenderItemFactory;

    enum RENDER_MODE
    {
        RENDER_MOL,
        RENDER_RXN,
        RENDER_NONE
    };

    class RenderParams
    {
    public:
        RenderParams();
        ~RenderParams();

        void clear();
        void clearArrays();

        float relativeThickness;
        float bondLineWidthFactor;
        bool smart_layout = false;
        RENDER_MODE rmode;

        std::unique_ptr<BaseMolecule> mol;
        std::unique_ptr<BaseReaction> rxn;

        PtrArray<BaseMolecule> mols;
        PtrArray<BaseReaction> rxns;

        ObjArray<Array<char>> titles;
        Array<int> refAtoms;

        RenderOptions rOpt;
        CanvasOptions cnvOpt;
    };

    class RenderParamInterface
    {
    public:
        DECL_ERROR;
        static void render(RenderParams& params);
        static int multilineTextUnit(RenderItemFactory& factory, int type, const Array<char>& titleStr, const float spacing,
                                     const MultilineTextLayout::Alignment alignment);

    private:
        static void _prepareMolecule(RenderParams& params, BaseMolecule& bm);
        static void _prepareReaction(RenderParams& params, BaseReaction& rxn);
        static bool needsLayoutSub(BaseMolecule& mol);
        static bool needsLayout(BaseMolecule& mol);
        RenderParamInterface();
        RenderParamInterface(const RenderParamInterface&);
    };

} // namespace indigo

#endif //__render_params_h__
