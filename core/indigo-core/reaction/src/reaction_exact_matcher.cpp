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

#include "reaction/reaction_exact_matcher.h"
#include "molecule/elements.h"
#include "molecule/molecule_arom_match.h"
#include "molecule/molecule_exact_matcher.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(ReactionExactMatcher, "reaction exact matcher");

ReactionExactMatcher::ReactionExactMatcher(Reaction& query, Reaction& target) : BaseReactionSubstructureMatcher(target), _query(query), _target(target)
{
    setQuery(query);
    match_atoms = _match_atoms;
    match_bonds = _match_bonds;
    context = this;
    prepare = _prepare;
    prepare_ee = _prepare_ee;
    flags = 0xFFFFFFFFUL;
}

bool ReactionExactMatcher::_match_atoms(BaseReaction& query_, Reaction& target, int sub_mol_idx, int sub_atom_idx, int super_mol_idx, int super_atom_idx,
                                        void* context)
{
    ReactionExactMatcher& self = *(ReactionExactMatcher*)context;
    Reaction& query = query_.asReaction();
    Molecule& submol = query.getMolecule(sub_mol_idx);
    Molecule& supermol = target.getMolecule(super_mol_idx);

    if (!MoleculeExactMatcher::matchAtoms(submol, supermol, sub_atom_idx, super_atom_idx, self.flags))
        return false;

    if (self.flags & CONDITION_AAM)
    {
        if ((query.getAAM(sub_mol_idx, sub_atom_idx) == 0) != (target.getAAM(super_mol_idx, super_atom_idx) == 0))
            return false;
    }

    return true;
}

bool ReactionExactMatcher::_match_bonds(BaseReaction& query_, Reaction& target, int sub_mol_idx, int sub_bond_idx, int super_mol_idx, int super_bond_idx,
                                        AromaticityMatcher* /*am*/, void* context)
{
    ReactionExactMatcher& self = *(ReactionExactMatcher*)context;
    Reaction& query = query_.asReaction();
    Molecule& submol = query.getMolecule(sub_mol_idx);
    Molecule& supermol = target.getMolecule(super_mol_idx);

    if (!MoleculeExactMatcher::matchBonds(submol, supermol, sub_bond_idx, super_bond_idx, self.flags))
        return false;

    if (self.flags & CONDITION_REACTING_CENTERS)
    {
        int sub_change = query.getReactingCenter(sub_mol_idx, sub_bond_idx);
        int super_change = target.getReactingCenter(super_mol_idx, super_bond_idx);

        if (sub_change != super_change)
            return false;
    }

    return true;
}

bool ReactionExactMatcher::_prepare(BaseReaction& query, Reaction& target, void* context)
{
    if (query.count() != target.count())
        return false;

    ReactionExactMatcher& self = *(ReactionExactMatcher*)context;

    if (self.flags & MoleculeExactMatcher::CONDITION_STEREO)
        self._match_stereo = true;
    else
        self._match_stereo = false;

    return true;
}

bool ReactionExactMatcher::_prepare_ee(EmbeddingEnumerator& ee, BaseMolecule& submol, Molecule& supermol, void* context)
{
    int i;

    ReactionExactMatcher& self = *(ReactionExactMatcher*)context;

    for (i = submol.vertexBegin(); i != submol.vertexEnd(); i = submol.vertexNext(i))
    {
        const Vertex& vertex = submol.getVertex(i);

        if (submol.getAtomNumber(i) == ELEM_H && vertex.degree() == 1 && submol.getAtomNumber(vertex.neiVertex(vertex.neiBegin())) != ELEM_H)
            if (submol.getAtomIsotope(i) == 0 || !(self.flags & MoleculeExactMatcher::CONDITION_ISOTOPE))
                ee.ignoreSubgraphVertex(i);
    }

    for (i = supermol.vertexBegin(); i != supermol.vertexEnd(); i = supermol.vertexNext(i))
    {
        const Vertex& vertex = supermol.getVertex(i);

        if (supermol.getAtomNumber(i) == ELEM_H && vertex.degree() == 1 && supermol.getAtomNumber(vertex.neiVertex(vertex.neiBegin())) != ELEM_H)
            if (supermol.getAtomIsotope(i) == 0 || !(self.flags & MoleculeExactMatcher::CONDITION_ISOTOPE))
                ee.ignoreSupergraphVertex(i);
    }

    if (ee.countUnmappedSubgraphVertices() != ee.countUnmappedSupergraphVertices())
        return false;
    if (ee.countUnmappedSubgraphEdges() != ee.countUnmappedSupergraphEdges())
        return false;

    return true;
}
