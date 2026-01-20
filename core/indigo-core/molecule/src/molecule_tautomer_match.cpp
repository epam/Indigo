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

#include "graph/embedding_enumerator.h"
#include "molecule/molecule.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_inchi.h"
#include "molecule/molecule_layered_molecules.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/molecule_tautomer.h"
#include "molecule/molecule_tautomer_enumerator.h"
#include "molecule/molecule_tautomer_utils.h"

using namespace indigo;

class PathRulesChecker;

CP_DEF(TautomerSearchContext);

TautomerSearchContext::TautomerSearchContext(BaseMolecule& g1_, BaseMolecule& g2_, GraphDecomposer& decomposer1_, GraphDecomposer& decomposer2_,
                                             const PtrArray<TautomerRule>& rules_list_, const AromaticityOptions& arom_options)
    : g1(g1_), g2(g2_), decomposer1(decomposer1_), decomposer2(decomposer2_), CP_INIT, TL_CP_GET(h_rep_count_1), TL_CP_GET(h_rep_count_2),
      rules_list(rules_list_), force_hydrogens(false), ring_chain(false), rules(0), substructure(false), cb_check_rules(0), TL_CP_GET(dearomatizations),
      TL_CP_GET(core_1), TL_CP_GET(core_2), TL_CP_GET(chains_2), TL_CP_GET(edges_1), TL_CP_GET(edges_2), TL_CP_GET(edge_types_2), TL_CP_GET(n1), TL_CP_GET(n2)
{
    this->arom_options = arom_options;
    if (g2.vertexCount() + g2.edgeCount() > 80)
        max_chains = 1;
    else if (g2.vertexCount() + g2.edgeCount() > 40)
        max_chains = 2;
    else
        max_chains = 0;

    dearomatizer = std::make_unique<Dearomatizer>(g2.asMolecule(), (int*)0, arom_options);
    dearomatizer->enumerateDearomatizations(dearomatizations);

    dearomatizationMatcher = std::make_unique<DearomatizationMatcher>(dearomatizations, g2.asMolecule(), (int*)0);
}

TautomerSearchContext::~TautomerSearchContext()
{
}

bool TautomerMatcher::_matchAtoms(Graph& subgraph, Graph& supergraph, const int* /*core_sub*/, int sub_idx, int super_idx, void* /*userdata*/)
{
    QueryMolecule& query = ((BaseMolecule&)subgraph).asQueryMolecule();
    QueryMolecule::Atom* atom = &query.getAtom(sub_idx);
    BaseMolecule& target = (BaseMolecule&)supergraph;

    if (!MoleculeSubstructureMatcher::matchQueryAtom(atom, (BaseMolecule&)supergraph, super_idx, 0, 0xFFFFFFFFUL))
        return false;

    if (query.stereocenters.getType(sub_idx) > target.stereocenters.getType(super_idx))
        return false;

    if (query.stereocenters.getType(sub_idx) > 0)
        // Perform hydrogen check only on stereocenters.
        // This is to avoid "cannot map pyramid" error.
        // Normally, hydrogen counters are not something to look after
        // when doing tautomer match.]
        if (!target.isPseudoAtom(super_idx) && !target.isRSite(super_idx) && !target.isTemplateAtom(super_idx))
            if (query.getAtomMinH(sub_idx) > target.getAtomMaxH(super_idx))
                return false;

    return true;
}

bool TautomerMatcher::_matchAtomsEx(Graph& subgraph, Graph& supergraph, const int* /*core_sub*/, int sub_idx, int super_idx, void* /*userdata*/)
{
    return MoleculeExactMatcher::matchAtoms(((BaseMolecule&)subgraph).asMolecule(), (BaseMolecule&)supergraph, sub_idx, super_idx, 0xFFFFFFFFUL);
}

bool TautomerMatcher::matchBondsTau(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    BaseMolecule& molecule = (BaseMolecule&)supergraph;
    BaseMolecule& query = (BaseMolecule&)subgraph;

    // check 'zeroed' bond
    if (molecule.getBondOrder(super_idx) == -1 && molecule.possibleBondOrder(super_idx, BOND_SINGLE))
        return false;

    /*
    if (sub_bond.type == BOND_AROMATIC || super_bond.type == BOND_AROMATIC)
       return true;

    if (sub_bond.type == super_bond.type)
       return true;
       */

    int sub_bond_order = query.getBondOrder(sub_idx);
    int super_bond_order = molecule.getBondOrder(super_idx);

    if (sub_bond_order == super_bond_order)
        return true;

    TautomerMatcher::MatchData& d = *(TautomerMatcher::MatchData*)userdata;

    if (super_bond_order == BOND_AROMATIC && sub_bond_order != BOND_AROMATIC)
        return d.context.dearomatizationMatcher->isAbleToFixBond(super_idx, sub_bond_order);

    return false;
}

bool TautomerMatcher::matchBondsTauSub(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* /*userdata*/)
{
    BaseMolecule& molecule = (BaseMolecule&)supergraph;
    QueryMolecule& query = ((BaseMolecule&)subgraph).asQueryMolecule();

    if (MoleculeSubstructureMatcher::matchQueryBond(&query.getBond(sub_idx), molecule, sub_idx, super_idx, 0, 0xFFFFFFFFUL))
        return true;

    int sub_bond_order = query.getBondOrder(sub_idx);
    int super_bond_order = molecule.getBondOrder(super_idx);

    if (super_bond_order == -1 && molecule.possibleBondOrder(super_idx, 1))
        super_bond_order = 0; // single bond that can appear in tautomer chain

    if (sub_bond_order < 0) // query bond
        return false;       // checked above in matchQueryBond()

    if (sub_bond_order != super_bond_order)
    {
        if (super_bond_order == BOND_AROMATIC)
            return true;

        if (sub_bond_order == BOND_AROMATIC)
            return false;

        if (abs(sub_bond_order - super_bond_order) == 1)
            return true;
    }

    return false;
}

bool TautomerMatcher::isTautomerActivatedCarbon(BaseMolecule& mol, int atom_idx)
{
    if (atom_idx < 0 || atom_idx >= mol.vertexEnd())
        return false;

    const Vertex& v = mol.getVertex(atom_idx);

    static const int ACTIVATING_HETERO[] = {ELEM_O, ELEM_N, ELEM_S, ELEM_P};
    static const int EWG_CENTERS[] = {ELEM_C, ELEM_N, ELEM_S, ELEM_P};
    static const int EWG_ATOMS[] = {ELEM_O, ELEM_N, ELEM_S};
    static const int ENOL_ATOMS[] = {ELEM_O, ELEM_N, ELEM_S, ELEM_P};
    static const int UNSAT_BONDS[] = {BOND_DOUBLE, BOND_TRIPLE, BOND_AROMATIC};

    auto matches = [&](int idx, const int* list, size_t size) {
        for (size_t i = 0; i < size; ++i)
            if (mol.possibleAtomNumber(idx, list[i]))
                return true;
        return false;
    };

    auto matchesBond = [&](int idx, const int* list, size_t size) {
        for (size_t i = 0; i < size; ++i)
            if (mol.possibleBondOrder(idx, list[i]))
                return true;
        return false;
    };

    for (int nei = v.neiBegin(); nei != v.neiEnd(); nei = v.neiNext(nei))
    {
        int nei_idx = v.neiVertex(nei);
        if (nei_idx < 0 || nei_idx >= mol.vertexEnd())
            continue;

        int edge_idx = v.neiEdge(nei);

        // Rule 1: Direct Heteroatom Connection
        if (matches(nei_idx, ACTIVATING_HETERO, NELEM(ACTIVATING_HETERO)))
            return true;

        // Rule 2: Alpha to EWG
        if (matches(nei_idx, EWG_CENTERS, NELEM(EWG_CENTERS)))
        {
            const Vertex& v_nei = mol.getVertex(nei_idx);
            bool neighbor_is_C = mol.possibleAtomNumber(nei_idx, ELEM_C);

            for (int n_nei = v_nei.neiBegin(); n_nei != v_nei.neiEnd(); n_nei = v_nei.neiNext(n_nei))
            {
                int n_nei_idx = v_nei.neiVertex(n_nei);
                if (n_nei_idx == atom_idx || n_nei_idx < 0 || n_nei_idx >= mol.vertexEnd())
                    continue;

                // Must be connected to O, N, S
                if (!matches(n_nei_idx, EWG_ATOMS, NELEM(EWG_ATOMS)))
                    continue;

                // Bond Check: If Center is C, bond must be Unsaturated.
                if (neighbor_is_C)
                {
                    int n_edge_idx = v_nei.neiEdge(n_nei);
                    if (!matchesBond(n_edge_idx, UNSAT_BONDS, NELEM(UNSAT_BONDS)))
                        continue;
                }
                // If Center is N/S/P, any bond is activating (implicit single/double logic handled by valency usually)
                return true;
            }
        }

        // Rule 3: Enolic System (C=C-OH)
        if (mol.possibleAtomNumber(nei_idx, ELEM_C) && mol.possibleBondOrder(edge_idx, BOND_DOUBLE))
        {
            const Vertex& v_nei = mol.getVertex(nei_idx);
            for (int n_nei = v_nei.neiBegin(); n_nei != v_nei.neiEnd(); n_nei = v_nei.neiNext(n_nei))
            {
                int n_nei_idx = v_nei.neiVertex(n_nei);
                if (n_nei_idx == atom_idx || n_nei_idx < 0 || n_nei_idx >= mol.vertexEnd())
                    continue;

                int n_edge_idx = v_nei.neiEdge(n_nei);
                // Must be Single Bond to Heteroatom
                if (!mol.possibleBondOrder(n_edge_idx, BOND_SINGLE))
                    continue;

                if (matches(n_nei_idx, ENOL_ATOMS, NELEM(ENOL_ATOMS)))
                    return true;
            }
        }
    }

    return false;
}

bool TautomerMatcher::matchAtomsTau(BaseMolecule& g1, BaseMolecule& g2, int n1, int n2, bool strict)
{
    if (n1 < 0 || n1 >= g1.vertexEnd() || n2 < 0 || n2 >= g2.vertexEnd())
        return false;

    if (g1.isPseudoAtom(n1) || g2.isPseudoAtom(n2))
        return false;

    if (g1.isTemplateAtom(n1) || g2.isTemplateAtom(n2))
        return false;

    if (g1.isRSite(n1) || g2.isRSite(n2))
        return false;

    if (g1.getAtomNumber(n1) == g2.getAtomNumber(n2) && g1.possibleAtomIsotope(n1, g2.getAtomIsotope(n2)))
    {
        // Anti-Tautomer Rule
        if (g1.possibleAtomNumber(n1, ELEM_C))
        {
            int h1 = g1.getAtomTotalH(n1);
            int h2 = g2.getAtomTotalH(n2);

            if (h1 != h2)
            {
                if (strict && (!isTautomerActivatedCarbon(g1, n1) || !isTautomerActivatedCarbon(g2, n2)))
                {
                    return false;
                }
            }
        }
        return true;
    }
    return false;
}

int TautomerMatcher::_remainderEmbedding(Graph& g1, Graph& g2, int* core1, int* core2, void* userdata)
{
    TautomerMatcher::MatchData& d = *(TautomerMatcher::MatchData*)userdata;

    // Check there's no redundant (unmapped) bonds in target
    for (int i = g2.edgeBegin(); i < g2.edgeEnd(); i = g2.edgeNext(i))
    {
        const Edge& edge = g2.getEdge(i);

        if (d.context.chains_2[edge.beg] > 0 && d.context.chains_2[edge.end] > 0)
            continue;

        if (core2[edge.beg] < 0)
            continue;

        if (core2[edge.end] < 0)
            continue;

        if (g1.findEdgeIndex(core2[edge.beg], core2[edge.end]) == -1)
            return 1;
    }

    if (!fixBondsNotInChains(d.context, core1, core2))
        return 1;

    d.context.core_1.clear_resize(d.context.g1.vertexEnd());
    d.context.core_2.clear_resize(d.context.g2.vertexEnd());

    memcpy(d.context.core_1.ptr(), core1, sizeof(int) * d.context.core_1.size());
    memcpy(d.context.core_2.ptr(), core2, sizeof(int) * d.context.core_2.size());

    return 0;
}

int TautomerMatcher::_preliminaryEmbedding(Graph& /*g1*/, Graph& g2, int* core1, int* core2, void* userdata)
{
    TautomerMatcher::MatchData& d = *(TautomerMatcher::MatchData*)userdata;

    QS_DEF(Array<int>, core_1);
    QS_DEF(Array<int>, core_2);

    core_1.copy(core1, d.context.initial_g1_vertexend);
    // can not use g1.vertexEnd() because atoms may have been
    // added to and removed from the query during the
    // TautomerChainChecker procedure.
    core_2.copy(core2, g2.vertexEnd());

    TautomerChainChecker cha_che(d.context, core_1, core_2, d.start_path_number);

    if (cha_che.check())
        return 1;

    return 0;
}

bool TautomerRule::check(BaseMolecule& molecule, int first_idx, int last_idx, char other_arom_first, char other_arom_last) const
{
    if (first_idx != -1 && last_idx != -1)
    {
        int first_atom = molecule.getAtomNumber(first_idx);
        int last_atom = molecule.getAtomNumber(last_idx);

        if (list1.find(first_atom) >= 0)
        {
            if (aromaticity1 == -1 || (aromaticity1 == 1 && (atomInAromaticRing(molecule, first_idx) || other_arom_first == 1)) ||
                (aromaticity1 == 0 && !atomInAromaticRing(molecule, first_idx)))
            {
                if (list2.find(last_atom) >= 0)
                {
                    if (aromaticity2 == -1 || (aromaticity2 == 1 && (atomInAromaticRing(molecule, last_idx) || other_arom_last == 1)) ||
                        (aromaticity2 == 0 && !atomInAromaticRing(molecule, last_idx)))
                    {
                        return true;
                    }
                }
            }
        }

        if (list2.find(first_atom) >= 0)
        {
            if (aromaticity2 == -1 || (aromaticity2 == 1 && (atomInAromaticRing(molecule, first_idx) || other_arom_first == 1)) ||
                (aromaticity2 == 0 && !atomInAromaticRing(molecule, first_idx)))
            {
                if (list1.find(last_atom) >= 0)
                {
                    if (aromaticity1 == -1 || (aromaticity1 == 1 && (atomInAromaticRing(molecule, last_idx) || other_arom_last == 1)) ||
                        (aromaticity1 == 0 && !atomInAromaticRing(molecule, last_idx)))
                    {
                        return true;
                    }
                }
            }
        }

        return false;
    }
    else if (first_idx != -1 || last_idx != -1)
    {
        if (first_idx == -1)
            first_idx = last_idx;

        int first_atom = molecule.getAtomNumber(first_idx);

        if (list1.find(first_atom) >= 0)
            if (aromaticity1 == -1 || (aromaticity1 == 1 && (atomInAromaticRing(molecule, first_idx) || other_arom_first == 1)) ||
                (aromaticity1 == 0 && !atomInAromaticRing(molecule, first_idx)))
                return true;

        if (list2.find(first_atom) >= 0)
            if (aromaticity2 == -1 || (aromaticity2 == 1 && (atomInAromaticRing(molecule, first_idx) || other_arom_first == 1)) ||
                (aromaticity2 == 0 && !atomInAromaticRing(molecule, first_idx)))
                return true;

        return false;
    }

    return true;
}

bool TautomerRule::atomInAromaticRing(BaseMolecule& mol, int atom_idx)
{
    if (atom_idx < 0)
        return true;

    return mol.getAtomAromaticity(atom_idx) == ATOM_AROMATIC;
}

TautomerMatcher::TautomerMatcher(TautomerSearchContext& context) : _d(context), _n_chains(0)
{
    _d.start_path_number = 0;

    _d.context.chains_2.clear_resize(_d.context.g2.vertexEnd());
    _d.context.chains_2.zerofill();

    _d.context.core_1.clear_resize(_d.context.g1.vertexEnd());
    _d.context.core_2.clear_resize(_d.context.g2.vertexEnd());

    _d.context.initial_g1_vertexend = _d.context.g1.vertexEnd();

    MoleculeSubstructureMatcher::markIgnoredHydrogens(_d.context.g1, _d.context.core_1.ptr(), EmbeddingEnumerator::UNMAPPED, EmbeddingEnumerator::IGNORE);

    MoleculeSubstructureMatcher::markIgnoredHydrogens(_d.context.g2, _d.context.core_2.ptr(), EmbeddingEnumerator::UNMAPPED, EmbeddingEnumerator::IGNORE);

    MoleculeTautomerUtils::countHReplacements(_d.context.g1, _d.context.h_rep_count_1);
    MoleculeTautomerUtils::countHReplacements(_d.context.g2, _d.context.h_rep_count_2);
}

TautomerMatcher::TautomerMatcher(TautomerSearchContext& context, int start_path_number, int n_chains) : _d(context), _n_chains(n_chains + 1)
{
    _d.start_path_number = start_path_number;
}

TautomerMatcher::~TautomerMatcher()
{
}

bool TautomerMatcher::_checkInterPathBonds()
{
    int idx1, idx2;
    BaseMolecule& g1 = _d.context.g1;
    BaseMolecule& g2 = _d.context.g2;

    // TODO: can check only for last chain
    for (idx1 = g1.edgeBegin(); idx1 < g1.edgeEnd(); idx1 = g1.edgeNext(idx1))
    {
        const Edge& edge = g1.getEdge(idx1);

        if (_d.context.core_1[edge.beg] < 0)
            continue;

        if (_d.context.core_1[edge.end] < 0)
            continue;

        if (abs(_d.context.chains_2[_d.context.core_1[edge.beg]] - _d.context.chains_2[_d.context.core_1[edge.end]]) != 1)
        {
            idx2 = g2.findEdgeIndex(_d.context.core_1[edge.beg], _d.context.core_1[edge.end]);

            if (idx2 == -1 || !matchBondsTau(g1, g2, idx1, idx2, &_d))
                return false;
        }
    }

    if (!_d.context.substructure)
        for (idx2 = g2.edgeBegin(); idx2 < g2.edgeEnd(); idx2 = g2.edgeNext(idx2))
        {
            const Edge& edge = g2.getEdge(idx2);

            if (_d.context.core_2[edge.beg] < 0)
                continue;

            if (_d.context.core_2[edge.end] < 0)
                continue;

            if (abs(_d.context.chains_2[edge.beg] - _d.context.chains_2[edge.end]) != 1)
            {
                idx1 = g1.findEdgeIndex(_d.context.core_2[edge.beg], _d.context.core_2[edge.end]);

                if (idx1 == -1 || !matchBondsTau(g1, g2, idx1, idx2, &_d))
                    return false;
            }
        }

    return true;
}

bool TautomerMatcher::nextPair(int& n1, int& n2, int& h_diff, int prev_n1, int prev_n2)
{
    if (prev_n1 == -1)
        n1 = _d.context.g1.vertexBegin();

    if (prev_n2 == -1)
        n2 = _d.context.g2.vertexBegin();
    else
        n2 = _d.context.g2.vertexNext(prev_n2);

    for (; n1 < _d.context.g1.vertexEnd(); n1 = _d.context.g1.vertexNext(n1))
    {
        for (; n2 < _d.context.g2.vertexEnd(); n2 = _d.context.g2.vertexNext(n2))
            if (isFeasiblePair(n1, n2, h_diff))
                return true;
        n2 = _d.context.g2.vertexBegin();
    }

    return false;
}

bool TautomerMatcher::isFeasiblePair(int n1, int n2, int& h_diff)
{
    if (_d.context.core_1[n1] != EmbeddingEnumerator::UNMAPPED || _d.context.core_2[n2] != EmbeddingEnumerator::UNMAPPED)
        return false;

    int charge1 = _d.context.g1.getAtomCharge(n1);
    int charge2 = _d.context.g2.getAtomCharge(n2);

    if (!matchAtomsTau(_d.context.g1, _d.context.g2, n1, n2, _d.context.strict))
        return false;

    int h_count_1 = _d.context.g1.getAtomTotalH(n1);
    int h_count_2 = _d.context.g2.getAtomTotalH(n2);

    if (!_d.context.force_hydrogens)
    {
        h_count_1 += _d.context.h_rep_count_1[n1];
        h_count_2 += _d.context.h_rep_count_2[n2];
    }
    else if (charge1 != charge2)
        return false;

    h_diff = h_count_1 - h_count_2;

    if (abs(h_diff) != 1)
        return false;

    return true;
}

void TautomerMatcher::addPair(int n1, int n2, int arom_bond_idx2, int bond_type2)
{
    _n2 = _d.context.core_1[n1] = n2;
    _n1 = _d.context.core_2[n2] = n1;

    _d.context.chains_2[n2] = _d.start_path_number;

    _d.start_path_number++;

    _bond_idx2 = arom_bond_idx2;

    if (_bond_idx2 >= 0)
        _d.context.dearomatizationMatcher->fixBond(_bond_idx2, bond_type2);

#ifdef TRACE_TAUTOMER_MATCHING
    for (int i = 0; i < _d.start_path_number; i++)
        printf("  ");
    printf("%2d\n", n1 + 1);
    for (int i = 0; i < _d.start_path_number; i++)
        printf("  ");
    printf("%2d\n", n2 + 1);
#endif
}

void TautomerMatcher::restore()
{
    _d.context.core_1[_n1] = EmbeddingEnumerator::UNMAPPED;
    _d.context.core_2[_n2] = EmbeddingEnumerator::UNMAPPED;

    _d.context.chains_2[_n2] = 0;

    if (_bond_idx2 >= 0)
        _d.context.dearomatizationMatcher->unfixBond(_bond_idx2);
}

bool TautomerMatcher::findMatch()
{
    int n1 = -1, n2 = -1;
    int h_difference;
    BaseMolecule& g1 = _d.context.g1;
    BaseMolecule& g2 = _d.context.g2;

    if (!_checkInterPathBonds())
        return true;

    EmbeddingEnumerator ee(g2);
    ee.setSubgraph(g1);

    int i;
    for (i = g1.vertexBegin(); i < g1.vertexEnd(); i = g1.vertexNext(i))
    {
        int val = _d.context.core_1[i];

        if (val == EmbeddingEnumerator::IGNORE)
            ee.ignoreSubgraphVertex(i);
        else if (val >= 0)
            ee.unsafeFix(i, val);
    }
    for (i = g2.vertexBegin(); i < g2.vertexEnd(); i = g2.vertexNext(i))
    {
        int val = _d.context.core_2[i];

        if (val == EmbeddingEnumerator::IGNORE)
            ee.ignoreSupergraphVertex(i);
    }

    ee.userdata = &_d;

    if (_d.context.substructure)
    {
        ee.userdata = &_d;
        ee.cb_match_edge = matchBondsTauSub;
        ee.cb_match_vertex = _matchAtoms;
        ee.cb_embedding = _preliminaryEmbedding;
        if (!ee.process())
            return false;
    }
    else
    {
        ee.cb_match_edge = matchBondsTau;
        ee.cb_match_vertex = _matchAtomsEx;
        ee.cb_embedding = _remainderEmbedding;

        if (!ee.process())
            return false;

        if (_d.context.max_chains > 0 && _n_chains >= _d.context.max_chains)
            return true;

        while (nextPair(n1, n2, h_difference, n1, n2))
        {
            TautomerChainFinder pe(_d.context, h_difference, _d.start_path_number, _n_chains);

            // pe.addPair(n1, n2, false, -1, 0);
            pe.addPair(n1, n2, _d.start_path_number != 0 || !_d.context.ring_chain, -1, 0);

            if (!pe.enumeratePaths())
                return false;

            pe.restore();
        }
    }

    return true;
}

bool TautomerMatcher::fixBondsNotInChains(TautomerSearchContext& context, const int* /*core1*/, const int* core2)
{
    bool ok = true;

    QS_DEF(Array<int>, fixed_bonds);

    fixed_bonds.clear();

    for (int i = context.g2.edgeBegin(); i < context.g2.edgeEnd(); i = context.g2.edgeNext(i))
    {
        const Edge& edge2 = context.g2.getEdge(i);

        if (context.g2.getBondOrder(i) != BOND_AROMATIC)
            continue;

        if (abs(context.chains_2[edge2.beg] - context.chains_2[edge2.end]) == 1)
            continue;

        if (core2[edge2.beg] < 0 || core2[edge2.end] < 0)
            continue;

        const Vertex& vert_beg1 = context.g1.getVertex(core2[edge2.beg]);

        int nei_idx1;

        if ((nei_idx1 = vert_beg1.findNeiVertex(core2[edge2.end])) < 0)
            continue;

        int edge_idx1 = vert_beg1.neiEdge(nei_idx1);

        // query bond?
        if (context.g1.getBondOrder(edge_idx1) == -1)
            continue;

        int type1;

        if ((type1 = context.g1.getBondOrder(edge_idx1)) == BOND_AROMATIC)
            continue;

        if (!context.dearomatizationMatcher->isAbleToFixBond(i, type1))
        {
            ok = false;
            break;
        }
        else
        {
            context.dearomatizationMatcher->fixBond(i, type1);
            fixed_bonds.push(i);
        }
    }

    if (!ok)
    {
        for (int i = 0; i < fixed_bonds.size(); i++)
            context.dearomatizationMatcher->unfixBond(fixed_bonds[i]);
    }

    return ok;
}
