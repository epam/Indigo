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

#include "reaction/reaction_automapper.h"
#include "base_cpp/red_black.h"
#include "graph/automorphism_search.h"
#include "molecule/elements.h"
#include "molecule/molecule_arom.h"
#include "molecule/molecule_neighbourhood_counters.h"
#include "reaction/crf_saver.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include <memory>

using namespace indigo;

IMPL_ERROR(ReactionAutomapper, "Reaction automapper");

ReactionAutomapper::ReactionAutomapper(BaseReaction& reaction)
    : ignore_atom_charges(false), ignore_atom_valence(false), ignore_atom_isotopes(false), ignore_atom_radicals(false), cancellation(nullptr),
      _initReaction(reaction), _maxMapUsed(0), _maxVertUsed(0), _maxCompleteMap(0), _mode(AAM_REGEN_DISCARD)
{
}

void ReactionAutomapper::automap(int mode)
{
    _mode = mode;

    QS_DEF(ObjArray<Array<int>>, mol_mappings);
    QS_DEF(Array<int>, react_mapping);

    /*
     * Set cancellation
     */
    cancellation = getCancellationHandler();

    /*
     * Check input atom mapping (if any)
     */
    if (mode != AAM_REGEN_DISCARD)
        _checkAtomMapping(true, false, false);
    /*
     * Clone reaction
     */
    _createReactionCopy(react_mapping, mol_mappings);

    /*
     * Create AAM map
     */
    _createReactionMap();
    _setupReactionInvMap(react_mapping, mol_mappings);
    _considerDissociation();
    _considerDimerization();

    /*
     * Check output atom mapping
     */
    _checkAtomMapping(false, true, false);
}

void ReactionAutomapper::_createReactionCopy(Array<int>& mol_mapping, ObjArray<Array<int>>& mappings)
{
    _reactionCopy.reset(_initReaction.neu());

    mol_mapping.clear();
    mappings.clear();
    int mol_idx = _initReaction.reactantBegin();
    for (; mol_idx != _initReaction.reactantEnd(); mol_idx = _initReaction.reactantNext(mol_idx))
    {
        _createMoleculeCopy(mol_idx, true, mol_mapping, mappings);
    }
    mol_idx = _initReaction.productBegin();
    for (; mol_idx != _initReaction.productEnd(); mol_idx = _initReaction.productNext(mol_idx))
    {
        _createMoleculeCopy(mol_idx, false, mol_mapping, mappings);
    }
    _reactionCopy->aromatize(arom_options);
}

void ReactionAutomapper::_createMoleculeCopy(int mol_idx, bool reactant, Array<int>& mol_mapping, ObjArray<Array<int>>& mappings)
{
    QS_DEF(Array<int>, vertices_map);
    QS_DEF(Array<int>, vertices_to_clone);
    int cmol_idx, ncomp, edge_beg, edge_end, edge_idx;
    BaseReaction& reaction = *_reactionCopy;
    BaseMolecule& mol = _initReaction.getBaseMolecule(mol_idx);
    /*
     * Calculate components
     */
    ncomp = mol.countComponents();
    const Array<int>& decomposition = mol.getDecomposition();
    /*
     * Add each component as a separate molecule
     */
    for (int comp_idx = 0; comp_idx < ncomp; ++comp_idx)
    {
        vertices_to_clone.clear();
        for (int j = mol.vertexBegin(); j < mol.vertexEnd(); j = mol.vertexNext(j))
        {
            if (decomposition[j] == comp_idx)
                vertices_to_clone.push(j);
        }
        if (reactant)
            cmol_idx = reaction.addReactant();
        else
            cmol_idx = reaction.addProduct();

        while (cmol_idx >= mol_mapping.size())
            mol_mapping.push(-1);
        while (cmol_idx >= mappings.size())
            mappings.push();

        mol_mapping[cmol_idx] = mol_idx;
        BaseMolecule& cmol = reaction.getBaseMolecule(cmol_idx);
        /*
         * Create component clone
         */
        cmol.makeSubmolecule(mol, vertices_to_clone, &vertices_map, 0);
        Array<int>& vertices_inv_map = mappings[cmol_idx];
        vertices_inv_map.resize(cmol.vertexEnd());
        _makeInvertMap(vertices_map, vertices_inv_map);
        /*
         * Fulfil AAM information
         */
        Array<int>& aam_array = reaction.getAAMArray(cmol_idx);
        aam_array.resize(cmol.vertexEnd());
        aam_array.zerofill();
        for (int i = cmol.vertexBegin(); i != cmol.vertexEnd(); i = cmol.vertexNext(i))
        {
            if (vertices_inv_map[i] < 0)
                throw Error("internal error: invalid clone for disconnected component");
            aam_array[i] = _initReaction.getAAM(mol_idx, vertices_inv_map[i]);
        }
        /*
         * Fulfil inversion information
         */
        Array<int>& inv_array = reaction.getInversionArray(cmol_idx);
        inv_array.resize(cmol.vertexEnd());
        inv_array.zerofill();
        for (int i = cmol.vertexBegin(); i != cmol.vertexEnd(); i = cmol.vertexNext(i))
        {
            if (vertices_inv_map[i] < 0)
                throw Error("internal error: invalid clone for disconnected component");
            inv_array[i] = _initReaction.getInversion(mol_idx, vertices_inv_map[i]);
        }
        /*
         * Fulfil Reacting centers information
         */
        Array<int>& rc_array = reaction.getReactingCenterArray(cmol_idx);
        rc_array.resize(cmol.edgeEnd());
        rc_array.zerofill();
        for (int i = cmol.edgeBegin(); i != cmol.edgeEnd(); i = cmol.edgeNext(i))
        {
            edge_beg = vertices_inv_map[cmol.getEdge(i).beg];
            edge_end = vertices_inv_map[cmol.getEdge(i).end];
            if (edge_beg < 0 || edge_end < 0)
                throw Error("internal error: invalid clone for disconnected component");
            edge_idx = mol.findEdgeIndex(edge_beg, edge_end);
            if (edge_idx < 0)
                throw Error("internal error: invalid clone for disconnected component");
            rc_array[i] = _initReaction.getReactingCenter(mol_idx, edge_idx);
        }
    }
}

void ReactionAutomapper::_makeInvertMap(Array<int>& map, Array<int>& invmap)
{
    invmap.fffill();
    for (int i = 0; i < map.size(); i++)
    {
        if (map[i] != -1)
        {
            invmap[map[i]] = i;
        }
    }
}

void ReactionAutomapper::_initMappings(BaseReaction& reaction)
{

    int i, j;

    BaseReaction& copy_reaction = *_reactionCopy;

    if (_mode == AAM_REGEN_ALTER || _mode == AAM_REGEN_DISCARD)
    {
        int current_map = 0;
        for (i = reaction.reactantBegin(); i < reaction.reactantEnd(); i = reaction.reactantNext(i))
        {
            for (j = 0; j < reaction.getAAMArray(i).size(); j++)
            {
                ++current_map;
                reaction.getAAMArray(i).at(j) = current_map;
            }
        }
        _usedVertices.resize(current_map + 1);
        _usedVertices.zerofill();
    }

    if (_mode == AAM_REGEN_KEEP)
    {
        RedBlackSet<int> used_maps;
        int max_value = 0;
        for (i = reaction.reactantBegin(); i < reaction.reactantEnd(); i = reaction.reactantNext(i))
        {
            for (j = 0; j < reaction.getAAMArray(i).size(); j++)
            {
                used_maps.find_or_insert(reaction.getAAM(i, j));
                if (reaction.getAAM(i, j) > max_value)
                    max_value = copy_reaction.getAAM(i, j);
            }
        }
        int new_size = used_maps.size();
        int current_map = 0;
        for (i = reaction.reactantBegin(); i < reaction.reactantEnd(); i = reaction.reactantNext(i))
        {
            for (j = 0; j < reaction.getAAMArray(i).size(); j++)
            {
                if (reaction.getAAM(i, j) == 0)
                {
                    while (new_size == used_maps.size())
                    {
                        ++current_map;
                        used_maps.find_or_insert(current_map);
                    }
                    new_size = used_maps.size();
                    reaction.getAAMArray(i).at(j) = current_map;
                }
            }
        }
        if (current_map > max_value)
            max_value = current_map;
        _usedVertices.resize(max_value + 1);
        _usedVertices.zerofill();
    }

    for (i = reaction.productBegin(); i < reaction.productEnd(); i = reaction.productNext(i))
    {
        reaction.getAAMArray(i).zerofill();
    }
}

void ReactionAutomapper::_createReactionMap()
{
    QS_DEF(ObjArray<Array<int>>, reactant_permutations);
    QS_DEF(Array<int>, product_mapping_tmp);

    BaseReaction& reaction = *_reactionCopy;

    ReactionMapMatchingData react_map_match(reaction);
    react_map_match.createAtomMatchingData();

    _initMappings(reaction);

    std::unique_ptr<BaseReaction> reaction_clone(reaction.neu());
    /*
     * Create all possible permutations for reactants
     */
    _createPermutations(reaction, reactant_permutations);

    for (int product = reaction.productBegin(); product < reaction.productEnd(); product = reaction.productNext(product))
    {
        product_mapping_tmp.clear_resize(reaction.getAAMArray(product).size());

        _maxMapUsed = 0;
        _maxVertUsed = 0;
        _maxCompleteMap = 0;

        for (int pmt = 0; pmt < reactant_permutations.size(); pmt++)
        {
            reaction_clone->clone(reaction, 0, 0, 0);
            /*
             * Apply new permutation
             */
            int map_complete = _handleWithProduct(reactant_permutations[pmt], product_mapping_tmp, *reaction_clone, product, react_map_match);
            /*
             * Collect statistic and choose the best mapping
             */
            if (_chooseBestMapping(reaction, product_mapping_tmp, product, map_complete))
                break;
            /*
             * Check for cancellation
             */
            if (cancellation != nullptr && cancellation->isCancelled())
                break;
        }
        _usedVertices.zerofill();
        for (int k = reaction.productBegin(); k <= product; k = reaction.productNext(k))
        {
            for (int j = 0; j < reaction.getAAMArray(k).size(); j++)
            {
                int m = reaction.getAAM(k, j);
                if (m > 0)
                    _usedVertices[m] = 1;
            }
        }
        //      _cleanReactants(reaction);
    }
}

void ReactionAutomapper::_cleanReactants(BaseReaction& reaction)
{
    for (int react = reaction.reactantBegin(); react < reaction.reactantEnd(); react = reaction.reactantNext(react))
    {
        BaseMolecule& rmol = reaction.getBaseMolecule(react);
        for (int vert = rmol.vertexBegin(); vert < rmol.vertexEnd();)
        {
            if (_usedVertices[reaction.getAAM(react, vert)])
            {
                int next_vert = rmol.vertexNext(vert);
                rmol.removeAtom(vert);
                vert = next_vert;
                continue;
            }
            vert = rmol.vertexNext(vert);
        }
    }
}

int ReactionAutomapper::_handleWithProduct(const Array<int>& reactant_cons, Array<int>& product_mapping_tmp, BaseReaction& reaction, int product,
                                           ReactionMapMatchingData& react_map_match)
{

    QS_DEF(Array<int>, matching_map);
    QS_DEF(Array<int>, rsub_map_in);
    QS_DEF(Array<int>, rsub_map_out);
    QS_DEF(Array<int>, vertices_to_remove);
    int map_complete = 0;

    BaseReaction& _reaction = *_reactionCopy;

    BaseMolecule& product_cut = reaction.getBaseMolecule(product);
    /*
     *delete hydrogens
     */
    vertices_to_remove.clear();
    for (int k : product_cut.vertices())
        if (product_cut.getAtomNumber(k) == ELEM_H)
            vertices_to_remove.push(k);
    product_cut.removeAtoms(vertices_to_remove);

    product_mapping_tmp.zerofill();

    _usedVertices[0] = 0;
    int previuosly_used = -1;

    while (previuosly_used != _usedVertices[0])
    {
        previuosly_used = _usedVertices[0];

        for (int perm_idx = 0; perm_idx < reactant_cons.size(); perm_idx++)
        {
            int react = reactant_cons.at(perm_idx);

            auto& reactant_r = reaction.getBaseMolecule(react);
            int react_vsize = reactant_r.vertexEnd();
            rsub_map_in.resize(react_vsize);
            for (int k = 0; k < react_vsize; k++)
                rsub_map_in[k] = SubstructureMcs::UNMAPPED;

            bool map_exc = false;
            if (_mode != AAM_REGEN_DISCARD)
            {
                for (int m : product_cut.vertices())
                {
                    react_map_match.getAtomMap(product, react, m, &matching_map);

                    for (int k = 0; k < matching_map.size(); k++)
                    {
                        rsub_map_in[matching_map[k]] = m;
                        map_exc = true;
                        break;
                    }
                }
            }

            if (!map_exc)
                rsub_map_in.clear();
            /*
             * First search substructure
             */
            RSubstructureMcs react_sub_mcs(reaction, react, product, *this);
            bool find_sub = react_sub_mcs.searchSubstructureReact(_reaction.getBaseMolecule(react), &rsub_map_in, &rsub_map_out);

            if (!find_sub)
            {
                react_sub_mcs.searchMaxCommonSubReact(&rsub_map_in, &rsub_map_out);
            }

            bool cur_used = false;
            for (int j = 0; j < rsub_map_out.size(); j++)
            {
                int v = rsub_map_out.at(j);
                if (v >= 0)
                {
                    /*
                     * Check delta Y exchange problem possibility
                     */
                    if (!product_cut.hasVertex(v))
                        continue;

                    cur_used = true;
                    product_mapping_tmp[v] = reaction.getAAM(react, j);
                    if (_usedVertices[product_mapping_tmp[v]] == 0)
                    {
                        _usedVertices[product_mapping_tmp[v]] = 1;
                        ++_usedVertices[0];
                    }
                    product_cut.removeAtom(v);
                }
            }
            if (!cur_used)
                ++map_complete;

            if (product_cut.vertexCount() == 0)
            {
                map_complete += reactant_cons.size() - perm_idx - 1;
                break;
            }
            /*
             * Remove mapped atoms for reactant
             */
            vertices_to_remove.clear();
            for (int k : reactant_r.vertices())
            {
                if (_usedVertices[reaction.getAAM(react, k)] > 0)
                    vertices_to_remove.push(k);
            }
            reactant_r.removeAtoms(vertices_to_remove);
        }
    }
    return map_complete;
}

bool ReactionAutomapper::_chooseBestMapping(BaseReaction& reaction, Array<int>& product_mapping, int product, int map_complete)
{
    int map_used = 0, total_map_used;
    for (int map_idx = 0; map_idx < product_mapping.size(); ++map_idx)
        if (product_mapping[map_idx] > 0)
            ++map_used;

    bool map_u = map_used > _maxMapUsed;
    bool map_c = (map_used == _maxMapUsed) && (map_complete > _maxCompleteMap);
    bool map_v = (map_used == _maxMapUsed) && (map_complete == _maxCompleteMap) && (_usedVertices[0] > _maxVertUsed);
    if (map_u || map_c || map_v)
    {
        _maxMapUsed = map_used;
        _maxVertUsed = _usedVertices[0];
        _maxCompleteMap = map_complete;
        reaction.getAAMArray(product).copy(product_mapping);
    }
    /*
     * Check if map covers all a reaction molecules
     */
    total_map_used = 0;
    for (int i = 1; i < _usedVertices.size(); ++i)
    {
        if (_usedVertices[i])
            ++total_map_used;
    }
    if (total_map_used >= (_usedVertices.size() - 1))
    {
        reaction.getAAMArray(product).copy(product_mapping);
        return true;
    }

    return false;
}

bool ReactionAutomapper::_checkAtomMapping(bool change_rc, bool change_aam, bool change_rc_null)
{

    ReactionMapMatchingData map_match(_initReaction);
    map_match.createBondMatchingData();

    QS_DEF(ObjArray<Array<int>>, bond_centers);
    QS_DEF(Array<int>, mapping);
    QS_DEF(Array<int>, v_mapping);
    QS_DEF(Array<int>, null_map);
    QS_DEF(Array<int>, rmol_map);
    std::unique_ptr<BaseReaction> reaction_copy_ptr;
    QS_DEF(ObjArray<Array<int>>, react_invmap);

    null_map.clear();
    bool unchanged = true;

    bond_centers.clear();
    for (int i = 0; i < _initReaction.end(); ++i)
        bond_centers.push();
    for (int i = _initReaction.begin(); i < _initReaction.end(); i = _initReaction.next(i))
    {
        bond_centers[i].resize(_initReaction.getBaseMolecule(i).edgeEnd());
        bond_centers[i].zerofill();
    }

    reaction_copy_ptr.reset(_initReaction.neu());

    BaseReaction& reaction_copy = *reaction_copy_ptr;

    reaction_copy.clone(_initReaction, &rmol_map, 0, &react_invmap);
    reaction_copy.aromatize(arom_options);

    for (int mol_idx = _initReaction.begin(); mol_idx != _initReaction.end(); mol_idx = _initReaction.next(mol_idx))
    {
        BaseMolecule& rmol = _initReaction.getBaseMolecule(mol_idx);
        for (int vert = rmol.vertexBegin(); vert < rmol.vertexEnd(); vert = rmol.vertexNext(vert))
        {
            if (_initReaction.getAAM(mol_idx, vert) > 0 && map_match.beginAtomMap(mol_idx, vert) >= map_match.endAtomMap())
            {
                _initReaction.getAAMArray(mol_idx).at(vert) = 0;
            }
        }
    }

    /*
     * Initialize reaction substructure handler by null molecules since it will not be callable by itself
     */
    RSubstructureMcs rsm(reaction_copy, *this);

    for (int mol_idx = _initReaction.begin(); mol_idx != _initReaction.end(); mol_idx = _initReaction.next(mol_idx))
    {
        BaseMolecule& rmol = _initReaction.getBaseMolecule(mol_idx);
        for (int bond_idx = rmol.edgeBegin(); bond_idx < rmol.edgeEnd(); bond_idx = rmol.edgeNext(bond_idx))
        {
            for (int opp_idx = map_match.beginMap(mol_idx); opp_idx < map_match.endMap(); opp_idx = map_match.nextMap(mol_idx, opp_idx))
            {
                BaseMolecule& pmol = _initReaction.getBaseMolecule(opp_idx);
                map_match.getBondMap(mol_idx, opp_idx, bond_idx, &mapping);

                if (mapping.size() == 0)
                {
                    int ve_beg = rmol.getEdge(bond_idx).beg;
                    int ve_end = rmol.getEdge(bond_idx).end;

                    map_match.getAtomMap(mol_idx, opp_idx, ve_beg, &v_mapping);

                    if (v_mapping.size() > 0)
                    {
                        bool change_broken = true;
                        for (int v_map = 0; v_map < v_mapping.size(); ++v_map)
                        {
                            const Vertex& end_vert = pmol.getVertex(v_mapping[v_map]);
                            for (int nei_vert = end_vert.neiBegin(); nei_vert < end_vert.neiEnd(); nei_vert = end_vert.neiNext(nei_vert))
                            {
                                int end_nei_vert = end_vert.neiVertex(nei_vert);
                                if (_initReaction.getAAM(opp_idx, end_nei_vert) == 0 &&
                                    RSubstructureMcs::atomConditionReact(rmol, pmol, 0, ve_end, end_nei_vert, &rsm))
                                {
                                    change_broken = false;
                                    break;
                                }
                            }
                        }
                        if (change_broken)
                            bond_centers[mol_idx][bond_idx] |= RC_MADE_OR_BROKEN;
                    }

                    map_match.getAtomMap(mol_idx, opp_idx, ve_end, &v_mapping);

                    if (v_mapping.size() > 0)
                    {
                        bool change_broken = true;
                        for (int v_map = 0; v_map < v_mapping.size(); ++v_map)
                        {
                            const Vertex& beg_vert = pmol.getVertex(v_mapping[v_map]);
                            for (int nei_vert = beg_vert.neiBegin(); nei_vert < beg_vert.neiEnd(); nei_vert = beg_vert.neiNext(nei_vert))
                            {
                                int beg_nei_vert = beg_vert.neiVertex(nei_vert);
                                if (_initReaction.getAAM(opp_idx, beg_nei_vert) == 0 &&
                                    RSubstructureMcs::atomConditionReact(rmol, pmol, 0, ve_beg, beg_nei_vert, &rsm))
                                {
                                    change_broken = false;
                                    break;
                                }
                            }
                        }
                        if (change_broken)
                            bond_centers[mol_idx][bond_idx] |= RC_MADE_OR_BROKEN;
                    }
                }
                else
                {
                    for (int i = 0; i < mapping.size(); ++i)
                    {

                        const Edge& r_edge = rmol.getEdge(bond_idx);
                        const Edge& p_edge = pmol.getEdge(mapping[i]);

                        int copy_mol_idx = rmol_map[mol_idx];
                        int copy_opp_idx = rmol_map[opp_idx];

                        BaseMolecule& cr_mol = reaction_copy.getBaseMolecule(copy_mol_idx);
                        BaseMolecule& cp_mol = reaction_copy.getBaseMolecule(copy_opp_idx);

                        int mol_edge_idx = cr_mol.findEdgeIndex(react_invmap.at(mol_idx)[r_edge.beg], react_invmap.at(mol_idx)[r_edge.end]);
                        int opp_edge_idx = cp_mol.findEdgeIndex(react_invmap.at(opp_idx)[p_edge.beg], react_invmap.at(opp_idx)[p_edge.end]);

                        bool react_arom = cr_mol.getBondOrder(mol_edge_idx) == BOND_AROMATIC;
                        bool prod_arom = cp_mol.getBondOrder(opp_edge_idx) == BOND_AROMATIC;

                        if (change_aam && (react_arom || prod_arom))
                        {
                            bond_centers[mol_idx][bond_idx] |= _initReaction.getReactingCenter(mol_idx, bond_idx);
                            continue;
                        }

                        bool bond_cond_simple = RSubstructureMcs::bondConditionReactSimple(pmol, rmol, mapping[i], bond_idx, &rsm);

                        if (bond_cond_simple || (react_arom && prod_arom))
                            bond_centers[mol_idx][bond_idx] |= RC_UNCHANGED;
                        else
                            bond_centers[mol_idx][bond_idx] |= RC_ORDER_CHANGED;

                        if (bond_cond_simple && (react_arom != prod_arom))
                            bond_centers[mol_idx][bond_idx] |= RC_ORDER_CHANGED;
                    }
                }
            }
        }
    }

    for (int mol_idx = _initReaction.begin(); mol_idx != _initReaction.end(); mol_idx = _initReaction.next(mol_idx))
    {
        BaseMolecule& rmol = _initReaction.getBaseMolecule(mol_idx);
        for (int bond_idx = rmol.edgeBegin(); bond_idx < rmol.edgeEnd(); bond_idx = rmol.edgeNext(bond_idx))
        {
            int rc_bond = _initReaction.getReactingCenter(mol_idx, bond_idx);
            if (rc_bond == RC_NOT_CENTER)
                rc_bond = RC_UNCHANGED;
            bool aam_bond =
                ((_initReaction.getAAM(mol_idx, rmol.getEdge(bond_idx).beg) > 0) && (_initReaction.getAAM(mol_idx, rmol.getEdge(bond_idx).end) > 0)) ||
                change_rc_null;
            if (aam_bond && ((bond_centers[mol_idx][bond_idx] & ~rc_bond) || rc_bond == 0))
            {
                if (!change_rc && !change_aam)
                {
                    return false;
                }
                else if (change_rc)
                {
                    _initReaction.getReactingCenterArray(mol_idx).at(bond_idx) = bond_centers[mol_idx][bond_idx];
                    unchanged = false;
                }
                else if (change_aam && rc_bond && _initReaction.getSideType(mol_idx) == Reaction::REACTANT)
                {
                    // only rc != 0 0 can match on every type of the bond
                    // only reactants rules (lazy users)
                    null_map.push(_initReaction.getAAM(mol_idx, rmol.getEdge(bond_idx).beg));
                    null_map.push(_initReaction.getAAM(mol_idx, rmol.getEdge(bond_idx).end));
                    unchanged = false;
                }
            }
        }
    }
    /*
     * erase all wrong map
     */
    if (change_aam)
    {
        for (int i = _initReaction.begin(); i < _initReaction.end(); i = _initReaction.next(i))
        {
            BaseMolecule& rmol = _initReaction.getBaseMolecule(i);
            for (int atom = rmol.vertexBegin(); atom < rmol.vertexEnd(); atom = rmol.vertexNext(atom))
            {
                for (int j = 0; j < null_map.size(); ++j)
                {
                    if (null_map[j] == _initReaction.getAAM(i, atom))
                        _initReaction.getAAMArray(i).at(atom) = 0;
                }
            }
        }
        null_map.clear();
        /*
         * Check for single aam for atoms was removed since mcs can be one atom
         */
        //      for(int i = _reaction.begin(); i < _reaction.end(); i = _reaction.next(i)) {
        //         BaseMolecule& rmol = _reaction.getBaseMolecule(i);
        //         for(int atom = rmol.vertexBegin(); atom < rmol.vertexEnd(); atom = rmol.vertexNext(atom)) {
        //            const Vertex& vertex = rmol.getVertex(atom);
        //            if(_reaction.getAAM(i ,atom) > 0 && vertex.degree() > 0) {
        //               bool has_aam = false;
        //               for(int nei = vertex.neiBegin(); nei < vertex.neiEnd(); nei = vertex.neiNext(nei)) {
        //                  int nei_atom = vertex.neiVertex(nei);
        //                  if(_reaction.getAAM(i, nei_atom) > 0) {
        //                     has_aam = true;
        //                     break;
        //                  }
        //               }
        //               if(!has_aam)
        //                  null_map.push(_reaction.getAAM(i, atom));
        //            }
        //         }
        //      }
        //      for(int i = _reaction.begin(); i < _reaction.end(); i = _reaction.next(i)) {
        //         BaseMolecule& rmol = _reaction.getBaseMolecule(i);
        //         for(int atom = rmol.vertexBegin(); atom < rmol.vertexEnd(); atom = rmol.vertexNext(atom)) {
        //            for(int j = 0; j < null_map.size(); ++j) {
        //               if(null_map[j] == _reaction.getAAM(i, atom))
        //                  _reaction.getAAMArray(i).at(atom) = 0;
        //            }
        //         }
        //      }
    }

    return unchanged;
}

void ReactionAutomapper::_setupReactionMap(Array<int>& react_mapping, ObjArray<Array<int>>& mol_mappings)
{
    int mol_idx, j, v;
    if (_mode == AAM_REGEN_KEEP)
        _usedVertices.zerofill();

    BaseReaction& reaction_copy = *_reactionCopy;

    for (mol_idx = _initReaction.productBegin(); mol_idx < _initReaction.productEnd(); mol_idx = _initReaction.productNext(mol_idx))
    {
        int mol_idx_u = react_mapping[mol_idx];
        Array<int>& react_aam = _initReaction.getAAMArray(mol_idx);
        for (j = 0; j < react_aam.size(); j++)
        {
            if (mol_mappings[mol_idx][j] == -1)
                continue;
            v = reaction_copy.getAAM(mol_idx_u, mol_mappings[mol_idx][j]);
            if (_mode == AAM_REGEN_DISCARD)
                react_aam[j] = v;

            if (_mode == AAM_REGEN_ALTER)
                react_aam[j] = v;

            if (_mode == AAM_REGEN_KEEP && _initReaction.getAAM(mol_idx, j) == 0)
            {
                react_aam[j] = v;
                _usedVertices[v] = 1;
            }
        }
    }
    for (mol_idx = _initReaction.reactantBegin(); mol_idx < _initReaction.reactantEnd(); mol_idx = _initReaction.reactantNext(mol_idx))
    {
        int mol_idx_u = react_mapping[mol_idx];
        Array<int>& react_aam = _initReaction.getAAMArray(mol_idx);
        for (j = 0; j < react_aam.size(); j++)
        {
            if (mol_mappings[mol_idx][j] == -1)
                continue;
            v = reaction_copy.getAAM(mol_idx_u, mol_mappings[mol_idx][j]);
            if (_mode == AAM_REGEN_DISCARD)
                react_aam[j] = v * _usedVertices[v];
            if (_mode == AAM_REGEN_ALTER)
                react_aam[j] = v * _usedVertices[v];
            if (_mode == AAM_REGEN_KEEP && _initReaction.getAAM(mol_idx, j) == 0)
                react_aam[j] = v * _usedVertices[v];
        }
    }
}
void ReactionAutomapper::_setupReactionInvMap(Array<int>& react_mapping, ObjArray<Array<int>>& mol_mappings)
{
    int mol_idx, mol_idx_map, j, v, map_j;
    if (_mode == AAM_REGEN_KEEP)
        _usedVertices.zerofill();

    BaseReaction& reaction_copy = *_reactionCopy;

    mol_idx = reaction_copy.productBegin();
    for (; mol_idx < reaction_copy.productEnd(); mol_idx = reaction_copy.productNext(mol_idx))
    {
        mol_idx_map = react_mapping[mol_idx];
        Array<int>& react_aam = _initReaction.getAAMArray(mol_idx_map);
        Array<int>& creact_aam = reaction_copy.getAAMArray(mol_idx);
        for (j = 0; j < creact_aam.size(); j++)
        {
            map_j = mol_mappings[mol_idx][j];
            if (map_j < 0)
                continue;
            v = creact_aam[j];
            if (_mode == AAM_REGEN_DISCARD)
                react_aam[map_j] = v;

            if (_mode == AAM_REGEN_ALTER)
                react_aam[map_j] = v;

            if (_mode == AAM_REGEN_KEEP && _initReaction.getAAM(mol_idx_map, map_j) == 0)
            {
                react_aam[map_j] = v;
                _usedVertices[v] = 1;
            }
        }
    }
    mol_idx = reaction_copy.reactantBegin();
    for (; mol_idx < reaction_copy.reactantEnd(); mol_idx = reaction_copy.reactantNext(mol_idx))
    {
        mol_idx_map = react_mapping[mol_idx];
        Array<int>& react_aam = _initReaction.getAAMArray(mol_idx_map);
        Array<int>& creact_aam = reaction_copy.getAAMArray(mol_idx);
        for (j = 0; j < creact_aam.size(); j++)
        {
            map_j = mol_mappings[mol_idx][j];
            if (map_j < 0)
                continue;
            v = creact_aam[j];

            if (_mode == AAM_REGEN_DISCARD)
                react_aam[map_j] = v * _usedVertices[v];

            if (_mode == AAM_REGEN_ALTER)
                react_aam[map_j] = v * _usedVertices[v];

            if (_mode == AAM_REGEN_KEEP && _initReaction.getAAM(mol_idx_map, map_j) == 0)
                react_aam[map_j] = v * _usedVertices[v];
        }
    }
}

void ReactionAutomapper::_considerDissociation()
{
    std::unique_ptr<BaseMolecule> null_map_cut;
    std::unique_ptr<BaseMolecule> full_map_cut;
    QS_DEF(Array<int>, map);
    int i, j, mcv, mcvsum;

    for (i = _initReaction.begin(); i < _initReaction.end(); i = _initReaction.next(i))
    {
        mcvsum = 0;
        mcv = 0;
        for (j = 0; j < _initReaction.getAAMArray(i).size(); j++)
        {
            if (_initReaction.getAAM(i, j) == 0)
                mcvsum++;
            else
                mcv++;
        }
        if (mcvsum < mcv || mcv <= _MIN_VERTEX_SUB)
            continue;
        BaseMolecule& ibase_mol = _initReaction.getBaseMolecule(i);
        full_map_cut.reset(ibase_mol.neu());
        full_map_cut->clone_KeepIndices(ibase_mol, 0);
        full_map_cut->aromatize(arom_options);

        for (j = 0; j < _initReaction.getAAMArray(i).size(); j++)
        {
            if (_initReaction.getAAM(i, j) == 0)
                full_map_cut->removeAtom(j);
        }
        if (full_map_cut->vertexCount() == 0)
            continue;
        while (mcvsum >= mcv)
        {
            null_map_cut.reset(ibase_mol.neu());
            null_map_cut->clone_KeepIndices(ibase_mol, 0);
            null_map_cut->aromatize(arom_options);
            for (j = 0; j < _initReaction.getAAMArray(i).size(); j++)
            {
                if (_initReaction.getAAM(i, j) > 0 || _initReaction.getBaseMolecule(i).getAtomNumber(j) == ELEM_H)
                    null_map_cut->removeAtom(j);
            }
            if (null_map_cut->vertexCount() == 0)
                break;

            RSubstructureMcs rsm(_initReaction, *full_map_cut, *null_map_cut, *this);
            rsm.userdata = &rsm;

            map.clear();
            if (!rsm.searchSubstructure(&map))
                break;
            for (j = 0; j < map.size(); j++)
            {
                if (map[j] >= 0 && map[j] < _initReaction.getAAMArray(i).size())
                {
                    _initReaction.getAAMArray(i)[map[j]] = _initReaction.getAAM(i, j);
                }
            }
            mcvsum = 0;
            for (j = 0; j < _initReaction.getAAMArray(i).size(); j++)
            {
                if (_initReaction.getAAM(i, j) == 0)
                    mcvsum++;
            }
        }
    }
}

void ReactionAutomapper::_considerDimerization()
{
    QS_DEF(Array<int>, mol_mapping);
    QS_DEF(ObjArray<Array<int>>, inv_mappings);
    QS_DEF(Array<int>, sub_map);
    QS_DEF(Array<int>, max_sub_map);
    bool way_exit = true, map_changed = false;
    int map_found, max_found, max_react_index = -1;
    std::unique_ptr<BaseReaction> reaction_copy_ptr(_initReaction.neu());
    BaseReaction& reaction_copy = *reaction_copy_ptr;
    reaction_copy.clone(_initReaction, &mol_mapping, 0, &inv_mappings);
    /*
     * Clear reactants
     */
    for (int react = reaction_copy.reactantBegin(); react < reaction_copy.reactantEnd(); react = reaction_copy.reactantNext(react))
    {
        _removeUnusedInfo(reaction_copy, react, false);
        _removeSmallComponents(reaction_copy.getBaseMolecule(react));
    }

    for (int prod = reaction_copy.productBegin(); prod < reaction_copy.productEnd(); prod = reaction_copy.productNext(prod))
    {
        BaseMolecule& pmol = reaction_copy.getBaseMolecule(prod);
        pmol.aromatize(arom_options);
        way_exit = true;
        while (way_exit)
        {
            /*
             * Clear presented AAM
             */
            _removeUnusedInfo(reaction_copy, prod, true);
            _removeSmallComponents(pmol);

            if (pmol.vertexCount() < _MIN_VERTEX_SUB)
                way_exit = false;

            max_found = _MIN_VERTEX_SUB;
            for (int react = reaction_copy.reactantBegin(); react < reaction_copy.reactantEnd() && way_exit; react = reaction_copy.reactantNext(react))
            {

                map_found = _validMapFound(reaction_copy, react, prod, sub_map);

                if (map_found > max_found)
                {
                    max_found = map_found;
                    max_sub_map.copy(sub_map);
                    max_react_index = react;
                }
            }
            if (max_found > _MIN_VERTEX_SUB)
            {
                for (int i = 0; i < max_sub_map.size(); ++i)
                {
                    if (max_sub_map[i] >= 0)
                    {
                        reaction_copy.getAAMArray(prod).at(max_sub_map[i]) = reaction_copy.getAAM(max_react_index, i);
                        map_changed = true;
                    }
                }
            }
            else
            {
                way_exit = false;
            }
        }
    }

    if (map_changed)
    {
        for (int rindex = _initReaction.productBegin(); rindex < _initReaction.productEnd(); rindex = _initReaction.productNext(rindex))
        {
            BaseMolecule& rmol = _initReaction.getBaseMolecule(rindex);
            int mrindex = mol_mapping[rindex];
            for (int vert = rmol.vertexBegin(); vert < rmol.vertexEnd(); vert = rmol.vertexNext(vert))
            {
                int copy_aam = reaction_copy.getAAM(mrindex, inv_mappings[rindex].at(vert));
                if (_initReaction.getAAM(rindex, vert) == 0 && copy_aam > 0)
                    _initReaction.getAAMArray(rindex).at(vert) = copy_aam;
            }
        }
    }
}

int ReactionAutomapper::_validMapFound(BaseReaction& reaction, int react, int prod, Array<int>& sub_map) const
{

    BaseMolecule& react_copy = reaction.getBaseMolecule(react);

    int result = 0;

    if (react_copy.vertexCount() < _MIN_VERTEX_SUB)
        return result;

    RSubstructureMcs rsub_mcs(reaction, react, prod, *this);
    rsub_mcs.cbMatchVertex = RSubstructureMcs::atomConditionReact;
    rsub_mcs.cbMatchEdge = RSubstructureMcs::bondConditionReact;
    rsub_mcs.userdata = &rsub_mcs;

    if (rsub_mcs.searchSubstructure(&sub_map))
        result = std::min(react_copy.vertexCount(), reaction.getBaseMolecule(prod).vertexCount());

    return result;
}

void ReactionAutomapper::_removeUnusedInfo(BaseReaction& reaction, int mol_idx, bool aam_presented) const
{
    QS_DEF(Array<int>, vertices_to_remove);
    QS_DEF(Array<int>, edges_to_remove);
    vertices_to_remove.clear();
    edges_to_remove.clear();

    BaseMolecule& mol = reaction.getBaseMolecule(mol_idx);

    int i;
    bool aam;
    for (i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
    {
        if (aam_presented)
            aam = (reaction.getAAM(mol_idx, i) > 0);
        else
            aam = (reaction.getAAM(mol_idx, i) == 0);

        if (aam || (mol.getAtomNumber(i) == ELEM_H))
            vertices_to_remove.push(i);
    }
    for (i = 0; i < vertices_to_remove.size(); ++i)
    {
        mol.removeAtom(vertices_to_remove[i]);
    }
    for (i = mol.edgeBegin(); i < mol.edgeEnd(); i = mol.edgeNext(i))
    {
        if (reaction.getReactingCenter(mol_idx, i) == RC_MADE_OR_BROKEN)
            edges_to_remove.push(i);
    }
    for (i = 0; i < edges_to_remove.size(); ++i)
    {
        mol.removeBond(edges_to_remove[i]);
    }
}

void ReactionAutomapper::_removeSmallComponents(BaseMolecule& mol) const
{

    int ncomp = mol.countComponents();
    const Array<int>& decomposition = mol.getDecomposition();

    QS_DEF(Array<int>, vertices_to_remove);
    vertices_to_remove.clear();

    for (int comp_idx = 0; comp_idx < ncomp; ++comp_idx)
    {
        if (mol.countComponentVertices(comp_idx) < _MIN_VERTEX_SUB)
        {
            for (int j = mol.vertexBegin(); j < mol.vertexEnd(); j = mol.vertexNext(j))
                if (decomposition[j] == comp_idx)
                    vertices_to_remove.push(j);
        }
    }
    for (int i = 0; i < vertices_to_remove.size(); ++i)
    {
        mol.removeAtom(vertices_to_remove[i]);
    }
}

void ReactionAutomapper::_createPermutations(BaseReaction& reaction, ObjArray<Array<int>>& permutations)
{
    QS_DEF(Array<int>, reactant_indexes);
    QS_DEF(Array<int>, reactant_small);
    QS_DEF(Array<int>, reactant_buf);
    reactant_indexes.clear();
    reactant_small.clear();
    /*
     * Permutate only big components
     */
    int v_count;
    for (int r_idx = reaction.reactantBegin(); r_idx < reaction.reactantEnd(); r_idx = reaction.reactantNext(r_idx))
    {
        BaseMolecule& mol = reaction.getBaseMolecule(r_idx);
        /*
         * Count only nonhydrogens
         */
        v_count = 0;
        for (int j = mol.vertexBegin(); j < mol.vertexEnd(); j = mol.vertexNext(j))
        {
            if (mol.getAtomNumber(j) != ELEM_H)
                v_count++;
        }

        if (v_count < MIN_PERMUTATION_SIZE)
            reactant_small.push(r_idx);
        else
            reactant_indexes.push(r_idx);
    }

    _permutation(reactant_indexes, permutations);
    /*
     * If there are no small components then exit
     */
    if (reactant_small.size() == 0)
        return;

    /*
     * Appent small components
     */
    int p_size = permutations.size();
    /*
     * Workaround to consider permutation limit
     */
    if (p_size * 2 < MAX_PERMUTATIONS_NUMBER)
    {
        /*
         * Add possible small components before and after permutations (increase p size)
         */
        for (int i = 0; i < p_size; ++i)
        {
            permutations.push().copy(permutations[i]);
        }
    }
    else
    {
        /*
         * Add possible small components and not increase p size
         */
        p_size /= 2;
    }
    /*
     * Add small components before and after
     */
    for (int i = 0; i < permutations.size(); ++i)
    {
        Array<int>& perm = permutations[i];
        if (i < p_size)
        {
            /*
             * After
             */
            perm.concat(reactant_small);
        }
        else
        {
            /*
             * Before
             */
            reactant_buf.copy(reactant_small);
            reactant_buf.concat(perm);
            perm.copy(reactant_buf);
        }
    }
}

// all transpositions for numbers from 0 to n-1
void ReactionAutomapper::_permutation(Array<int>& s_array, ObjArray<Array<int>>& p_array)
{

    p_array.clear();
    QS_DEF(Array<int>, per);
    QS_DEF(Array<int>, obr);
    int n = s_array.size();
    int i, j, k, tmp, min = -1, raz;
    bool flag;
    per.resize(n);
    obr.resize(n);
    for (i = 0; i < n; i++)
    {
        per[i] = i + 1;
    }
    while (1)
    {
        /*
         * Break to escape permutations out of memory error
         */
        if (p_array.size() > MAX_PERMUTATIONS_NUMBER)
            break;
        Array<int>& new_array = p_array.push();
        new_array.resize(n);
        for (k = 0; k < n; k++)
        {
            new_array.at(k) = s_array.at(per[k] - 1);
        }
        flag = false;
        for (i = n - 2; i >= 0; i--)
        {
            if (per[i] < per[i + 1])
            {
                flag = true;
                break;
            }
        }
        if (flag == false)
        {
            break;
        }
        raz = per[i + 1];
        for (j = i + 1; j < n; j++)
        {
            if (((per[j] - per[i]) < raz) && (per[i] < per[j]))
            {
                min = j;
            }
        }
        tmp = per[i];
        per[i] = per[min];
        per[min] = tmp;
        for (j = i + 1; j < n; j++)
        {
            obr[j] = per[j];
        }
        j = i + 1;
        for (k = n - 1; k >= i + 1; k--)
        {
            per[j] = obr[k];
            j++;
        }
    }
}

ReactionMapMatchingData::ReactionMapMatchingData(BaseReaction& r) : _reaction(r)
{
}

void ReactionMapMatchingData::createAtomMatchingData()
{

    _vertexMatchingArray.clear();
    for (int i = _reaction.begin(); i < _reaction.end(); i = _reaction.next(i))
    {
        for (int j = 0; j < _reaction.getBaseMolecule(i).vertexEnd(); j++)
        {
            _vertexMatchingArray.push();
        }
    }

    for (int react = _reaction.reactantBegin(); react < _reaction.reactantEnd(); react = _reaction.reactantNext(react))
    {
        BaseMolecule& rmol = _reaction.getBaseMolecule(react);
        for (int rvert = rmol.vertexBegin(); rvert < rmol.vertexEnd(); rvert = rmol.vertexNext(rvert))
        {
            if (_reaction.getAAM(react, rvert) > 0)
            {
                for (int prod = _reaction.productBegin(); prod < _reaction.productEnd(); prod = _reaction.productNext(prod))
                {
                    BaseMolecule& pmol = _reaction.getBaseMolecule(prod);
                    for (int pvert = pmol.vertexBegin(); pvert < pmol.vertexEnd(); pvert = pmol.vertexNext(pvert))
                    {
                        if (_reaction.getAAM(react, rvert) == _reaction.getAAM(prod, pvert))
                        {
                            int v_id1 = _getVertexId(react, rvert);
                            int v_id2 = _getVertexId(prod, pvert);
                            _vertexMatchingArray[v_id1].push(v_id2);
                            _vertexMatchingArray[v_id2].push(v_id1);
                        }
                    }
                }
            }
        }
    }
}

void ReactionMapMatchingData::createBondMatchingData()
{

    QS_DEF(Array<int>, matchingMap1);
    QS_DEF(Array<int>, matchingMap2);
    int v1, v2;

    createAtomMatchingData();

    _edgeMatchingArray.clear();
    for (int i = _reaction.begin(); i < _reaction.end(); i = _reaction.next(i))
    {
        for (int j = 0; j < _reaction.getBaseMolecule(i).edgeEnd(); j++)
        {
            _edgeMatchingArray.push();
        }
    }

    for (int react = _reaction.reactantBegin(); react < _reaction.reactantEnd(); react = _reaction.reactantNext(react))
    {
        BaseMolecule& rmol = _reaction.getBaseMolecule(react);
        for (int redge = rmol.edgeBegin(); redge < rmol.edgeEnd(); redge = rmol.edgeNext(redge))
        {
            for (int prod = _reaction.productBegin(); prod < _reaction.productEnd(); prod = _reaction.productNext(prod))
            {
                v1 = rmol.getEdge(redge).beg;
                v2 = rmol.getEdge(redge).end;
                getAtomMap(react, prod, v1, &matchingMap1);
                getAtomMap(react, prod, v2, &matchingMap2);
                for (int m = 0; m < matchingMap1.size(); m++)
                {
                    for (int n = 0; n < matchingMap2.size(); n++)
                    {
                        int pedge = _reaction.getBaseMolecule(prod).findEdgeIndex(matchingMap1[m], matchingMap2[n]);
                        if (pedge >= 0)
                        {
                            int e_id1 = _getEdgeId(react, redge);
                            int e_id2 = _getEdgeId(prod, pedge);
                            _edgeMatchingArray[e_id1].push(e_id2);
                            _edgeMatchingArray[e_id2].push(e_id1);
                        }
                    }
                }
            }
        }
    }
}

int ReactionMapMatchingData::endMap() const
{
    return _reaction.sideEnd();
}

int ReactionMapMatchingData::nextMap(int mol_idx, int opposite_idx) const
{
    int side_type = _reaction.getSideType(mol_idx) == Reaction::REACTANT ? Reaction::PRODUCT : Reaction::REACTANT;
    return _reaction.sideNext(side_type, opposite_idx);
}

int ReactionMapMatchingData::endAtomMap() const
{
    return _reaction.sideEnd();
}

int ReactionMapMatchingData::nextAtomMap(int mol_idx, int opposite_idx, int atom_idx) const
{
    int side_type = _reaction.getSideType(mol_idx) == Reaction::REACTANT ? Reaction::PRODUCT : Reaction::REACTANT;
    int rm_idx = _reaction.sideNext(side_type, opposite_idx);
    for (; rm_idx < _reaction.sideEnd(); rm_idx = _reaction.sideNext(side_type, rm_idx))
    {
        if (getAtomMap(mol_idx, rm_idx, atom_idx, 0))
            break;
    }
    return rm_idx;
}

bool ReactionMapMatchingData::getAtomMap(int mol_idx, int opposite_idx, int atom_idx, Array<int>* mapping) const
{
    bool result = false;
    int vertex_id = _getVertexId(mol_idx, atom_idx);
    int first_r_vertex = _getVertexId(opposite_idx, 0);
    int last_r_vertex = _getVertexId(opposite_idx + 1, 0);
    if (mapping)
        mapping->clear();
    for (int i = 0; i < _vertexMatchingArray[vertex_id].size(); i++)
    {
        int m_vertex = _vertexMatchingArray[vertex_id].at(i);
        if (m_vertex >= first_r_vertex && m_vertex < last_r_vertex)
        {
            if (mapping)
                mapping->push(m_vertex - first_r_vertex);
            else
                return true;
            result = true;
        }
    }
    return result;
}

int ReactionMapMatchingData::endBondMap() const
{
    return _reaction.sideEnd();
}

int ReactionMapMatchingData::nextBondMap(int mol_idx, int opposite_idx, int bond_idx) const
{
    int side_type = _reaction.getSideType(mol_idx) == Reaction::REACTANT ? Reaction::PRODUCT : Reaction::REACTANT;
    int rm_idx = _reaction.sideNext(side_type, opposite_idx);
    for (; rm_idx < _reaction.sideEnd(); rm_idx = _reaction.sideNext(side_type, rm_idx))
    {
        if (getBondMap(mol_idx, rm_idx, bond_idx, 0))
            break;
    }
    return rm_idx;
}

bool ReactionMapMatchingData::getBondMap(int mol_idx, int opposite_idx, int bond_idx, Array<int>* mapping) const
{
    bool result = false;
    int edge = _getEdgeId(mol_idx, bond_idx);
    int first_r_edge = _getEdgeId(opposite_idx, 0);
    int last_r_edge = _getEdgeId(opposite_idx + 1, 0);
    if (mapping)
        mapping->clear();
    for (int i = 0; i < _edgeMatchingArray[edge].size(); i++)
    {
        int m_edge = _edgeMatchingArray[edge].at(i);
        if (m_edge >= first_r_edge && m_edge < last_r_edge)
        {
            if (mapping)
                mapping->push(m_edge - first_r_edge);
            else
                return true;

            result = true;
        }
    }
    return result;
}

int ReactionMapMatchingData::_getVertexId(int mol_idx, int vert) const
{
    int result = 0;
    for (int i = _reaction.begin(); i < mol_idx; i = _reaction.next(i))
    {
        result += _reaction.getBaseMolecule(i).vertexEnd();
    }
    result += vert;
    return result;
}

int ReactionMapMatchingData::_getEdgeId(int mol_idx, int edge) const
{
    int result = 0;
    for (int i = _reaction.begin(); i < mol_idx; i = _reaction.next(i))
    {
        result += _reaction.getBaseMolecule(i).edgeEnd();
    }
    result += edge;
    return result;
}

RSubstructureMcs::RSubstructureMcs(BaseReaction& reaction, const ReactionAutomapper& context)
    : flags(CONDITION_ALL), _context(context), _reaction(reaction), _subReactNumber(-1), _superProductNumber(-1)
{
    setUpFlags(context);
}

RSubstructureMcs::RSubstructureMcs(BaseReaction& reaction, int sub_num, int super_num, const ReactionAutomapper& context)
    : flags(CONDITION_ALL), _context(context), _reaction(reaction), _subReactNumber(sub_num), _superProductNumber(super_num)
{
    setGraphs(reaction.getBaseMolecule(sub_num), reaction.getBaseMolecule(super_num));
    _createQueryTransposition();
    setUpFlags(context);
}

RSubstructureMcs::RSubstructureMcs(BaseReaction& reaction, BaseMolecule& sub, BaseMolecule& super, const ReactionAutomapper& context)
    : flags(CONDITION_ALL), _context(context), _reaction(reaction), _subReactNumber(-1), _superProductNumber(-1)
{
    setGraphs(sub, super);
    _createQueryTransposition();
    setUpFlags(context);
    cbMatchVertex = atomConditionReact;
    cbMatchEdge = bondConditionReactSimple;
}

void RSubstructureMcs::setUpFlags(const ReactionAutomapper& context)
{
    flags = CONDITION_NONE;

    if (!context.ignore_atom_charges)
        flags |= CONDITION_ATOM_CHARGES;
    if (!context.ignore_atom_isotopes)
        flags |= CONDITION_ATOM_ISOTOPES;
    if (!context.ignore_atom_radicals)
        flags |= CONDITION_ATOM_RADICAL;
    if (!context.ignore_atom_valence)
        flags |= CONDITION_ATOM_VALENCE;
    arom_options = context.arom_options;
}
bool RSubstructureMcs::searchSubstructure(Array<int>* map)
{
    bool result = false;
    /*
     * Disable old logic with timeout waiting for mcs. Can be a different result for some reaction though
     */
    // if (_context.cancellation != nullptr)
    // {
    //     try
    //     {
    //         result = SubstructureMcs::searchSubstructure(map);
    //     }
    //     catch (Exception&)
    //     {
    //         result = false;
    //     }
    // }
    // else
    // {
    // result = SubstructureMcs::searchSubstructure(map);
    // }
    // Will throw error on timeout
    result = SubstructureMcs::searchSubstructure(map);
    /*
     * Transpose map back
     */
    if (result)
        _detransposeOutputMap(map);
    return result;
}

bool RSubstructureMcs::searchSubstructureReact(BaseMolecule& init_rmol, const Array<int>* in_map, Array<int>* out_map)
{
    if (_sub == 0 || _super == 0)
        throw ReactionAutomapper::Error("internal AAM error: not initialized sub-mcs molecules");

    QS_DEF(ObjArray<Array<int>>, tmp_maps);
    QS_DEF(ObjArray<EmbeddingEnumerator>, emb_enums);
    QS_DEF(Array<int>, in_map_cut);
    QS_DEF(Array<int>, results);

    emb_enums.clear();
    tmp_maps.clear();
    int map_result = 0;
    results.resize(4);

    BaseMolecule& mol_react = _reaction.getBaseMolecule(_subReactNumber);

    int react_vsize = mol_react.vertexCount();

    if (react_vsize < 2)
    {
        mol_react.clone(init_rmol, 0, 0);
        react_vsize = mol_react.vertexCount();
        mol_react.aromatize(arom_options);
    }

    if (_super->vertexCount() < 2 || _sub->vertexCount() < 2)
        return false;

    for (int i = 0; i < 4; ++i)
    {
        EmbeddingEnumerator& emb_enum = emb_enums.push(*_super);
        emb_enum.setSubgraph(*_sub);
        emb_enum.cb_match_vertex = atomConditionReact;
        emb_enum.cb_embedding = _embedding;
        emb_enum.userdata = this;
        if (i & 1)
            emb_enum.cb_match_edge = bondConditionReact;
        else
            emb_enum.cb_match_edge = bondConditionReactStrict;
        tmp_maps.push().clear();
        results[i] = -1;
    }

    Array<int>* in_map_c = 0;

    if (react_vsize > 0 && in_map != 0 && in_map->size() > 0)
    {
        in_map_c = &in_map_cut;

        in_map_cut.clear();
        in_map_cut.resize(mol_react.vertexEnd());

        for (int k = 0; k < in_map_cut.size(); k++)
            in_map_cut[k] = SubstructureMcs::UNMAPPED;

        for (int k = mol_react.vertexBegin(); k < mol_react.vertexEnd(); k = mol_react.vertexNext(k))
        {
            in_map_cut[k] = in_map->at(k);
        }
    }

    // first more strict rules then more weak

    results[0] = _searchSubstructure(emb_enums[0], in_map_c, &tmp_maps[0]);
    results[1] = _searchSubstructure(emb_enums[1], in_map_c, &tmp_maps[1]);

    mol_react.clone(init_rmol, 0, 0);
    mol_react.aromatize(arom_options);

    if (mol_react.vertexCount() > react_vsize)
    {
        results[2] = _searchSubstructure(emb_enums[2], in_map, &tmp_maps[2]);
        results[3] = _searchSubstructure(emb_enums[3], in_map, &tmp_maps[3]);
    }

    map_result = 3;
    for (int i = 2; i >= 0; --i)
    {
        if (results[i] >= results[map_result])
            map_result = i;
    }
    if (results[map_result] < 2)
        return false;

    if (out_map != 0)
        out_map->copy(tmp_maps[map_result]);
    return true;
}

bool RSubstructureMcs::searchMaxCommonSubReact(const Array<int>* in_map, Array<int>* out_map)
{

    if (_sub == 0 || _super == 0)
        throw ReactionAutomapper::Error("internal AAM error: not initialized sub-mcs molecules");

    if (out_map != 0)
        out_map->clear();

    //   if(_super->vertexCount() < 2 || _sub->vertexCount() < 2)
    //      return false;

    Molecule* sub_molecule;
    Molecule* super_molecule;
    if (_invert)
    {
        sub_molecule = (Molecule*)_super;
        super_molecule = (Molecule*)_sub;
    }
    else
    {
        sub_molecule = (Molecule*)_sub;
        super_molecule = (Molecule*)_super;
    }

    MaxCommonSubmolecule mcs(*sub_molecule, *super_molecule);
    mcs.parametersForExact.maxIteration = MAX_ITERATION_NUMBER;
    mcs.conditionVerticesColor = atomConditionReact;
    mcs.conditionEdgeWeight = bondConditionReact;
    mcs.cbSolutionTerm = cbMcsSolutionTerm;
    mcs.userdata = this;
    /*
     * Do not randomize
     */
    //   mcs.parametersForApproximate.randomize = true;

    if (in_map != 0)
    {
        _transposeInputMap(in_map, mcs.incomingMap);
    }

    try
    {
        /*
         * Search for exact mcs first
         */
        mcs.findExactMCS();
        /*
         * Search for approximate mcs
         */
        if (mcs.parametersForExact.isStopped)
        {
            mcs.findApproximateMCS();
        }
    }
    catch (Exception& e)
    {
        if (strstr(e.message(), "input mapping incorrect") != 0)
            throw e;
        return false;
    }

    mcs.getMaxSolutionMap(out_map, 0);
    /*
     * Search best solution as far as mcs is not consider auto permutations
     */
    _selectBestAutomorphism(out_map);
    /*
     * Transpose map back
     */
    _detransposeOutputMap(out_map);
    return true;
}

int RSubstructureMcs::findReactionCenter(BaseMolecule& mol, int bondNum) const
{
    if (_sub == 0 || _super == 0)
        throw ReactionAutomapper::Error("internal AAM error: not initialized sub-mcs molecules");

    if (_sub == &mol)
    {
        bondNum = _getTransposedBondIndex(mol, bondNum);
        if (_invert)
        {
            return _reaction.getReactingCenter(_superProductNumber, bondNum);
        }
        else
        {
            return _reaction.getReactingCenter(_subReactNumber, bondNum);
        }
    }
    if (_super == &mol)
    {
        if (_invert)
        {
            return _reaction.getReactingCenter(_subReactNumber, bondNum);
        }
        else
        {
            return _reaction.getReactingCenter(_superProductNumber, bondNum);
        }
    }
    return -2;
}

void RSubstructureMcs::getReactingCenters(BaseMolecule& mol1, BaseMolecule& mol2, int bond1, int bond2, int& rc_reactant, int& rc_product) const
{
    if (_sub == 0 || _super == 0)
        throw ReactionAutomapper::Error("internal AAM error: not initialized sub-mcs molecules");

    if (_sub == &mol1 && _super == &mol2)
    {
        bond1 = _getTransposedBondIndex(mol1, bond1);
        if (_invert)
        {
            rc_reactant = _reaction.getReactingCenter(_subReactNumber, bond2);
            rc_product = _reaction.getReactingCenter(_superProductNumber, bond1);
        }
        else
        {
            rc_reactant = _reaction.getReactingCenter(_subReactNumber, bond1);
            rc_product = _reaction.getReactingCenter(_superProductNumber, bond2);
        }
    }
    if (_sub == &mol2 && _super == &mol1)
    {
        bond2 = _getTransposedBondIndex(mol2, bond2);
        if (_invert)
        {
            rc_reactant = _reaction.getReactingCenter(_subReactNumber, bond1);
            rc_product = _reaction.getReactingCenter(_superProductNumber, bond2);
        }
        else
        {
            rc_reactant = _reaction.getReactingCenter(_subReactNumber, bond2);
            rc_product = _reaction.getReactingCenter(_superProductNumber, bond1);
        }
    }
}

bool RSubstructureMcs::atomConditionReact(Graph& g1, Graph& g2, const int*, int i, int j, void* userdata)
{
    BaseMolecule& mol1 = (BaseMolecule&)g1;
    BaseMolecule& mol2 = (BaseMolecule&)g2;
    if (userdata == 0)
        throw ReactionAutomapper::Error("internal AAM error: userdata should be not null for atom match");
    RSubstructureMcs& rsm = *(RSubstructureMcs*)userdata;

    return _matchAtoms(mol1, mol2, i, j, rsm.flags);
}
bool RSubstructureMcs::bondConditionReact(Graph& g1, Graph& g2, int i, int j, void* userdata)
{

    BaseMolecule& mol1 = (BaseMolecule&)g1;
    BaseMolecule& mol2 = (BaseMolecule&)g2;
    if (userdata == 0)
        throw ReactionAutomapper::Error("internal AAM error: userdata should be not null for bond match");
    RSubstructureMcs& rsm = *(RSubstructureMcs*)userdata;

    int rc_reactant;
    int rc_product;

    rsm.getReactingCenters(mol1, mol2, i, j, rc_reactant, rc_product);

    // not consider
    if (rc_reactant == RC_MADE_OR_BROKEN || rc_product == RC_MADE_OR_BROKEN)
        return false;

    // aromatic
    if (mol1.getBondOrder(i) == BOND_AROMATIC || mol2.getBondOrder(j) == BOND_AROMATIC)
        return true;

    // not change
    if (rc_reactant == RC_UNMARKED && rc_product == RC_UNMARKED)
        return mol1.getBondOrder(i) == mol2.getBondOrder(j);

    // not change reactants
    if ((rc_reactant == RC_NOT_CENTER || rc_reactant == RC_UNCHANGED || rc_reactant == RC_UNCHANGED + RC_MADE_OR_BROKEN))
        return mol1.getBondOrder(i) == mol2.getBondOrder(j);

    // change reactants
    if ((rc_reactant == RC_ORDER_CHANGED || rc_reactant == RC_ORDER_CHANGED + RC_MADE_OR_BROKEN))
        return mol1.getBondOrder(i) != mol2.getBondOrder(j);

    // not change products
    if ((rc_product == RC_NOT_CENTER || rc_product == RC_UNCHANGED || rc_product == RC_UNCHANGED + RC_MADE_OR_BROKEN))
        return mol1.getBondOrder(i) == mol2.getBondOrder(j);

    // change products
    if ((rc_product == RC_ORDER_CHANGED || rc_product == RC_ORDER_CHANGED + RC_MADE_OR_BROKEN))
        return mol1.getBondOrder(i) != mol2.getBondOrder(j);

    // can change
    return true;
}

bool RSubstructureMcs::bondConditionReactStrict(Graph& g1, Graph& g2, int i, int j, void* userdata)
{

    BaseMolecule& mol1 = (BaseMolecule&)g1;
    BaseMolecule& mol2 = (BaseMolecule&)g2;
    if (userdata == 0)
        throw ReactionAutomapper::Error("internal AAM error: userdata should be not null for bond strict match");
    RSubstructureMcs& rsm = *(RSubstructureMcs*)userdata;

    int rc_reactant;
    int rc_product;

    rsm.getReactingCenters(mol1, mol2, i, j, rc_reactant, rc_product);

    // not consider
    if ((rc_reactant & RC_MADE_OR_BROKEN) || (rc_product & RC_MADE_OR_BROKEN))
        return false;

    // aromatic
    if (mol1.getBondOrder(i) == BOND_AROMATIC || mol2.getBondOrder(j) == BOND_AROMATIC)
        return true;

    // not change
    if (rc_reactant == RC_UNMARKED && rc_product == RC_UNMARKED)
        return mol1.getBondOrder(i) == mol2.getBondOrder(j);

    // not change reactants
    if (rc_reactant == RC_NOT_CENTER || rc_reactant == RC_UNCHANGED)
        return mol1.getBondOrder(i) == mol2.getBondOrder(j);

    // change reactants
    if (rc_reactant == RC_ORDER_CHANGED)
        return mol1.getBondOrder(i) != mol2.getBondOrder(j);

    // not change products
    if ((rc_product == RC_NOT_CENTER || rc_product == RC_UNCHANGED))
        return mol1.getBondOrder(i) == mol2.getBondOrder(j);

    // change products
    if (rc_product == RC_ORDER_CHANGED)
        return mol1.getBondOrder(i) != mol2.getBondOrder(j);

    // can change
    return true;
}

bool RSubstructureMcs::bondConditionReactSimple(Graph& g1, Graph& g2, int i, int j, void* userdata)
{
    BaseMolecule& mol1 = (BaseMolecule&)g1;
    BaseMolecule& mol2 = (BaseMolecule&)g2;

    if (mol1.getBondOrder(i) != mol2.getBondOrder(j))
        return false;

    return true;
}

int RSubstructureMcs::cbMcsSolutionTerm(Array<int>& a1, Array<int>& a2, void* context)
{
    int result = MaxCommonSubgraph::ringsSolutionTerm(a1, a2, context);
    if (result == 0)
    {
        const RSubstructureMcs* r_sub_mcs = (const RSubstructureMcs*)context;
        BaseReaction& reaction = r_sub_mcs->getReaction();
        int react = r_sub_mcs->getReactNumber();
        int prod = r_sub_mcs->getProductNumber();
        int e_size1 = a1[1];
        int e_size2 = a2[1];
        int r1_rc = 0, r2_rc = 0;
        for (int i = 0; i < e_size1; ++i)
        {
            int e_map = a1.at(2 + a1[0] + i);
            if (e_map >= 0)
            {
                if ((reaction.getReactingCenter(react, i) & RC_MADE_OR_BROKEN))
                    ++r1_rc;
                if ((reaction.getReactingCenter(prod, e_map) & RC_MADE_OR_BROKEN))
                    ++r1_rc;
            }
        }
        for (int i = 0; i < e_size2; ++i)
        {
            int e_map = a2.at(2 + a2[0] + i);
            if (e_map >= 0)
            {
                if ((reaction.getReactingCenter(react, i) & RC_MADE_OR_BROKEN))
                    ++r2_rc;
                if ((reaction.getReactingCenter(prod, e_map) & RC_MADE_OR_BROKEN))
                    ++r2_rc;
            }
        }
        result = r1_rc - r2_rc;
    }
    return result;
}

int RSubstructureMcs::_searchSubstructure(EmbeddingEnumerator& emb_enum, const Array<int>* in_map, Array<int>* out_map)
{

    if (_sub == 0 || _super == 0)
        throw ReactionAutomapper::Error("internal AAM error: not initialized sub-mcs molecules");
    QS_DEF(Array<int>, input_map);
    if (in_map != 0)
    {
        _transposeInputMap(in_map, input_map);
        in_map = &input_map;
    }
    int result = 0;
    if (in_map != 0)
    {
        for (int i = 0; i < in_map->size(); i++)
        {
            if (in_map->at(i) >= 0 && !_invert)
            {
                if (!emb_enum.fix(i, in_map->at(i)))
                {
                    result = -1;
                    break;
                }
            }

            if (in_map->at(i) >= 0 && _invert)
            {
                if (!emb_enum.fix(in_map->at(i), i))
                {
                    result = -1;
                    break;
                }
            }
        }
    }

    if (result == -1)
        return result;

    int proc = 1;
    /*
     * Disable old logic with timeout waiting for mcs. Can be a different result for some reaction though
     */
    // if (_context.cancellation != nullptr)
    // {
    //     try
    //     {
    //         proc = emb_enum.process();
    //     }
    //     catch (Exception&)
    //     {
    //         proc = 1;
    //     }
    // }
    // else
    // {
    // proc = emb_enum.process();
    // }

    // Will throw error on timeout
    proc = emb_enum.process();
    if (proc == 1)
        return -1;

    int ncomp = _sub->countComponents();
    const Array<int>& decomposition = _sub->getDecomposition();

    int j, max_index = 0;

    for (j = 1; j < ncomp; j++)
        if (_sub->countComponentVertices(j) > _sub->countComponentVertices(max_index))
            max_index = j;

    if (out_map != 0)
    {
        if (!_invert)
        {
            out_map->clear_resize(_sub->vertexEnd());
            for (int i = 0; i < out_map->size(); i++)
                out_map->at(i) = SubstructureMcs::UNMAPPED;
            for (int i = _sub->vertexBegin(); i < _sub->vertexEnd(); i = _sub->vertexNext(i))
            {
                out_map->at(i) = emb_enum.getSubgraphMapping()[i];
                if (max_index != decomposition[i])
                    out_map->at(i) = SubstructureMcs::UNMAPPED;

                if (out_map->at(i) >= 0)
                    ++result;
            }
        }
        else
        {
            out_map->clear_resize(_super->vertexEnd());
            for (int i = 0; i < out_map->size(); i++)
                out_map->at(i) = SubstructureMcs::UNMAPPED;
            for (int i = _super->vertexBegin(); i < _super->vertexEnd(); i = _super->vertexNext(i))
            {
                out_map->at(i) = emb_enum.getSupergraphMapping()[i];
                if ((out_map->at(i) >= 0) && (max_index != decomposition[out_map->at(i)]))
                    out_map->at(i) = SubstructureMcs::UNMAPPED;
                if (out_map->at(i) >= 0)
                    ++result;
            }
        }
    }
    _detransposeOutputMap(out_map);
    return result;
}

bool RSubstructureMcs::_matchAtoms(BaseMolecule& query, BaseMolecule& target, int sub_idx, int super_idx, int flags)
{

    if (query.isRSite(sub_idx) && target.isRSite(super_idx))
        return query.getRSiteBits(sub_idx) == target.getRSiteBits(super_idx);

    if (query.isRSite(sub_idx) || target.isRSite(super_idx))
        return false;

    if (query.isPseudoAtom(sub_idx) && target.isPseudoAtom(super_idx))
    {
        if (strcmp(query.getPseudoAtom(sub_idx), target.getPseudoAtom(super_idx)) != 0)
            return false;
    }
    else if (!query.isPseudoAtom(sub_idx) && !target.isPseudoAtom(super_idx))
    {
        if (query.getAtomNumber(sub_idx) != target.getAtomNumber(super_idx))
            return false;
    }
    else
        return false;

    if (flags & CONDITION_ATOM_ISOTOPES)
        if (query.getAtomIsotope(sub_idx) != target.getAtomIsotope(super_idx))
            return false;

    if (flags & CONDITION_ATOM_CHARGES)
    {
        int qcharge = query.getAtomCharge(sub_idx);
        int tcharge = target.getAtomCharge(super_idx);

        if (qcharge == CHARGE_UNKNOWN)
            qcharge = 0;
        if (tcharge == CHARGE_UNKNOWN)
            tcharge = 0;

        if (qcharge != tcharge)
            return false;
    }

    if (flags & CONDITION_ATOM_VALENCE)
    {
        if (!query.isPseudoAtom(sub_idx))
        {
            if (!query.isQueryMolecule() && !target.isQueryMolecule())
            {
                if (query.getAtomValence(sub_idx) != target.getAtomValence(super_idx))
                    return false;
            }
        }
    }

    if (flags & CONDITION_ATOM_RADICAL)
    {
        if (!query.isPseudoAtom(sub_idx))
        {
            int qrad = query.getAtomRadical(sub_idx);
            int trad = target.getAtomRadical(super_idx);

            if (qrad == -1)
                qrad = 0;
            if (trad == -1)
                trad = 0;

            if (qrad != trad)
                return false;
        }
    }

    return true;
}

void RSubstructureMcs::_selectBestAutomorphism(Array<int>* map_out)
{
    if (map_out == 0)
        return;

    QS_DEF(Array<int>, ignore_atoms);
    QS_DEF(Array<int>, current_map);

    BaseMolecule* sub_molecule;
    BaseMolecule* super_molecule;
    if (_invert)
    {
        sub_molecule = (BaseMolecule*)_super;
        super_molecule = (BaseMolecule*)_sub;
    }
    else
    {
        sub_molecule = (BaseMolecule*)_sub;
        super_molecule = (BaseMolecule*)_super;
    }

    ignore_atoms.resize(super_molecule->vertexEnd());
    ignore_atoms.fill(1);
    for (int i = 0; i < map_out->size(); ++i)
    {
        if (map_out->at(i) >= 0)
            ignore_atoms[map_out->at(i)] = 0;
    }

    /*
     * Set callbacks
     */
    AutomorphismSearch auto_search;
    auto_search.cb_check_automorphism = _cbAutoCheckAutomorphismReact;
    auto_search.ignored_vertices = ignore_atoms.ptr();
    auto_search.getcanon = false;
    auto_search.context = this;
    /*
     * Find all automorphisms
     */
    _autoMaps.clear();
    auto_search.process(*super_molecule);

    /*
     * Score initial solution
     */
    int best_solution = -1;
    int best_score;
    best_score = _scoreSolution(sub_molecule, super_molecule, *map_out);
    /*
     * Score the best solution
     */
    for (int aut_idx = 0; aut_idx < _autoMaps.size(); ++aut_idx)
    {
        /*
         * Convert mapping
         */
        current_map.copy(*map_out);
        for (int i = 0; i < current_map.size(); ++i)
        {
            int sup_idx = current_map[i];
            if (sup_idx >= 0)
                current_map[i] = _autoMaps[aut_idx][sup_idx];
        }
        /*
         * Calculate current score
         */
        int current_score = _scoreSolution(sub_molecule, super_molecule, current_map);
        if (current_score > best_score)
        {
            best_score = current_score;
            best_solution = aut_idx;
        }
    }
    /*
     * Select best scoring solution
     */
    if (best_solution >= 0)
    {
        current_map.copy(*map_out);
        for (int i = 0; i < current_map.size(); ++i)
        {
            int sup_idx = current_map[i];
            if (sup_idx >= 0)
                current_map[i] = _autoMaps[best_solution][sup_idx];
        }
        map_out->copy(current_map);
    }
}

int RSubstructureMcs::_cbAutoVertexReact(Graph& graph, int idx1, int idx2, const void* context)
{
    return atomConditionReact(graph, graph, 0, idx1, idx2, (void*)context);
}

bool RSubstructureMcs::_cbAutoCheckAutomorphismReact(Graph& graph, const Array<int>& mapping, const void* context)
{
    RSubstructureMcs& rsm = *(RSubstructureMcs*)context;
    rsm._autoMaps.push().copy(mapping);
    return false;
}

int RSubstructureMcs::_scoreSolution(BaseMolecule* sub_molecule, BaseMolecule* super_molecule, Array<int>& v_map)
{
    int res_score = 0;
    QS_DEF(Array<int>, edge_map);
    edge_map.clear_resize(sub_molecule->edgeEnd());
    edge_map.fffill();

    for (int vert_idx = 0; vert_idx < v_map.size(); ++vert_idx)
    {
        int super_vert = v_map[vert_idx];
        if (super_vert >= 0)
        {
            /*
             * Score vertex degree keeping
             */
            const Vertex& sub_vert = sub_molecule->getVertex(vert_idx);
            if (sub_vert.degree() == super_molecule->getVertex(super_vert).degree())
                ++res_score;
            for (int k = sub_vert.neiBegin(); k != sub_vert.neiEnd(); k = sub_vert.neiNext(k))
            {
                int nei_vert = sub_vert.neiVertex(k);
                if (v_map[nei_vert] >= 0)
                {
                    int sub_edge = sub_molecule->findEdgeIndex(vert_idx, nei_vert);
                    int super_edge = super_molecule->findEdgeIndex(super_vert, v_map[nei_vert]);
                    if (sub_edge != -1 && super_edge != -1)
                        edge_map[sub_edge] = super_edge;
                }
            }
        }
    }
    /*
     * Score bond order keeping with priority
     */
    for (int sub_idx = 0; sub_idx < edge_map.size(); ++sub_idx)
    {
        int super_idx = edge_map[sub_idx];
        if (super_idx >= 0)
        {
            if (bondConditionReact(*sub_molecule, *super_molecule, sub_idx, super_idx, this) &&
                (sub_molecule->getBondOrder(sub_idx) == super_molecule->getBondOrder(super_idx)))
            {
                res_score += HIGH_PRIORITY_SCORE;
            }
        }
    }
    /*
     * Score atom number keeping with priority
     */
    for (int sub_idx = 0; sub_idx < v_map.size(); ++sub_idx)
    {
        int super_idx = v_map[sub_idx];
        if (super_idx >= 0)
        {
            if (atomConditionReact(*sub_molecule, *super_molecule, 0, sub_idx, super_idx, this))
                res_score += HIGH_PRIORITY_SCORE;
        }
    }

    return res_score;
}

void RSubstructureMcs::_createQueryTransposition()
{
    QS_DEF(Array<int>, transposition);

    MoleculeAtomNeighbourhoodCounters nei_counters;
    if (_reaction.isQueryReaction())
    {
        nei_counters.calculate((QueryMolecule&)*_sub);
        _transposedQuery = std::make_unique<QueryMolecule>();
    }
    else
    {
        nei_counters.calculate((Molecule&)*_sub);
        _transposedQuery = std::make_unique<Molecule>();
    }

    nei_counters.makeTranspositionForSubstructure((BaseMolecule&)*_sub, transposition);
    /*
     * Create map
     */
    _transposedQuery->makeSubmolecule((BaseMolecule&)*_sub, transposition, &_transposition);

    /*
     * Create inv map
     */
    _invTransposition.resize(_transposition.size());
    _invTransposition.fffill();
    for (int i = 0; i < _transposition.size(); ++i)
    {
        if (_transposition[i] >= 0)
            _invTransposition[_transposition[i]] = i;
    }
    /*
     * Create bond map
     */
    _bondTransposition.resize(_transposedQuery->edgeEnd());
    _bondTransposition.fffill();
    for (int e_idx = _sub->edgeBegin(); e_idx != _sub->edgeEnd(); e_idx = _sub->edgeNext(e_idx))
    {
        int vert1 = _transposition[_sub->getEdge(e_idx).beg];
        int vert2 = _transposition[_sub->getEdge(e_idx).end];
        int edge = _transposedQuery->findEdgeIndex(vert1, vert2);
        if (edge >= 0)
            _bondTransposition[edge] = e_idx;
    }

    _sub = _transposedQuery.get();
}

void RSubstructureMcs::_detransposeOutputMap(Array<int>* map) const
{
    if (map && _transposedQuery.get())
    {
        QS_DEF(Array<int>, buf_map);
        if (_invert)
        {
            buf_map.resize(map->size());
            buf_map.fffill();
            for (int i = 0; i < map->size(); ++i)
            {
                if (map->at(i) >= 0)
                    buf_map[i] = _invTransposition[map->at(i)];
            }
            map->copy(buf_map);
        }
        else
        {
            buf_map.resize(_transposition.size());
            buf_map.fffill();
            for (int i = 0; i < map->size(); ++i)
            {
                if (_invTransposition[i] >= 0)
                    buf_map[_invTransposition[i]] = map->at(i);
            }
            map->copy(buf_map);
        }
    }
}

void RSubstructureMcs::_transposeInputMap(const Array<int>* map, Array<int>& input_map) const
{
    input_map.clear();
    if (map == 0)
        return;
    if (_transposedQuery.get())
    {
        input_map.resize(map->size());
        input_map.fffill();
        if (_invert)
        {
            for (int i = 0; i < map->size(); ++i)
            {
                if (map->at(i) >= 0)
                    input_map[i] = _transposition[map->at(i)];
            }
        }
        else
        {
            for (int i = 0; i < map->size(); ++i)
            {
                if (_transposition[i] >= 0)
                    input_map[_transposition[i]] = map->at(i);
            }
        }
    }
    else
    {
        input_map.copy(*map);
    }
}

int RSubstructureMcs::_getTransposedBondIndex(BaseMolecule& mol, int bond) const
{
    int result = bond;

    if (_transposedQuery.get() != 0 && _sub == &mol)
        result = _bondTransposition[bond];

    return result;
}

AAMCancellationWrapper::AAMCancellationWrapper(std::shared_ptr<CancellationHandler> canc) : _contains(false)
{
    _prev = resetCancellationHandler(canc);
    _contains = true;
}

void AAMCancellationWrapper::reset()
{
    resetCancellationHandler(_prev);
    _contains = false;
}
AAMCancellationWrapper::~AAMCancellationWrapper()
{
    if (_contains)
    {
        reset();
    }
}