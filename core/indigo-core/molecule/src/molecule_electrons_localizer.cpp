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

#include "molecule/molecule_electrons_localizer.h"

#include "base_cpp/obj_array.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"

using namespace indigo;

enum
{
    BOND_SINGLE_OR_DOUBLE = -100
};

IMPL_ERROR(MoleculeElectronsLocalizer, "Electron localizer");

CP_DEF(MoleculeElectronsLocalizer);

MoleculeElectronsLocalizer::MoleculeElectronsLocalizer(Molecule& skeleton)
    : _skeleton(skeleton), CP_INIT, TL_CP_GET(_extended_skeleton), TL_CP_GET(_edge_mapping), TL_CP_GET(_atom_info), TL_CP_GET(_edges_fixed_type)
{
    _edges_fixed_type.clear_resize(_skeleton.edgeEnd());
    _edges_fixed_type.fffill();

    _atom_info.clear_resize(_skeleton.vertexEnd());
    _extended_skeleton.clear();
    _edge_mapping.clear();

    _constrained_primary_double_bonds_conn = 0;
    _constrained_secondary_double_bonds_conn = 0;
    _constrained_primary_lonepairs = 0;
    _constrained_secondary_lonepairs = 0;
    _constrained_primary_atoms = 0;
    _constrained_secondary_atoms = 0;
    _constrained_saturated_atoms = 0;

    _construct();
}

void MoleculeElectronsLocalizer::_construct()
{
    QS_DEF(Array<int>, atom_mapping);
    _extended_skeleton.cloneGraph(_skeleton, &atom_mapping);

    // Find edge mapping between source skeleton and extended skeleton
    _edge_mapping.resize(_skeleton.edgeEnd());
    for (int e = _skeleton.edgeBegin(); e != _skeleton.edgeEnd(); e = _skeleton.edgeNext(e))
    {
        _edge_mapping[e] = Graph::findMappedEdge(_extended_skeleton, _skeleton, e, atom_mapping.ptr());
    }

    // Setup default
    for (int v = _skeleton.vertexBegin(); v != _skeleton.vertexEnd(); v = _skeleton.vertexNext(v))
    {
        _AtomInfo& info = _atom_info[v];
        info.atom_node = atom_mapping[v];
        info.atom_saturated = false;
        info.atom_fixed = false;
        info.atom_connectivity_fixed = false;
        info.fixed_connectivity = -1;
        info.fixed_lonepairs = -1;
        info.skeleton_connectivity = -1;

        // Create and attach orbital node
        info.orbitals_node = _extended_skeleton.addVertex();
        info.orbitals_edge = _extended_skeleton.addEdge(info.atom_node, info.orbitals_node);
    }

    _constructBMatchingFinder();
    _setupAtomProperties();
    _setupBMatchingNodes();
    _setupBMatchingEdges();

    _zc_atoms_connectivity = 0;
    _zc_lonepairs = 0;

    for (int v = _skeleton.vertexBegin(); v != _skeleton.vertexEnd(); v = _skeleton.vertexNext(v))
    {
        _AtomInfo& info = _atom_info[v];
        _zc_atoms_connectivity += std::max(info.zc_connectivity, 0);
        _zc_lonepairs += info.zc_lonepairs;

        // Store initial maximum add connectivity
        info.max_add_connectivity0 = info.max_add_connectivity;
    }

    // Saturate atom that can't be unsaturated
    for (int v = _skeleton.vertexBegin(); v != _skeleton.vertexEnd(); v = _skeleton.vertexNext(v))
    {
        if (!_canAtomBeUnsaturated(v))
            _fixAtomSaturated(v);
    }
}

void MoleculeElectronsLocalizer::_constructBMatchingFinder()
{
    QS_DEF(ObjArray<Array<int>>, nodes_per_set);
    nodes_per_set.clear();
    nodes_per_set.resize(_SET_MAX);

    for (int v = _skeleton.vertexBegin(); v != _skeleton.vertexEnd(); v = _skeleton.vertexNext(v))
    {
        _AtomInfo& info = _atom_info[v];

        nodes_per_set[_PRIMARY_ATOMS_SET].push(info.atom_node);
        nodes_per_set[_SECONDARY_ATOMS_SET].push(info.atom_node);
        nodes_per_set[_CONSTRAINED_ATOMS_SET].push(info.atom_node);

        nodes_per_set[_PRIMARY_LONEPAIRS_SET].push(info.orbitals_node);
        nodes_per_set[_SECONDARY_LONEPAIRS_SET].push(info.orbitals_node);
        nodes_per_set[_CONSTRAINED_LONEPAIRS_SET].push(info.orbitals_node);
    }

    QS_DEF(Array<int>, set_per_set);
    set_per_set.clear_resize(_SET_MAX);
    set_per_set.fffill();
    set_per_set[_PRIMARY_ATOMS_SET] = _SUM_ATOMS_SET;
    set_per_set[_SECONDARY_ATOMS_SET] = _SUM_ATOMS_SET;
    set_per_set[_PRIMARY_LONEPAIRS_SET] = _SUM_LONEPAIRS_SET;
    set_per_set[_SECONDARY_LONEPAIRS_SET] = _SUM_LONEPAIRS_SET;

    _finder.create(_extended_skeleton, nodes_per_set, &set_per_set);
}

void MoleculeElectronsLocalizer::_setupAtomProperties()
{
    for (int v = _skeleton.vertexBegin(); v != _skeleton.vertexEnd(); v = _skeleton.vertexNext(v))
    {
        _AtomInfo& info = _atom_info[v];

        int degree = _skeleton.getVertex(v).degree();
        info.skeleton_connectivity = _skeleton.getImplicitH(v) + degree;

        info.max_add_connectivity = 4 - info.skeleton_connectivity - Element::radicalOrbitals(_skeleton.getAtomRadical(v));

        if (info.max_add_connectivity < 0)
            throw Error("Atoms with d-obitals used are not supported");

        bool ret = _calcConnectivityAndLoneparis(v, 0, &info.zc_connectivity, &info.zc_lonepairs);
        if (!ret)
            throw Error("Invalid atom");

        info.zc_connectivity -= info.skeleton_connectivity;
    }
}

bool MoleculeElectronsLocalizer::_calcConnectivityAndLoneparis(int atom, int charge, int* connectivity, int* lonepairs)
{
    int label = _skeleton.getAtomNumber(atom);
    int radical = _skeleton.getAtomRadical(atom);

    int electrons = Element::electrons(label, 0) - Element::radicalElectrons(radical) - charge;
    int orbitals = 4 - Element::radicalOrbitals(radical);

    if (electrons < 0 || electrons > 2 * orbitals)
        return false;

    if (electrons > orbitals)
    {
        *lonepairs = electrons - orbitals;
        *connectivity = electrons - 2 * (*lonepairs);
    }
    else
    {
        *connectivity = electrons;
        *lonepairs = 0;
    }
    return true;
}

void MoleculeElectronsLocalizer::_setupBMatchingNodeAtom(int atom)
{
    _AtomInfo& info = _atom_info[atom];

    int zero_sum_conn = std::max(info.zc_connectivity, 0) + info.zc_lonepairs;
    _finder->setNodeCapacity(info.atom_node, zero_sum_conn, _PRIMARY_ATOMS_SET);

    int left_conn = info.max_add_connectivity - zero_sum_conn;
    if (left_conn < 0)
        left_conn = 0;
    _finder->setNodeCapacity(info.atom_node, left_conn, _SECONDARY_ATOMS_SET);
    _finder->setNodeCapacity(info.atom_node, 0, _CONSTRAINED_ATOMS_SET);
}

void MoleculeElectronsLocalizer::_setupBMatchingNodeOrbital(int atom)
{
    _AtomInfo& info = _atom_info[atom];

    _finder->setNodeCapacity(info.orbitals_node, info.zc_lonepairs, _PRIMARY_LONEPAIRS_SET);
    int left_lonepairs = info.max_add_connectivity - info.zc_lonepairs;
    if (left_lonepairs < 0)
        left_lonepairs = 0;

    _finder->setNodeCapacity(info.orbitals_node, left_lonepairs, _SECONDARY_LONEPAIRS_SET);
    _finder->setNodeCapacity(info.orbitals_node, 0, _CONSTRAINED_LONEPAIRS_SET);

    _finder->setMaxEdgeMultiplicity(info.orbitals_edge, info.max_add_connectivity);
}

void MoleculeElectronsLocalizer::_setupBMatchingNode(int atom)
{
    _setupBMatchingNodeAtom(atom);
    _setupBMatchingNodeOrbital(atom);
}

void MoleculeElectronsLocalizer::_setupBMatchingNodes()
{
    for (int v = _skeleton.vertexBegin(); v != _skeleton.vertexEnd(); v = _skeleton.vertexNext(v))
    {
        _setupBMatchingNode(v);
    }
}

void MoleculeElectronsLocalizer::_setupBMatchingEdges()
{
    for (int e = _skeleton.edgeBegin(); e != _skeleton.edgeEnd(); e = _skeleton.edgeNext(e))
    {
        const Edge& edge = _skeleton.getEdge(e);

        int beg_cap = _atom_info[edge.beg].max_add_connectivity;
        int end_cap = _atom_info[edge.end].max_add_connectivity;

        _finder->setMaxEdgeMultiplicity(e, std::min(beg_cap, end_cap));
    }
}

void MoleculeElectronsLocalizer::setParameters(int double_bonds, int primary_lonepairs, int secondary_lonepairs)
{
    _double_bonds = double_bonds;
    _primary_lonepairs = primary_lonepairs;
    _secondary_lonepairs = secondary_lonepairs;
}

bool MoleculeElectronsLocalizer::_setConstraintSetForAtoms()
{
    // Calculate and set atoms primary and secondary connectivity
    int atoms_connectivity = 2 * _double_bonds + _primary_lonepairs + _secondary_lonepairs;
    int atoms_primary_conn = atoms_connectivity;
    int atoms_secondary_conn = 0;
    int zero_conn = _zc_atoms_connectivity + _zc_lonepairs;
    if (atoms_primary_conn > zero_conn)
    {
        atoms_primary_conn = zero_conn;
        atoms_secondary_conn = atoms_connectivity - zero_conn;
    }

    int atoms_prim = atoms_primary_conn - _constrained_primary_atoms - _constrained_primary_double_bonds_conn;
    int atoms_sec = atoms_secondary_conn - _constrained_secondary_atoms - _constrained_secondary_double_bonds_conn;

    if (atoms_prim < 0 || atoms_sec < 0)
        return false;

    int atoms_sum = atoms_prim + atoms_sec - _constrained_saturated_atoms;
    if (atoms_sum < 0)
        return false;

    _finder->setNodeSetCapacity(_PRIMARY_ATOMS_SET, atoms_prim);
    _finder->setNodeSetCapacity(_SECONDARY_ATOMS_SET, atoms_sec);
    _finder->setNodeSetCapacity(_SUM_ATOMS_SET, atoms_sum);

    if (_constrained_saturated_atoms + _constrained_primary_atoms + _constrained_secondary_atoms < 0)
        _finder->setNodeSetCapacity(_CONSTRAINED_ATOMS_SET, 0);
    else
        _finder->setNodeSetCapacity(_CONSTRAINED_ATOMS_SET, _constrained_saturated_atoms + _constrained_primary_atoms + _constrained_secondary_atoms);

    return true;
}

bool MoleculeElectronsLocalizer::_setConstraintSetForLonepairs(bool only_check_possibility)
{
    // Calculate and set lonepairs primary and secondary connectivity
    int lp_prim = _primary_lonepairs - _constrained_primary_lonepairs;
    int lp_sec = _secondary_lonepairs - _constrained_secondary_lonepairs;
    if (lp_prim < 0 || lp_sec < 0)
        return false;

    int lp_sum = lp_prim + lp_sec;
    if (only_check_possibility)
        lp_sec = lp_prim = lp_sum;

    _finder->setNodeSetCapacity(_PRIMARY_LONEPAIRS_SET, lp_prim);
    _finder->setNodeSetCapacity(_SECONDARY_LONEPAIRS_SET, lp_sec);
    _finder->setNodeSetCapacity(_SUM_LONEPAIRS_SET, lp_sum);

    _finder->setNodeSetCapacity(_CONSTRAINED_LONEPAIRS_SET, _constrained_primary_lonepairs + _constrained_secondary_lonepairs);
    return true;
}

bool MoleculeElectronsLocalizer::localize(bool only_check_possibility)
{
    if (!_setConstraintSetForAtoms())
        return false;
    if (!_setConstraintSetForLonepairs(only_check_possibility))
        return false;

    // Find valid solution
    int constrained_double_bonds_conn = _constrained_primary_double_bonds_conn + _constrained_secondary_double_bonds_conn;
    if (constrained_double_bonds_conn % 2 != 0)
        throw Error("Internal error in localize");

    int cardinality = _double_bonds - constrained_double_bonds_conn / 2 + _primary_lonepairs + _secondary_lonepairs;

    if (only_check_possibility)
        return _finder->findMatching(cardinality);

    return _findValidSolution(cardinality);
}

bool MoleculeElectronsLocalizer::_findValidSolution(int cardinality)
{
    bool found = _finder->findMatching(cardinality);
    if (!found)
        return false;

    // Check if localization is valid
    int invalid_atom = -1;
    int ret_code = -1;
    for (int v = _skeleton.vertexBegin(); v != _skeleton.vertexEnd(); v = _skeleton.vertexNext(v))
    {
        if ((ret_code = _isLocalizationValid(v)) != OK)
        {
            invalid_atom = v;
            break;
        }
    }
    if (invalid_atom == -1)
        return true;

    if (ret_code == LONEPAIRS)
        return _branchOnLonepairs(cardinality, invalid_atom);
    else
        return _branchOnConnectivity(cardinality, invalid_atom);
}

bool MoleculeElectronsLocalizer::_branchOnLonepairs(int cardinality, int invalid_atom)
{
    _AtomInfo& info = _atom_info[invalid_atom];
    if (info.atom_fixed || info.atom_saturated)
        throw Error("Internal algorithm error in _branchOnLonepairs");

    // Set number of lonepairs to zero
    int max_mult = _finder->getMaxEdgeMultiplicity(info.orbitals_edge);
    _finder->setMaxEdgeMultiplicity(info.orbitals_edge, 0);
    bool found = _findValidSolution(cardinality);
    // Restore
    _finder->setMaxEdgeMultiplicity(info.orbitals_edge, max_mult);
    if (found)
        return true;

    // Saturate atom
    _fixAtomSaturated(invalid_atom);
    bool was_set = _setConstraintSetForAtoms();
    if (was_set)
        found = _findValidSolution(cardinality);
    // Restore
    _unfixAtomSaturated(invalid_atom);
    was_set = _setConstraintSetForAtoms();
    if (!was_set)
        throw Error("Internal algorithm error in _branchOnLonepairs #2");
    if (found)
        return true;

    return false;
}

bool MoleculeElectronsLocalizer::_branchOnConnectivity(int cardinality, int invalid_atom)
{
    _AtomInfo& info = _atom_info[invalid_atom];

    int max_lp = info.max_add_connectivity - info.fixed_connectivity;
    int lonepairs[2] = {0, max_lp};

    for (int i = 0; i < 2; i++)
    {
        int lp = lonepairs[i];
        if (lp < 0)
            continue;
        // Saturate atom
        bool f1 = _fixAtomConnectivityAndLonepairs(invalid_atom, info.fixed_connectivity, lp);
        if (!f1)
            continue;
        bool f2 = _setConstraintSetForAtoms();
        bool f3 = _setConstraintSetForLonepairs(false);

        bool found = false;
        if (f1 && f2 && f3)
            found = _findValidSolution(cardinality);
        // Restore
        _unfixAtomConnectivityAndLonepairs(invalid_atom);
        f2 = _setConstraintSetForAtoms();
        f3 = _setConstraintSetForLonepairs(false);
        if (!f2 || !f3)
            throw Error("Internal algorithm error in _branchOnConnectivity");

        if (found)
            return true;
    }

    return false;
}

int MoleculeElectronsLocalizer::_isLocalizationValid(int atom) const
{
    const _AtomInfo& info = _atom_info[atom];

    // Check if atom unsaturated then number of lonepairs must be zero
    int sum_added_connectivity = _finder->getNodeIncidentEdgesCount(info.atom_node);
    int lonepairs = _finder->getEdgeMultiplicity(info.orbitals_edge);

    if (sum_added_connectivity != info.max_add_connectivity)
    {
        if (lonepairs != 0)
            return LONEPAIRS;
    }
    if (info.atom_connectivity_fixed)
    {
        int conn = sum_added_connectivity - lonepairs;
        if (conn != info.fixed_connectivity)
            return CONNECTIVITY;
    }

    return OK;
}

void MoleculeElectronsLocalizer::copyBondsAndCharges(Molecule& dest, const Array<int>& mapping) const
{
    // Copy atom charges
    for (int v = _skeleton.vertexBegin(); v != _skeleton.vertexEnd(); v = _skeleton.vertexNext(v))
    {
        int dv = mapping[v];
        int charge = _getAtomCharge(v);
        dest.setAtomCharge(dv, charge);
    }

    // Copy bond types
    for (int e = _skeleton.edgeBegin(); e != _skeleton.edgeEnd(); e = _skeleton.edgeNext(e))
    {
        int de = Graph::findMappedEdge(_skeleton, dest, e, mapping.ptr());

        int multiplicity;
        int ftype = _edges_fixed_type[e];
        if (ftype != -1)
        {
            multiplicity = ftype - 1;
        }
        else
            multiplicity = _finder->getEdgeMultiplicity(e);
        dest.setBondOrder(de, multiplicity + 1);
    }
}

bool MoleculeElectronsLocalizer::fixAtomCharge(int atom, int charge)
{
    int conn, lonepairs;
    if (!_calcConnectivityAndLoneparis(atom, charge, &conn, &lonepairs))
        return false;

    const _AtomInfo& info = _atom_info[atom];

    conn -= info.skeleton_connectivity;
    if (conn < 0)
        return false;

    if (!_fixAtomConnectivityAndLonepairs(atom, conn, lonepairs))
        return false;

    return true;
}

bool MoleculeElectronsLocalizer::fixAtomConnectivity(int atom, int connectivity)
{
    _AtomInfo& info = _atom_info[atom];
    if (info.atom_connectivity_fixed)
        return false;
    connectivity -= info.skeleton_connectivity;
    if (info.atom_fixed && info.fixed_connectivity != connectivity)
        return false;

    info.atom_connectivity_fixed = true;
    info.fixed_connectivity = connectivity;
    return true;
}

bool MoleculeElectronsLocalizer::fixBond(int bond, int type)
{
    if (type != BOND_SINGLE && type != BOND_DOUBLE && type != BOND_TRIPLE)
        throw Error("Only single, double and triple bonds are supported");
    if (_edges_fixed_type[bond] != -1)
        throw Error("Bond has already been fixed");

    const Edge& edge = _skeleton.getEdge(bond);

    if (!_checkAtomBondFixed(edge.beg, type) || !_checkAtomBondFixed(edge.end, type))
        return false;

    _updateAtomBondFixed(edge.beg, type, true);
    _updateAtomBondFixed(edge.end, type, true);

    _finder->setMaxEdgeMultiplicity(_edge_mapping[bond], 0);
    _edges_fixed_type[bond] = type;
    return true;
}

bool MoleculeElectronsLocalizer::fixBondSingleDouble(int bond)
{
    if (_edges_fixed_type[bond] != -1)
        throw Error("Bond has already been fixed");

    _finder->setMaxEdgeMultiplicity(_edge_mapping[bond], 1);
    _edges_fixed_type[bond] = BOND_SINGLE_OR_DOUBLE;

    return true;
}

bool MoleculeElectronsLocalizer::_checkAtomBondFixed(int atom, int bond_type)
{
    _AtomInfo& info = _atom_info[atom];
    if (info.max_add_connectivity < bond_type - 1)
        return false;

    if (info.atom_saturated || info.atom_fixed)
    {
        int cap = _finder->getNodeCapacity(info.atom_node, _CONSTRAINED_ATOMS_SET);
        if (cap < bond_type - 1)
            return false;
    }
    return true;
}

void MoleculeElectronsLocalizer::_updateAtomBondFixed(int atom, int bond_type, bool fixed)
{
    _AtomInfo& info = _atom_info[atom];

    int fc = info.fixed_connectivity, fl = info.fixed_lonepairs;
    bool was_fixed = info.atom_fixed;
    if (was_fixed)
    {
        _unfixAtomConnectivityAndLonepairs(atom);
    }

    int sign = fixed ? 1 : -1;
    int delta = (bond_type - 1) * sign;

    int constr_prim_conn = -1, constr_sec_conn = -1;
    if (fixed)
        _splitConnectivity(atom, bond_type - 1, &constr_prim_conn, &constr_sec_conn);

    info.skeleton_connectivity += delta;
    info.max_add_connectivity -= delta;
    info.zc_connectivity -= delta;

    if (!fixed)
        _splitConnectivity(atom, bond_type - 1, &constr_prim_conn, &constr_sec_conn);

    if (info.atom_saturated)
        _constrained_saturated_atoms -= delta;

    if (was_fixed)
    {
        bool ret = _fixAtomConnectivityAndLonepairs(atom, fc - delta, fl);
        if (!ret)
            throw Error("Internal error while fixing atom");
    }
    else if (info.atom_saturated)
    {
        int cap = _finder->getNodeCapacity(info.atom_node, _CONSTRAINED_ATOMS_SET);
        _finder->setNodeCapacity(info.atom_node, cap - delta, _CONSTRAINED_ATOMS_SET);
    }
    else
    {
        _setupBMatchingNodeAtom(atom);
    }

    if (info.atom_connectivity_fixed && !info.atom_fixed)
        info.fixed_connectivity -= delta;

    _constrained_primary_double_bonds_conn += constr_prim_conn * sign;
    _constrained_secondary_double_bonds_conn += constr_sec_conn * sign;
}

void MoleculeElectronsLocalizer::unfixBond(int bond)
{
    if (_edges_fixed_type[bond] == -1)
        throw Error("Bond wasn't fixed");

    int type = _edges_fixed_type[bond];

    const Edge& edge = _skeleton.getEdge(bond);

    if (type != BOND_SINGLE_OR_DOUBLE)
    {
        _updateAtomBondFixed(edge.beg, type, false);
        _updateAtomBondFixed(edge.end, type, false);
    }

    int beg_cap = _atom_info[edge.beg].max_add_connectivity0;
    int end_cap = _atom_info[edge.end].max_add_connectivity0;

    _finder->setMaxEdgeMultiplicity(_edge_mapping[bond], std::min(beg_cap, end_cap));

    _edges_fixed_type[bond] = -1;
}

void MoleculeElectronsLocalizer::unfixAll()
{
    // Unfix bonds
    for (int e = _skeleton.edgeBegin(); e != _skeleton.edgeEnd(); e = _skeleton.edgeNext(e))
    {
        if (_edges_fixed_type[e] != -1)
            unfixBond(e);
    }

    // Unfix atoms
    for (int v = _skeleton.vertexBegin(); v != _skeleton.vertexEnd(); v = _skeleton.vertexNext(v))
    {
        if (_atom_info[v].atom_fixed)
            unfixAtom(v);
    }
}

bool MoleculeElectronsLocalizer::_fixAtomConnectivityAndLonepairs(int atom, int added_connectivity, int lonepairs)
{
    _AtomInfo& info = _atom_info[atom];

    if (info.max_add_connectivity < added_connectivity + lonepairs)
        return false;

    if (info.atom_fixed)
        return false;
    if (info.atom_connectivity_fixed && info.fixed_connectivity != added_connectivity)
        return false;

    // Check possibility of fixing: atom should be saturated or
    // shouldn't have lonepairs
    if (added_connectivity + lonepairs != info.max_add_connectivity)
    {
        if (lonepairs != 0)
            return false; // If atom isn't saturated then lonepairs must be zero
        if (info.atom_saturated)
            return false; // Saturated atom can't be unsaturated
    }

    int conn_prim, conn_sec;
    _splitConnectivity(atom, added_connectivity + lonepairs, &conn_prim, &conn_sec);
    _constrained_primary_atoms += conn_prim;
    _constrained_secondary_atoms += conn_sec;

    _finder->setNodeCapacity(info.atom_node, 0, _PRIMARY_ATOMS_SET);
    _finder->setNodeCapacity(info.atom_node, 0, _SECONDARY_ATOMS_SET);
    if (added_connectivity + lonepairs < 0)
        _finder->setNodeCapacity(info.atom_node, 0, _CONSTRAINED_ATOMS_SET);
    else
        _finder->setNodeCapacity(info.atom_node, added_connectivity + lonepairs, _CONSTRAINED_ATOMS_SET);

    int lonepairs_prim, lonepairs_sec;
    _splitLonepairs(atom, lonepairs, &lonepairs_prim, &lonepairs_sec);
    _constrained_primary_lonepairs += lonepairs_prim;
    _constrained_secondary_lonepairs += lonepairs_sec;

    _finder->setNodeCapacity(info.orbitals_node, 0, _PRIMARY_LONEPAIRS_SET);
    _finder->setNodeCapacity(info.orbitals_node, 0, _SECONDARY_LONEPAIRS_SET);
    _finder->setNodeCapacity(info.orbitals_node, lonepairs, _CONSTRAINED_LONEPAIRS_SET);

    info.atom_fixed = true;
    info.fixed_connectivity = added_connectivity;
    info.fixed_lonepairs = lonepairs;

    if (info.atom_saturated)
        _constrained_saturated_atoms -= added_connectivity + lonepairs;
    return true;
}

void MoleculeElectronsLocalizer::_unfixAtomConnectivityAndLonepairs(int atom)
{
    _AtomInfo& info = _atom_info[atom];
    if (!info.atom_fixed)
        throw Error("Can't unfix atom that wasn't fixed");

    int conn_prim, conn_sec;
    _splitConnectivity(atom, info.fixed_connectivity + info.fixed_lonepairs, &conn_prim, &conn_sec);
    _constrained_primary_atoms -= conn_prim;
    _constrained_secondary_atoms -= conn_sec;

    int lonepairs_prim, lonepairs_sec;
    _splitLonepairs(atom, info.fixed_lonepairs, &lonepairs_prim, &lonepairs_sec);
    _constrained_primary_lonepairs -= lonepairs_prim;
    _constrained_secondary_lonepairs -= lonepairs_sec;

    if (info.atom_saturated)
        _constrained_saturated_atoms += info.fixed_connectivity + info.fixed_lonepairs;

    info.atom_fixed = false;
    if (!info.atom_connectivity_fixed)
        info.fixed_connectivity = -1;
    info.fixed_lonepairs = -1;

    if (!info.atom_saturated)
        _setupBMatchingNodeAtom(atom);

    _setupBMatchingNodeOrbital(atom);
}

void MoleculeElectronsLocalizer::unfixAtom(int atom)
{
    _AtomInfo& info = _atom_info[atom];
    if (!info.atom_fixed && !info.atom_connectivity_fixed)
        throw Error("Can't unfix atom that wasn't fixed");
    info.atom_connectivity_fixed = false;
    if (!info.atom_fixed)
        info.fixed_connectivity = -1;
    else
        _unfixAtomConnectivityAndLonepairs(atom);
}

void MoleculeElectronsLocalizer::_splitConnectivity(int atom, int conn, int* prim, int* sec) const
{
    const _AtomInfo& info = _atom_info[atom];

    int zc_sum_conn = std::max(info.zc_connectivity, 0) + info.zc_lonepairs;
    if (conn < zc_sum_conn)
    {
        *prim = conn;
        *sec = 0;
    }
    else
    {
        *prim = zc_sum_conn;
        *sec = conn - zc_sum_conn;
    }
}

void MoleculeElectronsLocalizer::_splitLonepairs(int atom, int lonepairs, int* prim, int* sec) const
{
    const _AtomInfo& info = _atom_info[atom];

    if (lonepairs < info.zc_lonepairs)
    {
        *prim = lonepairs;
        *sec = 0;
    }
    else
    {
        *prim = info.zc_lonepairs;
        *sec = lonepairs - info.zc_lonepairs;
    }
}

void MoleculeElectronsLocalizer::_fixAtomSaturated(int atom)
{
    _AtomInfo& info = _atom_info[atom];
    if (info.atom_fixed)
        throw Error("Such call sequence wasn't expected");
    if (info.atom_saturated)
        return;

    _finder->setNodeCapacity(info.atom_node, 0, _PRIMARY_ATOMS_SET);
    _finder->setNodeCapacity(info.atom_node, 0, _SECONDARY_ATOMS_SET);
    _finder->setNodeCapacity(info.atom_node, info.max_add_connectivity, _CONSTRAINED_ATOMS_SET);

    _constrained_saturated_atoms += info.max_add_connectivity;

    info.atom_saturated = true;
}

void MoleculeElectronsLocalizer::_unfixAtomSaturated(int atom)
{
    _AtomInfo& info = _atom_info[atom];
    if (info.atom_fixed)
        throw Error("Such call sequence wasn't expected");
    if (!info.atom_saturated)
        return;

    _constrained_saturated_atoms -= info.max_add_connectivity;
    _setupBMatchingNodeAtom(atom);

    info.atom_saturated = false;
}

bool MoleculeElectronsLocalizer::_canAtomBeUnsaturated(int atom)
{
    int zero_charge_electrons = Element::electrons(_skeleton.getAtomNumber(atom), 0);
    if (zero_charge_electrons > 5)
    {
        // If such atom is unsaturated then its charge is more than 2
        return false;
    }

    return true;
}

int MoleculeElectronsLocalizer::getLocalizationChargesCount() const
{
    int charges = 0;
    for (int v = _skeleton.vertexBegin(); v != _skeleton.vertexEnd(); v = _skeleton.vertexNext(v))
    {
        charges += abs(_getAtomCharge(v));
    }

    return charges;
}

int MoleculeElectronsLocalizer::_getAtomCharge(int v) const
{
    const _AtomInfo& info = _atom_info[v];

    int added_conn, lonepairs;
    _getAtomConnAndLonepairs(v, added_conn, lonepairs);

    int label = _skeleton.getAtomNumber(v);
    int radical = _skeleton.getAtomRadical(v);

    int conn = added_conn + info.skeleton_connectivity;
    int electrons = conn + 2 * lonepairs + Element::radicalElectrons(radical);

    int zc_electrons = Element::electrons(label, 0);

    return zc_electrons - electrons;
}

void MoleculeElectronsLocalizer::_getAtomConnAndLonepairs(int atom, int& added_conn, int& lonepairs) const
{
    const _AtomInfo& info = _atom_info[atom];
    if (info.atom_fixed)
    {
        added_conn = info.fixed_connectivity;
        lonepairs = info.fixed_lonepairs;
    }
    else
    {
        added_conn = _finder->getNodeIncidentEdgesCount(info.atom_node);
        lonepairs = _finder->getEdgeMultiplicity(info.orbitals_edge);
        added_conn -= lonepairs;
    }
}

bool MoleculeElectronsLocalizer::isAllAtomsHaveOctet() const
{
    for (int v = _skeleton.vertexBegin(); v != _skeleton.vertexEnd(); v = _skeleton.vertexNext(v))
    {
        int added_conn, lonepairs;
        _getAtomConnAndLonepairs(v, added_conn, lonepairs);

        const _AtomInfo& info = _atom_info[v];
        if (added_conn + lonepairs != info.max_add_connectivity)
            return false;
    }
    return true;
}
