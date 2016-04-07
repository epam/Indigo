/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
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

#ifndef _ALGEBRA_H_
#define _ALGEBRA_H_

#include <math.h>

#include "base_c/defs.h"
#include "base_cpp/exception.h"

#define SQR(x) ((x) * (x))

#define DEG2RAD(x) ((x) * PI / 180)
#define RAD2DEG(x) ((x) * 180 / PI)

#ifdef INFINITY
#undef INFINITY
#endif

#ifdef PI
#undef PI
#endif

namespace indigo {

const float EPSILON = 0.000001f;

const float PI = 3.14159265358979323846f;

const float INFINITY = 1000000.f;


struct Transform3f;

struct Vec3f;
struct Vec2f
{
   DECL_ERROR;

   Vec2f () : x(0), y(0) {}
   Vec2f (const Vec2f &a) : x(a.x), y(a.y) {}
   Vec2f (float xx, float yy) : x(xx), y(yy) {}

   float x, y;

   inline void set (float xx, float yy)
   {
      x = xx;
      y = yy;
   }

   inline void copy (const Vec2f &a)
   {
      x = a.x;
      y = a.y;
   }

   inline void zero ()
   {
      x = 0;
      y = 0;
   }

   inline void negate () {x = -x; y = -y;}

   inline void negation (const Vec2f &v)
   {
      x = -v.x;
      y = -v.y;
   }

   inline void add (const Vec2f &v)
   {
      x += v.x;
      y += v.y;
   }

   inline void sum (const Vec2f &a, const Vec2f &b)
   {
      x = a.x + b.x;
      y = a.y + b.y;
   }

   inline void sub (const Vec2f &v)
   {
      x -= v.x;
      y -= v.y;
   }

   inline void diff (const Vec2f &a, const Vec2f &b)
   {
      x = a.x - b.x;
      y = a.y - b.y;
   }

   inline void min (const Vec2f &a)
   {
      x = __min(x, a.x);
      y = __min(y, a.y);
   }

   inline void max (const Vec2f &a)
   {
      x = __max(x, a.x);
      y = __max(y, a.y);
   }

   inline float lengthSqr () const
   {
      return x * x + y * y;
   }

   inline float length () const
   {
      return (float)sqrt(lengthSqr());
   }

   // OPERATORS:

   inline Vec2f operator+(const Vec2f& a) const {
      return Vec2f(x + a.x, y + a.y);
   }

   inline Vec2f operator-(const Vec2f& a) const {
      return Vec2f(x - a.x, y - a.y);
   }

   inline Vec2f operator*(float t) const {
      return Vec2f(x * t, y * t);
   }

   inline Vec2f operator/(float t) const {
      return Vec2f(x / t, y / t);
   }

   inline Vec2f operator+=(const Vec2f& a) {
      x += a.x;
      y += a.y;
      return *this;
   }

   inline Vec2f operator-=(const Vec2f& a) {
      x -= a.x;
      y -= a.y;
      return *this;
   }

   inline Vec2f operator*=(float t) {
      x *= t;
      y *= t;
      return *this;
   }

   inline Vec2f operator/=(float t) {
      x /= t;
      y /= t;
      return *this;
   }



   DLLEXPORT bool normalize ();

   DLLEXPORT bool normalization (const Vec2f &v);

   DLLEXPORT float tiltAngle ();

   DLLEXPORT float tiltAngle2 ();

   DLLEXPORT float calc_angle(Vec2f a, Vec2f b);

   DLLEXPORT float calc_angle_pos(Vec2f a, Vec2f b);

   inline void scale(float s)
   {
      x *= s;
      y *= s;
   }

   inline void scaled (const Vec2f &v, float s)
   {
      x = v.x * s;
      y = v.y * s;
   }

   inline void addScaled (const Vec2f &v, float s)
   {
      x += v.x * s;
      y += v.y * s;
   }

   inline void lineCombin (const Vec2f &a, const Vec2f &b, float t)
   {
      x = a.x + b.x * t;
      y = a.y + b.y * t;
   }

   inline void lineCombin2 (const Vec2f &a, float ta, const Vec2f &b, float tb)
   {
      x = a.x * ta + b.x * tb;
      y = a.y * ta + b.y * tb;
   }

   DLLEXPORT void rotate (float angle);
   DLLEXPORT void rotate (float si, float co);
   DLLEXPORT void rotate (Vec2f vec);
   DLLEXPORT void rotateL (float angle);
   DLLEXPORT void rotateL (float si, float co);
   DLLEXPORT void rotateL (Vec2f vec);
   DLLEXPORT void rotateAroundSegmentEnd(const Vec2f &a, const Vec2f &b, float angle);

   DLLEXPORT static float distSqr (const Vec2f &a, const Vec2f &b);
   DLLEXPORT static float dist    (const Vec2f &a, const Vec2f &b);
   DLLEXPORT static float dot     (const Vec2f &a, const Vec2f &b);
   DLLEXPORT static float cross   (const Vec2f &a, const Vec2f &b);
   DLLEXPORT static void projectZ (Vec2f& v2, const Vec3f& v3);
   DLLEXPORT static bool intersection (const Vec2f &v1_1, const Vec2f &v1_2, const Vec2f &v2_1, const Vec2f &v2_2, Vec2f &p);
   DLLEXPORT static float triangleArea (const Vec2f &a, const Vec2f &b, const Vec2f &c);
   DLLEXPORT static bool segmentsIntersect (const Vec2f &a0, const Vec2f &a1, const Vec2f &b0, const Vec2f &b1);
   DLLEXPORT static bool segmentsIntersectInternal (const Vec2f &a0, const Vec2f &a1, const Vec2f &b0, const Vec2f &b1);

   DLLEXPORT static double distPointSegment(Vec2f p, Vec2f q, Vec2f r);
   DLLEXPORT static double distSegmentSegment(Vec2f p, Vec2f q, Vec2f r, Vec2f s);

   DLLEXPORT static Vec2f get_circle_center(Vec2f p, Vec2f q, double angle);
   DLLEXPORT static Vec2f get_circle_center(Vec2f a, Vec2f b, Vec2f c);
};

struct Rect2f {

   explicit Rect2f () {}

   Rect2f (Vec2f a, Vec2f b)
   {
      _leftBottom = a;
      _leftBottom.min(b);
      _rightTop = a;
      _rightTop.max(b);
   }

   Rect2f (Rect2f a, Rect2f b)
   {
      _leftBottom = a._leftBottom;
      _leftBottom.min(b._leftBottom);
      _rightTop = a._rightTop;
      _rightTop.max(b._rightTop);
   }

   inline void copy (Rect2f &other)
   {
      _leftBottom = other._leftBottom;
      _rightTop   = other._rightTop;
   }

   inline float left() const { return _leftBottom.x; }
   inline float right() const { return _rightTop.x; }
   inline float bottom() const { return _leftBottom.y; }
   inline float top() const { return _rightTop.y; }

   inline float middleX() const { return (_leftBottom.x + _rightTop.x) / 2; }
   inline float middleY() const { return (_leftBottom.y + _rightTop.y) / 2; }

   inline Vec2f leftBottom() const { return _leftBottom; }
   inline Vec2f rightTop() const { return _rightTop; }

   inline Vec2f leftTop() const { return Vec2f(left(), top()); }
   inline Vec2f rightBottom() const { return Vec2f(right(), bottom()); }

   inline Vec2f leftMiddle() const { return Vec2f(left(), middleY()); }
   inline Vec2f rightMiddle() const { return Vec2f(right(), middleY()); }

   inline Vec2f bottomMiddle() const { return Vec2f(middleX(), bottom()); }
   inline Vec2f topMiddle() const { return Vec2f(middleX(), top()); }

   inline Vec2f center() const { return Vec2f(middleX(), middleY()); }

protected:
   Vec2f _leftBottom;
   Vec2f _rightTop;
};

struct Vec3f
{
   Vec3f () : x(0), y(0), z(0) {}
   Vec3f (float xx, float yy, float zz) : x(xx), y(yy), z(zz) {}
   Vec3f (Vec2f &v) : x(v.x), y(v.y), z(0) {}

   float x, y, z;

   inline void set (float xx, float yy, float zz)
   {
      x = xx;
      y = yy;
      z = zz;
   }

   inline void copy (const Vec3f &a)
   {
      x = a.x;
      y = a.y;
      z = a.z;
   }

   inline void zero ()
   {
      x = 0;
      y = 0;
      z = 0;
   }

   inline void negate () {x = -x; y = -y; z = -z;}

   inline void negation (const Vec3f &v)
   {
      x = -v.x;
      y = -v.y;
      z = -v.z;
   }

   inline void add (const Vec3f &v)
   {
      x += v.x;
      y += v.y;
      z += v.z;
   }

   inline void sum (const Vec3f &a, const Vec3f &b)
   {
      x = a.x + b.x;
      y = a.y + b.y;
      z = a.z + b.z;
   }

   inline void sub (const Vec3f &v)
   {
      x -= v.x;
      y -= v.y;
      z -= v.z;
   }

   inline void diff (const Vec3f &a, const Vec3f &b)
   {
      x = a.x - b.x;
      y = a.y - b.y;
      z = a.z - b.z;
   }

   inline void min (const Vec3f &a)
   {
      x = __min(x, a.x);
      y = __min(y, a.y);
      z = __min(z, a.z);
   }

   inline void max (const Vec3f &a)
   {
      x = __max(x, a.x);
      y = __max(y, a.y);
      z = __max(z, a.z);
   }

   inline void cross (const Vec3f &a, const Vec3f &b)
   {
      x = a.y * b.z - a.z * b.y;
      y = a.z * b.x - a.x * b.z;
      z = a.x * b.y - a.y * b.x;
   }

   inline float lengthSqr () const
   {
      return x * x + y * y + z * z;
   }

   DLLEXPORT float length () const;

   DLLEXPORT bool normalize ();
   DLLEXPORT bool normalization (const Vec3f &v);

   inline void scale (float s)
   {
      x *= s;
      y *= s;
      z *= s;
   }

   inline void scaled (const Vec3f &v, float s)
   {
      x = v.x * s;
      y = v.y * s;
      z = v.z * s;
   }

   inline void addScaled (const Vec3f &v, float s)
   {
      x += v.x * s;
      y += v.y * s;
      z += v.z * s;
   }

   inline void lineCombin (const Vec3f &a, const Vec3f &b, float t)
   {
      x = a.x + b.x * t;
      y = a.y + b.y * t;
      z = a.z + b.z * t;
   }

   inline void lineCombin2 (const Vec3f &a, float ta, const Vec3f &b, float tb)
   {
      x = a.x * ta + b.x * tb;
      y = a.y * ta + b.y * tb;
      z = a.z * ta + b.z * tb;
   }

   inline Vec2f projectZ () const
   {
      return Vec2f(x, y);
   }

   DLLEXPORT void rotateX (float angle);
   DLLEXPORT void rotateY (float angle);
   DLLEXPORT void rotateZ (float angle);

   DLLEXPORT void rotate (const Vec3f &around, float angle);

   DLLEXPORT void transformPoint  (const Transform3f &matr);
   DLLEXPORT void transformVector (const Transform3f &matr);
   DLLEXPORT void invTransformVector (const Transform3f &matr);

   DLLEXPORT void pointTransformation     (const Vec3f &v, const Transform3f &matr);
   DLLEXPORT void vectorTransformation    (const Vec3f &v, const Transform3f &matr);
   DLLEXPORT void invVectorTransformation (const Vec3f &v, const Transform3f &matr);

   // returns value in range 0..pi
   DLLEXPORT static bool  angle   (const Vec3f &a, const Vec3f &b, float &res);
   DLLEXPORT static float dot     (const Vec3f &a, const Vec3f &b);
   DLLEXPORT static float dist    (const Vec3f &a, const Vec3f &b);
   DLLEXPORT static float distSqr (const Vec3f &a, const Vec3f &b);
};

const Vec3f VZero3f (0.f, 0.f, 0.f);

struct Transform3f
{
   DECL_ERROR;

   float elements[16];

   void rotation (float x, float y, float z, float angle);

   void rotationX (float angle);
   void rotationY (float angle);
   void rotationZ (float angle);

   bool rotationVecVec (const Vec3f &v1, const Vec3f &v2);
   bool rotationQuat (float quat[4]);

   bool inversion (const Transform3f &matr);

   void copy (const Transform3f &matr);

   void identity (void);

   void getOrigin (Vec3f &origin);

   void composition (const Transform3f &matr, const Transform3f &transform);
   void transform      (const Transform3f &transform);
   void transformLocal (const Transform3f &transform);

   void setOrigin (float x, float y, float z);
   void setOrigin (const Vec3f &origin);
   void translate (const Vec3f &translation);
   void translateLocal (float x, float y, float z);
   void translateLocal (const Vec3f &translation);
   void translateLocalInv (const Vec3f &translation);
   void translateInv (const Vec3f &translation);

   void rotateX (float angle);
   void rotateY (float angle);
   void rotateZ (float angle);

   void rotateXLocal (float angle);
   void rotateYLocal (float angle);
   void rotateZLocal (float angle);

   bool bestFit (int npoints, const Vec3f points[], const Vec3f goals[], float *sqsum_out);
};

struct Matr3x3d
{
   double elements[9];

   DECL_ERROR;

   Matr3x3d ();

   void copy (const Matr3x3d &matr);
   void transpose ();
   void getTransposed (Matr3x3d &matr_out) const;
   void identity ();

   void matrixMatrixMultiply (const Matr3x3d &m, Matr3x3d &matrix_out) const;
   void matrixVectorMultiply (const Vec3f &a, Vec3f &b) const;

   void eigenSystem (Matr3x3d &evec_out);

protected:
   void _qrStep (int n, double gc[], double gs[]);
   void _givensRotation (double x0, double x1, double &c, double &s);
};

struct LSeg3f
{
  LSeg3f (const Vec3f &beg, const Vec3f &end);

  float distToPoint (const Vec3f &point, Vec3f *closest) const;

protected:
  Vec3f _beg;
  Vec3f _end;
  Vec3f _diff;
  float _length_sqr;
  bool  _is_degenerate;
};

struct Line3f
{
   Vec3f org;
   Vec3f dir;

   explicit Line3f ();

   void copy (Line3f &other);

   float distFromPoint (const Vec3f &point) const;

   bool bestFit (int npoints, const Vec3f points[], float *sqsum_out);
};

struct Plane3f
{
   explicit Plane3f ();

   void copy (const Plane3f &other);

   inline const Vec3f &getNorm () const { return _norm; }
   inline const float &getD () const { return _d; }

   void  projection (const Vec3f &point, Vec3f &proj_out) const;
   bool  byPointAndLine (const Vec3f &point, const Line3f &line);
   float distFromPoint (const Vec3f &point) const;

   bool bestFit (int npoints, const Vec3f points[], float *sqsum_out);

protected:
   Vec3f _norm;
   float _d;
};

}
#endif

