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

#include "molecule/molecule_arom.h"

#include "base_c/bitarray.h"
#include "base_cpp/gray_codes.h"
#include "graph/cycle_enumerator.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

using namespace indigo;

//
// AromatizerBase
//

IMPL_ERROR(AromatizerBase, "aromatizer");

CP_DEF(AromatizerBase);

AromatizerBase::AromatizerBase(BaseMolecule& molecule)
    : _basemol(molecule), CP_INIT, TL_CP_GET(_bonds_arom), TL_CP_GET(_bonds_arom_count), TL_CP_GET(_unsure_cycles), TL_CP_GET(_cycle_atoms)
{
    _bonds_arom.resize(bitGetSize(molecule.edgeEnd()));
    _bonds_arom_count.resize(molecule.edgeEnd());

    _cycle_atoms.clear_resize(_basemol.vertexEnd());
    reset();
    // collect superatoms' atoms for fast check
    for (int i = molecule.sgroups.begin(); i != molecule.sgroups.end(); i = molecule.sgroups.next(i))
    {
        SGroup& sgroup = molecule.sgroups.getSGroup(i);
        if (sgroup.sgroup_type == SGroup::SG_TYPE_SUP)
        {
            for (int j = 0; j < sgroup.atoms.size(); j++)
                _inside_superatoms.find_or_insert(sgroup.atoms[j]);
        }
    }
}

AromatizerBase::~AromatizerBase()
{
}

bool AromatizerBase::_checkDoubleBonds(const int* cycle, int cycle_len)
{
    int j;

    for (j = 0; j < cycle_len; j++)
    {
        int v_left_idx = cycle[j];
        int v_center_idx = cycle[(j + 1) % cycle_len];
        int v_right_idx = cycle[(j + 2) % cycle_len];

        const Vertex& vertex = _basemol.getVertex(v_center_idx);
        int i;

        int internal_double_bond_count = 0;
        for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
        {
            int nei_idx = vertex.neiVertex(i);

            int e_idx = vertex.neiEdge(i);
            int type = _basemol.getBondOrder(e_idx);
            if (type == BOND_DOUBLE && !isBondAromatic(e_idx))
            {
                if (nei_idx != v_left_idx && nei_idx != v_right_idx)
                {
                    // Double bond going outside
                    if (!_acceptOutgoingDoubleBond(v_center_idx, e_idx))
                        return false;
                }
                else if (nei_idx == v_left_idx || nei_idx == v_right_idx)
                    internal_double_bond_count++;
            }
        }
        if (internal_double_bond_count >= 2)
            return false;
    }
    return true;
}

void AromatizerBase::_aromatizeCycle(const int* cycle, int cycle_len)
{
    for (int i = 0; i < cycle_len; i++)
    {
        int a = cycle[i], b = cycle[(i + 1) % cycle_len];
        int e_idx = _basemol.findEdgeIndex(a, b);
        _bonds_arom_count[e_idx]++;
        bitSetBit(_bonds_arom.ptr(), e_idx, 1);
    }

    // Marks all single bonds that are inside this cycle like in CC1=CC2=CNC=CC2=N1 as aromatic
    _cycle_atoms_mark++;
    for (int i = 0; i < cycle_len; i++)
        _cycle_atoms[cycle[i]] = _cycle_atoms_mark;

    for (int i = 0; i < cycle_len; i++)
    {
        const Vertex& v = _basemol.getVertex(cycle[i]);
        for (int nei : v.neighbors())
        {
            int nei_index = v.neiVertex(nei);
            // Check that the end is in the cycle
            if (_cycle_atoms[nei_index] != _cycle_atoms_mark)
                continue;

            // Check if the bond has already been marked as aromatic
            int nei_edge = v.neiEdge(nei);
            if (_bonds_arom_count[nei_edge] != 0)
                continue;

            // Check that the bond is single
            if (_basemol.getBondOrder(nei_edge) == BOND_SINGLE)
            {
                bitSetBit(_bonds_arom.ptr(), nei_edge, 1);
                _bonds_arom_count[nei_edge]++;
            }
        }
    }

    _handleAromaticCycle(cycle, cycle_len);
}

void AromatizerBase::_handleCycle(const Array<int>& path)
{
    // Check Huckel's rule
    if (!_isCycleAromatic(path.ptr(), path.size()))
        return;
    addAromaticCycle(-1, path.ptr(), path.size());
}

bool AromatizerBase::handleUnsureCycles()
{
    bool changed = true;
    bool is_all_aromatic = true;

    while (changed)
    {
        changed = false;

        for (int i = 0; i < _unsure_cycles.size(); i++)
        {
            if (_unsure_cycles[i].is_empty)
                continue;

            if (_checkDoubleBonds(_unsure_cycles[i].cycle, _unsure_cycles[i].length))
            {
                _aromatizeCycle(_unsure_cycles[i].cycle, _unsure_cycles[i].length);
                _unsure_cycles[i].is_empty = true;
                changed = true;
            }
            else
                is_all_aromatic = false;
        }
    }
    return is_all_aromatic;
}

bool AromatizerBase::_cb_check_vertex(Graph& /*graph*/, int v_idx, void* context)
{
    AromatizerBase* arom = (AromatizerBase*)context;
    return arom->_checkVertex(v_idx);
}

bool AromatizerBase::_cb_handle_cycle(Graph& /*graph*/, const Array<int>& vertices, const Array<int>& /*edges*/, void* context)
{
    AromatizerBase* arom = (AromatizerBase*)context;
    arom->_handleCycle(vertices);
    return true;
}

void AromatizerBase::aromatize()
{
    CycleEnumerator cycle_enumerator(_basemol);

    cycle_enumerator.cb_check_vertex = _cb_check_vertex;
    cycle_enumerator.cb_handle_cycle = _cb_handle_cycle;
    cycle_enumerator.max_length = MAX_CYCLE_LEN;
    cycle_enumerator.context = this;
    cycle_enumerator.process();

    handleUnsureCycles();
}

bool AromatizerBase::isBondAromatic(int e_idx)
{
    return _bonds_arom_count[e_idx] != 0;
}

const byte* AromatizerBase::isBondAromaticArray(void)
{
    return _bonds_arom.ptr();
}

void AromatizerBase::addAromaticCycle(int id, const int* cycle, int cycle_len)
{
    if (!_checkDoubleBonds(cycle, cycle_len))
    {
        int empty_idx;
        if (_unsureCyclesCount == _unsure_cycles.size())
        {
            empty_idx = _unsure_cycles.size();
            _unsure_cycles.push();
        }
        else
        {
            // Find first empty space
            empty_idx = -1;
            for (int i = 0; i < _unsure_cycles.size(); i++)
                if (_unsure_cycles[i].is_empty)
                {
                    empty_idx = i;
                    break;
                }
            if (empty_idx == -1)
                throw Exception("AromatizerBase::addAromaticCycle: internal logic error");
        }

        CycleDef& cycleDef = _unsure_cycles[empty_idx];
        cycleDef.id = id;
        cycleDef.is_empty = false;
        cycleDef.length = cycle_len;

        memcpy(cycleDef.cycle, cycle, cycle_len * sizeof(int));

        _unsureCyclesCount++;
    }
    else
        _aromatizeCycle(cycle, cycle_len);
}

void AromatizerBase::removeAromaticCycle(int id, const int* cycle, int cycle_len)
{
    // Find id in unsure cycles
    for (int i = 0; i < _unsure_cycles.size(); i++)
        if (!_unsure_cycles[i].is_empty && _unsure_cycles[i].id == id)
        {
            _unsure_cycles[i].is_empty = true;
            return;
        }

    // Just decrease bond marks
    for (int i = 0; i < cycle_len; i++)
    {
        int a = cycle[i], b = cycle[(i + 1) % cycle_len];
        int e_idx = _basemol.findEdgeIndex(a, b);
        _bonds_arom_count[e_idx]--;
        if (_bonds_arom_count[e_idx] == 0)
            bitSetBit(_bonds_arom.ptr(), e_idx, 0);
    }
}

bool AromatizerBase::_checkVertex(int /*v_idx*/)
{
    return true;
}

void AromatizerBase::_handleAromaticCycle(const int* /*cycle*/, int /*cycle_len*/)
{
}

void AromatizerBase::reset(void)
{
    _unsure_cycles.clear();
    _bonds_arom.zerofill();
    _bonds_arom_count.zerofill();

    _cycle_atoms.zerofill();
    _cycle_atoms_mark = 1;

    _cyclesHandled = 0;
    _unsureCyclesCount = 0;
    _inside_superatoms.clear();
}

void AromatizerBase::setBondAromaticCount(int e_idx, int count)
{
    _bonds_arom_count[e_idx] = count;
    bitSetBit(_bonds_arom.ptr(), e_idx, count != 0);
}

//
// MoleculeAromatizer
//

CP_DEF(MoleculeAromatizer);

MoleculeAromatizer::MoleculeAromatizer(Molecule& molecule, const AromaticityOptions& options) : AromatizerBase(molecule), CP_INIT, TL_CP_GET(_pi_labels)
{
    _pi_labels.clear_resize(molecule.vertexEnd());
    _options = options;
}

int MoleculeAromatizer::_getPiLabel(int v_idx)
{
    if (!_basemol.vertexInRing(v_idx))
        return -1;

    const Vertex& vertex = _basemol.getVertex(v_idx);

    if (_options.aromatize_skip_superatoms && _inside_superatoms.find(v_idx))
        return -1;

    if (_basemol.isPseudoAtom(v_idx))
        return -1;

    if (_basemol.isTemplateAtom(v_idx))
        return -1;

    if (_basemol.isRSite(v_idx))
        return -1;

    if (!Element::canBeAromatic(_basemol.getAtomNumber(v_idx)))
        return -1;

    Molecule& mol = (Molecule&)_basemol;

    int non_arom_conn = 0, arom_bonds = 0;
    int n_double_ext = 0, n_double_ring = 0;
    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int bond_idx = vertex.neiEdge(i);
        int type = _basemol.getBondOrder(bond_idx);
        if (type == BOND_DOUBLE)
        {
            if (_basemol.getBondTopology(bond_idx) == TOPOLOGY_RING)
                n_double_ring++;
            else
            {
                if (!_acceptOutgoingDoubleBond(v_idx, bond_idx))
                    return -1;
                n_double_ext++;
            }
        }
        if (type == BOND_TRIPLE)
            return -1;
        if (type == BOND_AROMATIC)
            arom_bonds++;
        else
            non_arom_conn++;
    }

    if (arom_bonds == 0)
    {
        // Verify that this atom has valid valence
        if (mol.getImplicitH_NoThrow(v_idx, -1) == -1)
            return -1;
    }

    if (n_double_ring > 0)
        return 1;

    if (n_double_ext > 1)
        return -1;
    else if (n_double_ext == 1)
    {
        // Only a single external double bond that was accepted in _acceptOutgoingDoubleBond
        // It means that it is C=S, C=O, or C=N, like in O=C1NC=CC(=O)N1
        int atom_number = _basemol.getAtomNumber(v_idx);
        if (atom_number == ELEM_S)
            return 2;
        return 0;
    }

    int conn = mol.getAtomConnectivity_NoThrow(v_idx, -1);
    if (conn == -1)
        return -1;

    // Atom is already aromatic and in general number of hydrogens
    // cannot be deduced. But if atom can have one single or onle
    // double bond while being aromatic then pi label can be calculated
    if (arom_bonds != 0)
    {
        int single_bonds_conn = non_arom_conn + arom_bonds;
        int h_with_single = mol.calcImplicitHForConnectivity(v_idx, single_bonds_conn);

        bool can_have_single_bonds = false;
        if (h_with_single >= 0 && _getPiLabelByConn(v_idx, single_bonds_conn) >= 0)
        {
            can_have_single_bonds = true;
            conn = single_bonds_conn;
        }

        bool can_have_one_double_bond = (mol.calcImplicitHForConnectivity(v_idx, single_bonds_conn + 1) >= 0);

        bool can_have_more_double_bonds = false;
        for (int i = 2; i < arom_bonds; i++)
        {
            int h_with_double = mol.calcImplicitHForConnectivity(v_idx, single_bonds_conn + i);
            if (h_with_double >= 0)
                can_have_more_double_bonds = true;
        }

        if (!can_have_single_bonds && can_have_one_double_bond && !can_have_more_double_bonds)
            return 1; // This atom must have double bond
        if (can_have_single_bonds && !can_have_one_double_bond && !can_have_more_double_bonds)
            ; // This atom must have only single bonds as aromatic ones
        else
            return -1; // This case is ambiguous. Treat as nonaromatic.
    }

    return _getPiLabelByConn(v_idx, conn);
}

int MoleculeAromatizer::_getPiLabelByConn(int v_idx, int conn)
{
    Molecule& mol = (Molecule&)_basemol;
    int radical = _basemol.getAtomRadical(v_idx);
    if (radical > 0)
        return 1;

    int lonepairs = 0;
    if (mol.getVacantPiOrbitals(v_idx, conn, &lonepairs) > 0)
        return 0;

    if (lonepairs > 0)
        return 2;

    return -1;
}

void MoleculeAromatizer::precalculatePiLabels()
{
    for (int v_idx = _basemol.vertexBegin(); v_idx < _basemol.vertexEnd(); v_idx = _basemol.vertexNext(v_idx))
        _pi_labels[v_idx] = _getPiLabel(v_idx);
}

bool MoleculeAromatizer::_checkVertex(int v_idx)
{
    return _pi_labels[v_idx] != -1;
}

bool MoleculeAromatizer::_isCycleAromatic(const int* cycle, int cycle_len)
{
    int count = 0;
    // Check Huckel's rule
    for (int i = 0; i < cycle_len; i++)
        count += _pi_labels[cycle[i]];

    if (((count - 2) % 4) != 0)
        return false;
    return true;
}

bool MoleculeAromatizer::_acceptOutgoingDoubleBond(int atom, int bond)
{
    if (_options.method == AromaticityOptions::GENERIC)
    {
        // Note: this method should be in sync with QueryMoleculeAromatizer::_acceptOutgoingDoubleBond

        // CC1=CC=CC=[N]1=C
        int atom_number = _basemol.getAtomNumber(atom);
        if (atom_number == ELEM_C || atom_number == ELEM_S)
        {
            int end = _basemol.getEdgeEnd(atom, bond);
            int end_number = _basemol.getAtomNumber(end);
            if (atom_number == ELEM_C)
            {
                // [O-][N+](=O)C1=CNC=C(Cl)C1=O (see CID 11850826)
                // CN1SC(=N)N(C)C1=S (see CID 11949795)
                if (end_number == ELEM_N || end_number == ELEM_O || end_number == ELEM_S)
                    // Corresponding pi label is 0
                    return true;
            }
            if (atom_number == ELEM_S)
            {
                // O=S1N=CC=N1
                if (end_number == ELEM_O)
                    // Corresponding pi label is 0
                    return true;
            }
        }
    }

    Molecule& mol = _basemol.asMolecule();
    if (mol.isNitrogenV5(atom))
        return true;

    return false;
}

bool MoleculeAromatizer::aromatizeBonds(Molecule& mol, const AromaticityOptions& options)
{
    MoleculeAromatizer aromatizer(mol, options);

    aromatizer.precalculatePiLabels();
    aromatizer.aromatize();

    bool aromatic_bond_found = false;
    for (int e_idx = mol.edgeBegin(); e_idx < mol.edgeEnd(); e_idx = mol.edgeNext(e_idx))
        if (aromatizer.isBondAromatic(e_idx))
        {
            mol.setBondOrder(e_idx, BOND_AROMATIC, true);
            aromatic_bond_found = true;
        }

    // Aromatize RGroups
    int n_rgroups = mol.rgroups.getRGroupCount();
    for (int i = 1; i <= n_rgroups; i++)
    {
        PtrPool<BaseMolecule>& frags = mol.rgroups.getRGroup(i).fragments;

        for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
        {
            Molecule& fragment = frags[j]->asMolecule();
            aromatic_bond_found |= MoleculeAromatizer::aromatizeBonds(fragment, options);
        }
    }

    return aromatic_bond_found;
}

//
// QueryMoleculeAromatizer
//

CP_DEF(QueryMoleculeAromatizer);

QueryMoleculeAromatizer::QueryMoleculeAromatizer(QueryMolecule& molecule, const AromaticityOptions& options)
    : AromatizerBase(molecule), CP_INIT, TL_CP_GET(_pi_labels), TL_CP_GET(_aromatic_cycles)
{
    _pi_labels.clear_resize(molecule.vertexEnd());
    _aromatic_cycles.clear();
    _aromatic_cycles.reserve(100);
    _mode = FUZZY;
    _collecting = false;
    _options = options;
}

void QueryMoleculeAromatizer::precalculatePiLabels()
{
    for (int v_idx = _basemol.vertexBegin(); v_idx < _basemol.vertexEnd(); v_idx = _basemol.vertexNext(v_idx))
        _pi_labels[v_idx] = _getPiLabel(v_idx);
}

bool QueryMoleculeAromatizer::_checkVertex(int v_idx)
{
    return _pi_labels[v_idx].canBeAromatic();
}

bool QueryMoleculeAromatizer::_isCycleAromatic(const int* cycle, int cycle_len)
{
    QueryMolecule& query = (QueryMolecule&)_basemol;
    // Single/double bond can't be aromatic and Check if cycle wasn't aromatic
    bool all_aromatic = true;
    for (int i = 0; i < cycle_len; i++)
    {
        int a = cycle[i], b = cycle[(i + 1) % cycle_len];
        int e_idx = _basemol.findEdgeIndex(a, b);
        if (!query.possibleBondOrder(e_idx, BOND_AROMATIC))
            all_aromatic = false;
    }
    if (all_aromatic)
        return false;

    PiValue cycle_sum(0, 0);
    // Check Huckel's rule
    for (int i = 0; i < cycle_len; i++)
    {
        PiValue& cur = _pi_labels[cycle[i]];
        if (cur.min == -1 || cur.max == -1)
            throw Error("interal error in _isCycleAromatic");

        cycle_sum.max += cur.max;
        cycle_sum.min += cur.min;
    }

    // Check Huckel's rule
    if (_mode == EXACT)
    {
        if (cycle_sum.min != cycle_sum.max)
            return false;

        int sum = cycle_sum.min;
        // Check if cycle have pi-lables sum 4n+2 for drawn query
        if (sum % 4 != 2)
            return false;
        return true;
    }

    //
    // Fuzzy mode: check if circle can have 4n-2 value
    //

    if (cycle_sum.max - cycle_sum.min > 3)
        return true;

    int residue_min = (cycle_sum.min + 2) % 4;
    int residue_max = (cycle_sum.max + 2) % 4;

    if (residue_min == 0 || residue_min > residue_max)
        return true;
    return false;
}

QueryMoleculeAromatizer::PiValue QueryMoleculeAromatizer::_getPiLabel(int v_idx)
{
    int exact_double_bonds = 0;
    bool has_query_bond = false;

    QueryMolecule& query = (QueryMolecule&)_basemol;

    // Check double bonds
    const Vertex& vertex = _basemol.getVertex(v_idx);
    int min_conn = vertex.degree();
    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int bond_type = query.getBondOrder(vertex.neiEdge(i));

        switch (bond_type)
        {
        case BOND_DOUBLE:
            exact_double_bonds++;
            min_conn++;
            break;
        case BOND_TRIPLE:
            // Triple bonds can't be attached to atoms in aromatic rings
            return PiValue(-1, -1);
        case -1:
        case BOND_AROMATIC:
            has_query_bond = true;
            break;
        }
    }

    if (_options.aromatize_skip_superatoms && _inside_superatoms.size() && _inside_superatoms.find(v_idx))
        return PiValue(-1, -1);

    if (query.isRSite(v_idx))
    {
        if (vertex.degree() == 1)
        {
            // R-Group with single attachment point cannot be in aromatic ring
            return PiValue(-1, -1);
        }
        else
        {
            // R-Group with two attachment points
            // Here can be chain of atoms with different summary pi-labels
            return PiValue(0, 4);
        }
    }

    if (exact_double_bonds > 1)
    {
        if (_options.method == AromaticityOptions::BASIC)
        {
            if (!query.possibleNitrogenV5(v_idx))
                return PiValue(-1, -1);
        }
        else
        {
            bool possible_c = _basemol.possibleAtomNumber(v_idx, ELEM_C);
            bool possible_s = _basemol.possibleAtomNumber(v_idx, ELEM_S);
            if (possible_s && possible_c)
                return PiValue(0, 2);
            else if (possible_s)
                return PiValue(2, 2);
            else
                return PiValue(0, 0);
        }
    }

    if (has_query_bond)
    {
        if (_mode == EXACT)
            return PiValue(-1, -1);
        else
        {
            if (_options.method == AromaticityOptions::BASIC)
                if (exact_double_bonds > 0)
                    return PiValue(1, 1);

            return PiValue(0, 2); // TODO: check different cases
        }
    }

    // For aromaticity treat atoms without constrains as having default constraint.
    // For example if charge not specified then treat as charge is zero
    int number = query.getAtomNumber(v_idx);

    QueryMolecule::Atom& atom = query.getAtom(v_idx);
    // if (atom.hasConstraint(QueryMolecule::ATOM_FRAGMENT))
    //   throw Error("not implemented yet");

    if (number == -1)
        return PiValue(0, 2); // TODO: check different cases

    if (!Element::canBeAromatic(number))
        return PiValue(-1, -1);

    int radical = query.getAtomRadical(v_idx);
    if (radical == -1)
    {
        if (atom.hasConstraint(QueryMolecule::ATOM_RADICAL))
            return PiValue(0, 2); // TODO: check different cases
        radical = 0;
    }

    int charge = query.getAtomCharge(v_idx);
    if (charge == CHARGE_UNKNOWN)
    {
        if (atom.hasConstraint(QueryMolecule::ATOM_CHARGE))
            return PiValue(0, 2); // TODO: check different cases
        charge = 0;
    }

    if (radical > 0)
        return PiValue(1, 1);

    int valence, implicit_h;

    if (!Element::calcValence(number, charge, radical, min_conn, valence, implicit_h, false) && !query.possibleNitrogenV5(v_idx))
        return PiValue(-1, -1);

    if (_basemol.possibleAtomNumber(v_idx, ELEM_C) && query.getExplicitValence(v_idx) == 5)
        return PiValue(-1, -1);

    if (exact_double_bonds >= 1)
    {
        if (_options.method == AromaticityOptions::BASIC)
        {
            if (exact_double_bonds > 0)
                return PiValue(1, 1);
        }
        else
        {
            for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
            {
                int edge = vertex.neiEdge(i);
                if (query.possibleBondOrder(edge, BOND_DOUBLE))
                {
                    if (_acceptOutgoingDoubleBond(v_idx, edge))
                    {
                        bool possible_c = _basemol.possibleAtomNumber(v_idx, ELEM_C);
                        bool possible_s = _basemol.possibleAtomNumber(v_idx, ELEM_S);
                        if (possible_s && possible_c)
                            return PiValue(1, 2);
                        else if (possible_s)
                            return PiValue(1, 2);
                        else
                            return PiValue(0, 1);
                    }
                }
            }
        }
        return PiValue(1, 1);
    }

    int pi_label = -1;
    int lonepairs = 0;

    int group = Element::group(number);
    if (BaseMolecule::getVacantPiOrbitals(group, charge, radical, min_conn + implicit_h, &lonepairs) > 0)
        pi_label = 0;
    else if (lonepairs > 0)
        pi_label = 2;

    return PiValue(pi_label, pi_label);
}

void QueryMoleculeAromatizer::_handleAromaticCycle(const int* cycle, int cycle_len)
{
    if (!_collecting)
        return;
    // Add cycle to storage
    _aromatic_cycles.push();
    CycleDef& def = _aromatic_cycles[_aromatic_cycles.size() - 1];
    def.id = _aromatic_cycles.size() - 1;
    def.is_empty = false;
    def.length = cycle_len;
    memcpy(def.cycle, cycle, cycle_len * sizeof(int));

    AromatizerBase::_handleAromaticCycle(cycle, cycle_len);
}

bool QueryMoleculeAromatizer::_acceptOutgoingDoubleBond(int atom, int bond)
{
    if (_mode == EXACT)
        return false;

    if (_options.method == AromaticityOptions::GENERIC)
    {
        // Note: this method should be in sync with MoleculeAromatizer::_acceptOutgoingDoubleBond

        // CC1=CC=CC=[N]1=C
        bool possible_c = _basemol.possibleAtomNumber(atom, ELEM_C);
        bool possible_s = _basemol.possibleAtomNumber(atom, ELEM_S);
        if (possible_c || possible_s)
        {
            int end = _basemol.getEdgeEnd(atom, bond);
            if (possible_c)
            {
                // [O-][N+](=O)C1=CNC=C(Cl)C1=O (see CID 11850826)
                // CN1SC(=N)N(C)C1=S (see CID 11949795)
                if (_basemol.possibleAtomNumber(end, ELEM_N) || _basemol.possibleAtomNumber(end, ELEM_O) || _basemol.possibleAtomNumber(end, ELEM_S))
                    // Corresponding pi label is 0
                    return true;
            }
            if (possible_s)
            {
                // O=S1N=CC=N1
                if (_basemol.possibleAtomNumber(end, ELEM_O))
                    // Corresponding pi label is 0
                    return true;
            }
        }
    }

    QueryMolecule& qmol = _basemol.asQueryMolecule();
    if (qmol.possibleNitrogenV5(atom))
        return true;

    return false;
}

void QueryMoleculeAromatizer::setMode(int mode)
{
    _mode = mode;
}

bool QueryMoleculeAromatizer::aromatizeBonds(QueryMolecule& mol, const AromaticityOptions& options)
{
    return _aromatizeBonds(mol, -1, options);
}

bool QueryMoleculeAromatizer::_aromatizeBonds(QueryMolecule& mol, int additional_atom, const AromaticityOptions& options)
{
    bool aromatized = false;
    // Mark edges that can be aromatic in some matching
    aromatized |= _aromatizeBondsFuzzy(mol, options);
    // Aromatize all aromatic cycles
    aromatized |= _aromatizeBondsExact(mol, options);

    MoleculeRGroups& rgroups = mol.rgroups;
    int n_rgroups = rgroups.getRGroupCount();

    // Check if r-groups are attached with single bonds
    QS_DEF(Array<bool>, rgroups_attached_single);
    rgroups_attached_single.clear();
    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
    {
        if (v == additional_atom)
            continue;
        if (mol.isRSite(v))
        {
            // Check if neighbor bonds are single
            const Vertex& vertex = mol.getVertex(v);
            for (int nei = vertex.neiBegin(); nei != vertex.neiEnd(); nei = vertex.neiNext(nei))
            {
                int edge = vertex.neiEdge(nei);
                QueryMolecule::Bond& bond = mol.getBond(edge);

                // DP TODO: implement smth. like Node::possibleOtherValueExcept() ...

                bool can_be_double = bond.possibleValue(QueryMolecule::BOND_ORDER, BOND_DOUBLE);
                bool can_be_triple = bond.possibleValue(QueryMolecule::BOND_ORDER, BOND_TRIPLE);
                bool can_be_arom = bond.possibleValue(QueryMolecule::BOND_ORDER, BOND_AROMATIC);
                if (can_be_double || can_be_triple || can_be_arom)
                {
                    QS_DEF(Array<int>, sites);

                    mol.getAllowedRGroups(v, sites);
                    for (int j = 0; j < sites.size(); j++)
                    {
                        rgroups_attached_single.expandFill(sites[j] + 1, true);
                        rgroups_attached_single[sites[j]] = false;
                    }
                }
            }
        }
    }

    rgroups_attached_single.expandFill(n_rgroups + 1, true);
    for (int i = 1; i <= n_rgroups; i++)
    {
        PtrPool<BaseMolecule>& frags = rgroups.getRGroup(i).fragments;

        for (int j = frags.begin(); j != frags.end(); j = frags.next(j))
        {
            QueryMolecule& fragment = frags[j]->asQueryMolecule();

            aromatized |= _aromatizeRGroupFragment(fragment, rgroups_attached_single[i], options);
        }
    }
    return aromatized;
}

bool QueryMoleculeAromatizer::_aromatizeRGroupFragment(QueryMolecule& fragment, bool add_single_bonds, const AromaticityOptions& options)
{
    // Add additional atom to attachment points
    int additional_atom = fragment.addAtom(new QueryMolecule::Atom(QueryMolecule::ATOM_RSITE, 1));

    // Connect it with attachment points
    int maxOrder = fragment.attachmentPointCount();
    for (int i = 1; i <= maxOrder; i++)
    {
        int pointIndex = 0;
        int point;
        while (true)
        {
            point = fragment.getAttachmentPoint(i, pointIndex);
            if (point == -1)
                break;

            if (fragment.findEdgeIndex(point, additional_atom) == -1)
            {
                std::unique_ptr<QueryMolecule::Bond> bond;
                if (add_single_bonds)
                    bond = std::make_unique<QueryMolecule::Bond>(QueryMolecule::BOND_ORDER, BOND_SINGLE);
                else
                    bond = std::make_unique<QueryMolecule::Bond>();

                fragment.addBond(point, additional_atom, bond.release());
            }

            pointIndex++;
        }
    }

    bool aromatized = _aromatizeBonds(fragment, additional_atom, options);

    QS_DEF(Array<int>, indices);
    indices.clear();
    indices.push(additional_atom);

    fragment.removeAtoms(indices);
    return aromatized;
}

// Some cycles with query features can be aromatized
bool QueryMoleculeAromatizer::_aromatizeBondsExact(QueryMolecule& qmol, const AromaticityOptions& options)
{
    bool aromatized = false;
    QueryMoleculeAromatizer aromatizer(qmol, options);

    aromatizer.setMode(QueryMoleculeAromatizer::EXACT);
    aromatizer.precalculatePiLabels();
    aromatizer.aromatize();

    for (int e_idx = qmol.edgeBegin(); e_idx < qmol.edgeEnd(); e_idx = qmol.edgeNext(e_idx))
        if (aromatizer.isBondAromatic(e_idx))
        {
            std::unique_ptr<QueryMolecule::Bond> bond(qmol.releaseBond(e_idx));
            bond->removeConstraints(QueryMolecule::BOND_ORDER);

            std::unique_ptr<QueryMolecule::Bond> arom_bond = std::make_unique<QueryMolecule::Bond>(QueryMolecule::BOND_ORDER, BOND_AROMATIC);

            qmol.resetBond(e_idx, QueryMolecule::Bond::und(bond.release(), arom_bond.release()));

            aromatized = true;
        }
    return aromatized;
}

bool QueryMoleculeAromatizer::_aromatizeBondsFuzzy(QueryMolecule& mol, const AromaticityOptions& options)
{
    bool aromatized = false;
    QueryMoleculeAromatizer aromatizer(mol, options);

    aromatizer.setMode(QueryMoleculeAromatizer::FUZZY);
    aromatizer.precalculatePiLabels();
    aromatizer.aromatize();

    mol.aromaticity.clear();
    for (int e_idx = mol.edgeBegin(); e_idx < mol.edgeEnd(); e_idx = mol.edgeNext(e_idx))
    {
        bool aromatic_constraint = mol.getBond(e_idx).possibleValue(QueryMolecule::BOND_ORDER, BOND_AROMATIC);
        if (aromatic_constraint || aromatizer.isBondAromatic(e_idx))
        {
            mol.aromaticity.setCanBeAromatic(e_idx, true);
            aromatized = true;
        }
    }
    return aromatized;
}

void MoleculeAromatizer::findAromaticAtoms(BaseMolecule& mol, Array<int>* atoms, Array<int>* bonds, const AromaticityOptions& options)
{
    std::unique_ptr<BaseMolecule> clone;
    QS_DEF(Array<int>, mapping);

    clone.reset(mol.neu());
    mapping.clear();

    if (atoms != 0)
    {
        atoms->clear_resize(mol.vertexEnd());
        atoms->zerofill();
    }

    if (bonds != 0)
    {
        bonds->clear_resize(mol.edgeEnd());
        bonds->zerofill();
    }

    clone->clone(mol, &mapping, 0);

    clone->aromatize(options);

    for (int i = clone->edgeBegin(); i != clone->edgeEnd(); i = clone->edgeNext(i))
    {
        if (clone->getBondOrder(i) == BOND_AROMATIC)
        {
            const Edge& edge = clone->getEdge(i);

            if (atoms != 0)
            {
                atoms->at(mapping[edge.beg]) = 1;
                atoms->at(mapping[edge.end]) = 1;
            }

            if (bonds != 0)
                bonds->at(mol.findEdgeIndex(mapping[edge.beg], mapping[edge.end])) = 1;
        }
    }
}

//
// QueryMoleculeAromaticity
//

bool QueryMoleculeAromaticity::canBeAromatic(int edge_index) const
{
    if (edge_index >= can_bond_be_aromatic.size())
        return false;
    return can_bond_be_aromatic[edge_index];
}

void QueryMoleculeAromaticity::setCanBeAromatic(int edge_index, bool state)
{
    if (state == false && edge_index >= can_bond_be_aromatic.size())
        return;

    can_bond_be_aromatic.expandFill(edge_index + 1, false);
    can_bond_be_aromatic[edge_index] = state;
}

void QueryMoleculeAromaticity::clear()
{
    can_bond_be_aromatic.clear();
}
