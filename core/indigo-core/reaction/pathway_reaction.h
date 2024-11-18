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
                properties.copy(other.properties);
            }
            Array<int> reactantIndexes;
            Array<int> productIndexes;
            Array<Plus> pluses;
            int arrowMetaIndex;
            RedBlackStringObjMap<Array<char>> properties;
        };

        struct ReactionNode
        {
            ReactionNode() : reactionIdx(-1), multiTailMetaIndex(-1), text_width(0)
            {
            }

            ReactionNode(const ReactionNode& other)
            {
                reactionIdx = other.reactionIdx;
                multiTailMetaIndex = other.multiTailMetaIndex;
                text_width = other.text_width;
                for (int i = 0; i < other.successorReactionIndexes.size(); ++i)
                    successorReactionIndexes.push(other.successorReactionIndexes[i]);
                precursorReactionIndexes.copy(other.precursorReactionIndexes);

                for (int i = 0; i < other.name_text.size(); ++i)
                    name_text.push().copy(other.name_text[i]);
                for (int i = 0; i < other.conditions_text.size(); ++i)
                    conditions_text.push().copy(other.conditions_text[i]);
            }
            int reactionIdx;
            // vector of successor reactions indexes and their corresponding reactant indexes
            Array<int> successorReactionIndexes;
            // vector of precursor reactions indexes
            Array<int> precursorReactionIndexes;
            // utility information
            RedBlackMap<int, int> connectedReactants; // where the precursors' products are connected to
            int multiTailMetaIndex;
            ObjArray<Array<char>> name_text;
            ObjArray<Array<char>> conditions_text;
            float text_width;
        };

        PathwayReaction();
        ~PathwayReaction() override;

        std::unique_ptr<BaseReaction> getBaseReaction(int index) override
        {
            std::unique_ptr<BaseReaction> reaction(new Reaction());
            auto& sr = _reactions[index];
            for (auto pidx : sr.productIndexes)
                reaction->addProductCopy(*_molecules[pidx], 0, 0);
            for (auto ridx : sr.reactantIndexes)
                reaction->addReactantCopy(*_molecules[ridx], 0, 0);
            reaction->properties().copy(sr.properties);
            return reaction;
        }

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

        auto& getReaction(int reaction_idx)
        {
            return _reactions[reaction_idx];
        }

        int getReactionCount() const
        {
            return static_cast<int>(_reactions.size());
        }

        int reactionsCount() override
        {
            return _reactions.size();
        }

        Reaction& asReaction() override
        {
            _rootReaction.clone(*this);
            return _rootReaction;
        }

        void copyToReaction(BaseReaction& reaction)
        {
            reaction.clear();
            reaction.meta().clone(meta());
            // collect roles
            std::map<int, int> mol_roles;
            for (int i = 0; i < _reactions.size(); ++i)
            {
                auto& reac = _reactions[i];
                for (auto mol_idx : reac.productIndexes)
                {
                    auto product_it = mol_roles.find(mol_idx);
                    if (product_it != mol_roles.end())
                    {
                        if (product_it->second == REACTANT)
                            product_it->second = INTERMEDIATE;
                    }
                    else
                        mol_roles.emplace(mol_idx, PRODUCT);
                }

                for (auto mol_idx : reac.reactantIndexes)
                {
                    auto reactant_it = mol_roles.find(mol_idx);
                    if (reactant_it != mol_roles.end())
                    {
                        if (reactant_it->second == PRODUCT)
                            reactant_it->second = INTERMEDIATE;
                    }
                    else
                        mol_roles.emplace(mol_idx, REACTANT);
                }
            }

            // copy molecules into Reaction
            for (auto& kvp : mol_roles)
            {
                switch (kvp.second)
                {
                case PRODUCT:
                    reaction.addProductCopy(*_molecules[kvp.first], 0, 0);
                    break;
                case REACTANT:
                    reaction.addReactantCopy(*_molecules[kvp.first], 0, 0);
                    break;
                case INTERMEDIATE:
                    reaction.addIntermediateCopy(*_molecules[kvp.first], 0, 0);
                    break;
                case CATALYST:
                    reaction.addCatalystCopy(*_molecules[kvp.first], 0, 0);
                    break;
                case UNDEFINED:
                    reaction.addUndefinedCopy(*_molecules[kvp.first], 0, 0);
                    break;
                }
            }
        }

        bool isPathwayReaction() override
        {
            return true;
        }

        PathwayReaction& asPathwayReaction() override
        {
            return *this;
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

        BaseReaction* neu() override;
        bool aromatize(const AromaticityOptions& options) override;
        bool dearomatize(const AromaticityOptions& options) override;

        int reactionBegin() override
        {
            return _reactions.size() ? 0 : 1;
        }

        int reactionEnd() override
        {
            return _reactions.size();
        }

        int reactionNext(int i) override
        {
            return ++i;
        }

        void clear() override;

        DECL_ERROR;

    protected:
        int _addBaseMolecule(int side) override;
        void _cloneSub(BaseReaction& other) override;

    private:
        ObjArray<ReactionNode> _reactionNodes;
        PtrArray<BaseMolecule> _molecules;
        ObjArray<SimpleReaction> _reactions;
        Reaction _rootReaction; // copy of root reaction
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif
