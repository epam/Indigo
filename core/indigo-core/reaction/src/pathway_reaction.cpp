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
#include "molecule/meta_commons.h"
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
        if (_reactionNodes[i].successorReactionIndexes.size() == 0)
            root_reactions.push_back(i);
    return root_reactions;
}

void PathwayReaction::_cloneSub(BaseReaction& other)
{
    clear();
    PathwayReaction& other_pwr = other.asPathwayReaction();
    for (int i = 0; i < other_pwr._reactionNodes.size(); ++i)
    {
        auto& other_rnode = other_pwr._reactionNodes[i];
        auto& rn = _reactionNodes.push();
        rn.reactionIdx = other_rnode.reactionIdx;
        rn.precursorReactionIndexes.copy(other_rnode.precursorReactionIndexes);
        for (int j = 0; j < other_rnode.successorReactionIndexes.size(); ++j)
        {
            auto& sr = other_rnode.successorReactionIndexes[j];
            rn.successorReactionIndexes.push(sr);
        }
    }

    for (int i = 0; i < other_pwr._reactions.size(); ++i)
    {
        auto& other_reaction = other_pwr._reactions[i];
        auto& rc = _reactions.push();
        rc.productIndexes.copy(other_reaction.productIndexes);
        rc.reactantIndexes.copy(other_reaction.reactantIndexes);
    }

    for (int i = 0; i < other_pwr._molecules.size(); ++i)
    {
        auto other_molecule = other_pwr._molecules[i];
        addMolecule(*other_molecule);
    }

    _rootReaction.clone(other_pwr._rootReaction);
}

BaseReaction* PathwayReaction::neu()
{
    return new PathwayReaction;
}

void PathwayReaction::clear()
{
    BaseReaction::clear();
    _reactionNodes.clear();
    _reactions.clear();
    _rootReaction.clear();
    _molecules.clear();
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
    for (int i = 0; i < _molecules.size(); ++i)
        arom_found |= _molecules[i]->aromatize(options);
    return arom_found;
}

bool PathwayReaction::dearomatize(const AromaticityOptions& options)
{
    bool arom_found = false;
    for (int i = 0; i < _molecules.size(); ++i)
        arom_found |= _molecules[i]->dearomatize(options);
    return arom_found;
}
