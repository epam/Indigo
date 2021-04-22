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

Plane3f::Plane3f()
{
    _norm.set(0, 0, 1);
    _d = 0;
}

void Plane3f::copy(const Plane3f& other)
{
    _norm.copy(other._norm);
    _d = other._d;
}

float Plane3f::distFromPoint(const Vec3f& point) const
{
    return (float)fabs(Vec3f::dot(point, _norm) + _d);
}

void Plane3f::projection(const Vec3f& point, Vec3f& proj_out) const
{
    Vec3f org, diff, proj;

    org.scaled(_norm, _d);
    diff.diff(point, org);

    proj.scaled(_norm, Vec3f::dot(_norm, diff));
    diff.sub(proj);

    proj_out.sum(org, diff);
}

bool Plane3f::byPointAndLine(const Vec3f& point, const Line3f& line)
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
