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
    auto rrs = _reaction.getRootReactions();
    if (rrs.size())
    {
        auto& li = _layoutItems[rrs[0]];
        PathwayLayoutItem* root = &li;
        _depths.clear();
        _depths.resize(10, 0);
        _maxDepth = 0;

        firstWalk(root, 0, 1);
        determineDepths();
        secondWalk(root, nullptr, -root->prelim, 0);

        for (auto& li : _layoutItems)
        {
            std::cout << "width: " << li.width << " height: " << li.height << std::endl;
            std::cout << "x: " << li.x << " y: " << li.y << std::endl;
        }
    }
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
        std::cout << "reaction: " << i << " precursors:" << rn.precursorReactionsIndexes.size() << std::endl;
        std::cout << "layout item: " << i << " children: " << cur_li.children.size() << std::endl;
        // let's print siblings		
    }
}

void PathwayLayout::updateDepths(int depth, PathwayLayoutItem* item)
{
    float d = item->height;
    if (_depths.size() <= depth)
        _depths.resize(3 * depth / 2);
    _depths[depth] = std::max(_depths[depth], d);
    _maxDepth = std::max(_maxDepth, depth);
}

void PathwayLayout::determineDepths()
{
    for (int i = 1; i < _maxDepth; ++i)
        _depths[i] += _depths[i - 1];
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
    n->y = n->prelim + m;
    n->x = -_depths[depth];

    for (PathwayLayoutItem* c = n->getFirstChild(); c != nullptr; c = c->nextSibling)
    {
        secondWalk(c, n, m + n->mod, depth + 1);
    }

    n->clear();
}

PathwayLayoutItem* PathwayLayout::apportion(PathwayLayoutItem* v, PathwayLayoutItem* a)
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

PathwayLayoutItem* PathwayLayout::nextTop(PathwayLayoutItem* n)
{
    PathwayLayoutItem* c = n->getFirstChild();
    return (c != nullptr) ? c : n->thread;
}

PathwayLayoutItem* PathwayLayout::nextBottom(PathwayLayoutItem* n)
{
    PathwayLayoutItem* c = n->getLastChild();
    return (c != nullptr) ? c : n->thread;
}

PathwayLayoutItem* PathwayLayout::ancestor(PathwayLayoutItem* vim, PathwayLayoutItem* v, PathwayLayoutItem* a)
{
    PathwayLayoutItem* p = v->parent;
    return (vim->ancestor->parent == p) ? vim->ancestor : a;
}
