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

        traverse(root, [&rootItem, &depths, this](PathwayLayoutItem* item, int level) {
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
        yShift += pathwayBoundingBox.height() + MULTIPATHWAY_VERTICAL_SPACING;
        for (size_t i = 0; i < layoutItems.size(); ++i)
        {
            auto& layoutItem = *layoutItems[i];
            layoutItem.boundingBox.offset(Vec2f(-pathwayBoundingBox.left(), -pathwayBoundingBox.bottom() - yShift));
        }
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
        auto& simpleReaction = _reaction.getReaction(i);

        copyTextPropertiesToNode(simpleReaction, reactionNode);

        auto& currentLayoutItem = _layoutItems[i];
        // add successor reactants to layout items

        std::unordered_set<int> already_added;
        for (int j = 0; j < simpleReaction.reactantIndexes.size(); ++j)
        {
            // check if it is a final reactant
            auto pcr = reactionNode.connectedReactants.at2(j);
            if (pcr)
            {
                if (!already_added.count(*pcr))
                {
                    // add connected child
                    already_added.insert(*pcr);
                    auto& precursorLayoutItem = _layoutItems[*pcr];
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
            else
            {
                // add free reactant without precursors
                auto ridx = simpleReaction.reactantIndexes[j];
                currentLayoutItem.reactantsNoPrecursors.emplace_back(_reaction, *this, i, _bond_length, ridx);
                PathwayLayoutItem* item = &currentLayoutItem.reactantsNoPrecursors.back();
                currentLayoutItem.children.push_back(item);
                item->parent = &currentLayoutItem;
                if (currentLayoutItem.children.size() > 1)
                {
                    currentLayoutItem.children[currentLayoutItem.children.size() - 2]->nextSibling = item;
                    item->prevSibling = currentLayoutItem.children[currentLayoutItem.children.size() - 2];
                }
            }
        }

        auto totalLines = reactionNode.name_text.size() + reactionNode.conditions_text.size();
        if (totalLines && currentLayoutItem.children.size() > 1)
        {
            if (totalLines > MIN_LINES_COUNT)
                totalLines = MIN_LINES_COUNT;

            auto totalHeight = std::accumulate(currentLayoutItem.children.begin(), currentLayoutItem.children.end(), 0.0f,
                                               [](float summ, const PathwayLayoutItem* item) { return summ + item->height; });
            auto totalSpacing = (currentLayoutItem.children.size() - 1) * VERTICAL_SPACING;
            totalHeight -= currentLayoutItem.children.front()->height / 2;
            totalHeight -= currentLayoutItem.children.back()->height / 2;
            totalHeight /= 2.0f;
            float targetHeight = totalLines * _text_line_height + _reaction_margin_size;
            if (totalHeight + totalSpacing / 2 < targetHeight)
            {
                for (auto& child : currentLayoutItem.children)
                {
                    child->height *= (targetHeight - totalSpacing / 2);
                    child->height /= totalHeight;
                }
            }
        }
    }
}

void PathwayLayout::copyTextPropertiesToNode(const PathwayReaction::SimpleReaction& reaction, PathwayReaction::ReactionNode& node)
{
    node.name_text.clear();
    node.conditions_text.clear();
    auto& props = reaction.properties;
    // split text labels and put them into the reaction node name_text and conditions_text
    auto text_max_width = _default_arrow_size - _reaction_margin_size * 2;
    auto boldWidthLambda = [text_max_width](char ch) { return text_max_width / MAX_SYMBOLS; };   // TODO: implement bold font width
    auto italicWidthLambda = [text_max_width](char ch) { return text_max_width / MAX_SYMBOLS; }; // TODO: implement italic font width
    for (auto prop_idx = props.begin(); prop_idx != props.end(); prop_idx = props.next(prop_idx))
    {
        std::string prop_val = props.value(prop_idx).ptr();
        if (prop_val == REACTION_PROPERTY_NA)
            continue;

        if (props.key(prop_idx) == std::string(REACTION_NAME))
        {
            auto splitted_name = splitText(prop_val, text_max_width, boldWidthLambda);
            for (auto& line : splitted_name)
            {
                node.name_text.push().readString(line.c_str(), true);
                node.text_width = std::max(node.text_width, std::accumulate(line.begin(), line.end(), 0.0f,
                                                                            [&boldWidthLambda](float sum, char ch) { return sum + boldWidthLambda(ch); }));
            }
        }
        else if (props.key(prop_idx) == std::string(REACTION_CONDITIONS))
        {
            auto splitted_conditions = splitText(prop_val, text_max_width, italicWidthLambda);
            for (auto& line : splitted_conditions)
            {
                node.conditions_text.push().readString(line.c_str(), true);
                node.text_width = std::max(node.text_width, std::accumulate(line.begin(), line.end(), 0.0f,
                                                                            [&italicWidthLambda](float sum, char ch) { return sum + italicWidthLambda(ch); }));
            }
        }
    }

    if (node.conditions_text.size())
        node.name_text.push().readString("", true);
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
        node->prelim = layoutItem ? layoutItem->prelim + spacing(layoutItem, node, true) : 0;
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

void insertSorted(std::vector<std::pair<int, std::unique_ptr<MetaObject>>>& objs, std::pair<int, std::unique_ptr<MetaObject>> newPair)
{
    auto it = std::lower_bound(objs.begin(), objs.end(), newPair,
                               [](const std::pair<int, std::unique_ptr<MetaObject>>& element, const std::pair<int, std::unique_ptr<MetaObject>>& value) {
                                   return element.first < value.first;
                               });

    objs.emplace(it, std::move(newPair));
}

void PathwayLayout::applyLayout()
{
    // upload coordinates back to the reaction
    _reaction.meta().resetReactionData();
    std::vector<std::pair<int, std::unique_ptr<MetaObject>>> metaObjects;
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
                float text_height_limit = MIN_LINES_COUNT * _text_line_height;
                auto& node = _reaction.getReactionNode(layoutItem->reactionIndex);
                Vec2f textPos_bl(0, head.y);
                if (tails.size() > 1)
                {
                    Vec2f spineTop(tails.front().x + ARROW_TAIL_LENGTH * _bond_length, tails.front().y);
                    Vec2f spineBottom(tails.back().x + ARROW_TAIL_LENGTH * _bond_length, tails.back().y);
                    textPos_bl.x = std::max(spineBottom.x, spineTop.x) + _reaction_margin_size;
                    arrows.push_back(spineBottom);
                    arrows.push_back(spineTop);
                    insertSorted(metaObjects,
                                 std::make_pair(layoutItem->reactionIndex, std::make_unique<ReactionMultitailArrowObject>(arrows.begin(), arrows.end())));
                    text_height_limit = spineTop.y - head.y - _reaction_margin_size;
                }
                else if (tails.size())
                {
                    insertSorted(metaObjects, std::make_pair(layoutItem->reactionIndex,
                                                             std::make_unique<ReactionArrowObject>(ReactionArrowObject::EFilledTriangle, tails.front(), head)));
                    textPos_bl.x = tails.front().x + (_default_arrow_size - node.text_width - _reaction_margin_size * 2) / 2;
                }

                if (node.name_text.size() || node.conditions_text.size())
                    addMetaText(_reaction.getReactionNode(layoutItem->reactionIndex), textPos_bl, text_height_limit);
            }
        }
    }

    for (auto& arrow : metaObjects)
        _reaction.meta().addMetaObject(arrow.second.release());
}

void PathwayLayout::generateTextBlocks(SimpleTextObjectBuilder& tob, const ObjArray<Array<char>>& props, const std::string& style, float& height)
{
    for (int i = 0; i < props.size(); ++i)
    {
        if (std::round(height * ROUNDING_FACTOR) >= std::round(_text_line_height * ROUNDING_FACTOR))
        {
            height -= _text_line_height;
            SimpleTextLine textLine;
            textLine.text = props[i].ptr();
            if (std::round(height * ROUNDING_FACTOR) < std::round(_text_line_height * ROUNDING_FACTOR) && props.size() - i > 1)
            {
                const std::string ellipsis = "...";
                if (textLine.text.size() >= ellipsis.size() && (textLine.text.size() + ellipsis.size()) > MAX_SYMBOLS)
                    textLine.text.replace(textLine.text.size() - ellipsis.size(), ellipsis.size(), ellipsis);
                else
                    textLine.text.append(ellipsis);
            }
            auto& ts = textLine.text_styles.emplace_back();
            ts.offset = 0;
            ts.size = textLine.text.size();
            ts.styles.push_back(style);
            tob.addLine(textLine);
        }
    }
}

void PathwayLayout::addMetaText(PathwayReaction::ReactionNode& node, const Vec2f text_pos_bl, float text_height_limit)
{
    // add text meta-object
    SimpleTextObjectBuilder tob;
    auto height_limit = text_height_limit;
    generateTextBlocks(tob, node.name_text, KFontBoldStr, height_limit);
    generateTextBlocks(tob, node.conditions_text, KFontItalicStr, height_limit);
    tob.finalize();
    auto text_height = _text_line_height * tob.getLineCounter();
    Vec3f text_pos_tl(text_pos_bl.x, text_pos_bl.y - _text_line_height / 2.0f + text_height + _reaction_margin_size, 0.0f);
    _reaction.meta().addMetaObject(new SimpleTextObject(text_pos_tl, Vec2f(node.text_width, text_height), tob.getJsonString()), true);
}

std::vector<std::string> PathwayLayout::splitText(const std::string& text, float max_width, std::function<float(char ch)> symbol_width)
{
    std::vector<std::string> result;
    size_t start = 0;

    while (start < text.size())
    {
        float width = 0;
        size_t last_break_pos = start;
        size_t current_pos = start;

        while (current_pos < text.size() && width + symbol_width(text[current_pos]) < max_width)
        {
            if (text[current_pos] == '\n')
                break;

            width += symbol_width(text[current_pos]);

            if (std::isspace(text[current_pos]) || std::ispunct(text[current_pos]))
                last_break_pos = current_pos;

            ++current_pos;
        }

        if (current_pos == text.size())
        {
            result.push_back(text.substr(start));
            break;
        }

        if (text[current_pos] == '\n' || text[current_pos] == ' ')
        {
            // if the line ends with a space or a new line
            result.push_back(text.substr(start, current_pos - start));
            start = current_pos + 1;
        }
        else if (std::ispunct(text[current_pos]) || last_break_pos == start)
        {
            // if the line ends with a punctuation
            result.push_back(text.substr(start, current_pos - start));
            start = current_pos;
        }
        else if (last_break_pos > start)
        {
            // last break position is found
            if (std::isspace(text[last_break_pos]))
                result.push_back(text.substr(start, last_break_pos - start));
            else
                result.push_back(text.substr(start, last_break_pos - start + 1));

            start = last_break_pos + 1;
        }

        // skip spaces after break to avoid next line starting with space
        while (start < text.size() && std::isspace(text[start]))
            ++start;
    }

    return result;
}

PathwayLayout::PathwayLayoutItem* PathwayLayout::apportion(PathwayLayoutItem* currentNode, PathwayLayoutItem* ancestorNode)
{
    PathwayLayoutItem* previousSibling = currentNode->prevSibling;
    if (previousSibling)
    {
        PathwayLayoutItem *innerNode = currentNode, *siblingInner = previousSibling, *outerNode = currentNode,
                          *siblingOuter = currentNode->parent->getFirstChild();
        float modInner = innerNode->mod, modSiblingInner = siblingInner->mod, modOuter = outerNode->mod, modSiblingOuter = siblingOuter->mod;
        PathwayLayoutItem* next_lower = nextLower(siblingInner);
        PathwayLayoutItem* next_upper = nextUpper(innerNode);

        while (next_lower && next_upper)
        {
            siblingInner = next_lower;
            innerNode = next_upper;
            siblingOuter = nextUpper(siblingOuter);
            outerNode = nextLower(outerNode);
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

            next_lower = nextLower(siblingInner);
            next_upper = nextUpper(innerNode);
        }
        if (next_lower != nullptr && nextLower(outerNode) == nullptr)
        {
            outerNode->thread = next_lower;
            outerNode->mod += modSiblingInner - modOuter;
        }
        if (next_upper != nullptr && nextUpper(siblingOuter) == nullptr)
        {
            siblingOuter->thread = next_upper;
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

PathwayLayout::PathwayLayoutItem* PathwayLayout::nextUpper(PathwayLayoutItem* node)
{
    PathwayLayoutItem* child = node->getFirstChild();
    return (child != nullptr) ? child : node->thread;
}

PathwayLayout::PathwayLayoutItem* PathwayLayout::nextLower(PathwayLayoutItem* node)
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
            queue.push({child, level + 1});
    }
}
