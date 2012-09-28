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

#include "graph/graph_affine_matcher.h"
#include "math/algebra.h"
#include "base_cpp/tlscont.h"
#include "graph/graph.h"

using namespace indigo;

IMPL_ERROR(GraphAffineMatcher, "graph affine matcher");

GraphAffineMatcher::GraphAffineMatcher (Graph &subgraph, Graph &supergraph, const int *mapping) :
_subgraph(subgraph),
_supergraph(supergraph),
_mapping(mapping)
{
   cb_get_xyz = 0;
   fixed_vertices = 0;
}

bool GraphAffineMatcher::match (float rms_threshold)
{
   if (cb_get_xyz == 0)
      throw Error("cb_get_xyz not set");

   int i;
   Transform3f matr;
   Vec3f pos;

   QS_DEF(Array<Vec3f>, points);
   QS_DEF(Array<Vec3f>, goals);

   points.clear();
   goals.clear();

   if (fixed_vertices != 0)
   {
      for (i = 0; i < fixed_vertices->size(); i++)
      {
         if (_mapping[fixed_vertices->at(i)] < 0)
            continue;
         cb_get_xyz(_subgraph, fixed_vertices->at(i), pos);
         points.push(pos);
         cb_get_xyz(_supergraph, _mapping[fixed_vertices->at(i)], pos);
         goals.push(pos);
      }
   }
   else for (i = _subgraph.vertexBegin(); i < _subgraph.vertexEnd(); i = _subgraph.vertexNext(i))
   {
      if (_mapping[i] < 0)
         continue;
      cb_get_xyz(_subgraph, i, pos);
      points.push(pos);
      cb_get_xyz(_supergraph, _mapping[i], pos);
      goals.push(pos);
   }

   if (points.size() < 1)
      return true;

   float sqsum;

   if (!matr.bestFit(points.size(), points.ptr(), goals.ptr(), &sqsum))
      return false;

   if (sqsum > rms_threshold * rms_threshold)
      return false;

   return true;
}
