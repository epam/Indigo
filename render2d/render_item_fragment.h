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

#ifndef __render_item_fragment_h__
#define __render_item_fragment_h__

#include "render_item.h"

namespace indigo {
   
class RenderItemFactory;

class RenderItemFragment : public RenderItemBase {
public:
   RenderItemFragment (RenderItemFactory& factory);
   virtual ~RenderItemFragment ();
   void setMolecule (BaseMolecule* mol);
   void setMoleculeHighlighting (GraphHighlighting* highlighting);
   void setAAM (Array<int>* aam);
   void setReactingCenters (Array<int>* reactingCenters);
   void setInversionArray (Array<int>* inversionArray);
   void setExactChangeArray (Array<int>* exactChangeArray);

   DEF_ERROR("RenderItemFragment");

   virtual void estimateSize () { renderIdle(); }
   virtual void setObjScale (float scale) { 
      _scaleFactor = scale; 
   }
   virtual void init () {}
   virtual void render ();
   virtual float getTotalBondLength ();
   virtual float getTotalClosestAtomDistance ();
   virtual int getBondCount ();
   virtual int getAtomCount ();

private:
   BaseMolecule* _mol;
   GraphHighlighting* _highlighting;
   Array<int>* _aam;
   Array<int>* _reactingCenters;
   Array<int>* _inversionArray;
   Array<int>* _exactChangeArray;
   float _scaleFactor;
   Vec2f _min, _max;
};

}

#endif //__render_item_fragment_h__
