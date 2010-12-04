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
#include "render_item.h"
#include "render.h"

using namespace indigo;

Render::Render (RenderContext& rc, RenderItemBase& item) : _rc(rc), _settings(rc.getRenderSettings()), _cnvOpt(rc.cnvOpt), _item(item)
{}

Render::~Render()
{}

void Render::draw ()
{     
   _rc.initMetaSurface();
   _rc.fontsClear();

   _item.init();
   double objScale = 1.0f;
   int bondCount = _item.getBondCount();
   if (bondCount > 0)
      objScale = _item.getTotalBondLength() / bondCount;
   int atomCount = _item.getAtomCount();
   if (atomCount > 0)
      objScale = _item.getTotalClosestAtomDistance() / atomCount;

   int minMarg = 2; // small absolute margin to allow for cairo font scaling errors

   _item.estimateSize();
   const Vec2f& sz = _item.size;

   Vec2f delta;
   delta.set(sz.x, sz.y);

   float scale = _getScale(delta, _cnvOpt.marginX + minMarg, _cnvOpt.marginY + minMarg);

   _rc.initContext(_cnvOpt.width + 2 * _cnvOpt.commentMarginX, _cnvOpt.height + 2 * _cnvOpt.commentMarginY);
   _rc.translate((float)_cnvOpt.commentMarginX, (float)_cnvOpt.commentMarginY);
   _rc.storeTransform();
   if (_cnvOpt.xOffset > 0 || _cnvOpt.yOffset > 0)
      _rc.translate((float)_cnvOpt.xOffset, (float)_cnvOpt.yOffset);
   _rc.translate((_cnvOpt.width - delta.x * scale) / 2, 
      (_cnvOpt.height - delta.y * scale) / 2);
   _rc.scale(scale, scale);
   _rc.init();

   _rc.storeTransform();
   _item.render();
   _rc.removeStoredTransform();
   _rc.resetTransform();
   //if (opt.comment.size() > 1) {
   //   _rc.restoreTransform();
   //   TextItem ti;
   //   ti.text.copy(opt.comment);
   //   ti.fontsize = FONT_SIZE_COMMENT;
   //   Vec2f c;
   //   if (opt.commentAlign == ALIGNMENT_LEFT)
   //      c.x = 0.5f * commentWidth;
   //   else if (opt.commentAlign == ALIGNMENT_CENTER)
   //      c.x = 0.5f * _cnvOpt.width;
   //   else
   //      c.x = _cnvOpt.width - 0.5f * commentWidth;
   //   if (opt.commentPos == COMMENT_POS_TOP) {
   //      c.y = commentHeight/2.0f;
   //   } else {
   //      c.y = _cnvOpt.height - commentHeight/2.0f;
   //   }
   //   _rc.setTextItemSize(ti, c);
   //   _rc.drawTextItemText(ti, opt.commentColor, false);
   //}
   _rc.removeStoredTransform();

   _rc.destroyMetaSurface();
}

float Render::_getScale (const Vec2f& delta, int absMargX, int absMargY)
{
   float scale;
   if (_cnvOpt.width <= 0 || _cnvOpt.height <= 0)
   {
      scale = _cnvOpt.bondLength;

      _cnvOpt.width = (int)(delta.x * scale) + 2 * absMargX;
      _cnvOpt.height = (int)(delta.y * scale) + 2 * absMargY;
   }
   else
   {  
      int x = _cnvOpt.width - 2 * absMargX,
         y = _cnvOpt.height - 2 * absMargY;
      if (x * delta.y < y * delta.x)
         scale = x / delta.x;
      else
         scale = y / delta.y;
   }  
   
   int maxdim = __max(_cnvOpt.width, _cnvOpt.height);
   int maxPageSize = _rc.getMaxPageSize();
   // if maxPageSize is < 0, page size is not limited
   if (maxPageSize > 0 && maxdim > maxPageSize) 
   {
      // scale correction to fit in the max page size
      float fitScale = ((float)maxPageSize) / maxdim; 
      _cnvOpt.width = (int)floorf(_cnvOpt.width * fitScale);
      _cnvOpt.height = (int)floorf(_cnvOpt.height * fitScale);
      scale *= fitScale;
   }

   return scale;
}
//
//void Render::_setSize (Metalayout::LayoutItem& item)
//{
//   _rc.initNullContext();
//   Vec2f bbmin, bbmax;
//   Vec2f pos;
//   _drawItem(item, pos, true);
//   _rc.bbGetMin(bbmin);
//   _rc.bbGetMax(bbmax);
//   _rc.resetContext();
//   item.size.diff(bbmax, bbmin);
//   item.origin.copy(bbmin);
//}  
//
//BaseMolecule& Render::cb_getMol (int id, void* context)
//{
//   return ((Render*)context)->_getMol(id);
//}
//
//void Render::cb_process (Metalayout::LayoutItem& item, const Vec2f& pos, void* context)
//{
//   Render* render = (Render*)context;
//   render->_drawItem(item, pos, false);
//}
//
//void Render::cb_prepare (Metalayout::LayoutItem& item, const Vec2f& pos, void* context)
//{
//   Render* render = (Render*)context;
//   render->_setSize(item);
//}