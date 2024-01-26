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

#include "molecule/molecule.h"

#include "base_c/defs.h"
#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_dearom.h"
#include "molecule/molecule_standardize.h"
#include "molecule/monomer_commons.h"

using namespace indigo;

Molecule::Molecule()
{
    _aromatized = false;
    _ignore_bad_valence = false;
}

Molecule& Molecule::asMolecule()
{
    return *this;
}

void Molecule::clear()
{
    BaseMolecule::clear();

    _pseudo_atom_values.clear();
    _atoms.clear();
    _bond_orders.clear();
    _connectivity.clear();
    _aromaticity.clear();
    _implicit_h.clear();
    _total_h.clear();
    _valence.clear();
    _radicals.clear();
    _template_occurrences.clear();
    _template_names.clear();
    _template_classes.clear();

    _aromatized = false;
    _ignore_bad_valence = false;
    updateEditRevision();
}

void Molecule::_flipBond(int atom_parent, int atom_from, int atom_to)
{
    int src_bond_idx = findEdgeIndex(atom_parent, atom_from);
    int bond_order = getBondOrder(src_bond_idx);

    addBond(atom_parent, atom_to, bond_order);
    updateEditRevision();
}

void Molecule::_mergeWithSubmolecule(BaseMolecule& bmol, const Array<int>& vertices, const Array<int>* edges, const Array<int>& mapping, int skip_flags)
{
    Molecule& mol = bmol.asMolecule();
    _ignore_bad_valence = mol.getIgnoreBadValenceFlag();

    int i;

    // atoms and pseudo-atoms and connectivities and implicit H counters
    for (i = 0; i < vertices.size(); i++)
    {
        int newidx = mapping[vertices[i]];

        _atoms.expand(newidx + 1);
        _atoms[newidx] = mol._atoms[vertices[i]];

        if (mol.isPseudoAtom(vertices[i]))
            setPseudoAtom(newidx, mol.getPseudoAtom(vertices[i]));

        if (mol.isTemplateAtom(vertices[i]))
        {
            setTemplateAtom(newidx, mol.getTemplateAtom(vertices[i]));
            setTemplateAtomClass(newidx, mol.getTemplateAtomClass(vertices[i]));
            setTemplateAtomSeqid(newidx, mol.getTemplateAtomSeqid(vertices[i]));
            setTemplateAtomTemplateIndex(newidx, mol.getTemplateAtomTemplateIndex(vertices[i]));
        }

        bool nei_mapped = (getVertex(newidx).degree() == mol.getVertex(vertices[i]).degree());

        if (mol._connectivity.size() > vertices[i])
        {
            _connectivity.expandFill(newidx + 1, -1);
            if (nei_mapped)
                _connectivity[newidx] = mol._connectivity[vertices[i]];
        }

        if (mol._valence.size() > vertices[i])
        {
            _valence.expandFill(newidx + 1, -1);
            if (nei_mapped)
                _valence[newidx] = mol._valence[vertices[i]];
        }
        if (mol._implicit_h.size() > vertices[i])
        {
            _implicit_h.expandFill(newidx + 1, -1);
            if (nei_mapped)
                _implicit_h[newidx] = mol._implicit_h[vertices[i]];
        }
        if (mol._radicals.size() > vertices[i])
        {
            _radicals.expandFill(newidx + 1, -1);
            if (nei_mapped)
                _radicals[newidx] = mol._radicals[vertices[i]];
        }
    }

    // bonds
    if (edges != 0)
        for (i = 0; i < edges->size(); i++)
        {
            const Edge& edge = mol.getEdge(edges->at(i));
            int beg = mapping[edge.beg];
            int end = mapping[edge.end];

            if (beg == -1 || end == -1)
                // must have been thrown before in mergeWithSubgraph()
                throw Error("_mergeWithSubmolecule: internal");

            int idx = findEdgeIndex(beg, end);

            _bond_orders.expand(idx + 1);
            _bond_orders[idx] = mol._bond_orders[edges->at(i)];
        }
    else
        for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
        {
            const Edge& edge = mol.getEdge(i);
            int beg = mapping[edge.beg];
            int end = mapping[edge.end];

            if (beg == -1 || end == -1)
                continue;

            int idx = findEdgeIndex(beg, end);

            _bond_orders.expand(idx + 1);
            _bond_orders[idx] = mol._bond_orders[i];
        }

    _aromaticity.clear();
}

/*
void Molecule::setQueryBondAromatic (int idx)
{
   setBondAromatic(idx);
   _q_bonds->at(idx).type = 0;
   _q_bonds->at(idx).can_be_aromatic = false;
}

void Molecule::setQueryBondFuzzyAromatic (int idx)
{
   _q_bonds->at(idx).can_be_aromatic = true;
}

*/

void Molecule::_validateVertexConnectivity(int idx, bool validate)
{
    if (validate)
    {
        getAtomConnectivity_noImplH(idx);
        getImplicitH_NoThrow(idx, -1);
        getAtomValence_NoThrow(idx, -1);
    }
    else
    {
        if (_connectivity.size() > idx)
            _connectivity[idx] = -1;
        if (_implicit_h.size() > idx)
        {
            _atoms[idx].explicit_impl_h = false;
            _implicit_h[idx] = -1;
        }
        if (_total_h.size() > idx)
            _total_h[idx] = -1;
        if (_valence.size() > idx)
        {
            _atoms[idx].explicit_valence = false;
            _valence[idx] = -1;
        }
        if (_radicals.size() > idx)
        {
            _radicals[idx] = -1;
        }
    }
    updateEditRevision();
}

void Molecule::_invalidateVertexCache(int idx)
{
    if (!isExplicitValenceSet(idx) && _valence.size() > idx)
        _valence[idx] = -1;
    if (!isImplicitHSet(idx) && _implicit_h.size() > idx)
        _implicit_h[idx] = -1;
    if (_total_h.size() > idx)
        _total_h[idx] = -1;
}

void Molecule::setBondOrder(int idx, int order, bool keep_connectivity)
{
    const Edge& edge = getEdge(idx);

    // if (_atoms[edge.beg].explicit_valence || _atoms[edge.end].explicit_valence)
    //   throw Error("setBondOrder(): explicit valence on atom");

    // if (_atoms[edge.beg].explicit_impl_h || _atoms[edge.end].explicit_impl_h)
    //   throw Error("setBondOrder(): explicit H count on atom");

    if (keep_connectivity && _bond_orders[idx] != BOND_AROMATIC && order != BOND_AROMATIC)
        throw Error("setBondOrder(): keep_connectivity must be used only with aromatic bonds");

    // If connectivity should be kept then calculate connectivity and
    // all dependent constants (valence, implicit hydrogens)
    _validateVertexConnectivity(edge.beg, keep_connectivity);
    _validateVertexConnectivity(edge.end, keep_connectivity);

    if (_bond_orders[idx] == BOND_AROMATIC || order == BOND_AROMATIC)
        _aromaticity.clear();

    _bond_orders[idx] = order;

    if (order != BOND_DOUBLE)
        cis_trans.setParity(idx, 0);

    _aromatized = false;
    updateEditRevision();
}

void Molecule::setBondOrder_Silent(int idx, int order)
{
    _bond_orders[idx] = order;
    updateEditRevision();
}

void Molecule::setAtomCharge(int idx, int charge)
{
    _atoms[idx].charge = charge;
    if (_implicit_h.size() > idx)
        _implicit_h[idx] = -1;
    if (_total_h.size() > idx)
        _total_h[idx] = -1;
    if (_radicals.size() > idx)
        _radicals[idx] = -1;
    updateEditRevision();
}

void Molecule::setAtomCharge_Silent(int idx, int charge)
{
    _atoms[idx].charge = charge;
    updateEditRevision();
}

void Molecule::setAtomIsotope(int idx, int isotope)
{
    _atoms[idx].isotope = isotope;
    updateEditRevision();
}

void Molecule::setAtomRadical(int idx, int radical)
{
    _radicals.expandFill(idx + 1, -1);
    _radicals[idx] = radical;
    _invalidateVertexCache(idx);
    updateEditRevision();
}

void Molecule::setExplicitValence(int idx, int valence)
{
    _valence.expandFill(idx + 1, -1);
    _valence[idx] = valence;
    _atoms[idx].explicit_valence = true;
    _invalidateVertexCache(idx);
    updateEditRevision();
}

void Molecule::resetExplicitValence(int idx)
{
    if (_valence.size() > idx)
        _valence[idx] = -1;
    _atoms[idx].explicit_valence = false;
    _invalidateVertexCache(idx);
    updateEditRevision();
}

bool Molecule::isExplicitValenceSet(int idx)
{
    return _atoms[idx].explicit_valence;
}

void Molecule::setValence(int idx, int valence)
{
    _valence.expandFill(idx + 1, -1);
    _valence[idx] = valence;
    updateEditRevision();
}

void Molecule::setImplicitH(int idx, int impl_h)
{
    _implicit_h.expandFill(idx + 1, -1);
    _implicit_h[idx] = impl_h;
    _atoms[idx].explicit_impl_h = true;
    _invalidateVertexCache(idx);
    updateEditRevision();
}

bool Molecule::isImplicitHSet(int idx)
{
    return _atoms[idx].explicit_impl_h;
}

void Molecule::setPseudoAtom(int idx, const char* text)
{
    _atoms[idx].number = ELEM_PSEUDO;
    _atoms[idx].pseudoatom_value_idx = _pseudo_atom_values.add(text);
    // TODO: take care of memory allocated here in _pseudo_atom_values
    updateEditRevision();
}

void Molecule::renameTemplateAtom(int idx, const char* text)
{
    auto occur_idx = _atoms[idx].template_occur_idx;
    if (_atoms[idx].number == ELEM_TEMPLATE)
    {
        _TemplateOccurrence& occur = _template_occurrences.at(occur_idx);
        _template_names.set(occur.name_idx, text);
        updateEditRevision();
    }
}

void Molecule::setTemplateAtom(int idx, const char* text)
{
    _atoms[idx].number = ELEM_TEMPLATE;
    _atoms[idx].template_occur_idx = _template_occurrences.add();
    _TemplateOccurrence& occur = _template_occurrences.at(_atoms[idx].template_occur_idx);
    occur.name_idx = _template_names.add(text);
    occur.seq_id = -1;
    occur.template_idx = -1;
    occur.contracted = DisplayOption::Undefined;
    updateEditRevision();
}

void Molecule::setTemplateAtomName(int idx, const char* text)
{
    if (_atoms[idx].number != ELEM_TEMPLATE)
        throw Error("setTemplateAtomClass(): atom #%d is not a template atom", idx);

    _TemplateOccurrence& occur = _template_occurrences.at(_atoms[idx].template_occur_idx);
    occur.name_idx = _template_names.add(text);
    updateEditRevision();
}

void Molecule::setTemplateAtomClass(int idx, const char* text)
{
    if (_atoms[idx].number != ELEM_TEMPLATE)
        throw Error("setTemplateAtomClass(): atom #%d is not a template atom", idx);

    _TemplateOccurrence& occur = _template_occurrences.at(_atoms[idx].template_occur_idx);
    occur.class_idx = _template_classes.add(text);
    updateEditRevision();
}

void Molecule::setTemplateAtomSeqid(int idx, int seq_id)
{
    if (_atoms[idx].number != ELEM_TEMPLATE)
        throw Error("setTemplateAtomSeqid(): atom #%d is not a template atom", idx);

    _TemplateOccurrence& occur = _template_occurrences.at(_atoms[idx].template_occur_idx);
    occur.seq_id = seq_id;
    updateEditRevision();
}

void Molecule::setTemplateAtomTemplateIndex(int idx, int temp_idx)
{
    if (_atoms[idx].number != ELEM_TEMPLATE)
        throw Error("setTemplateAtomTemplateIndex(): atom #%d is not a template atom", idx);

    _TemplateOccurrence& occur = _template_occurrences.at(_atoms[idx].template_occur_idx);
    occur.template_idx = temp_idx;
    updateEditRevision();
}

void Molecule::setTemplateAtomDisplayOption(int idx, int option)
{
    if (_atoms[idx].number != ELEM_TEMPLATE)
        throw Error("setTemplateAtomDisplayOption(): atom #%d is not a template atom", idx);

    _TemplateOccurrence& occur = _template_occurrences.at(_atoms[idx].template_occur_idx);
    occur.contracted = (DisplayOption)option;
    updateEditRevision();
}

int Molecule::getVacantPiOrbitals(int atom_idx, int conn, int* lonepairs_out)
{
    int group = Element::group(getAtomNumber(atom_idx));
    int charge = getAtomCharge(atom_idx);
    int radical = getAtomRadical(atom_idx);

    return BaseMolecule::getVacantPiOrbitals(group, charge, radical, conn, lonepairs_out);
}

int Molecule::getVacantPiOrbitals(int atom_idx, int* lonepairs_out)
{
    return getVacantPiOrbitals(atom_idx, getAtomConnectivity(atom_idx), lonepairs_out);
}

int Molecule::getAtomConnectivity(int idx)
{
    int conn = getAtomConnectivity_noImplH(idx);

    if (conn < 0)
        return -1;

    int impl_h = getImplicitH(idx);

    return impl_h + conn;
}

int Molecule::getAtomConnectivity_NoThrow(int idx, int fallback)
{
    try
    {
        return getAtomConnectivity(idx);
    }
    catch (Element::Error&)
    {
        return fallback;
    }
}

int Molecule::getAtomConnectivity_noImplH(int idx)
{
    if (_connectivity.size() > idx && _connectivity[idx] >= 0)
        return _connectivity[idx];

    int conn = calcAtomConnectivity_noImplH(idx);

    _connectivity.expandFill(idx + 1, -1);
    _connectivity[idx] = conn;
    return conn;
}

int Molecule::calcAtomConnectivity_noImplH(int idx)
{
    const Vertex& vertex = getVertex(idx);
    int i, conn = 0;

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int order = getBondOrder(vertex.neiEdge(i));

        if (order == BOND_AROMATIC)
            return -1;

        if (order == -1) // can happen on TautomerSuperStructure
            continue;

        if (order == _BOND_HYDROGEN || order == _BOND_COORDINATION)
            continue;

        conn += order;
    }

    for (i = 1; i <= attachmentPointCount(); i++)
    {
        int j = 0, aidx;

        for (j = 0; (aidx = getAttachmentPoint(i, j)) != -1; j++)
        {
            if (aidx == idx)
                conn++;
        }
    }

    return conn;
}

void Molecule::calcAromaticAtomConnectivity(int idx, int& n_arom, int& min_conn)
{
    const Vertex& vertex = getVertex(idx);
    int i;

    n_arom = 0;
    min_conn = 0;

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int order = getBondOrder(vertex.neiEdge(i));

        if (order == BOND_AROMATIC)
        {
            min_conn++;
            n_arom++;
        }
        else
            min_conn += order;
    }

    if (isImplicitHSet(idx))
        min_conn += getImplicitH(idx);
}

int Molecule::totalHydrogensCount()
{
    int i, total_h = 0;

    for (i = vertexBegin(); i < vertexEnd(); i = vertexNext(i))
    {
        if (getAtomNumber(i) == ELEM_H)
            total_h++;

        total_h += getImplicitH(i);
    }

    return total_h;
}

int Molecule::matchAtomsCmp(Graph& g1, Graph& g2, int idx1, int idx2, void* userdata)
{
    Molecule& m1 = ((BaseMolecule&)g1).asMolecule();
    Molecule& m2 = ((BaseMolecule&)g2).asMolecule();

    if (m1.isPseudoAtom(idx1) && !m2.isPseudoAtom(idx2))
        return 1;

    if (!m1.isPseudoAtom(idx1) && m2.isPseudoAtom(idx2))
        return -1;

    if (m1.isTemplateAtom(idx1) && !m2.isTemplateAtom(idx2))
        return 1;

    if (!m1.isTemplateAtom(idx1) && m2.isTemplateAtom(idx2))
        return -1;

    if (m1.isRSite(idx1) && !m2.isRSite(idx2))
        return 1;

    if (!m1.isRSite(idx1) && m2.isRSite(idx2))
        return -1;

    if (m1.isAtomHighlighted(idx1) && !m2.isAtomHighlighted(idx2))
        return 1;

    if (!m1.isAtomHighlighted(idx1) && m2.isAtomHighlighted(idx2))
        return -1;

    QS_DEF(Array<int>, ai1);
    QS_DEF(Array<int>, ai2);

    m1.getAttachmentIndicesForAtom(idx1, ai1);
    m2.getAttachmentIndicesForAtom(idx2, ai2);

    if (ai1.size() != ai2.size())
        return ai1.size() - ai2.size();

    int i;

    for (i = 0; i != ai1.size(); i++)
        if (ai1[i] != ai2[i])
            return ai1[i] - ai2[i];

    bool pseudo = false;

    if (m1.isRSite(idx1) && m2.isRSite(idx2))
    {
        int res = m2.getRSiteBits(idx2) - m1.getRSiteBits(idx1);
        if (res != 0)
            return res;
        pseudo = true;
    }
    if (m1.isPseudoAtom(idx1) && m2.isPseudoAtom(idx2))
    {
        int res = strcmp(m1.getPseudoAtom(idx1), m2.getPseudoAtom(idx2));

        if (res != 0)
            return res;
        pseudo = true;
    }
    else if (m1.isTemplateAtom(idx1) && m2.isTemplateAtom(idx2))
    {
        int res = strcmp(m1.getTemplateAtom(idx1), m2.getTemplateAtom(idx2));

        if (res != 0)
            return res;
        pseudo = true;
    }
    else
    {
        if (m1.getAtomNumber(idx1) > m2.getAtomNumber(idx2))
            return 1;
        if (m1.getAtomNumber(idx1) < m2.getAtomNumber(idx2))
            return -1;
    }

    if (m1.getAtomIsotope(idx1) > m2.getAtomIsotope(idx2))
        return 1;
    if (m1.getAtomIsotope(idx1) < m2.getAtomIsotope(idx2))
        return -1;

    if (m1.getAtomCharge(idx1) > m2.getAtomCharge(idx2))
        return 1;
    if (m1.getAtomCharge(idx1) < m2.getAtomCharge(idx2))
        return -1;

    if (!pseudo && m1.getAtomRadical(idx1) > m2.getAtomRadical(idx2))
        return 1;
    if (!pseudo && m1.getAtomRadical(idx1) < m2.getAtomRadical(idx2))
        return -1;

    return 0;
}

int Molecule::getAtomAromaticity(int idx)
{
    if (_aromaticity.size() > idx && _aromaticity[idx] >= 0)
        return _aromaticity[idx];

    const Vertex& vertex = getVertex(idx);
    int i;

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        int order = getBondOrder(vertex.neiEdge(i));

        if (order == BOND_AROMATIC)
        {
            _aromaticity.expandFill(idx + 1, -1);
            _aromaticity[idx] = ATOM_AROMATIC;
            return ATOM_AROMATIC;
        }

        // not checking order == -1 because it is not QueryMolecule
    }
    _aromaticity.expandFill(idx + 1, -1);
    _aromaticity[idx] = ATOM_ALIPHATIC;
    return ATOM_ALIPHATIC;
}

void Molecule::_removeAtoms(const Array<int>& indices, const int* mapping)
{
    int i;

    for (i = 0; i < indices.size(); i++)
    {
        int idx = indices[i];
        const Vertex& vertex = getVertex(idx);
        int j;

        for (j = vertex.neiBegin(); j != vertex.neiEnd(); j = vertex.neiNext(j))
        {
            int nei = vertex.neiVertex(j);
            int order = getBondOrder(vertex.neiEdge(j));

            if (mapping[nei] < 0) // the neighbor is marked for removal too
                continue;

            // Precalculate and store into cache number of implicit hydrogens
            // This is required for correct hydrogens unfolding for molecules
            // like [H]S([H])([H])C (that is seems to be invalid)
            if (!isRSite(nei) && !isPseudoAtom(nei) && !isTemplateAtom(nei))
                if (_implicit_h.size() <= nei || _implicit_h[nei] < 0)
                    getImplicitH_NoThrow(nei, -1);

            if (_implicit_h.size() > nei && _implicit_h[nei] >= 0)
            {
                if (order == BOND_SINGLE)
                    _implicit_h[nei]++;
                else if (order == BOND_DOUBLE)
                    _implicit_h[nei] += 2;
                else if (order == BOND_TRIPLE)
                    _implicit_h[nei] += 3;
                else
                    _implicit_h[nei] = -1;
            }

            if (_connectivity.size() > nei && _connectivity[nei] >= 0)
            {
                if (order == BOND_SINGLE)
                    _connectivity[nei]--;
                else if (order == BOND_DOUBLE)
                    _connectivity[nei] -= 2;
                else if (order == BOND_TRIPLE)
                    _connectivity[nei] -= 3;
                else
                    _connectivity[nei] = -1;
            }
        }
        _validateVertexConnectivity(idx, false);
    }
    updateEditRevision();
}

int Molecule::getImplicitH(int idx, bool impl_h_no_throw)
{
    if (impl_h_no_throw)
        return getImplicitH_NoThrow(idx, 0);
    else
        return getImplicitH(idx);
}

int Molecule::getImplicitH(int idx)
{
    int conn = getAtomConnectivity_noImplH(idx);
    return _getImplicitHForConnectivity(idx, conn, true);
}

int Molecule::getImplicitH_NoThrow(int idx, int fallback)
{
    try
    {
        return getImplicitH(idx);
    }
    catch (Element::Error&)
    {
        return fallback;
    }
}

int Molecule::calcImplicitHForConnectivity(int idx, int conn)
{
    try
    {
        return _getImplicitHForConnectivity(idx, conn, false);
    }
    catch (Element::Error&)
    {
        return -1;
    }
}

int Molecule::_getImplicitHForConnectivity(int idx, int conn, bool use_cache)
{
    if (_atoms[idx].number == ELEM_PSEUDO)
        throw Error("getImplicitH() does not work on pseudo-atoms");

    if (_atoms[idx].number == ELEM_RSITE)
        throw Error("getImplicitH() does not work on R-sites");

    if (_atoms[idx].number == ELEM_TEMPLATE)
        throw Error("getImplicitH() does not work on template atoms");

    if (use_cache)
    {
        if (_implicit_h.size() > idx && _implicit_h[idx] >= 0)
            return _implicit_h[idx];
    }

    const _Atom& atom = _atoms[idx];

    int radical = 0;

    if (_radicals.size() > idx && _radicals[idx] >= 0)
        radical = _radicals[idx];

    int impl_h = -1;

    if (conn < 0)
    {
        if (getAtomAromaticity(idx) == ATOM_AROMATIC)
        {
            int degree = getVertex(idx).degree();
            int i;

            for (i = 1; i <= attachmentPointCount(); i++)
            {
                int j = 0, aidx;

                for (j = 0; (aidx = getAttachmentPoint(i, j)) != -1; j++)
                {
                    if (aidx == idx)
                        degree++;
                }
            }

            if (atom.number == ELEM_C && atom.charge == 0)
            {
                if (degree == 3)
                    impl_h = -Element::radicalElectrons(radical);
                else if (degree == 2)
                    impl_h = 1 - Element::radicalElectrons(radical);
            }
            else if (atom.number == ELEM_O && atom.charge == 0)
                impl_h = 0;
            else if (atom.number == ELEM_N && atom.charge == 0 && degree == 3)
                impl_h = 0;
            else if (atom.number == ELEM_N && atom.charge == 1 && degree == 3)
                impl_h = 0;
            else if (atom.number == ELEM_S && atom.charge == 0 && degree == 3)
                impl_h = 0;
        }
        else
            throw Error("internal: unsure connectivity on an aliphatic atom");

        if (impl_h < 0)
        {
            if (_ignore_bad_valence)
                impl_h = 0;
            else
                throw Element::Error("cannot calculate implicit hydrogens on aromatic %s, charge %d, degree %d, %d radical electrons",
                                     Element::toString(atom.number), atom.charge, getVertex(idx).degree(), Element::radicalElectrons(radical));
        }
    }
    else
    {
        if (atom.explicit_valence)
        {
            // Explicit valence means that the molecule was converted from Molfile.
            // Conventions are that if we have explicit valence, we discard radical
            // and charge when calculating implicit hydgogens.
            impl_h = _valence[idx] - Element::calcValenceMinusHyd(atom.number, 0, 0, conn);

            if (impl_h < 0)
            {
                if (_ignore_bad_valence)
                    impl_h = 0;
                else
                    throw Element::Error("explicit valence %d specified on %s, but %d bonds are drawn", _valence[idx], Element::toString(atom.number), conn);
            }
        }
        else if (isNitrogenV5(idx))
        {
            // special case of 5-connected nitrogen like "CN(=O)=O".
            // It should really be C[N+](O-)=O, but we let people live in happy ignorance.
            impl_h = 0;
        }
        else
        {
            int radical = -1;

            if (_radicals.size() > idx)
                radical = _radicals[idx];

            int valence;

            if (radical == -1)
            {
                // no information about implicit H, not sure about radical either --
                // this can happen exclusively in CML.
                if (Element::calcValence(atom.number, atom.charge, 0, conn, valence, impl_h, false))
                    radical = 0;
                else if (Element::calcValence(atom.number, atom.charge, RADICAL_SINGLET, conn, valence, impl_h, false))
                    radical = RADICAL_SINGLET;
                else if (Element::calcValence(atom.number, atom.charge, RADICAL_DOUBLET, conn, valence, impl_h, false))
                    radical = RADICAL_DOUBLET;
                else
                    throw Element::Error("can not calculate valence on %s, charge %d, connectivity %d", Element::toString(atom.number), atom.charge, conn);
                if (use_cache)
                {
                    _radicals.expandFill(idx + 1, -1);
                    _radicals[idx] = radical;
                }
            }
            else
            {
                // no information about implicit H, but sure about radical --
                // this is a commmon situtation for Molfiles or non-bracketed SMILES atoms.
                // Will throw an error on 5-valent carbon and such.
                if (_ignore_bad_valence)
                    Element::calcValence(atom.number, atom.charge, radical, conn, valence, impl_h, false);
                else
                    Element::calcValence(atom.number, atom.charge, radical, conn, valence, impl_h, true);
            }
        }
    }

    if (use_cache)
    {
        _implicit_h.expandFill(idx + 1, -1);
        _implicit_h[idx] = impl_h;
    }

    if (impl_h < 0)
        throw Error("_getImplicitHForConnectivity(): internal");

    return impl_h;
}

bool Molecule::isNitrogenV5(int idx)
{
    int conn = getAtomConnectivity_noImplH(idx);
    return isNitrogenV5ForConnectivity(idx, conn);
}

bool Molecule::isNitrogenV5ForConnectivity(int idx, int conn)
{
    if (getAtomNumber(idx) != ELEM_N)
        return false;

    if (getAtomCharge(idx) != 0)
        return false;

    int radical = 0;

    if (_radicals.size() > idx && _radicals[idx] >= 0)
        radical = _radicals[idx];

    int radical_elections = Element::radicalElectrons(radical);
    return (radical_elections == 0 && conn == 5) || (radical_elections == 1 && conn == 4);
}

int Molecule::getAtomNumber(int idx)
{
    return _atoms[idx].number;
}

int Molecule::getAtomCharge(int idx)
{
    return _atoms[idx].charge;
}

int Molecule::getAtomIsotope(int idx)
{
    return _atoms[idx].isotope;
}

bool Molecule::getIgnoreBadValenceFlag()
{
    return _ignore_bad_valence;
}

void Molecule::setIgnoreBadValenceFlag(bool flag)
{
    _ignore_bad_valence = flag;
}

int Molecule::getAtomValence(int idx)
{
    if (_atoms[idx].number == ELEM_PSEUDO)
        throw Error("getAtomValence() does not work on pseudo-atoms");

    if (_atoms[idx].number == ELEM_TEMPLATE)
        throw Error("getAtomValence() does not work on template atoms");

    if (_atoms[idx].number == ELEM_RSITE)
        throw Error("getAtomValence() does not work on R-sites");

    if (_valence.size() > idx && _valence[idx] >= 0)
        return _valence[idx];

    if (isNitrogenV5(idx))
    {
        _valence.expandFill(idx + 1, -1);
        _valence[idx] = 4;
        return 4;
    }

    const _Atom& atom = _atoms[idx];

    int conn = getAtomConnectivity_noImplH(idx);

    if (conn < 0)
    {
        int min_conn, n_arom;

        calcAromaticAtomConnectivity(idx, n_arom, min_conn);

        int val = Element::calcValenceOfAromaticAtom(atom.number, atom.charge, n_arom, min_conn);

        if (val >= 0)
        {
            _valence.expandFill(idx + 1, -1);
            _valence[idx] = val;
            return val;
        }

        if (_ignore_bad_valence)
        {
            val = min_conn;
            _valence.expandFill(idx + 1, -1);
            _valence[idx] = val;
            return val;
        }
        else
            throw Element::Error("can not calculate valence of %s (%d aromatic bonds, min connectivity %d, charge %d)", Element::toString(atom.number), n_arom,
                                 min_conn, atom.charge);
    }

    int radical = -1;
    int impl_h = -1;
    int valence;
    bool unusual_valence = false;

    if (_radicals.size() > idx && _radicals[idx] >= 0)
        radical = _radicals[idx];

    if (_implicit_h.size() > idx && _implicit_h[idx] >= 0)
    {
        impl_h = _implicit_h[idx];
        int normal_impl_h;

        if (radical == -1)
        {
            // have implicit H count, but no information about radical. Frequently occurs in SMILES
            // expressions like [CH2] or [C]
            if (Element::calcValence(atom.number, atom.charge, 0, conn, valence, normal_impl_h, false) && normal_impl_h == impl_h)
                radical = 0; // [SiH4]
            else if (Element::calcValence(atom.number, atom.charge, RADICAL_SINGLET, conn, valence, normal_impl_h, false) && normal_impl_h == impl_h)
                radical = RADICAL_SINGLET; // [CH2]
            else if (Element::calcValence(atom.number, atom.charge, RADICAL_DOUBLET, conn, valence, normal_impl_h, false) && normal_impl_h == impl_h)
                radical = RADICAL_DOUBLET; // [CH3]
            else if (Element::calcValence(atom.number, atom.charge, 0, conn + impl_h, valence, normal_impl_h, false) && normal_impl_h == 0)
            {
                radical = 0; // [PH5]
                valence = conn + impl_h;
                unusual_valence = true;
            }
            else if (Element::calcValence(atom.number, atom.charge, RADICAL_SINGLET, conn + impl_h, valence, normal_impl_h, false) && normal_impl_h == 0)
            {
                radical = RADICAL_SINGLET;
                valence = conn + impl_h;
                unusual_valence = true;
            }
            else if (Element::calcValence(atom.number, atom.charge, RADICAL_DOUBLET, conn + impl_h, valence, normal_impl_h, false) && normal_impl_h == 0)
            {
                radical = RADICAL_DOUBLET; // [PH4]
                valence = conn + impl_h;
                unusual_valence = true;
            }
            else if (atom.number == ELEM_C) // [C], [CH]
            {
                radical = 0;
                valence = conn + impl_h;
                unusual_valence = true;
            }
            else if ((abs(atom.charge) == 1) && (atom.number == ELEM_N || atom.number == ELEM_O)) // [N+], [O-]
            {
                radical = 0;
                valence = conn + impl_h;
                unusual_valence = true;
            }

            if (radical != -1)
            {
                _radicals.expandFill(idx + 1, -1);
                _radicals[idx] = radical;
            }
        }
        else
        {
            // have both implicit H count and radicals -- can happen in CML or in extended SMILES
            if (Element::calcValence(atom.number, atom.charge, radical, conn, valence, normal_impl_h, false) && normal_impl_h == impl_h)
                ;
            else
            {
                // rare case
                valence = conn + impl_h;
                unusual_valence = true;
            }
        }
    }
    else
    {
        if (radical == -1)
        {
            // no information about implicit H, not sure about radical either --
            // this can happen exclusively in CML.
            if (Element::calcValence(atom.number, atom.charge, 0, conn, valence, impl_h, false))
                radical = 0;
            else if (Element::calcValence(atom.number, atom.charge, RADICAL_SINGLET, conn, valence, impl_h, false))
                radical = RADICAL_SINGLET;
            else if (Element::calcValence(atom.number, atom.charge, RADICAL_DOUBLET, conn, valence, impl_h, false))
                radical = RADICAL_DOUBLET;
            else
                throw Element::Error("can not calculate valence on %s, charge %d, connectivity %d", Element::toString(atom.number), atom.charge, conn);
            _radicals.expandFill(idx + 1, -1);
            _radicals[idx] = radical;
        }
        else
        {
            // no information about implicit H, but sure about radical --
            // this is a commmon situtation for Molfiles or non-bracketed SMILES atoms.
            // Will throw an error on 5-valent carbon and such.
            if (_ignore_bad_valence)
            {
                Element::calcValence(atom.number, atom.charge, radical, conn, valence, impl_h, false);
            }
            else
            {
                Element::calcValence(atom.number, atom.charge, radical, conn, valence, impl_h, true);
            }
        }

        _implicit_h.expandFill(idx + 1, -1);
        _implicit_h[idx] = impl_h;
    }

    _valence.expandFill(idx + 1, -1);
    _valence[idx] = valence;

    if (unusual_valence)
        _atoms[idx].explicit_valence = true;

    return valence;
}

int Molecule::getAtomRadical(int idx)
{
    if (_atoms[idx].number == ELEM_PSEUDO)
        throw Error("getAtomRadical() does not work on pseudo-atoms");

    if (_atoms[idx].number == ELEM_RSITE)
        throw Error("getAtomRadical() does not work on R-sites");

    if (_atoms[idx].number == ELEM_TEMPLATE)
        throw Error("getAtomRadical() does not work on template atoms");

    if (_radicals.size() > idx && _radicals[idx] >= 0)
        return _radicals[idx];

    getAtomValence(idx);

    if (_radicals.size() > idx && _radicals[idx] >= 0)
        return _radicals[idx];

    // getAtomValence() did not help: now we know that
    // this is either an aromatic atom or 5-valence nitrogen;
    // in any case, radical is zero.

    _radicals.expandFill(idx + 1, -1);
    _radicals[idx] = 0;
    return 0;
}

void Molecule::saveBondOrders(Molecule& mol, Array<int>& orders)
{
    orders.copy(mol._bond_orders);
}

void Molecule::loadBondOrders(Molecule& mol, Array<int>& orders)
{
    mol._bond_orders.copy(orders);
    mol.updateEditRevision();
}

int Molecule::getAtomSubstCount(int idx)
{
    int i, res = 0;
    const Vertex& vertex = getVertex(idx);

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        if (_atoms[vertex.neiVertex(i)].number != ELEM_H)
            res++;
    }

    return res;
}

int Molecule::getAtomRingBondsCount(int idx)
{
    int i, res = 0;
    const Vertex& vertex = getVertex(idx);

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        if (getEdgeTopology(vertex.neiEdge(i)) == TOPOLOGY_RING)
            res++;
    }

    return res;
}

bool Molecule::isSaturatedAtom(int idx)
{
    int i;
    const Vertex& vertex = getVertex(idx);

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
        if (getBondOrder(vertex.neiEdge(i)) != BOND_SINGLE)
            return false;

    return true;
}

int Molecule::getBondOrder(int idx) const
{
    return _bond_orders[idx];
}

int Molecule::getBondTopology(int idx)
{
    return getEdgeTopology(idx);
}

int Molecule::getExplicitValence(int idx)
{
    if (_atoms[idx].explicit_valence)
        return _valence[idx];

    if (_atoms[idx].number == ELEM_PSEUDO || _atoms[idx].number == ELEM_RSITE || _atoms[idx].number == ELEM_TEMPLATE)
        return -1;

    // try to calculate explicit valence from hydrogens, as in elemental carbon [C]
    try
    {
        getAtomValence(idx);
    }
    catch (Element::Error&)
    {
        return -1;
    }

    if (_atoms[idx].explicit_valence)
        return _valence[idx];

    return -1;
}

bool Molecule::atomNumberBelongs(int idx, const int* numbers, int count)
{
    int number = _atoms[idx].number;
    int i;

    for (i = 0; i < count; i++)
        if (number == numbers[i])
            return true;

    return false;
}

bool Molecule::possibleAtomNumber(int idx, int number)
{
    return _atoms[idx].number == number;
}

bool Molecule::possibleAtomNumberAndCharge(int idx, int number, int charge)
{
    return _atoms[idx].number == number && _atoms[idx].charge == charge;
}

bool Molecule::possibleAtomNumberAndIsotope(int idx, int number, int isotope)
{
    return _atoms[idx].number == number && _atoms[idx].isotope == isotope;
}

bool Molecule::possibleAtomIsotope(int idx, int isotope)
{
    return _atoms[idx].isotope == isotope;
}

bool Molecule::possibleAtomCharge(int idx, int charge)
{
    return _atoms[idx].charge == charge;
}

void Molecule::getAtomDescription(int idx, Array<char>& description)
{
    _Atom& atom = _atoms[idx];
    ArrayOutput output(description);

    if (atom.isotope != 0)
        output.printf("%d", atom.isotope);

    if (isPseudoAtom(idx))
        output.printf("%s", getPseudoAtom(idx));
    else if (isTemplateAtom(idx))
        output.printf("%s", getTemplateAtom(idx));
    else
        output.printf("%s", Element::toString(atom.number));

    if (atom.charge == -1)
        output.printf("-");
    else if (atom.charge == 1)
        output.printf("+");
    else if (atom.charge > 0)
        output.printf("+%d", atom.charge);
    else if (atom.charge < 0)
        output.printf("-%d", -atom.charge);

    output.writeChar(0);
}

void Molecule::getBondDescription(int idx, Array<char>& description)
{
    ArrayOutput output(description);

    switch (_bond_orders[idx])
    {
    case BOND_SINGLE:
        output.printf("single");
        return;
    case BOND_DOUBLE:
        output.printf("double");
        return;
    case BOND_TRIPLE:
        output.printf("triple");
        return;
    case BOND_AROMATIC:
        output.printf("aromatic");
        return;
    }
}

bool Molecule::possibleBondOrder(int idx, int order)
{
    return _bond_orders[idx] == order;
}

int Molecule::addAtom(int number)
{
    int idx = _addBaseAtom();
    _atoms.expand(idx + 1);
    return resetAtom(idx, number);
}

int Molecule::resetAtom(int idx, int number)
{
    updateEditRevision();

    memset(&_atoms[idx], 0, sizeof(_Atom));
    _atoms[idx].number = number;
    _validateVertexConnectivity(idx, false);
    return idx;
}

int Molecule::addBond(int beg, int end, int order)
{
    updateEditRevision();
    int idx = _addBaseBond(beg, end);

    _bond_orders.expand(idx + 1);
    _bond_orders[idx] = order;

    _aromaticity.clear();
    _aromatized = false;

    _validateVertexConnectivity(beg, false);
    _validateVertexConnectivity(end, false);

    return idx;
}

int Molecule::addBond_Silent(int beg, int end, int order)
{
    updateEditRevision();
    int idx = _addBaseBond(beg, end);

    _bond_orders.expand(idx + 1);
    _bond_orders[idx] = order;

    _aromaticity.clear();
    _aromatized = false;

    return idx;
}

bool Molecule::isPseudoAtom(int idx)
{
    return _atoms[idx].number == ELEM_PSEUDO;
}

const char* Molecule::getPseudoAtom(int idx)
{
    const _Atom& atom = _atoms[idx];

    if (atom.number != ELEM_PSEUDO)
        throw Error("getPseudoAtom(): atom #%d is not a pseudoatom", idx);

    const char* res = _pseudo_atom_values.at(atom.pseudoatom_value_idx);

    if (res == 0)
        throw Error("pseudoatom string is zero");

    return res;
}

bool Molecule::isTemplateAtom(int idx)
{
    return _atoms[idx].number == ELEM_TEMPLATE;
}

const char* Molecule::getTemplateAtom(int idx)
{
    const _Atom& atom = _atoms[idx];

    if (atom.number != ELEM_TEMPLATE)
        throw Error("getTemplateAtom(): atom #%d is not a template atom", idx);

    _TemplateOccurrence& occur = _template_occurrences.at(atom.template_occur_idx);
    const char* res = _template_names.at(occur.name_idx);

    if (res == 0)
        throw Error("template atom string is zero");

    return res;
}

const char* Molecule::getTemplateAtomClass(int idx)
{
    const _Atom& atom = _atoms[idx];

    if (atom.number != ELEM_TEMPLATE)
        throw Error("getTemplateAtomClass(): atom #%d is not a template atom", idx);

    _TemplateOccurrence& occur = _template_occurrences.at(atom.template_occur_idx);
    const char* res = _template_classes.at(occur.class_idx);

    return res;
}

const int Molecule::getTemplateAtomTemplateIndex(int idx)
{
    const _Atom& atom = _atoms[idx];

    if (atom.number != ELEM_TEMPLATE)
        throw Error("getTemplateAtomTemplateIndex(): atom #%d is not a template atom", idx);

    _TemplateOccurrence& occur = _template_occurrences.at(atom.template_occur_idx);
    const int res = occur.template_idx;
    return res;
}

const int Molecule::getTemplateAtomSeqid(int idx)
{
    const _Atom& atom = _atoms[idx];

    if (atom.number != ELEM_TEMPLATE)
        throw Error("getTemplateAtomSeqid(): atom #%d is not a template atom", idx);

    _TemplateOccurrence& occur = _template_occurrences.at(atom.template_occur_idx);
    const int res = occur.seq_id;

    return res;
}

const int Molecule::getTemplateAtomDisplayOption(int idx)
{
    const _Atom& atom = _atoms[idx];

    if (atom.number != ELEM_TEMPLATE)
        throw Error("getTemplateAtomDisplayOption(): atom #%d is not a template atom", idx);

    _TemplateOccurrence& occur = _template_occurrences.at(atom.template_occur_idx);
    const int res = static_cast<int>(occur.contracted);
    // const int res = occur.contracted;

    return res;
}

void Molecule::getTemplatesMap(std::unordered_map<std::pair<std::string, std::string>, std::reference_wrapper<TGroup>, pair_hash>& templates_map)
{
    templates_map.clear();
    int temp_idx = 0;
    for (int i = tgroups.begin(); i != tgroups.end(); i = tgroups.next(i))
    {
        auto& tg = tgroups.getTGroup(i);
        std::string tname = tg.tgroup_name.size() ? tg.tgroup_name.ptr() : monomerAlias(tg);
        templates_map.emplace(std::make_pair(tname, tg.tgroup_class.ptr()), std::ref(tg));
    }
}

void Molecule::getTemplateAtomDirectionsMap(std::unordered_map<int, std::map<int, int>>& directions_map)
{
    for (int i = template_attachment_points.begin(); i != template_attachment_points.end(); i = template_attachment_points.next(i))
    {
        auto& tap = template_attachment_points[i];
        if (tap.ap_id.size())
        {
            Array<char> atom_label;
            getAtomSymbol(tap.ap_occur_idx, atom_label);
            int ap_id = tap.ap_id[0] - 'A';
            directions_map[tap.ap_occur_idx].emplace(ap_id, tap.ap_aidx);
        }
    }
}

BaseMolecule* Molecule::neu()
{
    return new Molecule();
}

bool Molecule::bondStereoCare(int idx)
{
    if (!cis_trans.exists())
        return false;
    // In ordinary molecule all bond's stereoconfigurations are important
    return cis_trans.getParity(idx) != 0;
}

bool Molecule::aromatize(const AromaticityOptions& options)
{
    updateEditRevision();
    bool arom_found = MoleculeAromatizer::aromatizeBonds(*this, options);
    _aromatized = true;
    return arom_found;
}

bool Molecule::dearomatize(const AromaticityOptions& options)
{
    updateEditRevision();
    return MoleculeDearomatizer::dearomatizeMolecule(*this, options);
}

int Molecule::getAtomMaxH(int idx)
{
    return getAtomTotalH(idx);
}

int Molecule::getAtomMinH(int idx)
{
    return getAtomTotalH(idx);
}

int Molecule::getAtomTotalH(int idx)
{
    if (_total_h.size() > idx && _total_h[idx] >= 0)
        return _total_h[idx];

    int i, h = getImplicitH(idx);

    const Vertex& vertex = getVertex(idx);

    for (i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
        if (getAtomNumber(vertex.neiVertex(i)) == ELEM_H)
            h++;

    _total_h.expandFill(idx + 1, -1);
    _total_h[idx] = h;

    return h;
}

bool Molecule::isRSite(int atom_idx)
{
    return _atoms[atom_idx].number == ELEM_RSITE;
}

dword Molecule::getRSiteBits(int atom_idx)
{
    if (_atoms[atom_idx].number != ELEM_RSITE)
        throw Error("getRSiteBits(): atom #%d is not an r-site", atom_idx);
    return _atoms[atom_idx].rgroup_bits;
}

void Molecule::setRSiteBits(int atom_idx, int bits)
{
    if (_atoms[atom_idx].number != ELEM_RSITE)
        throw Error("setRSiteBits(): atom #%d is not an r-site", atom_idx);
    _atoms[atom_idx].rgroup_bits = bits;
    updateEditRevision();
}

void Molecule::allowRGroupOnRSite(int atom_idx, int rg_idx)
{
    if (_atoms[atom_idx].number != ELEM_RSITE)
        throw Error("allowRGroupOnRSite(): atom #%d is not an r-site", atom_idx);

    if (rg_idx < 1 || rg_idx > 32)
        throw Error("allowRGroupOnRSite(): rgroup number %d is invalid", rg_idx);

    rg_idx--;

    _atoms[atom_idx].rgroup_bits |= (1 << rg_idx);
    updateEditRevision();
}

void Molecule::invalidateHCounters()
{
    _implicit_h.clear();
    _total_h.clear();
    _connectivity.clear();
}

void Molecule::checkForConsistency(Molecule& mol)
{
    // Try to restore hydrogens in aromatic cycles first
    mol.restoreAromaticHydrogens();

    for (int i : mol.vertices())
    {
        if (mol.isPseudoAtom(i) || mol.isRSite(i) || mol.isTemplateAtom(i))
            continue;

        // check that we are sure about valence
        // (if the radical is not set, it is calculated from the valence anyway)
        mol.getAtomValence(i);

        // check that we are sure about implicit H counter and valence
        mol.getImplicitH(i);
    }
}

bool Molecule::isAromatized()
{
    return _aromatized;
}

bool Molecule::shouldWriteHCount(Molecule& mol, int idx)
{
    return shouldWriteHCountEx(mol, idx, 0);
}

// Moved this method here to supply both Smiles and CML savers
bool Molecule::shouldWriteHCountEx(Molecule& mol, int idx, int h_to_ignore)
{

    if (mol.isPseudoAtom(idx) || mol.isRSite(idx) || mol.isTemplateAtom(idx))
        return false;

    bool aromatic = (mol.getAtomAromaticity(idx) == ATOM_AROMATIC);

    int atom_number = mol.getAtomNumber(idx);
    int charge = mol.getAtomCharge(idx);

    // We should write the H count if it is less than the normal (lowest valence)
    // count, like in atoms with radicals.
    if (mol.getAtomRadical_NoThrow(idx, -1) > 0)
        return true;

    // Should we write the H count for an aromatic atom or not?
    // In a better world, we would have been checking that the hydrogens
    // 'make difference' by de-aromatizing the molecule and comparing
    // the hydrogen counts in the de-aromatized atoms with the atoms we
    // are writing now.
    // In the real world, de-aromatization is complicated and takes time,
    // so we write hydrogen counts on all aromatic atoms, except
    // uncharged C and O with no radicals, for which we can always tell
    // the number of hydrogens by the number of bonds.
    //
    // Also handle some degerate cases with invalid valences like C[c]1(C)ccccc1
    // and store implicit hydrogens for unusual situations
    if (aromatic)
    {
        if (atom_number != ELEM_C && atom_number != ELEM_O)
            return true;
        if (charge != 0)
            return true;
        int n_arom, min_conn;
        mol.calcAromaticAtomConnectivity(idx, n_arom, min_conn);
        if (atom_number == ELEM_C)
        {
            // Ensure that there can be double bond connected
            // But do not save for O=c1ccocc1
            if (min_conn > 3 && mol.getVertex(idx).degree() > 3)
                return true; // Unusual aromatic Carbon atom
        }
        if (atom_number == ELEM_O)
        {
            // Ensure that there can be double bond connected
            if (min_conn != 2)
                return true; // Unusual aromatic Oxigen atom
        }
    }

    // We also should write the H count if it exceeds the normal (lowest valence)
    // count, like in [PH5]
    int normal_val, normal_hyd;

    int impl_h = mol.getImplicitH_NoThrow(idx, -1);
    if (impl_h >= 0)
        impl_h += h_to_ignore;
    if (mol.isNitrogenV5(idx))
    {
        normal_val = 4;
        normal_hyd = 0;
    }
    else
    {
        if (impl_h < 0)
            return false; // can not write an undefined H count

        int conn = mol.getAtomConnectivity_noImplH(idx) - h_to_ignore;

        if (conn < 0)
            return false; // this is an aromatic atom -- dealed with that before

        if (!Element::calcValence(atom_number, charge, 0, conn, normal_val, normal_hyd, false))
            return true;
    }

    if (impl_h != normal_hyd)
        return true;

    return false;
}

void Molecule::invalidateAtom(int index, int mask)
{
    BaseMolecule::invalidateAtom(index, mask);
}

bool Molecule::restoreAromaticHydrogens(bool unambiguous_only)
{
    return MoleculeDearomatizer::restoreHydrogens(*this, unambiguous_only);
}

bool Molecule::standardize(const StandardizeOptions& options)
{
    updateEditRevision();
    return MoleculeStandardizer::standardize(*this, options);
}

bool Molecule::ionize(float ph, float ph_toll, const IonizeOptions& options)
{
    updateEditRevision();
    return MoleculeIonizer::ionize(*this, ph, ph_toll, options);
}

bool Molecule::isPossibleFischerProjection(const char* options)
{
    if (!BaseMolecule::hasCoord(*this) || BaseMolecule::hasZCoord(*this))
        return false;

    for (auto i : edges())
    {
        if (getBondDirection(i) > 0)
            return false;
    }

    for (auto i : vertices())
    {
        if ((getAtomNumber(i) == ELEM_C) && (getVertex(i).degree() == 4))
        {
            const Vertex& v = getVertex(i);
            Vec3f& central_atom = getAtomXyz(i);
            Vec3f nei_coords[4];
            int nei_count = 0;
            for (auto j : v.neighbors())
            {
                nei_coords[nei_count++] = getAtomXyz(v.neiVertex(j));
            }

            float angle;
            Vec3f bond1, bond2;
            int ncount = 0;
            for (auto j = 0; j < 4; j++)
            {
                if (j == 3)
                {
                    bond1.diff(nei_coords[3], central_atom);
                    bond1.normalize();
                    bond2.diff(nei_coords[0], central_atom);
                    bond2.normalize();
                    Vec3f::angle(bond1, bond2, angle);
                }
                else
                {
                    bond1.diff(nei_coords[j], central_atom);
                    bond1.normalize();
                    bond2.diff(nei_coords[j + 1], central_atom);
                    bond2.normalize();
                    Vec3f::angle(bond1, bond2, angle);
                }

                if ((fabs(angle - M_PI / 2.f) < EPSILON) || (fabs(angle - M_PI) < EPSILON))
                    ncount++;
            }
            if (ncount == 4)
            {
                return true;
            }
        }
    }
    return false;
}

bool Molecule::isPiBonded(const int atom_index) const
{
    const Vertex& vertex = getVertex(atom_index);
    for (auto i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        const int order = getBondOrder(vertex.neiEdge(i));
        if (order == BOND_DOUBLE || order == BOND_TRIPLE || order == BOND_AROMATIC)
        {
            return true;
        }
    }
    return false;
}
