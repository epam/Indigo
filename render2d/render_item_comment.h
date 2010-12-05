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

#ifndef __render_item_comment_h__
#define __render_item_comment_h__

#include "render_item_container.h"

namespace indigo {

class RenderItemComment : public RenderItemContainer {
public:
   RenderItemComment (RenderItemFactory& factory);
   virtual ~RenderItemComment () {}

   DEF_ERROR("RenderItemComment");

   virtual void init ();
   virtual void estimateSize ();
   virtual void render ();

   int obj;
   int comment;
   float vSpace;

private:
   void _renderComment ();
   void _renderObj ();
};

}

#endif //__render_item_comment_h__
