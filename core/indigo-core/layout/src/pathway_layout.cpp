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

#include "layout/pathway_layout.h"
#include "molecule/ket_commons.h"

using namespace indigo;

IMPL_ERROR(PathwayLayout, "pathway_layout");

void PathwayLayout::make()
{
    buildLayoutTree();
    auto roots = _reaction.getRootReactions();
    _layoutRootItems.reserve(roots.size());
    float yShift = 0;
    for (auto rootIndexes : roots)
    {
        // collect items for the one pathway
        auto& rootItem = _layoutRootItems.emplace_back(rootIndexes);
        PathwayLayoutItem* root = &_layoutItems[rootIndexes];
        std::vector<std::pair<float, std::vector<int>>> depths;

        traverse(root, [&rootItem, &depths](PathwayLayoutItem* item, int level) {
            int item_index = (int)rootItem.layoutItems.size();
            if ((int)depths.size() > level)
            {
                depths[level].first = std::max(depths[level].first, item->width);
                depths[level].second.push_back(item_index);
            }
            else
                depths.emplace_back(item->width, std::initializer_list<int>{item_index});
            rootItem.layoutItems.push_back(item);
        });

        // update widths
        for (auto& depth : depths)
        {
            for (auto index : depth.second)
                rootItem.layoutItems[index]->width = depth.first;
        }

        // layout the one pathway
        _depths.clear();
        _depths.resize(depths.size(), 0);
        _maxDepth = 0;
        firstWalk(root, 0, 1);
        determineDepths();
        secondWalk(root, nullptr, -root->prelim, 0);

        // calculating bounding box for the one pathway
        auto& layoutItems = rootItem.layoutItems;
        Rect2f pathwayBoundingBox;
        for (size_t i = 0; i < layoutItems.size(); ++i)
        {
            auto& layoutItem = *layoutItems[i];
            if (i)
                pathwayBoundingBox.extend(layoutItem.boundingBox);
            else
                pathwayBoundingBox = layoutItem.boundingBox;
        }

        rootItem.boundingBox = pathwayBoundingBox;
        for (size_t i = 0; i < layoutItems.size(); ++i)
        {
            auto& layoutItem = *layoutItems[i];
            layoutItem.boundingBox.offset(Vec2f(-pathwayBoundingBox.left(), -pathwayBoundingBox.bottom() + yShift));
        }
        yShift += pathwayBoundingBox.height() + MULTIPATHWAY_VERTICAL_SPACING;
    }
    applyLayout();
}

void PathwayLayout::buildLayoutTree()
{
    // create layout items for all reaction nodes
    _layoutItems.reserve(_reaction.getReactionNodeCount());

    for (int i = 0; i < _reaction.getReactionNodeCount(); ++i)
        _layoutItems.emplace_back(_reaction, *this, i, _bond_length);

    // fill layout tree
    for (int i = 0; i < _reaction.getReactionNodeCount(); ++i)
    {
        auto& reactionNode = _reaction.getReactionNode(i);
        auto& currentLayoutItem = _layoutItems[i];
        // add successor reactants to layout items
        for (int j : reactionNode.precursorReactionIndexes)
        {
            auto& precursorLayoutItem = _layoutItems[j];
            auto lastChild = currentLayoutItem.getLastChild();
            if (lastChild != nullptr)
            {
                lastChild->nextSibling = &precursorLayoutItem;
                precursorLayoutItem.prevSibling = lastChild;
            }
            currentLayoutItem.children.push_back(&precursorLayoutItem);
            precursorLayoutItem.parent = &currentLayoutItem;
        }
    }
}

void PathwayLayout::updateDepths(int depth, PathwayLayoutItem* item)
{
    float d = item->width + _default_arrow_size + _reaction_margin_size * 2; // calculate section size
    if ((int)_depths.size() <= depth)
        _depths.resize(3 * depth / 2);
    _depths[depth] = std::max(_depths[depth], d);
    _maxDepth = std::max(_maxDepth, depth);
}

void PathwayLayout::determineDepths()
{
    _shifts = _depths;
    for (int i = 1; i < _maxDepth; ++i)
        _shifts[i] += _shifts[i - 1];
}

void PathwayLayout::firstWalk(PathwayLayoutItem* node, int level, int depth)
{
    node->levelTree = level;
    updateDepths(depth, node);

    if (node->children.empty())
    {
        PathwayLayoutItem* layoutItem = node->prevSibling;
        node->prelim = (layoutItem == nullptr) ? 0 : (layoutItem->prelim + spacing(layoutItem, node, true));
    }
    else
    {
        PathwayLayoutItem* topMost = node->getFirstChild();
        PathwayLayoutItem* bottomMost = node->getLastChild();
        PathwayLayoutItem* defaultAncestor = topMost;
        for (auto child : node->children)
        {
            firstWalk(child, level++, depth + 1);
            defaultAncestor = apportion(child, defaultAncestor);
        }

        executeShifts(node);

        float midpoint = (topMost->prelim + bottomMost->prelim) / 2.0f;
        PathwayLayoutItem* top = node->prevSibling;
        if (top != nullptr)
        {
            node->prelim = top->prelim + spacing(top, node, true);
            node->mod = node->prelim - midpoint;
        }
        else
        {
            node->prelim = midpoint;
        }
    }
}

void PathwayLayout::secondWalk(PathwayLayoutItem* node, PathwayLayoutItem* parent, float modifier, int depth)
{
    node->setXY(-_shifts[depth], -node->prelim - modifier);
    for (PathwayLayoutItem* child = node->getFirstChild(); child != nullptr; child = child->nextSibling)
        secondWalk(child, node, modifier + node->mod, depth + 1);
    node->clear();
}

void PathwayLayout::applyLayout()
{
    // upload coordinates back to the reaction
    _reaction.meta().resetReactionData();
    for (auto& rootItem : _layoutRootItems)
    {
        auto& layoutItems = rootItem.layoutItems;
        for (auto layoutItem : layoutItems)
            layoutItem->applyLayout();

        for (auto layoutItem : layoutItems)
        {
            // connect reactants with products
            if (!layoutItem->children.empty())
            {
                Vec2f head = layoutItem->boundingBox.leftMiddle();
                head.x -= _reaction_margin_size;
                std::vector<Vec2f> tails, arrows;
                arrows.push_back(head);
                for (auto child : layoutItem->children)
                {
                    Vec2f tail = child->boundingBox.rightMiddle();
                    auto tail_it = std::lower_bound(tails.begin(), tails.end(), tail, [](const Vec2f& a, const Vec2f& b) { return a.y > b.y; });
                    tails.insert(tail_it, tail);
                }
                auto rigt_most_x = std::max_element(tails.begin(), tails.end(), [](const Vec2f& a, const Vec2f& b) { return a.x < b.x; })->x;
                rigt_most_x += _reaction_margin_size;

                std::for_each(tails.begin(), tails.end(), [rigt_most_x](Vec2f& v) { v.x = rigt_most_x; });
                arrows.insert(arrows.end(), tails.begin(), tails.end());

                // add spines
                if (tails.size() > 1)
                {
                    Vec2f spineTop(tails.front().x + ARROW_TAIL_LENGTH, tails.front().y);
                    Vec2f spineBottom(tails.back().x + ARROW_TAIL_LENGTH, tails.back().y);
                    arrows.push_back(spineBottom);
                    arrows.push_back(spineTop);
                    _reaction.meta().addMetaObject(new KETReactionMultitailArrow(arrows.begin(), arrows.end()));
                }
                else if (tails.size())
                {
                    _reaction.meta().addMetaObject(new KETReactionArrow(KETReactionArrow::EFilledTriangle, tails.front(), head));
                }
            }
        }
    }
}

PathwayLayout::PathwayLayoutItem* PathwayLayout::apportion(PathwayLayoutItem* currentNode, PathwayLayoutItem* ancestorNode)
{
    PathwayLayoutItem* previousSibling = currentNode->prevSibling;
    if (previousSibling != nullptr)
    {
        PathwayLayoutItem *innerNode = currentNode, *siblingInner = previousSibling, *outerNode = currentNode,
                          *siblingOuter = currentNode->parent->getFirstChild();
        float modInner = innerNode->mod, modSiblingInner = siblingInner->mod, modOuter = outerNode->mod, modSiblingOuter = siblingOuter->mod;
        PathwayLayoutItem* nextLower = nextBottom(siblingInner);
        PathwayLayoutItem* nextUpper = nextTop(innerNode);

        while (nextLower != nullptr && nextUpper != nullptr)
        {
            siblingInner = nextLower;
            innerNode = nextUpper;
            siblingOuter = nextTop(siblingOuter);
            outerNode = nextBottom(outerNode);
            outerNode->ancestor = currentNode;
            float shift = (siblingInner->prelim + modSiblingInner) - (innerNode->prelim + modInner) + spacing(siblingInner, innerNode, false);
            if (shift > 0)
            {
                moveSubtree(ancestor(siblingInner, currentNode, ancestorNode), currentNode, shift);
                modInner += shift;
                modOuter += shift;
            }
            modSiblingInner += siblingInner->mod;
            modInner += innerNode->mod;
            modSiblingOuter += siblingOuter->mod;
            modOuter += outerNode->mod;

            nextLower = nextBottom(siblingInner);
            nextUpper = nextTop(innerNode);
        }
        if (nextLower != nullptr && nextBottom(outerNode) == nullptr)
        {
            outerNode->thread = nextLower;
            outerNode->mod += modSiblingInner - modOuter;
        }
        if (nextUpper != nullptr && nextTop(siblingOuter) == nullptr)
        {
            siblingOuter->thread = nextUpper;
            siblingOuter->mod += modInner - modSiblingOuter;
            ancestorNode = currentNode;
        }
    }
    return ancestorNode;
}

void PathwayLayout::moveSubtree(PathwayLayoutItem* parent, PathwayLayoutItem* child, float shift)
{
    float subtrees = static_cast<float>(child->levelTree - parent->levelTree);
    child->change -= shift / subtrees;
    child->shift += shift;
    parent->change += shift / subtrees;
    child->prelim += shift;
    child->mod += shift;
}

void PathwayLayout::executeShifts(PathwayLayoutItem* node)
{
    float shift = 0, change = 0;
    for (PathwayLayoutItem* child = node->getLastChild(); child != nullptr; child = child->prevSibling)
    {
        child->prelim += shift;
        child->mod += shift;
        change += child->change;
        shift += child->shift + change;
    }
}

PathwayLayout::PathwayLayoutItem* PathwayLayout::nextTop(PathwayLayoutItem* node)
{
    PathwayLayoutItem* child = node->getFirstChild();
    return (child != nullptr) ? child : node->thread;
}

PathwayLayout::PathwayLayoutItem* PathwayLayout::nextBottom(PathwayLayoutItem* node)
{
    PathwayLayoutItem* child = node->getLastChild();
    return (child != nullptr) ? child : node->thread;
}

PathwayLayout::PathwayLayoutItem* PathwayLayout::ancestor(PathwayLayoutItem* node1, PathwayLayoutItem* node2, PathwayLayoutItem* ancestor)
{
    PathwayLayoutItem* p = node2->parent;
    return (node1->ancestor->parent == p) ? node1->ancestor : ancestor;
}

void PathwayLayout::traverse(PathwayLayoutItem* root, std::function<void(PathwayLayoutItem*, int)> node_processor)
{
    std::queue<std::pair<PathwayLayoutItem*, int>> queue;

    if (root != nullptr)
        queue.push({root, 0});

    while (!queue.empty())
    {
        auto nodeInfo = queue.front();
        PathwayLayoutItem* node = nodeInfo.first;
        int level = nodeInfo.second;
        queue.pop();

        node_processor(node, level);

        for (auto child : node->children)
        {
            queue.push({child, level + 1});
        }
    }
}
