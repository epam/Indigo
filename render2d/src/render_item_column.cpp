/****************************************************************************
 * Copyright (C) 2009-2013 GGA Software Services LLC
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

#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "render_context.h"
#include "render_internal.h"
#include "render_item_column.h"
#include "render_item_factory.h"

using namespace indigo;

IMPL_ERROR(RenderItemColumn, "RenderItemColumn");

RenderItemColumn::RenderItemColumn (RenderItemFactory& factory) :
   RenderItemContainer(factory), vSpace(0.0), alignment(MultilineTextLayout::Alignment::Center)
{
}

void RenderItemColumn::init ()
{
}

void RenderItemColumn::setVerticalSpacing (float spacing)
{
   vSpace = spacing;
}

void RenderItemColumn::setAlignment (MultilineTextLayout::Alignment alignment)
{
   this->alignment = alignment;
}

void RenderItemColumn::estimateSize ()
{
   RenderItemContainer::estimateSize();
   size.set(0,0);
   for (int i = 0; i < items.size(); ++i) {
      RenderItemBase& item = _factory.getItem(items[i]);
      size.y += (i > 0 ? vSpace : 0) + item.size.y;
      size.x = __max(size.x, item.size.x); //__max(size.x, 2 * (fabs(item.referenceY) + item.size.x / 2));
   }
}

void RenderItemColumn::render ()
{
   _rc.translate(-origin.x, -origin.y);
   _rc.storeTransform();
   for (int i = 0; i < items.size(); ++i) {
      RenderItemBase& item = _factory.getItem(items[i]);
      _rc.storeTransform();
      _rc.translate(MultilineTextLayout::getRelativeOffset(alignment) * (size.x - item.size.x), 0);
      item.render();
      _rc.restoreTransform();
      _rc.removeStoredTransform();
      _rc.translate(0, item.size.y + vSpace);
   }
   _rc.restoreTransform();
   _rc.removeStoredTransform();
}