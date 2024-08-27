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

#pragma once

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <deque>
#include "base_cpp/array.h"
#include "reaction/base_reaction.h"

namespace indigo
{

    class Reaction;

    class DLLEXPORT PathwayReaction : public BaseReaction
    {
    public:
        struct SuccessorReaction
		{
            SuccessorReaction(int reactionIdx, Array<int>& ridxs) : reactionIdx(reactionIdx){
                reactantIndices.copy(ridxs);
            }
            SuccessorReaction(const SuccessorReaction& other) : reactionIdx(other.reactionIdx){
				reactantIndices.copy(other.reactantIndices);
			}
            SuccessorReaction& operator = (const SuccessorReaction& other)
            {
                reactionIdx = other.reactionIdx;
                reactantIndices.copy(other.reactantIndices);
                return *this;
            }
			int reactionIdx;
			Array<int> reactantIndices;
		};

        struct ReactionNode
        {
            // we don't keep products and reactants here, because they are stored in the Reaction object at reactionIdx
            int reactionIdx;
            // vector of successor reactions indexes and their corresponding reactant indexes
            ObjArray<SuccessorReaction> successorReactions;
            // vector of precursor reactions indexes
            Array<int> precursorReactionsIndexes;
        };

        PathwayReaction();
        PathwayReaction(std::deque<Reaction>& reactions, const Array<ReactionNode>& nodes);
        ~PathwayReaction() override;
        std::vector<int> getRootReactions() const;

        int reactionsCount() const;
        void clone(PathwayReaction&);

        BaseReaction* neu() override;
        bool aromatize(const AromaticityOptions& options) override;

        DECL_ERROR;

    protected:
        int _addBaseMolecule(int side) override;

    private:
        Array<ReactionNode> _reactionNodes;
        ObjArray< RedBlackMap<int, int> > _reactions;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif
