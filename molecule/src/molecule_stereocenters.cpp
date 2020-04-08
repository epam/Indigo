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

#include "molecule/molecule_stereocenters.h"
#include "graph/filter.h"
#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/haworth_projection_finder.h"
#include "molecule/molecule.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_stereocenter_options.h"

using namespace indigo;

IMPL_ERROR(MoleculeStereocenters, "stereocenters");

MoleculeStereocenters::MoleculeStereocenters()
{
}

BaseMolecule& MoleculeStereocenters::_getMolecule() const
{
    char dummy[sizeof(BaseMolecule)];

    int offset = (int)((char*)(&((BaseMolecule*)dummy)->stereocenters) - dummy);

    return *(BaseMolecule*)((char*)this - offset);
}

void MoleculeStereocenters::clear()
{
    _stereocenters.clear();
}

void MoleculeStereocenters::buildFromBonds(const StereocentersOptions& options, int* sensible_bonds_out)
{
    BaseMolecule& mol = _getMolecule();
    HaworthProjectionFinder haworth_finder(mol);
    if (options.detect_haworth_projection)
        haworth_finder.findAndAddStereocenters();

    const Array<bool>& bonds_ignore = haworth_finder.getBondsMask();
    const Array<bool>& atoms_ignore = haworth_finder.getAtomsMask();
    for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        if (bonds_ignore[i] && mol.getBondDirection(i))
            sensible_bonds_out[i] = 1;
    }

    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (atoms_ignore[i])
            continue;

        // Try to build sterecenters with bidirectional_mode only for either bonds
        bool found = false;
        try
        {
            found = _buildOneCenter(i, sensible_bonds_out, false, options.bidirectional_mode, bonds_ignore);
        }
        catch (Error&)
        {
            if (!options.ignore_errors)
                throw;
        }

        // Try to build a stereocenter with bidirectional_mode for all bonds
        // but ignore any errors that occur because such bidirection mode has low
        // priority
        if (options.bidirectional_mode && !found)
        {
            try
            {
                _buildOneCenter(i, sensible_bonds_out, true, options.bidirectional_mode, bonds_ignore);
            }
            catch (Error&)
            {
            }
        }
    }
}

void MoleculeStereocenters::buildFrom3dCoordinates(const StereocentersOptions& options)
{
    BaseMolecule& bmol = _getMolecule();

    if (bmol.isQueryMolecule())
        return;

    Molecule& mol = bmol.asMolecule();

    if (!BaseMolecule::hasZCoord(mol))
        return;

    _stereocenters.clear();

    int i;

    for (i = bmol.vertexBegin(); i != bmol.vertexEnd(); i = bmol.vertexNext(i))
    {
        try
        {
            _buildOneFrom3dCoordinates(i);
        }
        catch (Error&)
        {
            if (!options.ignore_errors)
                throw;
        }
    }

    MoleculeAutomorphismSearch am;
    am.detect_invalid_stereocenters = true;
    am.allow_undefined = true;
    am.process(mol);

    for (i = bmol.vertexBegin(); i != bmol.vertexEnd(); i = bmol.vertexNext(i))
    {
        if (!bmol.stereocenters.exists(i))
            continue;

        if (am.invalidStereocenter(i))
            remove(i);
    }
}

void MoleculeStereocenters::_buildOneFrom3dCoordinates(int idx)
{
    BaseMolecule& bmol = _getMolecule();

    Vec3f& v_pos = bmol.getAtomXyz(idx);

    if (!isPossibleStereocenter(idx))
        return;

    int pyramid[4];

    try
    {
        _restorePyramid(idx, pyramid, false);
    }
    catch (Exception&)
    {
        return;
    }

    Vec3f nei_coords[4];
    int nei_cnt = 0;
    for (int j = 0; j < 4; j++)
    {
        if (pyramid[j] != -1)
            nei_coords[nei_cnt++] = bmol.getAtomXyz(pyramid[j]);
    }

    if (nei_cnt != 4)
    {
        Vec3f v1, v2, v3;
        v1.copy(nei_coords[0]);
        v2.copy(nei_coords[1]);
        v3.copy(nei_coords[2]);

        // Check if substituents with center atom are on the same plane
        int plane_sign_v_pos = _onPlane(v1, v2, v3, v_pos);
        if (plane_sign_v_pos == 0)
            return;

        v1.sub(v_pos);
        v2.sub(v_pos);
        v3.sub(v_pos);
        v1.normalize();
        v2.normalize();
        v3.normalize();
        nei_coords[3] = Vec3f(0, 0, 0);
        nei_coords[3].add(v1);
        nei_coords[3].add(v2);
        nei_coords[3].add(v3);
        nei_coords[3].scale(-1);
        nei_coords[3].normalize();
        nei_coords[3].add(v_pos);
    }

    int plane_sign = _onPlane(nei_coords[0], nei_coords[1], nei_coords[2], nei_coords[3]);

    if (plane_sign == 0)
        return;

    if (plane_sign > 0)
        add(idx, ATOM_ABS, 0, true);
    else
        add(idx, ATOM_ABS, 0, false);
}

bool MoleculeStereocenters::isPossibleStereocenter(int atom_idx, bool* possible_implicit_h, bool* possible_lone_pair)
{
    BaseMolecule& mol = _getMolecule();
    const Vertex& vertex = mol.getVertex(atom_idx);

    int sure_double_bonds = 0;
    int possible_double_bonds = 0;

    int degree = vertex.degree();
    if (degree > 4 || degree <= 2)
        return 0;

    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int e_idx = vertex.neiEdge(i);

        if (mol.getBondOrder(e_idx) == BOND_TRIPLE)
            return false;
        if (mol.getBondOrder(e_idx) == BOND_AROMATIC)
            return false;

        if (mol.getBondOrder(e_idx) == BOND_DOUBLE)
            sure_double_bonds++;
        else if (mol.possibleBondOrder(e_idx, BOND_DOUBLE))
            possible_double_bonds++;
    }

    static const _Configuration allowed_stereocenters[] = {
        // element, charge, degree, double bonds, implicit degree
        {ELEM_C, 0, 3, 0, 4},  {ELEM_C, 0, 4, 0, 4}, {ELEM_Si, 0, 3, 0, 4}, {ELEM_Si, 0, 4, 0, 4}, {ELEM_As, 0, 4, 0, 4}, // see PubChem CID 6338551
        {ELEM_B, -1, 4, 0, 4},                                                                                            // see PubChem CID 6852133
        {ELEM_N, 1, 3, 0, 4},  {ELEM_N, 1, 4, 0, 4}, {ELEM_N, 0, 4, 1, 4},  {ELEM_N, 0, 3, 0, 3},  {ELEM_S, 0, 4, 2, 4},  {ELEM_S, 1, 3, 0, 3},
        {ELEM_S, 1, 4, 1, 4},  {ELEM_S, 0, 3, 1, 3}, {ELEM_P, 0, 3, 0, 3},  {ELEM_P, 1, 4, 0, 4},  {ELEM_P, 0, 4, 1, 4}};

    bool possible = false;
    if (possible_implicit_h != 0)
        *possible_implicit_h = false;
    if (possible_lone_pair != 0)
        *possible_lone_pair = false;
    int i;

    for (i = 0; i < (int)NELEM(allowed_stereocenters); i++)
    {
        const _Configuration& as = allowed_stereocenters[i];

        if (as.degree != vertex.degree())
            continue;

        if (as.n_double_bonds < sure_double_bonds || as.n_double_bonds > sure_double_bonds + possible_double_bonds)
            continue;

        if (!mol.possibleAtomNumberAndCharge(atom_idx, as.elem, as.charge))
            continue;

        possible = true;

        if (possible_implicit_h != 0 && as.implicit_degree == 4 && vertex.degree() == 3)
            *possible_implicit_h = true;

        if (possible_lone_pair != 0 && as.implicit_degree == 3)
            *possible_lone_pair = true;
    }

    return possible;
}

// When bidirectional mode is turned on (like is in ChemDraw) either bonds has
// no directions, and up bond means down for the neighbour (and vice versa).
// But such opposite directions has lower priority and if stereocenter configuration
// can be determined by normal direction then do not check if opposite directions
// contradicts original ones.
bool MoleculeStereocenters::_buildOneCenter(int atom_idx, int* sensible_bonds_out, bool bidirectional_mode, bool bidirectional_either_mode,
                                            const Array<bool>& bond_ignore)
{
    BaseMolecule& mol = _getMolecule();

    const Vertex& vertex = mol.getVertex(atom_idx);

    int degree = vertex.degree();

    _Atom stereocenter;

    stereocenter.group = 1;
    stereocenter.type = ATOM_ABS;

    int* pyramid = stereocenter.pyramid;
    int nei_idx = 0;
    _EdgeIndVec edge_ids[4];

    int last_atom_dir = 0;
    int sure_double_bonds = 0;
    int possible_double_bonds = 0;

    pyramid[0] = -1;
    pyramid[1] = -1;
    pyramid[2] = -1;
    pyramid[3] = -1;

    int n_pure_hydrogens = 0;

    if (degree <= 2 || degree > 4)
        return false;

    bool is_either = false;
    bool zero_bond_length = false;

    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int e_idx = vertex.neiEdge(i);
        int v_idx = vertex.neiVertex(i);

        edge_ids[nei_idx].edge_idx = e_idx;
        edge_ids[nei_idx].nei_idx = v_idx;

        if (mol.possibleAtomNumberAndIsotope(v_idx, ELEM_H, 0))
        {
            if (mol.getAtomNumber(v_idx) == ELEM_H && mol.getAtomIsotope(v_idx) == 0)
                n_pure_hydrogens++;
            edge_ids[nei_idx].rank = 10000;
        }
        else
            edge_ids[nei_idx].rank = v_idx;

        edge_ids[nei_idx].vec.diff(mol.getAtomXyz(v_idx), mol.getAtomXyz(atom_idx));

        if (!edge_ids[nei_idx].vec.normalize())
            zero_bond_length = true;

        if (mol.getBondOrder(e_idx) == BOND_TRIPLE)
            return false;
        if (mol.getBondOrder(e_idx) == BOND_AROMATIC)
            return false;

        if (mol.getBondOrder(e_idx) == BOND_DOUBLE)
            sure_double_bonds++;
        else if (mol.possibleBondOrder(e_idx, BOND_DOUBLE))
            possible_double_bonds++;

        if (_getDirection(mol, atom_idx, v_idx, bidirectional_either_mode) == BOND_EITHER)
            is_either = true;

        nei_idx++;
    }

    _EdgeIndVec tmp;

    bool possible_implicit_h = false;
    bool possible_lone_pair = false;
    int i;

    if (!isPossibleStereocenter(atom_idx, &possible_implicit_h, &possible_lone_pair))
        return false;

    // Local synonym to get bond direction
    auto getDir = [&](int from, int to) {
        int idx = mol.findEdgeIndex(from, to);
        if (bond_ignore[idx])
            return 0;
        return _getDirection(mol, from, to, bidirectional_mode);
    };

    if (is_either)
    {
        stereocenter.type = ATOM_ANY;
        for (i = 0; i < degree; i++)
        {
            stereocenter.pyramid[i] = edge_ids[i].nei_idx;
            if (getDir(atom_idx, edge_ids[i].nei_idx) > 0)
                sensible_bonds_out[edge_ids[i].edge_idx] = 1;
        }
        _stereocenters.insert(atom_idx, stereocenter);
        return true;
    }

    if (degree == 4)
    {
        // sort by neighbor atom index (ascending)
        if (edge_ids[0].rank > edge_ids[1].rank)
            __swap(edge_ids[0], edge_ids[1], tmp);
        if (edge_ids[1].rank > edge_ids[2].rank)
            __swap(edge_ids[1], edge_ids[2], tmp);
        if (edge_ids[2].rank > edge_ids[3].rank)
            __swap(edge_ids[2], edge_ids[3], tmp);
        if (edge_ids[1].rank > edge_ids[2].rank)
            __swap(edge_ids[1], edge_ids[2], tmp);
        if (edge_ids[0].rank > edge_ids[1].rank)
            __swap(edge_ids[0], edge_ids[1], tmp);
        if (edge_ids[1].rank > edge_ids[2].rank)
            __swap(edge_ids[1], edge_ids[2], tmp);

        int main1 = -1, main2 = -1, side1 = -1, side2 = -1;
        int main_dir = 0;

        for (nei_idx = 0; nei_idx < 4; nei_idx++)
        {
            int stereo = getDir(atom_idx, edge_ids[nei_idx].nei_idx);

            if (stereo == BOND_UP || stereo == BOND_DOWN)
            {
                main1 = nei_idx;
                main_dir = stereo;
                break;
            }
        }

        if (main1 == -1)
            return false;

        if (zero_bond_length)
            throw Error("zero bond length near atom %d", atom_idx);

        if (n_pure_hydrogens > 1)
            throw Error("%d hydrogens near stereocenter %d", n_pure_hydrogens, atom_idx);

        int xyz1, xyz2;

        // find main2 as opposite to main1
        if (main2 == -1)
        {
            xyz1 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 1) % 4].vec, edge_ids[(main1 + 2) % 4].vec);
            xyz2 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 1) % 4].vec, edge_ids[(main1 + 3) % 4].vec);

            if (xyz1 + xyz2 == 3 || xyz1 + xyz2 == 12)
            {
                main2 = (main1 + 1) % 4;
                side1 = (main1 + 2) % 4;
                side2 = (main1 + 3) % 4;
            }
        }
        if (main2 == -1)
        {
            xyz1 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 2) % 4].vec, edge_ids[(main1 + 1) % 4].vec);
            xyz2 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 2) % 4].vec, edge_ids[(main1 + 3) % 4].vec);

            if (xyz1 + xyz2 == 3 || xyz1 + xyz2 == 12)
            {
                main2 = (main1 + 2) % 4;
                side1 = (main1 + 1) % 4;
                side2 = (main1 + 3) % 4;
            }
        }
        if (main2 == -1)
        {
            xyz1 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 3) % 4].vec, edge_ids[(main1 + 1) % 4].vec);
            xyz2 = _xyzzy(edge_ids[main1].vec, edge_ids[(main1 + 3) % 4].vec, edge_ids[(main1 + 2) % 4].vec);

            if (xyz1 + xyz2 == 3 || xyz1 + xyz2 == 12)
            {
                main2 = (main1 + 3) % 4;
                side1 = (main1 + 2) % 4;
                side2 = (main1 + 1) % 4;
            }
        }

        if (main2 == -1)
            throw Error("internal error: can not find opposite bond near atom %d", atom_idx);

        if (main_dir == BOND_UP && getDir(atom_idx, edge_ids[main2].nei_idx) == BOND_DOWN)
            throw Error("stereo types of the opposite bonds mismatch near atom %d", atom_idx);
        if (main_dir == BOND_DOWN && getDir(atom_idx, edge_ids[main2].nei_idx) == BOND_UP)
            throw Error("stereo types of the opposite bonds mismatch near atom %d", atom_idx);

        if (main_dir == getDir(atom_idx, edge_ids[side1].nei_idx))
            throw Error("stereo types of non-opposite bonds match near atom %d", atom_idx);
        if (main_dir == getDir(atom_idx, edge_ids[side2].nei_idx))
            throw Error("stereo types of non-opposite bonds match near atom %d", atom_idx);

        if (main1 == 3 || main2 == 3)
            last_atom_dir = main_dir;
        else
            last_atom_dir = (main_dir == BOND_UP ? BOND_DOWN : BOND_UP);

        int sign = _sign(edge_ids[0].vec, edge_ids[1].vec, edge_ids[2].vec);

        if ((last_atom_dir == BOND_UP && sign > 0) || (last_atom_dir == BOND_DOWN && sign < 0))
        {
            pyramid[0] = edge_ids[0].nei_idx;
            pyramid[1] = edge_ids[1].nei_idx;
            pyramid[2] = edge_ids[2].nei_idx;
        }
        else
        {
            pyramid[0] = edge_ids[0].nei_idx;
            pyramid[1] = edge_ids[2].nei_idx;
            pyramid[2] = edge_ids[1].nei_idx;
        }

        pyramid[3] = edge_ids[3].nei_idx;
    }
    else if (degree == 3)
    {
        // sort by neighbor atom index (ascending)
        if (edge_ids[0].rank > edge_ids[1].rank)
            __swap(edge_ids[0], edge_ids[1], tmp);
        if (edge_ids[1].rank > edge_ids[2].rank)
            __swap(edge_ids[1], edge_ids[2], tmp);
        if (edge_ids[0].rank > edge_ids[1].rank)
            __swap(edge_ids[0], edge_ids[1], tmp);

        bool degenerate = true;
        int dirs[3] = {0, 0, 0};
        int main_nei = -1; // will be assigned if all three neighors belong to the same half-plane
        int n_up = 0, n_down = 0;

        for (nei_idx = 0; nei_idx < 3; nei_idx++)
        {
            dirs[nei_idx] = getDir(atom_idx, edge_ids[nei_idx].nei_idx);
            if (dirs[nei_idx] == BOND_UP)
                n_up++;
            else if (dirs[nei_idx] == BOND_DOWN)
                n_down++;
        }

        if (n_down == 0 && n_up == 0)
            return false;

        for (nei_idx = 0; nei_idx < 3; nei_idx++)
        {
            int xyzzy = _xyzzy(edge_ids[(nei_idx + 1) % 3].vec, edge_ids[(nei_idx + 2) % 3].vec, edge_ids[nei_idx].vec);

            if (xyzzy == 1)
                main_nei = nei_idx;
            if (xyzzy == 2)
                degenerate = false;
        }

        int dir = 1;

        if (main_nei != -1)
        {
            if (dirs[main_nei] != 0)
            {
                if (dirs[(main_nei + 1) % 3] == dirs[main_nei] || dirs[(main_nei + 2) % 3] == dirs[main_nei])
                    throw Error("directions of neighbor stereo bonds match near atom %d", atom_idx);
                if (dirs[main_nei] == BOND_UP)
                    dir = -1;
            }
            else
            {
                int d1 = dirs[(main_nei + 1) % 3];
                int d2 = dirs[(main_nei + 2) % 3];

                if (d1 == 0)
                    d1 = d2;
                else if (d2 != 0 && d1 != d2)
                    throw Error("directions of opposite stereo bonds do not match near atom %d", atom_idx);

                if (d1 == 0)
                    return false;

                if (d1 == BOND_DOWN)
                    dir = -1;
            }
        }
        else if (!degenerate)
        {
            if (n_down > 0 && n_up > 0)
                throw Error("one bond up, one bond down -- indefinite case near atom %d", atom_idx);

            if (!possible_lone_pair)
            {
                if (n_up == 3)
                    throw Error("all 3 bonds up near stereoatom %d", atom_idx);
                if (n_down == 3)
                    throw Error("all 3 bonds down near stereoatom %d", atom_idx);
            }
            if (n_down > 0)
                dir = -1;
        }
        else
            throw Error("degenerate case for 3 bonds near stereoatom %d", atom_idx);

        if (zero_bond_length)
            throw Error("zero bond length near atom %d", atom_idx);

        if (n_pure_hydrogens > 0 && !possible_lone_pair)
            throw Error("have hydrogen(s) besides implicit hydrogen near stereocenter %d", atom_idx);

        int sign = _sign(edge_ids[0].vec, edge_ids[1].vec, edge_ids[2].vec);

        if (sign == dir)
        {
            pyramid[0] = edge_ids[0].nei_idx;
            pyramid[1] = edge_ids[2].nei_idx;
            pyramid[2] = edge_ids[1].nei_idx;
        }
        else
        {
            pyramid[0] = edge_ids[0].nei_idx;
            pyramid[1] = edge_ids[1].nei_idx;
            pyramid[2] = edge_ids[2].nei_idx;
        }
        pyramid[3] = -1;
    }

    for (i = 0; i < degree; i++)
        if (getDir(atom_idx, edge_ids[i].nei_idx) > 0)
            sensible_bonds_out[edge_ids[i].edge_idx] = 1;

    _stereocenters.insert(atom_idx, stereocenter);
    return true;
}

// 1 -- in the smaller angle, 2 -- in the bigger angle,
// 4 -- in the 'positive' straight angle, 8 -- in the 'negative' straight angle
int MoleculeStereocenters::_xyzzy(const Vec3f& v1, const Vec3f& v2, const Vec3f& u)
{
    float eps = 1e-3f;

    Vec3f cross;

    cross.cross(v1, v2);

    float sine1 = cross.z;
    float cosine1 = Vec3f::dot(v1, v2);

    cross.cross(v1, u);

    float sine2 = cross.z;
    float cosine2 = Vec3f::dot(v1, u);

    if ((float)fabs(sine1) < eps)
    {
        if ((float)fabs(sine2) < eps)
            throw Error("degenerate case -- bonds overlap");

        return (sine2 > 0) ? 4 : 8;
    }

    if (sine1 * sine2 < -eps * eps)
        return 2;

    if (cosine2 < cosine1)
        return 2;

    return 1;
}

int MoleculeStereocenters::_sign(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3)
{
    // Check the angle between bonds
    float dot_eps = 0.997f; // Corresponds to 4.5 degrees
                            //   float dot_eps = 0.99999f; // Corresponds to 0 degrees
    if (Vec3f::dot(v1, v2) > dot_eps * v1.length() * v2.length() || Vec3f::dot(v1, v3) > dot_eps * v1.length() * v3.length() ||
        Vec3f::dot(v2, v3) > dot_eps * v2.length() * v3.length())
        throw Error("angle between bonds is too small");

    float res = (v1.x - v3.x) * (v2.y - v3.y) - (v1.y - v3.y) * (v2.x - v3.x);
    float eps = 1e-3f;

    if (res > eps)
        return 1;
    if (res < -eps)
        return -1;

    throw Error("degenerate triangle");
}

int MoleculeStereocenters::_onPlane(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const Vec3f& u)
{
    Vec3f v1u, v2u, v3u, p;
    float eps = 0.1f;

    v1u.diff(v1, u);
    v1u.normalize();
    v2u.diff(v2, u);
    v2u.normalize();
    v3u.diff(v3, u);
    v3u.normalize();

    float a12, a23, a13;
    Vec3f::angle(v1u, v2u, a12);
    Vec3f::angle(v2u, v3u, a23);
    Vec3f::angle(v1u, v3u, a13);

    float angle_sum = a12 + a23 + a13;

    if (fabs(angle_sum - 2 * M_PI) < eps)
        return 0;

    p.cross(v2u, v3u);
    float det = Vec3f::dot(v1u, p);

    if (det > 0)
        return 1;
    else
        return -1;
}

int MoleculeStereocenters::getType(int idx) const
{
    _Atom* atom = _stereocenters.at2(idx);

    if (atom == 0)
        return 0;

    return atom->type;
}

int MoleculeStereocenters::getGroup(int idx) const
{
    return _stereocenters.at(idx).group;
}

void MoleculeStereocenters::setGroup(int idx, int group)
{
    _stereocenters.at(idx).group = group;
}

void MoleculeStereocenters::setType(int idx, int type, int group)
{
    _stereocenters.at(idx).type = type;
    _stereocenters.at(idx).group = group;
}

void MoleculeStereocenters::setType(int idx, int type)
{
    _stereocenters.at(idx).type = type;
}

const int* MoleculeStereocenters::getPyramid(int idx) const
{
    return _stereocenters.at(idx).pyramid;
}

int* MoleculeStereocenters::getPyramid(int idx)
{
    _Atom* stereo = _stereocenters.at2(idx);

    if (stereo != 0)
        return stereo->pyramid;
    else
        return 0;
}

void MoleculeStereocenters::invertPyramid(int idx)
{
    int tmp;
    int* pyramid = getPyramid(idx);
    __swap(pyramid[0], pyramid[1], tmp);
}

void MoleculeStereocenters::getAbsAtoms(Array<int>& indices)
{
    indices.clear();

    for (int i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
    {
        if (_stereocenters.value(i).type == ATOM_ABS)
            indices.push(_stereocenters.key(i));
    }
}

void MoleculeStereocenters::_getGroups(int type, Array<int>& numbers)
{
    numbers.clear();

    for (int i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
    {
        if (_stereocenters.value(i).type == type)
        {
            int group = _stereocenters.value(i).group;

            if (numbers.find(group) == -1)
                numbers.push(group);
        }
    }
}

void MoleculeStereocenters::_getGroup(int type, int number, Array<int>& indices)
{
    indices.clear();

    for (int i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
    {
        const _Atom& atom = _stereocenters.value(i);

        if (atom.type == type && atom.group == number)
            indices.push(_stereocenters.key(i));
    }
}

void MoleculeStereocenters::getOrGroups(Array<int>& numbers)
{
    _getGroups(ATOM_OR, numbers);
}

void MoleculeStereocenters::getAndGroups(Array<int>& numbers)
{
    _getGroups(ATOM_AND, numbers);
}

void MoleculeStereocenters::getOrGroup(int number, Array<int>& indices)
{
    _getGroup(ATOM_OR, number, indices);
}

void MoleculeStereocenters::getAndGroup(int number, Array<int>& indices)
{
    _getGroup(ATOM_AND, number, indices);
}

bool MoleculeStereocenters::sameGroup(int idx1, int idx2)
{
    _Atom* center1 = _stereocenters.at2(idx1);
    _Atom* center2 = _stereocenters.at2(idx2);

    if (center1 == 0 && center2 == 0)
        return true;

    if (center1 == 0 || center2 == 0)
        return false;

    if (center1->type == ATOM_ABS)
        return center2->type == ATOM_ABS;

    if (center1->type == ATOM_OR)
        return center2->type == ATOM_OR && center1->group == center2->group;

    if (center1->type == ATOM_AND)
        return center2->type == ATOM_AND && center1->group == center2->group;

    return false;
}

bool MoleculeStereocenters::haveAllAbs()
{
    int i;

    for (i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
        if (_stereocenters.value(i).type != ATOM_ABS)
            return false;

    return true;
}

bool MoleculeStereocenters::haveAbs()
{
    int i;

    for (i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
        if (_stereocenters.value(i).type == ATOM_ABS)
            return true;

    return false;
}

bool MoleculeStereocenters::haveAllAbsAny()
{
    int i;

    for (i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
        if (_stereocenters.value(i).type != ATOM_ABS && _stereocenters.value(i).type != ATOM_ANY)
            return false;

    return true;
}

bool MoleculeStereocenters::haveAllAndAny()
{
    int i;
    int groupno = -1;

    for (i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
    {
        if (_stereocenters.value(i).type == ATOM_ANY)
            continue;
        if (_stereocenters.value(i).type != ATOM_AND)
            return false;
        if (groupno == -1)
            groupno = _stereocenters.value(i).group;
        else if (groupno != _stereocenters.value(i).group)
            return false;
    }

    return true;
}

bool MoleculeStereocenters::checkSub(const MoleculeStereocenters& query, const MoleculeStereocenters& target, const int* mapping, bool reset_h_isotopes,
                                     Filter* stereocenters_vertex_filter)
{
    QS_DEF(Array<int>, flags);

    flags.clear_resize(query._stereocenters.end());
    flags.zerofill();

    int i, j;

    for (i = query._stereocenters.begin(); i != query._stereocenters.end(); i = query._stereocenters.next(i))
    {
        if (flags[i])
            continue;

        flags[i] = 1;

        const _Atom& cq = query._stereocenters.value(i);
        int iq = query._stereocenters.key(i);

        if (mapping[iq] < 0)
            continue; // happens only on Exact match (when some query fragments are disabled)

        if (stereocenters_vertex_filter != 0 && !stereocenters_vertex_filter->valid(iq))
            continue;

        if (cq.type < ATOM_AND)
            continue;

        int stereo_group_and = -1;
        int stereo_group_or = -1;
        int revert = -1; // 0 -- not revert, 1 -- revert
        bool have_abs = false;

        int pyramid_mapping[4];
        int type = cq.type;

        if (type == ATOM_ABS)
        {
            try
            {
                getPyramidMapping(query, target, iq, mapping, pyramid_mapping, reset_h_isotopes);
            }
            catch (Exception e)
            {
                return false;
            }

            if (!isPyramidMappingRigid(pyramid_mapping))
                return false;
        }
        else if (type == ATOM_OR || type == ATOM_AND)
        {
            for (j = i; j != query._stereocenters.end(); j = query._stereocenters.next(j))
            {
                int iq2 = query._stereocenters.key(j);
                const _Atom& cq2 = query._stereocenters.value(j);

                if (cq2.type != type)
                    continue;

                if (cq2.group != cq.group)
                    continue;

                const _Atom* ct2 = target._stereocenters.at2(mapping[iq2]);

                if (ct2 == 0)
                    return false;

                if (ct2->type < type)
                    return false;

                flags[j] = 1;

                if (ct2->type == ATOM_AND)
                {
                    if (stereo_group_or != -1)
                        return false;
                    if (have_abs)
                        return false;

                    if (stereo_group_and == -1)
                        stereo_group_and = ct2->group;
                    else if (stereo_group_and != ct2->group)
                        return false;
                }
                else if (ct2->type == ATOM_OR)
                {
                    if (stereo_group_and != -1)
                        return false;
                    if (have_abs)
                        return false;

                    if (stereo_group_or == -1)
                        stereo_group_or = ct2->group;
                    else if (stereo_group_or != ct2->group)
                        return false;
                }
                else if (ct2->type == ATOM_ABS)
                {
                    if (stereo_group_and != -1)
                        return false;
                    if (stereo_group_or != -1)
                        return false;

                    have_abs = true;
                }

                getPyramidMapping(query, target, iq2, mapping, pyramid_mapping, reset_h_isotopes);
                int not_equal = isPyramidMappingRigid(pyramid_mapping) ? 0 : 1;

                if (revert == -1)
                    revert = not_equal;
                else if (revert != not_equal)
                    return false;
            }
        }
    }

    return true;
}

bool MoleculeStereocenters::isPyramidMappingRigid(const int mapping[4])
{
    int arr[4], tmp;
    bool rigid = true;

    memcpy(arr, mapping, 4 * sizeof(int));

    if (arr[0] > arr[1])
        __swap(arr[0], arr[1], tmp), rigid = !rigid;
    if (arr[1] > arr[2])
        __swap(arr[1], arr[2], tmp), rigid = !rigid;
    if (arr[2] > arr[3])
        __swap(arr[2], arr[3], tmp), rigid = !rigid;
    if (arr[1] > arr[2])
        __swap(arr[1], arr[2], tmp), rigid = !rigid;
    if (arr[0] > arr[1])
        __swap(arr[0], arr[1], tmp), rigid = !rigid;
    if (arr[1] > arr[2])
        __swap(arr[1], arr[2], tmp), rigid = !rigid;

    return rigid;
}

bool MoleculeStereocenters::isPyramidMappingRigid_Sort(int* pyramid, const int* mapping)
{
    bool rigid = true;
    int i, tmp;

    for (i = 0; i < 4; i++)
        if (pyramid[i] != -1 && mapping[pyramid[i]] < 0)
            pyramid[i] = -1;

    if (pyramid[0] == -1 || (pyramid[1] >= 0 && mapping[pyramid[0]] > mapping[pyramid[1]]))
        __swap(pyramid[0], pyramid[1], tmp), rigid = !rigid;
    if (pyramid[1] == -1 || (pyramid[2] >= 0 && mapping[pyramid[1]] > mapping[pyramid[2]]))
        __swap(pyramid[1], pyramid[2], tmp), rigid = !rigid;
    if (pyramid[2] == -1 || (pyramid[3] >= 0 && mapping[pyramid[2]] > mapping[pyramid[3]]))
        __swap(pyramid[2], pyramid[3], tmp), rigid = !rigid;
    if (pyramid[1] == -1 || (pyramid[2] >= 0 && mapping[pyramid[1]] > mapping[pyramid[2]]))
        __swap(pyramid[1], pyramid[2], tmp), rigid = !rigid;
    if (pyramid[0] == -1 || (pyramid[1] >= 0 && mapping[pyramid[0]] > mapping[pyramid[1]]))
        __swap(pyramid[0], pyramid[1], tmp), rigid = !rigid;
    if (pyramid[1] == -1 || (pyramid[2] >= 0 && mapping[pyramid[1]] > mapping[pyramid[2]]))
        __swap(pyramid[1], pyramid[2], tmp), rigid = !rigid;

    return rigid;
}

bool MoleculeStereocenters::isPyramidMappingRigid(const int* pyramid, int size, const int* mapping)
{
    if (size == 3)
    {
        int order[3] = {mapping[pyramid[0]], mapping[pyramid[1]], mapping[pyramid[2]]};
        int min = __min3(order[0], order[1], order[2]);

        while (order[0] != min)
        {
            int t = order[2];
            order[2] = order[1];
            order[1] = order[0];
            order[0] = t;
        }

        return order[1] < order[2];
    }

    if (size == 4)
    {
        int arr[4];

        arr[0] = mapping[pyramid[0]];
        arr[1] = mapping[pyramid[1]];
        arr[2] = mapping[pyramid[2]];
        arr[3] = mapping[pyramid[3]];

        return isPyramidMappingRigid(arr);
    }

    throw Error("IsPyramidMappingRigid: size = %d", size);
}

void MoleculeStereocenters::getPyramidMapping(const MoleculeStereocenters& query, const MoleculeStereocenters& target, int query_atom, const int* mapping,
                                              int* mapping_out, bool reset_h_isotopes)
{
    int i, j;

    BaseMolecule& tmol = target._getMolecule();
    BaseMolecule& qmol = query._getMolecule();

    const int* seq1 = query.getPyramid(query_atom);
    const int* seq2 = target.getPyramid(mapping[query_atom]);

    int seq2_matched[] = {0, 0, 0, 0};

    for (i = 0; i < 4; i++)
        mapping_out[i] = -1;

    for (i = 0; i < 4; i++)
    {
        // skip implicit hydrogen for the first pass
        if (seq1[i] == -1)
            continue;

        // unmapped atom?
        if (mapping[seq1[i]] < 0)
        {
            // only hydrogens are allowed to be unmapped
            if (qmol.getAtomNumber(seq1[i]) != ELEM_H)
                throw Error("unmapped non-hydrogen atom (atom number %d)", qmol.getAtomNumber(seq1[i]));
            continue;
        }

        for (j = 0; j < 4; j++)
            if (seq2[j] == mapping[seq1[i]])
                break;

        if (j == 4)
            throw Error("cannot map pyramid");

        mapping_out[i] = j;
        seq2_matched[j] = 1;
    }

    // take implicit hydrogen to the second pass
    for (i = 0; i < 4; i++)
    {
        if (mapping_out[i] != -1)
            continue;

        for (j = 0; j < 4; j++)
        {
            if (seq2[j] == -1)
                break; // match to implicit hydrogen

            if (!seq2_matched[j])
            {
                if (tmol.getAtomNumber(seq2[j]) == ELEM_H)
                    break; // match to explicit hydrogen

                // rare cases like '[S@](F)(Cl)=O' on '[S@](F)(Cl)(=O)=N'
                if (seq1[i] == -1 && tmol.getAtomNumber(mapping[query_atom]) == ELEM_S)
                    break; // match free electron pair to an atom

                // Match N[C@H](O)S on C[C@@](N)(O)S
                if (seq1[i] == -1)
                    break; // match vacant place in query to this atom
            }
        }

        if (j == 4)
            throw Error("cannot map pyramid");

        mapping_out[i] = j;
        seq2_matched[j] = 1;
    }
}

void MoleculeStereocenters::remove(int idx)
{
    _stereocenters.remove(idx);
}

void MoleculeStereocenters::removeAtoms(const Array<int>& indices)
{
    const BaseMolecule& mol = _getMolecule();

    for (int i = 0; i < indices.size(); i++)
    {
        int idx = indices[i];
        if (_stereocenters.find(idx))
            _stereocenters.remove(idx);
        else
        {
            const Vertex& vertex = mol.getVertex(idx);

            for (int k = vertex.neiBegin(); k != vertex.neiEnd(); k = vertex.neiNext(k))
            {
                int nei_vertex = vertex.neiVertex(k);
                _removeBondDir(idx, nei_vertex);
            }
        }
    }
}

void MoleculeStereocenters::removeBonds(const Array<int>& indices)
{
    const BaseMolecule& mol = _getMolecule();

    for (int i = 0; i < indices.size(); i++)
    {
        const Edge& edge = mol.getEdge(indices[i]);

        _removeBondDir(edge.beg, edge.end);
        _removeBondDir(edge.end, edge.beg);
    }
}

void MoleculeStereocenters::_removeBondDir(int atom_from, int atom_to)
{
    _Atom* stereo_atom = _stereocenters.at2(atom_to);
    if (stereo_atom != 0)
    {
        if (stereo_atom->pyramid[3] == -1)
            _stereocenters.remove(atom_to);
        else
        {
            BaseMolecule& mol = _getMolecule();

            if (!mol.isQueryMolecule() || mol.possibleAtomNumber(atom_from, ELEM_H) || mol.isRSite(atom_from))
                _convertAtomToImplicitHydrogen(stereo_atom->pyramid, atom_from);
        }
    }
}

void MoleculeStereocenters::buildOnSubmolecule(const MoleculeStereocenters& super, int* mapping)
{
    int i, j;
    BaseMolecule& mol = _getMolecule();

    for (i = super._stereocenters.begin(); i != super._stereocenters.end(); i = super._stereocenters.next(i))
    {
        int super_idx = super._stereocenters.key(i);
        const _Atom& super_stereocenter = super._stereocenters.value(i);
        int sub_idx = mapping[super_idx];

        if (sub_idx < 0)
            continue;

        _Atom new_stereocenter;

        new_stereocenter.group = super_stereocenter.group;
        new_stereocenter.type = super_stereocenter.type;

        for (j = 0; j < 4; j++)
        {
            int idx = super_stereocenter.pyramid[j];

            if (idx == -1)
                new_stereocenter.pyramid[j] = -1;
            else
            {
                int val = mapping[idx];
                if (val != -1 && mol.findEdgeIndex(sub_idx, val) == -1)
                    val = -1;
                new_stereocenter.pyramid[j] = val;
            }
        }

        moveMinimalToEnd(new_stereocenter.pyramid);
        if (new_stereocenter.pyramid[0] == -1 || new_stereocenter.pyramid[1] == -1 || new_stereocenter.pyramid[2] == -1)
            // pyramid is not mapped completely
            continue;

        _stereocenters.insert(sub_idx, new_stereocenter);

        const Vertex& super_vertex = super._getMolecule().getVertex(super_idx);

        for (j = super_vertex.neiBegin(); j != super_vertex.neiEnd(); j = super_vertex.neiNext(j))
        {
            int super_edge = super_vertex.neiEdge(j);
            if (mapping[super_vertex.neiVertex(j)] == -1)
                continue;

            int dir = super._getMolecule().getBondDirection(super_edge);
            if (dir != 0)
                mol.setBondDirection(mol.findEdgeIndex(sub_idx, mapping[super_vertex.neiVertex(j)]), dir);
        }
    }
}

int MoleculeStereocenters::size() const
{
    return _stereocenters.size();
}

void MoleculeStereocenters::add(int atom_idx, int type, int group, bool inverse_pyramid)
{
    int pyramid[4];
    _restorePyramid(atom_idx, pyramid, inverse_pyramid);
    add(atom_idx, type, group, pyramid);
}

void MoleculeStereocenters::add(int atom_idx, int type, int group, const int pyramid[4])
{
    if (atom_idx < 0)
        throw Error("stereocenter index is invalid");
    if (pyramid[0] == -1 || pyramid[1] == -1 || pyramid[2] == -1)
        throw Error("stereocenter (%d) pyramid must have at least 3 atoms", atom_idx);

    _Atom center;

    center.type = type;
    center.group = group;
    memcpy(center.pyramid, pyramid, 4 * sizeof(int));

    _stereocenters.insert(atom_idx, center);
}

int MoleculeStereocenters::begin() const
{
    return _stereocenters.begin();
}

int MoleculeStereocenters::end() const
{
    return _stereocenters.end();
}

int MoleculeStereocenters::next(int i) const
{
    return _stereocenters.next(i);
}

bool MoleculeStereocenters::exists(int atom_idx) const
{
    return _stereocenters.at2(atom_idx) != 0;
}

void MoleculeStereocenters::get(int i, int& atom_idx, int& type, int& group, int* pyramid) const
{
    const _Atom& center = _stereocenters.value(i);

    atom_idx = _stereocenters.key(i);
    type = center.type;
    group = center.group;
    if (pyramid != 0)
        memcpy(pyramid, center.pyramid, 4 * sizeof(int));
}

int MoleculeStereocenters::getAtomIndex(int i) const
{
    return _stereocenters.key(i);
}

void MoleculeStereocenters::get(int atom_idx, int& type, int& group, int* pyramid) const
{
    const _Atom& center = _stereocenters.at(atom_idx);

    type = center.type;
    group = center.group;
    if (pyramid != 0)
        memcpy(pyramid, center.pyramid, 4 * sizeof(int));
}

void MoleculeStereocenters::registerUnfoldedHydrogen(int atom_idx, int added_hydrogen)
{
    _Atom* center = _stereocenters.at2(atom_idx);
    if (center == 0)
        return;

    if (center->pyramid[3] != -1)
        throw Error("cannot unfold hydrogens for stereocenter without implicit hydrogens");
    center->pyramid[3] = added_hydrogen;
}

void MoleculeStereocenters::flipBond(int atom_parent, int atom_from, int atom_to)
{
    if (exists(atom_from))
    {
        _Atom* from_center = _stereocenters.at2(atom_from);

        if (from_center->pyramid[3] == -1)
            remove(atom_from);
        else
        {
            for (int i = 0; i < 4; i++)
                if (from_center->pyramid[i] == atom_parent)
                    from_center->pyramid[i] = -1;
            moveMinimalToEnd(from_center->pyramid);
        }
    }

    if (exists(atom_to))
    {
        _Atom* to_center = _stereocenters.at2(atom_to);

        if (to_center->pyramid[3] != -1)
            throw Error("Bad bond flipping. Stereocenter pyramid is already full");

        to_center->pyramid[3] = atom_parent;
    }

    if (!exists(atom_parent))
        return;

    _Atom* center = _stereocenters.at2(atom_parent);
    for (int i = 0; i < 4; i++)
        if (center->pyramid[i] == atom_from)
        {
            center->pyramid[i] = atom_to;
            break;
        }
}

void MoleculeStereocenters::_restorePyramid(int idx, int pyramid[4], int invert_pyramid)
{
    BaseMolecule& mol = _getMolecule();
    const Vertex& vertex = mol.getVertex(idx);
    int j, count = 0;

    pyramid[0] = -1;
    pyramid[1] = -1;
    pyramid[2] = -1;
    pyramid[3] = -1;

    for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
    {
        int nei = vertex.neiVertex(j);

        if (vertex.degree() == 3 || mol.getAtomNumber(nei) != ELEM_H || mol.getAtomIsotope(nei) != 0)
        {
            if (count == 4)
                throw Error("restorePyramid(): stereocenter has more than 4 neighbors");

            pyramid[count++] = nei;
        }
        else if (pyramid[3] == -1)
            pyramid[3] = nei;
        else
            throw Error("restorePyramid(): extra hydrogen");
    }

    int tmp;

    // sort pyramid indices
    if (pyramid[3] == -1)
    {
        if (pyramid[0] > pyramid[1])
            __swap(pyramid[0], pyramid[1], tmp);
        if (pyramid[1] > pyramid[2])
            __swap(pyramid[1], pyramid[2], tmp);
        if (pyramid[0] > pyramid[1])
            __swap(pyramid[0], pyramid[1], tmp);
    }
    else
    {
        if (pyramid[0] > pyramid[1])
            __swap(pyramid[0], pyramid[1], tmp);
        if (pyramid[1] > pyramid[2])
            __swap(pyramid[1], pyramid[2], tmp);
        if (pyramid[2] > pyramid[3])
            __swap(pyramid[2], pyramid[3], tmp);
        if (pyramid[1] > pyramid[2])
            __swap(pyramid[1], pyramid[2], tmp);
        if (pyramid[0] > pyramid[1])
            __swap(pyramid[0], pyramid[1], tmp);
        if (pyramid[1] > pyramid[2])
            __swap(pyramid[1], pyramid[2], tmp);
    }
    if (invert_pyramid)
        __swap(pyramid[1], pyramid[2], j);
}

void MoleculeStereocenters::rotatePyramid(int* pyramid)
{
    int tmp;

    tmp = pyramid[0];
    pyramid[0] = pyramid[1];
    pyramid[1] = pyramid[2];

    if (pyramid[3] == -1)
    {
        pyramid[2] = tmp;
    }
    else
    {
        pyramid[2] = pyramid[3];
        pyramid[3] = tmp;
    }
}

void MoleculeStereocenters::moveImplicitHydrogenToEnd(int pyramid[4])
{
    moveMinimalToEnd(pyramid);
    if (pyramid[3] != -1)
        throw Error("moveImplicitHydrogenToEnd(): no implicit hydrogen");
}

void MoleculeStereocenters::moveElementToEnd(int pyramid[4], int element)
{
    int cnt = 0;

    while (pyramid[3] != element)
    {
        if (cnt == 4)
            throw Error("moveElementToEnd(): internal error");

        rotatePyramid(pyramid);
        cnt++;
    }

    if (cnt & 1)
        __swap(pyramid[0], pyramid[1], cnt);
}

void MoleculeStereocenters::moveMinimalToEnd(int pyramid[4])
{
    int min_element = __min(__min(pyramid[0], pyramid[1]), __min(pyramid[2], pyramid[3]));
    moveElementToEnd(pyramid, min_element);
}

void MoleculeStereocenters::_convertAtomToImplicitHydrogen(int pyramid[4], int atom_to_remove)
{
    if (pyramid[3] == -1)
        throw Error("Cannot remove atoms form sterecenter with implicit hydrogen. "
                    "Stereocenter should be removed");

    bool removed = false;

    for (int i = 0; i < 4; i++)
        if (pyramid[i] == atom_to_remove)
        {
            pyramid[i] = -1;
            removed = true;
            break;
        }

    if (!removed)
        throw Error("Specified atom %d wasn't found in the stereopyramid", atom_to_remove);

    moveImplicitHydrogenToEnd(pyramid);
}

void MoleculeStereocenters::markBond(int atom_idx)
{
    const _Atom* atom_ptr = _stereocenters.at2(atom_idx);
    if (atom_ptr == NULL)
        return;

    BaseMolecule& mol = _getMolecule();
    const _Atom& atom = *atom_ptr;
    int pyramid[4];
    int mult = 1;
    int size = 0;
    int j;

    memcpy(pyramid, atom.pyramid, 4 * sizeof(int));

    const Vertex& vertex = mol.getVertex(atom_idx);
    if (atom.type <= ATOM_ANY)
    {
        // fill the pyramid
        for (j = vertex.neiBegin(); j != vertex.neiEnd() && size < 4; j = vertex.neiNext(j))
            pyramid[size++] = vertex.neiVertex(j);
    }
    else
        size = (pyramid[3] == -1 ? 3 : 4);

    // clear bond directions that goes to this atom, and not from this atom because they can
    // be marked by other sterecenter
    for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
        if (mol.getBondDirection2(atom_idx, vertex.neiVertex(j)) != 0)
            mol.setBondDirection(vertex.neiEdge(j), 0);

    int edge_idx = -1;

    for (j = 0; j < size; j++)
    {
        edge_idx = mol.findEdgeIndex(atom_idx, pyramid[size - 1]);
        if (mol.getBondDirection(edge_idx) == 0 && mol.getVertex(pyramid[size - 1]).degree() == 1)
            break;
        rotatePyramid(pyramid);
        if (size == 4)
            mult = -mult;
    }

    if (j == size)
    {
        for (j = 0; j < size; j++)
        {
            edge_idx = mol.findEdgeIndex(atom_idx, pyramid[size - 1]);
            if (mol.getBondDirection(edge_idx) == 0 && mol.getBondTopology(edge_idx) == TOPOLOGY_CHAIN && getType(pyramid[size - 1]) == 0)
                break;
            rotatePyramid(pyramid);
            if (size == 4)
                mult = -mult;
        }
    }

    if (j == size)
    {
        for (j = 0; j < size; j++)
        {
            edge_idx = mol.findEdgeIndex(atom_idx, pyramid[size - 1]);
            if (mol.getBondDirection(edge_idx) == 0 && getType(pyramid[size - 1]) == 0)
                break;
            rotatePyramid(pyramid);
            if (size == 4)
                mult = -mult;
        }
    }

    if (j == size)
    {
        for (j = 0; j < size; j++)
        {
            edge_idx = mol.findEdgeIndex(atom_idx, pyramid[size - 1]);
            if (mol.getBondDirection(edge_idx) == 0 && mol.getBondTopology(edge_idx) == TOPOLOGY_CHAIN)
                break;
            rotatePyramid(pyramid);
            if (size == 4)
                mult = -mult;
        }
    }

    if (j == size)
    {
        for (j = 0; j < size; j++)
        {
            edge_idx = mol.findEdgeIndex(atom_idx, pyramid[size - 1]);
            if (mol.getBondDirection(edge_idx) == 0)
                break;
            rotatePyramid(pyramid);
            if (size == 4)
                mult = -mult;
        }
    }

    if (j == size)
        throw Error("no bond can be marked");

    if (mol.getEdge(edge_idx).beg != atom_idx)
        mol.swapEdgeEnds(edge_idx);

    if (atom.type > ATOM_ANY)
    {
        Vec3f dirs[4];

        for (j = 0; j < size; j++)
        {
            dirs[j] = mol.getAtomXyz(pyramid[j]);
            dirs[j].sub(mol.getAtomXyz(atom_idx));
            if (!dirs[j].normalize())
                throw Error("zero bond length");
        }

        int sign = _sign(dirs[0], dirs[1], dirs[2]);

        if (size == 3)
        {
            // Check if all the three bonds belong to the same half-plane.
            // This is equal to that one of the bonds lies in the smaller
            // angle formed by the other two.
            if (_xyzzy(dirs[1], dirs[0], dirs[2]) == 1 || _xyzzy(dirs[2], dirs[1], dirs[0]) == 1 || _xyzzy(dirs[0], dirs[2], dirs[1]) == 1)
            {
                if (_xyzzy(dirs[1], dirs[0], dirs[2]) == 1)
                    mult = -1;
                mol.setBondDirection(edge_idx, (sign * mult == 1) ? BOND_DOWN : BOND_UP);
            }
            else
                mol.setBondDirection(edge_idx, (sign == 1) ? BOND_DOWN : BOND_UP);
        }
        else
            mol.setBondDirection(edge_idx, (sign * mult == 1) ? BOND_UP : BOND_DOWN);
    }
    else
        mol.setBondDirection(edge_idx, BOND_EITHER);
}

void MoleculeStereocenters::markBonds()
{
    int i;

    for (i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
        markBond(_stereocenters.key(i));
}

bool MoleculeStereocenters::isAutomorphism(BaseMolecule& mol, const Array<int>& mapping, const Filter* filter)
{
    MoleculeStereocenters& stereocenters = mol.stereocenters;

    for (int i = stereocenters.begin(); i != stereocenters.end(); i = stereocenters.next(i))
    {
        if (filter && !filter->valid(i))
            continue;

        int idx, type, group;
        int pyramid[4];
        int j;

        stereocenters.get(i, idx, type, group, pyramid);

        if (mapping[idx] == -1)
            continue;

        int size = 0;

        for (j = 0; j < 4; j++)
            if (pyramid[j] >= 0)
            {
                if (mapping[pyramid[j]] >= 0)
                    size++;
                else
                    pyramid[j] = -1;
            }

        if (size < 3)
            continue;

        if (type < MoleculeStereocenters::ATOM_AND)
            continue;

        if (stereocenters.getType(mapping[idx]) != type)
            return false;

        int pyra_map[4];

        MoleculeStereocenters::getPyramidMapping(stereocenters, stereocenters, idx, mapping.ptr(), pyra_map, false);

        if (!MoleculeStereocenters::isPyramidMappingRigid(pyra_map))
            return false;
    }
    return true;
}

int MoleculeStereocenters::_getDirection(BaseMolecule& mol, int atom_from, int atom_to, bool bidirectional_mode)
{
    int dir = mol.getBondDirection2(atom_from, atom_to);
    if (bidirectional_mode && dir == 0)
    {
        // Try to get direction in opposite direction
        dir = mol.getBondDirection2(atom_to, atom_from);
        if (dir == BOND_UP)
            dir = BOND_DOWN;
        else if (dir == BOND_DOWN)
            dir = BOND_UP;
    }
    return dir;
}
