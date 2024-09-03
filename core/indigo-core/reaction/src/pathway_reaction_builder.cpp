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

#include "layout/pathway_layout.h"
#include "molecule/inchi_wrapper.h"
#include "reaction/pathway_reaction_builder.h"
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
            std::map<int, std::vector<int>> reactionReactantIndexes;
            std::transform(rtr_it->second.begin(), rtr_it->second.end(), std::inserter(reactionReactantIndexes, reactionReactantIndexes.end()),
                           [](const auto& pair) { return std::make_pair(pair.first, std::vector<int>{pair.second}); });

            if (matchedReactions.empty())
                matchedReactions = reactionReactantIndexes;
            else
            {
                // if matchedReactions is not empty, then we find the intersection of the sets
                // because we need to find only the reactions that have all given products as reactants
                // typically we have only one product, but just it case it is possible to have more than one
                std::map<int, std::vector<int>> intersection;
                std::set_intersection(matchedReactions.begin(), matchedReactions.end(), reactionReactantIndexes.begin(), reactionReactantIndexes.end(),
                                      std::inserter(intersection, intersection.begin()), [](const auto& a, const auto& b) { return a.first < b.first; });

                for (auto& [key, values] : intersection)
                    values.insert(values.end(), reactionReactantIndexes.at(key).begin(), reactionReactantIndexes.at(key).end());

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
    for (size_t i = 0; i < _reactionInchiDescriptors.size(); ++i)
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
        // add empty reaction nodes meanwhile calculating inchiKeys for reactants and products
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
                _moleculeMapping.emplace(std::piecewise_construct, std::forward_as_tuple(i, j),
                                         std::forward_as_tuple(_pathwayReaction->addMolecule(reaction.getBaseMolecule(j).asMolecule())));
                std::string inchi_str(inchiKey.ptr(), inchiKey.size());
                auto rtr_it = _reactantToReactions.find(inchi_str);
                auto ridx = static_cast<int>(rd.reactantIndexes.size() - 1);
                if (rtr_it == _reactantToReactions.end())
                    _reactantToReactions.emplace(std::piecewise_construct, std::forward_as_tuple(inchi_str),
                                                 std::forward_as_tuple(std::initializer_list<std::pair<const int, int>>{{i, ridx}}));
                else
                    rtr_it->second.insert(std::make_pair(static_cast<int>(i), ridx));
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
        // looking for the reaction continuation
        auto matching_successor = findSuccessorReactions(i);
        // iterate over products of the reaction
        auto& productIndexes = _reactionInchiDescriptors[i].productIndexes;

        auto& rn = _pathwayReaction->getReactionNode(i);
        for (auto m_it = matching_successor.begin();
             m_it != matching_successor.end();) // [j, val] - j is the index of the matched sucessor reaction, val is the vector of reactant indexes
        {
            auto j = m_it->first;
            auto& val = m_it->second;
            Array<int> val_arr;
            val_arr.copy(val);
            if (rn.successorReactions.size() == 0)
            {
                auto& rnj = _pathwayReaction->getReactionNode(j);
                // check if the reactant is already in use as a successor
                bool found = false;
                for (auto ridx : val)
                {
                    found = rnj.successorReactants.find(ridx);
                    if (found)
                        break;
                }
                if (!found)
                {
                    m_it++;
                    rn.successorReactions.push(j, val_arr);
                    rnj.precursorReactionsIndexes.push(i);
                    for (auto ridx : val)
                        rnj.successorReactants.insert(ridx);
                }
                else
                {
                    m_it = matching_successor.erase(m_it);
                    // it's impossible that a reactant has multiple precursors.
                    // we can handle this case if needed.
                }
            }
            else
            {
                m_it = matching_successor.erase(m_it);
                // only one successor reaction is allowed for a reaction.
                // we can handle this case if needed.
            }
        }

        for (auto j = 0; j < productIndexes.size(); ++j)
        {
            auto pidx = productIndexes[j];
            int mol_idx = matching_successor.empty()
                              ? _pathwayReaction->addMolecule(reactions[i].getBaseMolecule(pidx).asMolecule())
                              : _moleculeMapping.at(std::make_pair(matching_successor.begin()->first, matching_successor.begin()->second[j]));
            _moleculeMapping.emplace(std::piecewise_construct, std::forward_as_tuple(i, pidx), std::forward_as_tuple(mol_idx));
        }
    }
}

std::unique_ptr<PathwayReaction> PathwayReactionBuilder::buildPathwayReaction(std::deque<Reaction>& reactions)
{
    buildInchiDescriptors(reactions);
    buildNodes(reactions);
    buildReactions(reactions);
    const auto& rr = _pathwayReaction->getRootReactions();
    PathwayLayout pl(*_pathwayReaction);
    pl.make();
    if (rr.size())
    {
        auto& first_root = _pathwayReaction->getReaction(rr.back());
        for (auto& idx : first_root.reactantIndexes)
        {
            auto& mol = _pathwayReaction->getMolecule(idx);
            _pathwayReaction->addReactantCopy(mol, 0, 0);
        }

        for (auto& idx : first_root.productIndexes)
        {
            auto& mol = _pathwayReaction->getMolecule(idx);
            _pathwayReaction->addProductCopy(mol, 0, 0);
        }
    }

    return std::move(_pathwayReaction);
}