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

#ifndef __render_single_h__
#define __render_single_h__

#include "render.h"

namespace indigo {

class RenderSingle : Render {
public:
   RenderSingle (RenderContext& rc, RenderItemFactory& factory, const CanvasOptions& cnvOpt, int bondLength, bool bondLengthSet);
   virtual ~RenderSingle();
   void draw ();

   DECL_ERROR;

   int obj;
   int comment;
   float scale;
   int commentOffset;
   Vec2f objSize;
   Vec2f commentSize;
   Vec2f outerMargin;
   Vec2f objArea;
   int width, height;
private:
   float _getScaleGivenSize (int w, int h);
   int _getDefaultWidth (const float s);
   int _getDefaultHeight (const float s);
   void _drawComment ();
   void _drawObj ();
};

}

#endif //__render_single_h__
