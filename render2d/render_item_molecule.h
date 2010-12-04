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

#ifndef __render_item_molecule_h__
#define __render_item_molecule_h__

#include "render_item_fragment.h"
#include "render_item_aux.h"
#include "render_item_container.h"

namespace indigo {

class RenderItemMolecule : public RenderItemContainer {
public:
   RenderItemMolecule (RenderContext& rc);
   virtual ~RenderItemMolecule () {}
   void setMolecule (BaseMolecule* mol) { _mol = mol; }
   void setMoleculeHighlighting (GraphHighlighting* highlighting) { _highlighting = highlighting; }

   DEF_ERROR("RenderItemMolecule");

   virtual void init ();
   virtual void estimateSize ();
   virtual void render ();

private:
   int _getRIfThenCount ();

   ObjPool<RenderItemFragment> _fragments;
   ObjPool<RenderItemAuxiliary> _aux;
   int _core, _ifThen;
   ObjArray<Array<int>> _rGroupFragments;
   Array<int> _rGroupLabels;

   BaseMolecule* _mol;
   GraphHighlighting* _highlighting;
};

}

#endif //__render_item_molecule_h__
