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

#ifndef __pathway_reaction_builder__
#define __pathway_reaction_builder__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <deque>
#include <list>
#include <unordered_map>
#include <utility>
#include <vector>
#include <algorithm>
#include <cmath>
#include <vector>

#include "base_cpp/exception.h"
#include "molecule/ket_commons.h"
#include "reaction/pathway_reaction.h"

namespace indigo
{
    class BaseMolecule;
    class BaseReaction;

    // PathwayReactionBuilder is a class that builds a pathway reaction tree from a list of reactions in O(N) time
    class PathwayReactionBuilder
    {
    public:
        PathwayReactionBuilder();
        ~PathwayReactionBuilder();
        std::unique_ptr<PathwayReaction> buildPathwayReaction(std::deque<Reaction>& reactions);
        DECL_ERROR;

    private:
        struct ReactionInchiDescriptor
        {
            std::unordered_set<std::string> reactants;
            std::vector<std::string> products;
        };

        void buildInchiDescriptors(std::deque<Reaction>& reactions);
        void populatePossibleReactions();
        auto findPossibleSuccessorReactions(int reactionIdx);
        auto getReactionComponents(const PathwayReaction::ReactionNode& rn, Reaction& reaction);
        std::vector<ReactionInchiDescriptor> _reactionInchiDescriptors;
        Array<PathwayReaction::ReactionNode> _reactionNodes;
        std::unordered_map<std::string, std::map<int, int>> _reactantToReactions;
        std::unordered_set<int> _ambigousSuccessorReactions; // reactions that have more than one possible successor
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
