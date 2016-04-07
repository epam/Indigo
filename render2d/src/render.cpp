/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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
#include "render.h"

using namespace indigo;

IMPL_ERROR(Render, "Render");

Render::Render (RenderContext& rc, RenderItemFactory& factory, const CanvasOptions& cnvOpt, int bondLength, bool bondLengthSet) :
   minMarg(2),
   _rc(rc), _settings(rc.getRenderSettings()), _cnvOpt(cnvOpt), _opt(rc.opt), 
   _factory(factory),
   _bondLength(bondLength), _bondLengthSet(bondLengthSet)
{}

Render::~Render()
{}

float Render::_getObjScale (int item)
{
   float avgBondLength = 1.0f;
   int bondCount = _factory.getItem(item).getBondCount();
   int atomCount = _factory.getItem(item).getAtomCount();
   if (bondCount > 0) {
      avgBondLength = _factory.getItem(item).getTotalBondLength() / bondCount;
   } else {
      avgBondLength = _factory.getItem(item).getTotalClosestAtomDistance() / atomCount;
   }
   if (avgBondLength < 1e-4) {
      avgBondLength = 1.0f;
   }
   float objScale = 1 / avgBondLength;
   return objScale;
}

int Render::_getMaxWidth ()
{
   int maxPageSize = _rc.getMaxPageSize();
   return _cnvOpt.maxWidth > 0 ? __min(_cnvOpt.maxWidth, maxPageSize) : maxPageSize;
}

int Render::_getMaxHeight ()
{
   int maxPageSize = _rc.getMaxPageSize();
   return _cnvOpt.maxHeight > 0 ? __min(_cnvOpt.maxHeight, maxPageSize) : maxPageSize;
}

float Render::_getScale (int w, int h)
{
   float scale = _getMaxScale(w, h);
   if (_bondLength > 0 && _bondLength < scale)
      return (float)_bondLength;
   return scale;
}

float Render::_getMaxScale (int w, int h)
{
   float s = (float)(_bondLength > 0 ? _bondLength : 100);
   int maxWidth = _getMaxWidth();
   int maxHeight = _getMaxHeight();
   int defaultWidth = _getDefaultWidth(s);
   int defaultHeight = _getDefaultHeight(s);
   if (h >= 1 && w >= 1)
      return _getScaleGivenSize(w, h);
   if (h >= 1)
      return _getScaleGivenSize(maxWidth, h);
   if (w >= 1)
      return _getScaleGivenSize(w, maxHeight);
   if (defaultWidth <= maxWidth && defaultHeight <= maxHeight)
      return s;
   return _getScaleGivenSize(__min(defaultWidth, maxWidth), __min(defaultHeight, maxHeight));
}