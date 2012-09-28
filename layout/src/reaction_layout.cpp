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

#include "molecule/molecule.h"
#include "reaction/reaction.h"
#include "layout/molecule_layout.h"
#include "layout/reaction_layout.h"

using namespace indigo;

ReactionLayout::ReactionLayout (BaseReaction& r) :
bond_length(1),
plus_interval_factor(4),
arrow_interval_factor(6),
preserve_molecule_layout(false),
_r(r)
{
   max_iterations = 0;
}

void ReactionLayout::make ()
{
   // update layout of molecules, if needed
   if (!preserve_molecule_layout)
   {
      for (int i = _r.begin(); i < _r.end(); i = _r.next(i))
      {
         MoleculeLayout molLayout(_r.getBaseMolecule(i));
         molLayout.max_iterations = max_iterations;
         molLayout.bond_length = bond_length;
         molLayout.make();
      }
   }

   // layout molecules in a row with the intervals specified
   Metalayout::LayoutLine& line = _ml.newLine();
   for (int i = _r.reactantBegin(); i < _r.reactantEnd(); i = _r.reactantNext(i))
   {  
      if (i != _r.reactantBegin())
         _pushSpace(line, plus_interval_factor);
      _pushMol(line, i);
   }
   _pushSpace(line, arrow_interval_factor);
   for (int i = _r.productBegin(); i < _r.productEnd(); i = _r.productNext(i))
   {  
      if (i != _r.productBegin())
         _pushSpace(line, plus_interval_factor);
      _pushMol(line, i);
   }

   _ml.bondLength = bond_length;
   _ml.cb_getMol = cb_getMol;
   _ml.cb_process = cb_process;
   _ml.context = this;
   _ml.prepare();
   _ml.scaleSz();
   _ml.calcContentSize();
   _ml.process();
}

Metalayout::LayoutItem& ReactionLayout::_pushMol (Metalayout::LayoutLine& line, int id)
{
   Metalayout::LayoutItem& item = line.items.push();
   item.type = 0;
   item.fragment = true;
   item.id = id;
   Metalayout::getBoundRect(item.min, item.max, _getMol(id));
   return item;
}

Metalayout::LayoutItem& ReactionLayout::_pushSpace (Metalayout::LayoutLine& line, float size)
{
   Metalayout::LayoutItem& item = line.items.push();
   item.type = 1;
   item.fragment = false;
   item.scaledSize.set(size, 0);
   return item;
}

BaseMolecule& ReactionLayout::_getMol (int id)
{
   return _r.getBaseMolecule(id);
}

BaseMolecule& ReactionLayout::cb_getMol (int id, void* context)
{
   return ((ReactionLayout*)context)->_getMol(id);
}  

void ReactionLayout::cb_process (Metalayout::LayoutItem& item, const Vec2f& pos, void* context)
{
   Vec2f pos2;
   pos2.copy(pos);
   pos2.y -= item.scaledSize.y / 2;
   if (item.fragment)
   {
      ReactionLayout* layout = (ReactionLayout*)context;
      layout->_ml.adjustMol(layout->_getMol(item.id), item.min, pos2);
   }
}

