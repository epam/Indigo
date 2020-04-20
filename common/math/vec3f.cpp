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

float Vec3f::length() const
{
    return (float)sqrt(lengthSqr());
}

void Vec3f::transformPoint(const Transform3f& matr)
{
    Vec3f v;

    v.pointTransformation(*this, matr);
    copy(v);
}

void Vec3f::transformVector(const Transform3f& matr)
{
    Vec3f v;

    v.vectorTransformation(*this, matr);
    copy(v);
}

void Vec3f::invTransformVector(const Transform3f& matr)
{
    Vec3f v;

    v.invVectorTransformation(*this, matr);
    copy(v);
}

void Vec3f::pointTransformation(const Vec3f& v, const Transform3f& matr)
{
    if (&v == this)
    {
        transformPoint(matr);
        return;
    }

    x = matr.elements[0] * v.x + matr.elements[4] * v.y + matr.elements[8] * v.z + matr.elements[12];
    y = matr.elements[1] * v.x + matr.elements[5] * v.y + matr.elements[9] * v.z + matr.elements[13];
    z = matr.elements[2] * v.x + matr.elements[6] * v.y + matr.elements[10] * v.z + matr.elements[14];
}

void Vec3f::vectorTransformation(const Vec3f& v, const Transform3f& matr)
{
    if (&v == this)
    {
        transformVector(matr);
        return;
    }

    x = matr.elements[0] * v.x + matr.elements[4] * v.y + matr.elements[8] * v.z;
    y = matr.elements[1] * v.x + matr.elements[5] * v.y + matr.elements[9] * v.z;
    z = matr.elements[2] * v.x + matr.elements[6] * v.y + matr.elements[10] * v.z;
}

void Vec3f::invVectorTransformation(const Vec3f& v, const Transform3f& matr)
{
    if (&v == this)
    {
        invTransformVector(matr);
        return;
    }

    x = matr.elements[0] * v.x + matr.elements[1] * v.y + matr.elements[2] * v.z;
    y = matr.elements[4] * v.x + matr.elements[5] * v.y + matr.elements[6] * v.z;
    z = matr.elements[8] * v.x + matr.elements[9] * v.y + matr.elements[10] * v.z;
}

void Vec3f::rotateX(float angle)
{
    float sine = (float)sin(angle);
    float cosine = (float)cos(angle);
    float yy = y * cosine - z * sine;

    z = y * sine + z * cosine;
    y = yy;
}

void Vec3f::rotateY(float angle)
{
    float sine = (float)sin(angle);
    float cosine = (float)cos(angle);
    float xx = x * cosine + z * sine;

    z = -x * sine + z * cosine;
    x = xx;
}

void Vec3f::rotateZ(float angle)
{
    float sine = (float)sin(angle);
    float cosine = (float)cos(angle);
    float xx = x * cosine - y * sine;

    y = x * sine + y * cosine;
    x = xx;
}

void Vec3f::rotate(const Vec3f& around, float angle)
{
    Transform3f matr;

    matr.rotation(around.x, around.y, around.z, angle);
    transformVector(matr);
}

bool Vec3f::normalize()
{
    float l = lengthSqr();

    if (l < EPSILON * EPSILON)
        return false;

    l = (float)sqrt(l);

    x /= l;
    y /= l;
    z /= l;

    return true;
}

bool Vec3f::normalization(const Vec3f& v)
{
    float l = v.lengthSqr();

    if (l < EPSILON * EPSILON)
        return false;

    l = (float)sqrt(l);

    x = v.x / l;
    y = v.y / l;
    z = v.z / l;

    return true;
}

bool Vec3f::angle(const Vec3f& a, const Vec3f& b, float& res)
{
    float a_len = a.length();
    float b_len = b.length();

    if (a_len < EPSILON || b_len < EPSILON)
        return false;

    res = acos(dot(a, b) / (a_len * b_len));
    return true;
}

float Vec3f::dot(const Vec3f& a, const Vec3f& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float Vec3f::distSqr(const Vec3f& a, const Vec3f& b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dz = b.z - a.z;

    return dx * dx + dy * dy + dz * dz;
}

float Vec3f::dist(const Vec3f& a, const Vec3f& b)
{
    return (float)sqrt(distSqr(a, b));
}
