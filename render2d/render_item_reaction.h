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

#ifndef __render_item_reaction_h__
#define __render_item_reaction_h__

#include "render_item_fragment.h"
#include "render_item_aux.h"
#include "render_item_hline.h"

namespace indigo {

class RenderItemReaction : public RenderItemContainer {
public:
   RenderItemReaction (RenderItemFactory& factory);
   virtual ~RenderItemReaction () {}

   DECL_ERROR;

   virtual void init ();
   virtual void estimateSize ();
   virtual void render ();

   BaseReaction* rxn;
   float hSpace, catalystOffset;
private:
   int _addFragment (int id);
   int _addPlus ();
   int _reactantLine, _catalystLineUpper, _catalystLineLower, _productLine, _arrow;
   bool _splitCatalysts;
   float _arrowWidth;
};

}

#endif //__render_item_reaction_h__
