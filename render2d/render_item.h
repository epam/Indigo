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

class RenderItemFactory;

class RenderItemBase {
public:
   RenderItemBase (RenderItemFactory& factory);

   virtual ~RenderItemBase ()
   {
   }

   DEF_ERROR("RenderItemBase");

   virtual void estimateSize () = 0;
   virtual void setObjScale (float scale) = 0;
   virtual void init () = 0;
   virtual void render () = 0;
   virtual double getTotalBondLength () = 0;
   virtual double getTotalClosestAtomDistance() = 0;
   virtual int getBondCount () = 0;
   virtual int getAtomCount () = 0;

   Vec2f size;
   Vec2f origin;
protected:
   void renderIdle ();

   const RenderSettings& _settings;
   RenderOptions& _opt;
   RenderContext& _rc;
   RenderItemFactory& _factory;
};

}

#endif //__render_item_h__
