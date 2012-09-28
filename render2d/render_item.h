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

   DECL_ERROR;

   virtual void estimateSize () = 0;
   virtual void setObjScale (float scale) = 0;
   virtual void init () = 0;
   virtual void render () = 0;
   virtual float getTotalBondLength () = 0;
   virtual float getTotalClosestAtomDistance() = 0;
   virtual int getBondCount () = 0;
   virtual int getAtomCount () = 0;

   Vec2f size;
   Vec2f origin;
   float referenceY;
protected:
   void renderIdle ();

   RenderItemFactory& _factory;
   RenderContext& _rc;
   const RenderSettings& _settings;
   const RenderOptions& _opt;
};

}

#endif //__render_item_h__
