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

#include "render_item_molecule.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "render_context.h"
#include "render_internal.h"
#include "render_item_factory.h"

using namespace indigo;

IMPL_ERROR(RenderItemMolecule, "RenderItemMolecule");

RenderItemMolecule::RenderItemMolecule(RenderItemFactory& factory) : RenderItemContainer(factory), mol(NULL), refAtom(-1), _core(-1), _meta(-1)
{
}

void RenderItemMolecule::initWithMetaData()
{
    _meta = _factory.addItemAuxiliary();
    auto& aux = _factory.getItemAuxiliary(_meta);
    aux.type = RenderItemAuxiliary::AUX_META;
    aux.meta = &mol->meta();
    aux.init();
    min = aux.min;
    max = aux.max;
    items.push(_meta);
    if (_getRIfThenCount() > 0)
    {
        int ifthen = _factory.addItemAuxiliary();
        _factory.getItemAuxiliary(ifthen).type = RenderItemAuxiliary::AUX_RGROUP_IFTHEN;
        _factory.getItemAuxiliary(ifthen).mol = mol;
        items.push(ifthen);
    }

    if (_core >= 0)
        items.push(_core);

    MoleculeRGroups& rGroups = mol->rgroups;
    for (int i = 1; i <= rGroups.getRGroupCount(); ++i)
    {
        RGroup& rg = rGroups.getRGroup(i);
        if (rg.fragments.size() == 0)
            continue;

        PtrPool<BaseMolecule>& frags = rg.fragments;
        Vec2f lb, rt;
        for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
        {
            int id = _factory.addItemFragment();
            auto& frag = _factory.getItemFragment(id);
            frag.mol = frags[j];
            frag.isRFragment = true;
            items.push(id);
            Rect2f fbbox;
            frag.mol->getBoundingBox(fbbox);
            if (j == frags.begin())
            {
                lb.copy(fbbox.leftBottom());
                rt.copy(fbbox.rightTop());
            }
            else
            {
                lb.min(fbbox.leftBottom());
                rt.max(fbbox.rightTop());
            }
        }
        // coordinates?
        int label = _factory.addItemAuxiliary();
        auto& rglabel = _factory.getItemAuxiliary(label);
        rglabel.type = RenderItemAuxiliary::AUX_RGROUP_LABEL;
        rglabel.mol = mol;
        rglabel.rLabelIdx = i;
        rglabel.hasOffset = true;
        rglabel.offset.set(lb.x, (rt.y + lb.y) / 2);
        items.push(label);
    }
}

void RenderItemMolecule::init()
{
    if (mol == NULL)
        throw Error("molecule not set");

    if (mol->vertexCount() == 0 && mol->meta().metaData().size() == 0 && mol->rgroups.getRGroupCount() == 0)
        return;

    if (mol->vertexCount())
    {
        _core = _factory.addItemFragment();
        auto& core = _factory.getItemFragment(_core);
        core.mol = mol;
        core.refAtom = refAtom;
        core.aam = &mol->getAAMArray();
        core.reactingCenters = &mol->getReactingCenterArray();
        core.inversionArray = &mol->getInversionArray();
        core.exactChangeArray = &mol->getExactChangeArray();

        if (!mol->meta().metaData().size())
        {
            core.init();
            int lineCore = _factory.addItemHLine();
            _factory.getItemHLine(lineCore).init();
            _factory.getItemHLine(lineCore).items.push(_core);
            items.push(lineCore);

            {
                if (_getRIfThenCount() > 0)
                {
                    int _ifThen = _factory.addItemAuxiliary();
                    _factory.getItemAuxiliary(_ifThen).type = RenderItemAuxiliary::AUX_RGROUP_IFTHEN;
                    _factory.getItemAuxiliary(_ifThen).mol = mol;
                    int lineIfThen = _factory.addItemHLine();
                    _factory.getItemHLine(lineIfThen).init();
                    _factory.getItemHLine(lineIfThen).items.push(_ifThen);
                    _factory.getItemAuxiliary(_ifThen).init();
                    items.push(lineIfThen);
                }
                MoleculeRGroups& rGroups = mol->rgroups;
                for (int i = 1; i <= rGroups.getRGroupCount(); ++i)
                {
                    RGroup& rg = rGroups.getRGroup(i);
                    if (rg.fragments.size() == 0)
                        continue;

                    int lineRFrag = _factory.addItemHLine();
                    _factory.getItemHLine(lineRFrag).init();
                    items.push(lineRFrag);

                    int label = _factory.addItemAuxiliary();
                    _factory.getItemAuxiliary(label).type = RenderItemAuxiliary::AUX_RGROUP_LABEL;
                    _factory.getItemAuxiliary(label).mol = mol;
                    _factory.getItemAuxiliary(label).rLabelIdx = i;
                    _factory.getItemHLine(lineRFrag).items.push(label);
                    _factory.getItemAuxiliary(label).init();

                    PtrPool<BaseMolecule>& frags = rg.fragments;

                    for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
                    {
                        int id = _factory.addItemFragment();
                        _factory.getItemFragment(id).mol = frags[j];
                        _factory.getItemFragment(id).isRFragment = true;
                        _factory.getItemFragment(id).init();
                        _factory.getItemHLine(lineRFrag).items.push(id);
                    }
                }
            }
            return;
        }
    }
    initWithMetaData();
}

int RenderItemMolecule::_getRIfThenCount()
{
    MoleculeRGroups& rgs = mol->rgroups;
    int cnt = 0;
    for (int i = 1; i <= rgs.getRGroupCount(); ++i)
        if (rgs.getRGroup(i).if_then > 0)
            ++cnt;
    return cnt;
}

void RenderItemMolecule::estimateSizeWithMeta()
{
    Vec2f bbmin, bbmax;
    for (int i = 0; i < items.size(); ++i)
    {
        RenderItemBase& item = _factory.getItem(items[i]);
        item.estimateSize();
        if (i)
        {
            bbmin.x = std::min(bbmin.x, item.origin.x);
            bbmin.y = std::min(bbmin.y, item.origin.y);
            auto bmin = origin;
            bmin.add(item.size);
            bbmax.x = std::max(bbmax.x, item.origin.x + item.size.x);
            bbmax.y = std::max(bbmax.y, item.origin.y + item.size.y);
        }
        else
        {
            bbmin = item.origin;
            bbmax = bbmin;
            bbmax.add(item.size);
        }
    }
    size.x = std::max(size.x, bbmax.x - bbmin.x);
    size.y = std::max(size.y, bbmax.y - bbmin.y);
    origin.x = bbmin.x;
    origin.y = bbmin.y;
}

void RenderItemMolecule::estimateSize()
{
    if (_meta >= 0)
    {
        estimateSizeWithMeta();
    }
    else
    {
        RenderItemContainer::estimateSize();
        origin.set(0, 0);
        size.set(0, 0);

        float vSpace = _settings.layoutMarginVertical;
        for (int i = 0; i < items.size(); ++i)
        {
            if (_factory.isItemHLine(items[i]))
            {
                RenderItemHLine& line = _factory.getItemHLine(items[i]);
                size.x = std::max(size.x, line.size.x);
                if (i > 0)
                    size.y += vSpace;
                size.y += line.size.y;
            }
        }
        if (_core >= 0)
            refAtomPos.copy(_factory.getItemFragment(_core).refAtomPos);
    }
}

void RenderItemMolecule::render(bool idle)
{
    _rc.translate(-origin.x, -origin.y);
    _rc.storeTransform();
    if (_meta >= 0)
    {
        renderWithMeta(idle);
    }
    else
    {
        float vSpace = _settings.layoutMarginVertical;
        for (int i = 0; i < items.size(); ++i)
        {
            if (_factory.isItemHLine(items[i]))
            {
                RenderItemHLine& line = _factory.getItemHLine(items[i]);
                line.render(idle);
                _rc.translate(0, line.size.y + vSpace);
            }
        }
    }
    _rc.restoreTransform();
    _rc.removeStoredTransform();
}

void RenderItemMolecule::renderWithMeta(bool idle)
{
    for (int i = 0; i < items.size(); ++i)
    {
        _rc.restoreTransform();
        RenderItemBase& item = _factory.getItem(items[i]);
        _rc.translate(item.origin.x, item.origin.y);
        item.render(idle);
    }
}
