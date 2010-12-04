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

#ifndef __render_base_h__
#define __render_base_h__

#include "render_internal.h"

namespace indigo {

class Metalayout;

class RenderBase {
public:
   RenderBase (RenderContext& rc);
   virtual ~RenderBase() = 0;
   void draw ();
   RenderOptions opt;
   CanvasOptions cnvOpt;

   DEF_ERROR("molecule render base");                                                       
protected:
   enum ITEM_TYPE_BASE {ITEM_TYPE_BASE_MOL = 0, ITEM_TYPE_BASE_MAX};
   virtual void _initLayout () = 0;   
   virtual BaseMolecule& _getMol (int id) = 0;
   virtual void _drawItem (Metalayout::LayoutItem& item, const Vec2f& pos, bool ignoreTransform) = 0;
   void _setSize (Metalayout::LayoutItem& item);

   Metalayout::LayoutItem& _pushMol (Metalayout::LayoutLine& line, int type, int id, BaseMolecule& mol, bool catalyst = false);
   Metalayout::LayoutItem& _pushItem (Metalayout::LayoutLine& line, int type, int id = -1);

   Metalayout _ml;
   RenderContext& _rc;
   const RenderSettings& _settings;

private:
   static void cb_process (Metalayout::LayoutItem& item, const Vec2f& pos, void* context);
   static void cb_prepare (Metalayout::LayoutItem& item, const Vec2f& pos, void* context);
   static BaseMolecule& cb_getMol (int id, void* context);
   float _getScale (const Vec2f& delta, int absMargX, int absMargY);
};

}

#endif //__render_base_h__
