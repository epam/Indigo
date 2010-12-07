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

void RenderGrid::draw ()
{     
   _rc.initMetaSurface();
   _rc.fontsClear();
       
   bool enableRefAtoms = refAtoms.size() > 0 && _factory.isItemMolecule(objs[0]);
   if (enableRefAtoms && refAtoms.size() != objs.size())
      throw Error("Number of reference atoms should be same as the number of objects");
   bool enableComments = comments.size() > 0;
   if (enableComments && comments.size() != objs.size())
      throw Error("Number of comments should be same as the number of objects");

   maxsz.set(0,0);
   Vec2f refSizeLT, refSizeRB;
   for (int i = 0; i < objs.size(); ++i) {
      if (enableRefAtoms)
         _factory.getItemMolecule(objs[i]).refAtom = refAtoms[i];
      _factory.getItem(objs[i]).init();
      _factory.getItem(objs[i]).setObjScale(_getObjScale(objs[i]));
      _factory.getItem(objs[i]).estimateSize();
      if (enableRefAtoms) {
         refSizeLT.max(_factory.getItemMolecule(objs[i]).refAtomPos);
         Vec2f d;
         d.diff(_factory.getItemMolecule(objs[i]).size, 
            _factory.getItemMolecule(objs[i]).refAtomPos);
         refSizeRB.max(d);
      } else {
         maxsz.max(_factory.getItem(objs[i]).size);
      }
   }
   if (enableRefAtoms)
      maxsz.sum(refSizeLT, refSizeRB);

   nRows = (objs.size() + nColumns - 1) / nColumns;

   maxCommentSize.set(0,0);
   commentOffset = 0;
   if (enableComments) {
      commentOffset = _cnvOpt.commentOffset;
      for (int i = 0; i < comments.size(); ++i) {
         _factory.getItem(comments[i]).init();
         _factory.getItem(comments[i]).estimateSize();
         maxCommentSize.max(_factory.getItem(comments[i]).size);
      }
   }

   outerMargin.x = (float)(minMarg + _cnvOpt.marginX);
   outerMargin.y = (float)(minMarg + _cnvOpt.marginY);
   
   scale = _getScale();
   _rc.initContext(_cnvOpt.width, _cnvOpt.height);
   cellsz.set(__max(maxsz.x * scale, maxCommentSize.x),
      maxsz.y * scale + maxCommentSize.y + commentOffset);
   clientArea.set(cellsz.x * nColumns + _cnvOpt.gridMarginX * (nColumns - 1),
      cellsz.y * nRows + _cnvOpt.gridMarginY * (nRows - 1));
   _rc.init();
   _rc.translate((_cnvOpt.width - clientArea.x) / 2, (_cnvOpt.height - clientArea.y) / 2);
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

            _rc.translate(x * (cellsz.x + _cnvOpt.gridMarginX), y * (cellsz.y + _cnvOpt.gridMarginY));
            _rc.storeTransform();
            {
               if (enableRefAtoms) {
                  _rc.translate(0.5f * (cellsz.x - maxsz.x * scale), 0);
                  Vec2f d;
                  d.diff(refSizeLT, _factory.getItemMolecule(objs[i]).refAtomPos);
                  d.scale(scale);
                  _rc.translate(d.x, d.y);
               } else {
                  _rc.translate(0.5f * (cellsz.x - size.x * scale), 0.5f * (maxsz.y - size.y) * scale);
               }
               _rc.scale(scale);
               _factory.getItem(objs[i]).render();
            }
            _rc.restoreTransform();
            _rc.removeStoredTransform();
            _rc.translate(0, maxsz.y * scale + commentOffset);

            if (enableComments) {
               Vec2f commentSize(_factory.getItem(comments[i]).size);
               _rc.translate(_opt.commentAlign * (cellsz.x - commentSize.x) / 2, 0.5f * (maxCommentSize.y - commentSize.y));
               _factory.getItem(comments[i]).render();
            }
         }
         _rc.restoreTransform();
         _rc.removeStoredTransform();
      }
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

      _cnvOpt.width = (int)ceil(__max(maxsz.x * s, maxCommentSize.x) * nColumns + _cnvOpt.gridMarginX * (nColumns - 1) + outerMargin.x * 2);
      _cnvOpt.height = (int)ceil((maxsz.y * s + maxCommentSize.y + commentOffset) * nRows + _cnvOpt.gridMarginY * (nRows - 1) + outerMargin.y * 2);

      if (maxPageSize < 0 || __max(_cnvOpt.width, _cnvOpt.height) < maxPageSize)
         return s;
      _cnvOpt.width = __min(_cnvOpt.width, maxPageSize);
      _cnvOpt.height = __min(_cnvOpt.height, maxPageSize);
   }

   float absX = _cnvOpt.gridMarginX * (nColumns - 1) + outerMargin.x * 2;
   float absY = (maxCommentSize.y + commentOffset) * nRows + _cnvOpt.gridMarginY * (nRows - 1) + outerMargin.y * 2;
   float x = _cnvOpt.width - absX,
      y = _cnvOpt.height - absY;
   if (x < maxCommentSize.x * nRows + 1 || y < 1)
      throw Error("Image too small, the layout requires at least %dx%d", 
         (int)(absX + maxCommentSize.x * nRows + 2), 
         (int)(absY + 2));
   Vec2f totalScaleableSize(maxsz.x * nColumns, maxsz.y * nRows);
   if (x * totalScaleableSize.y < y * totalScaleableSize.x)
      s = x / totalScaleableSize.x;
   else
      s = y / totalScaleableSize.y;
   return s;
}