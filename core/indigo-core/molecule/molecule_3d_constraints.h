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

#ifndef __molecule_3d_constraints__
#define __molecule_3d_constraints__

#include <map>

#include "base_cpp/ptr_array.h"
#include "base_cpp/tlscont.h"
#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class BaseMolecule;
    class QueryMolecule;

    class DLLEXPORT Molecule3dConstraints
    {
    public:
        Molecule3dConstraints(QueryMolecule& mol);

        void init();

        enum
        {
            POINT_ATOM = 1,
            POINT_DISTANCE = 2,
            POINT_PERCENTAGE = 3,
            POINT_NORMALE = 4,
            POINT_CENTROID = 5,
            LINE_NORMALE = 6,
            LINE_BEST_FIT = 7,
            PLANE_BEST_FIT = 8,
            PLANE_POINT_LINE = 9,
            ANGLE_3POINTS = 10,
            ANGLE_2LINES = 11,
            ANGLE_2PLANES = 12,
            ANGLE_DIHEDRAL = 13,
            DISTANCE_2POINTS = 14,
            DISTANCE_POINT_LINE = 15,
            DISTANCE_POINT_PLANE = 16,
            EXCLUSION_SPHERE = 17
        };

        struct Base
        {
            explicit Base(int type_) : type(type_)
            {
            }
            virtual ~Base()
            {
            }

            int type;
        };

        struct AngleBase : public Base
        {
        public:
            explicit AngleBase(int type) : Base(type)
            {
            }
            ~AngleBase() override
            {
            }

            float bottom;
            float top;
        };

        struct DistanceBase : public Base
        {
            explicit DistanceBase(int type) : Base(type)
            {
            }
            ~DistanceBase() override
            {
            }

            float bottom;
            float top;
        };

        struct Normale : public Base
        {
            explicit Normale() : Base(LINE_NORMALE)
            {
            }
            ~Normale() override
            {
            }

            int point_id;
            int plane_id;
        };

        struct BestFitLine : public Base
        {
            explicit BestFitLine() : Base(LINE_BEST_FIT)
            {
            }
            ~BestFitLine() override
            {
            }

            float max_deviation;
            Array<int> point_ids;
        };

        struct PointByAtom : public Base
        {
            explicit PointByAtom() : Base(POINT_ATOM)
            {
            }
            ~PointByAtom() override
            {
            }

            int atom_idx;
        };

        struct PointByDistance : public Base
        {
            explicit PointByDistance() : Base(POINT_DISTANCE)
            {
            }
            ~PointByDistance() override
            {
            }

            int beg_id;
            int end_id;
            float distance;
        };

        struct PointByPercentage : public Base
        {
            explicit PointByPercentage() : Base(POINT_PERCENTAGE)
            {
            }
            ~PointByPercentage() override
            {
            }

            int beg_id;
            int end_id;
            float percentage;
        };

        struct PointByNormale : public Base
        {
            explicit PointByNormale() : Base(POINT_NORMALE)
            {
            }
            ~PointByNormale() override
            {
            }

            int org_id;
            int norm_id;
            float distance;
        };

        struct Centroid : public Base
        {
            explicit Centroid() : Base(POINT_CENTROID)
            {
            }
            ~Centroid() override
            {
            }

            Array<int> point_ids;
        };

        struct BestFitPlane : public Base
        {
            explicit BestFitPlane() : Base(PLANE_BEST_FIT)
            {
            }
            ~BestFitPlane() override
            {
            }

            float max_deviation;
            Array<int> point_ids;
        };

        struct PlaneByPoint : public Base
        {
            explicit PlaneByPoint() : Base(PLANE_POINT_LINE)
            {
            }
            ~PlaneByPoint() override
            {
            }

            int point_id;
            int line_id;
        };

        struct DistanceByPoints : public DistanceBase
        {
            explicit DistanceByPoints() : DistanceBase(DISTANCE_2POINTS)
            {
            }
            ~DistanceByPoints() override
            {
            }

            int beg_id;
            int end_id;
        };

        struct DistanceByLine : public DistanceBase
        {
            explicit DistanceByLine() : DistanceBase(DISTANCE_POINT_LINE)
            {
            }
            ~DistanceByLine() override
            {
            }

            int point_id;
            int line_id;
        };

        struct DistanceByPlane : public DistanceBase
        {
            explicit DistanceByPlane() : DistanceBase(DISTANCE_POINT_PLANE)
            {
            }
            ~DistanceByPlane() override
            {
            }

            int point_id;
            int plane_id;
        };

        struct AngleByPoints : public AngleBase
        {
            explicit AngleByPoints() : AngleBase(ANGLE_3POINTS)
            {
            }
            ~AngleByPoints() override
            {
            }

            int point1_id;
            int point2_id;
            int point3_id;
        };

        struct AngleByLines : public AngleBase
        {
            explicit AngleByLines() : AngleBase(ANGLE_2LINES)
            {
            }
            ~AngleByLines() override
            {
            }

            int line1_id;
            int line2_id;
        };

        struct AngleByPlanes : public AngleBase
        {
            explicit AngleByPlanes() : AngleBase(ANGLE_2PLANES)
            {
            }
            ~AngleByPlanes() override
            {
            }

            int plane1_id;
            int plane2_id;
        };

        struct AngleDihedral : public AngleBase
        {
            explicit AngleDihedral() : AngleBase(ANGLE_DIHEDRAL)
            {
            }
            ~AngleDihedral() override
            {
            }

            int point1_id;
            int point2_id;
            int point3_id;
            int point4_id;
        };

        struct ExclusionSphere : public Base
        {
            explicit ExclusionSphere() : Base(EXCLUSION_SPHERE)
            {
            }
            ~ExclusionSphere() override
            {
            }

            int center_id;
            float radius;
            bool allow_unconnected;
            Array<int> allowed_atoms;
        };

        Base& add(Base* constraint);

        int begin() const;
        int end() const;
        int next(int idx) const;

        const Base& at(int idx) const;

        // takes mapping from supermolecule to submolecule
        void buildOnSubmolecule(const Molecule3dConstraints& super, const int* mapping);

        void removeAtoms(const int* mapping);

        // if have real constraints (not features)
        bool haveConstraints();

        void clear();

        DECL_ERROR;

    protected:
        QueryMolecule& _mol;
        PtrArray<Base> _constraints;

        static void _buildSub(PtrArray<Base>& sub, const PtrArray<Base>& super, const int* mapping);

    private:
        Molecule3dConstraints(const Molecule3dConstraints&); // no implicit copy
    };

    class Molecule3dConstraintsChecker
    {
    public:
        Molecule3dConstraintsChecker(const Molecule3dConstraints& constraints);

        bool check(BaseMolecule& target, const int* mapping);

        void markUsedAtoms(int* arr, int value);

        DECL_ERROR;

    protected:
        void _cache(int idx);
        float _getAngle(int idx);
        float _getDistance(int idx);
        void _mark(int idx);

        const Molecule3dConstraints& _constraints;

        // saves a bit of typing
        typedef Molecule3dConstraints MC;

        // can't have comma-containing type names in macro declarations below
        typedef std::map<int, Vec3f> MapV;
        typedef std::map<int, Line3f> MapL;
        typedef std::map<int, Plane3f> MapP;

        CP_DECL;
        TL_CP_DECL(MapV, _cache_v);
        TL_CP_DECL(MapL, _cache_l);
        TL_CP_DECL(MapP, _cache_p);

        BaseMolecule* _target;
        const int* _mapping;

        int* _to_mark;
        int _mark_value;

        TL_CP_DECL(RedBlackSet<int>, _cache_mark);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
