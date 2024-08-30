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
#include "layout/pathway_layout.h"
#include "reaction/reaction.h"

using namespace indigo;

IMPL_ERROR(PathwayReactionBuilder, "pathway reaction builder");

PathwayReactionBuilder::~PathwayReactionBuilder()
{
}

PathwayReactionBuilder::PathwayReactionBuilder() : _pathwayReaction(std::make_unique<PathwayReaction>())
{
}

auto PathwayReactionBuilder::findSuccessorReactions(int reactionIdx)
{
    // key - reaction index, value - vector of reactant indexes
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

void PathwayReactionBuilder::buildReactions(std::deque<Reaction>& reactions)
{
    for (int i = 0; i < _reactionInchiDescriptors.size(); ++i)
    {
        auto& rid = _reactionInchiDescriptors[i];
        auto [sri, sr] = _pathwayReaction->addReaction();
        for (auto rci : rid.reactantIndexes)
        {
            auto mol_idx = _moleculeMapping.at(std::make_pair(i, rci));
            sr.reactantIndexes.push(mol_idx);
        }

        for (auto pid : rid.productIndexes)
        {
            auto mol_idx = _moleculeMapping.at(std::make_pair(i, pid));
            sr.productIndexes.push(mol_idx);
        }
    }
}

void PathwayReactionBuilder::buildInchiDescriptors(std::deque<Reaction>& reactions)
{
    _reactionInchiDescriptors.clear();
    InchiWrapper inchiWrapper;
    Array<char> inchi, inchiKey;
    for (int i = 0; i < reactions.size(); ++i)
    {
        _pathwayReaction->addReactionNode();
        auto& reaction = reactions[i];
        ReactionInchiDescriptor& rd = _reactionInchiDescriptors.emplace_back();
        // iterate over molecules in the reaction, calculate inchiKeys for reactants and products
        for (int j = reaction.begin(); j < reaction.end(); j = reaction.next(j))
        {
            inchiWrapper.saveMoleculeIntoInchi(reaction.getBaseMolecule(j).asMolecule(), inchi);
            InchiWrapper::InChIKey(inchi.ptr(), inchiKey);
            switch (reaction.getSideType(j))
            {
            case BaseReaction::REACTANT: {
                rd.reactantIndexes.push_back(static_cast<int>(j));
                // copy reactant molecule to the pathway reaction, add mapping
                _moleculeMapping.emplace(std::piecewise_construct, std::forward_as_tuple(i, j),
                                         std::forward_as_tuple(_pathwayReaction->addMolecule(reaction.getBaseMolecule(j).asMolecule())));
                std::string inchi_str(inchiKey.ptr(), inchiKey.size());
                rd.reactants.insert(inchi_str);
                auto rtr_it = _reactantToReactions.find(inchi_str);
                if (rtr_it == _reactantToReactions.end())
                {
                    _reactantToReactions.emplace(std::piecewise_construct, std::forward_as_tuple(inchi_str),
                                                 std::forward_as_tuple(std::initializer_list<std::pair<const int, int>>{
                                                     {i, static_cast<int>(j)}}));
                }
                else
                    rtr_it->second.insert(std::make_pair(static_cast<int>(i), static_cast<int>(j)));
            }
            break;
            case BaseReaction::PRODUCT:
                rd.products.emplace_back(inchiKey.ptr(), inchiKey.size());
                rd.productIndexes.push_back(static_cast<int>(j));
                break;
            default:
                break;
            }
        }
    }
}

void PathwayReactionBuilder::buildNodes(std::deque<Reaction>& reactions)
{
    // find all reactants of a reaction that match the products of the current reaction
    for (auto i = 0; i < _reactionInchiDescriptors.size(); i++)
    {
        auto matching_successor = findSuccessorReactions(i);
        _reactionInchiDescriptors[i].successor = matching_successor;
        auto& productIndexes = _reactionInchiDescriptors[i].productIndexes;
        for (auto j = 0; j < productIndexes.size(); ++j)
        {
            // copy reactant molecule to the pathway reaction, add mapping
            auto pidx = productIndexes[j];
            int mol_idx = matching_successor.empty()
                              ? _pathwayReaction->addMolecule(reactions[i].getBaseMolecule(productIndexes[j]).asMolecule())
                              : _moleculeMapping.at(std::make_pair(matching_successor.begin()->first, matching_successor.begin()->second[j]));
            _moleculeMapping.emplace(std::piecewise_construct, std::forward_as_tuple(i, pidx), std::forward_as_tuple(mol_idx));
        }

        for (auto& [j, val] : matching_successor) // [j, val] - j is the index of the reaction, val is the vector of reactant indexes
        {
            Array<int> val_arr;
            val_arr.copy(val);
            auto& rnodes = _pathwayReaction->getReactionNodes();
            auto& rn = rnodes[i];
            if (rn.successorReactions.size() == 0)
            {
                rn.successorReactions.push(PathwayReaction::SuccessorReaction(j, val_arr));
                rnodes[j].precursorReactionsIndexes.push(i);
            }
            else
            {
                // only one successor reaction is allowed. skip the reaction if there are more than one.
            }
        }
    }
}

std::unique_ptr<PathwayReaction> PathwayReactionBuilder::buildPathwayReaction(std::deque<Reaction>& reactions)
{
    buildInchiDescriptors(reactions);
    buildNodes(reactions);
    buildReactions(reactions);
    auto& rr = _pathwayReaction->getRootReactions();
    PathwayLayout pl(*_pathwayReaction);
    pl.make();
    std::cout << "root nodes: " << rr.size() << std::endl;
    return std::move(_pathwayReaction);
}