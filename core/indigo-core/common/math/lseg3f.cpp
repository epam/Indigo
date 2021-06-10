/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#include "math/algebra.h"

using namespace indigo;

LSeg3f::LSeg3f(const Vec3f& beg, const Vec3f& end) : _beg(beg), _end(end)
{
    _diff.diff(_end, _beg);

    _length_sqr = _diff.lengthSqr();

    _is_degenerate = (_length_sqr < EPSILON);
}

float LSeg3f::distToPoint(const Vec3f& point, Vec3f* closest) const
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
