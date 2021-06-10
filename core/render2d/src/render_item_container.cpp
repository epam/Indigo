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

#include "render_item_container.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "render_context.h"
#include "render_internal.h"
#include "render_item_factory.h"

using namespace indigo;

IMPL_ERROR(RenderItemContainer, "RenderItemContainer");

RenderItemContainer::RenderItemContainer(RenderItemFactory& factory) : RenderItemBase(factory)
{
}

void RenderItemContainer::estimateSize()
{
    for (int i = 0; i < items.size(); ++i)
    {
        RenderItemBase& item = _factory.getItem(items[i]);
        item.estimateSize();
    }
}

void RenderItemContainer::setObjScale(float scale)
{
    for (int i = 0; i < items.size(); ++i)
    {
        RenderItemBase& item = _factory.getItem(items[i]);
        item.setObjScale(scale);
    }
}

float RenderItemContainer::getTotalBondLength()
{
    float sum = 0.0;
    for (int i = 0; i < items.size(); ++i)
    {
        RenderItemBase& item = _factory.getItem(items[i]);
        sum += item.getTotalBondLength();
    }
    return sum;
}

float RenderItemContainer::getTotalClosestAtomDistance()
{
    float sum = 0.0;
    for (int i = 0; i < items.size(); ++i)
    {
        RenderItemBase& item = _factory.getItem(items[i]);
        sum += item.getTotalClosestAtomDistance();
    }
    return sum;
}

int RenderItemContainer::getBondCount()
{
    int sum = 0;
    for (int i = 0; i < items.size(); ++i)
    {
        RenderItemBase& item = _factory.getItem(items[i]);
        sum += item.getBondCount();
    }
    return sum;
}

int RenderItemContainer::getAtomCount()
{
    int sum = 0;
    for (int i = 0; i < items.size(); ++i)
    {
        RenderItemBase& item = _factory.getItem(items[i]);
        sum += item.getAtomCount();
    }
    return sum;
}