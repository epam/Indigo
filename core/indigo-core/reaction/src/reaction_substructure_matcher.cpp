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

#include "reaction/reaction_substructure_matcher.h"
#include "molecule/molecule_3d_constraints.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_neighbourhood_counters.h"
#include "molecule/molecule_substructure_matcher.h"
#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_neighborhood_counters.h"

using namespace indigo;

IMPL_ERROR(ReactionSubstructureMatcher, "reaction substructure matcher");

ReactionSubstructureMatcher::ReactionSubstructureMatcher(Reaction& target) : BaseReactionSubstructureMatcher(target), TL_CP_GET(_fmcaches)
{
    match_atoms = _match_atoms;
    match_bonds = _match_bonds;
    add_bond = _add_bond;
    remove_atom = _remove_atom;
    prepare_ee = _prepare_ee;
    use_daylight_aam_mode = false;
    context = this;
    _fmcaches.clear();
}

bool ReactionSubstructureMatcher::_match_atoms(BaseReaction& query_, Reaction& target, int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
                                               void* context)
{
    QueryReaction& query = query_.asQueryReaction();
    QueryMolecule& submol = query.getQueryMolecule(sub_mol_idx);
    Molecule& supermol = target.getMolecule(super_mol_idx);
    ReactionSubstructureMatcher& self = *(ReactionSubstructureMatcher*)context;

    self._fmcaches.expand(sub_mol_idx + 1);

    if (!MoleculeSubstructureMatcher::matchQueryAtom(&submol.getAtom(sub_atom_idx), supermol, super_atom_idx, &self._fmcaches[sub_mol_idx], 0xFFFFFFFFUL))
        return false;

    if (submol.stereocenters.getType(sub_atom_idx) > supermol.stereocenters.getType(super_atom_idx))
        return false;

    if (query.getExactChange(sub_mol_idx, sub_atom_idx) == 1)
    {
        const Vertex& can_vertex = submol.getVertex(sub_atom_idx);
        int ch_flag;
        int bonds_changes[RC_TOTAL] = {0};
        int i;

        for (i = can_vertex.neiBegin(); i != can_vertex.neiEnd(); i = can_vertex.neiNext(i))
        {
            ch_flag = query.getReactingCenter(sub_atom_idx, can_vertex.neiEdge(i));

            if (ch_flag == RC_NOT_CENTER)
                ch_flag = RC_UNCHANGED;

            if (ch_flag > 0)
                bonds_changes[ch_flag]++;
        }

        const Vertex& pat_vertex = supermol.getVertex(super_atom_idx);

        for (i = pat_vertex.neiBegin(); i != pat_vertex.neiEnd(); i = pat_vertex.neiNext(i))
        {

            ch_flag = target.getReactingCenter(super_mol_idx, pat_vertex.neiEdge(i));
            if (ch_flag > 0)
                bonds_changes[ch_flag]--;
        }

        int n_centers = bonds_changes[RC_CENTER];
        bonds_changes[RC_CENTER] = 0;

        if (n_centers < 0)
            return false;

        for (int i = 0; i < NELEM(bonds_changes); i++)
        {
            if ((ch_flag = bonds_changes[i]) > 0)
                return false;
            else if (ch_flag < 0)
            {
                if ((n_centers += ch_flag) < 0)
                    return false;
            }
        }

        if (n_centers != 0)
            return false;
    }

    return true;
}

bool ReactionSubstructureMatcher::_match_bonds(BaseReaction& query_, Reaction& target, int sub_mol_idx, int sub_bond_idx, int super_mol_idx, int super_bond_idx,
                                               AromaticityMatcher* am, void* context)
{
    QueryReaction& query = query_.asQueryReaction();
    QueryMolecule& submol = query.getQueryMolecule(sub_mol_idx);
    Molecule& supermol = target.getMolecule(super_mol_idx);

    if (!MoleculeSubstructureMatcher::matchQueryBond(&submol.getBond(sub_bond_idx), supermol, sub_bond_idx, super_bond_idx, am, 0xFFFFFFFFUL))
        return false;

    int sub_change = query.getReactingCenter(sub_mol_idx, sub_bond_idx);
    int super_change = target.getReactingCenter(super_mol_idx, super_bond_idx);

    if (super_change == RC_UNMARKED)
        return true;

    // super_change == (RC_UNCHANGED + RC_ORDER_CHANGED) is for changed aromatics
    if (sub_change == RC_NOT_CENTER || sub_change == RC_UNCHANGED)
        return super_change == 0 || super_change == RC_UNCHANGED || super_change == (RC_UNCHANGED + RC_ORDER_CHANGED);

    if (sub_change == RC_CENTER)
        return super_change != 0 && super_change != RC_UNCHANGED && super_change != RC_NOT_CENTER;

    if ((sub_change & super_change) != sub_change)
        return false;

    return true;
}

void ReactionSubstructureMatcher::_remove_atom(BaseMolecule& submol, int sub_idx, AromaticityMatcher* am)
{
    MoleculeSubstructureMatcher::removeAtom(submol, sub_idx, am);
}

void ReactionSubstructureMatcher::_add_bond(BaseMolecule& submol, Molecule& supermol, int sub_idx, int super_idx, AromaticityMatcher* am)
{
    MoleculeSubstructureMatcher::addBond(submol, supermol, sub_idx, super_idx, am);
}

bool ReactionSubstructureMatcher::_prepare(BaseReaction& query_, Reaction& target, void* context)
{
    ReactionSubstructureMatcher& self = *(ReactionSubstructureMatcher*)context;

    self._fmcaches.clear();
    return true;
}

bool ReactionSubstructureMatcher::_prepare_ee(EmbeddingEnumerator& ee, BaseMolecule& submol, Molecule& supermol, void* context)
{
    // mark hydrogens to ignore
    QS_DEF(Array<int>, ignored);

    ignored.clear_resize(submol.vertexEnd());

    MoleculeSubstructureMatcher::markIgnoredQueryHydrogens(submol.asQueryMolecule(), ignored.ptr(), 0, 1);

    for (int i = submol.vertexBegin(); i != submol.vertexEnd(); i = submol.vertexNext(i))
        if (ignored[i])
            ee.ignoreSubgraphVertex(i);

    return true;
}

bool ReactionSubstructureMatcher::_checkAAM()
{
    if (!use_daylight_aam_mode)
        return BaseReactionSubstructureMatcher::_checkAAM();

    QS_DEF(ObjArray<RedBlackSet<int>>, classes_mapping_left);
    QS_DEF(ObjArray<RedBlackSet<int>>, classes_mapping_right);
    int i;

    classes_mapping_left.clear();
    classes_mapping_right.clear();

    // Collect AAM class-to-class mappings separately for
    // the reactands and products
    for (i = 0; i < _matchers.size() - 1; i++)
    {
        int qmol_idx = _matchers[i]->_current_molecule_1;
        int tmol_idx = _matchers[i]->_current_molecule_2;
        BaseMolecule& qmol = _query->getBaseMolecule(qmol_idx);
        ObjArray<RedBlackSet<int>>* cm;
        int j;

        if (_query->getSideType(qmol_idx) == BaseReaction::REACTANT)
            cm = &classes_mapping_left;
        else
            cm = &classes_mapping_right;

        for (j = qmol.vertexBegin(); j != qmol.vertexEnd(); j = qmol.vertexNext(j))
        {
            int qaam = _query->getAAM(qmol_idx, j);

            if (qaam == 0)
                continue;

            if (_query->asQueryReaction().getIgnorableAAM(qmol_idx, j))
                continue;

            int mapped = _matchers[i]->_current_core_1[j];

            if (mapped < 0)
                throw Error("internal error: can not call atom with negative number %d", mapped);

            int taam = _target.getAAM(tmol_idx, mapped);

            if (taam == 0)
                return false;

            cm->expand(qaam + 1);
            cm->at(qaam).find_or_insert(taam);
        }
    }

    // http://www.daylight.com/dayhtml/doc/theory/theory.smarts.html
    // "The query reactants can be bound to any classes in the target.
    // These bindings form the set of allowed product bindings.
    // The product query atoms are then tested against this list.
    // If all of the product atoms pass, then the path is a match"
    for (i = 0; i < classes_mapping_right.size(); i++)
    {
        if (i >= classes_mapping_left.size())
            // "When a query class is not found on both sides of the query, it is ignored"
            break;

        RedBlackSet<int>& right = classes_mapping_right[i];
        RedBlackSet<int>& left = classes_mapping_left[i];

        for (int j = right.begin(); j != right.end(); j = right.next(j))
        {
            int aam_class = right.key(j);
            if (!left.find(aam_class))
                return false;
        }
    }

    return true;
}
