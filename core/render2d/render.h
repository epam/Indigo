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

#ifndef __render_h__
#define __render_h__

#include "render_internal.h"
#include "render_item_factory.h"

#ifdef _WIN32
#pragma warning(push, 4)
#endif

namespace indigo
{

    class Render
    {
    public:
        Render(RenderContext& rc, RenderItemFactory& factory, const CanvasOptions& cnvOpt, int bondLength);
        virtual ~Render() = 0;

        DECL_ERROR;

    protected:
        float _getObjScale(int item);
        int _getMaxWidth();
        int _getMaxHeight();
        float _getScale(int w, int h);
        float _getMaxScale(int w, int h);
        virtual float _getScaleGivenSize(int w, int h) = 0;
        virtual int _getDefaultWidth(float s) = 0;
        virtual int _getDefaultHeight(float s) = 0;

        int minMarg;
        RenderContext& _rc;
        const RenderSettings& _settings;
        const CanvasOptions& _cnvOpt;
        const RenderOptions& _opt;
        RenderItemFactory& _factory;
        int _bondLength;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif //__render_h__
