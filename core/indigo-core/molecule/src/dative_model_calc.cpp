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

#include "molecule/dative_model.h"

#include <algorithm>

#include "base_cpp/exception.h"
#include "graph/graph.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_dearom.h"

using namespace indigo;

namespace
{
    bool isComputableAtom(Molecule& mol, int atom_idx)
    {
        return !mol.isPseudoAtom(atom_idx) && !mol.isRSite(atom_idx) && !mol.isTemplateAtom(atom_idx);
    }

    // rounddown(value / 2) of the specification.
    int roundDownHalf(int value)
    {
        return value >= 0 ? value / 2 : -(((-value) + 1) / 2);
    }

    // roundup(value / 2) of the specification.
    int roundUpHalf(int value)
    {
        return value >= 0 ? (value + 1) / 2 : -((-value) / 2);
    }
}

DativeModel::DativeModel(Molecule& mol) : _mol(mol)
{
    for (int i = _mol.edgeBegin(); i != _mol.edgeEnd(); i = _mol.edgeNext(i))
        if (_mol.getBondOrder(i) == BOND_AROMATIC)
        {
            _has_aromatic = true;
            break;
        }

    if (!_has_aromatic)
        return;

    // Requirement 1.1: aromatic atoms are evaluated on the dearomatized structure.
    // The connectivity is obtained without touching the molecule — see
    // MoleculeDearomatizer::calculateDearomatizedConnectivity.
    try
    {
        AromaticityOptions options;
        // Fused systems may admit several Kekule structures; taking the dearomatizer's
        // choice keeps the calculation total instead of throwing on such molecules.
        options.unique_dearomatization = false;

        // A group that could not be dearomatized leaves its atoms at connectivity zero,
        // which would understate `bonds` and inflate El and Or. Declining to compute is
        // the honest answer: the caller keeps the existing behaviour instead of being
        // told about a valence error that was derived from missing data.
        _usable = MoleculeDearomatizer::calculateDearomatizedConnectivity(_mol, options, _kekule_connectivity);
    }
    catch (Exception&)
    {
        // Dearomatization failed: degrade gracefully instead of surfacing a
        // kekulization error out of what the caller believes is a valence query.
        _usable = false;
    }
}

int DativeModel::_countBonds(int atom_idx) const
{
    const Vertex& vertex = _mol.getVertex(atom_idx);
    bool aromatic = false;
    int bonds = 0;

    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        const int order = _mol.getBondOrder(vertex.neiEdge(i));

        if (order == BOND_AROMATIC)
        {
            aromatic = true;
            continue;
        }

        // Dative and hydrogen bonds contribute no shared pair. Implicit hydrogens are
        // excluded by construction: they are not edges of the graph.
        if (order < 0 || isNonValenceBond(order))
            continue;

        bonds += order;
    }

    // For an aromatic atom the dearomatized connectivity already covers both its
    // aromatic and non-aromatic bonds, so it replaces the sum computed above.
    if (aromatic && atom_idx < _kekule_connectivity.size())
        return _kekule_connectivity[atom_idx];

    return bonds;
}

bool DativeModel::compute(int atom_idx, AtomResult& out) const
{
    out = AtomResult();

    if (!_usable || !isComputableAtom(_mol, atom_idx))
        return false;

    const int elem = _mol.getAtomNumber(atom_idx);
    const int el0 = dative::valenceElectrons(elem);
    const int or0 = dative::valenceOrbitals(elem);

    if (el0 == 0 && or0 == 0)
        return false; // element outside the table

    const int charge = _mol.getAtomCharge(atom_idx);
    // A radical is an explicit property of the atom here. It is never inferred, both
    // because the specification treats it as given and because inferring it would
    // re-enter the valence computation this model can be called from.
    const int radicals = Element::radicalElectrons(_mol.getStoredRadical(atom_idx));
    const int bonds = _countBonds(atom_idx);

    // Requirements 1 and 2.
    out.el = el0 - charge - bonds - radicals;
    out.orb = or0 - bonds - radicals;

    // Requirement 3: a donor bond and an acceptor bond on the same atom cancel out.
    int donors = 0, acceptors = 0;
    const Vertex& vertex = _mol.getVertex(atom_idx);
    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        const int edge_idx = vertex.neiEdge(i);
        if (_mol.getBondOrder(edge_idx) != _BOND_COORDINATION)
            continue;

        // The arrow runs from donor to acceptor and the graph preserves that order:
        // getEdge().beg is the donor side, getEdge().end the acceptor side.
        if (_mol.getEdge(edge_idx).beg == atom_idx)
            donors++;
        else
            acceptors++;
    }

    const int cancelled = std::min(donors, acceptors);
    out.donor_bonds = donors - cancelled;
    out.acceptor_bonds = acceptors - cancelled;

    // Requirements 4 and 5. The `Or >= n > 0` gates also absorb the hypervalent case
    // where Or goes negative (Q2, won't-fix): both gates fail, so such an atom can hold
    // no dative bond at all and requirement 6 reports it. Do not clamp Or.
    const int n_donor = roundDownHalf(out.el);
    if (out.orb >= n_donor && n_donor > 0)
        out.max_donor = n_donor;

    const int n_acceptor = out.orb - roundUpHalf(out.el);
    if (out.orb >= n_acceptor && n_acceptor > 0)
        out.max_acceptor = n_acceptor;

    // Requirement 6.
    out.valence_error = out.donor_bonds > out.max_donor || out.acceptor_bonds > out.max_acceptor;

    // Requirements 7 and 8.
    out.implicit_h = _implicitHydrogens(elem, out);

    return true;
}

int DativeModel::_implicitHydrogens(int elem, const AtomResult& result)
{
    // Requirement 8. Both sets already answer "no hydrogens" in the existing valence
    // code -- transition metals are accepted exactly as drawn, noble gases are held at
    // zero -- so declining here leaves that code in charge instead of restating its
    // conclusion in a second place.
    if (isTransitionMetal(elem) || Element::group(elem) == 8)
        return -1;

    // An atom that cannot hold the dative bonds drawn on it has no meaningful remainder
    // to derive hydrogens from. The specification stops at the valence error for such
    // atoms, and so do we.
    if (result.valence_error)
        return -1;

    const int el_remaining = result.el - 2 * result.donor_bonds;
    const int or_remaining = result.orb - result.donor_bonds - result.acceptor_bonds;

    // Neither can go negative in the absence of a valence error: donor_bonds is bounded
    // by rounddown(El/2), and cancellation (req 3) leaves at most one of the two counts
    // non-zero, each bounded by Or. The guard is here because a negative hydrogen count
    // would only be noticed much further away from its cause.
    if (el_remaining < 0 || or_remaining < 0)
        return -1;

    if (el_remaining <= or_remaining)
        return el_remaining;

    if (el_remaining < 2 * or_remaining)
        return 2 * or_remaining - el_remaining;

    return 0;
}
