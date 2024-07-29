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
#include <algorithm>
#include <array>

using namespace indigo;

IMPL_ERROR(MoleculeStereocenters, "stereocenters");

MoleculeStereocenters::MoleculeStereocenters()
{
}

void MoleculeStereocenters::clear()
{
    _stereocenters.clear();
}

void MoleculeStereocenters::buildFromBonds(BaseMolecule& baseMolecule, const StereocentersOptions& options, int* sensible_bonds_out)
{
    HaworthProjectionFinder haworth_finder(baseMolecule);
    if (options.detect_haworth_projection)
        haworth_finder.findAndAddStereocenters();

    const Array<bool>& bonds_ignore = haworth_finder.getBondsMask();
    const Array<bool>& atoms_ignore = haworth_finder.getAtomsMask();
    bool check_atropisomery = false;
    for (int i = baseMolecule.edgeBegin(); i != baseMolecule.edgeEnd(); i = baseMolecule.edgeNext(i))
    {
        auto bdir = baseMolecule.getBondDirection(i);
        if (bonds_ignore[i] && bdir)
            sensible_bonds_out[i] = 1;
        if (!check_atropisomery && bdir && baseMolecule.getBondTopology(i) == TOPOLOGY_RING)
            check_atropisomery = true;
    }

    for (int i = baseMolecule.vertexBegin(); i != baseMolecule.vertexEnd(); i = baseMolecule.vertexNext(i))
    {
        if (atoms_ignore[i])
            continue;

        // Try to build sterecenters with bidirectional_mode only for either bonds
        bool found = false;
        try
        {
            found = _buildOneCenter(baseMolecule, i, sensible_bonds_out, false, options.bidirectional_mode, bonds_ignore, check_atropisomery);
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
                _buildOneCenter(baseMolecule, i, sensible_bonds_out, true, options.bidirectional_mode, bonds_ignore);
            }
            catch (Error&)
            {
            }
        }
    }

    for (int i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
    {
        _Atom& atom = _stereocenters.value(i);
        if (atom.is_atropisomeric)
        {
            int atom_idx = _stereocenters.key(i);
            _AtropoCenter& ac = _atropocenters.at(atom_idx);
            std::unordered_set<int> visited_bonds;
            if (findAtropoStereobonds(baseMolecule, ac.bond_directions, atom_idx, visited_bonds, false, sensible_bonds_out))
            {
                auto bdir = baseMolecule.getBondDirection(ac.atropo_bond);
                // include possible atropobond itself if its direction is not sensible, but direction is set
                if (bdir && !sensible_bonds_out[ac.atropo_bond])
                {
                    ac.bond_directions.insert(ac.atropo_bond, bdir);
                    sensible_bonds_out[ac.atropo_bond] = 1;
                }
            }
            else
            {
                atom.is_atropisomeric = false;
                _atropocenters.remove(atom_idx);
            }
        }
    }
}

void MoleculeStereocenters::buildFrom3dCoordinates(BaseMolecule& baseMolecule, const StereocentersOptions& options)
{
    if (baseMolecule.isQueryMolecule())
        return;

    Molecule& mol = baseMolecule.asMolecule();

    if (!BaseMolecule::hasZCoord(mol))
        return;

    _stereocenters.clear();

    int i;

    for (i = baseMolecule.vertexBegin(); i != baseMolecule.vertexEnd(); i = baseMolecule.vertexNext(i))
    {
        try
        {
            _buildOneFrom3dCoordinates(baseMolecule, i);
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

    for (i = baseMolecule.vertexBegin(); i != baseMolecule.vertexEnd(); i = baseMolecule.vertexNext(i))
    {
        if (!baseMolecule.stereocenters.exists(i))
            continue;

        if (am.invalidStereocenter(i))
            remove(i);
    }
}

void MoleculeStereocenters::_buildOneFrom3dCoordinates(BaseMolecule& baseMolecule, int idx)
{
    Vec3f& v_pos = baseMolecule.getAtomXyz(idx);

    if (!isPossibleStereocenter(baseMolecule, idx))
        return;

    int pyramid[4];

    try
    {
        _restorePyramid(baseMolecule, idx, pyramid, false);
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
            nei_coords[nei_cnt++] = baseMolecule.getAtomXyz(pyramid[j]);
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
        add(baseMolecule, idx, ATOM_ABS, 0, true);
    else
        add(baseMolecule, idx, ATOM_ABS, 0, false);
}

bool MoleculeStereocenters::hasAtropoStereoBonds(BaseMolecule& /* baseMolecule */, int atom_idx)
{
    return _atropocenters.find(atom_idx) && _atropocenters.at(atom_idx).bond_directions.size();
}

bool MoleculeStereocenters::isPossibleAtropocenter(BaseMolecule& baseMolecule, int atom_idx, int& possible_atropo_bond)
{
    if (baseMolecule.vertexInRing(atom_idx)) // check if the atom belongs to ring
    {
        // bool has_stereo = false;
        const Vertex& v = baseMolecule.getVertex(atom_idx);
        // check if the atom has at least one stereo-bond
        for (int vi = v.neiBegin(); vi != v.neiEnd(); vi = v.neiNext(vi))
        {
            if (baseMolecule.getBondDirection(v.neiEdge(vi)))
            {
                for (int i = v.neiBegin(); i != v.neiEnd(); i = v.neiNext(i))
                {
                    auto bond_idx = v.neiEdge(i);
                    if (baseMolecule.getEdgeTopology(bond_idx) == TOPOLOGY_CHAIN && baseMolecule.getBondOrder(bond_idx) == BOND_SINGLE &&
                        baseMolecule.vertexInRing(v.neiVertex(i)))
                    {
                        std::unordered_set<int> visited;
                        RedBlackMap<int, int> dir_map;
                        visited.insert(bond_idx);
                        if (findAtropoStereobonds(baseMolecule, dir_map, atom_idx, visited, true))
                        {
                            possible_atropo_bond = bond_idx;
                            return true;
                        }
                        //{ // advanced rings search. currently not in use.
                        //    visited.clear();
                        //    visited.insert(bond_idx);
                        //    if (hasRing(baseMolecule, v.neiVertex(i), visited))
                        //    {
                        //        possible_atropo_bond = bond_idx;
                        //        return true;
                        //    }
                        //}
                    }
                }
                break;
            }
        }
    }
    return false;
}

// recursive search of for stereobonds withing connected rings
bool MoleculeStereocenters::findAtropoStereobonds(BaseMolecule& baseMolecule, RedBlackMap<int, int>& directions_map, int atom_idx,
                                                  std::unordered_set<int>& visited_bonds, bool first_only, int* sensible_bonds_out)
{
    const Vertex& v = baseMolecule.getVertex(atom_idx);
    for (int i = v.neiBegin(); i != v.neiEnd(); i = v.neiNext(i))
    {
        auto bond_idx = v.neiEdge(i);
        if (visited_bonds.find(bond_idx) == visited_bonds.end())
        {
            visited_bonds.insert(bond_idx);
            auto bdir = baseMolecule.getBondDirection(bond_idx);
            if (baseMolecule.getBondTopology(bond_idx) == TOPOLOGY_RING)
            {
                if (bdir && !directions_map.find(bond_idx))
                {
                    if (sensible_bonds_out)
                    {
                        if (sensible_bonds_out[bond_idx])
                            continue;
                        else
                            sensible_bonds_out[bond_idx] = 1;
                    }
                    directions_map.insert(bond_idx, bdir);
                    if (first_only)
                        return true;
                }
                findAtropoStereobonds(baseMolecule, directions_map, v.neiVertex(i), visited_bonds, first_only, sensible_bonds_out);
            }
        }
    }
    return directions_map.size();
}

bool MoleculeStereocenters::hasRing(BaseMolecule& baseMolecule, int atom_idx, std::unordered_set<int>& visited_bonds)
{
    const Vertex& v = baseMolecule.getVertex(atom_idx);
    for (int i = v.neiBegin(); i != v.neiEnd(); i = v.neiNext(i))
    {
        auto bond_idx = v.neiEdge(i);
        if (visited_bonds.find(bond_idx) == visited_bonds.end())
        {
            visited_bonds.insert(bond_idx);
            return baseMolecule.getBondTopology(bond_idx) == TOPOLOGY_RING ? true : hasRing(baseMolecule, v.neiVertex(i), visited_bonds);
        }
    }
    return false;
}

bool MoleculeStereocenters::isPossibleStereocenter(BaseMolecule& baseMolecule, int atom_idx, bool* possible_implicit_h, bool* possible_lone_pair)
{
    const Vertex& vertex = baseMolecule.getVertex(atom_idx);

    int sure_double_bonds = 0;
    int possible_double_bonds = 0;

    int degree = vertex.degree();
    if (degree > 4 || degree <= 2)
        return 0;

    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int e_idx = vertex.neiEdge(i);

        if (baseMolecule.getBondOrder(e_idx) == BOND_TRIPLE)
            return false;
        if (baseMolecule.getBondOrder(e_idx) == BOND_AROMATIC)
            return false;

        if (baseMolecule.getBondOrder(e_idx) == BOND_DOUBLE)
            sure_double_bonds++;
        else if (baseMolecule.possibleBondOrder(e_idx, BOND_DOUBLE))
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

    for (i = 0; i < NELEM(allowed_stereocenters); i++)
    {
        const _Configuration& as = allowed_stereocenters[i];

        if (as.degree != vertex.degree())
            continue;

        if (as.n_double_bonds < sure_double_bonds || as.n_double_bonds > sure_double_bonds + possible_double_bonds)
            continue;

        if (!baseMolecule.possibleAtomNumberAndCharge(atom_idx, as.elem, as.charge))
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
bool MoleculeStereocenters::_buildOneCenter(BaseMolecule& baseMolecule, int atom_idx, int* sensible_bonds_out, bool bidirectional_mode,
                                            bool bidirectional_either_mode, const Array<bool>& bond_ignore, bool check_atropocenter)
{
    int possible_atropobond = -1;
    _Atom stereocenter;
    stereocenter.group = 1;
    stereocenter.type = ATOM_ABS;

    if (check_atropocenter && isPossibleAtropocenter(baseMolecule, atom_idx, possible_atropobond))
    {
        stereocenter.is_atropisomeric = true;
        _AtropoCenter& ac = _atropocenters.findOrInsert(atom_idx);
        ac.atropo_bond = possible_atropobond;
        if (_stereocenters.find(atom_idx))
            _stereocenters.at(atom_idx).is_atropisomeric = true;
        else
        {
            stereocenter.is_tetrahydral = false;
            _stereocenters.insert(atom_idx, stereocenter);
        }
    }

    // check if there is a tetrahydral stereocenter already
    if (_stereocenters.find(atom_idx) && _stereocenters.at(atom_idx).is_tetrahydral)
        return true;

    const Vertex& vertex = baseMolecule.getVertex(atom_idx);
    int degree = vertex.degree();
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
    std::unordered_set<int> atropo_bonds_ignore;

    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int e_idx = vertex.neiEdge(i);
        int v_idx = vertex.neiVertex(i);

        edge_ids[nei_idx].edge_idx = e_idx;
        edge_ids[nei_idx].nei_idx = v_idx;

        if (stereocenter.is_atropisomeric && baseMolecule.getBondDirection(e_idx) && baseMolecule.getBondTopology(e_idx) == TOPOLOGY_RING)
            atropo_bonds_ignore.insert(e_idx);
        if (baseMolecule.possibleAtomNumberAndIsotope(v_idx, ELEM_H, 0))
        {
            if (baseMolecule.getAtomNumber(v_idx) == ELEM_H && baseMolecule.getAtomIsotope(v_idx) == 0)
                n_pure_hydrogens++;
            edge_ids[nei_idx].rank = 10000;
        }
        else
            edge_ids[nei_idx].rank = v_idx;

        edge_ids[nei_idx].vec.diff(baseMolecule.getAtomXyz(v_idx), baseMolecule.getAtomXyz(atom_idx));

        if (!edge_ids[nei_idx].vec.normalize())
            zero_bond_length = true;

        if (baseMolecule.getBondOrder(e_idx) == BOND_TRIPLE || baseMolecule.getBondOrder(e_idx) == BOND_AROMATIC)
            return false;

        if (baseMolecule.getBondOrder(e_idx) == BOND_DOUBLE)
            sure_double_bonds++;
        else if (baseMolecule.possibleBondOrder(e_idx, BOND_DOUBLE))
            possible_double_bonds++;

        if (_getDirection(baseMolecule, atom_idx, v_idx, bidirectional_either_mode) == BOND_EITHER)
            is_either = true;

        nei_idx++;
    }

    bool possible_implicit_h = false;
    bool possible_lone_pair = false;

    stereocenter.is_tetrahydral = isPossibleStereocenter(baseMolecule, atom_idx, &possible_implicit_h, &possible_lone_pair);

    if (!stereocenter.is_tetrahydral && !stereocenter.is_atropisomeric)
        return false;

    // Local synonym to get bond direction
    auto getDir = [&](int from, int to) {
        int idx = baseMolecule.findEdgeIndex(from, to);
        if (bond_ignore[idx] /* || atropo_bonds_ignore.find(idx) != atropo_bonds_ignore.end()*/)
            return 0;
        return _getDirection(baseMolecule, from, to, bidirectional_mode);
    };

    if (stereocenter.is_tetrahydral)
    {
        if (is_either)
        {
            stereocenter.type = ATOM_ANY;
            for (int i = 0; i < degree; i++)
            {
                stereocenter.pyramid[i] = edge_ids[i].nei_idx;
                if (getDir(atom_idx, edge_ids[i].nei_idx) > 0)
                    sensible_bonds_out[edge_ids[i].edge_idx] = 1;
            }
        }
        else
        {
            if (degree == 4)
            {
                // sort by neighbor atom index (ascending)
                if (edge_ids[0].rank > edge_ids[1].rank)
                    std::swap(edge_ids[0], edge_ids[1]);
                if (edge_ids[1].rank > edge_ids[2].rank)
                    std::swap(edge_ids[1], edge_ids[2]);
                if (edge_ids[2].rank > edge_ids[3].rank)
                    std::swap(edge_ids[2], edge_ids[3]);
                if (edge_ids[1].rank > edge_ids[2].rank)
                    std::swap(edge_ids[1], edge_ids[2]);
                if (edge_ids[0].rank > edge_ids[1].rank)
                    std::swap(edge_ids[0], edge_ids[1]);
                if (edge_ids[1].rank > edge_ids[2].rank)
                    std::swap(edge_ids[1], edge_ids[2]);

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

                if (main1 != -1)
                {
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
                else
                    stereocenter.is_tetrahydral = false;
            }
            else if (degree == 3)
            {
                // sort by neighbor atom index (ascending)
                if (edge_ids[0].rank > edge_ids[1].rank)
                    std::swap(edge_ids[0], edge_ids[1]);
                if (edge_ids[1].rank > edge_ids[2].rank)
                    std::swap(edge_ids[1], edge_ids[2]);
                if (edge_ids[0].rank > edge_ids[1].rank)
                    std::swap(edge_ids[0], edge_ids[1]);

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

                if (n_down || n_up)
                {
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
                else
                    stereocenter.is_tetrahydral = false;
            }

            if (stereocenter.is_tetrahydral)
                for (int i = 0; i < degree; i++)
                    if (getDir(atom_idx, edge_ids[i].nei_idx) > 0)
                        sensible_bonds_out[edge_ids[i].edge_idx] = 1;
        }
    }

    if (stereocenter.is_tetrahydral)
    {
        if (_stereocenters.find(atom_idx))
        {
            auto& sc = _stereocenters.at(atom_idx);
            sc.is_tetrahydral = true;
            std::copy(std::begin(stereocenter.pyramid), std::end(stereocenter.pyramid), std::begin(sc.pyramid));
        }
        else
            _stereocenters.insert(atom_idx, stereocenter);
        return true;
    }
    return false;
}

// 1 -- in the smaller angle, 2 -- in the bigger angle,
// 4 -- in t5he 'positive' straight angle, 8 -- in the 'negative' straight angle
int MoleculeStereocenters::_xyzzy(const Vec3f& v1, const Vec3f& v2, const Vec3f& u)
{
    const float eps = 1e-3f;

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
    const float eps = 1e-3f;

    if (res > eps)
        return 1;
    if (res < -eps)
        return -1;

    throw Error("degenerate triangle");
}

int MoleculeStereocenters::_onPlane(const Vec3f& v1, const Vec3f& v2, const Vec3f& v3, const Vec3f& u)
{
    Vec3f v1u, v2u, v3u, p;
    const float eps = 0.1f;

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

void MoleculeStereocenters::setAtropisomeric(int idx, bool val)
{
    _stereocenters.at(idx).is_atropisomeric = val;
}

bool MoleculeStereocenters::isAtropisomeric(int idx) const
{
    return _stereocenters.at(idx).is_atropisomeric;
}

void MoleculeStereocenters::setTetrahydral(int idx, bool val)
{
    _stereocenters.at(idx).is_tetrahydral = val;
}

bool MoleculeStereocenters::isTetrahydral(int idx) const
{
    return _stereocenters.at(idx).is_tetrahydral;
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
    int* pyramid = getPyramid(idx);
    std::swap(pyramid[0], pyramid[1]);
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

bool MoleculeStereocenters::haveEnhancedStereocenter()
{
    for (auto i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
    {
        if (_stereocenters.value(i).type == ATOM_OR || _stereocenters.value(i).type == ATOM_AND)
        {
            return true;
        }
    }
    return false;
}

bool MoleculeStereocenters::checkSub(BaseMolecule& query, BaseMolecule& target, const int* mapping, bool reset_h_isotopes, Filter* stereocenters_vertex_filter)
{
    QS_DEF(Array<int>, flags);

    flags.clear_resize(query.stereocenters._stereocenters.end());
    flags.zerofill();

    int i, j;

    for (i = query.stereocenters._stereocenters.begin(); i != query.stereocenters._stereocenters.end(); i = query.stereocenters._stereocenters.next(i))
    {
        if (flags[i])
            continue;

        flags[i] = 1;

        const _Atom& cq = query.stereocenters._stereocenters.value(i);
        int iq = query.stereocenters._stereocenters.key(i);

        if (mapping[iq] < 0)
            continue; // happens only on Exact match (when some query fragments are disabled)

        if (stereocenters_vertex_filter != 0 && !stereocenters_vertex_filter->valid(iq))
            continue;

        if (cq.type < ATOM_AND || !cq.is_tetrahydral)
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
            for (j = i; j != query.stereocenters._stereocenters.end(); j = query.stereocenters._stereocenters.next(j))
            {
                int iq2 = query.stereocenters._stereocenters.key(j);
                const _Atom& cq2 = query.stereocenters._stereocenters.value(j);

                if (cq2.type != type)
                    continue;

                if (cq2.group != cq.group)
                    continue;

                const _Atom* ct2 = target.stereocenters._stereocenters.at2(mapping[iq2]);

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
    int arr[4];
    bool rigid = true;

    memcpy(arr, mapping, 4 * sizeof(int));

    if (arr[0] > arr[1])
        std::swap(arr[0], arr[1]), rigid = !rigid;
    if (arr[1] > arr[2])
        std::swap(arr[1], arr[2]), rigid = !rigid;
    if (arr[2] > arr[3])
        std::swap(arr[2], arr[3]), rigid = !rigid;
    if (arr[1] > arr[2])
        std::swap(arr[1], arr[2]), rigid = !rigid;
    if (arr[0] > arr[1])
        std::swap(arr[0], arr[1]), rigid = !rigid;
    if (arr[1] > arr[2])
        std::swap(arr[1], arr[2]), rigid = !rigid;

    return rigid;
}

bool MoleculeStereocenters::isPyramidMappingRigid_Sort(int* pyramid, const int* mapping)
{
    bool rigid = true;
    int i;

    for (i = 0; i < 4; i++)
        if (pyramid[i] != -1 && mapping[pyramid[i]] < 0)
            pyramid[i] = -1;

    if (pyramid[0] == -1 || (pyramid[1] >= 0 && mapping[pyramid[0]] > mapping[pyramid[1]]))
        std::swap(pyramid[0], pyramid[1]), rigid = !rigid;
    if (pyramid[1] == -1 || (pyramid[2] >= 0 && mapping[pyramid[1]] > mapping[pyramid[2]]))
        std::swap(pyramid[1], pyramid[2]), rigid = !rigid;
    if (pyramid[2] == -1 || (pyramid[3] >= 0 && mapping[pyramid[2]] > mapping[pyramid[3]]))
        std::swap(pyramid[2], pyramid[3]), rigid = !rigid;
    if (pyramid[1] == -1 || (pyramid[2] >= 0 && mapping[pyramid[1]] > mapping[pyramid[2]]))
        std::swap(pyramid[1], pyramid[2]), rigid = !rigid;
    if (pyramid[0] == -1 || (pyramid[1] >= 0 && mapping[pyramid[0]] > mapping[pyramid[1]]))
        std::swap(pyramid[0], pyramid[1]), rigid = !rigid;
    if (pyramid[1] == -1 || (pyramid[2] >= 0 && mapping[pyramid[1]] > mapping[pyramid[2]]))
        std::swap(pyramid[1], pyramid[2]), rigid = !rigid;

    return rigid;
}

bool MoleculeStereocenters::isPyramidMappingRigid(const int* pyramid, int size, const Array<int>& mapping)
{
    for (int i = 0; i < size; ++i)
    {
        if (pyramid[i] >= mapping.size() || pyramid[i] < 0)
            return false;
    }

    if (size == 3)
    {
        int order[3] = {mapping[pyramid[0]], mapping[pyramid[1]], mapping[pyramid[2]]};
        int min = *std::min_element(order, order + NELEM(order));

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

void MoleculeStereocenters::getPyramidMapping(BaseMolecule& query, BaseMolecule& target, int query_atom, const int* mapping, int* mapping_out,
                                              bool /* reset_h_isotopes */)
{
    int i, j;

    const int* seq1 = query.getPyramidStereocenters(query_atom);
    const int* seq2 = target.getPyramidStereocenters(mapping[query_atom]);

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
            if (query.getAtomNumber(seq1[i]) != ELEM_H)
                throw Error("unmapped non-hydrogen atom (atom number %d)", query.getAtomNumber(seq1[i]));
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
                if (target.getAtomNumber(seq2[j]) == ELEM_H)
                    break; // match to explicit hydrogen

                // rare cases like '[S@](F)(Cl)=O' on '[S@](F)(Cl)(=O)=N'
                if (seq1[i] == -1 && target.getAtomNumber(mapping[query_atom]) == ELEM_S)
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

void MoleculeStereocenters::removeAtoms(BaseMolecule& baseMolecule, const Array<int>& indices)
{
    for (int i = 0; i < indices.size(); i++)
    {
        int idx = indices[i];
        if (_stereocenters.find(idx))
            _stereocenters.remove(idx);
        else
        {
            const Vertex& vertex = baseMolecule.getVertex(idx);

            for (int k = vertex.neiBegin(); k != vertex.neiEnd(); k = vertex.neiNext(k))
            {
                int nei_vertex = vertex.neiVertex(k);
                _removeBondDir(baseMolecule, idx, nei_vertex);
            }
        }
    }
}

void MoleculeStereocenters::removeBonds(BaseMolecule& baseMolecule, const Array<int>& indices)
{
    for (int i = 0; i < indices.size(); i++)
    {
        const Edge& edge = baseMolecule.getEdge(indices[i]);

        _removeBondDir(baseMolecule, edge.beg, edge.end);
        _removeBondDir(baseMolecule, edge.end, edge.beg);
    }
}

void MoleculeStereocenters::_removeBondDir(BaseMolecule& baseMolecule, int atom_from, int atom_to)
{
    _Atom* stereo_atom = _stereocenters.at2(atom_to);
    if (stereo_atom != 0)
    {
        if (stereo_atom->pyramid[3] == -1)
            _stereocenters.remove(atom_to);
        else
        {
            if (!baseMolecule.isQueryMolecule() || baseMolecule.possibleAtomNumber(atom_from, ELEM_H) || baseMolecule.isRSite(atom_from))
                _convertAtomToImplicitHydrogen(stereo_atom->pyramid, atom_from);
        }
    }
}

void MoleculeStereocenters::buildOnSubmolecule(BaseMolecule& baseMolecule, const BaseMolecule& super, int* mapping)
{
    int i, j;
    for (i = super.stereocenters._stereocenters.begin(); i != super.stereocenters._stereocenters.end(); i = super.stereocenters._stereocenters.next(i))
    {
        int super_idx = super.stereocenters._stereocenters.key(i);
        const _Atom& super_stereocenter = super.stereocenters._stereocenters.value(i);
        int sub_idx = mapping[super_idx];

        if (sub_idx < 0)
            continue;

        _Atom new_stereocenter;

        new_stereocenter.group = super_stereocenter.group;
        new_stereocenter.type = super_stereocenter.type;
        new_stereocenter.is_atropisomeric = super_stereocenter.is_atropisomeric;
        new_stereocenter.is_tetrahydral = super_stereocenter.is_tetrahydral;

        // copy tetrahydral center
        if (new_stereocenter.is_tetrahydral)
        {
            for (j = 0; j < 4; j++)
            {
                int idx = super_stereocenter.pyramid[j];

                if (idx == -1)
                    new_stereocenter.pyramid[j] = -1;
                else
                {
                    int val = mapping[idx];
                    if (val != -1 && baseMolecule.findEdgeIndex(sub_idx, val) == -1)
                        val = -1;
                    new_stereocenter.pyramid[j] = val;
                }
            }

            moveMinimalToEnd(new_stereocenter.pyramid);
            // copy bond directions for tetrahydral center
            const Vertex& super_vertex = super.getVertex(super_idx);
            for (j = super_vertex.neiBegin(); j != super_vertex.neiEnd(); j = super_vertex.neiNext(j))
            {
                int super_edge = super_vertex.neiEdge(j);
                if (mapping[super_vertex.neiVertex(j)] == -1)
                    continue;

                int bdir = super.getBondDirection(super_edge);
                if (bdir)
                    baseMolecule.setBondDirection(baseMolecule.findEdgeIndex(sub_idx, mapping[super_vertex.neiVertex(j)]), bdir);
            }
        }

        // copy atropocenter
        if (new_stereocenter.is_atropisomeric && super.stereocenters._atropocenters.find(super_idx))
        {
            const auto& ac_super = super.stereocenters._atropocenters.at(super_idx);
            auto& ac_new = baseMolecule.stereocenters._atropocenters.findOrInsert(sub_idx);
            const auto& e = super.getEdge(ac_super.atropo_bond);
            ac_new.atropo_bond = baseMolecule.findEdgeIndex(mapping[e.beg], mapping[e.end]);
            // copy bond directions for atropisomeric center
            ac_new.bond_directions.clear();
            for (j = ac_super.bond_directions.begin(); j != ac_super.bond_directions.end(); j = ac_super.bond_directions.next(j))
            {
                const auto& atropo_edge = super.getEdge(ac_super.bond_directions.key(j));
                int atropo_edge_idx = baseMolecule.findEdgeIndex(mapping[atropo_edge.beg], mapping[atropo_edge.end]);
                int bdir = ac_super.bond_directions.value(j);
                ac_new.bond_directions.insert(atropo_edge_idx, bdir);
                baseMolecule.setBondDirection(atropo_edge_idx, bdir);
            }
        }
        _stereocenters.insert(sub_idx, new_stereocenter);
    }
}

int MoleculeStereocenters::size() const
{
    return _stereocenters.size();
}

void MoleculeStereocenters::add(BaseMolecule& baseMolecule, int atom_idx, int type, int group, bool inverse_pyramid)
{
    int pyramid[4];
    _restorePyramid(baseMolecule, atom_idx, pyramid, inverse_pyramid);
    add(baseMolecule, atom_idx, type, group, pyramid);
}

void MoleculeStereocenters::add(BaseMolecule& /* baseMolecule */, int atom_idx, int type, int group, const int pyramid[4])
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

void MoleculeStereocenters::add_ignore(BaseMolecule& baseMolecule, int atom_idx, int type, int group, bool inverse_pyramid)
{
    int pyramid[4];
    _restorePyramid(baseMolecule, atom_idx, pyramid, inverse_pyramid);
    add_ignore(baseMolecule, atom_idx, type, group, pyramid);
}

void MoleculeStereocenters::add_ignore(BaseMolecule& /* baseMolecule */, int atom_idx, int type, int group, const int pyramid[4])
{
    if (atom_idx < 0)
        throw Error("stereocenter index is invalid");

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

void MoleculeStereocenters::_restorePyramid(BaseMolecule& baseMolecule, int idx, int pyramid[4], int invert_pyramid)
{
    const Vertex& vertex = baseMolecule.getVertex(idx);
    int j, count = 0;

    pyramid[0] = -1;
    pyramid[1] = -1;
    pyramid[2] = -1;
    pyramid[3] = -1;

    for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
    {
        int nei = vertex.neiVertex(j);

        if (vertex.degree() == 3 || baseMolecule.getAtomNumber(nei) != ELEM_H || baseMolecule.getAtomIsotope(nei) != 0)
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

    // sort pyramid indices
    if (pyramid[2] == -1)
    {
        if (pyramid[0] > pyramid[1])
            std::swap(pyramid[0], pyramid[1]);
    }
    else if (pyramid[3] == -1)
    {
        if (pyramid[0] > pyramid[1])
            std::swap(pyramid[0], pyramid[1]);
        if (pyramid[1] > pyramid[2])
            std::swap(pyramid[1], pyramid[2]);
        if (pyramid[0] > pyramid[1])
            std::swap(pyramid[0], pyramid[1]);
    }
    else
    {
        if (pyramid[0] > pyramid[1])
            std::swap(pyramid[0], pyramid[1]);
        if (pyramid[1] > pyramid[2])
            std::swap(pyramid[1], pyramid[2]);
        if (pyramid[2] > pyramid[3])
            std::swap(pyramid[2], pyramid[3]);
        if (pyramid[1] > pyramid[2])
            std::swap(pyramid[1], pyramid[2]);
        if (pyramid[0] > pyramid[1])
            std::swap(pyramid[0], pyramid[1]);
        if (pyramid[1] > pyramid[2])
            std::swap(pyramid[1], pyramid[2]);
    }
    if (invert_pyramid)
        std::swap(pyramid[1], pyramid[2]);
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
        std::swap(pyramid[0], pyramid[1]);
}

void MoleculeStereocenters::moveMinimalToEnd(int pyramid[4])
{
    int min_element = std::min(std::min(pyramid[0], pyramid[1]), std::min(pyramid[2], pyramid[3]));
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

void MoleculeStereocenters::markBond(BaseMolecule& baseMolecule, int atom_idx)
{
    const _Atom* atom_ptr = _stereocenters.at2(atom_idx);
    if (atom_ptr == NULL)
        return;

    const _Atom& atom = *atom_ptr;

    if (atom.is_tetrahydral)
    {
        int pyramid[4];
        int mult = 1;
        int size = 0;
        int j;

        memcpy(pyramid, atom.pyramid, 4 * sizeof(int));

        const Vertex& vertex = baseMolecule.getVertex(atom_idx);
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
            if (baseMolecule.getBondDirection2(atom_idx, vertex.neiVertex(j)) != 0)
                baseMolecule.setBondDirection(vertex.neiEdge(j), 0);

        int edge_idx = -1;

        for (j = 0; j < size; j++)
        {
            edge_idx = baseMolecule.findEdgeIndex(atom_idx, pyramid[size - 1]);
            if (baseMolecule.getBondDirection(edge_idx) == 0 && baseMolecule.getBondOrder(edge_idx) == BOND_SINGLE &&
                baseMolecule.getVertex(pyramid[size - 1]).degree() == 1)
                break;
            rotatePyramid(pyramid);
            if (size == 4)
                mult = -mult;
        }

        if (j == size)
        {
            for (j = 0; j < size; j++)
            {
                edge_idx = baseMolecule.findEdgeIndex(atom_idx, pyramid[size - 1]);
                if (baseMolecule.getBondDirection(edge_idx) == 0 && baseMolecule.getBondOrder(edge_idx) == BOND_SINGLE &&
                    baseMolecule.getBondTopology(edge_idx) == TOPOLOGY_CHAIN && getType(pyramid[size - 1]) == 0)
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
                edge_idx = baseMolecule.findEdgeIndex(atom_idx, pyramid[size - 1]);
                if (baseMolecule.getBondDirection(edge_idx) == 0 && getType(pyramid[size - 1]) == 0)
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
                edge_idx = baseMolecule.findEdgeIndex(atom_idx, pyramid[size - 1]);
                if (baseMolecule.getBondDirection(edge_idx) == 0 && baseMolecule.getBondTopology(edge_idx) == TOPOLOGY_CHAIN)
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
                edge_idx = baseMolecule.findEdgeIndex(atom_idx, pyramid[size - 1]);
                if (baseMolecule.getBondDirection(edge_idx) == 0)
                    break;
                rotatePyramid(pyramid);
                if (size == 4)
                    mult = -mult;
            }
        }

        if (j == size)
            throw Error("no bond can be marked");

        if (baseMolecule.getEdge(edge_idx).beg != atom_idx)
            baseMolecule.swapEdgeEnds(edge_idx);

        if (BaseMolecule::hasCoord(baseMolecule))
        {
            if (atom.type > ATOM_ANY)
            {
                std::array<Vec3f, 4> dirs;
                dirs.fill({0.0, 0.0, 0.0});
                for (j = 0; j < size; j++)
                {
                    dirs[j] = baseMolecule.getAtomXyz(pyramid[j]);
                    dirs[j].sub(baseMolecule.getAtomXyz(atom_idx));
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
                        baseMolecule.setBondDirection(edge_idx, (sign * mult == 1) ? BOND_DOWN : BOND_UP);
                    }
                    else
                        baseMolecule.setBondDirection(edge_idx, (sign == 1) ? BOND_DOWN : BOND_UP);
                }
                else
                    baseMolecule.setBondDirection(edge_idx, (sign * mult == 1) ? BOND_UP : BOND_DOWN);
            }
            else
                baseMolecule.setBondDirection(edge_idx, BOND_EITHER);
        }
    }
}

void MoleculeStereocenters::markAtropisomericBond(BaseMolecule& baseMolecule, int atom_idx)
{
    const _Atom* atom_ptr = _stereocenters.at2(atom_idx);
    if (atom_ptr == NULL)
        return;
    const _Atom& atom = *atom_ptr;
    if (atom.is_atropisomeric)
    {
        const auto& ac = _atropocenters.at(atom_idx);
        for (int i = ac.bond_directions.begin(); i != ac.bond_directions.end(); i = ac.bond_directions.next(i))
        {
            int bond_idx = ac.bond_directions.key(i);
            int bdir = ac.bond_directions.value(i);
            baseMolecule.setBondDirection(bond_idx, bdir);
        }
    }
}

void MoleculeStereocenters::markBonds(BaseMolecule& baseMolecule)
{
    for (int i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
        markBond(baseMolecule, _stereocenters.key(i));
    for (int i = _stereocenters.begin(); i != _stereocenters.end(); i = _stereocenters.next(i))
        markAtropisomericBond(baseMolecule, _stereocenters.key(i));
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

        MoleculeStereocenters::getPyramidMapping(mol, mol, idx, mapping.ptr(), pyra_map, false);

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
