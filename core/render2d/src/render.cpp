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

#include "render.h"

#include "base_cpp/array.h"
#include "base_cpp/output.h"
#include "math/algebra.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"

#include "render_context.h"
#include "render_item_factory.h"

using namespace indigo;

IMPL_ERROR(Render, "Render");

Render::Render(RenderContext& rc, RenderItemFactory& factory, const CanvasOptions& cnvOpt, int bondLength)
    : minMarg(2), _rc(rc), _settings(rc.getRenderSettings()), _cnvOpt(cnvOpt), _opt(rc.opt), _factory(factory), _bondLength(bondLength)
{
}

Render::~Render()
{
}

float Render::_getObjScale(int item)
{
    float avgBondLength = 1.0f;
    int bondCount = _factory.getItem(item).getBondCount();
    int atomCount = _factory.getItem(item).getAtomCount();
    if (bondCount)
    {
        avgBondLength = _factory.getItem(item).getTotalBondLength() / bondCount;
    }
    else if (atomCount)
    {
        avgBondLength = _factory.getItem(item).getTotalClosestAtomDistance() / atomCount;
    }
    if (avgBondLength < 1e-4)
    {
        avgBondLength = 1.0f;
    }
    float objScale = 1 / avgBondLength;
    return objScale;
}

int Render::_getMaxWidth()
{
    int maxPageSize = _rc.getMaxPageSize();
    return _cnvOpt.maxWidth > 0 ? std::min(_cnvOpt.maxWidth, maxPageSize) : maxPageSize;
}

int Render::_getMaxHeight()
{
    int maxPageSize = _rc.getMaxPageSize();
    return _cnvOpt.maxHeight > 0 ? std::min(_cnvOpt.maxHeight, maxPageSize) : maxPageSize;
}

float Render::_getScale(int w, int h)
{
    float scale = _getMaxScale(w, h);
    if (_bondLength > 0 && _bondLength < scale)
        return (float)_bondLength;
    return scale;
}

float Render::_getMaxScale(int w, int h)
{
    float s = (float)_bondLength > 0 ? _bondLength : LayoutOptions::DEFAULT_BOND_LENGTH_PX;
    int maxWidth = _getMaxWidth();
    int maxHeight = _getMaxHeight();
    int defaultWidth = _getDefaultWidth(s);
    int defaultHeight = _getDefaultHeight(s);
    if (h >= 1 && w >= 1)
        return _getScaleGivenSize(w, h);
    if (h >= 1)
        return _getScaleGivenSize(maxWidth, h);
    if (w >= 1)
        return _getScaleGivenSize(w, maxHeight);
    if (defaultWidth <= maxWidth && defaultHeight <= maxHeight)
        return s;
    return _getScaleGivenSize(std::min(defaultWidth, maxWidth), std::min(defaultHeight, maxHeight));
}