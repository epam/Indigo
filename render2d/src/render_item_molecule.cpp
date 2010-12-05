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
#include "render_item_factory.h"

using namespace indigo;

RenderItemMolecule::RenderItemMolecule (RenderItemFactory& factory) : 
   RenderItemContainer(factory),
   mol(NULL),
   highlighting(NULL)
{
}

void RenderItemMolecule::init ()
{
   if (mol == NULL)
      throw Error("molecule not set");

   if (mol->vertexCount() == 0)
      return;

   int _core = _factory.addItemFragment();
   _factory.getItemFragment(_core).mol = mol;
   _factory.getItemFragment(_core).highlighting = highlighting;
   _factory.getItemFragment(_core).init();
   
   int lineCore = _factory.addItemHLine();
   _factory.getItemHLine(lineCore).init();
   _factory.getItemHLine(lineCore).items.push(_core);
   items.push(lineCore);

   QUERY_MOL_BEGIN(mol);
   {
      MoleculeRGroups& rGroups = qmol.rgroups;
      if (_getRIfThenCount() > 0) {
         int _ifThen = _factory.addItemAuxiliary();
         _factory.getItemAuxiliary(_ifThen).type = RenderItemAuxiliary::AUX_RGROUP_IFTHEN;
         _factory.getItemAuxiliary(_ifThen).mol = mol;
         int lineIfThen = _factory.addItemHLine();
         _factory.getItemHLine(lineIfThen).init();
         _factory.getItemHLine(lineIfThen).items.push(_ifThen);
         _factory.getItemAuxiliary(_ifThen).init();
         items.push(_ifThen);
      }
      for (int i = 1; i <= rGroups.getRGroupCount(); ++i)
      {
         int lineRFrag = _factory.addItemHLine();
         _factory.getItemHLine(lineRFrag).init();
         items.push(lineRFrag);

         RGroup& rg = rGroups.getRGroup(i);
         int label = _factory.addItemAuxiliary();
         _factory.getItemAuxiliary(label).type = RenderItemAuxiliary::AUX_RGROUP_LABEL;
         _factory.getItemAuxiliary(label).mol = mol;
         _factory.getItemAuxiliary(label).rLabelIdx = i;
         _factory.getItemHLine(lineRFrag).items.push(label);
         _factory.getItemAuxiliary(label).init();

         for (int j = 0; j < rg.fragmentsCount(); ++j) {
            int id = _factory.addItemFragment();
            _factory.getItemFragment(id).mol = rg.fragments[j];
            _factory.getItemFragment(id).init();
            _factory.getItemHLine(lineRFrag).items.push(id);
         }
      }
   }
   QUERY_MOL_END;
}

int RenderItemMolecule::_getRIfThenCount ()
{
   QUERY_MOL_BEGIN(mol);
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

void RenderItemMolecule::estimateSize ()
{
   RenderItemContainer::estimateSize();
   origin.set(0, 0);
   size.set(0, 0);

   float vSpace = _settings.layoutMarginVertical;
   for (int i = 0; i < items.size(); ++i) {
      RenderItemHLine& line = _factory.getItemHLine(items[i]);
      size.x = __max(size.x, line.size.x);
      if (i > 0)
         size.y += vSpace;
      size.y += line.size.y;
   }
}

void RenderItemMolecule::render ()
{                                     
   _rc.translate(-origin.x, -origin.y);
   float vSpace = _settings.layoutMarginVertical;
   for (int i = 0; i < items.size(); ++i) {
      RenderItemHLine& line = _factory.getItemHLine(items[i]);
      line.render();
      _rc.translate(0, line.size.y + vSpace);
   }
}