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

#ifndef __render_item_aux_h__
#define __render_item_aux_h__

#include "render_item.h"

namespace indigo {

class RenderItemAuxiliary : public RenderItemBase {
public:
   enum AUX_TYPE {AUX_TEXT = 0, AUX_RXN_PLUS, AUX_RXN_ARROW, AUX_RGROUP_LABEL, AUX_RGROUP_IFTHEN};

   RenderItemAuxiliary (RenderItemFactory& factory);
   virtual ~RenderItemAuxiliary ();
   DEF_ERROR("RenderItemAuxiliary");

   virtual void estimateSize () { renderIdle(); }
   virtual void setObjScale (float scale) {}
   virtual void init () {}
   virtual void render ();
   virtual double getTotalBondLength () { return 0.0; }
   virtual double getTotalClosestAtomDistance () { return 0.0; }
   virtual int getBondCount ()  { return 0; }
   virtual int getAtomCount ()  { return 0; }

   AUX_TYPE type;
   Array<char> text;
   double fontsz;
   BaseMolecule* mol;
   int rLabelIdx;
private:
   void _drawText ();
};

}

#endif //__render_item_aux_h__
