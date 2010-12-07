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

#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "render_context.h"
#include "render_internal.h"
#include "render_item_comment.h"
#include "render_item_factory.h"

using namespace indigo;

RenderItemComment::RenderItemComment (RenderItemFactory& factory) : 
   RenderItemContainer(factory),
      obj(-1), comment(-1), vSpace(_settings.layoutMarginVertical)
{
}

void RenderItemComment::init ()
{  
   if (obj < 0)
      throw Error("Object not set");
   _factory.getItem(obj).init();
   items.push(obj);

   comment = _factory.addItemAuxiliary();
   _factory.getItemAuxiliary(comment).type = RenderItemAuxiliary::AUX_COMMENT;
   _factory.getItemAuxiliary(comment).text.copy(_opt.comment);
   _factory.getItemAuxiliary(comment).init();
   items.push(comment);
}

void RenderItemComment::estimateSize ()
{
   RenderItemContainer::estimateSize();
   RenderItemBase& itemComment = _factory.getItem(comment);
   RenderItemBase& itemObj = _factory.getItem(obj);
   size.x = __max(itemComment.size.x, itemObj.size.x);
   size.y = itemComment.size.y + vSpace + itemObj.size.y;
}

void RenderItemComment::_renderComment ()
{
   RenderItemBase& itemComment = _factory.getItem(comment);
   _rc.storeTransform();
   _rc.translate(_opt.commentAlign * 0.5f * (size.x - itemComment.size.x), 0);
   itemComment.render();
   _rc.restoreTransform();
   _rc.removeStoredTransform();
   _rc.translate(0, itemComment.size.y);
}

void RenderItemComment::_renderObj ()
{
   RenderItemBase& itemObj = _factory.getItem(obj);
   _rc.storeTransform();
   _rc.translate(0.5f * (size.x - itemObj.size.x), 0);
   itemObj.render();
   _rc.restoreTransform();
   _rc.removeStoredTransform();
   _rc.translate(0, itemObj.size.y);
}

void RenderItemComment::render ()
{
   if (_opt.commentPos == COMMENT_POS_TOP) {
      _renderComment();
      _rc.translate(0, vSpace);
      _renderObj();
   } else {
      _renderObj();
      _rc.translate(0, vSpace);
      _renderComment();
   }
}