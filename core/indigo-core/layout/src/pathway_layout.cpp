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
    for (auto rri : roots)
    {
        auto& li_root = _layoutRootItems.emplace_back(rri);
        PathwayLayoutItem* root = &_layoutItems[rri];
        _depths.clear();
        _depths.resize(10, 0);
        _maxDepth = 0;

        firstWalk(root, 0, 1);
        determineDepths();
        secondWalk(root, nullptr, -root->prelim, 0);

        traverse(root, [&li_root](PathwayLayoutItem* item) { li_root.li_items.push_back(item); });

        // calculating bounding box for the one pathway
        auto& li_items = li_root.li_items;
        Rect2f pw_bbox;
        for (size_t i = 0; i < li_items.size(); ++i)
        {
            auto& li = *li_items[i];
            if (i)
                pw_bbox.extend(li.bbox);
            else
                pw_bbox = li.bbox;
        }

        li_root.bbox = pw_bbox;
        for (size_t i = 0; i < li_items.size(); ++i)
        {
            auto& li = *li_items[i];
            li.bbox.offset(Vec2f(-pw_bbox.left(), -pw_bbox.bottom() + yShift));
        }
        yShift += pw_bbox.height() + MULTIPATHWAY_VERTICAL_SPACING;
    }
    applyLayout();
}

void PathwayLayout::buildLayoutTree()
{
    // create layout items for all reaction nodes
    _layoutItems.reserve(_reaction.getReactionNodeCount());

    for (int i = 0; i < _reaction.getReactionNodeCount(); ++i)
        _layoutItems.emplace_back(_reaction, i);

    // fill layout tree
    for (int i = 0; i < _reaction.getReactionNodeCount(); ++i)
    {
        auto& rn = _reaction.getReactionNode(i);

        auto& cur_li = _layoutItems[i];
        // add successor reactants to layout items
        for (int j : rn.precursorReactionsIndexes)
        {
            auto& prec_li = _layoutItems[j];
            auto last_child = cur_li.getLastChild();
            if (last_child != nullptr)
            {
                last_child->nextSibling = &prec_li;
                prec_li.prevSibling = last_child;
            }
            cur_li.children.push_back(&prec_li);
            prec_li.parent = &cur_li;
        }
    }
}

void PathwayLayout::updateDepths(int depth, PathwayLayoutItem* item)
{
    float d = item->width + HORIZONTAL_SPACING;
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

void PathwayLayout::firstWalk(PathwayLayoutItem* n, int num, int depth)
{
    n->number = num;
    updateDepths(depth, n);

    if (n->children.empty())
    {
        PathwayLayoutItem* l = n->prevSibling;
        n->prelim = (l == nullptr) ? 0 : (l->prelim + spacing(l, n, true));
    }
    else
    {
        PathwayLayoutItem* topMost = n->getFirstChild();
        PathwayLayoutItem* bottomMost = n->getLastChild();
        PathwayLayoutItem* defaultAncestor = topMost;
        for (auto c : n->children)
        {
            firstWalk(c, num++, depth + 1);
            defaultAncestor = apportion(c, defaultAncestor);
        }

        executeShifts(n);

        float midpoint = (topMost->prelim + bottomMost->prelim) / 2.0f;
        PathwayLayoutItem* top = n->prevSibling;
        if (top != nullptr)
        {
            n->prelim = top->prelim + spacing(top, n, true);
            n->mod = n->prelim - midpoint;
        }
        else
        {
            n->prelim = midpoint;
        }
    }
}

void PathwayLayout::secondWalk(PathwayLayoutItem* n, PathwayLayoutItem* p, float m, int depth)
{
    n->setXY(-_shifts[depth], n->prelim + m);
    for (PathwayLayoutItem* c = n->getFirstChild(); c != nullptr; c = c->nextSibling)
        secondWalk(c, n, m + n->mod, depth + 1);
    n->clear();
}

void PathwayLayout::applyLayout()
{
    // upload coordinates back to the reaction
    _reaction.meta().resetReactionData();
    for (auto& li_root : _layoutRootItems)
    {
        auto& li_items = li_root.li_items;
        for (auto li : li_items)
            li->applyLayout();

        for (auto li : li_items)
        {
            // connect reactants with products
            if (!li->children.empty())
            {
                Vec2f head = li->bbox.leftMiddle();
                head.x -= MARGIN;
                std::vector<Vec2f> tails, arrows;
                arrows.push_back(head);
                for (auto c : li->children)
                {
                    Vec2f tail = c->bbox.rightMiddle();
                    auto tail_it = std::lower_bound(tails.begin(), tails.end(), tail, [](const Vec2f& a, const Vec2f& b) { return a.y > b.y; });
                    tails.insert(tail_it, tail);
                }
                auto rigt_most_x = std::max_element(tails.begin(), tails.end(), [](const Vec2f& a, const Vec2f& b) { return a.x < b.x; })->x;
                rigt_most_x += MARGIN;

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
                    _reaction.meta().addMetaObject(new KETReactionArrow(KETReactionArrow::EOpenAngle, tails.front(), head));
                }
            }
        }
    }
}

PathwayLayout::PathwayLayoutItem* PathwayLayout::apportion(PathwayLayoutItem* v, PathwayLayoutItem* a)
{
    PathwayLayoutItem* w = v->prevSibling;
    if (w != nullptr)
    {
        PathwayLayoutItem *vip = v, *vim = w, *vop = v, *vom = v->parent->getFirstChild();
        float sip = vip->mod, sim = vim->mod, sop = vop->mod, som = vom->mod;
        PathwayLayoutItem* nr = nextBottom(vim);
        PathwayLayoutItem* nl = nextTop(vip);

        while (nr != nullptr && nl != nullptr)
        {
            vim = nr;
            vip = nl;
            vom = nextTop(vom);
            vop = nextBottom(vop);
            vop->ancestor = v;
            float shift = (vim->prelim + sim) - (vip->prelim + sip) + spacing(vim, vip, false);
            if (shift > 0)
            {
                moveSubtree(ancestor(vim, v, a), v, shift);
                sip += shift;
                sop += shift;
            }
            sim += vim->mod;
            sip += vip->mod;
            som += vom->mod;
            sop += vop->mod;

            nr = nextBottom(vim);
            nl = nextTop(vip);
        }
        if (nr != nullptr && nextBottom(vop) == nullptr)
        {
            vop->thread = nr;
            vop->mod += sim - sop;
        }
        if (nl != nullptr && nextTop(vom) == nullptr)
        {
            vom->thread = nl;
            vom->mod += sip - som;
            a = v;
        }
    }
    return a;
}

void PathwayLayout::moveSubtree(PathwayLayoutItem* wm, PathwayLayoutItem* wp, float shift)
{
    float subtrees = static_cast<float>(wp->number - wm->number);
    wp->change -= shift / subtrees;
    wp->shift += shift;
    wm->change += shift / subtrees;
    wp->prelim += shift;
    wp->mod += shift;
}

void PathwayLayout::executeShifts(PathwayLayoutItem* n)
{
    float shift = 0, change = 0;
    for (PathwayLayoutItem* c = n->getLastChild(); c != nullptr; c = c->prevSibling)
    {
        c->prelim += shift;
        c->mod += shift;
        change += c->change;
        shift += c->shift + change;
    }
}

PathwayLayout::PathwayLayoutItem* PathwayLayout::nextTop(PathwayLayoutItem* n)
{
    PathwayLayoutItem* c = n->getFirstChild();
    return (c != nullptr) ? c : n->thread;
}

PathwayLayout::PathwayLayoutItem* PathwayLayout::nextBottom(PathwayLayoutItem* n)
{
    PathwayLayoutItem* c = n->getLastChild();
    return (c != nullptr) ? c : n->thread;
}

PathwayLayout::PathwayLayoutItem* PathwayLayout::ancestor(PathwayLayoutItem* vim, PathwayLayoutItem* v, PathwayLayoutItem* a)
{
    PathwayLayoutItem* p = v->parent;
    return (vim->ancestor->parent == p) ? vim->ancestor : a;
}

void PathwayLayout::traverse(PathwayLayoutItem* root, std::function<void(PathwayLayoutItem*)> node_processor)
{
    std::stack<PathwayLayoutItem*> stack;

    if (root != nullptr)
        stack.push(root);

    while (!stack.empty())
    {
        PathwayLayoutItem* node = stack.top();
        // call lambda here
        stack.pop();
        node_processor(node);
        std::for_each(node->children.rbegin(), node->children.rend(), [&stack](PathwayLayoutItem* child) { stack.push(child); });
    }
    // return result;
}
