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

#ifndef __molecule_3d_constraints__
#define __molecule_3d_constraints__

#include "base_cpp/ptr_array.h"
#include "base_cpp/red_black.h"
#include "base_cpp/tlscont.h"
#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class BaseMolecule;
class QueryMolecule;

class DLLEXPORT Molecule3dConstraints
{
public:
   Molecule3dConstraints ();

   void init ();

   enum
   {
      POINT_ATOM           = 1,
      POINT_DISTANCE       = 2,
      POINT_PERCENTAGE     = 3,
      POINT_NORMALE        = 4,
      POINT_CENTROID       = 5,
      LINE_NORMALE         = 6,
      LINE_BEST_FIT        = 7,
      PLANE_BEST_FIT       = 8,
      PLANE_POINT_LINE     = 9,
      ANGLE_3POINTS        = 10,
      ANGLE_2LINES         = 11,
      ANGLE_2PLANES        = 12,
      ANGLE_DIHEDRAL       = 13,
      DISTANCE_2POINTS     = 14,
      DISTANCE_POINT_LINE  = 15,
      DISTANCE_POINT_PLANE = 16,
      EXCLUSION_SPHERE     = 17
   };

   struct Base
   {
      explicit Base (int type_) : type(type_) {}
      virtual ~Base () {}

      int type;
   };

   struct AngleBase : public Base
   {
   public:
      explicit AngleBase (int type) : Base(type) {}
      virtual ~AngleBase () {}

      float bottom;
      float top;
   };

   struct DistanceBase : public Base
   {
      explicit DistanceBase (int type) : Base(type) {}
      virtual ~DistanceBase () {}

      float bottom;
      float top;
   };

   struct Normale : public Base
   {
      explicit Normale () : Base(LINE_NORMALE) {}
      virtual ~Normale () {}

      int point_id;
      int plane_id;
   };

   struct BestFitLine : public Base
   {
      explicit BestFitLine () : Base(LINE_BEST_FIT) {}
      virtual ~BestFitLine () {}

      float max_deviation;
      Array<int> point_ids;
   };

   struct PointByAtom : public Base
   {
      explicit PointByAtom () : Base(POINT_ATOM) {}
      virtual ~PointByAtom () {}

      int atom_idx;
   };

   struct PointByDistance : public Base
   {
      explicit PointByDistance () : Base(POINT_DISTANCE) {}
      virtual ~PointByDistance () {}

      int   beg_id;
      int   end_id;
      float distance;
   };

   struct PointByPercentage : public Base
   {
      explicit PointByPercentage () : Base(POINT_PERCENTAGE) {}
      virtual ~PointByPercentage () {}

      int   beg_id;
      int   end_id;
      float percentage;
   };

   struct PointByNormale : public Base
   {
      explicit PointByNormale () : Base(POINT_NORMALE) {}
      virtual ~PointByNormale () {}

      int   org_id;
      int   norm_id;
      float distance;
   };

   struct Centroid : public Base
   {
      explicit Centroid () : Base(POINT_CENTROID) {}
      virtual ~Centroid () {}

      Array<int> point_ids;
   };

   struct BestFitPlane : public Base
   {
      explicit BestFitPlane () : Base(PLANE_BEST_FIT) {}
      virtual ~BestFitPlane () {}

      float max_deviation;
      Array<int> point_ids;
   };

   struct PlaneByPoint : public Base
   {
      explicit PlaneByPoint () : Base(PLANE_POINT_LINE) {}
      virtual ~PlaneByPoint () {}

      int point_id;
      int line_id;
   };

   struct DistanceByPoints : public DistanceBase
   {
      explicit DistanceByPoints () : DistanceBase(DISTANCE_2POINTS) {}
      virtual ~DistanceByPoints () {}

      int beg_id;
      int end_id;
   };

   struct DistanceByLine : public DistanceBase
   {
      explicit DistanceByLine () : DistanceBase(DISTANCE_POINT_LINE) {}
      virtual ~DistanceByLine () {}

      int point_id;
      int line_id;
   };

   struct DistanceByPlane : public DistanceBase
   {
      explicit DistanceByPlane () : DistanceBase(DISTANCE_POINT_PLANE) {}
      virtual ~DistanceByPlane () {}

      int point_id;
      int plane_id;
   };

   struct AngleByPoints : public AngleBase
   {
      explicit AngleByPoints () : AngleBase(ANGLE_3POINTS) {}
      virtual ~AngleByPoints () {}

      int point1_id;
      int point2_id;
      int point3_id;
   };

   struct AngleByLines : public AngleBase
   {
      explicit AngleByLines () : AngleBase(ANGLE_2LINES) {}
      virtual ~AngleByLines () {}

      int line1_id;
      int line2_id;
   };

   struct AngleByPlanes : public AngleBase
   {
      explicit AngleByPlanes () : AngleBase(ANGLE_2PLANES) {}
      virtual ~AngleByPlanes () {}

      int plane1_id;
      int plane2_id;
   };

   struct AngleDihedral : public AngleBase
   {
      explicit AngleDihedral () : AngleBase(ANGLE_DIHEDRAL) {}
      virtual ~AngleDihedral () {}

      int point1_id;
      int point2_id;
      int point3_id;
      int point4_id;
   };

   struct ExclusionSphere : public Base
   {
      explicit ExclusionSphere () : Base(EXCLUSION_SPHERE) {}
      virtual ~ExclusionSphere () {}

      int   center_id;
      float radius;
      bool  allow_unconnected;
      Array<int> allowed_atoms;
   };

   Base & add (Base *constraint);

   int begin () const;
   int end   () const;
   int next  (int idx) const;

   const Base & at (int idx) const;

   // takes mapping from supermolecule to submolecule
   void buildOnSubmolecule (const Molecule3dConstraints &super, const int *mapping);

   void removeAtoms (const int *mapping);

   // if have real constraints (not features)
   bool haveConstraints ();

   void clear ();

   DECL_ERROR;

protected:

   QueryMolecule & _getMolecule ();
   PtrArray<Base> _constraints;

   static void _buildSub (PtrArray<Base> &sub, const PtrArray<Base> &super, const int *mapping);

private:
   Molecule3dConstraints (const Molecule3dConstraints &); // no implicit copy
};

class Molecule3dConstraintsChecker 
{
public:
   Molecule3dConstraintsChecker (const Molecule3dConstraints &constraints);

   bool check (BaseMolecule &target, const int *mapping);

   void markUsedAtoms (int *arr, int value);

   DECL_ERROR;
protected:
   void  _cache       (int idx);
   float _getAngle    (int idx);
   float _getDistance (int idx);
   void  _mark        (int idx);

   const Molecule3dConstraints &_constraints;

   // saves a bit of typing
   typedef Molecule3dConstraints MC;

   // can't have comma-containing type names in macro declarations below
   typedef RedBlackMap<int, Vec3f>   MapV;
   typedef RedBlackMap<int, Line3f>  MapL;
   typedef RedBlackMap<int, Plane3f> MapP;

   TL_CP_DECL(MapV, _cache_v);
   TL_CP_DECL(MapL, _cache_l);
   TL_CP_DECL(MapP, _cache_p);

   BaseMolecule *_target;
   const int    *_mapping;

   int *_to_mark;
   int  _mark_value;

   TL_CP_DECL(RedBlackSet<int>, _cache_mark);
};

}

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
