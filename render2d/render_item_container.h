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

#ifndef __render_item_container_h__
#define __render_item_container_h__

#include "render_item.h"

namespace indigo {

class RenderItemContainer : public RenderItemBase {
public:
   RenderItemContainer (RenderContext& rc);
   virtual ~RenderItemContainer () {}

   DEF_ERROR("RenderItemContainer");

   virtual void estimateSize ();
   virtual void setObjScale (float scale);
   virtual double getTotalBondLength ();
   virtual double getTotalClosestAtomDistance ();
   virtual int getBondCount ();
   virtual int getAtomCount ();

protected:
   Array<RenderItemBase*> _items;
};

}

#endif //__render_item_container_h__
