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

#include "layout/pathway_layout.h"

using namespace indigo;

IMPL_ERROR(PathwayLayout, "pathway_layout");

void PathwayLayout::updateDepths(int depth, const PathwayLayoutItem& item)
{
    double d = item.height;
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

std::vector<PathwayLayoutItem> PathwayLayout::getLayoutItems() const
{
    // convert _reactionNodes to std::vector<PathwayLayoutItem>
    return std::vector<PathwayLayoutItem>();
}

// make layout
void PathwayLayout::make()
{
    std::vector<PathwayLayoutItem> nodes;
    auto roots = _reaction.getRootReactions();
    for (auto rootIndex : roots)
    {
        std::fill(_depths.begin(), _depths.end(), 0);
        _maxDepth = 0;
        firstWalk(nodes, rootIndex, 0, 1);
        determineDepths();
        secondWalk(nodes, rootIndex, -nodes[rootIndex].prelim, 0);
    }
}

void PathwayLayout::firstWalk(std::vector<PathwayLayoutItem>& nodes, int nIndex, int num, int depth)
{
    PathwayLayoutItem& n = nodes[nIndex];
    n.number = num;
    updateDepths(depth, n);

    if (n.precursors.empty())
    {
        int lIndex = n.prevSibling;
        if (lIndex == -1)
        {
            n.prelim = 0;
        }
        else
        {
            n.prelim = nodes[lIndex].prelim + spacing(nodes[lIndex], n, true);
        }
    }
    else
    {
        int leftMost = n.getFirstPrecursor();
        int rightMost = n.getLastPrecursor();
        int defaultAncestor = leftMost;

        for (int i = 0, c = leftMost; c != -1; ++i, c = nodes[c].nextSibling)
        {
            firstWalk(nodes, c, i, depth + 1);
            defaultAncestor = apportion(nodes, c, defaultAncestor);
        }

        executeShifts(nodes, nIndex);

        double midpoint = 0.5 * (nodes[leftMost].prelim + nodes[rightMost].prelim);
        int leftIndex = n.prevSibling;
        if (leftIndex != -1)
        {
            n.prelim = nodes[leftIndex].prelim + spacing(nodes[leftIndex], n, true);
            n.mod = n.prelim - midpoint;
        }
        else
        {
            n.prelim = midpoint;
        }
    }
}

// apportion function is used to calculate the preliminary x-coordinate of the children of a node
int PathwayLayout::apportion(std::vector<PathwayLayoutItem>& nodes, int vIndex, int aIndex)
{
    int wIndex = nodes[vIndex].prevSibling;
    if (wIndex != -1)
    {
        int vip = vIndex, vim = wIndex;
        int vop = vIndex, vom = nodes[vip].getFirstPrecursor();
        double sip = nodes[vip].mod, sim = nodes[vim].mod;
        double sop = nodes[vop].mod, som = nodes[vom].mod;

        while (vim != -1 && vip != -1)
        {
            vim = nextRight(nodes, vim);
            vip = nextLeft(nodes, vip);
            vom = nextLeft(nodes, vom);
            vop = nextRight(nodes, vop);
            nodes[vop].ancestor = vIndex;

            double shift = (nodes[vim].prelim + sim) - (nodes[vip].prelim + sip) + spacing(nodes[vim], nodes[vip], false);
            if (shift > 0)
            {
                moveSubtree(nodes, ancestor(nodes, vim, vIndex, aIndex), vIndex, shift);
                sip += shift;
                sop += shift;
            }

            sim += nodes[vim].mod;
            sip += nodes[vip].mod;
            som += nodes[vom].mod;
            sop += nodes[vop].mod;

            vim = nextRight(nodes, vim);
            vip = nextLeft(nodes, vip);
        }

        if (vim != -1 && nextRight(nodes, vop) == -1)
        {
            nodes[vop].thread = vim;
            nodes[vop].mod += sim - sop;
        }
        if (vip != -1 && nextLeft(nodes, vom) == -1)
        {
            nodes[vom].thread = vip;
            nodes[vom].mod += sip - som;
            aIndex = vIndex;
        }
    }
    return aIndex;
}

int PathwayLayout::nextLeft(const std::vector<PathwayLayoutItem>& nodes, int nIndex) const
{
    int child = nodes[nIndex].getFirstPrecursor();
    return child != -1 ? child : nodes[nIndex].thread;
}

int PathwayLayout::nextRight(const std::vector<PathwayLayoutItem>& nodes, int nIndex) const
{
    int child = nodes[nIndex].getLastPrecursor();
    return child != -1 ? child : nodes[nIndex].thread;
}

void PathwayLayout::moveSubtree(std::vector<PathwayLayoutItem>& nodes, int wmIndex, int wpIndex, double shift)
{
    double subtrees = nodes[wpIndex].number - nodes[wmIndex].number;
    nodes[wpIndex].change -= shift / subtrees;
    nodes[wpIndex].shift += shift;
    nodes[wmIndex].change += shift / subtrees;
    nodes[wpIndex].prelim += shift;
    nodes[wpIndex].mod += shift;
}

void PathwayLayout::executeShifts(std::vector<PathwayLayoutItem>& nodes, int nIndex)
{
    double shift = 0, change = 0;
    for (int c = nodes[nIndex].getLastPrecursor(); c != -1; c = nodes[c].prevSibling)
    {
        nodes[c].prelim += shift;
        nodes[c].mod += shift;
        change += nodes[c].change;
        shift += nodes[c].shift + change;
    }
}

int PathwayLayout::ancestor(const std::vector<PathwayLayoutItem>& nodes, int vimIndex, int vIndex, int aIndex) const
{
    return nodes[vimIndex].ancestor == nodes[vIndex].successor ? nodes[vimIndex].ancestor : aIndex;
}

void PathwayLayout::secondWalk(std::vector<PathwayLayoutItem>& nodes, int nIndex, double m, int depth)
{
    nodes[nIndex].x = nodes[nIndex].prelim + m;
    nodes[nIndex].y = _depths[depth];

    for (int c = nodes[nIndex].getFirstPrecursor(); c != -1; c = nodes[c].nextSibling)
    {
        secondWalk(nodes, c, m + nodes[nIndex].mod, depth + 1);
    }

    nodes[nIndex].clear();
}
