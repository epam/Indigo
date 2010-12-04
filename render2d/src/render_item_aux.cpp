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

#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "render_context.h"
#include "render_internal.h"
#include "render_item_aux.h"

using namespace indigo;

RenderItemAuxiliary::RenderItemAuxiliary (RenderContext& rc) : 
   RenderItemBase(rc)
{
}

RenderItemAuxiliary::~RenderItemAuxiliary ()
{
}

void RenderItemAuxiliary::_drawText ()
{                                  
   TextItem ti;
   ti.text.copy(text);
   ti.fontsize = FONT_SIZE_COMMENT;
   Vec2f c;
   c.scaled(size, 0.5f);
   _rc.setTextItemSize(ti, c);
   _rc.drawTextItemText(ti);
}

void RenderItemAuxiliary::render ()
{
   //_rc.translate(-origin.x, -origin.y);
   // TODO: draw
   switch (type) {
      case AUX_TEXT:
         return;
      case AUX_RXN_PLUS:
         return;
      case AUX_RXN_ARROW:
         return;
      case AUX_RGROUP_LABEL:
         return;
      case AUX_RGROUP_IFTHEN:
         return;
      default:
         throw Error("Item type not set or invalid");
   }
   //MoleculeRenderInternal render(_opt, _settings, context);
   //render.setMolecule(_mol);
   //if (_highlighting != NULL)
   //   render.setHighlighting(_highlighting);
   //render.setScaleFactor(scaleFactor, _min, _max);
   //render.setReactionComponentProperties(_aam, _reactingCenters, _inversionArray);
   //render.setQueryReactionComponentProperties(_exactChangeArray);
   //render.render();
}