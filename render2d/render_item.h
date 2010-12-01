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

#ifndef __render_item_h__
#define __render_item_h__

#include "render_context.h"

namespace indigo {

class RenderItemBase {
public:
   RenderItemBase (const RenderSettings& settings, RenderOptions& opt) : _settings(settings), _opt(opt)
   { // TODO: make RenderOptions const
   }
   virtual ~RenderItemBase ();

   DEF_ERROR("item render");

   virtual void estimateSize (Vec2f& sz, const float scale) = 0;
   virtual void render (RenderContext& context) = 0;
   virtual double getTotalBondLength () = 0;
   virtual double getTotalClosestAtomDistance() = 0;
   virtual int getBondCount () = 0;
   virtual int getAtomCount () = 0;

protected:
   const RenderSettings& _settings;
   RenderOptions& _opt;
};

}

#endif //__render_item_h__
