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

#ifndef __render_item_column_h__
#define __render_item_column_h__

#include "render_item_container.h"

namespace indigo {

class RenderItemColumn : public RenderItemContainer {
public:
   RenderItemColumn (RenderItemFactory& factory);
   virtual ~RenderItemColumn () {}

   DECL_ERROR;

   virtual void init ();
   void setVerticalSpacing(float spacing);
   void setAlignment(MultilineTextLayout::Alignment alignment);
   virtual void estimateSize ();
   virtual void render ();

private:
   float vSpace;
   MultilineTextLayout::Alignment alignment;
};

}

#endif //__render_item_column_h__
