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
#include "render_item_factory.h"
#include "render_item_fragment.h"

using namespace indigo;


RenderItemFragment::RenderItemFragment (RenderItemFactory& factory) : 
   RenderItemBase(factory),
   _mol(NULL),
   _highlighting(NULL),
   _aam(NULL),
   _reactingCenters(NULL),
   _inversionArray(NULL),
   _exactChangeArray(NULL),
   _scaleFactor(1.0)
{
}

RenderItemFragment::~RenderItemFragment ()
{
}

void RenderItemFragment::setMolecule (BaseMolecule* mol)
{
   _mol = mol;
   _min.set(0,0);
   _max.set(0,0);
   for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i)) {
      const Vec3f& v = _mol->getAtomXyz(i);
      Vec2f v2(v.x, v.y);
      if (i == _mol->vertexBegin()) {
         _min.copy(v2);
         _max.copy(v2);
      } else {
         _min.min(v2);
         _max.max(v2);
      }
   }
}

void RenderItemFragment::setMoleculeHighlighting (GraphHighlighting* highlighting)
{
   _highlighting = highlighting;
}

void RenderItemFragment::setAAM (Array<int>* aam) {
   _aam = aam;
}

void RenderItemFragment::setReactingCenters (Array<int>* reactingCenters)
{
   _reactingCenters = reactingCenters;
}

void RenderItemFragment::setInversionArray (Array<int>* inversionArray)
{
   _inversionArray = inversionArray;
}

void RenderItemFragment::setExactChangeArray (Array<int>* exactChangeArray)
{
   _exactChangeArray = exactChangeArray;
}

void RenderItemFragment::render ()
{
   _rc.translate(-origin.x, -origin.y); // TODO: shouldn't we take the scaleFactor into account here as well?
   MoleculeRenderInternal rnd(_opt, _settings, _rc);
   rnd.setMolecule(_mol);
   if (_highlighting != NULL)
      rnd.setHighlighting(_highlighting);
   rnd.setScaleFactor(_scaleFactor, _min, _max);
   rnd.setReactionComponentProperties(_aam, _reactingCenters, _inversionArray);
   rnd.setQueryReactionComponentProperties(_exactChangeArray);
   rnd.render();
}

static float get2dDist (const Vec3f& v1, const Vec3f& v2)
{
   return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

float RenderItemFragment::getTotalBondLength ()
{
   float sum = 0.0;
   for (int i = _mol->edgeBegin(); i < _mol->edgeEnd(); i = _mol->edgeNext(i)) {
      const Edge& edge = _mol->getEdge(i);
      sum += get2dDist(_mol->getAtomXyz(edge.beg), _mol->getAtomXyz(edge.end));
   }
   return sum;
}

float RenderItemFragment::getTotalClosestAtomDistance()
{
   if (_mol->vertexCount() < 2)
      return 0;
   float sum = 0.0;
   for (int i = _mol->vertexBegin(); i < _mol->vertexEnd(); i = _mol->vertexNext(i)) {
      float minDist = -1;
      for (int j = _mol->vertexBegin(); j < _mol->vertexEnd(); j = _mol->vertexNext(j)) {
         if (i == j)
            continue;
         float dist = get2dDist(_mol->getAtomXyz(i), _mol->getAtomXyz(j));
         if (minDist < 0 || dist < minDist)
            minDist = dist;
      }
      sum += minDist;
   }
   return sum;
}

int RenderItemFragment::getBondCount ()
{
   return _mol->edgeCount();
}

int RenderItemFragment::getAtomCount ()
{
   return _mol->vertexCount();
}