/****************************************************************************
 * Copyright (C) 2009-2011 GGA Software Services LLC
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

#include "math/algebra.h"
#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/output.h"
#include "base_cpp/reusable_obj_array.h"
#include "layout/metalayout.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "render_context.h"
#include "render_item.h"
#include "render_item_factory.h"
#include "render_single.h"

using namespace indigo;

RenderSingle::RenderSingle (RenderContext& rc, RenderItemFactory& factory, const CanvasOptions& cnvOpt) : Render(rc, factory, cnvOpt)
{}

RenderSingle::~RenderSingle()
{}

void RenderSingle::_drawObj ()
{
   _rc.storeTransform();
   {
      _rc.translate((objArea.x - objSize.x * scale) / 2, (objArea.y - objSize.y * scale) / 2);
      _rc.scale(scale);
      _factory.getItem(obj).render();
   }
   _rc.restoreTransform();
   _rc.removeStoredTransform();
   _rc.translate(0, objArea.y);
}

void RenderSingle::_drawComment ()
{
   if (comment < 0)
      return;
   _rc.storeTransform();
   {
      float diff = (float)(width - 2 * outerMargin.x - commentSize.x);
      _rc.translate(diff * _cnvOpt.commentAlign, 0);
      _factory.getItem(comment).render();
   }
   _rc.restoreTransform();
   _rc.removeStoredTransform();
   _rc.translate(0, commentSize.y);
}

void RenderSingle::draw ()
{     
   width = _cnvOpt.width;
   height = _cnvOpt.height;
   _rc.fontsClear();

   _factory.getItem(obj).init();

   float objScale = _getObjScale(obj);
   _factory.getItem(obj).setObjScale(objScale);
   _factory.getItem(obj).estimateSize();
   objSize.copy(_factory.getItem(obj).size);

   commentSize.set(0,0);
   commentOffset = 0;
   if (comment >= 0) {
      _factory.getItem(comment).init();
      _factory.getItem(comment).estimateSize();
      commentSize.copy(_factory.getItem(comment).size);
      commentOffset = _cnvOpt.commentOffset;
   }
   outerMargin.x = (float)(minMarg + _cnvOpt.marginX);
   outerMargin.y = (float)(minMarg + _cnvOpt.marginY);
   
   scale = _getScale();
   _rc.initContext(width, height);
   objArea.set((float)width, (float)height);
   objArea.addScaled(outerMargin, -2);
   objArea.y -= commentSize.y + commentOffset;
   _rc.init();
   _rc.translate((float)outerMargin.x, (float)outerMargin.y);
   if (_cnvOpt.xOffset > 0 || _cnvOpt.yOffset > 0)
      _rc.translate((float)_cnvOpt.xOffset, (float)_cnvOpt.yOffset);
   _rc.storeTransform();
   {
      if (_cnvOpt.commentPos == COMMENT_POS_TOP) {
         _drawComment();
         _rc.translate(0, (float)commentOffset);
         _drawObj();
      } else {
         _drawObj();
         _rc.translate(0, (float)commentOffset);
         _drawComment();
      }
   }
   _rc.resetTransform();
   _rc.removeStoredTransform();
}

float RenderSingle::_getScale ()
{
   int maxPageSize = _rc.getMaxPageSize();
   float s;
   if (width <= 0 || height <= 0)
   {
      s = _cnvOpt.bondLength;

      width = (int)ceil(__max(objSize.x * s, commentSize.x) + outerMargin.x * 2);
      height = (int)ceil(objSize.y * s + commentOffset + commentSize.y + outerMargin.y * 2);

      if (maxPageSize < 0 || __max(width, height) < maxPageSize)
         return s;
      width = __min(width, maxPageSize);
      height = __min(height, maxPageSize);
   }

   float absX = 2 * outerMargin.x;
   float absY = commentSize.y + 2 * outerMargin.y + commentOffset;
   float x = width - absX,
      y = height - absY;
   if (x < commentSize.x + 1 || y < 1)
      throw Error("Image too small, the layout requires at least %dx%d", 
         (int)(absX + commentSize.x + 2), 
         (int)(absY + 2));
   if (x * objSize.y < y * objSize.x)
      s = x / objSize.x;
   else
      s = y / objSize.y;
   return s;
}