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

#ifndef __render_grid_h__
#define __render_grid_h__

#include "render.h"

namespace indigo {

class RenderGrid : Render {
public:
   RenderGrid (RenderContext& rc, RenderItemFactory& factory, const CanvasOptions& cnvOpt, int bondLength, bool bondLengthSet);
   virtual ~RenderGrid();
   void draw ();

   DECL_ERROR;

   Array<int> objs;
   Array<int> titles;
   Array<int> refAtoms;
   int titleOffset;
   int nColumns;
   int commentOffset;
   int comment;

private:
   void _drawComment();

   int nRows;
   float scale;
   Vec2f maxsz;
   Vec2f cellsz;
   Vec2f outerMargin;
   Vec2f maxTitleSize;
   Vec2f clientArea;
   Vec2f commentSize;
   int _width, _height;

   float _getScaleGivenSize (int w, int h);
   int _getDefaultWidth (const float s);
   int _getDefaultHeight (const float s);
};

}

#endif //__render_grid_h__
