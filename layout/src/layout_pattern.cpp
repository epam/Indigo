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

#include "base_cpp/tlscont.h"
#include "graph/morgan_code.h"
#include "layout/layout_pattern.h"

using namespace indigo;

IMPL_ERROR(PatternLayout, "molecule");

PatternLayout::PatternLayout () : _morgan_code(0), _fixed(false)
{
}

PatternLayout::~PatternLayout ()
{
}

int PatternLayout::addAtom (float x, float y)
{
   int idx = addVertex();

   _atoms.expand(idx + 1);

   _atoms[idx].pos.set(x, y);

   return idx;
}

int PatternLayout::addBond (int atom_beg, int atom_end, int type)
{
   int idx = addEdge(atom_beg, atom_end);

   _bonds.expand(idx + 1);
   _bonds[idx].type = type;

   return idx;
}

int PatternLayout::addOutlinePoint (float x, float y)
{
   Vec2f &p = _outline.push();

   p.set(x, y);

   return _outline.size() - 1;
}


const PatternAtom & PatternLayout::getAtom (int idx) const
{
   return _atoms[idx];
}

const PatternBond & PatternLayout::getBond (int idx) const
{
   return _bonds[idx];
}

void PatternLayout::calcMorganCode ()
{
   MorganCode morgan(*this);
   QS_DEF(Array<long>, morgan_codes);

   morgan.calculate(morgan_codes, 3, 7);

   _morgan_code = 0;

   for (int i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
      _morgan_code += morgan_codes[i];
}
