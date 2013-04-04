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
#include "render_grid.h"

using namespace indigo;

IMPL_ERROR(RenderGrid, "RenderGrid");

RenderGrid::RenderGrid (RenderContext& rc, RenderItemFactory& factory, const CanvasOptions& cnvOpt, int bondLength, bool bondLengthSet) :
   Render(rc, factory, cnvOpt, bondLength, bondLengthSet), nColumns(cnvOpt.gridColumnNumber), comment(-1)
{}

RenderGrid::~RenderGrid()
{}

void RenderGrid::_drawComment ()
{
   if (comment < 0)
      return;
   _rc.storeTransform();
   {
      float diff = (float)(_width - 2 * outerMargin.x - commentSize.x);
      _rc.translate(diff * _cnvOpt.commentAlign, 0);
      _factory.getItem(comment).render();
   }
   _rc.restoreTransform();
   _rc.removeStoredTransform();
   _rc.translate(0, commentSize.y);
}

void RenderGrid::draw ()
{
   _width = _cnvOpt.width;
   _height = _cnvOpt.height;
   _rc.fontsClear();

   bool enableRefAtoms = refAtoms.size() > 0 && _factory.isItemMolecule(objs[0]);
   if (enableRefAtoms && refAtoms.size() != objs.size())
      throw Error("Number of reference atoms should be same as the number of objects");
   bool enableTitles = titles.size() > 0;
   if (enableTitles && titles.size() != objs.size())
      throw Error("Number of titles should be same as the number of objects");

   nRows = (objs.size() + nColumns - 1) / nColumns;

   commentSize.set(0,0);
   commentOffset = 0;
   if (comment >= 0) {
      _factory.getItem(comment).init();
      _factory.getItem(comment).estimateSize();
      commentSize.copy(_factory.getItem(comment).size);
      commentOffset = _cnvOpt.commentOffset;
   }

   maxsz.set(0,0);
   Vec2f refSizeLT, refSizeRB;
   Array<float> columnExtentLeft, columnExtentRight, rowExtentTop, rowExtentBottom;
   columnExtentLeft.clear_resize(nColumns);
   columnExtentRight.clear_resize(nColumns);
   columnExtentLeft.fill(0);
   columnExtentRight.fill(0);
   rowExtentTop.clear_resize(nRows);
   rowExtentBottom.clear_resize(nRows);
   rowExtentTop.fill(0);
   rowExtentBottom.fill(0);
   for (int i = 0; i < objs.size(); ++i) {
      if (enableRefAtoms)
         _factory.getItemMolecule(objs[i]).refAtom = refAtoms[i];
      _factory.getItem(objs[i]).init();
      _factory.getItem(objs[i]).setObjScale(_getObjScale(objs[i]));
      _factory.getItem(objs[i]).estimateSize();
      if (enableRefAtoms) {
         const Vec2f& r = _factory.getItemMolecule(objs[i]).refAtomPos;
         Vec2f d;
         d.diff(_factory.getItemMolecule(objs[i]).size, r);
         refSizeLT.max(r);
         int col = i % nColumns;
         int row = i / nColumns;
         columnExtentLeft[col] = __max(columnExtentLeft[col], r.x);
         columnExtentRight[col] = __max(columnExtentRight[col], d.x);
         rowExtentTop[row] = __max(rowExtentTop[row], r.y);
         rowExtentBottom[row] = __max(rowExtentBottom[row], d.y);
         refSizeRB.max(d);
      } else {
         maxsz.max(_factory.getItem(objs[i]).size);
      }
   }
   if (enableRefAtoms)
      maxsz.sum(refSizeLT, refSizeRB);

   maxTitleSize.set(0,0);
   titleOffset = 0;
   if (enableTitles) {
      titleOffset = _cnvOpt.titleOffset;
      for (int i = 0; i < titles.size(); ++i) {
         _factory.getItem(titles[i]).init();
         _factory.getItem(titles[i]).estimateSize();
         maxTitleSize.max(_factory.getItem(titles[i]).size);
      }
   }

   outerMargin.x = (float)(minMarg + _cnvOpt.marginX);
   outerMargin.y = (float)(minMarg + _cnvOpt.marginY);

   _width = __min(_width, _getMaxWidth());
   _height = __min(_height, _getMaxHeight());
   scale = _getScale(_width, _height);
   if (_width < 1)
      _width = _getDefaultWidth(scale);
   if (_height < 1)
      _height = _getDefaultHeight(scale);

   _rc.initContext(_width, _height);
   cellsz.set(__max(maxsz.x * scale, maxTitleSize.x),
      maxsz.y * scale + maxTitleSize.y + titleOffset);
   clientArea.set(cellsz.x * nColumns + _cnvOpt.gridMarginX * (nColumns - 1),
      cellsz.y * nRows + _cnvOpt.gridMarginY * (nRows - 1));
   _rc.init();
   if (_cnvOpt.xOffset > 0 || _cnvOpt.yOffset > 0)
      _rc.translate((float)_cnvOpt.xOffset, (float)_cnvOpt.yOffset);
   _rc.translate(outerMargin.x, outerMargin.y);
   if (_cnvOpt.commentPos == COMMENT_POS_TOP) {
      _drawComment();
      _rc.translate(0, (float)commentOffset);
   }
   _rc.storeTransform();
   {
      _rc.translate((_width - clientArea.x) / 2 - outerMargin.x, (_height - commentSize.y - commentOffset - clientArea.y) / 2 - outerMargin.y);
      for (int i = 0; i < objs.size(); ++i) {
         _rc.storeTransform();
         {
            int y = i / nColumns;
            int x = i % nColumns;
            Vec2f size(_factory.getItem(objs[i]).size);

            _rc.translate(x * (cellsz.x + _cnvOpt.gridMarginX), y * (cellsz.y + _cnvOpt.gridMarginY));
            _rc.storeTransform();
            {
               if (enableRefAtoms) {
                  _rc.translate(0.5f * (cellsz.x - (columnExtentRight[x] + columnExtentLeft[x]) * scale), 0.5f * (maxsz.y - (rowExtentBottom[y]+ rowExtentTop[y])) * scale);
                  const Vec2f r = _factory.getItemMolecule(objs[i]).refAtomPos;
                  _rc.translate((columnExtentLeft[x] - r.x) * scale, (rowExtentTop[y] - r.y) * scale);
               } else {
                  _rc.translate(0.5f * (cellsz.x - size.x * scale), 0.5f * (maxsz.y - size.y) * scale);
               }
               _rc.scale(scale);
               _factory.getItem(objs[i]).render();
            }
            _rc.restoreTransform();
            _rc.removeStoredTransform();
            _rc.translate(0, maxsz.y * scale + titleOffset);

            if (enableTitles) {
               Vec2f titleSize(_factory.getItem(titles[i]).size);
               _rc.translate(_cnvOpt.titleAlign * (cellsz.x - titleSize.x), 0.5f * (maxTitleSize.y - titleSize.y));
               _factory.getItem(titles[i]).render();
            }
         }
         _rc.restoreTransform();
         _rc.removeStoredTransform();
      }
   }
   _rc.restoreTransform();
   _rc.removeStoredTransform();
   if (_cnvOpt.commentPos == COMMENT_POS_BOTTOM) {
      _rc.translate(0, _height - commentOffset - commentSize.y - 2*outerMargin.y);
      _drawComment();
   }
}

int RenderGrid::_getDefaultWidth (const float s)
{
   return (int)ceil(__max(__max(maxsz.x * s, maxTitleSize.x) * nColumns + _cnvOpt.gridMarginX * (nColumns - 1), commentSize.x) + outerMargin.x * 2);
}
int RenderGrid::_getDefaultHeight (const float s) {
   return (int)ceil((maxsz.y * s + maxTitleSize.y + titleOffset) * nRows + _cnvOpt.gridMarginY * (nRows - 1) + outerMargin.y * 2 + commentSize.y + commentOffset);
}

float RenderGrid::_getScaleGivenSize(int w, int h)
{
   float absX = _cnvOpt.gridMarginX * (nColumns - 1) + outerMargin.x * 2;
   float absY = (maxTitleSize.y + titleOffset) * nRows + _cnvOpt.gridMarginY * (nRows - 1) + outerMargin.y * 2 + commentSize.y + commentOffset;
   float x = w - absX,
      y = h - absY;
   if (x < maxTitleSize.x * nRows + 1 || w < commentSize.x + outerMargin.x * 2 + 1 || y < 1)
      throw Error("Image too small, the layout requires at least %dx%d",
         (int)__max(absX + maxTitleSize.x * nRows + 2,commentSize.x + outerMargin.x * 2 + 2),
         (int)(absY + 2));
   Vec2f totalScaleableSize(maxsz.x * nColumns, maxsz.y * nRows);
   if (x * totalScaleableSize.y < y * totalScaleableSize.x)
      return x / totalScaleableSize.x;
   return y / totalScaleableSize.y;
}
