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

#include "render_item_hline.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "render_context.h"
#include "render_internal.h"
#include "render_item_factory.h"

using namespace indigo;

IMPL_ERROR(RenderItemHLine, "RenderItemHLine");

RenderItemHLine::RenderItemHLine(RenderItemFactory& factory) : RenderItemContainer(factory)
{
}

void RenderItemHLine::init()
{
    hSpace = _settings.layoutMarginHorizontal;
}

void RenderItemHLine::estimateSize()
{
    RenderItemContainer::estimateSize();
    size.set(0, 0);
    for (int i = 0; i < items.size(); ++i)
    {
        RenderItemBase& item = _factory.getItem(items[i]);
        size.y = std::max(size.y, 2.f * (fabsf(item.referenceY) + item.size.y / 2.f));
        size.x += (i > 0 ? hSpace : 0) + item.size.x;
    }
}

void RenderItemHLine::render(bool idle)
{
    _rc.translate(-origin.x, -origin.y);
    _rc.storeTransform();
    for (int i = 0; i < items.size(); ++i)
    {
        RenderItemBase& item = _factory.getItem(items[i]);
        _rc.storeTransform();
        _rc.translate(0, 0.5f * (size.y - item.size.y) + item.referenceY);
        item.render(idle);
        _rc.restoreTransform();
        _rc.removeStoredTransform();
        _rc.translate(item.size.x + hSpace, 0);
    }
    _rc.restoreTransform();
    _rc.removeStoredTransform();
}