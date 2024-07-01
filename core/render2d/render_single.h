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

#ifndef __render_single_h__
#define __render_single_h__

#include "render.h"

namespace indigo
{

    class RenderSingle : Render
    {
    public:
        RenderSingle(RenderContext& rc, RenderItemFactory& factory, const CanvasOptions& cnvOpt, int bondLength, bool bondLengthSet);
        ~RenderSingle() override;
        void draw();

        DECL_ERROR;

        int obj;
        int comment;
        float scale;
        int commentOffset;
        Vec2f objSize;
        Vec2f commentSize;
        Vec2f outerMargin;
        Vec2f objArea;
        int width, height;

    private:
        float _getScaleGivenSize(int w, int h);
        int _getDefaultWidth(float s);
        int _getDefaultHeight(float s);
        void _drawComment();
        void _drawObj();
    };

} // namespace indigo

#endif //__render_single_h__
