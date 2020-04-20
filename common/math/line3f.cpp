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

Line3f::Line3f()
{
    org.zero();
    dir.set(0, 0, 1);
}

void Line3f::copy(Line3f& other)
{
    org.copy(other.org);
    dir.copy(other.dir);
}

float Line3f::distFromPoint(const Vec3f& point) const
{
    Vec3f diff;

    diff.diff(point, org);

    float prod = Vec3f::dot(dir, diff);

    diff.addScaled(dir, -prod);

    return diff.length();
}
