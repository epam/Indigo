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
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "render_context.h"
#include "render_molecule.h"

using namespace indigo;

MoleculeRender::MoleculeRender (RenderContext& rc) : RenderBase(rc), _mol(NULL)
{
}

MoleculeRender::~MoleculeRender ()
{
}

void MoleculeRender::setMolecule (BaseMolecule* mol)
{
   _mol = mol;
}

void MoleculeRender::setMoleculeHighlighting (const GraphHighlighting* highlighting)
{
   _highlighting = highlighting;
}

void MoleculeRender::_initLayout ()
{
   if (_mol == NULL)
      throw Error("molecule not set");

   if (_mol->vertexCount() == 0)
      return;

   _ml.context = this;
   _ml.cb_process = cb_process;
   _ml.cb_getMol = cb_getMol;
   
   _map.clear();
   _pushMol(_ml.newLine(), *_mol);

   QUERY_MOL_BEGIN;
   {
      MoleculeRGroups& rGroups = qmol.rgroups;
      if (_getRIfThenHeight() > 0)
         _pushSymbol(_ml.newLine(), ITEM_TYPE_MOL_RIFTHEN);
      for (int i = 1; i < rGroups.getRGroupCount(); ++i)
      {
         RGroup& rg = rGroups.getRGroup(i);
         Metalayout::LayoutLine& line = _ml.newLine();
         _pushSymbol(line, ITEM_TYPE_MOL_RLABEL, i);

         for (int j = 0; j < rg.fragmentsCount(); ++j)
            _pushMol(line, *rg.fragments[j]);
      }
   }
   QUERY_MOL_END;
}

Metalayout::LayoutItem& MoleculeRender::_pushMol (Metalayout::LayoutLine& line, BaseMolecule& mol)
{  
   Metalayout::LayoutItem& item = RenderBase::_pushMol(line, ITEM_TYPE_BASE_MOL, _map.size(), mol);
   _map.push(&mol);
   return item;   
}

Metalayout::LayoutItem& MoleculeRender::_pushSymbol (Metalayout::LayoutLine& line, int type, int id)
{
   Metalayout::LayoutItem& item = RenderBase::_pushItem(line, type, id);
   switch (type)
   {
   case ITEM_TYPE_MOL_RLABEL:
      item.scaledSize.set(1, _settings.fzz[FONT_SIZE_RGROUP_LOGIC]);
      break;
   case ITEM_TYPE_MOL_RIFTHEN:
      item.scaledSize.set(1, _getRIfThenHeight() *
         (_settings.fzz[FONT_SIZE_RGROUP_LOGIC] + _settings.rGroupIfThenInterval) - _settings.rGroupIfThenInterval - 
         _settings.layoutMarginHorizontal / 2/* dirty hack to reduce the gap */);
      break;
   default:
      throw Error("unknown layout item type: %d", type);
   }

   return item;
}

BaseMolecule& MoleculeRender::_getMol (int id)
{
   return *_map[id];
}

void MoleculeRender::_drawMol (const Metalayout::LayoutItem& item)
{
   //_rc.setSingleSource(CWC_WHITE);
   //_rc.drawRectangle(Vec2f(), item.scaledSize);
   _rc.translate(-item.scaledOffset.x, -item.scaledOffset.y);
   BaseMolecule& mol = _getMol(item.id);
   MoleculeRenderInternal render(opt, _settings, _rc);
   render.setMolecule(&mol);
   if (item.id == 0)
      render.setHighlighting(_highlighting);
   render.setScaleFactor(_ml.getScaleFactor(), item.min, item.max);
   render.render();   
}

void MoleculeRender::_drawRGroupLabel (const Metalayout::LayoutItem& item)
{
   QUERY_MOL_BEGIN;
   {
      MoleculeRGroups& rgs = qmol.rgroups;
      RGroup& rg = rgs.getRGroup(item.id);

      TextItem tiR;
      tiR.fontsize = FONT_SIZE_LABEL;
      tiR.color = CWC_BASE;
      bprintf(tiR.text, "R%d=", item.id); 
      _rc.setTextItemSize(tiR);
      tiR.bbp.set(0,0);
      _rc.drawTextItemText(tiR);

      float ypos = tiR.bbp.y + tiR.bbsz.y + _settings.bondLineWidth;

      if (rg.occurrence.size() > 0)
      {
         TextItem tiOccurrence;
         tiOccurrence.fontsize = FONT_SIZE_RGROUP_LOGIC_INDEX;
         tiOccurrence.color = CWC_BASE;
         ArrayOutput output(tiOccurrence.text);
         for (int i = 0; i < rg.occurrence.size(); ++i)
         {
            int v = rg.occurrence[i];
            int a = (v >> 16) & 0xFFFF;
            int b = v & 0xFFFF;
            if (i > 0)
               output.printf(", ");
            if (a == b)
               output.printf("%d", a);
            else if (a == 0)
               output.printf("<%d", b+1);
            else if (b == 0xFFFF)
               output.printf(">%d", a-1);
            else
               output.printf("%d-%d", a, b);
         }
         output.writeByte(0);

         _rc.setTextItemSize(tiOccurrence);
         tiOccurrence.bbp.set(0, ypos);
         _rc.drawTextItemText(tiOccurrence);

         ypos += tiOccurrence.bbsz.y + _settings.bondLineWidth;
      }

      if (rg.rest_h > 0)
      {
         TextItem tiRestH;
         tiRestH.fontsize = FONT_SIZE_RGROUP_LOGIC_INDEX;
         tiRestH.color = CWC_BASE;
         bprintf(tiRestH.text, "RestH");

         _rc.setTextItemSize(tiRestH);
         tiRestH.bbp.set(0, ypos);
         _rc.drawTextItemText(tiRestH);
      }
   }
   QUERY_MOL_END;
}

void MoleculeRender::_drawRIfThen (const Metalayout::LayoutItem& item)
{
   QUERY_MOL_BEGIN;
   {
      MoleculeRGroups& rgs = qmol.rgroups;

      float ypos = 0;
      for (int i = 1; i <= rgs.getRGroupCount(); ++i)
      {
         const RGroup& rg = rgs.getRGroup(i);
         if (rg.if_then > 0)
         {
            TextItem tiIfThen;
            tiIfThen.fontsize = FONT_SIZE_RGROUP_LOGIC;
            tiIfThen.color = CWC_BASE;
            bprintf(tiIfThen.text, "IF R%d THEN R%d", i, rg.if_then); 
            _rc.setTextItemSize(tiIfThen);
            tiIfThen.bbp.set(0, ypos);
            _rc.drawTextItemText(tiIfThen);

            ypos += tiIfThen.bbsz.y + _settings.rGroupIfThenInterval;
         }
      }
   }
   QUERY_MOL_END;
}

int MoleculeRender::_getRIfThenHeight ()
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
   throw Error("internal: _getRIfThenHeight()");
}

void MoleculeRender::cb_process (Metalayout::LayoutItem& item, const Vec2f& pos, void* context)
{
   MoleculeRender* render = (MoleculeRender*)context;
   render->_rc.restoreTransform();
   render->_rc.translate(pos.x, pos.y - item.scaledSize.y / 2);

   switch (item.type)
   {
   case ITEM_TYPE_BASE_MOL:  
      render->_drawMol(item);
      break;
   case ITEM_TYPE_MOL_RLABEL:
      render->_drawRGroupLabel(item);
      break;
   case ITEM_TYPE_MOL_RIFTHEN:
      render->_drawRIfThen(item);
      break;
   default:
      RenderBase::cb_process(item, pos, context);
   }
}