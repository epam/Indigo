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
#include <limits>

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
            return std::numeric_limits<decltype(x)>::min();
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

        DLLEXPORT bool normalize();

        DLLEXPORT bool normalization(const Vec2f& v);

        DLLEXPORT float tiltAngle();

        DLLEXPORT float tiltAngle2();

        DLLEXPORT float calc_angle(Vec2f a, Vec2f b);

        DLLEXPORT float calc_angle_pos(Vec2f a, Vec2f b);

        inline void scale(float s)
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

        inline bool rayIntersectsRect(const Vec2f& begin, const Vec2f& end)
        {
            Vec2f v = end - begin;
            Vec2f vr(v.y, -v.x); // perpendicular vector
            auto lb = _leftBottom - begin;
            auto lt = leftTop() - begin;
            auto rt = _rightTop - begin;
            auto rb = rightBottom() - begin;
            // same_sign means no intersection
            bool same_sign = std::signbit(Vec2f::dot(vr, lb)) == std::signbit(Vec2f::dot(vr, lt)) &&
                             std::signbit(Vec2f::dot(vr, lt)) == std::signbit(Vec2f::dot(vr, rt)) &&
                             std::signbit(Vec2f::dot(vr, rt)) == std::signbit(Vec2f::dot(vr, rb));

            return !same_sign && Vec2f::cross(lb, vr) > 0 && Vec2f::cross(lt, vr) > 0 && Vec2f::cross(rt, vr) > 0 && Vec2f::cross(rb, vr) > 0;
        }

        inline double pointDistance(const Vec2f& pt)
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

} // namespace indigo
#endif
