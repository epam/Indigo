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

#include "molecule/molecule_3d_constraints.h"

#include "base_cpp/red_black.h"
#include "molecule/query_molecule.h"

using namespace indigo;

IMPL_ERROR(Molecule3dConstraints, "molecule 3d constraints");

Molecule3dConstraints::Molecule3dConstraints(QueryMolecule& mol) : _mol(mol)
{
}

void Molecule3dConstraints::init()
{
    for (int i : _mol.vertices())
    {
        std::unique_ptr<PointByAtom> constr = std::make_unique<PointByAtom>();
        constr->atom_idx = i;
        _constraints.add(constr.release());
    }
}

int Molecule3dConstraints::begin() const
{
    return 0;
}

int Molecule3dConstraints::end() const
{
    return _constraints.size();
}

int Molecule3dConstraints::next(int idx) const
{
    return idx + 1;
}

const Molecule3dConstraints::Base& Molecule3dConstraints::at(int idx) const
{
    return *_constraints[idx];
}

Molecule3dConstraints::Base& Molecule3dConstraints::add(Molecule3dConstraints::Base* constraint)
{
    return _constraints.add(constraint);
}

bool Molecule3dConstraints::haveConstraints()
{
    for (int i = 0; i < _constraints.size(); i++)
    {
        const Base& base = *_constraints.at(i);

        switch (base.type)
        {
        case ANGLE_2LINES:
        case ANGLE_DIHEDRAL:
        case ANGLE_2PLANES:
        case ANGLE_3POINTS:
        case DISTANCE_2POINTS:
        case DISTANCE_POINT_LINE:
        case DISTANCE_POINT_PLANE:
        case EXCLUSION_SPHERE:
        case LINE_BEST_FIT:
        case PLANE_BEST_FIT:
            return true;
        }
    }

    return false;
}

void Molecule3dConstraints::_buildSub(PtrArray<Base>& sub, const PtrArray<Base>& super, const int* mapping)
{
    QS_DEF(Array<int>, cmapping); // mapping of constraints from supermolecule to submolecule
    int i, j;

    cmapping.resize(super.size());
    for (i = 0; i < super.size(); i++)
        cmapping[i] = -1;

    sub.clear();

    do
    {
        for (i = 0; i < super.size(); i++)
        {
            int oldsize = sub.size();

            if (cmapping[i] >= 0)
                continue;

            const Base& base = *super.at(i);

            switch (base.type)
            {
            case POINT_ATOM: {
                int atom_idx = ((const Molecule3dConstraints::PointByAtom&)base).atom_idx;

                if (mapping[atom_idx] < 0)
                    continue;

                std::unique_ptr<PointByAtom> newconstr = std::make_unique<PointByAtom>();
                newconstr->atom_idx = mapping[atom_idx];

                sub.add(newconstr.release());
                break;
            }
            case POINT_DISTANCE: {
                const PointByDistance& constr = (const PointByDistance&)base;

                int beg_id = cmapping[constr.beg_id];
                int end_id = cmapping[constr.end_id];

                if (beg_id < 0 || end_id < 0)
                    continue;

                std::unique_ptr<PointByDistance> newconstr = std::make_unique<PointByDistance>();
                newconstr->beg_id = beg_id;
                newconstr->end_id = end_id;
                newconstr->distance = constr.distance;

                sub.add(newconstr.release());
                break;
            }
            case POINT_PERCENTAGE: {
                const PointByPercentage& constr = (const PointByPercentage&)base;

                int beg_id = cmapping[constr.beg_id];
                int end_id = cmapping[constr.end_id];

                if (beg_id < 0 || end_id < 0)
                    continue;

                std::unique_ptr<PointByPercentage> newconstr = std::make_unique<PointByPercentage>();
                newconstr->beg_id = beg_id;
                newconstr->end_id = end_id;
                newconstr->percentage = constr.percentage;
                sub.add(newconstr.release());
                break;
            }
            case POINT_NORMALE: {
                const PointByNormale& constr = (const PointByNormale&)base;

                int org_id = cmapping[constr.org_id];
                int norm_id = cmapping[constr.norm_id];

                if (org_id < 0 || norm_id < 0)
                    continue;

                std::unique_ptr<PointByNormale> newconstr = std::make_unique<PointByNormale>();
                newconstr->norm_id = norm_id;
                newconstr->org_id = org_id;
                newconstr->distance = constr.distance;
                sub.add(newconstr.release());
                break;
            }
            case POINT_CENTROID: {
                const Centroid& constr = (const Centroid&)base;

                std::unique_ptr<Centroid> newconstr = std::make_unique<Centroid>();

                for (j = 0; j < constr.point_ids.size(); j++)
                {
                    int pt_idx = cmapping[constr.point_ids[j]];

                    if (pt_idx < 0)
                        break;

                    newconstr->point_ids.push(pt_idx);
                }

                if (newconstr->point_ids.size() < constr.point_ids.size())
                    continue;

                sub.add(newconstr.release());
                break;
            }
            case LINE_NORMALE: {
                const Normale& constr = (const Normale&)base;

                int plane_id = cmapping[constr.plane_id];
                int point_id = cmapping[constr.point_id];

                if (plane_id < 0 || point_id < 0)
                    continue;

                std::unique_ptr<Normale> newconstr = std::make_unique<Normale>();
                newconstr->plane_id = plane_id;
                newconstr->point_id = point_id;

                sub.add(newconstr.release());
                break;
            }
            case LINE_BEST_FIT: {
                const BestFitLine& constr = (const BestFitLine&)base;

                std::unique_ptr<BestFitLine> newconstr = std::make_unique<BestFitLine>();
                for (j = 0; j < constr.point_ids.size(); j++)
                {
                    int pt_idx = cmapping[constr.point_ids[j]];

                    if (pt_idx < 0)
                        break;

                    newconstr->point_ids.push(pt_idx);
                }

                if (newconstr->point_ids.size() < constr.point_ids.size())
                    continue;

                newconstr->max_deviation = constr.max_deviation;
                sub.add(newconstr.release());
                break;
            }
            case PLANE_BEST_FIT: {
                const BestFitPlane& constr = (const BestFitPlane&)base;

                std::unique_ptr<BestFitPlane> newconstr = std::make_unique<BestFitPlane>();
                for (j = 0; j < constr.point_ids.size(); j++)
                {
                    int pt_idx = cmapping[constr.point_ids[j]];

                    if (pt_idx < 0)
                        break;

                    newconstr->point_ids.push(pt_idx);
                }

                if (newconstr->point_ids.size() < constr.point_ids.size())
                    continue;

                newconstr->max_deviation = constr.max_deviation;
                sub.add(newconstr.release());
                break;
            }
            case PLANE_POINT_LINE: {
                const PlaneByPoint& constr = (const PlaneByPoint&)base;

                int point_id = cmapping[constr.point_id];
                int line_id = cmapping[constr.line_id];

                if (line_id < 0 || point_id < 0)
                    continue;

                std::unique_ptr<PlaneByPoint> newconstr = std::make_unique<PlaneByPoint>();
                newconstr->line_id = line_id;
                newconstr->point_id = point_id;
                sub.add(newconstr.release());
                break;
            }
            case ANGLE_3POINTS: {
                const AngleByPoints& constr = (const AngleByPoints&)base;

                int point1_id = cmapping[constr.point1_id];
                int point2_id = cmapping[constr.point2_id];
                int point3_id = cmapping[constr.point3_id];

                if (point1_id < 0 || point2_id < 0 || point3_id < 0)
                    continue;

                std::unique_ptr<AngleByPoints> newconstr = std::make_unique<AngleByPoints>();
                newconstr->point1_id = point1_id;
                newconstr->point2_id = point2_id;
                newconstr->point3_id = point3_id;

                newconstr->bottom = constr.bottom;
                newconstr->top = constr.top;
                sub.add(newconstr.release());
                break;
            }
            case ANGLE_2LINES: {
                const AngleByLines& constr = (const AngleByLines&)base;

                int line1_id = cmapping[constr.line1_id];
                int line2_id = cmapping[constr.line2_id];

                if (line1_id < 0 || line2_id < 0)
                    continue;

                std::unique_ptr<AngleByLines> newconstr = std::make_unique<AngleByLines>();
                newconstr->line1_id = line1_id;
                newconstr->line2_id = line2_id;

                newconstr->bottom = constr.bottom;
                newconstr->top = constr.top;
                sub.add(newconstr.release());
                break;
            }
            case ANGLE_2PLANES: {
                const AngleByPlanes& constr = (const AngleByPlanes&)base;

                int plane1_id = cmapping[constr.plane1_id];
                int plane2_id = cmapping[constr.plane2_id];

                if (plane1_id < 0 || plane2_id < 0)
                    continue;

                std::unique_ptr<AngleByPlanes> newconstr = std::make_unique<AngleByPlanes>();
                newconstr->plane1_id = plane1_id;
                newconstr->plane2_id = plane2_id;

                newconstr->bottom = constr.bottom;
                newconstr->top = constr.top;
                sub.add(newconstr.release());
                break;
            }
            case ANGLE_DIHEDRAL: {
                const AngleDihedral& constr = (const AngleDihedral&)base;

                int point1_id = cmapping[constr.point1_id];
                int point2_id = cmapping[constr.point2_id];
                int point3_id = cmapping[constr.point3_id];
                int point4_id = cmapping[constr.point4_id];

                if (point1_id < 0 || point2_id < 0 || point3_id < 0 || point4_id < 0)
                    continue;

                std::unique_ptr<AngleDihedral> newconstr = std::make_unique<AngleDihedral>();
                newconstr->point1_id = point1_id;
                newconstr->point2_id = point2_id;
                newconstr->point3_id = point3_id;
                newconstr->point4_id = point4_id;

                newconstr->bottom = constr.bottom;
                newconstr->top = constr.top;
                sub.add(newconstr.release());
                break;
            }
            case DISTANCE_2POINTS: {
                const DistanceByPoints& constr = (const DistanceByPoints&)base;

                int beg_id = cmapping[constr.beg_id];
                int end_id = cmapping[constr.end_id];

                if (beg_id < 0 || end_id < 0)
                    continue;

                std::unique_ptr<DistanceByPoints> newconstr = std::make_unique<DistanceByPoints>();
                newconstr->beg_id = beg_id;
                newconstr->end_id = end_id;
                newconstr->bottom = constr.bottom;
                newconstr->top = constr.top;
                sub.add(newconstr.release());
                break;
            }
            case DISTANCE_POINT_LINE: {
                const DistanceByLine& constr = (const DistanceByLine&)base;

                int point_id = cmapping[constr.point_id];
                int line_id = cmapping[constr.line_id];

                if (line_id < 0 || point_id < 0)
                    continue;

                std::unique_ptr<DistanceByLine> newconstr = std::make_unique<DistanceByLine>();
                newconstr->line_id = line_id;
                newconstr->point_id = point_id;
                newconstr->top = constr.top;
                newconstr->bottom = constr.bottom;
                sub.add(newconstr.release());
                break;
            }
            case DISTANCE_POINT_PLANE: {
                const DistanceByPlane& constr = (const DistanceByPlane&)base;

                int plane_id = cmapping[constr.plane_id];
                int point_id = cmapping[constr.point_id];

                if (plane_id < 0 || point_id < 0)
                    continue;

                std::unique_ptr<DistanceByPlane> newconstr = std::make_unique<DistanceByPlane>();
                newconstr->plane_id = plane_id;
                newconstr->point_id = point_id;
                newconstr->bottom = constr.bottom;
                newconstr->top = constr.top;
                sub.add(newconstr.release());
                break;
            }
            case EXCLUSION_SPHERE: {
                const ExclusionSphere& constr = (const ExclusionSphere&)base;

                int center_id = cmapping[constr.center_id];

                if (center_id < 0)
                    continue;

                std::unique_ptr<ExclusionSphere> newconstr = std::make_unique<ExclusionSphere>();
                for (j = 0; j < constr.allowed_atoms.size(); j++)
                {
                    int atom_idx = mapping[constr.allowed_atoms[j]];

                    if (atom_idx >= 0)
                        newconstr->allowed_atoms.push(atom_idx);
                }

                newconstr->center_id = center_id;
                newconstr->allow_unconnected = constr.allow_unconnected;
                newconstr->radius = constr.radius;
                sub.add(newconstr.release());
                break;
            }
            default:
                throw Error("build on submolecule: unknown feature %d", base.type);
            }
            cmapping[i] = oldsize;
        }
        if (i == super.size())
            break;
    } while (1);
}

void Molecule3dConstraints::buildOnSubmolecule(const Molecule3dConstraints& super, const int* mapping)
{
    _buildSub(_constraints, super._constraints, mapping);
}

void Molecule3dConstraints::removeAtoms(const int* mapping)
{
    PtrArray<Base> new_constraints;
    int i;

    _buildSub(new_constraints, _constraints, mapping);

    _constraints.clear();

    for (i = 0; i < new_constraints.size(); i++)
    {
        _constraints.add(new_constraints.at(i));
        new_constraints.release(i);
    }
}

IMPL_ERROR(Molecule3dConstraintsChecker, "molecule 3d constraints checker");

CP_DEF(Molecule3dConstraintsChecker);

Molecule3dConstraintsChecker::Molecule3dConstraintsChecker(const Molecule3dConstraints& constraints)
    : _constraints(constraints), CP_INIT, TL_CP_GET(_cache_v), TL_CP_GET(_cache_l), TL_CP_GET(_cache_p), TL_CP_GET(_cache_mark)
{
}

bool Molecule3dConstraintsChecker::check(BaseMolecule& target, const int* mapping)
{
    _cache_l.clear();
    _cache_p.clear();
    _cache_v.clear();
    _cache_mark.clear();

    _target = &target;
    _mapping = mapping;

    for (int i = _constraints.begin(); i != _constraints.end(); i = _constraints.next(i))
    {
        const MC::Base& base = _constraints.at(i);

        switch (base.type)
        {
        case MC::ANGLE_DIHEDRAL:
        case MC::ANGLE_3POINTS: {
            float value = _getAngle(i);
            const MC::AngleBase& constr = (const MC::AngleBase&)base;

            if (value < constr.bottom || value > constr.top)
                return false;
            break;
        }
        case MC::ANGLE_2LINES:
        case MC::ANGLE_2PLANES: {
            float value = _getAngle(i);
            const MC::AngleBase& constr = (const MC::AngleBase&)base;

            if ((value < constr.bottom || value > constr.top) && (M_PI - value < constr.bottom || M_PI - value > constr.top))
                return false;
            break;
        }
        case MC::DISTANCE_2POINTS:
        case MC::DISTANCE_POINT_LINE:
        case MC::DISTANCE_POINT_PLANE: {
            float value = _getDistance(i);
            const MC::DistanceBase& constr = (const MC::DistanceBase&)base;

            if (value < constr.bottom || value > constr.top)
                return false;
            break;
        }
        case MC::EXCLUSION_SPHERE: {
            const MC::ExclusionSphere& constr = (const MC::ExclusionSphere&)base;

            _cache(constr.center_id);

            const Vec3f& center = _cache_v.at(constr.center_id);
            QS_DEF(Array<int>, allowed);
            int i;

            allowed.clear_resize(_target->vertexCount());
            allowed.zerofill();

            for (i = 0; i < constr.allowed_atoms.size(); i++)
                allowed[_mapping[constr.allowed_atoms[i]]] = 1;

            for (i = _target->vertexBegin(); i < _target->vertexEnd(); i = _target->vertexNext(i))
            {
                if (allowed[i])
                    continue;

                if (constr.allow_unconnected && _target->getVertex(i).degree() < 1)
                    continue;

                const Vec3f& pos = _target->getAtomXyz(i);

                if (Vec3f::dist(pos, center) < constr.radius - EPSILON)
                    return false;
            }

            break;
        }
        case MC::LINE_BEST_FIT: {
            _cache(i);

            const Line3f& bfl = _cache_l.at(i);
            const MC::BestFitLine& constr = (const MC::BestFitLine&)base;
            float rms = .0f, dist = .0f;

            for (int i = 0; i < constr.point_ids.size(); i++)
            {
                dist = bfl.distFromPoint(_cache_v.at(constr.point_ids[i]));
                rms += dist * dist;
            }
            if (rms > constr.max_deviation + 1e-6)
                return false;

            break;
        }
        case MC::PLANE_BEST_FIT: {
            _cache(i);

            const Plane3f& bfp = _cache_p.at(i);
            const MC::BestFitPlane& constr = (const MC::BestFitPlane&)base;
            float rms = .0f, dist = .0f;

            for (int i = 0; i < constr.point_ids.size(); i++)
            {
                dist = bfp.distFromPoint(_cache_v.at(constr.point_ids[i]));
                rms += dist * dist;
            }
            if (rms > constr.max_deviation + 1e-6)
                return false;

            break;
        }
        }
    }
    return true;
}

void Molecule3dConstraintsChecker::_cache(int idx)
{
    if (_cache_v.find(idx) != _cache_v.end() || _cache_l.find(idx) != _cache_l.end() || _cache_p.find(idx) != _cache_p.end())
    {
        return;
    }

    const MC::Base& base = _constraints.at(idx);

    switch (base.type)
    {
    case MC::POINT_ATOM: {
        int atom_idx = ((const Molecule3dConstraints::PointByAtom&)base).atom_idx;

        _cache_v.emplace(idx, _target->getAtomXyz(_mapping[atom_idx]));
        break;
    }
    case MC::POINT_DISTANCE: {
        const MC::PointByDistance& constr = (const MC::PointByDistance&)base;

        _cache(constr.beg_id);
        _cache(constr.end_id);

        const Vec3f& beg = _cache_v.at(constr.beg_id);
        const Vec3f& end = _cache_v.at(constr.end_id);
        Vec3f dir;

        dir.diff(end, beg);

        if (!dir.normalize())
            throw Error("point-by-distance: degenerate case");

        Vec3f res;

        res.lineCombin(beg, dir, constr.distance);

        _cache_v.emplace(idx, res);
        break;
    }
    case MC::POINT_PERCENTAGE: {
        const MC::PointByPercentage& constr = (const MC::PointByPercentage&)base;

        _cache(constr.beg_id);
        _cache(constr.end_id);

        const Vec3f& beg = _cache_v.at(constr.beg_id);
        const Vec3f& end = _cache_v.at(constr.end_id);
        Vec3f dir;

        dir.diff(end, beg);

        if (!dir.normalize())
            throw Error("point-by-percentage: degenerate case");

        Vec3f res;

        res.lineCombin2(beg, 1.f - constr.percentage, end, constr.percentage);

        _cache_v.emplace(idx, res);
        break;
    }
    case MC::POINT_NORMALE: {
        const MC::PointByNormale& constr = (const MC::PointByNormale&)base;

        _cache(constr.org_id);
        _cache(constr.norm_id);

        const Vec3f& org = _cache_v.at(constr.org_id);
        const Line3f& norm = _cache_l.at(constr.norm_id);

        Vec3f res;

        res.lineCombin(org, norm.dir, constr.distance);
        _cache_v.emplace(idx, res);
        break;
    }
    case MC::POINT_CENTROID: {
        const MC::Centroid& constr = (const MC::Centroid&)base;

        Vec3f res;

        if (constr.point_ids.size() < 1)
            throw Error("centroid: have %d points", constr.point_ids.size());

        for (int i = 0; i < constr.point_ids.size(); i++)
        {
            _cache(constr.point_ids[i]);

            const Vec3f& pt = _cache_v.at(constr.point_ids[i]);

            res.add(pt);
        }

        res.scale(1.f / constr.point_ids.size());
        _cache_v.emplace(idx, res);
        break;
    }
    case MC::LINE_NORMALE: {
        const MC::Normale& constr = (const MC::Normale&)base;

        _cache(constr.plane_id);
        _cache(constr.point_id);

        const Plane3f& plane = _cache_p.at(constr.plane_id);
        const Vec3f& point = _cache_v.at(constr.point_id);

        Vec3f projection;
        Line3f res;

        plane.projection(point, projection);

        res.dir.copy(plane.getNorm());
        res.org.copy(projection);

        _cache_l.emplace(idx, res);
        break;
    }
    case MC::LINE_BEST_FIT: {
        const MC::BestFitLine& constr = (const MC::BestFitLine&)base;

        if (constr.point_ids.size() < 2)
            throw Error("best fit line: only %d points", constr.point_ids.size());

        QS_DEF(Array<Vec3f>, points);

        points.clear();
        for (int i = 0; i < constr.point_ids.size(); i++)
        {
            _cache(constr.point_ids[i]);
            points.push(_cache_v.at(constr.point_ids[i]));
        }

        Line3f res;

        res.bestFit(points.size(), points.ptr(), 0);

        _cache_l.emplace(idx, res);
        break;
    }
    case MC::PLANE_BEST_FIT: {
        const MC::BestFitPlane& constr = (const MC::BestFitPlane&)base;

        if (constr.point_ids.size() < 3)
            throw Error("best fit line: only %d points", constr.point_ids.size());

        QS_DEF(Array<Vec3f>, points);

        points.clear();
        for (int i = 0; i < constr.point_ids.size(); i++)
        {
            _cache(constr.point_ids[i]);
            points.push(_cache_v.at(constr.point_ids[i]));
        }

        Plane3f res;

        res.bestFit(points.size(), points.ptr(), 0);

        _cache_p.emplace(idx, res);
        break;
    }
    case MC::PLANE_POINT_LINE: {
        const MC::PlaneByPoint& constr = (const MC::PlaneByPoint&)base;

        _cache(constr.point_id);
        _cache(constr.line_id);

        const Vec3f& point = _cache_v.at(constr.point_id);
        const Line3f& line = _cache_l.at(constr.line_id);

        Plane3f res;

        res.byPointAndLine(point, line);

        _cache_p.emplace(idx, res);
        break;
    }
    default:
        throw Error("unknown constraint type %d", base.type);
    }
}

float Molecule3dConstraintsChecker::_getAngle(int idx)
{
    const MC::Base& base = _constraints.at(idx);

    switch (base.type)
    {
    case MC::ANGLE_3POINTS: {
        const MC::AngleByPoints& constr = (const MC::AngleByPoints&)base;

        _cache(constr.point1_id);
        _cache(constr.point2_id);
        _cache(constr.point3_id);

        const Vec3f& v1 = _cache_v.at(constr.point1_id);
        const Vec3f& v2 = _cache_v.at(constr.point2_id);
        const Vec3f& v3 = _cache_v.at(constr.point3_id);

        Vec3f dir1, dir3;

        dir1.diff(v1, v2);
        dir3.diff(v3, v2);

        float ang;

        if (!Vec3f::angle(dir1, dir3, ang))
            throw Error("angle by points: degerenate");

        return ang;
    }
    case MC::ANGLE_2LINES: {
        const MC::AngleByLines& constr = (const MC::AngleByLines&)base;

        _cache(constr.line1_id);
        _cache(constr.line2_id);

        const Line3f& line1 = _cache_l.at(constr.line1_id);
        const Line3f& line2 = _cache_l.at(constr.line2_id);

        float ang;

        if (!Vec3f::angle(line1.dir, line2.dir, ang))
            throw Error("angle by lines: degerenate");

        return ang;
    }
    case MC::ANGLE_2PLANES: {
        const MC::AngleByPlanes& constr = (const MC::AngleByPlanes&)base;

        _cache(constr.plane1_id);
        _cache(constr.plane2_id);

        const Plane3f& plane1 = _cache_p.at(constr.plane1_id);
        const Plane3f& plane2 = _cache_p.at(constr.plane2_id);

        float ang;

        if (!Vec3f::angle(plane1.getNorm(), plane2.getNorm(), ang))
            throw Error("angle by planes: degerenate");

        return ang;
    }
    case MC::ANGLE_DIHEDRAL: {
        const MC::AngleDihedral& constr = (const MC::AngleDihedral&)base;

        _cache(constr.point1_id);
        _cache(constr.point2_id);
        _cache(constr.point3_id);
        _cache(constr.point4_id);

        const Vec3f& v1 = _cache_v.at(constr.point1_id);
        const Vec3f& v2 = _cache_v.at(constr.point2_id);
        const Vec3f& v3 = _cache_v.at(constr.point3_id);
        const Vec3f& v4 = _cache_v.at(constr.point4_id);

        Vec3f d1, d2, axis;

        d1.diff(v2, v1);
        d2.diff(v3, v4);
        axis.diff(v2, v3);

        if (!axis.normalize())
            throw Error("dihedral angle: degenerate axis");

        d1.addScaled(axis, -Vec3f::dot(d1, axis));
        d2.addScaled(axis, -Vec3f::dot(d2, axis));

        float ang;

        if (!Vec3f::angle(d1, d2, ang))
            throw Error("dihedral angle: degenerate");

        return ang;
    }
    default:
        throw Error("get angle: bad constraint type %d", base.type);
    }
}

float Molecule3dConstraintsChecker::_getDistance(int idx)
{
    const MC::Base& base = _constraints.at(idx);

    switch (base.type)
    {
    case MC::DISTANCE_2POINTS: {
        const MC::DistanceByPoints& constr = (const MC::DistanceByPoints&)base;

        _cache(constr.beg_id);
        _cache(constr.end_id);

        const Vec3f& beg = _cache_v.at(constr.beg_id);
        const Vec3f& end = _cache_v.at(constr.end_id);

        return Vec3f::dist(beg, end);
    }
    case MC::DISTANCE_POINT_LINE: {
        const MC::DistanceByLine& constr = (const MC::DistanceByLine&)base;

        _cache(constr.line_id);
        _cache(constr.point_id);

        const Vec3f& point = _cache_v.at(constr.point_id);
        const Line3f& line = _cache_l.at(constr.line_id);

        return line.distFromPoint(point);
    }
    case MC::DISTANCE_POINT_PLANE: {
        const MC::DistanceByPlane& constr = (const MC::DistanceByPlane&)base;

        _cache(constr.plane_id);
        _cache(constr.point_id);

        const Vec3f& point = _cache_v.at(constr.point_id);
        const Plane3f& plane = _cache_p.at(constr.plane_id);

        return plane.distFromPoint(point);
    }
    default:
        throw Error("get distance: bad constraint type %d", base.type);
    }
}

void Molecule3dConstraintsChecker::markUsedAtoms(int* arr, int value)
{
    int i;

    _to_mark = arr;
    _mark_value = value;

    for (i = _constraints.begin(); i != _constraints.end(); i = _constraints.next(i))
    {
        const MC::Base& base = _constraints.at(i);

        switch (base.type)
        {
        case MC::ANGLE_2LINES:
        case MC::ANGLE_DIHEDRAL:
        case MC::ANGLE_2PLANES:
        case MC::ANGLE_3POINTS:
        case MC::DISTANCE_2POINTS:
        case MC::DISTANCE_POINT_LINE:
        case MC::DISTANCE_POINT_PLANE:
        case MC::EXCLUSION_SPHERE: {
            _mark(i);
            break;
        }
        }
    }
}

void Molecule3dConstraintsChecker::_mark(int idx)
{
    if (_cache_mark.find(idx))
        return;

    _cache_mark.insert(idx);

    const MC::Base& base = _constraints.at(idx);

    switch (base.type)
    {
    case MC::POINT_ATOM: {
        int atom_idx = ((const Molecule3dConstraints::PointByAtom&)base).atom_idx;

        _to_mark[atom_idx] = _mark_value;
        break;
    }
    case MC::POINT_DISTANCE: {
        const MC::PointByDistance& constr = (const MC::PointByDistance&)base;

        _mark(constr.beg_id);
        _mark(constr.end_id);
        break;
    }
    case MC::POINT_PERCENTAGE: {
        const MC::PointByPercentage& constr = (const MC::PointByPercentage&)base;

        _mark(constr.beg_id);
        _mark(constr.end_id);
        break;
    }
    case MC::POINT_NORMALE: {
        const MC::PointByNormale& constr = (const MC::PointByNormale&)base;

        _mark(constr.org_id);
        _mark(constr.norm_id);
        break;
    }
    case MC::POINT_CENTROID: {
        const MC::Centroid& constr = (const MC::Centroid&)base;

        for (int i = 0; i < constr.point_ids.size(); i++)
            _mark(constr.point_ids[i]);

        break;
    }
    case MC::LINE_NORMALE: {
        const MC::Normale& constr = (const MC::Normale&)base;

        _mark(constr.plane_id);
        _mark(constr.point_id);
        break;
    }
    case MC::LINE_BEST_FIT: {
        const MC::BestFitLine& constr = (const MC::BestFitLine&)base;

        for (int i = 0; i < constr.point_ids.size(); i++)
            _mark(constr.point_ids[i]);

        break;
    }
    case MC::PLANE_BEST_FIT: {
        const MC::BestFitPlane& constr = (const MC::BestFitPlane&)base;

        for (int i = 0; i < constr.point_ids.size(); i++)
            _mark(constr.point_ids[i]);

        break;
    }
    case MC::PLANE_POINT_LINE: {
        const MC::PlaneByPoint& constr = (const MC::PlaneByPoint&)base;

        _mark(constr.point_id);
        _mark(constr.line_id);

        break;
    }
    case MC::ANGLE_3POINTS: {
        const MC::AngleByPoints& constr = (const MC::AngleByPoints&)base;

        _mark(constr.point1_id);
        _mark(constr.point2_id);
        _mark(constr.point3_id);
        break;
    }
    case MC::ANGLE_2LINES: {
        const MC::AngleByLines& constr = (const MC::AngleByLines&)base;

        _mark(constr.line1_id);
        _mark(constr.line2_id);
        break;
    }
    case MC::ANGLE_2PLANES: {
        const MC::AngleByPlanes& constr = (const MC::AngleByPlanes&)base;

        _mark(constr.plane1_id);
        _mark(constr.plane2_id);
        break;
    }
    case MC::ANGLE_DIHEDRAL: {
        const MC::AngleDihedral& constr = (const MC::AngleDihedral&)base;

        _mark(constr.point1_id);
        _mark(constr.point2_id);
        _mark(constr.point3_id);
        _mark(constr.point4_id);
        break;
    }
    case MC::DISTANCE_2POINTS: {
        const MC::DistanceByPoints& constr = (const MC::DistanceByPoints&)base;

        _mark(constr.beg_id);
        _mark(constr.end_id);
        break;
    }
    case MC::DISTANCE_POINT_LINE: {
        const MC::DistanceByLine& constr = (const MC::DistanceByLine&)base;

        _mark(constr.line_id);
        _mark(constr.point_id);
        break;
    }
    case MC::DISTANCE_POINT_PLANE: {
        const MC::DistanceByPlane& constr = (const MC::DistanceByPlane&)base;

        _mark(constr.plane_id);
        _mark(constr.point_id);
        break;
    }
    case MC::EXCLUSION_SPHERE: {
        const MC::ExclusionSphere& constr = (const MC::ExclusionSphere&)base;

        _mark(constr.center_id);
        break;
    }
    }
}

void Molecule3dConstraints::clear()
{
    _constraints.clear();
}
