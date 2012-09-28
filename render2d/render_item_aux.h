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

#ifndef __render_item_aux_h__
#define __render_item_aux_h__

#include "render_item.h"

namespace indigo {

class RenderItemAuxiliary : public RenderItemBase {
public:
   enum AUX_TYPE {AUX_COMMENT = 0, AUX_TITLE, AUX_RXN_PLUS, AUX_RXN_ARROW, AUX_RGROUP_LABEL, AUX_RGROUP_IFTHEN};

   RenderItemAuxiliary (RenderItemFactory& factory);
   virtual ~RenderItemAuxiliary ();
   DECL_ERROR;

   virtual void estimateSize () { renderIdle(); }
   virtual void setObjScale (float scale) {}
   virtual void init () {}
   virtual void render ();
   virtual float getTotalBondLength () { return 0.0f; }
   virtual float getTotalClosestAtomDistance () { return 0.0f; }
   virtual int getBondCount ()  { return 0; }
   virtual int getAtomCount ()  { return 0; }

   AUX_TYPE type;
   Array<char> text;
   BaseMolecule* mol;
   int rLabelIdx;
   float arrowLength;
private:
   void _drawRGroupLabel ();
   void _drawRIfThen ();
   void _drawText ();
   void _drawPlus ();
   void _drawArrow ();
};

}

#endif //__render_item_aux_h__
