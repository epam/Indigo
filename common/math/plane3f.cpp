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

#include "math/algebra.h"

using namespace indigo;

Plane3f::Plane3f ()
{
   _norm.set(0, 0, 1);
   _d = 0;
}

void Plane3f::copy (const Plane3f &other)
{
   _norm.copy(other._norm);
   _d = other._d;
}

float Plane3f::distFromPoint (const Vec3f &point) const
{
   return (float)fabs(Vec3f::dot(point, _norm) + _d);
}

void Plane3f::projection (const Vec3f &point, Vec3f &proj_out) const
{
   Vec3f org, diff, proj;

   org.scaled(_norm, _d);
   diff.diff(point, org);

   proj.scaled(_norm, Vec3f::dot(_norm, diff));
   diff.sub(proj);

   proj_out.sum(org, diff);
}

bool Plane3f::byPointAndLine (const Vec3f &point, const Line3f &line)
{
   Vec3f diff, cross;

   diff.diff(point, line.org);
   cross.cross(diff, line.dir);

   if (!cross.normalize())
      return false;

   _norm.copy(cross);
   _d = -Vec3f::dot(_norm, line.org);
   return true;
}
