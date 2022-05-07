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

#include "render_item_fragment.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "render_context.h"
#include "render_internal.h"
#include "render_item_factory.h"

using namespace indigo;

IMPL_ERROR(RenderItemFragment, "RenderItemFragment");

RenderItemFragment::RenderItemFragment(RenderItemFactory& factory)
    : RenderItemBase(factory), mol(NULL), aam(NULL), reactingCenters(NULL), inversionArray(NULL), exactChangeArray(NULL), refAtom(-1), _scaleFactor(1.0f),
      isRFragment(false)
{
}

RenderItemFragment::~RenderItemFragment()
{
}

void RenderItemFragment::init()
{
    min.set(0, 0);
    max.set(0, 0);
    Rect2f bbox;
    mol->getBoundingBox(bbox);
    min.copy(bbox.leftBottom());
    max.copy(bbox.rightTop());
}

void RenderItemFragment::estimateSize()
{
    _renderIdle();
    if (refAtom >= 0)
    {
        const Vec3f& v = mol->getAtomXyz(refAtom);
        Vec2f v2(v.x, v.y);
        refAtomPos.set(v2.x - min.x, max.y - v2.y);
        refAtomPos.scale(_scaleFactor);
        refAtomPos.sub(origin);
    }
}

void RenderItemFragment::_renderIdle()
{
    _rc.initNullContext();
    Vec2f bbmin, bbmax;
    Vec2f pos;
    render(true);
    _rc.bbGetMin(bbmin);
    _rc.bbGetMax(bbmax);
    _rc.closeContext(true);
    size.diff(bbmax, bbmin);
    origin.copy(bbmin);
}

void RenderItemFragment::render(bool idle)
{
    _rc.translate(-origin.x, -origin.y);
    MoleculeRenderInternal rnd(_opt, _settings, _rc, idle);
    rnd.setMolecule(mol);
    rnd.setIsRFragment(isRFragment);
    rnd.setScaleFactor(_scaleFactor, min, max);
    rnd.setReactionComponentProperties(aam, reactingCenters, inversionArray);
    rnd.setQueryReactionComponentProperties(exactChangeArray);
    rnd.render();
}

static float get2dDist(const Vec3f& v1, const Vec3f& v2)
{
    return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

float RenderItemFragment::getTotalBondLength()
{
    float sum = 0.0;
    for (int i = mol->edgeBegin(); i < mol->edgeEnd(); i = mol->edgeNext(i))
    {
        const Edge& edge = mol->getEdge(i);
        sum += get2dDist(mol->getAtomXyz(edge.beg), mol->getAtomXyz(edge.end));
    }
    return sum;
}

float RenderItemFragment::getTotalClosestAtomDistance()
{
    if (mol->vertexCount() < 2)
        return 0;
    float sum = 0.0;
    for (int i = mol->vertexBegin(); i < mol->vertexEnd(); i = mol->vertexNext(i))
    {
        float minDist = -1;
        for (int j = mol->vertexBegin(); j < mol->vertexEnd(); j = mol->vertexNext(j))
        {
            if (i == j)
                continue;
            float dist = get2dDist(mol->getAtomXyz(i), mol->getAtomXyz(j));
            if (minDist < 0 || dist < minDist)
                minDist = dist;
        }
        sum += minDist;
    }
    return sum;
}

int RenderItemFragment::getBondCount()
{
    return mol->edgeCount();
}

int RenderItemFragment::getAtomCount()
{
    return mol->vertexCount();
}