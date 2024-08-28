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

#include <queue>

#include "molecule/inchi_wrapper.h"
#include "molecule/ket_commons.h"
#include "reaction/pathway_reaction.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(PathwayReaction, "pathway reaction");

PathwayReaction::PathwayReaction()
{
}

PathwayReaction::~PathwayReaction()
{
}

PathwayReaction::PathwayReaction(std::deque<Reaction>& reactions, const Array<ReactionNode>& nodes)
{
    _reactionNodes.copy(nodes);
    for (size_t i = 0; i < reactions.size(); i++)
    {
        auto& reactionComponents = _reactions.push();
        for (int j = reactions[i].begin(); j < reactions[i].end(); j = reactions[i].next(j))
        {
            auto molecule = std::make_unique<Molecule>();
            molecule->clone(reactions[i].getBaseMolecule(j));
            int id = _allMolecules.add(molecule.release());
            _addedBaseMolecule(id, reactions[i].getSideType(j), *_allMolecules[id]);
            reactionComponents.insert(j, id);
        }
    }
}


std::vector<std::reference_wrapper<const PathwayReaction::ReactionNode>> PathwayReaction::getRootReactions() const
{
    std::vector<std::reference_wrapper<const ReactionNode>> root_reactions;
    for (const auto& rn : _reactionNodes)
    {
        if (rn.successorReactions.size() == 0)
            root_reactions.push_back(std::cref(rn));
    }
    return root_reactions;
}

int PathwayReaction::reactionsCount() const
{
    return _reactions.size();
}

void PathwayReaction::clone(PathwayReaction& reaction)
{
    BaseReaction::clone(reaction);
    _reactionNodes.copy(reaction._reactionNodes);
    // copy reactions ObjArray
    for (int i = 0; i < reaction._reactions.size(); i++)
	{
        auto& other = reaction._reactions[i];
		auto& reactionComponents = _reactions.push();
        reactionComponents.copy(other);
	}
}

BaseReaction* PathwayReaction::neu()
{
    return new PathwayReaction;
}

int PathwayReaction::_addBaseMolecule(int side)
{
    int idx = _allMolecules.add(new Molecule());
    _addedBaseMolecule(idx, side, *_allMolecules[idx]);
    return idx;
}

bool PathwayReaction::aromatize(const AromaticityOptions& options)
{
    bool arom_found = false;
    for (int i = begin(); i < end(); i = next(i))
    {
        arom_found |= MoleculeAromatizer::aromatizeBonds(*(Molecule*)_allMolecules[i], options);
    }
    return arom_found;
}
