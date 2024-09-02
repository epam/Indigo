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
    for (auto rri : roots)
    {
        std::cout << "Root node: " << rri << std::endl;
        auto& li_root = _layoutRootItems.emplace_back(rri);
        PathwayLayoutItem* root = &_layoutItems[rri];
        _depths.clear();
        _depths.resize(10, 0);
        _maxDepth = 0;

        firstWalk(root, 0, 1);
        determineDepths();
        secondWalk(root, nullptr, -root->prelim, 0);
        li_root.li_items = traverse(root);
        std::cout << "Number of items: " << li_root.li_items.size() << std::endl;
        // calculating bounding box for the one pathway
        auto& li_items = li_root.li_items;
        Rect2f pw_bbox;
        for (auto i = 0; i < li_items.size(); ++i)
        {
            auto& li = *li_items[i];
            Rect2f item_bbox(Vec2f(li.x - li.width / 2, li.y - li.height / 2), Vec2f(li.x + li.width / 2, li.y + li.height / 2));
            std::cout << "bbox: " << item_bbox.left() << " " << item_bbox.bottom() << " " << item_bbox.right() << " " << item_bbox.top() << std::endl;
            if (i)
                pw_bbox.extend(item_bbox);
            else
                pw_bbox = item_bbox;
        }

        li_root.bbox = pw_bbox;
        if (rri == 5)
        {
            _log_file << "<svg width=\"" << pw_bbox.width() * 100 << "\" height=\"" << pw_bbox.height() * 100 << "\" xmlns=\"http://www.w3.org/2000/svg\">"
                      << std::endl;
            for (auto i = 0; i < li_items.size(); ++i)
            {
                auto& li = *li_items[i];
                Rect2f item_bbox(Vec2f(li.x - li.width / 2, li.y - li.height / 2), Vec2f(li.x + li.width / 2, li.y + li.height / 2));
                _log_file << "<rect width=\"" << item_bbox.width() * 100 << "\" height=\"" << item_bbox.height() * 100 << "\" x=\""
                          << (item_bbox.left() - pw_bbox.left()) * 100 << "\" y=\"" << (pw_bbox.top() - item_bbox.top()) * 100 << "\" fill=\""
                          << "blue"
                          << "\"/>" << std::endl;
            }

            _log_file << "</svg>" << std::endl;
            _log_file.close();
        }

        for (auto i = 0; i < li_items.size(); ++i)
        {
            auto& li = *li_items[i];
            li.x -= pw_bbox.left();
            li.y -= pw_bbox.bottom() + yShift;
        }
        yShift += pw_bbox.height();
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
    float d = item->width + 1.0f;
    if (_depths.size() <= depth)
        _depths.resize(3 * depth / 2);
    _depths[depth] = std::max(_depths[depth], d);
    _maxDepth = std::max(_maxDepth, depth);
    std::cout << "Depth: " << depth << " width: " << _depths[depth] << std::endl;
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
    n->y = n->prelim + m;

    n->x = -(_shifts[depth] + _depths[depth + 1] / 2);

    std::cout << "depth:" << depth << " n->x " << n->x << " width:" << n->width << std::endl;

    for (PathwayLayoutItem* c = n->getFirstChild(); c != nullptr; c = c->nextSibling)
    {
        secondWalk(c, n, m + n->mod, depth + 1);
    }

    n->clear();
}

void PathwayLayout::applyLayout()
{
    // upload coordinates back to the reaction
    for (auto& li_root : _layoutRootItems)
    {
        std::cout << " root:" << li_root.root_index << " count:" << li_root.li_items.size() << std::endl;
        auto& li_items = li_root.li_items;
        for (auto li : li_items)
            li->applyLayout();
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

std::vector<PathwayLayout::PathwayLayoutItem*> PathwayLayout::traverse(PathwayLayoutItem* root)
{
    std::vector<PathwayLayoutItem*> result;
    std::stack<PathwayLayoutItem*> stack;

    if (root != nullptr)
    {
        stack.push(root);
    }

    while (!stack.empty())
    {
        PathwayLayoutItem* node = stack.top();
        stack.pop();
        result.push_back(node);
        std::for_each(node->children.rbegin(), node->children.rend(), [&stack](PathwayLayoutItem* child) { stack.push(child); });
    }
    return result;
}
