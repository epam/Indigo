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
#include "molecule/molecule_arom.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_exact_matcher.h"
#include "molecule/molecule_substructure_matcher.h"
#include "molecule/molecule_tautomer.h"
#include "molecule/molecule_tautomer_utils.h"

using namespace indigo;

enum
{
    BOND_ZEROED = 0 // Bond can appear during tautomerism
};

TautomerChainFinder::TautomerChainFinder(TautomerSearchContext& context, int h_difference, int start_path_number, int n_chains)
    : _context(context), _path_length(0), _h_difference(h_difference), _is_zero_bond_present(!context.ring_chain), _path_number(start_path_number + 2),
      _n_chains(n_chains)
{
}

TautomerChainFinder::TautomerChainFinder(TautomerChainFinder& other)
    : _context(other._context), _path_length(other._path_length + 1), _h_difference(other._h_difference * -1), _path_number(other._path_number),
      _start_idx1(other._start_idx1), _start_idx2(other._start_idx2), _n_chains(other._n_chains)
{
}

TautomerChainFinder::~TautomerChainFinder()
{
}

bool TautomerChainFinder::enumeratePaths()
{
    int n1, n2, e1 = -1, e2 = -1;

    while (nextPair(n1, n2, e1, e2, e1, e2))
    {
        int rc;
        bool zero_bond;
        int arom_bond2, bond_type2 = -1;

        if ((rc = isFeasiblePair(n1, n2, zero_bond, arom_bond2, bond_type2)) == 1)
        {
            TautomerChainFinder pe(*this);

            pe.addPair(n1, n2, zero_bond, arom_bond2, bond_type2);

            if (!pe.enumeratePaths())
                return false;

            pe.restore();
        }
        else if (rc == -1)
        {
            TautomerMatcher tm_state(_context, _path_number, _n_chains);

            tm_state.addPair(n1, n2, arom_bond2, bond_type2);

            if (!tm_state.findMatch())
                return false;

            tm_state.restore();
        }
    }
    return true;
}

bool TautomerChainFinder::nextPair(int& n1, int& n2, int& e1, int& e2, int prev_e1, int prev_e2)
{
    const Vertex& vertex1 = _context.g1.getVertex(_prev_n1);
    const Vertex& vertex2 = _context.g2.getVertex(_prev_n2);

    if (_is_zero_bond_present)
    {
        if (prev_e1 == -1)
            e1 = vertex1.neiBegin();

        if (prev_e2 == -1)
            e2 = vertex2.neiBegin();
        else
            e2 = vertex2.neiNext(prev_e2);

        for (; e1 != vertex1.neiEnd(); e1 = vertex1.neiNext(e1))
        {
            n1 = vertex1.neiVertex(e1);

            if (_context.core_1[n1] != EmbeddingEnumerator::UNMAPPED)
                continue;

            for (; e2 != vertex2.neiEnd(); e2 = vertex2.neiNext(e2))
            {
                n2 = vertex2.neiVertex(e2);

                if (_context.core_2[n2] != EmbeddingEnumerator::UNMAPPED)
                    continue;

                if (TautomerMatcher::matchAtomsTau(_context.g1, _context.g2, n1, n2))
                    return true;
            }

            e2 = vertex2.neiBegin();
        }
    }
    else if (_h_difference == 1)
    {
        if (prev_e1 == -1)
            e1 = _context.g1.vertexBegin();

        if (prev_e2 == -1)
            e2 = vertex2.neiBegin();
        else
            e2 = vertex2.neiNext(prev_e2);

        for (; e1 < _context.g1.vertexEnd(); e1 = _context.g1.vertexNext(e1))
        {
            n1 = e1;

            if (_context.core_1[n1] != EmbeddingEnumerator::UNMAPPED)
                continue;

            for (; e2 != vertex2.neiEnd(); e2 = vertex2.neiNext(e2))
            {
                n2 = vertex2.neiVertex(e2);

                if (_context.core_2[n2] != EmbeddingEnumerator::UNMAPPED)
                    continue;

                if (TautomerMatcher::matchAtomsTau(_context.g1, _context.g2, n1, n2))
                    return true;
            }

            e2 = vertex2.neiBegin();
        }
    }
    else
    {
        if (prev_e1 == -1)
            e1 = vertex1.neiBegin();

        if (prev_e2 == -1)
            e2 = _context.g2.vertexBegin();
        else
            e2 = _context.g2.vertexNext(prev_e2);

        for (; e1 != vertex1.neiEnd(); e1 = vertex1.neiNext(e1))
        {
            n1 = vertex1.neiVertex(e1);

            if (_context.core_1[n1] != EmbeddingEnumerator::UNMAPPED)
                continue;

            for (; e2 < _context.g2.vertexEnd(); e2 = _context.g2.vertexNext(e2))
            {
                n2 = e2;

                if (_context.core_2[n2] != EmbeddingEnumerator::UNMAPPED)
                    continue;

                if (TautomerMatcher::matchAtomsTau(_context.g1, _context.g2, n1, n2))
                    return true;
            }

            e2 = _context.g2.vertexBegin();
        }
    }

    return false;
}

int TautomerChainFinder::isFeasiblePair(int n1, int n2, bool& zero_bond, int& arom_bond_idx2, int& bond_type2)
{
    if (_prev_n1 != -1 && _context.decomposer1.getComponent(_prev_n1) != _context.decomposer1.getComponent(n1))
        return 0;
    if (_prev_n2 != -1 && _context.decomposer2.getComponent(_prev_n2) != _context.decomposer2.getComponent(n2))
        return 0;

    int edge1 = _context.g1.findEdgeIndex(_prev_n1, n1);
    int edge2 = _context.g2.findEdgeIndex(_prev_n2, n2);

    int type1 = 0;
    int type2 = 0;

    zero_bond = _is_zero_bond_present;
    arom_bond_idx2 = -1;

    if (edge1 == -1)
    {
        zero_bond = true;
    }
    else
        type1 = _context.g1.getBondOrder(edge1);

    if (type1 == BOND_AROMATIC || type1 == BOND_TRIPLE)
        return 0;

    if (edge2 == -1)
    {
        zero_bond = true;
    }
    else if (_context.g2.getBondOrder(edge2) == -1 && _context.g2.possibleBondOrder(edge2, BOND_SINGLE))
    {
        if (_is_zero_bond_present || type1 == 0)
            return 0;

        zero_bond = true;
    }
    else
        type2 = _context.g2.getBondOrder(edge2);

    if (type2 == BOND_TRIPLE)
        return 0;

    if (type2 == BOND_AROMATIC)
        arom_bond_idx2 = edge2;

    if (edge1 == -1 && type2 == BOND_AROMATIC)
    {
        if (!_context.dearomatizationMatcher->isAbleToFixBond(edge2, 1))
            return 0;

        bond_type2 = 1;
    }

    if (type2 != BOND_AROMATIC) // both non-aromatic
    {
        if (type1 + _h_difference != type2)
            return 0;
    }
    else // exactly one aromatic
    {
        bond_type2 = type1 + _h_difference;

        if (bond_type2 < 1 || bond_type2 > 2)
            return 0;

        if (!_context.dearomatizationMatcher->isAbleToFixBond(edge2, bond_type2))
            return 0;
    }

    int h_count_1 = _context.g1.getAtomTotalH(n1);
    int h_count_2 = _context.g2.getAtomTotalH(n2);

    if (!_context.force_hydrogens)
    {
        h_count_1 += _context.h_rep_count_1[n1];
        h_count_2 += _context.h_rep_count_2[n2];
    }

    if (h_count_1 == h_count_2)
        return (_context.g1.possibleAtomCharge(n1, _context.g2.getAtomCharge(n2)) ? 1 : 0);
    if ((_path_length & 1) && h_count_1 - _h_difference == h_count_2)
    {
        if (_context.cb_check_rules == 0 || _context.cb_check_rules(_context, _start_idx1, _start_idx2, n1, n2))
            return -1;
    }

    return 0;
}

void TautomerChainFinder::addPair(int n1, int n2, bool is_zero_bond, int arom_bond_idx2, int bond_type2)
{
    _context.core_1[n1] = n2;
    _context.core_2[n2] = n1;

    _context.chains_2[n2] = _path_number;

    _path_number++;

    _prev_n1 = n1;
    _prev_n2 = n2;

    _is_zero_bond_present = is_zero_bond;

    if (_path_length == 0)
    {
        _start_idx1 = n1;
        _start_idx2 = n2;
    }

#ifdef TRACE_TAUTOMER_MATCHING
    for (int i = 0; i < _path_number; i++)
        printf("  ");
    printf("%2d\n", n1 + 1);
    for (int i = 0; i < _path_number; i++)
        printf("  ");
    printf("%2d\n", n2 + 1);
#endif

    _bond_idx2 = arom_bond_idx2;

    if (_bond_idx2 >= 0)
        _context.dearomatizationMatcher->fixBond(_bond_idx2, bond_type2);
}

void TautomerChainFinder::restore()
{
    _context.core_1[_prev_n1] = EmbeddingEnumerator::UNMAPPED;
    _context.core_2[_prev_n2] = EmbeddingEnumerator::UNMAPPED;

    _context.chains_2[_prev_n2] = 0;

    if (_bond_idx2 >= 0)
        _context.dearomatizationMatcher->unfixBond(_bond_idx2);
}

IMPL_ERROR(TautomerChainChecker, "tautomer chain checker");

TautomerChainChecker::TautomerChainChecker(TautomerSearchContext& context, const Array<int>& core1, const Array<int>& core2, int start_path_number)
    : _context(context), _path_length(0), _h_difference(0), _is_zero_bond_present(!context.ring_chain), _is_query_bond_present(false),
      _is_non_aromatic_bond_present(false), _path_number(start_path_number + 2), _core_1(core1), _core_2(core2), _tau_bonds_to_match(0), _prev_n1(-1),
      _prev_n2(-1), _bond_idx1(-1), _bond_idx2(-1), _bond_type2(0)
{
    // calculate tautomer bonds
    for (int i = _context.g1.edgeBegin(); i < _context.g1.edgeEnd(); i = _context.g1.edgeNext(i))
    {
        int bond_1 = _context.g1.getBondOrder(i);

        if (bond_1 == -1)
            continue;

        const Edge& edge_1 = _context.g1.getEdge(i);

        int beg = _core_1[edge_1.beg];
        int end = _core_1[edge_1.end];

        if (beg < 0 || end < 0)
            continue;

        if (_context.chains_2[beg] > 0 && _context.chains_2[end] > 0)
            continue;

        const Vertex& vert_2_beg = _context.g2.getVertex(beg);

        int edge_2_idx = vert_2_beg.neiEdge(vert_2_beg.findNeiVertex(end));

        int bond_2 = _context.g2.getBondOrder(edge_2_idx);

        if (bond_1 != bond_2 && bond_1 != BOND_AROMATIC && bond_2 != BOND_AROMATIC)
            _tau_bonds_to_match++;
    }

    _context.core_1.copy(_core_1.ptr(), _context.initial_g1_vertexend);
    // see the comment in TautomerMatcher::_preliminaryEmbedding
    _context.core_2.copy(_core_2.ptr(), _context.g2.vertexEnd());
}

TautomerChainChecker::TautomerChainChecker(TautomerChainChecker& other)
    : _context(other._context), _path_length(other._path_length), _h_difference(other._h_difference * -1), _is_zero_bond_present(other._is_zero_bond_present),
      _is_query_bond_present(other._is_query_bond_present), _is_non_aromatic_bond_present(other._is_non_aromatic_bond_present),
      _path_number(other._path_number), _core_1(other._core_1), _core_2(other._core_2), _tau_bonds_to_match(other._tau_bonds_to_match), _prev_n1(-1),
      _prev_n2(-1), _bond_idx1(-1), _bond_idx2(-1), _bond_type2(0), _start_idx1(other._start_idx1), _start_idx2(other._start_idx2)
{
}

TautomerChainChecker::~TautomerChainChecker()
{
}

bool TautomerChainChecker::check()
{
    int n1 = -1, n2 = -1;

    if (!_checkInterPathBonds())
        return true;

    if (_tau_bonds_to_match == 0 && _path_length == 0 && _matchAromatizedQuery())
    {
        _context.core_1.copy(_core_1);
        _context.core_2.copy(_core_2);

        return false;
    }

    if (_path_length == 0)
    {
        int diff;

        while (nextStartingPair(n1, n2))
            if (isFeasibleStartingPair(n1, n2, diff))
            {
                TautomerChainChecker cc(*this);

                cc._path_number++;
                cc.addPair(n1, n2);
                cc._path_length++;
                cc._h_difference = -diff;

                if (!cc.check())
                    return false;

                cc.restore();
            }
    }
    else
    {
        int e1 = -1, e2 = -1;
        int rc;

        while (nextPair(n1, n2, e1, e2))
        {
            TautomerChainChecker cc1(*this);
            TautomerChainChecker cc2(*this);

            rc = isFeasiblePair(n1, n2, cc1, cc2);

            if (rc & 1) // intermediate pair in chain
            {
                cc1.addPair(n1, n2);

                if (!cc1.check())
                    return false;

                cc1.restore();
            }
            if (rc & 2) // last pair in chain
            {
                cc2.addPair(n1, n2);

                if (cc2.releaseChain())
                    if (!cc2.check())
                        return false;

                cc2.restoreChain();
                cc2.restore();
            }
        }
    }

    return true;
}

bool TautomerChainChecker::nextStartingPair(int& n1, int& n2)
{
    if (n2 == -1)
        n2 = _context.g2.vertexBegin();
    else
        n2 = _context.g2.vertexNext(n2);

    // n1 = _core_2[n2];

    for (; n2 < _context.g2.vertexEnd(); n2 = _context.g2.vertexNext(n2))
    {
        n1 = _core_2[n2];

        if (n1 != EmbeddingEnumerator::IGNORE && _context.chains_2[n2] <= 0)
            break;
    }

    if (n2 >= _context.g2.vertexEnd())
        return false;

    if (n1 < 0)
    {
        if (n1 != EmbeddingEnumerator::UNMAPPED && n1 != EmbeddingEnumerator::TERM_OUT)
            throw Error("some strange situation");

        n1 = -1;
    }

    return true;
}

bool TautomerChainChecker::isFeasibleStartingPair(int n1, int n2, int& h_diff)
{
    // if (n2 != -1 && _context.chains_2[n2] > 0)
    //   return false;

    if (n1 < 0)
    {
        h_diff = 0;
        return true;
    }

    if (!TautomerMatcher::matchAtomsTau(_context.g1, _context.g2, n1, n2))
        return false;

    int h_count_2 = _context.g2.getAtomTotalH(n2);
    int h_rep_1 = 0;

    if (!_context.force_hydrogens)
    {
        h_rep_1 = _context.h_rep_count_1[n1];
        h_count_2 += _context.h_rep_count_2[n2];
    }
    else if (!_context.g1.possibleAtomCharge(n1, _context.g2.getAtomCharge(n2)))
        return false;

    bool more = false, less = false;

    if (_context.g1.possibleAtomTotalH(n1, h_count_2 - h_rep_1 + 1))
        more = true;
    if (_context.g1.possibleAtomTotalH(n1, h_count_2 - h_rep_1 - 1))
        less = true;

    if (!less && !more)
        return false;

    if (less && more)
        h_diff = 0;
    else if (more)
        h_diff = -1;
    else
        h_diff = 1;

    return true;
}

bool TautomerChainChecker::nextPair(int& n1, int& n2, int& e1, int& e2)
{
    const Vertex& vertex2 = _context.g2.getVertex(_prev_n2);

    if (e2 == -1)
        e2 = vertex2.neiBegin();
    else
        e2 = vertex2.neiNext(e2);

    for (; e2 != vertex2.neiEnd(); e2 = vertex2.neiNext(e2))
    {
        n2 = vertex2.neiVertex(e2);

        if (_context.chains_2[n2] == 0 && _core_2[n2] != EmbeddingEnumerator::IGNORE)
            break;
    }

    if (e2 == vertex2.neiEnd())
        return false;

    e1 = -1;

    if ((n1 = _core_2[n2]) < 0)
        n1 = -1;
    else if (_prev_n1 != -1)
    {
        const Vertex& vertex1 = _context.g1.getVertex(_prev_n1);

        e1 = vertex1.findNeiEdge(_core_2[n2]);
    }

    return true;
}

int TautomerChainChecker::isFeasiblePair(int n1, int n2, TautomerChainChecker& next1, TautomerChainChecker& next2)
{
    // if (n1 == -1 && _prev_n1 == -1)
    //   return 0;

    // if (n2 != -1 && _context.chains_2[n2] > 0)
    //   return 0;

    if (_prev_n1 != -1 && n1 != -1 && _context.decomposer1.getComponent(_prev_n1) != _context.decomposer1.getComponent(n1))
        return 0;
    if (_prev_n2 != -1 && n2 != -1 && _context.decomposer2.getComponent(_prev_n2) != _context.decomposer2.getComponent(n2))
        return 0;

    if (_context.g2.isPseudoAtom(n2) || _context.g2.isTemplateAtom(n2)) // n1 are checked in matchAtomsTau later
        return 0;

    int h_diff = _h_difference;

    int edge2 = _context.g2.findEdgeIndex(_prev_n2, n2);

    int type1 = 0;
    int type2 = 0;

    bool zero_bond = _is_zero_bond_present;
    bool query_bond = _is_query_bond_present;
    bool na_bond = _is_non_aromatic_bond_present;

    int bond_type2 = 0;

    bool tau_bond = true;

    if (edge2 == -1)
    {
        zero_bond = true;
    }
    else
    {
        type2 = _context.g2.getBondOrder(edge2);

        if (type2 == -1 && _context.g2.possibleBondOrder(edge2, BOND_SINGLE))
            type2 = BOND_ZEROED;

        if (type2 == BOND_ZEROED)
        {
            if (_is_zero_bond_present)
                return 0;

            zero_bond = true;
        }
    }

    int edge1 = -1;

    if (n1 != -1)
    {
        if (_prev_n1 != -1)
            edge1 = _context.g1.findEdgeIndex(_prev_n1, n1);

        if (edge1 != -1)
        {
            type1 = _context.g1.getBondOrder(edge1);

            if (type1 == -1)
                return 0;
        }
    }

    if (type1 == BOND_AROMATIC || type1 == BOND_TRIPLE || type2 == BOND_TRIPLE)
        return 0;

    // Check degrees on previous vertices
    if (_prev_n1 != -1 && _path_length != 1)
    {
        const Vertex& prev_vert1 = _context.g1.getVertex(_prev_n1);
        const Vertex& prev_vert2 = _context.g2.getVertex(_prev_n2);

        int i;

        int bond_count1 = prev_vert1.degree();
        int prev_value_chain = _context.chains_2[_core_1[_prev_n1]];

        for (i = prev_vert1.neiBegin(); i < prev_vert1.neiEnd(); i = prev_vert1.neiNext(i))
        {
            int value_chain = _context.chains_2[_core_1[prev_vert1.neiVertex(i)]];

            if (value_chain > 0 && abs(value_chain - prev_value_chain) == 1)
                bond_count1--;
        }

        int bond_count2 = prev_vert2.degree();

        for (i = prev_vert2.neiBegin(); i < prev_vert2.neiEnd(); i = prev_vert2.neiNext(i))
        {
            int value_chain = _context.chains_2[prev_vert2.neiVertex(i)];

            if ((value_chain > 0 && abs(value_chain - prev_value_chain) == 1) ||
                (_context.g2.getBondOrder(prev_vert2.neiEdge(i)) == -1 && _context.g2.possibleBondOrder(prev_vert2.neiEdge(i), BOND_SINGLE)))
                bond_count2--;
        }

        if (type1 == 0)
            bond_count1++;

        if (type2 == 0)
            bond_count2++;

        // if (bond_count1 != bond_count2)
        if (bond_count1 > bond_count2 || bond_count2 - bond_count1 > 1)
            return 0;
    }

    // zero bond must be
    if (edge1 == -1 && type2 == BOND_SINGLE && _h_difference == 1)
    {
        if (zero_bond)
            return 0;

        zero_bond = true;
    }

    if (zero_bond && !_is_zero_bond_present)
    {
        // Assume that aromatic ring cannot be destroyed
        if (type2 > 1)
            return 0;
        if (type2 == BOND_SINGLE)
        {
            if (_h_difference != 0)
            {
                if (_h_difference != 1)
                    return 0;
            }
            else
                h_diff = 1;
        }
    }

    if (edge1 == -1 || type2 == BOND_AROMATIC)
        tau_bond = false;

    if (edge1 != -1)
        query_bond = true;

    if (type2 != BOND_AROMATIC)
        na_bond = true;

    if (type1 != 0)
    {
        if (type2 != BOND_AROMATIC) // both non-aromatic
        {
            if (_h_difference != 0)
            {
                if (type1 + _h_difference != type2)
                    return 0;
            }
            else if (abs(h_diff = (type2 - type1)) != 1)
                return 0;
        }
        else // exactly one aromatic
        {
            if (_h_difference != 0)
            {
                bond_type2 = type1 + _h_difference;

                if (bond_type2 < 1 || bond_type2 > 2)
                    return 0;

                if (!_context.dearomatizationMatcher->isAbleToFixBond(edge2, bond_type2))
                    return 0;
            }
            else
            {
                if (type1 == BOND_SINGLE)
                {
                    h_diff = 1;
                    bond_type2 = 2;
                }
                else if (type1 == BOND_DOUBLE)
                {
                    h_diff = -1;
                    bond_type2 = 1;
                }

                if (bond_type2 > 0 && !_context.dearomatizationMatcher->isAbleToFixBond(edge2, bond_type2))
                    return 0;
            }
        }
    }

    int rc = 0;

    if (n1 != -1)
    {
        if (!TautomerMatcher::matchAtomsTau(_context.g1, _context.g2, n1, n2))
            return 0;

        int h_rep_1 = 0;
        int h_count_2 = _context.g2.getAtomTotalH(n2);

        if (!_context.force_hydrogens)
        {
            h_rep_1 = _context.h_rep_count_1[n1];
            h_count_2 += _context.h_rep_count_2[n2];
        }

        if (query_bond && na_bond)
        {
            if (h_diff != 0)
            {
                if (!(_path_length & 1))
                    if (_context.g1.possibleAtomTotalH(n1, h_count_2 + h_diff - h_rep_1))
                    {
                        if (_context.cb_check_rules == 0 || _context.cb_check_rules(_context, _start_idx1, _start_idx2, n1, n2))
                            rc += 2;
                    }
            }
            else if (!(_path_length & 1))
                if (_context.g1.possibleAtomTotalH(n1, h_count_2 + 1 - h_rep_1) || _context.g1.possibleAtomTotalH(n1, h_count_2 - 1 - h_rep_1))
                {
                    if (_context.cb_check_rules == 0 || _context.cb_check_rules(_context, _start_idx1, _start_idx2, n1, n2))
                        rc += 2;
                }
        }

        if (_context.g1.possibleAtomTotalH(n1, h_count_2 - h_rep_1))
        {
            int charge_2 = _context.g2.getAtomCharge(n2);

            if (_context.g1.possibleAtomCharge(n1, charge_2))
                rc++;
        }
    }
    else
    {
        rc++;

        if (!(_path_length & 1) && query_bond && na_bond)
        {
            if (_context.cb_check_rules == 0 || _context.cb_check_rules(_context, _start_idx1, _start_idx2, n1, n2))
                rc += 2;
        }
    }

    if (rc & 1)
    {
        next1._path_length++;
        next1._h_difference = -h_diff;
        next1._is_zero_bond_present = zero_bond;
        next1._is_query_bond_present = query_bond;
        next1._is_non_aromatic_bond_present = na_bond;

        if (tau_bond)
            next1._tau_bonds_to_match--;

        next1._bond_idx1 = edge1;
        next1._bond_idx2 = edge2;
        next1._bond_type2 = bond_type2 > 0 ? bond_type2 : type2;
    }

    if (rc & 2)
    {
        next2._final_path_length = next2._path_length + 1;
        next2._path_length = 0;
        next2._h_difference = 0;
        next2._final_h_difference = -h_diff;
        next2._is_query_bond_present = false;
        next2._is_non_aromatic_bond_present = false;
        // next2._is_zero_bond_present = false;
        next2._is_zero_bond_present = true;

        if (tau_bond)
            next2._tau_bonds_to_match--;

        next2._bond_idx1 = edge1;
        next2._bond_idx2 = edge2;
        next2._bond_type2 = bond_type2 > 0 ? bond_type2 : type2;
    }

    return rc;
}

void TautomerChainChecker::addPair(int n1, int n2)
{
    _context.chains_2[n2] = _path_number;

    _context.n1.expand(_path_number + 1);
    _context.n2.expand(_path_number + 1);
    _context.n1[_path_number] = n1;
    _context.n2[_path_number] = n2;

    _context.edges_1.expand(_path_number + 1);
    _context.edges_2.expand(_path_number + 1);
    _context.edge_types_2.expand(_path_number + 1);

    _context.edges_1[_path_number] = _bond_idx1;
    _context.edges_2[_path_number] = _bond_idx2;
    _context.edge_types_2[_path_number] = _bond_type2;

    _path_number++;

    _prev_n1 = n1;
    _prev_n2 = n2;

    if (_path_length == 0)
    {
        _start_idx1 = n1;
        _start_idx2 = n2;
    }

    if (_bond_type2 > 0 && _bond_type2 != BOND_AROMATIC)
        _context.dearomatizationMatcher->fixBond(_bond_idx2, _bond_type2);

#ifdef TRACE_TAUTOMER_MATCHING
    for (int i = 0; i < _path_number; i++)
        printf("  ");
    printf("%2d\n", n1 + 1);
    for (int i = 0; i < _path_number; i++)
        printf("  ");
    printf("%2d\n", n2 + 1);
#endif
}

void TautomerChainChecker::restore()
{
    _context.chains_2[_prev_n2] = 0;
    if (_bond_type2 > 0 && _bond_type2 != BOND_AROMATIC)
        _context.dearomatizationMatcher->unfixBond(_bond_idx2);
}

// Make chain as in target
bool TautomerChainChecker::releaseChain()
{
    if (_final_h_difference == 0)
        throw Error("unknown hydrogen difference");

    // if (_bond_type1 < 0 || _bond_type1 > 2 || _bond_type2 == BOND_AROMATIC)
    //   throw Error("bad bond type");

    int i = 1;
    int type, idx1, idx2, new_idx;
    int beg, end;
    BaseMolecule& query = _context.g1;
    BaseMolecule& target = _context.g2;
    int h_diff = _final_h_difference;
    bool res = true;

    for (i = 1; i < _final_path_length; i++, h_diff *= -1)
    {
        type = _context.edge_types_2[_path_number - i];
        idx1 = _context.edges_1[_path_number - i];
        idx2 = _context.edges_2[_path_number - i];

        if (type == BOND_AROMATIC)
        {
            if (idx1 < 0)
            {
                bool can_be_single = _context.dearomatizationMatcher->isAbleToFixBond(idx2, BOND_SINGLE);
                bool can_be_double = _context.dearomatizationMatcher->isAbleToFixBond(idx2, BOND_DOUBLE);

                if (can_be_single != can_be_double)
                    type = can_be_single ? BOND_SINGLE : BOND_DOUBLE;
            }
            else
            {
                if (h_diff == 1)
                    type = BOND_SINGLE;
                else
                    type = BOND_DOUBLE;
            }
        }

        if (type > 0 && idx1 < 0)
        {
            // Forbidden zero bond
            if (!_context.ring_chain && type == BOND_SINGLE && h_diff == -1)
                res = false;

            const Edge& edge2 = target.getEdge(idx2);

            beg = _context.core_2[edge2.beg];
            if (beg < 0)
            {
                if (_context.substructure)
                {
                    QueryMolecule& qmol = query.asQueryMolecule();

                    new_idx = qmol.addAtom(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, target.getAtomNumber(edge2.beg)));
                    if (target.getAtomCharge(edge2.beg) != 0)
                    {
                        qmol.resetAtom(new_idx, QueryMolecule::Atom::und(qmol.releaseAtom(new_idx),
                                                                         new QueryMolecule::Atom(QueryMolecule::ATOM_CHARGE, target.getAtomCharge(edge2.beg))));
                    }
                }
                else
                {
                    Molecule& mol = query.asMolecule();

                    new_idx = mol.addAtom(target.getAtomNumber(edge2.beg));
                    mol.setAtomCharge(new_idx, target.getAtomCharge(edge2.beg));
                }

                _context.core_2[edge2.beg] = new_idx;
                _context.core_1.expand(new_idx + 1);
                _context.core_1[new_idx] = edge2.beg;
            }

            end = _context.core_2[edge2.end];
            if (end < 0)
            {
                if (_context.substructure)
                {
                    QueryMolecule& qmol = query.asQueryMolecule();

                    new_idx = qmol.addAtom(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, target.getAtomNumber(edge2.end)));
                    if (target.getAtomCharge(edge2.end) != 0)
                    {
                        qmol.resetAtom(new_idx, QueryMolecule::Atom::und(qmol.releaseAtom(new_idx),
                                                                         new QueryMolecule::Atom(QueryMolecule::ATOM_CHARGE, target.getAtomCharge(edge2.end))));
                    }
                }
                else
                {
                    Molecule& mol = query.asMolecule();

                    new_idx = mol.addAtom(target.getAtomNumber(edge2.end));
                    mol.setAtomCharge(new_idx, target.getAtomCharge(edge2.end));
                }

                _context.core_2[edge2.end] = new_idx;
                _context.core_1.expand(new_idx + 1);
                _context.core_1[new_idx] = edge2.end;
            }

            int new_bond_idx;

            if (_context.substructure)
            {
                QueryMolecule& qmol = query.asQueryMolecule();

                new_bond_idx = qmol.addBond(_context.core_2[edge2.beg], _context.core_2[edge2.end], new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, type));
            }
            else
            {
                Molecule& mol = query.asMolecule();

                new_bond_idx = mol.addBond(_context.core_2[edge2.beg], _context.core_2[edge2.end], type);
            }

            _context.edges_1[_path_number - i] = -2 - new_bond_idx;
        }
        else if (idx1 >= 0)
        {
            if (_context.substructure)
            {
                QueryMolecule& qmol = query.asQueryMolecule();

                qmol.resetBond(idx1, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, type));
            }
            else
            {
                Molecule& mol = query.asMolecule();

                mol.setBondOrder(idx1, type);
            }
        }
    }
    return res;
}

// Restore original chain
void TautomerChainChecker::restoreChain()
{
    int diff = _final_h_difference;
    int i = 1;
    int type, idx1, vidx1, vidx2;
    BaseMolecule& query = _context.g1;

    for (i = 1; i < _final_path_length; i++, diff *= -1)
    {
        type = _context.edge_types_2[_path_number - i] + diff;
        idx1 = _context.edges_1[_path_number - i];

        if (idx1 < -1)
        {
            idx1 = -idx1 - 2;

            query.removeBond(idx1);
            _context.edges_1[_path_number - i] = -1;
        }
        else if (idx1 >= 0)
        {
            if (_context.substructure)
            {
                QueryMolecule& qmol = query.asQueryMolecule();

                qmol.resetBond(idx1, new QueryMolecule::Bond(QueryMolecule::BOND_ORDER, type));
            }
            else
            {
                Molecule& mol = query.asMolecule();

                mol.setBondOrder(idx1, type);
            }
        }

        if (_context.n1[_path_number - i] < 0)
        {
            vidx2 = _context.n2[_path_number - i];
            vidx1 = _context.core_2[vidx2];

            if (vidx1 >= 0)
            {
                query.removeAtom(vidx1);
                _context.core_1[vidx1] = -1;
                _context.core_2[vidx2] = -1;
            }
        }
    }

    if (_context.n1[_path_number - i] < 0)
    {
        vidx2 = _context.n2[_path_number - i];
        vidx1 = _context.core_2[vidx2];

        if (vidx1 >= 0)
        {
            query.removeAtom(vidx1);
            _context.core_1[vidx1] = -1;
            _context.core_2[vidx2] = -1;
        }
    }
}

bool TautomerChainChecker::_checkInterPathBonds()
{
    BaseMolecule& g1 = _context.g1;
    BaseMolecule& g2 = _context.g2;

    if (_prev_n1 != -1)
    {
        const Vertex& vertex = g1.getVertex(_prev_n1);

        int idx1, idx2;

        for (int i = vertex.neiBegin(); i < vertex.neiEnd(); i = vertex.neiNext(i))
        {
            int nei_idx = vertex.neiVertex(i);

            if (_context.core_1[nei_idx] < 0 || _context.chains_2[_context.core_1[nei_idx]] == 0)
                continue;

            if (abs(_context.chains_2[_context.core_1[_prev_n1]] - _context.chains_2[_context.core_1[nei_idx]]) != 1)
            {
                idx1 = vertex.neiEdge(i);
                idx2 = g2.findEdgeIndex(_context.core_1[_prev_n1], _context.core_1[nei_idx]);

                int type1 = g1.getBondOrder(idx1);
                int type2 = g2.getBondOrder(idx2);

                bool matched;

                if (_context.substructure)
                {
                    matched = MoleculeSubstructureMatcher::matchQueryBond(&(g1.asQueryMolecule()).getBond(idx1), g2, idx1, idx2, 0, 0xFFFFFFFFUL);
                }
                else
                {
                    matched = (g1.getBondOrder(idx1) == g2.getBondOrder(idx2));
                }

                if (!matched)
                {
                    if (type1 != BOND_AROMATIC && type2 == BOND_AROMATIC)
                    {
                        if (!_context.dearomatizationMatcher->isAbleToFixBond(idx2, type1))
                            return false;
                    }
                    else
                        return false;
                }
            }
        }
    }

    return true;
}

bool TautomerChainChecker::_matchAromBonds(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    AromaticityMatcher* am = (AromaticityMatcher*)userdata;
    QueryMolecule::Bond* bond = &((BaseMolecule&)subgraph).asQueryMolecule().getBond(sub_idx);

    return MoleculeSubstructureMatcher::matchQueryBond(bond, (BaseMolecule&)supergraph, sub_idx, super_idx, am, 0xFFFFFFFFUL);
}

void TautomerChainChecker::_removeAtom(Graph& subgraph, int sub_idx, void* userdata)
{
    AromaticityMatcher* am = (AromaticityMatcher*)userdata;

    MoleculeSubstructureMatcher::removeAtom(subgraph, sub_idx, am);
}

void TautomerChainChecker::_addBond(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    AromaticityMatcher* am = (AromaticityMatcher*)userdata;

    MoleculeSubstructureMatcher::addBond(subgraph, supergraph, sub_idx, super_idx, am);
}

int TautomerChainChecker::_embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata)
{
    AromaticityMatcher* am = (AromaticityMatcher*)userdata;
    QueryMolecule& query = ((BaseMolecule&)subgraph).asQueryMolecule();
    BaseMolecule& target = (BaseMolecule&)supergraph;

    if (!MoleculeStereocenters::checkSub(query, target, core_sub, false))
        return 1;

    if (!MoleculeCisTrans::checkSub(query, target, core_sub))
        return 1;

    // Check possible aromatic configuration
    if (am != 0)
    {
        if (!am->match(core_sub, core_super))
            return 1;
    }

    return 0;
}

bool TautomerChainChecker::_matchAromatizedQuery()
{
    QS_DEF(QueryMolecule, aromatized_query);
    QS_DEF(Array<int>, mapping);

    aromatized_query.clone(((BaseMolecule&)_context.g1).asQueryMolecule(), 0, &mapping);
    QueryMoleculeAromatizer::aromatizeBonds(aromatized_query, _context.arom_options);

    EmbeddingEnumerator ee(_context.g2);

    ee.setSubgraph(aromatized_query);

    AromaticityMatcher am(aromatized_query, (Molecule&)_context.g2, _context.arom_options);

    ee.userdata = &am;

    ee.cb_match_edge = _matchAromBonds;
    ee.cb_vertex_remove = _removeAtom;
    ee.cb_edge_add = _addBond;
    ee.cb_embedding = _embedding;

    int i;

    for (i = _context.g1.vertexBegin(); i < _context.g1.vertexEnd(); i = _context.g1.vertexNext(i))
    {
        int val = _context.core_1[i];

        if (val == EmbeddingEnumerator::IGNORE)
            ee.ignoreSubgraphVertex(i);
        else if (!ee.fix(mapping[i], val))
            return false;
    }

    if (!ee.process())
        return true;
    return false;
}
