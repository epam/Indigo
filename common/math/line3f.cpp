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

Line3f::Line3f ()
{
   org.zero();
   dir.set(0, 0, 1);
}

void Line3f::copy (Line3f &other)
{
   org.copy(other.org);
   dir.copy(other.dir);
}

float Line3f::distFromPoint (const Vec3f &point) const
{
   Vec3f diff;

   diff.diff(point, org);
   
   float prod = Vec3f::dot(dir, diff);

   diff.addScaled(dir, -prod);

   return diff.length();
}
