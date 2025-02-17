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

#ifndef _ALGEBRA_H_
#define _ALGEBRA_H_

#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <vector>

#include "base_c/defs.h"
#include "base_cpp/exception.h"

#define SQR(x) ((x) * (x))

#define DEG2RAD(x) ((x)*M_PI / 180)
#define RAD2DEG(x) ((x)*180 / M_PI)
#define HYPOT(a, b) (sqrt((a) * (a) + (b) * (b)))

namespace indigo
{

    const float EPSILON = 0.000001f;

    // frac of type 1/n for acos_stable
    const double frac1[25] = {0.,      1.,      1. / 2,  1. / 3,  1. / 4,  1. / 5,  1. / 6,  1. / 7,  1. / 8,  1. / 9,  1. / 10, 1. / 11, 1. / 12,
                              1. / 13, 1. / 14, 1. / 15, 1. / 16, 1. / 17, 1. / 18, 1. / 19, 1. / 20, 1. / 21, 1. / 22, 1. / 23, 1. / 24};

    struct Transform3f;

    struct Vec3f;
    struct Vec2f
    {
        DECL_ERROR;

        Vec2f() : x(0), y(0)
        {
        }
        Vec2f(const Vec2f& a) : x(a.x), y(a.y)
        {
        }
        Vec2f(float xx, float yy) : x(xx), y(yy)
        {
        }

        float x, y;

        static constexpr auto min_coord()
        {
            return std::numeric_limits<decltype(x)>::lowest();
        }

        static constexpr auto max_coord()
        {
            return std::numeric_limits<decltype(x)>::max();
        }

        inline void set(float xx, float yy)
        {
            x = xx;
            y = yy;
        }

        inline void copy(const Vec2f& a)
        {
            x = a.x;
            y = a.y;
        }

        inline void zero()
        {
            x = 0;
            y = 0;
        }

        inline void clear()
        {
            zero();
        }

        inline void negate()
        {
            x = -x;
            y = -y;
        }

        inline void negation(const Vec2f& v)
        {
            x = -v.x;
            y = -v.y;
        }

        inline void add(const Vec2f& v)
        {
            x += v.x;
            y += v.y;
        }

        inline void sum(const Vec2f& a, const Vec2f& b)
        {
            x = a.x + b.x;
            y = a.y + b.y;
        }

        inline void sub(const Vec2f& v)
        {
            x -= v.x;
            y -= v.y;
        }

        inline void diff(const Vec2f& a, const Vec2f& b)
        {
            x = a.x - b.x;
            y = a.y - b.y;
        }

        inline void min(const Vec2f& a)
        {
            x = std::min(x, a.x);
            y = std::min(y, a.y);
        }

        inline void max(const Vec2f& a)
        {
            x = std::max(x, a.x);
            y = std::max(y, a.y);
        }

        inline float lengthSqr() const
        {
            return x * x + y * y;
        }

        inline float length() const
        {
            return _2FLOAT(sqrt(lengthSqr()));
        }

        // OPERATORS:

        inline bool operator<(const Vec2f& a) const
        {
            return std::make_pair(x, y) < std::make_pair(a.x, a.y);
        }

        inline float operator*(const Vec2f& a) const
        {
            return x * a.x + y * a.y;
        }

        inline Vec2f operator+(const Vec2f& a) const
        {
            return Vec2f(x + a.x, y + a.y);
        }

        inline Vec2f operator-(const Vec2f& a) const
        {
            return Vec2f(x - a.x, y - a.y);
        }

        inline Vec2f operator*(float t) const
        {
            return Vec2f(x * t, y * t);
        }

        inline Vec2f operator/(float t) const
        {
            return Vec2f(x / t, y / t);
        }

        inline Vec2f operator+=(const Vec2f& a)
        {
            x += a.x;
            y += a.y;
            return *this;
        }

        inline Vec2f operator-=(const Vec2f& a)
        {
            x -= a.x;
            y -= a.y;
            return *this;
        }

        inline Vec2f operator*=(float t)
        {
            x *= t;
            y *= t;
            return *this;
        }

        inline Vec2f operator/=(float t)
        {
            x /= t;
            y /= t;
            return *this;
        }

        inline float vcos(const Vec2f& a) const
        {
            float scalar = *this * a;
            float ta = length() * a.length();
            if (ta < EPSILON)
                ta = EPSILON;
            return scalar / ta;
        }

        inline float vsin(const Vec2f& a) const
        {
            float scalar = *this * a;
            float ta = lengthSqr() * a.lengthSqr();
            if (ta < EPSILON)
                ta = EPSILON;
            return sqrt(1 - scalar * scalar / ta);
        }

        DLLEXPORT bool normalize();

        DLLEXPORT bool normalization(const Vec2f& v);

        DLLEXPORT float tiltAngle();

        DLLEXPORT float tiltAngle2();

        DLLEXPORT float calc_angle(Vec2f a, Vec2f b);

        DLLEXPORT float calc_angle_pos(Vec2f a, Vec2f b);

        inline void scale(const float s)
        {
            x *= s;
            y *= s;
        }

        inline void scaled(const Vec2f& v, float s)
        {
            x = v.x * s;
            y = v.y * s;
        }

        inline void addScaled(const Vec2f& v, float s)
        {
            x += v.x * s;
            y += v.y * s;
        }

        inline void lineCombin(const Vec2f& a, const Vec2f& b, float t)
        {
            x = a.x + b.x * t;
            y = a.y + b.y * t;
        }

        inline void lineCombin2(const Vec2f& a, float ta, const Vec2f& b, float tb)
        {
            x = a.x * ta + b.x * tb;
            y = a.y * ta + b.y * tb;
        }

        DLLEXPORT void rotate(float angle);
        DLLEXPORT void rotate(float si, float co);
        DLLEXPORT void rotate(Vec2f vec);
        DLLEXPORT void rotateL(float angle);
        DLLEXPORT void rotateL(float si, float co);
        DLLEXPORT void rotateL(Vec2f vec);
        DLLEXPORT void rotateAroundSegmentEnd(const Vec2f& a, const Vec2f& b, float angle);
        DLLEXPORT float relativeCross(const Vec2f& a, const Vec2f& b) const;

        DLLEXPORT static float distSqr(const Vec2f& a, const Vec2f& b);
        DLLEXPORT static float dist(const Vec2f& a, const Vec2f& b);
        DLLEXPORT static float dot(const Vec2f& a, const Vec2f& b);
        DLLEXPORT static float cross(const Vec2f& a, const Vec2f& b);
        DLLEXPORT static void projectZ(Vec2f& v2, const Vec3f& v3);
        DLLEXPORT static bool intersection(const Vec2f& v1_1, const Vec2f& v1_2, const Vec2f& v2_1, const Vec2f& v2_2, Vec2f& p);
        DLLEXPORT static float triangleArea(const Vec2f& a, const Vec2f& b, const Vec2f& c);
        DLLEXPORT static bool segmentsIntersect(const Vec2f& a0, const Vec2f& a1, const Vec2f& b0, const Vec2f& b1);
        DLLEXPORT static bool segmentsIntersectInternal(const Vec2f& a0, const Vec2f& a1, const Vec2f& b0, const Vec2f& b1);

        DLLEXPORT static float distPointSegment(Vec2f p, Vec2f q, Vec2f r);
        DLLEXPORT static float distSegmentSegment(Vec2f p, Vec2f q, Vec2f r, Vec2f s);

        DLLEXPORT static Vec2f get_circle_center(Vec2f p, Vec2f q, float angle);
        DLLEXPORT static Vec2f get_circle_center(Vec2f a, Vec2f b, Vec2f c);

        DLLEXPORT static float asin_stable(float x)
        {
            double x2 = _2DOUBLE(x) * _2DOUBLE(x);
            double res = 0.;
            double y = _2DOUBLE(x);
            for (int i = 0; i < 12; i++)
            {
                res += y * frac1[2 * i + 1];
                y *= (1. - frac1[2 * i + 2]) * x2;
            }
            return _2FLOAT(res);
        }

        DLLEXPORT static float acos_stable(float x)
        {
            return _2FLOAT((M_PI / 2.) - asin_stable(x));
        }
    };

    inline bool rayIntersectsSegment(const Vec2f& beg, const Vec2f& end, const Vec2f& a, const Vec2f& b)
    {
        Vec2f ray = end - beg;

        // Vector of the segment
        Vec2f segment = a - b;

        // Calculate the cross products
        float cross1 = Vec2f::cross(ray, segment);
        if (fabs(cross1) < 1e-6)
        {
            // Parallel segments
            return false;
        }

        // Compute the cross product of vectors (beg - a) and (beg - b)
        Vec2f vecBA = beg - a;
        Vec2f vecBB = beg - b;

        float t = Vec2f::cross(vecBA, segment) / cross1;
        if (t < 0)
        {
            // Intersection is behind the starting point
            return false;
        }

        float u = Vec2f::cross(vecBA, ray) / cross1;
        return (u >= 0 && u <= 1);
    }

    struct Rect2f
    {

        explicit Rect2f()
        {
        }

        Rect2f(Vec2f a, Vec2f b)
        {
            _leftBottom = a;
            _leftBottom.min(b);
            _rightTop = a;
            _rightTop.max(b);
        }

        Rect2f(Rect2f a, Rect2f b)
        {
            _leftBottom = a._leftBottom;
            _leftBottom.min(b._leftBottom);
            _rightTop = a._rightTop;
            _rightTop.max(b._rightTop);
        }

        inline void copy(Rect2f& other)
        {
            _leftBottom = other._leftBottom;
            _rightTop = other._rightTop;
        }

        inline bool pointInRect(const Vec2f& pt) const
        {
            return _leftBottom.x < pt.x && _leftBottom.y < pt.y && _rightTop.x > pt.x && _rightTop.y > pt.y;
        }

        inline bool intersects(const Rect2f& other) const
        {
            return !(right() < other.left() || left() > other.right() || top() < other.bottom() || bottom() > other.top());
        }

        inline float distanceTo(const Rect2f& other) const
        {
            if (intersects(other))
                return 0.0f;

            float dx = std::max(0.0f, std::max(other.left() - right(), left() - other.right()));
            float dy = std::max(0.0f, std::max(other.bottom() - top(), bottom() - other.top()));

            return (dx > 0.0f && dy > 0.0f) ? HYPOT(dx, dy) : (dx + dy);
        }

        inline bool rayIntersectsRect(const Vec2f& begin, const Vec2f& end)
        {
            auto lb = _leftBottom;
            auto lt = leftTop();
            auto rt = _rightTop;
            auto rb = rightBottom();
            return rayIntersectsSegment(begin, end, lb, lt) || rayIntersectsSegment(begin, end, lt, rt) || rayIntersectsSegment(begin, end, rt, rb) ||
                   rayIntersectsSegment(begin, end, rb, lb);
        }

        inline float pointDistance(const Vec2f& pt)
        {
            if (pt.x <= left())
            {
                if (pt.y <= bottom())
                    return HYPOT(left() - pt.x, bottom() - pt.y);
                if (top() <= pt.y)
                    return HYPOT(left() - pt.x, pt.y - top());
                return left() - pt.x;
            }
            if (pt.x >= right())
            {
                if (pt.y <= bottom())
                    return HYPOT(pt.x - right(), bottom() - pt.y);
                if (top() <= pt.y)
                    return HYPOT(pt.x - right(), pt.y - top());
                return pt.x - right();
            }
            if (pt.y <= bottom())
                return bottom() - pt.y;
            if (top() <= pt.y)
                return pt.y - top();
            return 0.0; // if pt is inside the box
        }

        inline void extend(const Rect2f& second)
        {
            _leftBottom.min(second._leftBottom);
            _rightTop.max(second._rightTop);
        }

        inline void offset(const Vec2f& offset)
        {
            _leftBottom += offset;
            _rightTop += offset;
        }

        inline float left() const
        {
            return _leftBottom.x;
        }
        inline float right() const
        {
            return _rightTop.x;
        }
        inline float bottom() const
        {
            return _leftBottom.y;
        }
        inline float top() const
        {
            return _rightTop.y;
        }

        inline static float middle(const float& one, const float& second)
        {
            return (one + second) / 2;
        }

        inline float middleX() const
        {
            return middle(_leftBottom.x, _rightTop.x);
        }
        inline float middleY() const
        {
            return middle(_leftBottom.y, _rightTop.y);
        }

        inline float between_left_box(const Rect2f& second) const
        {
            return middle(second.right(), left());
        }

        inline Vec2f leftBottom() const
        {
            return _leftBottom;
        }
        inline Vec2f rightTop() const
        {
            return _rightTop;
        }

        inline Vec2f leftTop() const
        {
            return Vec2f(left(), top());
        }
        inline Vec2f rightBottom() const
        {
            return Vec2f(right(), bottom());
        }

        inline Vec2f leftMiddle() const
        {
            return Vec2f(left(), middleY());
        }
        inline Vec2f rightMiddle() const
        {
            return Vec2f(right(), middleY());
        }

        inline Vec2f bottomMiddle() const
        {
            return Vec2f(middleX(), bottom());
        }
        inline Vec2f topMiddle() const
        {
            return Vec2f(middleX(), top());
        }

        inline Vec2f center() const
        {
            return Vec2f(middleX(), middleY());
        }

        inline float width() const
        {
            return _rightTop.x - _leftBottom.x;
        }

        inline float height() const
        {
            return _rightTop.y - _leftBottom.y;
        }

    protected:
        Vec2f _leftBottom;
        Vec2f _rightTop;
    };

    struct Vec3f
    {
        Vec3f() : x(0), y(0), z(0)
        {
        }
        Vec3f(float xx, float yy, float zz) : x(xx), y(yy), z(zz)
        {
        }
        Vec3f(Vec2f& v) : x(v.x), y(v.y), z(0)
        {
        }

        float x, y, z;

        inline void set(float xx, float yy, float zz)
        {
            x = xx;
            y = yy;
            z = zz;
        }

        inline void copy(const Vec3f& a)
        {
            x = a.x;
            y = a.y;
            z = a.z;
        }

        inline void zero()
        {
            x = 0;
            y = 0;
            z = 0;
        }

        inline void clear()
        {
            zero();
        }

        inline void negate()
        {
            x = -x;
            y = -y;
            z = -z;
        }

        inline void negation(const Vec3f& v)
        {
            x = -v.x;
            y = -v.y;
            z = -v.z;
        }

        inline void add(const Vec3f& v)
        {
            x += v.x;
            y += v.y;
            z += v.z;
        }

        inline void sum(const Vec3f& a, const Vec3f& b)
        {
            x = a.x + b.x;
            y = a.y + b.y;
            z = a.z + b.z;
        }

        inline void sub(const Vec3f& v)
        {
            x -= v.x;
            y -= v.y;
            z -= v.z;
        }

        inline void diff(const Vec3f& a, const Vec3f& b)
        {
            x = a.x - b.x;
            y = a.y - b.y;
            z = a.z - b.z;
        }

        inline void min(const Vec3f& a)
        {
            x = std::min(x, a.x);
            y = std::min(y, a.y);
            z = std::min(z, a.z);
        }

        inline void max(const Vec3f& a)
        {
            x = std::max(x, a.x);
            y = std::max(y, a.y);
            z = std::max(z, a.z);
        }

        inline void cross(const Vec3f& a, const Vec3f& b)
        {
            x = a.y * b.z - a.z * b.y;
            y = a.z * b.x - a.x * b.z;
            z = a.x * b.y - a.y * b.x;
        }

        inline float lengthSqr() const
        {
            return x * x + y * y + z * z;
        }

        DLLEXPORT float length() const;

        DLLEXPORT bool normalize();
        DLLEXPORT bool normalization(const Vec3f& v);

        inline void scale(float s)
        {
            x *= s;
            y *= s;
            z *= s;
        }

        inline void scaled(const Vec3f& v, float s)
        {
            x = v.x * s;
            y = v.y * s;
            z = v.z * s;
        }

        inline void addScaled(const Vec3f& v, float s)
        {
            x += v.x * s;
            y += v.y * s;
            z += v.z * s;
        }

        inline void lineCombin(const Vec3f& a, const Vec3f& b, float t)
        {
            x = a.x + b.x * t;
            y = a.y + b.y * t;
            z = a.z + b.z * t;
        }

        inline void lineCombin2(const Vec3f& a, float ta, const Vec3f& b, float tb)
        {
            x = a.x * ta + b.x * tb;
            y = a.y * ta + b.y * tb;
            z = a.z * ta + b.z * tb;
        }

        inline Vec2f projectZ() const
        {
            return Vec2f(x, y);
        }

        DLLEXPORT void rotateX(float angle);
        DLLEXPORT void rotateY(float angle);
        DLLEXPORT void rotateZ(float angle);

        DLLEXPORT void rotate(const Vec3f& around, float angle);

        DLLEXPORT void transformPoint(const Transform3f& matr);
        DLLEXPORT void transformVector(const Transform3f& matr);
        DLLEXPORT void invTransformVector(const Transform3f& matr);

        DLLEXPORT void pointTransformation(const Vec3f& v, const Transform3f& matr);
        DLLEXPORT void vectorTransformation(const Vec3f& v, const Transform3f& matr);
        DLLEXPORT void invVectorTransformation(const Vec3f& v, const Transform3f& matr);

        // returns value in range 0..pi
        DLLEXPORT static bool angle(const Vec3f& a, const Vec3f& b, float& res);
        DLLEXPORT static float dot(const Vec3f& a, const Vec3f& b);
        DLLEXPORT static float dist(const Vec3f& a, const Vec3f& b);
        DLLEXPORT static float distSqr(const Vec3f& a, const Vec3f& b);
    };

    const Vec3f VZero3f(0.f, 0.f, 0.f);

    struct Transform3f
    {
        DECL_ERROR;

        float elements[16];

        void rotation(float x, float y, float z, float angle);

        void rotationX(float angle);
        void rotationY(float angle);
        void rotationZ(float angle);

        bool rotationVecVec(const Vec3f& v1, const Vec3f& v2);
        bool rotationQuat(float quat[4]);

        bool inversion(const Transform3f& matr);

        void copy(const Transform3f& matr);

        void identity(void);

        void getOrigin(Vec3f& origin);

        void composition(const Transform3f& matr, const Transform3f& transform);
        void transform(const Transform3f& transform);
        void transformLocal(const Transform3f& transform);

        void setOrigin(float x, float y, float z);
        void setOrigin(const Vec3f& origin);
        void translate(const Vec3f& translation);
        void translateLocal(float x, float y, float z);
        void translateLocal(const Vec3f& translation);
        void translateLocalInv(const Vec3f& translation);
        void translateInv(const Vec3f& translation);

        void rotateX(float angle);
        void rotateY(float angle);
        void rotateZ(float angle);

        void rotateXLocal(float angle);
        void rotateYLocal(float angle);
        void rotateZLocal(float angle);

        bool bestFit(int npoints, const Vec3f points[], const Vec3f goals[], float* sqsum_out);
    };

    struct Matr3x3d
    {
        double elements[9];

        DECL_ERROR;

        Matr3x3d();

        void copy(const Matr3x3d& matr);
        void transpose();
        void getTransposed(Matr3x3d& matr_out) const;
        void identity();

        void matrixMatrixMultiply(const Matr3x3d& m, Matr3x3d& matrix_out) const;
        void matrixVectorMultiply(const Vec3f& a, Vec3f& b) const;

        void eigenSystem(Matr3x3d& evec_out);

    protected:
        void _qrStep(int n, double gc[], double gs[]);
        void _givensRotation(double x0, double x1, double& c, double& s);
    };

    struct LSeg3f
    {
        LSeg3f(const Vec3f& beg, const Vec3f& end);

        float distToPoint(const Vec3f& point, Vec3f* closest) const;

    protected:
        Vec3f _beg;
        Vec3f _end;
        Vec3f _diff;
        float _length_sqr;
        bool _is_degenerate;
    };

    struct Line3f
    {
        Vec3f org;
        Vec3f dir;

        explicit Line3f();

        void copy(Line3f& other);

        float distFromPoint(const Vec3f& point) const;

        bool bestFit(int npoints, const Vec3f points[], float* sqsum_out);
    };

    struct Plane3f
    {
        explicit Plane3f();

        void copy(const Plane3f& other);

        inline const Vec3f& getNorm() const
        {
            return _norm;
        }
        inline const float& getD() const
        {
            return _d;
        }

        void projection(const Vec3f& point, Vec3f& proj_out) const;
        bool byPointAndLine(const Vec3f& point, const Line3f& line);
        float distFromPoint(const Vec3f& point) const;

        bool bestFit(int npoints, const Vec3f points[], float* sqsum_out);

    protected:
        Vec3f _norm;
        float _d;
    };

    inline bool isPointOnSegment(const Vec2f& p, const Vec2f& a, const Vec2f& b)
    {
        float c = Vec2f::cross(p - a, b - a);
        if (std::fabs(c) > EPSILON)
            return false;
        float d = (p - a) * (b - a);
        if (d < 0)
            return false;
        float l = (b - a) * (b - a);
        if (d > l)
            return false;
        return true;
    }

    // Ray casting algorithm
    inline bool isPointInPolygon(const Vec2f& p, const std::vector<Vec2f>& poly)
    {
        bool in = false;
        size_t n = poly.size();
        for (size_t i = 0; i < n; ++i)
        {
            size_t j = (i + 1) % n;
            if (isPointOnSegment(p, poly[i], poly[j]))
                return true;
            if (((poly[i].y > p.y) != (poly[j].y > p.y)) &&
                (((p.x > poly[i].x) && (p.x < poly[j].x)) || (p.x < (poly[i].x + (p.y - poly[i].y) * (poly[j].x - poly[i].x) / (poly[j].y - poly[i].y)))))
                in = !in;
        }
        return in;
    }

    inline std::vector<Vec2f> getPointsInsidePolygon(const std::vector<Vec2f>& polygon1, const std::vector<Vec2f>& polygon2)
    {
        std::vector<Vec2f> result;
        result.reserve(polygon1.size());
        std::copy_if(polygon1.begin(), polygon1.end(), std::back_inserter(result), [&](auto& p) { return isPointInPolygon(p, polygon2); });
        return result;
    }

    inline bool isPointInConvexPolygon(const Vec2f& p, const std::vector<Vec2f>& poly)
    {
        auto cross = [](const Vec2f& a, const Vec2f& b, const Vec2f& c) { return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x); };
        bool sign = cross(poly.back(), poly[0], p) < 0;
        for (size_t i = 0, n = poly.size(); i < n; ++i)
            if ((cross(poly[i], poly[(i + 1) % n], p) < 0) != sign)
                return false;
        return true;
    }

    inline std::vector<Vec2f> getPointsInsideConvexPolygon(const std::vector<Vec2f>& polygon1, const std::vector<Vec2f>& polygon2)
    {
        std::vector<Vec2f> result;
        result.reserve(polygon1.size());
        std::copy_if(polygon1.begin(), polygon1.end(), std::back_inserter(result), [&](auto& p) { return isPointInConvexPolygon(p, polygon2); });
        return result;
    }

    // Shoelace formula - triangle form
    inline float convexPolygonArea(const std::vector<Vec2f>& poly)
    {
        float area = 0.0f;
        size_t n = poly.size();
        for (size_t i = 0; i < n; ++i)
        {
            const Vec2f& p1 = poly[i];
            const Vec2f& p2 = poly[(i + 1) % n];
            area += Vec2f::cross(p1, p2);
        }
        return std::abs(area) * 0.5f;
    }

    // Separating axis theorem
    inline bool convexPolygonsIntersect(const std::vector<Vec2f>& poly1, const std::vector<Vec2f>& poly2)
    {
        auto project = [](const std::vector<Vec2f>& poly, const Vec2f& axis) {
            auto [min, max] = std::minmax_element(poly.begin(), poly.end(), [&](const Vec2f& a, const Vec2f& b) { return a * axis < b * axis; });
            return std::pair{*min * axis, *max * axis};
        };

        auto overlap = [](const auto& range1, const auto& range2) { return !(range1.first > range2.second || range2.first > range1.second); };

        auto getNormals = [](const std::vector<Vec2f>& poly) {
            std::vector<Vec2f> normals;
            for (size_t i = 0; i < poly.size(); ++i)
            {
                Vec2f edge = poly[(i + 1) % poly.size()] - poly[i];
                normals.emplace_back(-edge.y, edge.x);
            }
            return normals;
        };

        auto normals1 = getNormals(poly1), normals2 = getNormals(poly2);
        for (const auto& normal : normals1)
            if (!overlap(project(poly1, normal), project(poly2, normal)))
                return false;
        for (const auto& normal : normals2)
            if (!overlap(project(poly1, normal), project(poly2, normal)))
                return false;
        return true;
    }

    inline Vec2f computeIntersection(const Vec2f& s, const Vec2f& e, const Vec2f& edgeStart, const Vec2f& edgeEnd)
    {
        float dx1 = e.x - s.x, dy1 = e.y - s.y, dx2 = edgeEnd.x - edgeStart.x, dy2 = edgeEnd.y - edgeStart.y;
        float det = dx1 * dy2 - dy1 * dx2;
        if (std::abs(det) < 1e-6)
            return s;
        float t = ((edgeStart.x - s.x) * dy2 - (edgeStart.y - s.y) * dx2) / det;
        return {s.x + t * dx1, s.y + t * dy1};
    }

    // Sutherland–Hodgman algorithm
    inline std::vector<Vec2f> convexClip(const std::vector<Vec2f>& subject, const std::vector<Vec2f>& clip)
    {
        std::vector<Vec2f> result = subject;
        for (size_t i = 0; i < clip.size(); ++i)
        {
            const Vec2f& edgeStart = clip[i];
            const Vec2f& edgeEnd = clip[(i + 1) % clip.size()];
            std::vector<Vec2f> input = std::move(result);
            result.clear();
            if (input.empty())
                break;
            Vec2f prev = input.back();
            for (const Vec2f& curr : input)
            {
                if (edgeStart.relativeCross(edgeEnd, curr) <= 0)
                {
                    if (edgeStart.relativeCross(edgeEnd, prev) > 0)
                        result.push_back(computeIntersection(prev, curr, edgeStart, edgeEnd));
                    result.push_back(curr);
                }
                else if (edgeStart.relativeCross(edgeEnd, prev) <= 0)
                {
                    result.push_back(computeIntersection(prev, curr, edgeStart, edgeEnd));
                }
                prev = curr;
            }
        }
        return result;
    }

    inline float computeConvexContainment(const std::vector<Vec2f>& poly1, const std::vector<Vec2f>& poly2)
    {
        float area = convexPolygonArea(poly1);
        if (area == 0.0f)
            return 0.0f;
        std::vector<Vec2f> intersection = convexClip(poly1, poly2);
        if (intersection.empty())
            return 0.0f;
        float intersectionArea = convexPolygonArea(intersection);
        if (std::abs(intersectionArea - area) < 1e-6f)
            return 1.0f;
        return intersectionArea / area;
    }

    inline float distancePointToEdge(const Vec2f& p, const Vec2f& a, const Vec2f& b)
    {
        Vec2f ab = b - a;
        Vec2f ap = p - a;
        float t = Vec2f::dot(ap, ab) / Vec2f::dot(ab, ab);
        t = std::max(0.0f, std::min(1.0f, t));
        Vec2f projection(a.x + ab.x * t, a.y + ab.y * t);
        Vec2f diff = p - projection;
        return diff.length();
    }

    inline float computeConvexDistance(const std::vector<Vec2f>& poly1, const std::vector<Vec2f>& poly2)
    {
        float minDist = std::numeric_limits<float>::max();
        for (const auto& p : poly1)
        {
            float dist = std::numeric_limits<float>::max();
            size_t n = poly2.size();
            for (size_t i = 0; i < n; ++i)
            {
                float d = distancePointToEdge(p, poly2[i], poly2[(i + 1) % n]);
                dist = std::min(dist, d);
                if (d < minDist)
                    break;
            }
            minDist = std::min(minDist, dist);
        }
        for (const auto& p : poly2)
        {
            float dist = std::numeric_limits<float>::max();
            size_t n = poly1.size();
            for (size_t i = 0; i < n; ++i)
            {
                float d = distancePointToEdge(p, poly1[i], poly1[(i + 1) % n]);
                dist = std::min(dist, d);
                if (d < minDist)
                    break;
            }
            minDist = std::min(minDist, dist);
        }
        return minDist;
    }

    inline bool doesVerticalLineIntersectPolygon(float x, const std::vector<Vec2f>& poly)
    {
        for (size_t i = 0, n = poly.size(); i < n; ++i)
        {
            const Vec2f& a = poly[i];
            const Vec2f& b = poly[(i + 1) % n];
            if ((x > std::min(a.x, b.x) && x < std::max(a.x, b.x)))
                return true;
        }
        return false;
    }

    inline bool doesHorizontalLineIntersectPolygon(float y, const std::vector<Vec2f>& poly)
    {
        for (size_t i = 0, n = poly.size(); i < n; ++i)
        {
            const Vec2f& a = poly[i];
            const Vec2f& b = poly[(i + 1) % n];
            if ((y > std::min(a.y, b.y) && y < std::max(a.y, b.y)))
                return true;
        }
        return false;
    }

    inline bool doesRayIntersectPolygon(const Vec2f& p1, const Vec2f& p2, const std::vector<Vec2f>& poly)
    {
        Vec2f dir = p2 - p1;
        for (size_t i = 0, n = poly.size(); i < n; ++i)
        {
            const Vec2f& A = poly[i];
            const Vec2f& B = poly[(i + 1) % n];
            Vec2f a = A - p1;
            Vec2f b = B - p1;
            Vec2f s = b - a;
            float den = Vec2f::cross(dir, s);
            if (std::fabs(den) <= +0.0f)
                continue;
            float t = Vec2f::cross(a, s) / den;
            float u = Vec2f::cross(a, dir) / den;
            if (t >= 0.f && u >= 0.f && u <= 1.f)
                return true;
        }
        return false;
    }

} // namespace indigo
#endif
