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

#ifndef __render_item_hline_h__
#define __render_item_hline_h__

#include "render_item_container.h"

namespace indigo {

class RenderItemHLine : public RenderItemContainer {
public:
   RenderItemHLine (RenderItemFactory& factory);
   virtual ~RenderItemHLine () {}

   DECL_ERROR;

   virtual void init ();
   virtual void estimateSize ();
   virtual void render ();

   float hSpace;
};

}

#endif //__render_item_hline_h__
