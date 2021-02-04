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

#include <string.h>

#include "math/algebra.h"

using namespace indigo;

IMPL_ERROR(Transform3f, "transform3f");

void Transform3f::copy(const Transform3f& other)
{
    memcpy(elements, other.elements, 16 * sizeof(float));
}

void Transform3f::getOrigin(Vec3f& origin)
{
    origin.set(elements[12], elements[13], elements[14]);
}

bool Transform3f::inversion(const Transform3f& matr)
{
    if (&matr == this)
        throw Error("can not do inversion() of self");

    if ((float)fabs(matr.elements[3]) > EPSILON || (float)fabs(matr.elements[7]) > EPSILON || (float)fabs(matr.elements[11]) > EPSILON)
        return false;

    elements[0] = matr.elements[0];
    elements[1] = matr.elements[4];
    elements[2] = matr.elements[8];
    elements[3] = 0;
    elements[4] = matr.elements[1];
    elements[5] = matr.elements[5];
    elements[6] = matr.elements[9];
    elements[7] = 0;
    elements[8] = matr.elements[2];
    elements[9] = matr.elements[6];
    elements[10] = matr.elements[10];
    elements[11] = 0;

    elements[12] = -matr.elements[0] * matr.elements[12] - matr.elements[1] * matr.elements[13] - matr.elements[2] * matr.elements[14];
    elements[13] = -matr.elements[4] * matr.elements[12] - matr.elements[5] * matr.elements[13] - matr.elements[6] * matr.elements[14];
    elements[14] = -matr.elements[8] * matr.elements[12] - matr.elements[9] * matr.elements[13] - matr.elements[10] * matr.elements[14];

    elements[15] = 1.f;

    return true;
}

void Transform3f::rotation(float x, float y, float z, float angle)
{
    float len = (float)sqrt(x * x + y * y + z * z);
    float Sin = (float)sin(angle), Cos = (float)cos(angle), Vers = 1 - Cos;

    if (len > EPSILON)
    {
        x /= len;
        y /= len;
        z /= len;
    }

    elements[0] = x * x + Cos * (1 - x * x);
    elements[1] = x * Vers * y - z * Sin;
    elements[2] = x * Vers * z + y * Sin;
    elements[3] = 0;

    elements[4] = x * Vers * y + z * Sin;
    elements[5] = y * y + Cos * (1 - y * y);
    elements[6] = y * Vers * z - x * Sin;
    elements[7] = 0;

    elements[8] = x * Vers * z - y * Sin;
    elements[9] = y * Vers * z + x * Sin;
    elements[10] = z * z + Cos * (1 - z * z);
    elements[11] = 0;

    elements[12] = 0;
    elements[13] = 0;
    elements[14] = 0;
    elements[15] = 1;
}

void Transform3f::transform(const Transform3f& transform)
{
    Transform3f tmp;

    tmp.composition(*this, transform);
    copy(tmp);
}

void Transform3f::transformLocal(const Transform3f& transform)
{
    Transform3f tmp;

    tmp.composition(transform, *this);
    copy(tmp);
}

void Transform3f::rotateX(float angle)
{
    Transform3f rot;

    rot.rotationX(angle);
    transform(rot);
}

void Transform3f::rotateY(float angle)
{
    Transform3f rot;

    rot.rotationY(angle);
    transform(rot);
}

void Transform3f::rotateZ(float angle)
{
    Transform3f rot;

    rot.rotationZ(angle);
    transform(rot);
}

void Transform3f::rotateXLocal(float angle)
{
    Transform3f rot;

    rot.rotationX(angle);
    transformLocal(rot);
}

void Transform3f::rotateYLocal(float angle)
{
    Transform3f rot;

    rot.rotationY(angle);
    transformLocal(rot);
}

void Transform3f::rotateZLocal(float angle)
{
    Transform3f rot;

    rot.rotationZ(angle);
    transformLocal(rot);
}

void Transform3f::translate(const Vec3f& translation)
{
    elements[12] += translation.x;
    elements[13] += translation.y;
    elements[14] += translation.z;
}

void Transform3f::translateInv(const Vec3f& translation)
{
    elements[12] -= translation.x;
    elements[13] -= translation.y;
    elements[14] -= translation.z;
}

void Transform3f::rotationX(float angle)
{
    float sine = (float)sin(angle), cosine = (float)cos(angle);

    memset(elements, 0, 16 * sizeof(float));

    elements[0] = 1.f;
    elements[5] = cosine;
    elements[6] = sine;
    elements[9] = -sine;
    elements[10] = cosine;
    elements[15] = 1.f;
}

void Transform3f::rotationY(float angle)
{
    float sine = (float)sin(angle), cosine = (float)cos(angle);

    memset(elements, 0, 16 * sizeof(float));

    elements[0] = cosine;
    elements[2] = -sine;
    elements[8] = sine;
    elements[10] = cosine;
    elements[5] = 1.f;
    elements[15] = 1.f;
}

void Transform3f::rotationZ(float angle)
{
    float sine = (float)sin(angle), cosine = (float)cos(angle);

    memset(elements, 0, 16 * sizeof(float));

    elements[0] = cosine;
    elements[1] = sine;
    elements[4] = -sine;
    elements[5] = cosine;
    elements[10] = 1.f;
    elements[15] = 1.f;
}

void Transform3f::composition(const Transform3f& a, const Transform3f& b)
{
    elements[0] = a.elements[0] * b.elements[0] + a.elements[1] * b.elements[4] + a.elements[2] * b.elements[8];
    elements[1] = a.elements[0] * b.elements[1] + a.elements[1] * b.elements[5] + a.elements[2] * b.elements[9];
    elements[2] = a.elements[0] * b.elements[2] + a.elements[1] * b.elements[6] + a.elements[2] * b.elements[10];
    elements[3] = 0;
    elements[4] = a.elements[4] * b.elements[0] + a.elements[5] * b.elements[4] + a.elements[6] * b.elements[8];
    elements[5] = a.elements[4] * b.elements[1] + a.elements[5] * b.elements[5] + a.elements[6] * b.elements[9];
    elements[6] = a.elements[4] * b.elements[2] + a.elements[5] * b.elements[6] + a.elements[6] * b.elements[10];
    elements[7] = 0;
    elements[8] = a.elements[8] * b.elements[0] + a.elements[9] * b.elements[4] + a.elements[10] * b.elements[8];
    elements[9] = a.elements[8] * b.elements[1] + a.elements[9] * b.elements[5] + a.elements[10] * b.elements[9];
    elements[10] = a.elements[8] * b.elements[2] + a.elements[9] * b.elements[6] + a.elements[10] * b.elements[10];
    elements[11] = 0;
    elements[12] = a.elements[12] * b.elements[0] + a.elements[13] * b.elements[4] + a.elements[14] * b.elements[8] + b.elements[12];
    elements[13] = a.elements[12] * b.elements[1] + a.elements[13] * b.elements[5] + a.elements[14] * b.elements[9] + b.elements[13];
    elements[14] = a.elements[12] * b.elements[2] + a.elements[13] * b.elements[6] + a.elements[14] * b.elements[10] + b.elements[14];
    elements[15] = 1;
}

void Transform3f::identity(void)
{
    memset(elements, 0, 16 * sizeof(float));

    elements[0] = 1.f;
    elements[5] = 1.f;
    elements[10] = 1.f;
    elements[15] = 1.f;
}

void Transform3f::translateLocal(float x, float y, float z)
{
    elements[12] += elements[0] * x + elements[4] * y + elements[8] * z;
    elements[13] += elements[1] * x + elements[5] * y + elements[9] * z;
    elements[14] += elements[2] * x + elements[6] * y + elements[10] * z;
}

void Transform3f::translateLocal(const Vec3f& translation)
{
    translateLocal(translation.x, translation.y, translation.z);
}

void Transform3f::translateLocalInv(const Vec3f& translation)
{
    translateLocal(-translation.x, -translation.y, -translation.z);
}

void Transform3f::setOrigin(float x, float y, float z)
{
    elements[12] = x;
    elements[13] = y;
    elements[14] = z;
}

void Transform3f::setOrigin(const Vec3f& origin)
{
    setOrigin(origin.x, origin.y, origin.z);
}

bool Transform3f::rotationVecVec(const Vec3f& v1, const Vec3f& v2)
{
    Vec3f v1_norm, v2_norm;

    if (!v1_norm.normalization(v1) || !v2_norm.normalization(v2))
        return false;

    Vec3f cross;

    cross.cross(v1_norm, v2_norm);

    if (!cross.normalize())
    {
        // cross product have zero length -> v1 & v2 are codirectional
        identity();
        return true;
    }

    float dot = Vec3f::dot(v1_norm, v2_norm);

    float ang;

    if (dot > 1.f - EPSILON)
        ang = 0.f;
    else if (dot < -1.f + EPSILON)
        ang = _2FLOAT(-M_PI);
    else
        ang = _2FLOAT(-acos(dot));

    rotation(cross.x, cross.y, cross.z, ang);
    return true;
}
