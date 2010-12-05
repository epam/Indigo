/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "render_context.h"
#include "render_internal.h"
#include "render_item_reaction.h"
#include "render_item_factory.h"

using namespace indigo;

RenderItemReaction::RenderItemReaction (RenderItemFactory& factory) : 
   RenderItemContainer(factory),
   rxn(NULL),
   highlighting(NULL),
   _reactantLine(-1),
   _catalystLine(-1),
   _productLine(-1),
   _arrow(-1),
   hSpace(_settings.layoutMarginHorizontal),
   catalystOffset(_settings.layoutMarginVertical / 2)
{
}

void RenderItemReaction::init ()
{
   if (rxn == NULL)
      throw Error("reaction not set");

   if (rxn->begin() >= rxn->end()) // no reactants or products
      return;

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
      _catalystLine = _factory.addItemHLine();
      _factory.getItemHLine(_catalystLine).init();
      items.push(_catalystLine);
      for (int i = rxn->catalystBegin(); i < rxn->catalystEnd(); i = rxn->catalystNext(i)) {
         _factory.getItemHLine(_catalystLine).items.push(_addFragment(i));
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

int RenderItemReaction::_addPlus ()
{
   int plus = _factory.addItemAuxiliary();
   _factory.getItemAuxiliary(plus).init();
   _factory.getItemAuxiliary(plus).type = RenderItemAuxiliary::AUX_RXN_PLUS;
   return plus;
}

int RenderItemReaction::_addFragment (int i)
{
   int mol = _factory.addItemFragment();
   _factory.getItemFragment(mol).mol = &rxn->getBaseMolecule(i);
   if (highlighting != NULL && i < highlighting->getCount()) {
      _factory.getItemFragment(mol).highlighting = &highlighting->getGraphHighlighting(i);
   }
   _factory.getItemFragment(mol).aam = &rxn->getAAMArray(i);
   _factory.getItemFragment(mol).reactingCenters = &rxn->getReactingCenterArray(i);
   _factory.getItemFragment(mol).inversionArray = &rxn->getInversionArray(i);
   QUERY_RXN_BEGIN1(rxn);
   _factory.getItemFragment(mol).exactChangeArray = &qr.getExactChangeArray(i);
   QUERY_RXN_END;
   _factory.getItemFragment(mol).init();
   return mol;
}

void RenderItemReaction::estimateSize ()
{
   RenderItemContainer::estimateSize();
   size.set(0,0);
   origin.set(0,0);
   RenderItemBase& reactants = _factory.getItem(_reactantLine);
   RenderItemBase& products = _factory.getItem(_productLine);
   RenderItemAuxiliary& arrow = _factory.getItemAuxiliary(_arrow);
   size.x = reactants.size.x + products.size.x + 2 * hSpace;
   size.y = __max(__max(reactants.size.y, products.size.y), arrow.size.y);

   float arrowWidth = arrow.size.x;
   if (_catalystLine >= 0) {
      RenderItemBase& catalysts = _factory.getItem(_catalystLine);
      arrowWidth = __max(arrowWidth, catalysts.size.x);
      size.y = __max(size.y, 2 * catalysts.size.y + 2 * catalystOffset + arrow.size.y);
   }
   size.x += arrowWidth;
}

void RenderItemReaction::render ()
{                                     
   _rc.translate(-origin.x, -origin.y);
   _rc.storeTransform();
   {
      RenderItemBase& reactants = _factory.getItem(_reactantLine);
      RenderItemBase& products = _factory.getItem(_productLine);
      RenderItemAuxiliary& arrow = _factory.getItemAuxiliary(_arrow);
      _rc.storeTransform();
      {
         _rc.translate(0, 0.5f * (size.y - reactants.size.y));
         reactants.render();
      }
      _rc.restoreTransform();
      _rc.removeStoredTransform();
      _rc.translate(reactants.size.x + hSpace, 0);

      float arrowWidth = arrow.size.x;
      if (_catalystLine >= 0) {
         RenderItemBase& catalysts = _factory.getItem(_catalystLine);
         arrowWidth = __max(arrowWidth, catalysts.size.x);
         _rc.storeTransform();
         {
            _rc.translate(0.5f * (arrowWidth - catalysts.size.x), 0.5f * (size.y - arrow.size.y) - catalysts.size.y - catalystOffset);
            catalysts.render();
         }
         _rc.restoreTransform();
         _rc.removeStoredTransform();
      }
      _rc.storeTransform();
      _rc.translate(0, 0.5f * (size.y - arrow.size.y));
      {
         arrow.arrowLength = arrowWidth;
         arrow.render();
      }
      _rc.restoreTransform();
      _rc.removeStoredTransform();
      _rc.translate(arrowWidth + hSpace, 0);

      _rc.storeTransform();
      _rc.translate(0, 0.5f * (size.y - products.size.y));
      {
         products.render();
      }
      _rc.restoreTransform();
      _rc.removeStoredTransform();
   }
   _rc.restoreTransform();
   _rc.removeStoredTransform();
}