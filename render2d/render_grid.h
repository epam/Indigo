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

#ifndef __render_grid_h__
#define __render_grid_h__

#include "render.h"

namespace indigo {

class RenderGrid : Render {
public:
   RenderGrid (RenderContext& rc, RenderItemFactory& factory);
   virtual ~RenderGrid();
   void draw ();

   DEF_ERROR("RenderGrid");

   Array<int> objs;
   Array<int> refAtoms;
   Array<int> comments;
   float scale;
   float commentOffset;
   int nColumns;
   Vec2f clientArea;
   Vec2f maxsz;
   Vec2f totalsz;
   Vec2f outerMargin;
private:
   float _getScale ();
   void _drawComment ();
   void _drawObj ();
};

}

#endif //__render_grid_h__
