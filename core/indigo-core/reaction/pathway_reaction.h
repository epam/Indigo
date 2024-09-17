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

#include "base_cpp/array.h"
#include "reaction/reaction.h"
#include <deque>

namespace indigo
{

    class Reaction;

    class DLLEXPORT PathwayReaction : public BaseReaction
    {
    public:
        struct SuccessorReaction
        {
            SuccessorReaction(int reactionIdx, Array<int>& ridxs) : reactionIdx(reactionIdx)
            {
                reactantIndexes.copy(ridxs);
            }
            SuccessorReaction(const SuccessorReaction& other) : reactionIdx(other.reactionIdx)
            {
                reactantIndexes.copy(other.reactantIndexes);
            }
            SuccessorReaction& operator=(const SuccessorReaction& other)
            {
                reactionIdx = other.reactionIdx;
                reactantIndexes.copy(other.reactantIndexes);
                return *this;
            }
            int reactionIdx;
            Array<int> reactantIndexes;
        };

        struct SimpleReaction
        {
            struct Plus
            {
                int metaIndex;
                int componentIndex1;
                int componentIndex2;
            };

            SimpleReaction() : arrowMetaIndex(-1)
            {
            }

            SimpleReaction(const SimpleReaction& other) : arrowMetaIndex(other.arrowMetaIndex)
            {
                reactantIndexes.copy(other.reactantIndexes);
                productIndexes.copy(other.productIndexes);
                pluses.copy(other.pluses);
            }
            Array<int> reactantIndexes;
            Array<int> productIndexes;
            Array<Plus> pluses;
            int arrowMetaIndex;
        };

        struct ReactionNode
        {
            ReactionNode() : reactionIdx(-1), multiTailMetaIndex(-1){};
            ReactionNode(const ReactionNode& other)
            {
                reactionIdx = other.reactionIdx;
                multiTailMetaIndex = other.multiTailMetaIndex;
                for (int i = 0; i < other.successorReactions.size(); ++i)
                    successorReactions.push(other.successorReactions[i]);
                precursorReactionsIndexes.copy(other.precursorReactionsIndexes);
            }
            int reactionIdx;
            // vector of successor reactions indexes and their corresponding reactant indexes
            ObjArray<SuccessorReaction> successorReactions;
            // vector of precursor reactions indexes
            Array<int> precursorReactionsIndexes;
            // utility information
            RedBlackSet<int> successorReactants;
            int multiTailMetaIndex;
        };

        PathwayReaction();
        ~PathwayReaction() override;

        std::vector<int> getRootReactions() const;

        auto& getReactionNode(int node_idx)
        {
            return _reactionNodes[node_idx];
        }

        int getReactionNodeCount() const
        {
            return static_cast<int>(_reactionNodes.size());
        }

        auto& getMolecule(int mol_idx)
        {
            return *_molecules[mol_idx];
        }

        int getMoleculeCount() const
        {
            return static_cast<int>(_molecules.size());
        }

        const auto& getReaction(int reaction_idx)
        {
            return _reactions[reaction_idx];
        }

        int getReactionCount() const
        {
            return static_cast<int>(_reactions.size());
        }

        Reaction& asReaction() override
        {
            _rootReaction.clone(*this);
            return _rootReaction;
        }

        ReactionNode& addReactionNode()
        {
            ReactionNode rn;
            rn.reactionIdx = static_cast<int>(_reactionNodes.size());
            _reactionNodes.push(rn);
            return _reactionNodes[_reactionNodes.size() - 1];
        }

        int addMolecule(BaseMolecule& mol)
        {
            std::unique_ptr<BaseMolecule> mol_copy(mol.neu());
            mol_copy->clone(mol);
            _molecules.add(mol_copy.release());
            return static_cast<int>(_molecules.size() - 1);
        }

        std::pair<int, SimpleReaction&> addReaction()
        {
            SimpleReaction sr;
            _reactions.push(sr);
            return {static_cast<int>(_reactions.size() - 1), _reactions[_reactions.size() - 1]};
        }

        void clone(PathwayReaction&);
        BaseReaction* neu() override;
        bool aromatize(const AromaticityOptions& options) override;
        DECL_ERROR;

    protected:
        int _addBaseMolecule(int side) override;

    private:
        ObjArray<ReactionNode> _reactionNodes;
        PtrArray<BaseMolecule> _molecules;
        ObjArray<SimpleReaction> _reactions;
        Reaction _rootReaction;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif
