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

#include <algorithm>
#include <cmath>
#include <deque>
#include <list>
#include <unordered_map>
#include <utility>
#include <vector>

#include "base_cpp/exception.h"
#include "molecule/meta_commons.h"
#include "reaction/pathway_reaction.h"

namespace indigo
{
    class BaseMolecule;
    class BaseReaction;
    struct LayoutOptions;

    // PathwayReactionBuilder is a class that builds a pathway reaction tree from a list of reactions
    class PathwayReactionBuilder
    {
    public:
        PathwayReactionBuilder();
        ~PathwayReactionBuilder();
        std::unique_ptr<PathwayReaction> buildPathwayReaction(std::deque<Reaction>& reactions, LayoutOptions& options);
        static void buildRootReaction(PathwayReaction& reaction);
        DECL_ERROR;

    private:
        struct ReactionInchiDescriptor
        {
            std::vector<std::string> products;
            // useful to collect it here to avoid look them up in the reaction object
            std::vector<int> productIndexes;
            std::vector<int> reactantIndexes;
            std::vector<std::pair<std::string, std::string>> properties;
        };

        void buildInchiDescriptors(std::deque<Reaction>& reactions);
        void buildNodes(std::deque<Reaction>& reactions);
        auto findSuccessorReactions(int reactionIdx);
        void buildReactions();

        std::vector<ReactionInchiDescriptor> _reactionInchiDescriptors;
        std::unordered_map<std::string, std::map<int, int>> _reactantToReactions;
        std::unique_ptr<PathwayReaction> _pathwayReaction;
        std::unordered_map<std::pair<int, int>, int, pair_hash> _moleculeMapping;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
