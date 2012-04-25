/****************************************************************************
 * Copyright (C) 2010-2011 GGA Software Services LLC
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

#ifndef __indigo_renderer_internal__
#define __indigo_renderer_internal__

#include "render2d/render_params.h"
#include "base_cpp/tlscont.h"
#include "indigo_internal.h"

class IndigoRenderer
{
public:
   IndigoRenderer ();
   ~IndigoRenderer ();

   RenderParams renderParams;
   
protected:
};

class IndigoHDCOutput : public IndigoObject
{
public:
   enum
   { HDC_OUTPUT = 110 };


   IndigoHDCOutput (void* hdc, bool printing)  : IndigoObject(HDC_OUTPUT), dc(hdc), prn(printing) {}
   void* dc;
   bool prn;

   virtual ~IndigoHDCOutput () {}

protected:
   RenderParams params;
};


//TL_DECL_EXT(IndigoRenderer, indigo_renderer_self);

#endif
