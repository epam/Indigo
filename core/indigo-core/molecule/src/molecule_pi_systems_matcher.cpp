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

#include "molecule/molecule_pi_systems_matcher.h"

#include "graph/filter.h"
#include "graph/spanning_tree.h"
#include "molecule/elements.h"

using namespace indigo;

IMPL_ERROR(MoleculePiSystemsMatcher, "Pi-system matcher");

CP_DEF(MoleculePiSystemsMatcher);

MoleculePiSystemsMatcher::MoleculePiSystemsMatcher(Molecule& target)
    : _target(target), CP_INIT, TL_CP_GET(_atom_pi_system_idx), TL_CP_GET(_pi_systems), TL_CP_GET(_connectivity)
{
    _calcConnectivity(_target, _connectivity);

    _atom_pi_system_idx.clear_resize(target.vertexEnd());
    int n_pi_systems = _initMarks();
    _pi_systems.clear();
    _pi_systems.resize(n_pi_systems);
}

void MoleculePiSystemsMatcher::_calcConnectivity(Molecule& mol, Array<int>& conn)
{
    conn.clear_resize(mol.vertexEnd());
    conn.zerofill();
    for (int e = mol.edgeBegin(); e != mol.edgeEnd(); e = mol.edgeNext(e))
    {
        int bond_order = mol.getBondOrder(e);

        const Edge& edge = mol.getEdge(e);
        conn[edge.beg] += bond_order;
        conn[edge.end] += bond_order;
    }
    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
        if (!mol.isPseudoAtom(v) && !mol.isRSite(v) && !mol.isTemplateAtom(v))
            conn[v] += mol.getImplicitH(v);
}

int MoleculePiSystemsMatcher::_initMarks(void)
{
    _markAtomsFirst();

    // Decompose molecule into pi-systems
    Filter filter(_atom_pi_system_idx.ptr(), Filter::NEQ, _NOT_IN_PI_SYSTEM);

    // Decompose 'pi_systems' into connected components
    _decomposer.create(_target);
    int n_comp = _decomposer->decompose(&filter);

    // Copy pi-system indices
    _copyPiSystemsIdFromDecomposer();

    QS_DEF(Array<bool>, pi_system_used);
    pi_system_used.clear_resize(n_comp);
    for (int i = 0; i < n_comp; i++)
        pi_system_used[i] = false;

    _markUnstablePiSystems(pi_system_used);

    bool use_any_pi_system = false;
    for (int i = 0; i < n_comp; i++)
    {
        use_any_pi_system |= pi_system_used[i];
        if (use_any_pi_system)
            break;
    }
    _markVerticesInUnusedPiSystems(pi_system_used);
    if (!use_any_pi_system)
        return 0;

    _markVerticesInSingleAtomPiSystem(n_comp);
    _markVerticesInPiSystemsWithCycles();

    // Decompose again because cycles could split pi-system
    n_comp = _decomposer->decompose(&filter);

    _copyPiSystemsIdFromDecomposer();
    _markVerticesInSingleAtomPiSystem(n_comp);

    return n_comp;
}

void MoleculePiSystemsMatcher::_calculatePiSystemsSizes(int n_pi_systems, Array<int>& sizes)
{
    sizes.clear_resize(n_pi_systems);
    sizes.zerofill();

    // Collect sizes
    for (int v = _target.vertexBegin(); v != _target.vertexEnd(); v = _target.vertexNext(v))
    {
        int p = _atom_pi_system_idx[v];
        if (p != _NOT_IN_PI_SYSTEM)
            sizes[p]++;
    }
}

void MoleculePiSystemsMatcher::_markVerticesInSingleAtomPiSystem(int n_pi_systems)
{
    QS_DEF(Array<int>, pi_system_size);
    _calculatePiSystemsSizes(n_pi_systems, pi_system_size);

    // Exclude single atoms
    for (int v = _target.vertexBegin(); v != _target.vertexEnd(); v = _target.vertexNext(v))
    {
        int p = _atom_pi_system_idx[v];
        if (p != _NOT_IN_PI_SYSTEM && pi_system_size[p] == 1)
            _atom_pi_system_idx[v] = _NOT_IN_PI_SYSTEM;
    }
}

void MoleculePiSystemsMatcher::_copyPiSystemsIdFromDecomposer()
{
    const Array<int>& pi_system_per_vertex = _decomposer->getDecomposition();
    for (int v = _target.vertexBegin(); v != _target.vertexEnd(); v = _target.vertexNext(v))
    {
        int pi_system = pi_system_per_vertex[v];
        if (pi_system == -1)
            pi_system = _NOT_IN_PI_SYSTEM;

        _atom_pi_system_idx[v] = pi_system;
    }
}

void MoleculePiSystemsMatcher::_markUnstablePiSystems(Array<bool>& pi_system_used)
{
    // Mark pi-systems that contain atoms with partail octet or charge
    for (int v = _target.vertexBegin(); v != _target.vertexEnd(); v = _target.vertexNext(v))
    {
        int pi_system = _atom_pi_system_idx[v];
        if (pi_system < 0)
            continue;
        if (_connectivity[v] == -1)
            continue; // This means that atom is in aromatic ring

        if (_target.getAtomCharge(v) != 0 || _target.getAtomRadical(v) != 0)
            pi_system_used[pi_system] = true;

        int lonepairs;
        int vac = _target.getVacantPiOrbitals(v, _connectivity[v], &lonepairs);
        if (vac != 0)
            pi_system_used[pi_system] = true;
    }
}

void MoleculePiSystemsMatcher::_markVerticesInUnusedPiSystems(Array<bool>& pi_system_used)
{
    for (int v = _target.vertexBegin(); v != _target.vertexEnd(); v = _target.vertexNext(v))
    {
        int pi_system = _atom_pi_system_idx[v];
        if (pi_system == _NOT_IN_PI_SYSTEM)
            continue;

        if (!pi_system_used[pi_system])
            _atom_pi_system_idx[v] = _NOT_IN_PI_SYSTEM;
    }
}

void MoleculePiSystemsMatcher::_markVerticesInPiSystemsWithCycles()
{
    // Developed algorithm doesn't operate with pi-systems with cycles
    Filter filter(_atom_pi_system_idx.ptr(), Filter::NEQ, _NOT_IN_PI_SYSTEM);
    SpanningTree sp_tree(_target, &filter);

    QS_DEF(Array<int>, edge_in_cycle);
    edge_in_cycle.clear_resize(_target.edgeEnd());
    edge_in_cycle.zerofill();
    sp_tree.markAllEdgesInCycles(edge_in_cycle.ptr(), 1);

    for (int e = _target.edgeBegin(); e != _target.edgeEnd(); e = _target.edgeNext(e))
    {
        if (edge_in_cycle[e])
        {
            const Edge& edge = _target.getEdge(e);

            _atom_pi_system_idx[edge.beg] = _NOT_IN_PI_SYSTEM;
            _atom_pi_system_idx[edge.end] = _NOT_IN_PI_SYSTEM;
        }
    }
}

void MoleculePiSystemsMatcher::_markAtomsFirst()
{
    for (int i = 0; i < _atom_pi_system_idx.size(); i++)
        _atom_pi_system_idx[i] = _UNKNOWN;

    // Aromatic bonds matches by aromaticity matcher
    for (int e = _target.edgeBegin(); e != _target.edgeEnd(); e = _target.edgeNext(e))
    {
        if (_target.getBondOrder(e) == BOND_AROMATIC)
        {
            const Edge& edge = _target.getEdge(e);
            _atom_pi_system_idx[edge.beg] = _IN_AROMATIC;
            _atom_pi_system_idx[edge.end] = _IN_AROMATIC;

            _connectivity[edge.beg] = -1;
            _connectivity[edge.end] = -1;
        }
    }

    // Mark atoms that can be in pi system
    for (int v = _target.vertexBegin(); v != _target.vertexEnd(); v = _target.vertexNext(v))
    {
        if (_atom_pi_system_idx[v] == _NOT_IN_PI_SYSTEM)
            continue;
        // Atoms in aromatic rings will be exclused later
        // because they are in rings
        if (_atom_pi_system_idx[v] == _IN_AROMATIC)
            continue;

        if (!_canAtomBeInPiSystem(v))
            _atom_pi_system_idx[v] = _NOT_IN_PI_SYSTEM;
    }
}

bool MoleculePiSystemsMatcher::_canAtomBeInPiSystem(int v)
{
    if (_target.isPseudoAtom(v) || _target.isRSite(v) || _target.isTemplateAtom(v))
        return false;

    int label = _target.getAtomNumber(v);
    if (!Element::canBeAromatic(label))
        return false;
    if (label == ELEM_Pb || label == ELEM_Tl)
        return false; // Skip this elements because of lonepairs behavior

    // Check if d-orbital is used (not supported now)
    int electrons = Element::electrons(label, _target.getAtomCharge(v));
    int conn = _connectivity[v];
    int radical = _target.getAtomRadical(v);
    int used = electrons + conn + 2 * Element::radicalOrbitals(radical);
    if (used > 8 || conn > 4)
        return false;

    int skeleton_conn = _target.getVertex(v).degree() + _target.getImplicitH(v);
    if (skeleton_conn == conn && conn == 4)
        return false;

    // Check if atom is correct: charge corresponds to connectivity
    // For examples there are molecules like Cl-Ga(v2)-Cl, that are incorrect
    // but stored in database
    int total_electrons = electrons + conn + Element::radicalElectrons(radical);
    if (total_electrons % 2 != 0)
        return false;
    return true;
}

bool MoleculePiSystemsMatcher::isAtomInPiSystem(int atom)
{
    return _atom_pi_system_idx[atom] != _NOT_IN_PI_SYSTEM;
}

bool MoleculePiSystemsMatcher::isBondInPiSystem(int bond)
{
    const Edge& edge = _target.getEdge(bond);
    int p1 = _atom_pi_system_idx[edge.beg];
    int p2 = _atom_pi_system_idx[edge.end];
    if (p1 != p2)
        return false;
    return p1 != _NOT_IN_PI_SYSTEM;
}

void MoleculePiSystemsMatcher::_extractPiSystem(int pi_system_index)
{
    _Pi_System& pi_system = _pi_systems[pi_system_index];
    pi_system.initialized = true;

    Molecule& ps = pi_system.pi_system;
    Filter filt(_decomposer->getDecomposition().ptr(), Filter::EQ, pi_system_index);
    ps.makeSubmolecule(_target, filt, &pi_system.mapping, &pi_system.inv_mapping);

    // Replace bonds outside pi-system to implicit hydrogens
    QS_DEF(Array<int>, conn);
    _calcConnectivity(ps, conn);
    for (int v = ps.vertexBegin(); v != ps.vertexEnd(); v = ps.vertexNext(v))
    {
        int original_v = pi_system.mapping[v];
        int delta = _connectivity[original_v] - conn[v];
        if (delta > 0)
            ps.setImplicitH(v, ps.getImplicitH(v) + delta);
    }

    pi_system.localizations.clear();
    pi_system.localizer.create(pi_system.pi_system);
    _findPiSystemLocalization(pi_system_index);
}

void MoleculePiSystemsMatcher::_findPiSystemLocalization(int pi_system_index)
{
    // This function should be implemented outside this class
    // for finding pi-systems localizations during databse index
    // creation.

    _Pi_System& pi_system = _pi_systems[pi_system_index];

    // Calculate number of electrons and electrons for zero-charge
    int electrons = 0, zc_electrons = 0, octet_electron_pairs = 0;

    Molecule& mol = pi_system.pi_system;
    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
    {
        int degree = mol.getVertex(v).degree();
        int min_connectivity = mol.getImplicitH(v) + degree;

        int label = mol.getAtomNumber(v);
        electrons += Element::electrons(label, mol.getAtomCharge(v)) - min_connectivity;
        zc_electrons += std::max(Element::electrons(label, 0) - min_connectivity, 0);

        octet_electron_pairs += 4 - min_connectivity - Element::radicalOrbitals(mol.getAtomRadical(v));
    }

    // Find localizations
    if (electrons % 2 != 0)
        throw Error("Electrons number must be even");
    int electron_pairs = electrons / 2;
    int zc_lonepairs = pi_system.localizer->getZeroChargeLonepairs();

    int double_bonds = electron_pairs;
    if (double_bonds > octet_electron_pairs / 2)
        double_bonds = octet_electron_pairs / 2;

    bool was_found = false;

    int other_lp = 0, best_other_lp = -1;
    bool double_bonds_checks = false;
    int octet_charges_count = -1;
    while (double_bonds >= 0)
    {
        int zero_lp = electron_pairs - double_bonds - other_lp;
        if (zero_lp > zc_lonepairs)
        {
            other_lp = zero_lp - zc_lonepairs;
            zero_lp = zc_lonepairs;
        }
        if (zero_lp < 0)
        {
            other_lp = 0;
            double_bonds--;
            double_bonds_checks = false;
            continue;
        }

        if (best_other_lp != -1 && best_other_lp < other_lp)
        {
            // Such localization has more atoms with
            // charges then in previous localization
            break;
        }

        pi_system.localizer->setParameters(double_bonds, zero_lp, other_lp);

        bool found;
        if (!double_bonds_checks)
        {
            found = pi_system.localizer->localize(true);
            if (!found)
            {
                // Such number of double bonds can't be
                other_lp = 0;
                double_bonds--;
                continue;
            }
            double_bonds_checks = true;
        }

        found = pi_system.localizer->localize();

        if (found)
        {
            bool octet = pi_system.localizer->isAllAtomsHaveOctet();
            int charges_count = pi_system.localizer->getLocalizationChargesCount();

            if (octet)
                octet_charges_count = charges_count;
            else
            {
                if (octet_charges_count != -1 && octet_charges_count <= charges_count)
                    break;
            }

            best_other_lp = other_lp;
            was_found = true;
        }
        else
        {
            other_lp++;
            continue;
        }

        // Add localization
        _Pi_System::Localizations& loc = pi_system.localizations.push();
        loc.double_bonds = double_bonds;
        loc.primary_lp = zero_lp;
        loc.seconary_lp = other_lp;

        if (zero_lp == zc_lonepairs)
            break;

        other_lp = 0;
        double_bonds--;
        double_bonds_checks = false;
    }
}

bool MoleculePiSystemsMatcher::checkEmbedding(QueryMolecule& query, const int* mapping)
{
    for (int i = 0; i < _pi_systems.size(); i++)
        _pi_systems[i].pi_system_mapped = false;

    _markMappedPiSystems(query, mapping);

    bool ret;

    // Fix bonds
    ret = _fixBonds(query, mapping);
    if (!ret)
        return false;

    // Fix charges
    ret = _fixAtoms(query, mapping);
    if (!ret)
        return false;

    return _findMatching();
}

bool MoleculePiSystemsMatcher::_findMatching()
{
    for (int i = 0; i < _pi_systems.size(); i++)
    {
        if (!_pi_systems[i].initialized)
            continue;

        _Pi_System& pi_system = _pi_systems[i];
        if (!pi_system.pi_system_mapped)
            continue;

        if (!_findMatchingForPiSystem(i))
            return false;
    }

    return true;
}

bool MoleculePiSystemsMatcher::_findMatchingForPiSystem(int pi_system_index)
{
    _Pi_System& pi_system = _pi_systems[pi_system_index];

    for (int i = 0; i < pi_system.localizations.size(); i++)
    {
        _Pi_System::Localizations& loc = pi_system.localizations[i];
        pi_system.localizer->setParameters(loc.double_bonds, loc.primary_lp, loc.seconary_lp);

        if (pi_system.localizer->localize())
            return true;
    }

    return false;
}

void MoleculePiSystemsMatcher::_markMappedPiSystems(QueryMolecule& query, const int* mapping)
{
    for (int qv = query.vertexBegin(); qv != query.vertexEnd(); qv = query.vertexNext(qv))
    {
        int v = mapping[qv];

        if (v < 0)
            continue; // Such vertex must be ignored

        int pi_system_idx = _atom_pi_system_idx[v];
        if (pi_system_idx == _NOT_IN_PI_SYSTEM)
            continue;

        if (!_pi_systems[pi_system_idx].initialized)
            _extractPiSystem(pi_system_idx);

        _Pi_System& pi_system = _pi_systems[pi_system_idx];
        if (!pi_system.pi_system_mapped)
        {
            pi_system.pi_system_mapped = true;
            pi_system.localizer->unfixAll();
        }
    }
}

bool MoleculePiSystemsMatcher::_fixAtoms(QueryMolecule& query, const int* mapping)
{
    // Fix charges
    for (int qv = query.vertexBegin(); qv != query.vertexEnd(); qv = query.vertexNext(qv))
    {
        int v = mapping[qv];
        if (v < 0)
            continue; // Such vertex must be ignored

        int pi_system_idx = _atom_pi_system_idx[v];
        if (pi_system_idx == _NOT_IN_PI_SYSTEM)
            continue;

        _Pi_System& pi_system = _pi_systems[pi_system_idx];

        QueryMolecule::Atom& qatom = query.getAtom(qv);
        int pv = pi_system.inv_mapping[v];

        int charge = query.getAtomCharge(qv);
        if (charge != CHARGE_UNKNOWN)
        {
            bool ret = pi_system.localizer->fixAtomCharge(pv, charge);
            if (!ret)
                return false;
        }
        else if (qatom.hasConstraint(QueryMolecule::ATOM_CHARGE))
            throw Error("Unsupported atom charge specified");

        int valence = query.getExplicitValence(qv);
        if (valence != -1)
        {
            bool ret = pi_system.localizer->fixAtomConnectivity(pv, valence);
            if (!ret)
                return false;
        }
        else if (qatom.hasConstraint(QueryMolecule::ATOM_VALENCE))
            throw Error("Unsupported atom charge specified");
    }
    return true;
}

bool MoleculePiSystemsMatcher::_fixBonds(QueryMolecule& query, const int* mapping)
{
    for (int e = query.edgeBegin(); e != query.edgeEnd(); e = query.edgeNext(e))
    {
        const Edge& query_edge = query.getEdge(e);
        if (mapping[query_edge.beg] < 0 || mapping[query_edge.end] < 0)
            continue; // Edges connected with ignored vertices

        int target_edge = Graph::findMappedEdge(query, _target, e, mapping);
        const Edge& edge = _target.getEdge(target_edge);

        int p1_idx = _atom_pi_system_idx[edge.beg];
        int p2_idx = _atom_pi_system_idx[edge.end];
        if (p1_idx == _NOT_IN_PI_SYSTEM || p2_idx == _NOT_IN_PI_SYSTEM || p1_idx != p2_idx)
            continue;

        if (!_pi_systems[p1_idx].initialized)
            throw Error("pi-system must be initialized here");

        _Pi_System& pi_system = _pi_systems[p1_idx];

        int pi_sys_edge = Graph::findMappedEdge(_target, pi_system.pi_system, target_edge, pi_system.inv_mapping.ptr());

        // Get target topology
        int topology = _target.getBondTopology(target_edge);

        QueryMolecule::Bond& qbond = query.getBond(e);

        bool can_be_single = qbond.possibleValuePair(QueryMolecule::BOND_ORDER, BOND_SINGLE, QueryMolecule::BOND_TOPOLOGY, topology);
        bool can_be_double = qbond.possibleValuePair(QueryMolecule::BOND_ORDER, BOND_DOUBLE, QueryMolecule::BOND_TOPOLOGY, topology);
        bool can_be_triple = qbond.possibleValuePair(QueryMolecule::BOND_ORDER, BOND_TRIPLE, QueryMolecule::BOND_TOPOLOGY, topology);

        if (!can_be_single && !can_be_double && !can_be_triple)
            return false;
        if (can_be_single && can_be_double && can_be_triple)
            continue;

        bool ret = false; // initializing to avoid compiler warning
        if (can_be_single && can_be_double)
            // Here can_be_triple = false because of previous check
            ret = pi_system.localizer->fixBondSingleDouble(pi_sys_edge);
        else
        {
            if (can_be_triple)
            {
                if (can_be_single)
                    throw Error("Unsupported bond order specified (can be single or triple)");
                else if (can_be_double)
                    throw Error("Unsupported bond order specified (can be double or triple)");
                ret = pi_system.localizer->fixBond(pi_sys_edge, BOND_TRIPLE);
            }
            if (can_be_single)
                ret = pi_system.localizer->fixBond(pi_sys_edge, BOND_SINGLE);
            if (can_be_double)
                ret = pi_system.localizer->fixBond(pi_sys_edge, BOND_DOUBLE);
        }

        if (!ret)
            return false;
    }

    return true;
}

void MoleculePiSystemsMatcher::copyLocalization(Molecule& target)
{
    for (int i = 0; i < _pi_systems.size(); i++)
    {
        if (!_pi_systems[i].initialized)
            continue;

        _Pi_System& system = _pi_systems[i];
        system.localizer->copyBondsAndCharges(target, system.mapping);
    }
}

void MoleculePiSystemsMatcher::_Pi_System::clear()
{
    initialized = false;
    localizer.free();
    pi_system.clear();
    inv_mapping.clear();
    mapping.clear();
    localizations.clear();
}
