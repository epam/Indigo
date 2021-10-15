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

#ifndef __render_grid_h__
#define __render_grid_h__

#include "render.h"

namespace indigo
{

    class RenderGrid : Render
    {
    public:
        RenderGrid(RenderContext& rc, RenderItemFactory& factory, const CanvasOptions& cnvOpt, int bondLength, bool bondLengthSet);
        ~RenderGrid() override;
        void draw();

        DECL_ERROR;

        Array<int> objs;
        Array<int> titles;
        Array<int> refAtoms;
        int titleOffset;
        int nColumns;
        int commentOffset;
        int comment;

    private:
        void _drawComment();

        int nRows;
        float scale;
        Vec2f maxsz;
        Vec2f cellsz;
        Vec2f outerMargin;
        Vec2f maxTitleSize;
        Vec2f clientArea;
        Vec2f commentSize;
        int _width, _height;

        float _getScaleGivenSize(int w, int h) override;
        int _getDefaultWidth(float s) override;
        int _getDefaultHeight(float s) override;
    };

} // namespace indigo

#endif //__render_grid_h__
