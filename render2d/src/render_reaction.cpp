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

#include "base_cpp/output.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "reaction/reaction.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction_highlighting.h"
#include "layout/metalayout.h"
#include "render_context.h"
#include "render_reaction.h"

ReactionRender::ReactionRender (RenderContext& rc) : RenderBase(rc), _highlighting(NULL), _r(NULL)
{

}

ReactionRender::~ReactionRender ()
{

}

void ReactionRender::setReaction (BaseReaction* r)
{
   _r = r;
}

void ReactionRender::setReactionHighlighting (ReactionHighlighting* highlighting)
{
   _highlighting = highlighting;
}

Metalayout::LayoutItem& ReactionRender::_pushMol (Metalayout::LayoutLine& line, int id, bool catalyst)
{
   return RenderBase::_pushMol(line, ITEM_TYPE_BASE_MOL, id, _r->getBaseMolecule(id), catalyst);
}

Metalayout::LayoutItem& ReactionRender::_pushSpace (Metalayout::LayoutLine& line, float size)
{
   Metalayout::LayoutItem& item = line.items.push();
   item.type = ITEM_TYPE_RXN_SPACE;
   item.fragment = false;
   item.scaledSize.set(size, 0);
   return item;
}

Metalayout::LayoutItem& ReactionRender::_pushSymbol (Metalayout::LayoutLine& line, int type)
{
   Metalayout::LayoutItem& item = RenderBase::_pushItem(line, type, -1);
   switch (type)
   {
   case ITEM_TYPE_RXN_PLUS:
      item.scaledSize.set(_settings.plusSize, _settings.plusSize);
      break;
   case ITEM_TYPE_RXN_ARROW:
      item.scaledSize.set(_settings.arrowLength, _settings.arrowHeadWidth);
      break;
   case ITEM_TYPE_RXN_BEGIN_ARROW:
      item.scaledSize.set(0, 0);
      break;
   case ITEM_TYPE_RXN_END_ARROW:
      item.scaledSize.set(0, 0);
      break;
   default:
      throw Error("unknown layout item type: %d", type);
   }
   return item;
}

void ReactionRender::_initLayout ()
{
   if (_r == NULL)
      throw Error("reaction not set");

   if (_r->begin() >= _r->end()) // no reactants or products
      return;

   _ml.context = this;
   _ml.cb_process = cb_process;
   _ml.cb_getMol = cb_getMol;

   Metalayout::LayoutLine& line = _ml.newLine();

   for (int i = _r->reactantBegin(); i < _r->reactantEnd(); i = _r->reactantNext(i))
   {
      if (i > _r->reactantBegin())
         _pushSymbol(line, ITEM_TYPE_RXN_PLUS);
      _pushMol(line, i);
   }
   if (_r->catalystCount() > 0)
   {
      _pushSymbol(line, ITEM_TYPE_RXN_BEGIN_ARROW);
      for (int i = _r->catalystBegin(); i < _r->catalystEnd(); i = _r->catalystNext(i)) {
         if (i != _r->catalystBegin())
            _pushSpace(line, 0.0f);
         _pushMol(line, i, true);
      }
      _pushSymbol(line, ITEM_TYPE_RXN_END_ARROW);
   }
   else
      _pushSymbol(line, ITEM_TYPE_RXN_ARROW);

   for (int i = _r->productBegin(); i < _r->productEnd(); i = _r->productNext(i))
   {
      if (i > _r->productBegin())
         _pushSymbol(line, ITEM_TYPE_RXN_PLUS);
      _pushMol(line, i);
   }
}

BaseMolecule& ReactionRender::_getMol (int id)
{
   return _r->getBaseMolecule(id);
}

void ReactionRender::_drawMol (const Metalayout::LayoutItem& item)
{
   _rc.translate(0, -item.scaledSize.y / 2);
   //_rc.setSingleSource(CWC_WHITE);
   //_rc.drawRectangle(Vec2f(), item.scaledSize);
   _rc.translate(-item.scaledOffset.x, -item.scaledOffset.y);
   MoleculeRenderInternal render(opt, _settings, _rc);
   BaseMolecule& mol = _r->getBaseMolecule(item.id);
   render.setMolecule(&mol);
   if (_highlighting != NULL && item.id < _highlighting->getCount())
      render.setHighlighting(&_highlighting->getGraphHighlighting(item.id));
   render.setScaleFactor(_ml.getScaleFactor(), item.min, item.max);
   render.setReactionComponentProperties(
      _r->getAAMArray(item.id),
      _r->getReactingCenterArray(item.id),
      _r->getInversionArray(item.id));
   QUERY_RXN_BEGIN;
   render.setQueryReactionComponentProperties(qr.getExactChangeArray(item.id));
   QUERY_RXN_END;
   render.render();
}

void ReactionRender::_drawPlus ()
{
   _rc.setSingleSource(CWC_BASE);
   _rc.drawPlus(Vec2f(_settings.plusSize/2, 0),
      _settings.metaLineWidth, _settings.plusSize);
}

void ReactionRender::_drawArrow (float length)
{
   _rc.setSingleSource(CWC_BASE);
   _rc.drawArrow(Vec2f(0, 0), Vec2f(length < 0 ? _settings.arrowLength : length, 0), 
      _settings.metaLineWidth, _settings.arrowHeadWidth, 
      _settings.arrowHeadSize); 
}

void ReactionRender::cb_process (Metalayout::LayoutItem& item, const Vec2f& pos, void* context)
{
   ReactionRender* render = (ReactionRender*)context;
   render->_rc.restoreTransform();
   render->_rc.translate(pos.x, pos.y);

   switch (item.type)
   {
   case ITEM_TYPE_BASE_MOL:  
      if (item.over)
         render->_rc.translate(0, -item.scaledSize.y / 2 - 1);
      render->_drawMol(item);
      break;
   case ITEM_TYPE_RXN_PLUS:
      render->_drawPlus();
      break;
   case ITEM_TYPE_RXN_ARROW:
      render->_drawArrow();
      break;
   case ITEM_TYPE_RXN_BEGIN_ARROW:
      render->_ax = pos.x;
      break;
   case ITEM_TYPE_RXN_END_ARROW:
      render->_ax = pos.x - render->_ax;
      render->_rc.translate(-render->_ax, 0);
      render->_drawArrow(render->_ax);
      break;
   case ITEM_TYPE_RXN_SPACE:
      break;
   default:
      RenderBase::cb_process(item, pos, context);
   }
}