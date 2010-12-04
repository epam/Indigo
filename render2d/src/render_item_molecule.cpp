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
#include "render_item_molecule.h"

using namespace indigo;

RenderItemMolecule::RenderItemMolecule (RenderContext& rc) : 
   RenderItemContainer(rc),
   _mol(NULL),
   _highlighting(NULL),
   _core(NULL)
{
}

void RenderItemMolecule::init ()
{
   if (_mol == NULL)
      throw Error("molecule not set");

   if (_mol->vertexCount() == 0)
      return;

   _core = _fragments.add(_rc);
   _fragments[_core].setMolecule(_mol);
   _fragments[_core].setMoleculeHighlighting(_highlighting);

   QUERY_MOL_BEGIN;
   {
      MoleculeRGroups& rGroups = qmol.rgroups;
      if (_getRIfThenCount() > 0) {
         _ifThen = _aux.add(_rc);
         _aux[_ifThen].type = RenderItemAuxiliary::AUX_RGROUP_IFTHEN;
         _aux[_ifThen].mol = _mol;
      }
      for (int i = 1; i <= rGroups.getRGroupCount(); ++i)
      {
         RGroup& rg = rGroups.getRGroup(i);
         Array<int>& line = _rGroupFragments.push();
         int label = _aux.add(_rc);
         _aux[label].type = RenderItemAuxiliary::AUX_RGROUP_LABEL;
         _aux[label].rLabelIdx = i;
         _rGroupLabels.push(label);

         for (int j = 0; j < rg.fragmentsCount(); ++j) {
            int id = _fragments.add(_rc);
            _fragments[id].setMolecule(rg.fragments[j]);
            line.push(id);
         }
      }
   }
   QUERY_MOL_END;
   for (int i = 0; i < _fragments.size(); ++i)
      _items.push(&_fragments[i]);
   for (int i = 0; i < _aux.size(); ++i)
      _items.push(&_aux[i]);
}

int RenderItemMolecule::_getRIfThenCount ()
{
   QUERY_MOL_BEGIN;
   {
      MoleculeRGroups& rgs = qmol.rgroups;
      int cnt = 0;
      for (int i = 1; i <= rgs.getRGroupCount(); ++i)
         if (rgs.getRGroup(i).if_then > 0)
            ++cnt;
      return cnt;
   }
   QUERY_MOL_END;
   throw Error("internal: _getRIfThenCount()");
}


//Metalayout::LayoutItem& RenderItemMolecule::_pushMol (Metalayout::LayoutLine& line, BaseMolecule& mol)
//{  
//   Metalayout::LayoutItem& item = RenderBase::_pushMol(line, ITEM_TYPE_BASE_MOL, _map.size(), mol);
//   _map.push(&mol);
//   return item;   
//}
//
//Metalayout::LayoutItem& RenderItemMolecule::_pushSymbol (Metalayout::LayoutLine& line, int type, int id)
//{
//   Metalayout::LayoutItem& item = RenderBase::_pushItem(line, type, id);
//   switch (type)
//   {
//   case ITEM_TYPE_MOL_RLABEL:
//      item.size.set(1, _settings.fzz[FONT_SIZE_RGROUP_LOGIC]);
//      break;
//   case ITEM_TYPE_MOL_RIFTHEN:
//      item.size.set(1, _getRIfThenCount() *
//         (_settings.fzz[FONT_SIZE_RGROUP_LOGIC] + _settings.rGroupIfThenInterval) - _settings.rGroupIfThenInterval - 
//         _settings.layoutMarginHorizontal / 2/* dirty hack to reduce the gap */);
//      break;
//   default:
//      throw Error("unknown layout item type: %d", type);
//   }
//
//   return item;
//}

void RenderItemMolecule::estimateSize ()
{
   RenderItemContainer::estimateSize();
   float vSpace = _settings.layoutMarginVertical;
   float hSpace = _settings.layoutMarginHorizontal;
   origin.set(0, 0);
   size.copy(_fragments[_core].size);

   if (_ifThen >= 0) {
      RenderItemAuxiliary& ifThenItem = _aux[_ifThen];
      size.y += vSpace + ifThenItem.size.y;
      size.x = __max(size.x, ifThenItem.size.x);
   }

   for (int i = 0; i < _rGroupFragments.size(); ++i) {
      size.y += vSpace;
      RenderItemAuxiliary& label = _aux[_rGroupLabels[i]];
      float lineHeight = label.size.y;
      float lineWidth = label.size.x;
      for (int j = 0; j < _rGroupFragments[i].size(); ++j) {
         RenderItemFragment& frag = _fragments[_rGroupFragments[i][j]];
         lineHeight = __max(lineHeight, frag.size.y);
         lineWidth += hSpace + frag.size.x;
      }
      size.y += lineHeight;
      size.x = __max(size.x, lineWidth);
   }
}

void RenderItemMolecule::render ()
{                                     
   _rc.translate(-origin.x, -origin.y);
   float vSpace = _settings.layoutMarginVertical;
   float hSpace = _settings.layoutMarginHorizontal;
   _fragments[_core].render();
   _rc.translate(0, _fragments[_core].size.y);

   if (_ifThen >= 0) {
      RenderItemAuxiliary& ifThenItem = _aux[_ifThen];
      _rc.translate(0, vSpace);
      ifThenItem.render();
      _rc.translate(0, ifThenItem.size.y);
   }

   for (int i = 0; i < _rGroupFragments.size(); ++i) {
      _rc.translate(0, vSpace);
      RenderItemAuxiliary& label = _aux[_rGroupLabels[i]];
      float lineHeight = label.size.y;
      for (int j = 0; j < _rGroupFragments[i].size(); ++j) {
         RenderItemFragment& frag = _fragments[_rGroupFragments[i][j]];
         lineHeight = __max(lineHeight, frag.size.y);
      }
      _rc.storeTransform();
      _rc.translate(0, lineHeight - label.size.y);
      label.render();
      _rc.restoreTransform();
      _rc.removeStoredTransform();
      _rc.translate(label.size.x, 0);
      for (int j = 0; j < _rGroupFragments[i].size(); ++j) {
         _rc.translate(hSpace, 0);
         _rc.storeTransform();
         RenderItemFragment& frag = _fragments[_rGroupFragments[i][j]];
         _rc.translate(0, lineHeight - frag.size.y);
         frag.render();
         _rc.restoreTransform();
         _rc.removeStoredTransform();
         _rc.translate(frag.size.x, 0);
      }
      _rc.translate(0, lineHeight);
   }

   for (int i = 0; i < _rGroupFragments.size(); ++i) {
      for (int j = 0; j < _rGroupFragments[i].size(); ++j) {
         _fragments[_rGroupFragments[i][j]].render();
      }
   }
}