/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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
#include "render_item_hline.h"
#include "render_item_factory.h"

using namespace indigo;

IMPL_ERROR(RenderItemHLine, "RenderItemHLine");

RenderItemHLine::RenderItemHLine (RenderItemFactory& factory) : 
   RenderItemContainer(factory)
{
}

void RenderItemHLine::init ()
{
   hSpace = _settings.layoutMarginHorizontal;
}

void RenderItemHLine::estimateSize ()
{
   RenderItemContainer::estimateSize();
   size.set(0,0);
   for (int i = 0; i < items.size(); ++i) {
      RenderItemBase& item = _factory.getItem(items[i]);
      size.y = __max(size.y, 2 * (fabs(item.referenceY) + item.size.y / 2));
      size.x += (i > 0 ? hSpace : 0) + item.size.x;
   }
}

void RenderItemHLine::render ()
{                                     
   _rc.translate(-origin.x, -origin.y);
   _rc.storeTransform();
   for (int i = 0; i < items.size(); ++i) {
      RenderItemBase& item = _factory.getItem(items[i]);
      _rc.storeTransform();
      _rc.translate(0, 0.5f * (size.y - item.size.y) + item.referenceY);
      item.render();
      _rc.restoreTransform();
      _rc.removeStoredTransform();
      _rc.translate(item.size.x + hSpace, 0);
   }
   _rc.restoreTransform();
   _rc.removeStoredTransform();
}