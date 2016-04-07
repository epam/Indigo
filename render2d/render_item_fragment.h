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

#ifndef __render_item_fragment_h__
#define __render_item_fragment_h__

#include "render_item.h"

namespace indigo {
   
class RenderItemFactory;

class RenderItemFragment : public RenderItemBase {
public:
   RenderItemFragment (RenderItemFactory& factory);
   virtual ~RenderItemFragment ();

   DECL_ERROR;

   virtual void estimateSize ();
   virtual void setObjScale (float scale) { 
      _scaleFactor = scale;
   }
   virtual void init ();
   virtual void render ();
   virtual float getTotalBondLength ();
   virtual float getTotalClosestAtomDistance ();
   virtual int getBondCount ();
   virtual int getAtomCount ();

   BaseMolecule* mol;
   bool isRFragment;
   Array<int>* aam;
   Array<int>* reactingCenters;
   Array<int>* inversionArray;
   Array<int>* exactChangeArray;
   int refAtom;
   Vec2f refAtomPos;

private:
   float _scaleFactor;
   Vec2f _min, _max;
};

}

#endif //__render_item_fragment_h__
