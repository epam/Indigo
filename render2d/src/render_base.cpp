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

#include "math/algebra.h"
#include "base_cpp/array.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/output.h"
#include "graph/graph_highlighting.h"
#include "base_cpp/reusable_obj_array.h"
#include "layout/metalayout.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "render_context.h"
#include "render_base.h"

using namespace indigo;

RenderBase::RenderBase (RenderContext& rc) : _rc(rc), _settings(rc.getRenderSettings())
{}

RenderBase::~RenderBase()
{}

void RenderBase::draw ()
{     
   _rc.initMetaSurface();
   Vec2f commentSz, commentRel;
   int commentHeight = 0, commentWidth = 0;
   if (opt.comment.size() > 1) {
      _rc.getTextSize(commentSz, commentRel, FONT_SIZE_COMMENT, opt.comment.ptr());
      commentWidth = (int)ceil(commentSz.x);
      commentHeight = (int)ceil(commentSz.y);
   }
   if ((cnvOpt.width >= 0 && commentWidth > cnvOpt.width) || 
      (cnvOpt.height >= 0 && commentHeight > cnvOpt.height))
      throw Error("Comment doesn't fit in the picture size requested");

   _initLayout(); 

   _rc.fontsClear();
   if (_ml.isEmpty())
   {
      if (cnvOpt.width < 0 || cnvOpt.height < 0)
         cnvOpt.width = cnvOpt.height = 1;
      _rc.initContext(cnvOpt.width, cnvOpt.height);
      _rc.destroyMetaSurface();
      return;
   }

   _ml.prepare();

   int minMarg = 2; // small absolute margin to allow for cairo font scaling errors
   void (*cb_process_tmp) (Metalayout::LayoutItem& item, const Vec2f& pos, void* context);
   cb_process_tmp = _ml.cb_process;
   _ml.cb_process = cb_prepare;
   _ml.process();
   _ml.cb_process = cb_process_tmp;

   _ml.calcContentSize();
   Vec2f sz;
   sz.copy(_ml.getContentSize());

   Vec2f delta;
   delta.set(sz.x, sz.y);

   cnvOpt.height -= commentHeight; // if height is not set, this will make no difference
   float scale = _getScale(delta, cnvOpt.marginX + minMarg, cnvOpt.marginY + minMarg);
   cnvOpt.height += commentHeight;

   if (cnvOpt.width < commentWidth)
      cnvOpt.width = commentWidth;
   _rc.initContext(cnvOpt.width + 2 * cnvOpt.commentMarginX, cnvOpt.height + 2 * cnvOpt.commentMarginY);
   _rc.translate((float)cnvOpt.commentMarginX, (float)cnvOpt.commentMarginY);
   _rc.storeTransform();
   if (cnvOpt.xOffset > 0 || cnvOpt.yOffset > 0)
      _rc.translate((float)cnvOpt.xOffset, (float)cnvOpt.yOffset);
   if (opt.commentPos == COMMENT_POS_TOP)
      _rc.translate(0, (float)commentHeight);
   _rc.translate((cnvOpt.width - delta.x * scale) / 2, 
      (cnvOpt.height - commentHeight - delta.y * scale) / 2);
   _rc.scale(scale, scale);
   _rc.init();

   _rc.storeTransform();
   _ml.process();
   _rc.removeStoredTransform();
   _rc.resetTransform();
   if (opt.comment.size() > 1) {
      _rc.restoreTransform();
      TextItem ti;
      ti.text.copy(opt.comment);
      ti.fontsize = FONT_SIZE_COMMENT;
      Vec2f c;
      if (opt.commentAlign == ALIGNMENT_LEFT)
         c.x = 0.5f * commentWidth;
      else if (opt.commentAlign == ALIGNMENT_CENTER)
         c.x = 0.5f * cnvOpt.width;
      else
         c.x = cnvOpt.width - 0.5f * commentWidth;
      if (opt.commentPos == COMMENT_POS_TOP) {
         c.y = commentHeight/2.0f;
      } else {
         c.y = cnvOpt.height - commentHeight/2.0f;
      }
      _rc.setTextItemSize(ti, c);
      _rc.drawTextItemText(ti, opt.commentColor, false);
   }
   _rc.removeStoredTransform();

   _rc.destroyMetaSurface();
}

Metalayout::LayoutItem& RenderBase::_pushItem (Metalayout::LayoutLine& line, int type, int id)
{
   Metalayout::LayoutItem& item = line.items.push();
   item.type = type;
   item.id = id;
   item.over = false;
   item.fragment = false;
   return item;
}  

Metalayout::LayoutItem& RenderBase::_pushMol (Metalayout::LayoutLine& line, int type, int id, BaseMolecule& mol, bool catalyst)
{
   Metalayout::LayoutItem& item = _pushItem(line, type, id);
   Metalayout::getBoundRect(item.min, item.max, mol);
   item.over = catalyst;
   item.fragment = true;
   item.scaledSize.diff(item.max, item.min);
   item.scaledOffset.set(0, 0);
   return item;
}

float RenderBase::_getScale (const Vec2f& delta, int absMargX, int absMargY)
{
   float scale;
   if (cnvOpt.width <= 0 || cnvOpt.height <= 0)
   {
      scale = cnvOpt.bondLength;

      cnvOpt.width = (int)(delta.x * scale) + 2 * absMargX;
      cnvOpt.height = (int)(delta.y * scale) + 2 * absMargY;
   }
   else
   {  
      int x = cnvOpt.width - 2 * absMargX,
         y = cnvOpt.height - 2 * absMargY;
      if (x * delta.y < y * delta.x)
         scale = x / delta.x;
      else
         scale = y / delta.y;
   }  
   
   int maxdim = __max(cnvOpt.width, cnvOpt.height);
   int maxPageSize = _rc.getMaxPageSize();
   // if maxPageSize is < 0, page size is not limited
   if (maxPageSize > 0 && maxdim > maxPageSize) 
   {
      // scale correction to fit in the max page size
      float fitScale = ((float)maxPageSize) / maxdim; 
      cnvOpt.width = (int)floorf(cnvOpt.width * fitScale);
      cnvOpt.height = (int)floorf(cnvOpt.height * fitScale);
      scale *= fitScale;
   }

   return scale;
}

void RenderBase::_setSize (Metalayout::LayoutItem& item)
{
   _rc.initNullContext();
   Vec2f bbmin, bbmax;
   _drawMol(item);
   _rc.bbGetMin(bbmin);
   _rc.bbGetMax(bbmax);
   _rc.resetContext();
   item.scaledSize.diff(bbmax, bbmin);
   item.scaledOffset.copy(bbmin);
}  

BaseMolecule& RenderBase::cb_getMol (int id, void* context)
{
   return ((RenderBase*)context)->_getMol(id);
}

void RenderBase::cb_process (Metalayout::LayoutItem& item, const Vec2f& pos, void* context)
{
   RenderBase* render = (RenderBase*)context;
   render->_rc.restoreTransform();
   render->_rc.translate(pos.x, pos.y - item.scaledSize.y / 2);
}

void RenderBase::cb_prepare (Metalayout::LayoutItem& item, const Vec2f& pos, void* context)
{
   RenderBase* render = (RenderBase*)context;
   if (item.type == ITEM_TYPE_BASE_MOL)
      render->_setSize(item);
}