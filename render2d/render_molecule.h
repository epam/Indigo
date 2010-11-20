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

#ifndef __render_molecule_h__
#define __render_molecule_h__

#include "render_base.h"

namespace indigo {

class MoleculeRender : public RenderBase {
public:
   MoleculeRender (RenderContext& rc);
   virtual ~MoleculeRender ();
   void setMolecule (BaseMolecule* mol);
   void setMoleculeHighlighting (const GraphHighlighting* highlighting);

   DEF_ERROR("molecule render");

protected:
   enum ITEM_TYPE_MOL {ITEM_TYPE_MOL_RLABEL = ITEM_TYPE_BASE_MAX, ITEM_TYPE_MOL_RIFTHEN, ITEM_TYPE_MOL_MAX};

   virtual void _initLayout ();
   virtual void _drawItem (Metalayout::LayoutItem& item, const Vec2f& pos, bool ignoreTransform);
   virtual BaseMolecule& _getMol (int id);

private:
   Metalayout::LayoutItem& _pushMol (Metalayout::LayoutLine& line, BaseMolecule& mol);
   Metalayout::LayoutItem& _pushSymbol (Metalayout::LayoutLine& line, int type, int idx = -1);

   void _drawMol (Metalayout::LayoutItem& item);
   void _drawRGroupLabel (Metalayout::LayoutItem& item);
   void _drawRIfThen (Metalayout::LayoutItem& item);
   int _getRIfThenHeight ();

   BaseMolecule* _mol;
   Array<BaseMolecule*> _map; 
   const GraphHighlighting* _highlighting;
};

}

#endif //__render_molecule_h__
