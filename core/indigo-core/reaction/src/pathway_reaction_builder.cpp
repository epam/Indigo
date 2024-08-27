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

#include <algorithm>
#include <numeric>

#include "molecule/inchi_wrapper.h"
#include "reaction/pathway_reaction_builder.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(PathwayReactionBuilder, "pathway reaction builder");

PathwayReactionBuilder::~PathwayReactionBuilder()
{
}

PathwayReactionBuilder::PathwayReactionBuilder()
{
}

auto PathwayReactionBuilder::findPossibleSuccessorReactions(int reactionIdx)
{
    // find possible precursors
    std::map<int, std::vector<int>> matchedReactions;
    // iterate over products of the reaction
    for (auto& product : _reactionInchiDescriptors[reactionIdx].products)
    {
        // find all reactions that have this product as a reactant (successors)
        auto rtr_it = _reactantToReactions.find(product);
        if (rtr_it != _reactantToReactions.end())
        {
            // if this is a first iteration and matchedReactions is empty,
            // then we just copy the set of reactions where the product is a reactant
            std::map<int, std::vector<int>> reactionReactantIndices;
            std::transform(rtr_it->second.begin(), rtr_it->second.end(), std::inserter(reactionReactantIndices, reactionReactantIndices.end()),
                           [](const auto& pair) { return std::make_pair(pair.first, std::vector<int>{pair.second}); });

            if (matchedReactions.empty())
                matchedReactions = reactionReactantIndices;
            else
            {
                // if matchedReactions is not empty, then we find the intersection of the sets
                // because we need to find only the reactions that have all given products as reactants
                // typically we have only one product, but just it case it is possible to have more than one
                std::map<int, std::vector<int>> intersection;
                std::set_intersection(matchedReactions.begin(), matchedReactions.end(), reactionReactantIndices.begin(), reactionReactantIndices.end(),
                                      std::inserter(intersection, intersection.begin()), [](const auto& a, const auto& b) { return a.first < b.first; });

                for (auto& [key, values] : intersection)
                    values.insert(values.end(), intersection.at(key).begin(), intersection.at(key).end());

                matchedReactions = std::move(intersection);
                // we need at least one reaction where all products are reactants
                if (matchedReactions.empty())
                    break;
            }
        }
    }
    // remove the reaction itself from the set of possible precursors
    matchedReactions.erase(reactionIdx);
    return matchedReactions;
}

void PathwayReactionBuilder::buildInchiDescriptors(std::deque<Reaction>& reactions)
{
    _reactionInchiDescriptors.clear();
    InchiWrapper inchiWrapper;
    Array<char> inchi, inchiKey;
    for (auto& reaction : reactions)
    {
        ReactionInchiDescriptor rd;
        // iterate over molecules in the reaction, calculate inchiKeys for reactants and products
        for (int i = reaction.begin(); i < reaction.end(); i = reaction.next(i))
        {
            inchiWrapper.saveMoleculeIntoInchi(reaction.getBaseMolecule(i).asMolecule(), inchi);
            InchiWrapper::InChIKey(inchi.ptr(), inchiKey);
            switch (reaction.getSideType(i))
            {
            case BaseReaction::REACTANT: {
                std::string inchi_str(inchiKey.ptr(), inchiKey.size());
                rd.reactants.insert(inchi_str);
                auto rtr_it = _reactantToReactions.find(inchi_str);
                if (rtr_it == _reactantToReactions.end())
                {
                    _reactantToReactions.emplace(std::piecewise_construct, std::forward_as_tuple(inchi_str),
                                                 std::forward_as_tuple(std::initializer_list<std::pair<const int, int>>{
                                                     {static_cast<int>(_reactionInchiDescriptors.size()), static_cast<int>(i)}}));
                }
                else
                    rtr_it->second.insert(std::make_pair(static_cast<int>(_reactionInchiDescriptors.size()), static_cast<int>(i)));
            }
            break;
            case BaseReaction::PRODUCT:
                rd.products.emplace_back(inchiKey.ptr(), inchiKey.size());
                break;
            default:
                break;
            }
        }
        _reactionInchiDescriptors.push_back(rd);
    }
}

void PathwayReactionBuilder::populatePossibleReactions()
{
    // iterate over reactionDescriptors and fill possibleSuccessors
    _reactionNodes.resize((int)_reactionInchiDescriptors.size());
    _reactionNodes.zerofill();
    for (auto i = 0; i < _reactionInchiDescriptors.size(); i++)
    {
        _reactionNodes[i].reactionIdx = i;
        auto matchedReactions = findPossibleSuccessorReactions(i);
        for (auto& [j, val] : matchedReactions) // [j, val] - j is the index of the reaction, val is the vector of reactant indexes
        {
            Array<int> val_arr;
            val_arr.copy(val);
            _reactionNodes[i].successorReactions.push(PathwayReaction::SuccessorReaction(j, val_arr));
            _reactionNodes[j].precursorReactionsIndexes.push(i);
            if (_reactionNodes[i].successorReactions.size() > 1 && _ambigousSuccessorReactions.count(i) == 0)
                _ambigousSuccessorReactions.insert(i);
        }
    }
}

auto PathwayReactionBuilder::getReactionComponents(const PathwayReaction::ReactionNode& rn, Reaction& reaction)
{
    std::vector<int> products, reactants;
    std::vector<std::vector<int>> incoming_reactants;

    std::unordered_set<int> used_reactants;
    // look up for incoming reactants from precursor reactions
    for (auto precursor_idx : rn.precursorReactionsIndexes)
    {
        auto& precursor_rd = _reactionNodes[precursor_idx];
        if (precursor_rd.successorReactions.size() == 1)
        {
            auto& reactant = precursor_rd.successorReactions[0].reactantIndices;
            incoming_reactants.push_back(std::vector<int>(reactant.begin(), reactant.end()));
            used_reactants.insert(reactant.begin(), reactant.end());
        }
    }

    for (int i = reaction.begin(); i < reaction.end(); i = reaction.next(i))
    {
        switch (reaction.getSideType(i))
        {
        case BaseReaction::REACTANT:
            if (used_reactants.count(i) == 0)
                reactants.emplace_back(i);
            break;
        case BaseReaction::PRODUCT:
            products.emplace_back(i);
            break;
        }
    }
    // return std::make_pair(products, reactants);
    return 1;
}

std::unique_ptr<PathwayReaction> PathwayReactionBuilder::buildPathwayReaction(std::deque<Reaction>& reactions)
{
    buildInchiDescriptors(reactions);
    populatePossibleReactions();

    if (_ambigousSuccessorReactions.size() > 0)
    {
        int product = 1;
        for (auto ridx : _ambigousSuccessorReactions)
            product *= static_cast<int>(_reactionNodes[ridx].successorReactions.size());
        throw Error("Unable to build the pathway. The provided reaction set has multiple potential connection sites for products, resulting in %d possible "
                    "combination pathways.",
                    product);
    }

    auto pathway_reaction = std::make_unique<PathwayReaction>(reactions, _reactionNodes);
    auto rr = pathway_reaction->getRootReactions();
    std::cout << "Root reactions: " << rr.size() << std::endl;
    // layout the reactions
    return pathway_reaction;
}