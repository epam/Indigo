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

#include "molecule/molecule_allene_stereo.h"

#include "molecule/base_molecule.h"
#include "molecule/cmf_loader.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"

using namespace indigo;

IMPL_ERROR(MoleculeAlleneStereo, "allene stereo");

MoleculeAlleneStereo::MoleculeAlleneStereo()
{
}

int MoleculeAlleneStereo::sameside(const Vec3f& dir1, const Vec3f& dir2, const Vec3f& sep)
{
    Vec3f norm, norm_cross;

    // Use double cross product for getting vector lying the same plane with dir1 and sep
    norm_cross.cross(sep, dir1);
    norm.cross(norm_cross, sep);

    if (!norm.normalize())
        throw Error("internal: zero vector length");

    float prod1 = Vec3f::dot(dir1, norm);
    float prod2 = Vec3f::dot(dir2, norm);

    if ((float)(fabs(prod1)) < 1e-3 || (float)(fabs(prod2)) < 1e-3)
        return 0;

    return (prod1 * prod2 > 0) ? 1 : -1;
}

bool MoleculeAlleneStereo::possibleCenter(BaseMolecule& mol, int idx, int& left, int& right, int subst[4], bool pure_h[4])
{
    const Vertex& vertex = mol.getVertex(idx);

    // Check that we have [C,Si]=[C,Si]=[C,Si] fragment with the middle "C" being the i-th atom.
    if (vertex.degree() != 2)
        return false;

    if (mol.getAtomNumber(idx) != ELEM_C && mol.getAtomNumber(idx) != ELEM_Si)
        return false;

    int j = vertex.neiBegin();
    int left_edge = vertex.neiEdge(j);
    int right_edge = vertex.neiEdge(vertex.neiNext(j));

    left = vertex.neiVertex(j);
    right = vertex.neiVertex(vertex.neiNext(j));

    if (mol.getBondOrder(left_edge) != BOND_DOUBLE || mol.getBondOrder(right_edge) != BOND_DOUBLE)
        return false;

    if (mol.getAtomNumber(left) != ELEM_C && mol.getAtomNumber(left) != ELEM_Si)
        return false;

    if (mol.getAtomNumber(right) != ELEM_C && mol.getAtomNumber(right) != ELEM_Si)
        return false;

    // Also check that left and right "C" atoms have one or two single bonds
    const Vertex& v_left = mol.getVertex(left);
    const Vertex& v_right = mol.getVertex(right);

    if (v_left.degree() < 2 || v_left.degree() > 3)
        return false;

    if (v_right.degree() < 2 || v_right.degree() > 3)
        return false;

    int k;

    pure_h[0] = pure_h[1] = pure_h[2] = pure_h[3] = true; // explicit pure H or implicit H

    subst[0] = subst[1] = subst[2] = subst[3] = -1;

    for (k = 0, j = v_left.neiBegin(); j != v_left.neiEnd(); j = v_left.neiNext(j))
    {
        if (v_left.neiVertex(j) == idx)
            continue;
        if (mol.getBondOrder(v_left.neiEdge(j)) != BOND_SINGLE)
            return false;
        subst[k] = v_left.neiVertex(j);
        if (mol.getAtomNumber(subst[k]) != ELEM_H || !mol.possibleAtomIsotope(subst[k], 0))
            pure_h[k] = false;
        k++;
    }

    for (k = 2, j = v_right.neiBegin(); j != v_right.neiEnd(); j = v_right.neiNext(j))
    {
        if (v_right.neiVertex(j) == idx)
            continue;
        if (mol.getBondOrder(v_right.neiEdge(j)) != BOND_SINGLE)
            return false;
        subst[k] = v_right.neiVertex(j);
        if (mol.getAtomNumber(subst[k]) != ELEM_H || !mol.possibleAtomIsotope(subst[k], 0))
            pure_h[k] = false;
        k++;
    }

    // no non-H substituents => no symmetry
    if (pure_h[0] && pure_h[1])
        return false;

    if (pure_h[2] && pure_h[3])
        return false;

    return true;
}

bool MoleculeAlleneStereo::_isAlleneCenter(BaseMolecule& mol, int idx, _Atom& atom, int* sensible_bonds_out)
{
    bool pure_h[4];

    if (!possibleCenter(mol, idx, atom.left, atom.right, atom.subst, pure_h))
        return false;

    int dirs[4] = {0, 0, 0, 0};
    Vec3f subst_vecs[4];
    int j, k;

    bool zero_bond_length = false;

    for (k = 0; k < 2; k++)
        if (atom.subst[k] >= 0)
        {
            dirs[k] = mol.getBondDirection2(atom.left, atom.subst[k]);
            subst_vecs[k].diff(mol.getAtomXyz(atom.subst[k]), mol.getAtomXyz(atom.left));
            if (!subst_vecs[k].normalize())
                zero_bond_length = true;
        }

    for (k = 2; k < 4; k++)
        if (atom.subst[k] >= 0)
        {
            dirs[k] = mol.getBondDirection2(atom.right, atom.subst[k]);
            subst_vecs[k].diff(mol.getAtomXyz(atom.subst[k]), mol.getAtomXyz(atom.right));
            if (!subst_vecs[k].normalize())
                zero_bond_length = true;
        }

    if (dirs[0] == 0 && dirs[1] == 0 && dirs[2] == 0 && dirs[3] == 0)
        return false; // no oriented bonds => no stereochemistry

    // check that they do not have the same orientation
    if (dirs[0] != 0 && dirs[0] == dirs[1] && dirs[0] != BOND_EITHER)
        return false;
    if (dirs[2] != 0 && dirs[2] == dirs[3] && dirs[2] != BOND_EITHER)
        return false;

    if (zero_bond_length)
        throw Error("zero bond length");

    Vec3f pos_center = mol.getAtomXyz(idx);
    Vec3f vec_left = mol.getAtomXyz(atom.left);
    Vec3f vec_right = mol.getAtomXyz(atom.right);

    vec_left.sub(pos_center);
    vec_right.sub(pos_center);

    if (!vec_left.normalize() || !vec_right.normalize())
        throw Error("zero bond length");

    // they should go in one line
    // 0.04 is equivalent to 16 degress because it is hard to draw a straight line accurately
    if (fabs(Vec3f::dot(vec_left, vec_right) + 1) > 0.04)
        return false;

    // check that if there are two left substituents, they do not lie on the same side
    if (atom.subst[1] != -1 && sameside(subst_vecs[0], subst_vecs[1], vec_left) != -1)
        return false;
    // the same check for the two right substituents
    if (atom.subst[3] != -1 && sameside(subst_vecs[2], subst_vecs[3], vec_right) != -1)
        return false;

    if (dirs[0] == BOND_EITHER || dirs[1] == BOND_EITHER || dirs[2] == BOND_EITHER || dirs[3] == BOND_EITHER)
        atom.parity = 3;
    else
    {
        if (dirs[0] == 0 && dirs[1] != 0)
            dirs[0] = 3 - dirs[1];
        if (dirs[2] == 0 && dirs[3] != 0)
            dirs[2] = 3 - dirs[3];

        int ss = sameside(subst_vecs[0], subst_vecs[2], vec_right);

        if (ss == 0)
            return false;

        if (dirs[0] == 0)
            dirs[0] = (ss == 1) ? 3 - dirs[2] : dirs[2];
        else if (dirs[2] == 0)
            dirs[2] = (ss == 1) ? 3 - dirs[0] : dirs[0];

        if ((ss == 1 && dirs[0] == dirs[2]) || (ss == -1 && dirs[0] != dirs[2]))
            return false; // square-planar configuration?

        if ((ss == 1 && dirs[0] == BOND_UP) || (ss == -1 && dirs[0] == BOND_DOWN))
            atom.parity = 1;
        else
            atom.parity = 2;
    }

    const Vertex& v_left = mol.getVertex(atom.left);
    const Vertex& v_right = mol.getVertex(atom.right);

    // mark bonds as sensible
    for (k = 0, j = v_left.neiBegin(); j != v_left.neiEnd(); j = v_left.neiNext(j))
    {
        int dir = mol.getBondDirection2(atom.left, v_left.neiVertex(j));
        if (dir != 0)
            sensible_bonds_out[v_left.neiEdge(j)] = 1;
    }
    for (k = 0, j = v_right.neiBegin(); j != v_right.neiEnd(); j = v_right.neiNext(j))
    {
        int dir = mol.getBondDirection2(atom.right, v_right.neiVertex(j));
        if (dir != 0)
            sensible_bonds_out[v_right.neiEdge(j)] = 1;
    }

    // "either" allene centers do not count
    if (atom.parity == 3)
        return false;

    Vec3f prod;

    prod.cross(vec_left, subst_vecs[0]);

    if (prod.z > 0)
        atom.parity = 3 - atom.parity;

    // move hydrogens from [0] and [2] to [1] and [3] respectively
    if (pure_h[0])
    {
        std::swap(atom.subst[0], atom.subst[1]);
        atom.parity = 3 - atom.parity;
    }
    if (pure_h[2])
    {
        std::swap(atom.subst[2], atom.subst[3]);
        atom.parity = 3 - atom.parity;
    }

    return true;
}

void MoleculeAlleneStereo::buildFromBonds(BaseMolecule& baseMolecule, bool ignore_errors, int* sensible_bonds_out)
{
    int i;

    for (i = baseMolecule.vertexBegin(); i != baseMolecule.vertexEnd(); i = baseMolecule.vertexNext(i))
    {
        _Atom atom;

        try
        {
            if (!_isAlleneCenter(baseMolecule, i, atom, sensible_bonds_out))
                continue;
        }
        catch (Error& err)
        {
            if (!ignore_errors)
                throw err;
        }
        _centers.insert(i, atom);
    }
}

void MoleculeAlleneStereo::clear()
{
    _centers.clear();
}

bool MoleculeAlleneStereo::isCenter(int atom_idx)
{
    return _centers.at2(atom_idx) != 0;
}

void MoleculeAlleneStereo::invert(int atom_idx)
{
    _Atom& atom = _centers.at(atom_idx);
    atom.parity = 3 - atom.parity;
}

void MoleculeAlleneStereo::reset(int atom_idx)
{
    _centers.remove(atom_idx);
}

int MoleculeAlleneStereo::size()
{
    return _centers.size();
}

bool MoleculeAlleneStereo::checkSub(BaseMolecule& query, BaseMolecule& target, const int* mapping)
{
    int i;

    for (i = query.vertexBegin(); i != query.vertexEnd(); i = query.vertexNext(i))
    {
        const _Atom* qa = query.allene_stereo._centers.at2(i);

        if (qa == 0)
            continue;

        const _Atom* ta = target.allene_stereo._centers.at2(mapping[i]);

        if (ta == 0)
            return false;

        int parity = qa->parity;
        int qs[4], ts[4];

        memcpy(qs, qa->subst, 4 * sizeof(int));
        memcpy(ts, ta->subst, 4 * sizeof(int));

        if (mapping[qs[0]] == ts[2] || mapping[qs[0]] == ts[3])
        {
            std::swap(qs[0], qs[2]);
            std::swap(qs[1], qs[3]);
        }

        if (mapping[qs[0]] == ts[0])
            ;
        else if (mapping[qs[0]] == ts[1])
            parity = 3 - parity;
        else
            throw Error("checkSub() subst[0] not mapped");

        if (mapping[qs[2]] == ts[2])
            ;
        else if (mapping[qs[2]] == ts[3])
            parity = 3 - parity;
        else
            throw Error("checkSub() subst[2] not mapped");

        if (parity != ta->parity)
            return false;
    }
    return true;
}

void MoleculeAlleneStereo::buildOnSubmolecule(BaseMolecule& baseMolecule, BaseMolecule& super, int* mapping)
{
    int i, j;

    for (i = super.allene_stereo._centers.begin(); i != super.allene_stereo._centers.end(); i = super.allene_stereo._centers.next(i))
    {
        int super_idx = super.allene_stereo._centers.key(i);
        const _Atom& super_center = super.allene_stereo._centers.value(i);
        int sub_idx = mapping[super_idx];

        if (sub_idx < 0)
            continue;

        _Atom new_center;

        new_center.left = mapping[super_center.left];
        new_center.right = mapping[super_center.right];
        new_center.parity = super_center.parity;

        if (new_center.left < 0 || new_center.right < 0)
            continue;

        for (j = 0; j < 4; j++)
        {
            if (super_center.subst[j] >= 0)
                new_center.subst[j] = mapping[super_center.subst[j]];
            else
                new_center.subst[j] = -1;
        }

        if (new_center.subst[0] == -1 && new_center.subst[1] == -1)
            continue;

        if (new_center.subst[2] == -1 && new_center.subst[3] == -1)
            continue;

        if (baseMolecule.getAtomNumber(new_center.subst[0]) == ELEM_H && baseMolecule.possibleAtomIsotope(new_center.subst[0], 0))
        {
            std::swap(new_center.subst[0], new_center.subst[1]);
            new_center.parity = 3 - new_center.parity;
        }

        if (baseMolecule.getAtomNumber(new_center.subst[2]) == ELEM_H && baseMolecule.possibleAtomIsotope(new_center.subst[2], 0))
        {
            std::swap(new_center.subst[2], new_center.subst[3]);
            new_center.parity = 3 - new_center.parity;
        }

        if (new_center.subst[0] == -1)
        {
            new_center.subst[0] = new_center.subst[1];
            new_center.subst[1] = -1;
            new_center.parity = 3 - new_center.parity;
        }

        if (new_center.subst[2] == -1)
        {
            new_center.subst[2] = new_center.subst[3];
            new_center.subst[3] = -1;
            new_center.parity = 3 - new_center.parity;
        }

        _centers.insert(sub_idx, new_center);

        const Vertex& super_left = super.getVertex(super_center.left);
        const Vertex& super_right = super.getVertex(super_center.right);

        for (j = super_left.neiBegin(); j != super_left.neiEnd(); j = super_left.neiNext(j))
        {
            int super_edge = super_left.neiEdge(j);
            if (mapping[super_left.neiVertex(j)] == -1)
                continue;

            int dir = super.getBondDirection(super_edge);
            if (dir != 0)
                baseMolecule.setBondDirection(baseMolecule.findEdgeIndex(new_center.left, mapping[super_left.neiVertex(j)]), dir);
        }

        for (j = super_right.neiBegin(); j != super_right.neiEnd(); j = super_right.neiNext(j))
        {
            int super_edge = super_right.neiEdge(j);
            if (mapping[super_right.neiVertex(j)] == -1)
                continue;

            int dir = super.getBondDirection(super_edge);
            if (dir != 0)
                baseMolecule.setBondDirection(baseMolecule.findEdgeIndex(new_center.right, mapping[super_right.neiVertex(j)]), dir);
        }
    }
}

int MoleculeAlleneStereo::begin() const
{
    return _centers.begin();
}

int MoleculeAlleneStereo::end() const
{
    return _centers.end();
}

int MoleculeAlleneStereo::next(int i) const
{
    return _centers.next(i);
}

void MoleculeAlleneStereo::get(int i, int& atom_idx, int& left, int& right, int subst[4], int& parity)
{
    _Atom& atom = _centers.value(i);

    atom_idx = _centers.key(i);
    left = atom.left;
    right = atom.right;
    parity = atom.parity;
    memcpy(subst, atom.subst, sizeof(int) * 4);
}

void MoleculeAlleneStereo::getByAtomIdx(int atom_idx, int& left, int& right, int subst[4], int& parity)
{
    _Atom& atom = _centers.at(atom_idx);

    left = atom.left;
    right = atom.right;
    parity = atom.parity;
    memcpy(subst, atom.subst, sizeof(int) * 4);
}

void MoleculeAlleneStereo::add(int atom_idx, int left, int right, int subst[4], int parity)
{
    _Atom atom;

    atom.left = left;
    atom.right = right;
    memcpy(atom.subst, subst, 4 * sizeof(int));
    atom.parity = parity;

    _centers.insert(atom_idx, atom);
}

void MoleculeAlleneStereo::markBonds(BaseMolecule& baseMolecule)
{
    int i, j;
    for (i = _centers.begin(); i != _centers.end(); i = _centers.next(i))
    {
        int idx = _centers.key(i);
        _Atom& atom = _centers.value(i);

        Vec3f subst_vecs[4];
        int k;

        for (k = 0; k < 2; k++)
            if (atom.subst[k] >= 0)
            {
                subst_vecs[k].diff(baseMolecule.getAtomXyz(atom.subst[k]), baseMolecule.getAtomXyz(atom.left));
                if (!subst_vecs[k].normalize())
                    throw Error("zero bond length");
            }

        for (k = 2; k < 4; k++)
            if (atom.subst[k] >= 0)
            {
                subst_vecs[k].diff(baseMolecule.getAtomXyz(atom.subst[k]), baseMolecule.getAtomXyz(atom.right));
                if (!subst_vecs[k].normalize())
                    throw Error("zero bond length");
            }

        Vec3f pos_center = baseMolecule.getAtomXyz(idx);
        Vec3f vec_left = baseMolecule.getAtomXyz(atom.left);
        Vec3f vec_right = baseMolecule.getAtomXyz(atom.right);

        vec_left.sub(pos_center);
        vec_right.sub(pos_center);

        if (!vec_left.normalize() || !vec_right.normalize())
            throw Error("zero bond length");

        // they should go in one line
        if (fabs(Vec3f::dot(vec_left, vec_right) + 1) > 0.001)
            continue; // throw Error("markBonds(): double bonds do not form a flat angle")

        if (atom.subst[1] != -1 && sameside(subst_vecs[0], subst_vecs[1], vec_left) != -1)
            continue; // throw Error("markBonds(): same-side substituents on the left")
        if (atom.subst[3] != -1 && sameside(subst_vecs[2], subst_vecs[3], vec_right) != -1)
            continue; // throw Error("markBonds(): same-side substituents on the right")

        int ss = sameside(subst_vecs[0], subst_vecs[2], vec_right);

        if (ss == 0)
            continue; // throw Error("markBonds(): same-side substituents")

        bool to_mark1[4] = {false, false, false, false};
        int n_to_mark1 = 0;
        bool to_mark2[4] = {false, false, false, false};
        int n_to_mark2 = 0;
        bool to_mark3[4] = {false, false, false, false};
        int n_to_mark3 = 0;

        for (j = 0; j < 4; j++)
        {
            if (atom.subst[j] < 0)
                continue;

            int edge_idx = baseMolecule.findEdgeIndex(atom.subst[j], j < 2 ? atom.left : atom.right);
            if (edge_idx < 0)
                throw Error("markBonds(): internal: edge not found");

            if (baseMolecule.getBondDirection(edge_idx) != 0)
                continue;

            if (baseMolecule.getVertex(atom.subst[j]).degree() == 1)
            {
                to_mark1[j] = 1;
                n_to_mark1++;
            }
            if (baseMolecule.getEdgeTopology(edge_idx) != TOPOLOGY_RING)
            {
                to_mark2[j] = 1;
                n_to_mark2++;
            }
            to_mark3[j] = 1;
            n_to_mark3++;
        }

        bool* to_mark;

        if (n_to_mark1 > 0)
            to_mark = to_mark1;
        else if (n_to_mark2 > 0)
            to_mark = to_mark2;
        else if (n_to_mark3 > 0)
            to_mark = to_mark3;
        else
            throw Error("no bond can be marked");

        if (to_mark[0] && to_mark[1])
            to_mark[2] = to_mark[3] = false;
        if (to_mark[2] && to_mark[3])
            to_mark[0] = to_mark[1] = false;

        int dirs[4];

        if (atom.parity == 2)
        {
            dirs[0] = BOND_DOWN;
            dirs[1] = BOND_UP;
        }
        else
        {
            dirs[0] = BOND_UP;
            dirs[1] = BOND_DOWN;
        }

        if (ss == -1)
        {
            dirs[0] = 3 - dirs[0];
            dirs[1] = 3 - dirs[1];
        }

        Vec3f prod;

        prod.cross(vec_left, subst_vecs[0]);

        if (prod.z > 0)
        {
            dirs[0] = 3 - dirs[0];
            dirs[1] = 3 - dirs[1];
        }

        if (ss == 1)
        {
            dirs[2] = 3 - dirs[0];
            dirs[3] = 3 - dirs[1];
        }
        else
        {
            dirs[2] = dirs[0];
            dirs[3] = dirs[1];
        }

        for (j = 0; j < 4; j++)
        {
            if (to_mark[j])
            {
                int edge_idx = baseMolecule.findEdgeIndex(atom.subst[j], j < 2 ? atom.left : atom.right);

                if (baseMolecule.getEdge(edge_idx).beg != (j < 2 ? atom.left : atom.right))
                    baseMolecule.swapEdgeEnds(edge_idx);

                baseMolecule.setBondDirection(edge_idx, dirs[j]);
            }
        }
    }
}

void MoleculeAlleneStereo::removeAtoms(BaseMolecule& baseMolecule, const Array<int>& indices)
{
    int i, j;

    QS_DEF(Array<int>, centers_to_remove);
    centers_to_remove.clear();

    for (i = 0; i < indices.size(); i++)
    {
        int idx = indices[i];
        if (_centers.find(idx))
        {
            centers_to_remove.push(idx);
            continue;
        }

        // TODO: this can be done without looping through all centers
        for (j = _centers.begin(); j != _centers.end(); j = _centers.next(j))
        {
            int center_idx = _centers.key(j);
            _Atom& atom = _centers.value(j);

            if (idx == atom.left || idx == atom.right)
            {
                centers_to_remove.push(center_idx);
                continue;
            }

            if (idx == atom.subst[1])
                atom.subst[1] = -1;
            else if (idx == atom.subst[3])
                atom.subst[3] = -1;
            else if (idx == atom.subst[0])
            {
                if (atom.subst[1] == -1 || (baseMolecule.getAtomNumber(atom.subst[1]) == ELEM_H && baseMolecule.possibleAtomIsotope(atom.subst[1], 0)))
                {
                    centers_to_remove.push(center_idx);
                    continue;
                }
                atom.subst[0] = atom.subst[1];
                atom.parity = 3 - atom.parity;
            }
            else if (idx == atom.subst[2])
            {
                if (atom.subst[3] == -1 || (baseMolecule.getAtomNumber(atom.subst[3]) == ELEM_H && baseMolecule.possibleAtomIsotope(atom.subst[3], 0)))
                {
                    centers_to_remove.push(center_idx);
                    continue;
                }
                atom.subst[2] = atom.subst[3];
                atom.parity = 3 - atom.parity;
            }
        }
    }

    for (int i = 0; i < centers_to_remove.size(); i++)
    {
        int idx = centers_to_remove[i];
        if (_centers.find(idx))
            _centers.remove(idx);
    }
}

void MoleculeAlleneStereo::removeBonds(BaseMolecule& baseMolecule, const Array<int>& indices)
{
    int i, j;

    for (i = 0; i < indices.size(); i++)
    {
        int idx = indices[i];

        // TODO: this can be done without looping through all centers
        for (j = _centers.begin(); j != _centers.end(); j = _centers.next(j))
        {
            int center_idx = _centers.key(j);
            _Atom& atom = _centers.value(j);

            if (idx == baseMolecule.findEdgeIndex(center_idx, atom.left) || idx == baseMolecule.findEdgeIndex(center_idx, atom.right))
            {
                _centers.remove(center_idx);
                continue;
            }

            if (idx == baseMolecule.findEdgeIndex(atom.left, atom.subst[1]))
                atom.subst[1] = -1;
            else if (idx == baseMolecule.findEdgeIndex(atom.right, atom.subst[3]))
                atom.subst[3] = -1;
            else if (idx == baseMolecule.findEdgeIndex(atom.left, atom.subst[0]))
            {
                if (atom.subst[1] == -1 || (baseMolecule.getAtomNumber(atom.subst[1]) == ELEM_H && baseMolecule.possibleAtomIsotope(atom.subst[1], 0)))
                {
                    _centers.remove(center_idx);
                    continue;
                }
                atom.subst[0] = atom.subst[1];
                atom.parity = 3 - atom.parity;
            }
            else if (idx == baseMolecule.findEdgeIndex(atom.right, atom.subst[2]))
            {
                if (atom.subst[3] == -1 || (baseMolecule.getAtomNumber(atom.subst[3]) == ELEM_H && baseMolecule.possibleAtomIsotope(atom.subst[3], 0)))
                {
                    _centers.remove(center_idx);
                    continue;
                }
                atom.subst[2] = atom.subst[3];
                atom.parity = 3 - atom.parity;
            }
        }
    }
}

void MoleculeAlleneStereo::registerUnfoldedHydrogen(int atom_idx, int added_hydrogen)
{
    int j;

    // TODO: this can be done without looping through all centers
    for (j = _centers.begin(); j != _centers.end(); j = _centers.next(j))
    {
        _Atom& atom = _centers.value(j);

        if (atom_idx == atom.left && atom.subst[1] == -1)
            atom.subst[1] = added_hydrogen;
        else if (atom_idx == atom.right && atom.subst[3] == -1)
            atom.subst[3] = added_hydrogen;
    }
}
