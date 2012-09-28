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

#include "molecule/base_molecule.h"
#include "base_cpp/output.h"
#include "render_item.h"
#include "render_item_factory.h"

using namespace indigo;

IMPL_ERROR(RenderItemBase, "RenderItemBase");

RenderItemBase::RenderItemBase (RenderItemFactory& factory) : referenceY(0),
        _factory(factory),
        _rc(factory.rc), _settings(factory.rc._settings), _opt(factory.rc.opt)
{
}

void RenderItemBase::renderIdle ()
{
   _rc.initNullContext();
   Vec2f bbmin, bbmax;
   Vec2f pos;
   render();
   _rc.bbGetMin(bbmin);
   _rc.bbGetMax(bbmax);
   _rc.closeContext(true);
   size.diff(bbmax, bbmin);
   origin.copy(bbmin);
}
