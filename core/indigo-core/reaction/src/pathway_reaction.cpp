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

PathwayReaction::PathwayReaction(std::deque<Reaction>& reactions)
{
    for (size_t i = 0; i < reactions.size(); i++)
    {
        for (int j = reactions[i].begin(); j < reactions[i].end(); j = reactions[i].next(j))
        {
            auto molecule = std::make_unique<Molecule>();
            molecule->clone(reactions[i].getBaseMolecule(j));
            int id = _allMolecules.add(molecule.release());
            _addedBaseMolecule(id, reactions[i].getSideType(j), *_allMolecules[id]);
            _reactions.expand(id + 1);
            _reactions[id] = static_cast<int>(i);
        }
    }
}

int PathwayReaction::reactionId(int moleculeId) const
{
    return _reactions.at(moleculeId);
}

int PathwayReaction::reactionsCount() const
{
    return _reactions.size();
}

void PathwayReaction::clone(PathwayReaction& reaction)
{
    BaseReaction::clone(reaction);
    _reactions.copy(reaction._reactions);
}

std::pair<std::vector<std::pair<int, Vec2f>>, std::vector<std::vector<Vec2f>>> PathwayReaction::makeTreePoints()
{
    auto reaction = this;
    std::vector<std::string> inchiKeys(reaction->reactionsCount());
    InchiWrapper inchiWrapper;
    Array<char> inchi, inchiKey;
    for (int i = reaction->begin(); i < reaction->end(); i = reaction->next(i))
    {
        auto& molecule = dynamic_cast<Molecule&>(reaction->getBaseMolecule(i));
        inchiWrapper.saveMoleculeIntoInchi(molecule, inchi);
        InchiWrapper::InChIKey(inchi.ptr(), inchiKey);
        inchiKeys.at(i).assign(inchiKey.ptr(), inchiKey.size());
    }

    int finalProductId;
    std::vector<std::vector<int>> reactantIdsByReactions(reaction->reactionsCount());
    std::unordered_map<std::string, int> productIds;
    for (int i = reaction->begin(); i < reaction->end(); i = reaction->next(i))
    {
        if (BaseReaction::REACTANT == reaction->getSideType(i))
            reactantIdsByReactions.at(reaction->reactionId(i)).push_back(i);
        else if (BaseReaction::PRODUCT == reaction->getSideType(i))
        {
            productIds.emplace(inchiKeys.at(i), i);
            finalProductId = i;
        }
    }

    std::unordered_map<int, Rect2f> sumBoxes;
    std::stack<int> dfsStack;
    dfsStack.push(finalProductId);
    while (!dfsStack.empty())
    {
        auto stackSize = dfsStack.size();
        auto id = dfsStack.top();

        auto productIter = productIds.find(inchiKeys.at(id));
        if (productIter == productIds.cend())
        {
            sumBoxes[id];
            dfsStack.pop();
            continue;
        }

        auto productId = productIter->second;
        for (int reactantId : reactantIdsByReactions[reaction->reactionId(productId)])
            if (!sumBoxes.count(reactantId))
                dfsStack.push(reactantId);
        if (dfsStack.size() > stackSize)
            continue;

        Vec2f rightTop(0, -2 * MARGIN);
        for (int reactantId : reactantIdsByReactions[reaction->reactionId(productId)])
        {
            Rect2f box;
            reaction->getBaseMolecule(reactantId).getBoundingBox(box);
            rightTop.x = std::max(rightTop.x, box.width());
            rightTop.y += std::max(box.height(), sumBoxes[reactantId].height()) + 2 * MARGIN;
        }
        sumBoxes[id] = {{}, rightTop};
        dfsStack.pop();
    }

    std::unordered_map<int, Vec2f> points;
    std::vector<std::vector<Vec2f>> arrows;
    std::queue<int> bfsQueue;
    bfsQueue.push(finalProductId);
    Rect2f box;
    reaction->getBaseMolecule(finalProductId).getBoundingBox(box);
    float offsetX = box.width() / 2 + ARROW_WIDTH + MARGIN;
    while (!bfsQueue.empty())
    {
        float nextOffsetX = 0;
        auto size = bfsQueue.size();
        for (size_t i = 0; i < size; i++)
        {
            auto id = bfsQueue.front();
            bfsQueue.pop();

            auto productIter = productIds.find(inchiKeys.at(id));
            if (productIter == productIds.cend())
                continue;

            auto zero = points[id];
            auto& reactantIds = reactantIdsByReactions[reaction->reactionId(productIter->second)];
            float offsetY = sumBoxes[id].height() / 2;
            Rect2f boxFront, boxBack;
            reaction->getBaseMolecule(reactantIds.front()).getBoundingBox(boxFront);
            reaction->getBaseMolecule(reactantIds.back()).getBoundingBox(boxBack);
            offsetY -= -std::max(boxFront.height(), sumBoxes[reactantIds.front()].height()) / 4;
            offsetY -= std::max(boxBack.height(), sumBoxes[reactantIds.back()].height()) / 4;
            nextOffsetX = std::max(nextOffsetX, sumBoxes[id].width());

            arrows.emplace_back().reserve(1 + reactantIds.size());
            arrows.back().emplace_back(zero.x - offsetX + ARROW_WIDTH, zero.y);
            for (int reactantId : reactantIds)
            {
                Rect2f box;
                reaction->getBaseMolecule(reactantId).getBoundingBox(box);
                float x = offsetX + sumBoxes[id].width() / 2 + MARGIN;
                float y = -offsetY + std::max(box.height(), sumBoxes[reactantId].height()) / 2;
                points[reactantId] = zero - Vec2f(x, y);
                bfsQueue.push(reactantId);
                offsetY -= std::max(box.height(), sumBoxes[reactantId].height()) + 2 * MARGIN;
                arrows.back().emplace_back(zero.x - offsetX, zero.y - y);
            }

            if (arrows.back().size() > 2)
            {
                // Add spines. The first and the last reactant arrows "y" are the highest and the lowest.
                arrows.back().emplace_back(zero.x - offsetX + ARROW_TAIL_WIDTH, arrows.back().back().y);
                arrows.back().emplace_back(zero.x - offsetX + ARROW_TAIL_WIDTH, arrows.back()[1].y);
            }
        }
        offsetX = nextOffsetX / 2 + ARROW_WIDTH + MARGIN;
    }

    return {std::piecewise_construct, std::forward_as_tuple(points.cbegin(), points.cend()), std::forward_as_tuple(arrows)};
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
