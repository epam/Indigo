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
#include "molecule/query_molecule.h"
#include "base_cpp/output.h"
#include "layout/metalayout.h"
#include "render_context.h"
#include "render_internal.h"
#include "render_item_factory.h"
#include "render_item_fragment.h"

using namespace indigo;

IMPL_ERROR(RenderItemFragment, "RenderItemFragment");

RenderItemFragment::RenderItemFragment (RenderItemFactory& factory) : 
   RenderItemBase(factory),
   mol(NULL),
   aam(NULL),
   reactingCenters(NULL),
   inversionArray(NULL),
   exactChangeArray(NULL),
   refAtom(-1),
   _scaleFactor(1.0f),
   isRFragment(false)
{
}

RenderItemFragment::~RenderItemFragment ()
{
}

void RenderItemFragment::init ()
{
   _min.set(0,0);
   _max.set(0,0);
   for (int i = mol->vertexBegin(); i < mol->vertexEnd(); i = mol->vertexNext(i)) {
      const Vec3f& v = mol->getAtomXyz(i);
      Vec2f v2(v.x, v.y);
      if (i == mol->vertexBegin()) {
         _min.copy(v2);
         _max.copy(v2);
      } else {
         _min.min(v2);
         _max.max(v2);
      }
   }
}

void RenderItemFragment::estimateSize ()
{ 
   renderIdle();
   if (refAtom >= 0) {
      const Vec3f& v = mol->getAtomXyz(refAtom);
      Vec2f v2(v.x, v.y);
      refAtomPos.set(v2.x - _min.x, _max.y - v2.y);
      refAtomPos.scale(_scaleFactor);
      refAtomPos.sub(origin);
   }
}

void RenderItemFragment::render ()
{
   _rc.translate(-origin.x, -origin.y);
   MoleculeRenderInternal rnd(_opt, _settings, _rc);
   rnd.setMolecule(mol);
   rnd.setIsRFragment(isRFragment);
   rnd.setScaleFactor(_scaleFactor, _min, _max);
   rnd.setReactionComponentProperties(aam, reactingCenters, inversionArray);
   rnd.setQueryReactionComponentProperties(exactChangeArray);
   rnd.render();
}

static float get2dDist (const Vec3f& v1, const Vec3f& v2)
{
   return sqrt((v1.x - v2.x) * (v1.x - v2.x) + (v1.y - v2.y) * (v1.y - v2.y));
}

float RenderItemFragment::getTotalBondLength ()
{
   float sum = 0.0;
   for (int i = mol->edgeBegin(); i < mol->edgeEnd(); i = mol->edgeNext(i)) {
      const Edge& edge = mol->getEdge(i);
      sum += get2dDist(mol->getAtomXyz(edge.beg), mol->getAtomXyz(edge.end));
   }
   return sum;
}

float RenderItemFragment::getTotalClosestAtomDistance()
{
   if (mol->vertexCount() < 2)
      return 0;
   float sum = 0.0;
   for (int i = mol->vertexBegin(); i < mol->vertexEnd(); i = mol->vertexNext(i)) {
      float minDist = -1;
      for (int j = mol->vertexBegin(); j < mol->vertexEnd(); j = mol->vertexNext(j)) {
         if (i == j)
            continue;
         float dist = get2dDist(mol->getAtomXyz(i), mol->getAtomXyz(j));
         if (minDist < 0 || dist < minDist)
            minDist = dist;
      }
      sum += minDist;
   }
   return sum;
}

int RenderItemFragment::getBondCount ()
{
   return mol->edgeCount();
}

int RenderItemFragment::getAtomCount ()
{
   return mol->vertexCount();
}