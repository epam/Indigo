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

#include "render_item_reaction.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "render_context.h"
#include "render_internal.h"
#include "render_item_factory.h"

using namespace indigo;

IMPL_ERROR(RenderItemReaction, "RenderItemReaction");

RenderItemReaction::RenderItemReaction(RenderItemFactory& factory)
    : RenderItemContainer(factory), rxn(NULL), hSpace(_settings.layoutMarginHorizontal), catalystOffset(_settings.layoutMarginVertical / 2), _reactantLine(-1),
      _catalystLineUpper(-1), _catalystLineLower(-1), _productLine(-1), _arrow(-1), _splitCatalysts(false)
{
}

void RenderItemReaction::init()
{
    if (rxn == NULL)
        throw Error("reaction not set");

    if (rxn->begin() >= rxn->end()) // no reactants or products
        return;

    _splitCatalysts = _opt.agentsBelowArrow;
    _reactantLine = _factory.addItemHLine();
    _factory.getItemHLine(_reactantLine).init();
    items.push(_reactantLine);
    for (int i = rxn->reactantBegin(); i < rxn->reactantEnd(); i = rxn->reactantNext(i))
    {
        if (i > rxn->reactantBegin())
            _factory.getItemHLine(_reactantLine).items.push(_addPlus());
        _factory.getItemHLine(_reactantLine).items.push(_addFragment(i));
    }

    if (rxn->catalystCount() > 0)
    {
        _catalystLineUpper = _factory.addItemHLine();
        _factory.getItemHLine(_catalystLineUpper).init();
        items.push(_catalystLineUpper);
        if (_splitCatalysts)
        {
            _catalystLineLower = _factory.addItemHLine();
            _factory.getItemHLine(_catalystLineLower).init();
            items.push(_catalystLineLower);
        }
        int halfTheNumberOfCatalysts = (rxn->catalystCount() + 1) / 2;
        for (int i = rxn->catalystBegin(), j = 0; i < rxn->catalystEnd(); i = rxn->catalystNext(i), ++j)
        {
            if (!_splitCatalysts || j < halfTheNumberOfCatalysts)
                _factory.getItemHLine(_catalystLineUpper).items.push(_addFragment(i));
            else
                _factory.getItemHLine(_catalystLineLower).items.push(_addFragment(i));
        }
    }

    _productLine = _factory.addItemHLine();
    _factory.getItemHLine(_productLine).init();
    items.push(_productLine);
    for (int i = rxn->productBegin(); i < rxn->productEnd(); i = rxn->productNext(i))
    {
        if (i > rxn->productBegin())
            _factory.getItemHLine(_productLine).items.push(_addPlus());
        _factory.getItemHLine(_productLine).items.push(_addFragment(i));
    }

    _arrow = _factory.addItemAuxiliary();
    _factory.getItemAuxiliary(_arrow).type = RenderItemAuxiliary::AUX_RXN_ARROW;
    _factory.getItemAuxiliary(_arrow).init();
    items.push(_arrow);
}

int RenderItemReaction::_addPlus()
{
    int plus = _factory.addItemAuxiliary();
    _factory.getItemAuxiliary(plus).init();
    _factory.getItemAuxiliary(plus).type = RenderItemAuxiliary::AUX_RXN_PLUS;
    return plus;
}

int RenderItemReaction::_addFragment(int i)
{
    int mol = _factory.addItemFragment();
    _factory.getItemFragment(mol).mol = &rxn->getBaseMolecule(i);
    _factory.getItemFragment(mol).aam = &rxn->getAAMArray(i);
    _factory.getItemFragment(mol).reactingCenters = &rxn->getReactingCenterArray(i);
    _factory.getItemFragment(mol).inversionArray = &rxn->getInversionArray(i);
    QUERY_RXN_BEGIN1(rxn);
    _factory.getItemFragment(mol).exactChangeArray = &qr.getExactChangeArray(i);
    QUERY_RXN_END;
    _factory.getItemFragment(mol).init();
    return mol;
}

void RenderItemReaction::estimateSize()
{
    RenderItemContainer::estimateSize();
    size.set(0, 0);
    origin.set(0, 0);

    size.x = 0;
    size.y = 0;

    if (_reactantLine >= 0)
    {
        RenderItemBase& reactants = _factory.getItem(_reactantLine);
        size.x += reactants.size.x;
        size.y = __max(size.y, reactants.size.y);
    }
    if (_productLine >= 0)
    {
        RenderItemBase& products = _factory.getItem(_productLine);
        size.x += products.size.x;
        size.y = __max(size.y, products.size.y);
    }
    if (_arrow >= 0)
    {
        RenderItemAuxiliary& arrow = _factory.getItemAuxiliary(_arrow);
        _arrowWidth = arrow.size.x;
        size.y = __max(size.y, arrow.size.y);
        if (_catalystLineUpper >= 0)
        {
            RenderItemBase& catalysts = _factory.getItem(_catalystLineUpper);
            _arrowWidth = __max(_arrowWidth, catalysts.size.x);
            size.y = __max(size.y, 2 * catalysts.size.y + 2 * catalystOffset + arrow.size.y);
        }
        if (_catalystLineLower >= 0)
        {
            RenderItemBase& catalysts = _factory.getItem(_catalystLineLower);
            _arrowWidth = __max(_arrowWidth, catalysts.size.x);
            size.y = __max(size.y, 2 * catalysts.size.y + 2 * catalystOffset + arrow.size.y);
        }
        size.x += _arrowWidth + 2 * hSpace;
    }
}

void RenderItemReaction::render(bool idle)
{
    _rc.translate(-origin.x, -origin.y);
    _rc.storeTransform();
    {
        if (_reactantLine >= 0)
        {
            RenderItemBase& reactants = _factory.getItem(_reactantLine);
            _rc.storeTransform();
            {
                _rc.translate(0, 0.5f * (size.y - reactants.size.y));
                reactants.render(idle);
            }
            _rc.restoreTransform();
            _rc.removeStoredTransform();
            _rc.translate(reactants.size.x, 0);
        }
        if (_arrow >= 0)
        {
            RenderItemAuxiliary& arrow = _factory.getItemAuxiliary(_arrow);
            _rc.translate(hSpace, 0);
            if (_catalystLineUpper >= 0)
            {
                RenderItemBase& catalysts = _factory.getItem(_catalystLineUpper);
                _rc.storeTransform();
                {
                    _rc.translate(0.5f * (_arrowWidth - catalysts.size.x), 0.5f * (size.y - arrow.size.y) - catalysts.size.y - catalystOffset);
                    catalysts.render(idle);
                }
                _rc.restoreTransform();
                _rc.removeStoredTransform();
            }
            if (_catalystLineLower >= 0)
            {
                RenderItemBase& catalysts = _factory.getItem(_catalystLineLower);
                _rc.storeTransform();
                {
                    _rc.translate(0.5f * (_arrowWidth - catalysts.size.x), 0.5f * (size.y + arrow.size.y) /*+ catalysts.size.y*/ + catalystOffset);
                    catalysts.render(idle);
                }
                _rc.restoreTransform();
                _rc.removeStoredTransform();
            }
            _rc.storeTransform();
            {
                _rc.translate(0, 0.5f * (size.y - arrow.size.y));
                arrow.arrowLength = _arrowWidth;
                arrow.render(idle);
            }
            _rc.restoreTransform();
            _rc.removeStoredTransform();
            _rc.translate(_arrowWidth + hSpace, 0);
        }

        if (_productLine >= 0)
        {
            RenderItemBase& products = _factory.getItem(_productLine);

            _rc.storeTransform();
            {
                _rc.translate(0, 0.5f * (size.y - products.size.y));
                products.render(idle);
            }
            _rc.restoreTransform();
            _rc.removeStoredTransform();
        }
    }
    _rc.restoreTransform();
    _rc.removeStoredTransform();
}