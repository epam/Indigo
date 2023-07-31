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

#include "reaction/base_reaction_substructure_matcher.h"
#include "molecule/molecule_3d_constraints.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_neighbourhood_counters.h"
#include "molecule/molecule_substructure_matcher.h"
#include "reaction/base_reaction_substructure_matcher.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_neighborhood_counters.h"

using namespace indigo;

IMPL_ERROR(BaseReactionSubstructureMatcher, "reaction substructure matcher");

CP_DEF(BaseReactionSubstructureMatcher);

BaseReactionSubstructureMatcher::BaseReactionSubstructureMatcher(Reaction& target)
    : _target(target), CP_INIT, TL_CP_GET(_matchers), TL_CP_GET(_aam_to_second_side_1), TL_CP_GET(_aam_to_second_side_2), TL_CP_GET(_molecule_core_1),
      TL_CP_GET(_molecule_core_2), TL_CP_GET(_aam_core_first_side)
{
    use_aromaticity_matcher = true;
    highlight = false;

    match_atoms = 0;
    match_bonds = 0;
    context = 0;
    remove_atom = 0;
    add_bond = 0;
    prepare = 0;
    _match_stereo = true;

    _query_nei_counters = 0;
    _target_nei_counters = 0;

    _query = 0;

    _matchers.clear();
    _matchers.add(new _Matcher(*this));
}

void BaseReactionSubstructureMatcher::setQuery(BaseReaction& query)
{
    _query = &query;
}

void BaseReactionSubstructureMatcher::setNeiCounters(const ReactionAtomNeighbourhoodCounters* query_counters,
                                                     const ReactionAtomNeighbourhoodCounters* target_counters)
{
    _query_nei_counters = query_counters;
    _target_nei_counters = target_counters;
}

bool BaseReactionSubstructureMatcher::find()
{
    if (_query == 0)
        throw Error("no query");

    if (prepare != 0 && !prepare(*_query, _target, context))
        return false;

    if (_query->reactantsCount() > _target.reactantsCount() || _query->productsCount() > _target.productsCount())
        return false;

    if (_query->reactantsCount() * _target.reactantsCount() < _query->productsCount() * _target.productsCount())
        _first_side = Reaction::REACTANT, _second_side = Reaction::PRODUCT;
    else
        _first_side = Reaction::PRODUCT, _second_side = Reaction::REACTANT;

    _initMap(*_query, _second_side, _aam_to_second_side_1);
    _initMap(_target, _second_side, _aam_to_second_side_2);

    _molecule_core_1.resize(_query->end());
    _molecule_core_1.fffill();
    _molecule_core_2.resize(_target.end());
    _molecule_core_2.fffill();
    _aam_core_first_side.clear();

    _matchers.top()->match_stereo = _match_stereo;

    while (1)
    {
        int command = _matchers.top()->nextPair();

        if (command == _CONTINUE)
            continue;

        if (command == _RETURN)
        {
            if (_checkAAM())
            {
                _highlight();
                return true;
            }
            command = _NO_WAY;
        }
        else if (command != _NO_WAY)
        {
            int mol1 = _matchers.top()->_current_molecule_1;
            int mol2 = _matchers.top()->_current_molecule_2;
            Array<int>& core1 = _matchers.top()->_current_core_1;
            Array<int>& core2 = _matchers.top()->_current_core_2;
            int mode = _matchers.top()->getMode();

            //_matchers.reserve(_matchers.size() + 1);
            _matchers.add(new _Matcher(*_matchers.top()));
            _matchers.top()->setMode(command);
            if (!_matchers.top()->addPair(mol1, mol2, core1, core2, mode == _FIRST_SIDE))
                _matchers.removeLast();
        }

        if (command == _NO_WAY)
        {
            if (_matchers.size() > 1)
            {
                _matchers.top()->restore();
                _matchers.removeLast();
            }
            else
                return false;
        }
    }
}

// Init data for reaction substructure search
void BaseReactionSubstructureMatcher::_initMap(BaseReaction& reaction, int side, std::map<int, int>& aam_map)
{
    int i, j;
    int* val;

    aam_map.clear();

    // collect aam-to-molecule index mapping for reaction second side
    for (i = reaction.sideBegin(side); i < reaction.sideEnd(); i = reaction.sideNext(side, i))
    {
        BaseMolecule& i_mol = reaction.getBaseMolecule(i);

        for (j = i_mol.vertexBegin(); j < i_mol.vertexEnd(); j = i_mol.vertexNext(j))
        {
            int aam_number = reaction.getAAM(i, j);

            if (aam_number != 0)
            {
                auto it = aam_map.find(aam_number);
                val = it != aam_map.end() ? &(it->second) : nullptr;

                if (!val)
                    aam_map.emplace(aam_number, i);
                else if (*val < 0)
                    (*val)--;
                else
                    (*val) = -1;
            }
        }
    }
}

// Check correct AAM relationship between query and target reaction
bool BaseReactionSubstructureMatcher::_checkAAM()
{
    int *aam, aam1, aam2;
    int i, j;

    for (i = 1; i < _matchers.size() - 1; i++)
    {
        if (_matchers[i]->getMode() == _FIRST_SIDE)
            continue;

        BaseMolecule& mol1 = _query->getBaseMolecule(_matchers[i]->_current_molecule_1);

        for (j = mol1.vertexBegin(); j < mol1.vertexEnd(); j = mol1.vertexNext(j))
        {
            int k = _matchers[i]->_current_core_1[j];

            if (k < 0)
                continue;

            aam1 = _query->getAAM(_matchers[i]->_current_molecule_1, j);
            aam2 = _target.getAAM(_matchers[i]->_current_molecule_2, k);

            if (aam1 > 0 && aam2 > 0)
            {
                auto it = _aam_core_first_side.find(aam1);
                aam = it != _aam_core_first_side.end() ? &(it->second) : nullptr;

                if (aam && *aam != aam2)
                    return false;
            }
        }
    }

    return true;
}

int BaseReactionSubstructureMatcher::getTargetMoleculeIndex(int query_mol_idx)
{
    // can be optimized, but as the number of molecules
    // seldom exceeds 5, the linear search is acceptable
    for (int i = 0; i < _matchers.size() - 1; i++)
        if (_matchers[i]->_current_molecule_1 == query_mol_idx)
            return _matchers[i]->_current_molecule_2;

    throw Error("getTargetMoleculeIndex(): can not find mapping for query molecule %d", query_mol_idx);
}

const int* BaseReactionSubstructureMatcher::getQueryMoleculeMapping(int query_mol_idx)
{
    for (int i = 0; i < _matchers.size() - 1; i++)
        if (_matchers[i]->_current_molecule_1 == query_mol_idx)
            return _matchers[i]->_current_core_1.ptr();

    throw Error("getQueryMoleculeMapping(): can not find mapping for query molecule %d", query_mol_idx);
}

void BaseReactionSubstructureMatcher::_highlight()
{
    if (!highlight)
        return;

    int i;

    for (i = 0; i < _matchers.size() - 1; i++)
        _target.getBaseMolecule(_matchers[i]->_current_molecule_2)
            .highlightSubmolecule(_query->getBaseMolecule(_matchers[i]->_current_molecule_1), _matchers[i]->_current_core_1.ptr(), true);
}

CP_DEF(BaseReactionSubstructureMatcher::_Matcher);

BaseReactionSubstructureMatcher::_Matcher::_Matcher(BaseReactionSubstructureMatcher& context)
    : CP_INIT, TL_CP_GET(_current_core_1), TL_CP_GET(_current_core_2), _context(context), TL_CP_GET(_mapped_aams)
{
    _mode = _FIRST_SIDE;
    _selected_molecule_1 = -1;
    _selected_molecule_2 = -1;
    _current_molecule_1 = -1;
    _current_molecule_2 = -1;
    _mapped_aams.clear();
    match_stereo = true;
    _current_core_1.clear();
    _current_core_2.clear();
}

BaseReactionSubstructureMatcher::_Matcher::_Matcher(const BaseReactionSubstructureMatcher::_Matcher& other)
    : CP_INIT, TL_CP_GET(_current_core_1), TL_CP_GET(_current_core_2), _context(other._context), TL_CP_GET(_mapped_aams)
{
    _current_molecule_1 = -1;
    _current_molecule_2 = -1;
    _mapped_aams.clear();
    match_stereo = other.match_stereo;
    _current_core_1.clear();
    _current_core_2.clear();
    _selected_molecule_1 = -1;
    _selected_molecule_2 = -1;
}

int BaseReactionSubstructureMatcher::_Matcher::_nextPair()
{
    int side;

    if (_mode == _FIRST_SIDE)
        side = _context._first_side;
    else // _SECOND_SIDE_REST
        side = _context._second_side;

    if (_enumerator.get() == 0 || !_enumerator->processNext())
    {
        do
        {
            while (1)
            {
                if (_current_molecule_1 == -1)
                {
                    for (_current_molecule_1 = _context._query->sideBegin(side); _current_molecule_1 < _context._query->sideEnd();
                         _current_molecule_1 = _context._query->sideNext(side, _current_molecule_1))
                        if (_context._molecule_core_1[_current_molecule_1] < 0)
                            break;
                    if (_current_molecule_1 == _context._query->sideEnd())
                    {
                        if (_mode == _FIRST_SIDE)
                        {
                            _mode = _SECOND_SIDE_REST;
                            _current_molecule_1 = -1;
                            return _nextPair();
                        }

                        return _RETURN;
                    }
                }

                if (_current_molecule_2 == -1)
                    _current_molecule_2 = _context._target.sideBegin(side);
                else
                    _current_molecule_2 = _context._target.sideNext(side, _current_molecule_2);

                for (; _current_molecule_2 < _context._target.sideEnd(); _current_molecule_2 = _context._target.sideNext(side, _current_molecule_2))
                    if (_context._molecule_core_2[_current_molecule_2] < 0)
                        break;

                if (_current_molecule_2 == _context._target.sideEnd())
                    return _NO_WAY;

                _enumerator.free();

                BaseMolecule& mol_1 = _context._query->getBaseMolecule(_current_molecule_1);
                Molecule& mol_2 = _context._target.getMolecule(_current_molecule_2);

                if (!_initEnumerator(mol_1, mol_2))
                {
                    _enumerator.free();
                    continue;
                }
                break;
            }

            _enumerator->processStart();

        } while (!_enumerator->processNext());
    }

    return _mode == _FIRST_SIDE ? _SECOND_SIDE : _SECOND_SIDE_REST;
}

int BaseReactionSubstructureMatcher::_Matcher::nextPair()
{
    if (_mode != _SECOND_SIDE)
    {
        int next = _nextPair();

        if (next != _SECOND_SIDE)
            return next;

        // Switch to _SECOND_SIDE
        BaseMolecule& mol_1 = _context._query->getBaseMolecule(_current_molecule_1);

        int first_aam_1 = 0;
        int first_aam_2 = 0;

        int i;

        for (i = mol_1.vertexBegin(); i < mol_1.vertexEnd(); i = mol_1.vertexNext(i))
            if (_current_core_1[i] >= 0)
            {
                first_aam_1 = _context._query->getAAM(_current_molecule_1, i);
                first_aam_2 = _context._target.getAAM(_current_molecule_2, _current_core_1[i]);
                break;
            }

        if (first_aam_1 > 0 && first_aam_2 > 0)
        {
            // Check the other side if needed
            auto it_1 = _context._aam_to_second_side_1.find(first_aam_1);
            int* mol_1_idx_ss_ptr = it_1 != _context._aam_to_second_side_1.end() ? &(it_1->second) : nullptr;
            auto it_2 = _context._aam_to_second_side_2.find(first_aam_2);
            int* mol_2_idx_ss_ptr = it_2 != _context._aam_to_second_side_2.end() ? &(it_2->second) : nullptr;

            if (mol_1_idx_ss_ptr == 0 && mol_2_idx_ss_ptr == 0)
                // There is no pair for both atom
                return _FIRST_SIDE;

            if (mol_1_idx_ss_ptr == 0 || mol_2_idx_ss_ptr == 0)
                // One atom has a pair atom while other hasn't one
                return _CONTINUE;

            int mol_1_idx_ss = *mol_1_idx_ss_ptr;
            int mol_2_idx_ss = *mol_2_idx_ss_ptr;
            if ((mol_1_idx_ss < 0 && mol_1_idx_ss < mol_2_idx_ss))
                return _CONTINUE; // subreactions equal AAM-numbers more than superreaction

            if (mol_2_idx_ss < 0)
                return _FIRST_SIDE; // check this molecules in the completion phase

            if (_context._molecule_core_1[mol_1_idx_ss] >= 0)
            {
                if (_context._molecule_core_1[mol_1_idx_ss] != mol_2_idx_ss)
                    return _CONTINUE;

                int first_idx_1_ss = _context._query->findAtomByAAM(mol_1_idx_ss, first_aam_1);
                int first_idx_2_ss = _context._target.findAtomByAAM(mol_2_idx_ss, first_aam_2);
                int i;

                for (i = 0; i < _context._matchers.size(); i++)
                    if (_context._matchers[i]->_current_molecule_1 == mol_1_idx_ss)
                    {
                        if (_context._matchers[i]->_current_core_1[first_idx_1_ss] != first_idx_2_ss)
                            return _CONTINUE;
                        return _FIRST_SIDE;
                    }
            }

            return _SECOND_SIDE;
        }

        return _FIRST_SIDE;
    }

    // _SECOND_SIDE
    if (_enumerator.get() == 0)
    {
        BaseMolecule& src_mol_1 = _context._query->getBaseMolecule(_selected_molecule_1);
        Molecule& src_mol_2 = _context._target.getMolecule(_selected_molecule_2);

        int src_aam_1 = 0;
        int src_aam_2 = 0;

        Array<int>& prev_core_1 = _context._matchers[_context._matchers.size() - 2]->_current_core_1;
        for (int i = src_mol_1.vertexBegin(); i < src_mol_1.vertexEnd(); i = src_mol_1.vertexNext(i))
            if (prev_core_1[i] >= 0)
            {
                src_aam_1 = _context._query->getAAM(_selected_molecule_1, i);
                src_aam_2 = _context._target.getAAM(_selected_molecule_2, prev_core_1[i]);
                break;
            }

        BaseMolecule& mol_1 = _context._query->getBaseMolecule(_current_molecule_1);
        Molecule& mol_2 = _context._target.getMolecule(_current_molecule_2);

        int first_idx_1 = _context._query->findAtomByAAM(_current_molecule_1, src_aam_1);
        int first_idx_2 = _context._target.findAtomByAAM(_current_molecule_2, src_aam_2);

        // init embedding enumerator context
        _initEnumerator(mol_1, mol_2);

        if (!_enumerator->fix(first_idx_1, first_idx_2))
            return _NO_WAY;

        _enumerator->processStart();
    }

    if (!_enumerator->processNext())
        return _NO_WAY;

    return _FIRST_SIDE;
}

bool BaseReactionSubstructureMatcher::_Matcher::_initEnumerator(BaseMolecule& mol_1, Molecule& mol_2)
{
    // init embedding enumerator context
    _enumerator.create(mol_2);

    _enumerator->cb_match_edge = _matchBonds;
    _enumerator->cb_match_vertex = _matchAtoms;
    _enumerator->cb_edge_add = _addBond;
    _enumerator->cb_vertex_remove = _removeAtom;
    _enumerator->cb_embedding = _embedding;

    if (mol_1.isQueryMolecule() && _context.use_aromaticity_matcher && AromaticityMatcher::isNecessary(mol_1.asQueryMolecule()))
        _am = std::make_unique<AromaticityMatcher>(mol_1.asQueryMolecule(), mol_2, _context.arom_options);
    else
        _am.reset(nullptr);

    _enumerator->userdata = this;
    _enumerator->setSubgraph(mol_1);

    if (_context.prepare_ee != 0)
    {
        if (!_context.prepare_ee(_enumerator.ref(), mol_1, mol_2, _context.context))
            return false;
    }

    return true;
}

bool BaseReactionSubstructureMatcher::_Matcher::addPair(int mol1_idx, int mol2_idx, const Array<int>& core1, const Array<int>& core2, bool from_first_side)
{
    _selected_molecule_1 = mol1_idx;
    _selected_molecule_2 = mol2_idx;

    _mapped_aams.clear();

    BaseMolecule& mol1 = _context._query->getBaseMolecule(mol1_idx);

    if (from_first_side)
    {
        int i;

        for (i = mol1.vertexBegin(); i < mol1.vertexEnd(); i = mol1.vertexNext(i))
            if (core1[i] >= 0)
            {
                int aam1 = _context._query->getAAM(mol1_idx, i);
                int aam2 = _context._target.getAAM(mol2_idx, core1[i]);
                int* aam;

                if (aam1 > 0 && aam2 > 0)
                {
                    auto it = _context._aam_core_first_side.find(aam1);
                    aam = it != _context._aam_core_first_side.end() ? &(it->second) : nullptr;

                    if (!aam)
                    {
                        _context._aam_core_first_side.emplace(aam1, aam2);
                        _mapped_aams.push(aam1);
                    }
                    else if (*aam != aam2)
                    {
                        while (_mapped_aams.size() > 0)
                            _context._aam_core_first_side.erase(_mapped_aams.pop());
                        return false;
                    }
                }
            }
    }

    if (_mode == _SECOND_SIDE)
    {
        int first_aam_1 = 0;
        int first_aam_2 = 0;

        int i;

        for (i = mol1.vertexBegin(); i < mol1.vertexEnd(); i = mol1.vertexNext(i))
            if (core1[i] >= 0)
            {
                first_aam_1 = _context._query->getAAM(mol1_idx, i);
                first_aam_2 = _context._target.getAAM(mol2_idx, core1[i]);
                break;
            }

        _current_molecule_1 = _context._aam_to_second_side_1.at(first_aam_1);
        _current_molecule_2 = _context._aam_to_second_side_2.at(first_aam_2);
    }

    _context._molecule_core_1[mol1_idx] = mol2_idx;
    _context._molecule_core_2[mol2_idx] = mol1_idx;

    return true;
}

void BaseReactionSubstructureMatcher::_Matcher::restore()
{
    _context._molecule_core_1[_selected_molecule_1] = -1;
    _context._molecule_core_2[_selected_molecule_2] = -1;

    while (_mapped_aams.size() > 0)
        _context._aam_core_first_side.erase(_mapped_aams.pop());
}

int BaseReactionSubstructureMatcher::_Matcher::_embedding(Graph& subgraph, Graph& supergraph, int* core_sub, int* core_super, void* userdata)
{
    BaseReactionSubstructureMatcher::_Matcher& self = *(BaseReactionSubstructureMatcher::_Matcher*)userdata;

    QueryMolecule& query = (QueryMolecule&)subgraph;
    Molecule& target = (Molecule&)supergraph;

    if (self.match_stereo)
    {
        if (!MoleculeStereocenters::checkSub(query, target, core_sub, false))
            return 1;

        if (!MoleculeCisTrans::checkSub(query, target, core_sub))
            return 1;
    }

    // Check possible aromatic configuration
    if (self._am.get() != 0)
    {
        if (!self._am->match(core_sub, core_super))
            return 1;
    }

    self._current_core_1.copy(core_sub, subgraph.vertexEnd());
    self._current_core_2.copy(core_super, supergraph.vertexEnd());

    return 0;
}

bool BaseReactionSubstructureMatcher::_Matcher::_matchAtoms(Graph& subgraph, Graph& supergraph, const int* core_sub, int sub_idx, int super_idx, void* userdata)
{
    BaseReactionSubstructureMatcher::_Matcher* self = (BaseReactionSubstructureMatcher::_Matcher*)userdata;

    if (self->_context.match_atoms != 0 && !self->_context.match_atoms(*self->_context._query, self->_context._target, self->_current_molecule_1, sub_idx,
                                                                       self->_current_molecule_2, super_idx, self->_context.context))
        return false;

    if (self->_mode == _SECOND_SIDE)
    {
        int *aam, aam1, aam2;

        aam1 = self->_context._query->getAAM(self->_current_molecule_1, sub_idx);
        if (aam1 != 0)
        {
            aam2 = self->_context._target.getAAM(self->_current_molecule_2, super_idx);
            if (aam2 != 0)
            {
                auto it = self->_context._aam_core_first_side.find(aam1);
                aam = it != self->_context._aam_core_first_side.end() ? &(it->second) : nullptr;

                if (aam && *aam != aam2)
                    return false;
            }
        }
    }

    if (self->_context._query_nei_counters != 0 && self->_context._target_nei_counters != 0)
    {
        const MoleculeAtomNeighbourhoodCounters& mol_count1 = self->_context._query_nei_counters->getCounters(self->_current_molecule_1);
        const MoleculeAtomNeighbourhoodCounters& mol_count2 = self->_context._target_nei_counters->getCounters(self->_current_molecule_2);

        if (!mol_count1.testSubstructure(mol_count2, sub_idx, super_idx, true))
            return false;
    }

    int sub_atom_inv = self->_context._query->getInversion(self->_current_molecule_1, sub_idx);
    int super_atom_inv = self->_context._target.getInversion(self->_current_molecule_2, super_idx);

    if (sub_atom_inv != STEREO_UNMARKED && sub_atom_inv != super_atom_inv)
        return false;

    return true;
}

bool BaseReactionSubstructureMatcher::_Matcher::_matchBonds(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    BaseReactionSubstructureMatcher::_Matcher* self = (BaseReactionSubstructureMatcher::_Matcher*)userdata;

    if (self->_context.match_bonds != 0 && !self->_context.match_bonds(*self->_context._query, self->_context._target, self->_current_molecule_1, sub_idx,
                                                                       self->_current_molecule_2, super_idx, self->_am.get(), self->_context.context))
        return false;

    return true;
}

void BaseReactionSubstructureMatcher::_Matcher::_removeAtom(Graph& subgraph, int sub_idx, void* userdata)
{
    BaseReactionSubstructureMatcher::_Matcher* self = (BaseReactionSubstructureMatcher::_Matcher*)userdata;

    if (self->_context.remove_atom != 0)
        self->_context.remove_atom((BaseMolecule&)subgraph, sub_idx, self->_am.get());
}

void BaseReactionSubstructureMatcher::_Matcher::_addBond(Graph& subgraph, Graph& supergraph, int sub_idx, int super_idx, void* userdata)
{
    BaseReactionSubstructureMatcher::_Matcher* self = (BaseReactionSubstructureMatcher::_Matcher*)userdata;

    if (self->_context.add_bond != 0)
        self->_context.add_bond((BaseMolecule&)subgraph, (Molecule&)supergraph, sub_idx, super_idx, self->_am.get());
}
