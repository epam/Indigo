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

#ifndef __render_item_molecule_h__
#define __render_item_molecule_h__

#include "render_item_fragment.h"
#include "render_item_aux.h"
#include "render_item_hline.h"

namespace indigo {

class RenderItemMolecule : public RenderItemContainer {
public:
   RenderItemMolecule (RenderItemFactory& factory);
   virtual ~RenderItemMolecule () {}

   DECL_ERROR;

   virtual void init ();
   virtual void estimateSize ();
   virtual void render ();

   BaseMolecule* mol;
   int refAtom;
   Vec2f refAtomPos;
private:
   int _core;
   int _getRIfThenCount ();
};

}

#endif //__render_item_molecule_h__
