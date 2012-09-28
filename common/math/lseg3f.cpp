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

LSeg3f::LSeg3f(const Vec3f &beg, const Vec3f &end) :
_beg(beg),
_end(end)
{
   _diff.diff(_end, _beg);

   _length_sqr = _diff.lengthSqr();

   _is_degenerate = (_length_sqr < EPSILON);
}

float LSeg3f::distToPoint(const Vec3f &point, Vec3f *closest) const
{
   if (_is_degenerate)
   {
      if (closest != 0)
         closest->copy(_beg);

      return Vec3f::dist(point, _beg);
   }

   Vec3f p;
   float t;

   p.diff(point, _beg);
   t = Vec3f::dot(p, _diff) / _length_sqr;

   if (t < 0.f)
      p.copy(_beg);
   else if (t > 1.f)
      p.copy(_end);
   else
      p.lineCombin(_beg, _diff, t);

   if (closest != 0)
      closest->copy(p);

   return Vec3f::dist(point, p);
}
