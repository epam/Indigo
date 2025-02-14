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
#include <stdio.h>

using namespace indigo;

IMPL_ERROR(Vec2f, "Vec2f");

bool Vec2f::normalize()
{
    float l = lengthSqr();

    if (l < EPSILON * EPSILON)
    {
        return false;
    }

    l = (float)sqrt(l);

    x /= l;
    y /= l;

    return true;
}

bool Vec2f::normalization(const Vec2f& v)
{
    float l = v.lengthSqr();

    if (l < EPSILON * EPSILON)
        return false;

    l = (float)sqrt(l);

    x = v.x / l;
    y = v.y / l;

    return true;
}

void Vec2f::rotate(float angle)
{
    rotate(sin(angle), cos(angle));
}

void Vec2f::rotate(float si, float co)
{
    Vec2f a(*this);

    x = co * a.x - si * a.y;
    y = si * a.x + co * a.y;
}

void Vec2f::rotate(Vec2f vec)
{
    rotate(vec.y, vec.x);
}

void Vec2f::rotateL(Vec2f vec)
{
    rotateL(vec.y, vec.x);
}

void Vec2f::rotateL(float angle)
{
    rotateL(sin(angle), cos(angle));
}

void Vec2f::rotateL(float si, float co)
{
    rotate(-si, co);
}

void Vec2f::rotateAroundSegmentEnd(const Vec2f& a, const Vec2f& b, float angle)
{
    Vec2f c;

    c.diff(a, b);
    c.rotate(angle);

    sum(b, c);
}

float Vec2f::tiltAngle()
{
    float l = length();

    if (l < EPSILON)
        return 0;

    if (y >= 0)
        return acos(x / l);
    return -acos(x / l);
}

float Vec2f::tiltAngle2()
{
    float l = length();

    if (l < EPSILON)
        return 0;

    if (y >= 0)
        return acos(x / l);
    return _2FLOAT(2. * M_PI - acos(x / l));
}

float Vec2f::calc_angle(Vec2f a, Vec2f b)
{
    a -= *this;
    b -= *this;
    double len_sqr_a = a.lengthSqr();
    double len_sqr_b = b.lengthSqr();
    double mult_ab = len_sqr_a * len_sqr_b;
    double sqr = sqrt(mult_ab);

    double cross = Vec2f::cross(a, b);
    double dot = Vec2f::dot(a, b);
    float cos = _2FLOAT(dot / sqr);
    float sin = _2FLOAT(cross / sqr);

    float angle;
    if (2 * cos * cos < 1)
    {
        angle = acos_stable(cos);
        if (cross < 0)
            angle = -angle;
    }
    else
    {
        angle = asin_stable(sin);
        if (dot < 0)
        {
            if (cross >= 0)
                angle = _2FLOAT(M_PI - angle);
            else
                angle = _2FLOAT(-M_PI - angle);
        }
    }

    return angle;
}

float Vec2f::calc_angle_pos(Vec2f a, Vec2f b)
{
    float angle = this->calc_angle(a, b);
    if (angle < 0)
        angle += _2FLOAT(2. * M_PI);
    return angle;
}

float Vec2f::distSqr(const Vec2f& a, const Vec2f& b)
{
    float dx = b.x - a.x;
    float dy = b.y - a.y;

    return dx * dx + dy * dy;
}

float Vec2f::dist(const Vec2f& a, const Vec2f& b)
{
    return (float)sqrt(distSqr(a, b));
}

float Vec2f::dot(const Vec2f& a, const Vec2f& b)
{
    return a.x * b.x + a.y * b.y;
}

float Vec2f::cross(const Vec2f& a, const Vec2f& b)
{
    return a.x * b.y - a.y * b.x;
}

float Vec2f::relativeCross(const Vec2f& a, const Vec2f& b) const
{
    return cross(a - *this, b - *this);
}

void Vec2f::projectZ(Vec2f& v2, const Vec3f& v3)
{
    v2.x = v3.x;
    v2.y = v3.y;
}

// two edges:
//    x = x1_1 + (x1_2 - x1_1)t1;
//    y = y1_1 + (y1_2 - y1_1)t1;
// and
//    x = x2_1 + (x2_2 - x2_1)t2;
//    y = y2_1 + (y2_2 - y2_1)t2;
// then
//       (x2_2 - x2_1)(y2_1 - y1_1) - (x2_1 - x1_1)(y2_2 - y2_1)   a2 * b12 - a12 * b2
// t1 =  ------------------------------------------------------- = -------------------
//       (x2_2 - x2_1)(y1_2 - y1_1) - (x1_2 - x1_1)(y2_2 - y2_1)    a2 * b1 - a1 * b2
//
//       (x1_2 - x1_1)(y2_1 - y1_1) - (x2_1 - x1_1)(y1_2 - y1_1)   a1 * b12 - a12 * b1
// t2 =  ------------------------------------------------------- = -------------------
//       (x2_2 - x2_1)(y1_2 - y1_1) - (x1_2 - x1_1)(y2_2 - y2_1)    a2 * b1 - a1 * b2
bool Vec2f::intersection(const Vec2f& v1_1, const Vec2f& v1_2, const Vec2f& v2_1, const Vec2f& v2_2, Vec2f& p)
{
    float a1, a12, b12, a2, b1, b2;
    float delta, delta1, delta2, t1, t2;

    a1 = v1_2.x - v1_1.x;
    b1 = v1_2.y - v1_1.y;
    a12 = v2_1.x - v1_1.x;
    b12 = v2_1.y - v1_1.y;
    a2 = v2_2.x - v2_1.x;
    b2 = v2_2.y - v2_1.y;

    delta = a2 * b1 - a1 * b2;
    delta1 = a2 * b12 - a12 * b2;
    delta2 = a1 * b12 - a12 * b1;

    if (fabs(delta) < EPSILON)
        return false;

    t1 = delta1 / delta;
    t2 = delta2 / delta;

    if (fabs(t1) < EPSILON || fabs(t1 - 1.f) < EPSILON || fabs(t2) < EPSILON || fabs(t2 - 1.f) < EPSILON)
        return false;

    if (t1 < 0.f || t1 > 1.f || t2 < 0.f || t2 > 1.f)
        return false;

    p.x = v1_1.x + (v1_2.x - v1_1.x) * t1;
    p.y = v1_1.y + (v1_2.y - v1_1.y) * t1;

    return true;
}

float Vec2f::triangleArea(const Vec2f& a, const Vec2f& b, const Vec2f& c)
{
    return (b.x - a.x) * (c.y - a.y) - (c.x - a.x) * (b.y - a.y);
}

bool Vec2f::segmentsIntersect(const Vec2f& a0, const Vec2f& a1, const Vec2f& b0, const Vec2f& b1)
{

    float maxax = std::max(a0.x, a1.x);
    float maxay = std::max(a0.y, a1.y);
    float maxbx = std::max(b0.x, b1.x);
    float maxby = std::max(b0.y, b1.y);
    float minax = std::min(a0.x, a1.x);
    float minay = std::min(a0.y, a1.y);
    float minbx = std::min(b0.x, b1.x);
    float minby = std::min(b0.y, b1.y);

    float big_eps = 0.001f;

    if (maxax + big_eps < minbx || maxbx + big_eps < minax || maxay + big_eps < minby || maxby + big_eps < minay)
        return false;

    // regular check
    return triangleArea(a0, a1, b0) * triangleArea(a0, a1, b1) < EPSILON && triangleArea(b0, b1, a0) * triangleArea(b0, b1, a1) < EPSILON;
}

bool Vec2f::segmentsIntersectInternal(const Vec2f& a0, const Vec2f& a1, const Vec2f& b0, const Vec2f& b1)
{

    float maxax = std::max(a0.x, a1.x);
    float maxay = std::max(a0.y, a1.y);
    float maxbx = std::max(b0.x, b1.x);
    float maxby = std::max(b0.y, b1.y);
    float minax = std::min(a0.x, a1.x);
    float minay = std::min(a0.y, a1.y);
    float minbx = std::min(b0.x, b1.x);
    float minby = std::min(b0.y, b1.y);

    float big_eps = 0.001f;

    if (maxax < minbx + big_eps || maxbx < minax + big_eps || maxay < minby + big_eps || maxby < minay + big_eps)
        return false;

    // regular check
    return triangleArea(a0, a1, b0) * triangleArea(a0, a1, b1) < -EPSILON && triangleArea(b0, b1, a0) * triangleArea(b0, b1, a1) < -EPSILON;
}

float Vec2f::distPointSegment(Vec2f p, Vec2f q, Vec2f r)
{
    if (dot(p - q, r - q) <= 0)
        return dist(p, q);
    if (dot(p - r, q - r) <= 0)
        return dist(p, r);

    Vec2f normal = r - q;
    normal.rotate(_2FLOAT(M_PI / 2.));
    float c = cross(q, r);
    float s = normal.length();

    float t = -c - dot(normal, p);

    return fabs(t / s);
}

float Vec2f::distSegmentSegment(Vec2f p, Vec2f q, Vec2f r, Vec2f s)
{
    if (Vec2f::segmentsIntersect(p, q, r, s))
        return 0;

    return std::min(std::min(distPointSegment(p, r, s), distPointSegment(q, r, s)), std::min(distPointSegment(r, p, q), distPointSegment(s, p, q)));
}

Vec2f Vec2f::get_circle_center(Vec2f p, Vec2f q, float angle)
{

    Vec2f vec(q - p);

    return (p + q) / 2.f + vec / _2FLOAT(tan((M_PI - angle) / 2.));
}

Vec2f Vec2f::get_circle_center(Vec2f a, Vec2f p, Vec2f q)
{
    p -= a;
    q -= a;
    float cross = Vec2f::cross(p, q);
    if (fabs(cross) < EPSILON)
        return (p + q) / 2 + a;
    float c1 = -p.lengthSqr() / 2;
    float c2 = -q.lengthSqr() / 2;

    Vec2f center(c1 * q.x - c2 * p.x, p.y * c2 - q.y * c1);
    return center / cross + a;
}
