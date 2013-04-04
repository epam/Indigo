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

#ifndef __render_h__
#define __render_h__

#include "render_internal.h"

namespace indigo {

class Render {
public:
   Render (RenderContext& rc, RenderItemFactory& factory, const CanvasOptions& cnvOpt, int bondLength, bool bondLengthSet);
   virtual ~Render() = 0;

   DECL_ERROR;

protected:
   float _getObjScale (int item);
   int _getMaxWidth ();
   int _getMaxHeight ();
   float _getScale (int w, int h);
   virtual float _getScaleGivenSize (int w, int h) = 0;
   virtual int _getDefaultWidth (const float s) = 0;
   virtual int _getDefaultHeight (const float s) = 0;

   int minMarg;
   RenderContext& _rc;
   const RenderSettings& _settings;
   const CanvasOptions& _cnvOpt;
   const RenderOptions& _opt;
   RenderItemFactory& _factory;
   int _bondLength;
   bool _bondLengthSet;
};

}

#endif //__render_h__
