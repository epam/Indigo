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

#include "render_single.h"
#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/output.h"
#include "base_cpp/reusable_obj_array.h"
#include "layout/metalayout.h"
#include "math/algebra.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "render_context.h"
#include "render_item.h"
#include "render_item_factory.h"

using namespace indigo;

IMPL_ERROR(RenderSingle, "RenderSingle");

RenderSingle::RenderSingle(RenderContext& rc, RenderItemFactory& factory, const CanvasOptions& cnvOpt, int bondLength, bool bondLengthSet)
    : Render(rc, factory, cnvOpt, bondLength, bondLengthSet)
{
}

RenderSingle::~RenderSingle()
{
}

void RenderSingle::_drawObj()
{
    _rc.storeTransform();
    {
        _rc.translate((objArea.x - objSize.x * scale) / 2, (objArea.y - objSize.y * scale) / 2);
        _rc.scale(scale);
        _factory.getItem(obj).render(false);
    }
    _rc.restoreTransform();
    _rc.removeStoredTransform();
    _rc.translate(0, objArea.y);
}

void RenderSingle::_drawComment()
{
    if (comment < 0)
        return;
    _rc.storeTransform();
    {
        double diff = (double)(width - 2 * outerMargin.x - commentSize.x);
        _rc.translate(diff * _cnvOpt.commentAlign.getBboxRelativeOffset(), 0);
        _factory.getItem(comment).render(false);
    }
    _rc.restoreTransform();
    _rc.removeStoredTransform();
    _rc.translate(0, commentSize.y);
}

void RenderSingle::draw()
{
    width = _cnvOpt.width;
    height = _cnvOpt.height;
    _rc.fontsClear();

    _factory.getItem(obj).init();

    double objScale = _getObjScale(obj);
    _factory.getItem(obj).setObjScale(objScale);
    _factory.getItem(obj).estimateSize();

    objSize.copy(_factory.getItem(obj).size);

    commentSize.set(0, 0);
    commentOffset = 0;
    if (comment >= 0)
    {
        _factory.getItem(comment).init();
        _factory.getItem(comment).estimateSize();
        commentSize.copy(_factory.getItem(comment).size);
        commentOffset = _cnvOpt.commentOffset;
    }
    outerMargin.x = (double)(minMarg + _cnvOpt.marginX);
    outerMargin.y = (double)(minMarg + _cnvOpt.marginY);

    width = std::min(width, _getMaxWidth());
    height = std::min(height, _getMaxHeight());
    scale = _getScale(width, height);
    if (width < 1)
        width = _getDefaultWidth(scale);
    if (height < 1)
        height = _getDefaultHeight(scale);

    _rc.initContext(width, height);
    objArea.set((double)width, (double)height);
    objArea.addScaled(outerMargin, -2);
    objArea.y -= commentSize.y + commentOffset;
    _rc.init();
    _rc.translate((double)outerMargin.x, (double)outerMargin.y);
    if (_cnvOpt.xOffset > 0 || _cnvOpt.yOffset > 0)
        _rc.translate((double)_cnvOpt.xOffset, (double)_cnvOpt.yOffset);
    _rc.storeTransform();
    {
        if (_cnvOpt.commentPos == COMMENT_POS_TOP)
        {
            _drawComment();
            _rc.translate(0, (double)commentOffset);
            _drawObj();
        }
        else
        {
            _drawObj();
            _rc.translate(0, (double)commentOffset);
            _drawComment();
        }
    }
    _rc.resetTransform();
    _rc.removeStoredTransform();
}

int RenderSingle::_getDefaultWidth(const double s)
{
    return (int)ceil(std::max(std::max(objSize.x * s, commentSize.x) + outerMargin.x * 2, 1.0));
}

int RenderSingle::_getDefaultHeight(const double s)
{
    return (int)ceil(std::max(objSize.y * s + commentOffset + commentSize.y + outerMargin.y * 2, 1.0));
}

double RenderSingle::_getScaleGivenSize(int w, int h)
{
    double absX = 2 * outerMargin.x;
    double absY = commentSize.y + 2 * outerMargin.y + commentOffset;
    double x = w - absX, y = h - absY;
    if (x < commentSize.x + 1 || y < 1)
        throw Error("Image too small, the layout requires at least %dx%d", (int)(absX + commentSize.x + 2), (int)(absY + 2));
    if (x * objSize.y < y * objSize.x)
        return x / objSize.x;
    else
        return y / objSize.y;
}
