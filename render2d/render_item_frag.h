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

#ifndef __render_component_h__
#define __render_component_h__

#include "render_item.h"

namespace indigo {

class RenderItemFragment : public RenderItemBase {
public:
   RenderItemFragment (const RenderSettings& settings, RenderOptions& opt);
   virtual ~RenderItemFragment ();
   void setMolecule (BaseMolecule* mol);
   void setMoleculeHighlighting (GraphHighlighting* highlighting);
   void setAAM (Array<int>* aam);
   void setReactingCenters (Array<int>* reactingCenters);
   void setInversionArray (Array<int>* inversionArray);
   void setExactChangeArray (Array<int>* exactChangeArray);

   DEF_ERROR("molecule render");

   virtual void estimateSize (Vec2f& sz, const float scaleFactor);
   virtual void render (RenderContext& context, const float scaleFactor);
   virtual double getTotalBondLength ();
   virtual double getTotalClosestAtomDistance ();
   virtual int getBondCount ();
   virtual int getAtomCount ();

protected:
   virtual void _draw (RenderContext& context);

private:
   BaseMolecule* _mol;
   GraphHighlighting* _highlighting;
   Array<int>* _aam;
   Array<int>* _reactingCenters;
   Array<int>* _inversionArray;
   Array<int>* _exactChangeArray;
   Vec2f _min, _max;
};

}

#endif //__render_component_h__
