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
#include "render_item_factory.h"
#include "render_grid.h"

using namespace indigo;

RenderGrid::RenderGrid (RenderContext& rc, RenderItemFactory& factory) : 
   Render(rc, factory), nColumns(1)
{}

RenderGrid::~RenderGrid()
{}

//void RenderGrid::_drawObj ()
//{
//   _rc.storeTransform();
//   {
//      _rc.translate((objArea.x - objSize.x * scale) / 2, (objArea.y - objSize.y * scale) / 2);
//      _rc.scale(scale);
//      _factory.getItem(obj).render();
//   }
//   _rc.restoreTransform();
//   _rc.removeStoredTransform();
//   _rc.translate(0, objArea.y);
//}
//
//void RenderGrid::_drawComment ()
//{
//   if (comment < 0)
//      return;
//   _rc.storeTransform();
//   {
//      float diff = (float)(_cnvOpt.width - 2 * outerMargin.x - commentSize.x);
//      switch (_opt.commentAlign) {
//         case ALIGNMENT_LEFT:
//            break;
//         case ALIGNMENT_CENTER:
//            _rc.translate(0.5f * diff, 0);
//            break;
//         case ALIGNMENT_RIGHT:
//            _rc.translate(diff, 0);
//            break;
//         default:
//            throw Error("Alignment value invalid");
//      }     
//      _factory.getItem(comment).render();
//   }
//   _rc.restoreTransform();
//   _rc.removeStoredTransform();
//   _rc.translate(0, commentSize.y);
//}

void RenderGrid::draw ()
{     
   _rc.initMetaSurface();
   _rc.fontsClear();
       
   bool enableRefAtoms = refAtoms.size() > 0;
   if (enableRefAtoms && refAtoms.size() != objs.size())
      throw Error("Number of reference atoms should be same as the number of objects");
   bool enableComments = comments.size() > 0;
   if (enableComments && comments.size() != objs.size())
      throw Error("Number of comments should be same as the number of objects");

   maxsz.set(0,0);
   for (int i = 0; i < objs.size(); ++i) {
      _factory.getItem(objs[i]).init();
      _factory.getItem(objs[i]).setObjScale(_getObjScale(objs[i]));
      _factory.getItem(objs[i]).estimateSize();
      maxsz.max(_factory.getItem(objs[i]).size);
   }

   int nRows = (objs.size() + nColumns - 1) / nColumns;
   totalsz.copy(maxsz);
   totalsz.x *= nColumns;
   totalsz.y *= nRows;
   
   //for (int i = 0; i < objs.size(); ++i) {
   //   _factory.getItem(objs[i]).init();
   //   _factory.getItem(objs[i]).setObjScale(_getObjScale(objs[i]));
   //}

   //for (int i = 0; i < comments.size(); ++i) {
   //   _factory.getItem(comments[i]).init();
   //}

   //commentSize.set(0,0);
   //commentOffset = 0;
   //if (comment >= 0) {
   //   _factory.getItem(comment).init();
   //   _factory.getItem(comment).estimateSize();
   //   commentSize.copy(_factory.getItem(comment).size);
   //   commentOffset = _cnvOpt.commentOffset;
   //}
   outerMargin.x = (float)(minMarg + _cnvOpt.commentMarginX);
   outerMargin.y = (float)(minMarg + _cnvOpt.commentMarginY);
   
   scale = _getScale();
   _rc.initContext(_cnvOpt.width, _cnvOpt.height);
   clientArea.set((float)_cnvOpt.width, (float)_cnvOpt.height);
   clientArea.sub(outerMargin);
   //objArea.y -= commentSize.y + commentOffset;
   _rc.init();
   _rc.translate((float)outerMargin.x, (float)outerMargin.y);
   if (_cnvOpt.xOffset > 0 || _cnvOpt.yOffset > 0)
      _rc.translate((float)_cnvOpt.xOffset, (float)_cnvOpt.yOffset);
   _rc.storeTransform();
   {
      for (int i = 0; i < objs.size(); ++i) {
         _rc.storeTransform();
         {
            int y = i % nRows;
            int x = i / nRows;
            Vec2f size(_factory.getItem(objs[i]).size);

            _rc.translate(x * maxsz.x * scale, y * maxsz.y * scale);
            _rc.scale(scale);
            _rc.translate(0.5f * (maxsz.x - size.x), 0.5f * (maxsz.y - size.y));
            _factory.getItem(objs[i]).render();
         }
         _rc.restoreTransform();
         _rc.removeStoredTransform();
      }
      //if (_opt.commentPos == COMMENT_POS_TOP) {
      //   _drawComment();
      //   _rc.translate(0, commentOffset);
      //   _drawObj();
      //} else {
      //   _drawObj();
      //   _rc.translate(0, commentOffset);
      //  _drawComment();
      //}
   }
   _rc.resetTransform();
   _rc.removeStoredTransform();
   _rc.destroyMetaSurface();
}

float RenderGrid::_getScale ()
{
   int maxPageSize = _rc.getMaxPageSize();
   float s;
   if (_cnvOpt.width <= 0 || _cnvOpt.height <= 0)
   {
      s = _cnvOpt.bondLength;

      //_cnvOpt.width = (int)ceil(__max(total.x * s, commentSize.x) + outerMargin.x * 2);
      //_cnvOpt.height = (int)ceil(objSize.y * s + commentOffset + commentSize.y + outerMargin.y * 2);
      _cnvOpt.width = (int)ceil(totalsz.x * s + outerMargin.x * 2);
      _cnvOpt.height = (int)ceil(totalsz.y * s + outerMargin.y * 2);

      //if (maxPageSize < 0 || __max(_cnvOpt.width, _cnvOpt.height) < maxPageSize)
      //   return s;
      //_cnvOpt.width = __min(_cnvOpt.width, maxPageSize);
      //_cnvOpt.height = __min(_cnvOpt.height, maxPageSize);
   }

   //float x = _cnvOpt.width - 2 * outerMargin.x,
   //   y = _cnvOpt.height - (commentSize.y + 2 * outerMargin.y + commentOffset);
   //if (x < 1 || y < 1)
   //   throw Error("Image too small, the layout requires at least %dx%d", 
   //      2 * outerMargin.x + 1, 
   //      commentSize.y + 2 * outerMargin.y + commentOffset + 1);
   //if (x * objSize.y < y * objSize.x)
   //   s = x / objSize.x;
   //else
   //   s = y / objSize.y;
   return s;
}