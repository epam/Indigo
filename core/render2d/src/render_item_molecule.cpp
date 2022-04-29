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

void RenderItemMolecule::init()
{
    if (mol == NULL)
        throw Error("molecule not set");

    if (mol->vertexCount() == 0 && mol->metaData().size() == 0)
        return;

    _core = _factory.addItemFragment();
    _factory.getItemFragment(_core).mol = mol;
    _factory.getItemFragment(_core).refAtom = refAtom;
    _factory.getItemFragment(_core).init();

    int lineCore = _factory.addItemHLine();
    _factory.getItemHLine(lineCore).init();
    _factory.getItemHLine(lineCore).items.push(_core);

    items.push(lineCore);

    {
        MoleculeRGroups& rGroups = mol->rgroups;
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

    // add meta
    if (mol->metaData().size())
    {
        _meta = _factory.addItemAuxiliary();
        _factory.getItemAuxiliary(_meta).type = RenderItemAuxiliary::AUX_META;
        _factory.getItemAuxiliary(_meta).meta = mol;
        _factory.getItemAuxiliary(_meta).init();
        items.push(_meta);
    }
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

void RenderItemMolecule::estimateSize()
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

    if (_meta >= 0)
    {
        Vec2f diff(0, 0);
        RenderItemAuxiliary& meta = _factory.getItemAuxiliary(_meta);
        if (_core >= 0)
        {
            auto& frag = _factory.getItemFragment(_core);
            diff.x = frag.min().x - meta._min.x;
            diff.y = meta._max.y - frag.max().y;

            meta.origin = diff;
            if (diff.x > 0)
            {
                origin.x -= diff.x;
                meta.size.x += diff.x;
            }
            else
            {
                meta.size.x -= diff.x;
            }
            if (diff.y < 0)
            {
                meta.size.y -= diff.y;
            }
            else
            {
                origin.y -= diff.y;
                meta.size.y += diff.y;
            }
        }
        size.x = std::max(size.x, meta.size.x);
        size.y = std::max(size.y, meta.size.y);
    }
}

void RenderItemMolecule::render(bool idle)
{
    _rc.translate(-origin.x, -origin.y);
    if (_meta >= 0)
    {
        RenderItemAuxiliary& meta = _factory.getItemAuxiliary(_meta);
        _rc.storeTransform();
        meta.render(idle);
        _rc.restoreTransform();
        _rc.removeStoredTransform();
    }

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