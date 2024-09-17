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

std::vector<int> PathwayReaction::getRootReactions() const
{
    std::vector<int> root_reactions;
    for (int i = 0; i < _reactionNodes.size(); ++i)
        if (_reactionNodes[i].successorReactions.size() == 0)
            root_reactions.push_back(i);
    return root_reactions;
}

void PathwayReaction::clone(PathwayReaction& reaction)
{
    BaseReaction::clone(reaction);
    for (int i = 0; i < _reactionNodes.size(); ++i)
    {
        auto& other = _reactionNodes[i];
        auto& rn = reaction._reactionNodes.push();
        rn.reactionIdx = other.reactionIdx;
        rn.precursorReactionsIndexes.copy(other.precursorReactionsIndexes);
        for (int j = 0; j < other.successorReactions.size(); ++j)
        {
            auto& sr = other.successorReactions[j];
            rn.successorReactions.push(sr);
        }
    }

    for (int i = 0; i < reaction._reactions.size(); ++i)
    {
        auto& other = reaction._reactions[i];
        auto& rc = _reactions.push();
        rc.productIndexes.copy(other.productIndexes);
        rc.reactantIndexes.copy(other.reactantIndexes);
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