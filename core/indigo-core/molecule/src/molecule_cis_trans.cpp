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

#include "molecule/molecule_cis_trans.h"

#include "graph/filter.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"

using namespace indigo;

IMPL_ERROR(MoleculeCisTrans, "cis-trans");

int MoleculeCisTrans::sameside(const Vec3f& beg, const Vec3f& end, const Vec3f& nei_beg, const Vec3f& nei_end)
{
    Vec3f norm, norm_cross;
    Vec3f diff, norm_beg, norm_end;

    diff.diff(beg, end);

    norm_beg.diff(nei_beg, beg);

    // Use double cross product for getting vector lying the same plane with (beg-end) and (nei_beg-beg)
    norm_cross.cross(diff, norm_beg);
    norm.cross(norm_cross, diff);

    if (!norm.normalize())
        return 0;
    // throw Error("cannot normalize stereo double bond");

    norm_end.diff(nei_end, end);

    if (!norm_beg.normalize())
        return 0;
    // throw Error("cannot normalize neighbor bond of stereo double bond");
    if (!norm_end.normalize())
        return 0;
    // throw Error("cannot normalize neighbor bond of stereo double bond");

    float prod_beg = Vec3f::dot(norm_beg, norm);
    float prod_end = Vec3f::dot(norm_end, norm);

    if ((float)(fabs(prod_beg)) < 0.1 || (float)(fabs(prod_end)) < 0.1)
        return 0;
    // throw Error("double stereo bond is collinear with its neighbor bond");

    return (prod_beg * prod_end > 0) ? 1 : -1;
}

int MoleculeCisTrans::_sameside(BaseMolecule& molecule, int i_beg, int i_end, int i_nei_beg, int i_nei_end)
{
    return sameside(molecule.getAtomXyz(i_beg), molecule.getAtomXyz(i_end), molecule.getAtomXyz(i_nei_beg), molecule.getAtomXyz(i_nei_end));
}

bool MoleculeCisTrans::sameline(const Vec3f& beg, const Vec3f& end, const Vec3f& nei_beg)
{
    Vec3f norm_diff, norm_beg;

    norm_diff.diff(beg, end);
    if (!norm_diff.normalize())
        return true;
    norm_beg.diff(nei_beg, beg);
    if (!norm_beg.normalize())
        return true;

    Vec3f cross;
    cross.cross(norm_diff, norm_beg);
    float sin_angle = cross.lengthSqr();
    if (fabs(sin_angle) < 0.01)
        return true;

    return false;
}

bool MoleculeCisTrans::_sameline(BaseMolecule& molecule, int i_beg, int i_end, int i_nei_beg)
{
    return sameline(molecule.getAtomXyz(i_beg), molecule.getAtomXyz(i_end), molecule.getAtomXyz(i_nei_beg));
}

bool MoleculeCisTrans::_pureH(BaseMolecule& mol, int idx)
{
    return mol.getAtomNumber(idx) == ELEM_H && mol.getAtomIsotope(idx) == 0;
}

bool MoleculeCisTrans::_commonHasLonePair(BaseMolecule& mol, int v1, int v2)
{
    if (v1 != -1 && v2 != -1)
        return false;
    const Vertex* v;
    if (v1 == -1)
        v = &mol.getVertex(v2);
    else
        v = &mol.getVertex(v1);
    int common = v->neiVertex(v->neiBegin());
    if (mol.getAtomNumber(common) == ELEM_N && mol.getAtomCharge(common) == 0)
        return true;
    return false;
}

bool MoleculeCisTrans::convertableToImplicitHydrogen(BaseMolecule& baseMolecule, int idx)
{
    // check [H]\N=C\C
    const Vertex& v = baseMolecule.getVertex(idx);
    int nei = v.neiVertex(v.neiBegin());

    // Find double bond
    const Vertex& base = baseMolecule.getVertex(nei);
    for (int i = base.neiBegin(); i != base.neiEnd(); i = base.neiNext(i))
    {
        int edge = base.neiEdge(i);
        if (baseMolecule.getBondOrder(edge) == BOND_DOUBLE)
            return getParity(edge) == 0 || base.degree() != 2;
    }
    return true;
}

bool MoleculeCisTrans::sortSubstituents(BaseMolecule& mol, int* substituents, bool* parity_changed)
{
    bool e0 = substituents[0] < 0;
    bool e1 = substituents[1] < 0;
    bool e2 = substituents[2] < 0;
    bool e3 = substituents[3] < 0;

    if (e0 && e1)
        return false;
    if (e2 && e3)
        return false;

    bool h0 = !e0 && _pureH(mol, substituents[0]);
    bool h1 = !e1 && _pureH(mol, substituents[1]);
    bool h2 = !e2 && _pureH(mol, substituents[2]);
    bool h3 = !e3 && _pureH(mol, substituents[3]);
    // Query molecules [H]/C=C/C and [H]\C=C/C are different
    // But normal molecules are the same.
    if (!mol.isQueryMolecule())
    {
        // Handle [H]/N=C\C and [H]/N=C/C
        if (!_commonHasLonePair(mol, substituents[0], substituents[1]))
        {
            h0 |= e0;
            h1 |= e1;
        }
        if (!_commonHasLonePair(mol, substituents[2], substituents[3]))
        {
            h2 |= e2;
            h3 |= e3;
        }
    }

    if (h0 && h1)
        return false;
    if (h2 && h3)
        return false;

    // If hydrogens are explicit then keep them
    // And do not place explicit hydrogens to the end, because all static methods
    // should be converted into non-static with checking whether atom is hydrogen
    // or not.
    // For example, molecule [H]\C(O)=C/C can get invalid parity because static
    // functions getMappingParitySign, applyMapping doesn't know about
    bool swapped = false;
    if (!e1)
        if (e0 || substituents[0] > substituents[1])
        {
            std::swap(substituents[0], substituents[1]);
            swapped = !swapped;
        }

    if (!e3)
        if (e2 || substituents[2] > substituents[3])
        {
            std::swap(substituents[2], substituents[3]);
            swapped = !swapped;
        }

    if (parity_changed != 0)
        *parity_changed = swapped;

    return true;
}

bool MoleculeCisTrans::isGeomStereoBond(BaseMolecule& mol, int bond_idx, int* substituents, bool have_xyz)
{
    int substituents_local[4];

    if (substituents == 0)
        substituents = substituents_local;

    // it must be [C,N,Si,Ge]=[C,N,Si,Ge] bond
    if (!mol.possibleBondOrder(bond_idx, BOND_DOUBLE))
        return false;

    const Edge& edge = mol.getEdge(bond_idx);
    int beg_idx = edge.beg;
    int end_idx = edge.end;

    if (!mol.possibleAtomNumber(beg_idx, ELEM_C) && !mol.possibleAtomNumber(beg_idx, ELEM_N) && !mol.possibleAtomNumber(beg_idx, ELEM_Si) &&
        !mol.possibleAtomNumber(beg_idx, ELEM_Ge))
        return false;
    if (!mol.possibleAtomNumber(end_idx, ELEM_C) && !mol.possibleAtomNumber(end_idx, ELEM_N) && !mol.possibleAtomNumber(end_idx, ELEM_Si) &&
        !mol.possibleAtomNumber(end_idx, ELEM_Ge))
        return false;

    // Double bonds with R-sites are excluded because cis-trans configuration
    // cannot be determined when R-site is substituted with R-group
    if (mol.isRSite(beg_idx) || mol.isRSite(end_idx))
        return false;

    // the atoms should have 1 or 2 single bonds
    // (apart from the double bond under consideration)
    const Vertex& beg = mol.getVertex(beg_idx);
    const Vertex& end = mol.getVertex(end_idx);

    if (beg.degree() < 2 || beg.degree() > 3 || end.degree() < 2 || end.degree() > 3)
        return false;

    // If double bond is inside single cycle with size 7 or smaller it can be just cis
    // it requires additional check
    /*
       if ( (mol.getEdgeTopology(bond_idx) == TOPOLOGY_RING) && (mol.edgeSmallestRingSize(bond_idx) <= 7) &&
            (mol.vertexSmallestRingSize(beg_idx) <= 7) && (mol.vertexSmallestRingSize(end_idx) <= 7) )
          return false;
    */

    substituents[0] = -1;
    substituents[1] = -1;
    substituents[2] = -1;
    substituents[3] = -1;

    int i;

    for (i = beg.neiBegin(); i != beg.neiEnd(); i = beg.neiNext(i))
    {
        int nei_edge_idx = beg.neiEdge(i);

        if (nei_edge_idx == bond_idx)
            continue;

        if (!mol.possibleBondOrder(nei_edge_idx, BOND_SINGLE) && !mol.possibleBondOrder(nei_edge_idx, BOND_AROMATIC))
            return false;

        if (substituents[0] == -1)
            substituents[0] = beg.neiVertex(i);
        else // (substituents[1] == -1)
            substituents[1] = beg.neiVertex(i);
    }

    for (i = end.neiBegin(); i != end.neiEnd(); i = end.neiNext(i))
    {
        int nei_edge_idx = end.neiEdge(i);

        if (nei_edge_idx == bond_idx)
            continue;

        if (!mol.possibleBondOrder(nei_edge_idx, BOND_SINGLE) && !mol.possibleBondOrder(nei_edge_idx, BOND_AROMATIC))
            return false;

        if (substituents[2] == -1)
            substituents[2] = end.neiVertex(i);
        else // (substituents[3] == -1)
            substituents[3] = end.neiVertex(i);
    }

    // Check trianges when substituents are the same: CC1=C(N)C1
    if (substituents[0] >= 0)
        if (substituents[0] == substituents[2] || substituents[0] == substituents[3])
            return false;
    if (substituents[1] >= 0)
        if (substituents[1] == substituents[2] || substituents[1] == substituents[3])
            return false;

    if (have_xyz)
    {
        if (substituents[1] != -1 && _sameside(mol, beg_idx, end_idx, substituents[0], substituents[1]) != -1)
            return false;
        else if (_sameline(mol, beg_idx, end_idx, substituents[0]))
            return false;

        if (substituents[3] != -1 && _sameside(mol, beg_idx, end_idx, substituents[2], substituents[3]) != -1)
            return false;
        else if (_sameline(mol, beg_idx, end_idx, substituents[2]))
            return false;
    }

    return true;
}

void MoleculeCisTrans::restoreSubstituents(BaseMolecule& baseMolecule, int bond_idx)
{
    _Bond& bond = _bonds[bond_idx];
    int* substituents = bond.substituents;

    if (!bond.ignored)
    {
        if (!isGeomStereoBond(baseMolecule, bond_idx, substituents, false))
            throw Error("restoreSubstituents(): not a cis-trans bond");

        if (!sortSubstituents(baseMolecule, substituents, 0))
            throw Error("can't sort restored substituents");
    }
}

void MoleculeCisTrans::registerUnfoldedHydrogen(BaseMolecule& baseMolecule, int atom_idx, int added_hydrogen)
{
    const Vertex& vertex = baseMolecule.getVertex(atom_idx);
    int i;

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int bond_idx = vertex.neiEdge(i);
        const Edge& edge = baseMolecule.getEdge(bond_idx);

        if (_bonds.size() <= bond_idx)
            continue;

        if (_bonds[bond_idx].parity == 0)
            continue;

        int* subst = _bonds[bond_idx].substituents;

        if (atom_idx == edge.beg && subst[1] == -1)
            subst[1] = added_hydrogen;
        if (atom_idx == edge.end && subst[3] == -1)
            subst[3] = added_hydrogen;
    }
}

MoleculeCisTrans::MoleculeCisTrans()
{
}

void MoleculeCisTrans::clear()
{
    _bonds.clear();
}

bool MoleculeCisTrans::exists() const
{
    return _bonds.size() > 0;
}

void MoleculeCisTrans::registerBond(int idx)
{
    while (_bonds.size() <= idx)
        _bonds.push().clear();
    _bonds[idx].clear();
}

void MoleculeCisTrans::validate(BaseMolecule& baseMolecule)
{
    for (int i = baseMolecule.edgeBegin(); i != baseMolecule.edgeEnd(); i = baseMolecule.edgeNext(i))
    {
        if (getParity(i) != 0)
        {
            int subs[4];
            if (!isGeomStereoBond(baseMolecule, i, subs, false))
                setParity(i, 0);
        }
    }
}

bool MoleculeCisTrans::registerBondAndSubstituents(BaseMolecule& baseMolecule, int idx)
{
    registerBond(idx);

    if (!isGeomStereoBond(baseMolecule, idx, _bonds[idx].substituents, false))
        return false;

    if (!sortSubstituents(baseMolecule, _bonds[idx].substituents, 0))
        return false;

    return true;
}

void MoleculeCisTrans::build(BaseMolecule& baseMolecule, int* exclude_bonds)
{
    int i;

    clear();
    _bonds.clear_resize(baseMolecule.edgeEnd());
    for (i = baseMolecule.edgeBegin(); i != baseMolecule.edgeEnd(); i = baseMolecule.edgeNext(i))
    {
        _bonds[i].parity = 0;
        _bonds[i].ignored = 0;

        int beg = baseMolecule.getEdge(i).beg;
        int end = baseMolecule.getEdge(i).end;

        int* substituents = _bonds[i].substituents;

        // Ignore only bonds that can be cis-trans ?
        // Ignore bonds marked as ignored
        if (exclude_bonds != 0 && exclude_bonds[i])
        {
            _bonds[i].ignored = 1;
            continue;
        }

        bool have_xyz = true;
        // If bond is marked with ignore flag then read this flag
        // even if coordinates are not valid.
        if (exclude_bonds != 0 && exclude_bonds[i])
            have_xyz = false;
        if (!isGeomStereoBond(baseMolecule, i, substituents, have_xyz))
            continue;

        if (!sortSubstituents(baseMolecule, substituents, 0))
            continue;

        int sign = _sameside(baseMolecule, beg, end, substituents[0], substituents[2]);

        if (sign == 1)
            setParity(i, CIS);
        else if (sign == -1)
            setParity(i, TRANS);
    }
}

void MoleculeCisTrans::buildFromSmiles(BaseMolecule& baseMolecule, int* dirs)
{
    QS_DEF(Array<int>, subst_used);
    int i, j;

    clear();
    subst_used.clear_resize(baseMolecule.vertexEnd());
    subst_used.zerofill();

    _bonds.clear_resize(baseMolecule.edgeEnd());

    for (i = baseMolecule.edgeBegin(); i != baseMolecule.edgeEnd(); i = baseMolecule.edgeNext(i))
    {
        if (!registerBondAndSubstituents(baseMolecule, i))
            continue;

        int beg = baseMolecule.getEdge(i).beg;
        int end = baseMolecule.getEdge(i).end;

        int substituents[4];
        getSubstituents_All(baseMolecule, i, substituents);

        int subst_dirs[4] = {0, 0, 0, 0};
        int nei_edge;

        nei_edge = baseMolecule.findEdgeIndex(beg, substituents[0]);

        if (dirs[nei_edge] == 1)
            subst_dirs[0] = baseMolecule.getEdge(nei_edge).beg == beg ? 1 : 2;
        if (dirs[nei_edge] == 2)
            subst_dirs[0] = baseMolecule.getEdge(nei_edge).beg == beg ? 2 : 1;

        if (substituents[1] != -1)
        {
            nei_edge = baseMolecule.findEdgeIndex(beg, substituents[1]);

            if (dirs[nei_edge] == 1)
                subst_dirs[1] = baseMolecule.getEdge(nei_edge).beg == beg ? 1 : 2;
            if (dirs[nei_edge] == 2)
                subst_dirs[1] = baseMolecule.getEdge(nei_edge).beg == beg ? 2 : 1;
        }

        nei_edge = baseMolecule.findEdgeIndex(end, substituents[2]);

        if (dirs[nei_edge] == 1)
            subst_dirs[2] = baseMolecule.getEdge(nei_edge).beg == end ? 1 : 2;
        if (dirs[nei_edge] == 2)
            subst_dirs[2] = baseMolecule.getEdge(nei_edge).beg == end ? 2 : 1;

        if (substituents[3] != -1)
        {
            nei_edge = baseMolecule.findEdgeIndex(end, substituents[3]);

            if (dirs[nei_edge] == 1)
                subst_dirs[3] = baseMolecule.getEdge(nei_edge).beg == end ? 1 : 2;
            if (dirs[nei_edge] == 2)
                subst_dirs[3] = baseMolecule.getEdge(nei_edge).beg == end ? 2 : 1;
        }

        if ((subst_dirs[0] != 0 && subst_dirs[0] == subst_dirs[1]) || (subst_dirs[2] != 0 && subst_dirs[2] == subst_dirs[3]))
            // throw Error("cis-trans bonds %d have co-directed subsituents", i);
            // can happen on fragments such as CC=C(C=CN)C=CO
            continue;

        if ((subst_dirs[0] == 0 && subst_dirs[1] == 0) || (subst_dirs[2] == 0 && subst_dirs[3] == 0))
            continue;

        if (subst_dirs[1] == 1)
            subst_dirs[0] = 2;
        else if (subst_dirs[1] == 2)
            subst_dirs[0] = 1;

        if (subst_dirs[3] == 1)
            subst_dirs[2] = 2;
        else if (subst_dirs[3] == 2)
            subst_dirs[2] = 1;

        if (subst_dirs[0] == subst_dirs[2])
            setParity(i, CIS);
        else
            setParity(i, TRANS);

        for (j = 0; j < 4; j++)
            if (substituents[j] != -1)
                subst_used[substituents[j]] = 1;
    }

    /*for (i = _baseMolecule.edgeBegin(); i != _baseMolecule.edgeEnd(); i = _baseMolecule.edgeNext(i))
    {
       if (dirs[i] == 0)
          continue;

       const Edge &edge = _baseMolecule.getEdge(i);

       if (!subst_used[edge.beg] && !subst_used[edge.end])
          throw Error("direction of bond %d makes no sense", i);
    }*/
}

int MoleculeCisTrans::getParity(int bond_idx) const
{
    if (bond_idx >= _bonds.size())
        return 0;
    return _bonds[bond_idx].parity;
}

bool MoleculeCisTrans::isIgnored(int bond_idx) const
{
    if (bond_idx >= _bonds.size())
        return false;
    return _bonds[bond_idx].ignored == 1;
}

void MoleculeCisTrans::ignore(int bond_idx)
{
    while (bond_idx >= _bonds.size())
        _bonds.push().clear();
    _bonds[bond_idx].parity = 0;
    _bonds[bond_idx].ignored = 1;
}

void MoleculeCisTrans::setParity(int bond_idx, int parity)
{
    while (_bonds.size() <= bond_idx)
        _bonds.push().clear();
    _bonds[bond_idx].parity = parity;
}

const int* MoleculeCisTrans::getSubstituents(int bond_idx) const
{
    return _bonds[bond_idx].substituents;
}

void MoleculeCisTrans::_fillAtomExplicitHydrogens(BaseMolecule& mol, int atom_idx, int subst[2])
{
    const Vertex& vertex = mol.getVertex(atom_idx);

    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int nei = vertex.neiVertex(i);
        // check [H]\N=C\C
        if (_pureH(mol, nei))
        {
            if (subst[0] != nei)
                subst[1] = nei;
            else if (subst[1] != nei)
                subst[0] = nei;
            else
                throw Error("internal error in _fillAtomExplicitHydrogens");
        }
    }
}

void MoleculeCisTrans::_fillExplicitHydrogens(BaseMolecule& mol, int bond_idx, int subst[4])
{
    _fillAtomExplicitHydrogens(mol, mol.getEdge(bond_idx).beg, subst);
    _fillAtomExplicitHydrogens(mol, mol.getEdge(bond_idx).end, subst + 2);
}

void MoleculeCisTrans::getSubstituents_All(BaseMolecule& baseMolecule, int bond_idx, int subst[4])
{
    memcpy(subst, _bonds[bond_idx].substituents, 4 * sizeof(int));
    _fillExplicitHydrogens(baseMolecule, bond_idx, subst);
}

void MoleculeCisTrans::add(int bond_idx, int substituents[4], int parity)
{
    registerBond(bond_idx);
    setParity(bond_idx, parity);
    for (int i = 0; i < 4; i++)
        _bonds[bond_idx].substituents[i] = substituents[i];
}

int MoleculeCisTrans::_getPairParity(int v1, int v2, const int* mapping, bool sort)
{
    if (v1 < 0 || mapping[v1] < 0)
    {
        if (v2 < 0 || mapping[v2] < 0)
            // Both mapped values result in undefined parity
            return 0;
        // v1 and v2 needs to be swapped for sorting, but mapping is stil rigid
        return sort ? -1 : 1;
    }
    // mapping[v1] >= 0
    if (v2 < 0 || mapping[v2] < 0)
        // Second vertex is unmapped => no need to swap
        return 1;

    // mapping[v1] >= 0 && mapping[v2] >= 0
    int m1 = mapping[v1];
    int m2 = mapping[v2];
    // Check that ordering is preserved
    return (m1 < m2) == (v1 < v2) ? 1 : -1;
}

int MoleculeCisTrans::applyMapping(int parity, const int* substituents, const int* mapping, bool sort)
{
    int p1 = _getPairParity(substituents[0], substituents[1], mapping, sort);
    int p2 = _getPairParity(substituents[2], substituents[3], mapping, sort);
    if (p1 == 0 || p2 == 0)
        return 0;

    if (p1 * p2 > 0)
        return parity;

    return (parity == CIS) ? TRANS : CIS;
}

int MoleculeCisTrans::getMappingParitySign(BaseMolecule& query, BaseMolecule& target, int bond_idx, const int* mapping)
{
    int query_parity = query.cis_trans.getParity(bond_idx);

    int target_edge_idx = Graph::findMappedEdge(query, target, bond_idx, mapping);
    int target_parity = target.cis_trans.getParity(target_edge_idx);

    if (target_parity == 0)
    {
        if (query_parity != 0)
        {
            return -2; // Mapping is not valid
        }
        else
        {
            return 0;
        }
    }

    const int* query_subst = query.cis_trans.getSubstituents(bond_idx);
    int query_subst_mapped[4];
    for (int i = 0; i < 4; i++)
        query_subst_mapped[i] = query_subst[i] >= 0 ? mapping[query_subst[i]] : -1;

    int config_parity = 0;
    int v1;
    if (query_subst_mapped[0] >= 0)
        v1 = query_subst_mapped[0];
    else
    {
        v1 = query_subst_mapped[1];
        config_parity++;
    }
    if (v1 < 0)
        return 0;

    int v2;
    if (query_subst_mapped[2] >= 0)
        v2 = query_subst_mapped[2];
    else
    {
        v2 = query_subst_mapped[3];
        config_parity++;
    }
    if (v2 < 0)
        return 0;

    // Check configuration of v1 and v2 in the target
    const int* target_subst = target.cis_trans.getSubstituents(target_edge_idx);
    if (v1 == target_subst[0] || v1 == target_subst[2])
        ;
    else if (v1 == target_subst[1] || v1 == target_subst[3])
        config_parity++;
    else
        throw Error("Internal error in MoleculeCisTrans::getMappingParitySign: mapping is invalid");
    if (v2 == target_subst[0] || v2 == target_subst[2])
        ;
    else if (v2 == target_subst[1] || v2 == target_subst[3])
        config_parity++;
    else
        throw Error("Internal error in MoleculeCisTrans::getMappingParitySign: mapping is invalid");

    if (query_parity == TRANS)
        config_parity++;
    if (target_parity == TRANS)
        config_parity++;

    return config_parity % 2 == 0 ? 1 : -1;
}

bool MoleculeCisTrans::checkSub(BaseMolecule& query, BaseMolecule& target, const int* mapping)
{
    int i;

    for (i = query.edgeBegin(); i != query.edgeEnd(); i = query.edgeNext(i))
    {
        if (!query.bondStereoCare(i))
            continue;

        int query_parity = query.cis_trans.getParity(i);

        if (query_parity == 0)
            throw Error("bond #%d has stereo-care flag, but is not cis-trans bond", i);

        if (getMappingParitySign(query, target, i, mapping) < 0)
            return false;
    }
    return true;
}

void MoleculeCisTrans::buildOnSubmolecule(BaseMolecule& baseMolecule, BaseMolecule& super, int* mapping)
{
    if (!super.cis_trans.exists())
        return;

    while (_bonds.size() < baseMolecule.edgeEnd())
    {
        _Bond& bond = _bonds.push();

        memset(&bond, 0, sizeof(_Bond));
    }

    int i, j;

    for (i = super.edgeBegin(); i != super.edgeEnd(); i = super.edgeNext(i))
    {
        int parity = super.cis_trans.getParity(i);
        int sub_edge_idx = Graph::findMappedEdge(super, baseMolecule, i, mapping);

        if (sub_edge_idx < 0)
            continue;

        _Bond& bond = _bonds[sub_edge_idx];
        bond.ignored = super.cis_trans.isIgnored(i);

        if (parity == 0)
        {
            bond.parity = 0;
            continue;
        }

        const int* substituents = super.cis_trans.getSubstituents(i);

        for (j = 0; j < 4; j++)
        {
            if (substituents[j] < 0 || mapping[substituents[j]] < 0)
                bond.substituents[j] = -1;
            else
                bond.substituents[j] = mapping[substituents[j]];
        }

        bond.parity = parity;
        bool parity_changed;
        if (!sortSubstituents(baseMolecule, bond.substituents, &parity_changed))
        {
            bond.parity = 0;
            continue;
        }

        if (parity_changed)
            bond.parity = 3 - bond.parity;
    }
}

bool MoleculeCisTrans::isAutomorphism(BaseMolecule& mol, const Array<int>& mapping, const Filter* edge_filter)
{
    for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
    {
        if (edge_filter && !edge_filter->valid(i))
            continue;

        const Edge& edge = mol.getEdge(i);
        int parity = mol.cis_trans.getParity(i);
        int parity2 = MoleculeCisTrans::applyMapping(parity, mol.cis_trans.getSubstituents(i), mapping.ptr(), false);

        int i2 = mol.findEdgeIndex(mapping[edge.beg], mapping[edge.end]);
        if (mol.cis_trans.getParity(i2) != parity2)
            return false;
    }

    return true;
}

int MoleculeCisTrans::applyMapping(int idx, const int* mapping, bool sort) const
{
    return applyMapping(getParity(idx), getSubstituents(idx), mapping, sort);
}

void MoleculeCisTrans::flipBond(BaseMolecule& baseMolecule, int atom_parent, int atom_from, int atom_to)
{
    int parent_edge_index = baseMolecule.findEdgeIndex(atom_parent, atom_from);
    if (parent_edge_index == -1)
        // || getParity(parent_edge_index) != 0)
        // Such call wasn't expected and wasn't implemented
        throw Error("bond flipping attempt for nonexisting bond. ");
    //         "Such functionality isn't implemented yet.");

    const Vertex& parent_vertex = baseMolecule.getVertex(atom_parent);
    for (int i = parent_vertex.neiBegin(); i != parent_vertex.neiEnd(); i = parent_vertex.neiNext(i))
    {
        int edge = parent_vertex.neiEdge(i);
        if (getParity(edge) == 0)
            continue;

        _Bond& bond = _bonds[edge];

        for (int i = 0; i < 4; i++)
            if (bond.substituents[i] == atom_from)
            {
                bond.substituents[i] = atom_to;
                break;
            }
    }

    const Vertex& from_vertex = baseMolecule.getVertex(atom_from);
    for (int i = from_vertex.neiBegin(); i != from_vertex.neiEnd(); i = from_vertex.neiNext(i))
    {
        int edge = from_vertex.neiEdge(i);
        if (getParity(edge) == 0)
            continue;

        _Bond& bond = _bonds[edge];

        for (int i = 0; i < 4; i++)
            if (bond.substituents[i] == atom_parent)
            {
                bond.substituents[i] = atom_to;
                break;
            }
    }

    const Vertex& to_vertex = baseMolecule.getVertex(atom_to);
    for (int i = to_vertex.neiBegin(); i != to_vertex.neiEnd(); i = to_vertex.neiNext(i))
    {
        int edge = to_vertex.neiEdge(i);
        if (getParity(edge) == 0)
            continue;

        _Bond& bond = _bonds[edge];

        int edge_beg = baseMolecule.getEdge(edge).beg;
        if (atom_to == edge_beg)
        {
            if (bond.substituents[1] != -1)
                throw Error("Cannot flip bond if all substituents are present");
            bond.substituents[1] = atom_parent;
        }
        else
        {
            if (bond.substituents[3] != -1)
                throw Error("Cannot flip bond if all substituents are present");
            bond.substituents[3] = atom_parent;
        }
    }
}

int MoleculeCisTrans::count()
{
    int i, res = 0;

    for (i = 0; i < _bonds.size(); i++)
        if (_bonds[i].parity != 0)
            res++;

    return res;
}

bool MoleculeCisTrans::isRingTransBond(BaseMolecule& baseMolecule, int i)
{
    const int* subst = getSubstituents(i);
    int parity = getParity(i); // 1(CIS) or 2(TRANS)
    const Edge& edge = baseMolecule.getEdge(i);

    if (baseMolecule.getBondTopology(i) != TOPOLOGY_RING)
        throw Error("is RingTransBond(): not a ring bond given");

    if (baseMolecule.getBondTopology(baseMolecule.findEdgeIndex(edge.beg, subst[0])) != TOPOLOGY_RING)
    {
        if (baseMolecule.getBondTopology(baseMolecule.findEdgeIndex(edge.beg, subst[1])) != TOPOLOGY_RING)
            throw Error("unexpected: have not found ring substutient");
        // invert parity
        parity = 3 - parity;
    }
    if (baseMolecule.getBondTopology(baseMolecule.findEdgeIndex(edge.end, subst[2])) != TOPOLOGY_RING)
    {
        if (baseMolecule.getBondTopology(baseMolecule.findEdgeIndex(edge.end, subst[3])) != TOPOLOGY_RING)
            throw Error("unexpected: have not found ring substutient");
        // invert parity
        parity = 3 - parity;
    }
    return (parity == MoleculeCisTrans::TRANS);
}

bool MoleculeCisTrans::sameside(int edge_idx, int v1, int v2)
{
    int parity = getParity(edge_idx);
    if (parity == 0)
        throw Error("Bond %d is not a cis-trans bond", edge_idx);

    const int* subst = getSubstituents(edge_idx);
    int v[2] = {v1, v2};
    int v_pos[2] = {-1};
    for (int j = 0; j < 2; j++)
    {
        for (int i = 0; i < 4; i++)
            if (subst[i] == v[j])
            {
                v_pos[j] = i;
                break;
            }
        if (v_pos[j] == -1)
            throw Error("Vertex %d has not been found near bond %d", v[j], edge_idx);
    }

    bool same_parity = (v_pos[0] % 2) == (v_pos[1] % 2);
    if (parity == TRANS)
        same_parity = !same_parity;
    return same_parity;
}
