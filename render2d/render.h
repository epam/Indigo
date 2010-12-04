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

#ifndef __render_h__
#define __render_h__

#include "render_internal.h"

namespace indigo {

class Render {
public:
   Render (RenderContext& rc, RenderItemBase& item);
   virtual ~Render();
   void draw ();

   DEF_ERROR("Render");

private:
   float _getScale (const Vec2f& delta, int absMargX, int absMargY);

   RenderItemBase& _item;
   RenderContext& _rc;
   const RenderSettings& _settings;
   CanvasOptions& _cnvOpt;
};

}

#endif //__render_h__
