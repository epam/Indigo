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

float PathwayLayout::spacing(PathwayLayoutItem* l, PathwayLayoutItem* r, bool /*siblings*/)
{
    return (l->width + r->width)/2;
}

PathwayLayoutItem* PathwayLayout::nextLeft(PathwayLayoutItem* n)
{
    return n->getFirstChild() ? n->getFirstChild() : n->thread;
}

PathwayLayoutItem* PathwayLayout::nextRight(PathwayLayoutItem* n)
{
    return n->getLastChild() ? n->getLastChild() : n->thread;
}

PathwayLayoutItem* PathwayLayout::ancestor(PathwayLayoutItem* vim, PathwayLayoutItem* v, PathwayLayoutItem* a)
{
    return (vim->ancestor->parent == v->parent) ? vim->ancestor : a;
}

void PathwayLayout::make()
{
    buildLayoutTree();
    auto rrs = _reaction.getRootReactions();
    if (rrs.size())
    {
        auto& li = _layoutItems[rrs[0]];
        PathwayLayoutItem* root = &li;
        std::fill(_depths.begin(), _depths.end(), 0.0f);
        _maxDepth = 0;
        firstWalk(root, 0, 1);
        determineDepths();
        secondWalk(root, nullptr, -root->prelim, 0);
    }
}

void PathwayLayout::buildLayoutTree()
{
    for (int i = 0; i < _reaction.getReactionNodeCount(); ++i)
        _layoutItems.emplace_back(_reaction, i);
}

void PathwayLayout::updateDepths(int depth, PathwayLayoutItem* item)
{
    float d = item->height;
    if (_depths.size() <= static_cast<size_t>(depth))
    {
        _depths.resize(3 * depth / 2);
    }
    _depths[depth] = std::max(_depths[depth], d);
    _maxDepth = std::max(_maxDepth, depth);
}

void PathwayLayout::determineDepths()
{
    for (int i = 1; i < _maxDepth; ++i)
    {
        _depths[i] += _depths[i - 1];
    }
}

void PathwayLayout::firstWalk(PathwayLayoutItem* v, int num, int depth)
{
    v->number = num;
    updateDepths(depth, v);
    if (v->children.empty())
    {
        if (PathwayLayoutItem* l = v->prevSibling)
            v->prelim = l->prelim + spacing(l, v, true);
        else
            v->prelim = 0.0;
    }
    else
    {
        PathwayLayoutItem* leftMost = v->getFirstChild();
        PathwayLayoutItem* rightMost = v->getLastChild();
        PathwayLayoutItem* defaultAncestor = leftMost;
        for (PathwayLayoutItem* c = leftMost; c != nullptr; ++num)
        {
            firstWalk(c, num, depth + 1);
            defaultAncestor = apportion(c, defaultAncestor);
            c = c->nextSibling;
        }
        executeShifts(v);
        float midPoint = (leftMost->prelim + rightMost->prelim)/2;
        if (PathwayLayoutItem* l = v->prevSibling)
        {
            v->prelim = l->prelim + spacing(l, v, true);
            v->mod += v->prelim - midPoint;
        }
        else
        {
            v->prelim = midPoint;
        }
    }
}

void PathwayLayout::secondWalk(PathwayLayoutItem* v, PathwayLayoutItem* p, float m, int depth)
{
    v->x = v->prelim + m;
    v->y = _depths[depth];
    m += v->mod;
    for (PathwayLayoutItem* w : v->children)
    {
        secondWalk(w, v, m, depth + 1);
    }
    v->clear();
}

PathwayLayoutItem* PathwayLayout::apportion(PathwayLayoutItem* v, PathwayLayoutItem* a)
{
    if (PathwayLayoutItem* w = v->prevSibling)
    {
        PathwayLayoutItem *vip = v, *vop = v;
        PathwayLayoutItem *vim = w, *vom = v->getFirstChild();
        float sip = vip->mod, sop = vop->mod;
        float sim = vim->mod, som = vom != nullptr ? vom->mod : 0;
        PathwayLayoutItem* nr = nextRight(vim);
        PathwayLayoutItem* nl = nextLeft(vip);
        while (nr != nullptr && nl != nullptr)
        {
            vim = nr;
            vip = nl;
            vom = nextLeft(vom);
            vop = nextRight(vop);
            if (vop)
                vop->ancestor = v;
            float shift = (vim->prelim + sim) - (vip->prelim + sip) + spacing(vim, vip, false);
            if (shift > 0)
            {
                PathwayLayoutItem* from = ancestor(vim, v, a);
                moveSubtree(from, v, shift);
                sip += shift;
                sop += shift;
            }
            sim += vim->mod;
            sip += vip->mod;
            if (vom)
                som += vom->mod;
            if (vop)
                sop += vop->mod;

            nr = nextRight(vim);
            nl = nextLeft(vip);
        }
        if (nr && vop && !nextRight(vop))
        {
            vop->thread = nr;
            vop->mod += sim - sop;
        }
        if (nl && (!vom || !nextLeft(vom)))
        {
            if (vom)
            {
                vom->thread = nl;
                vom->mod += sip - som;
            }
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

void PathwayLayout::executeShifts(PathwayLayoutItem* v)
{
    float shift = 0, change = 0;
    for (auto it = v->children.rbegin(); it != v->children.rend(); ++it)
    {
        PathwayLayoutItem* c = *it;
        c->prelim += shift;
        c->mod += shift;
        change += c->change;
        shift += c->shift + change;
    }
}

