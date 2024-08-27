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

#ifndef __pathway_layout_h__
#define __pathway_layout_h__

#include "reaction/pathway_reaction.h"

namespace indigo
{
    // PathwayLayoutItem is a struct that represents a reaction in the pathway
    struct PathwayLayoutItem
    {
        std::pair<int, std::vector<int>> associatedReactionItems;
        int number = -1;
        double prelim = 0;
        double mod = 0;
        int ancestor = -1;
        int thread = -1;
        double change = 0;
        double shift = 0;
        double width = 0, height = 0;
        double x = 0, y = 0;
        std::vector<int> precursors;
        int successor = -1;
        int nextSibling = -1;
        int prevSibling = -1;

        int getFirstPrecursor() const
        {
            return precursors.empty() ? -1 : precursors.front();
        }

        int getLastPrecursor() const
        {
            return precursors.empty() ? -1 : precursors.back();
        }

        void clear()
        {
            number = -2;
            prelim = mod = shift = change = 0;
            ancestor = thread = -1;
        }
    };

    // PathwayLayout is a class which makes a tree layout of the reactions in O(N) time
    class PathwayLayout
    {
    public:
        static constexpr float MARGIN = 1.f;
        static constexpr float ARROW_HEAD_WIDTH = 2.5f;
        static constexpr float ARROW_TAIL_WIDTH = 0.5f;
        static constexpr float ARROW_WIDTH = ARROW_HEAD_WIDTH + ARROW_TAIL_WIDTH;

        PathwayLayout(PathwayReaction& reaction) : _depths(10, 0.0), _reaction(reaction)
        {
        }
        // make layout
        void make();

    private:

        double spacing(const PathwayLayoutItem& l, const PathwayLayoutItem& r, bool siblings)
        {
            return 0.5 * (l.width + r.width);
        }

        void updateDepths(int depth, const PathwayLayoutItem& item);

        void determineDepths();

        std::vector<PathwayLayoutItem> getLayoutItems() const;

        DECL_ERROR;

    private:
        // firstWalk function calculates the preliminary x-coordinate of each node in the tree
        void firstWalk(std::vector<PathwayLayoutItem>& nodes, int nIndex, int num, int depth);
        // apportion function is used to calculate the preliminary x-coordinate of the children of a node
        int apportion(std::vector<PathwayLayoutItem>& nodes, int vIndex, int aIndex);
        int nextLeft(const std::vector<PathwayLayoutItem>& nodes, int nIndex) const;
        int nextRight(const std::vector<PathwayLayoutItem>& nodes, int nIndex) const;
        void moveSubtree(std::vector<PathwayLayoutItem>& nodes, int wmIndex, int wpIndex, double shift);
        void executeShifts(std::vector<PathwayLayoutItem>& nodes, int nIndex);
        int ancestor(const std::vector<PathwayLayoutItem>& nodes, int vimIndex, int vIndex, int aIndex) const;
        void secondWalk(std::vector<PathwayLayoutItem>& nodes, int nIndex, double m, int depth);

        std::vector<double> _depths;
        int _maxDepth = 0;
        PathwayReaction& _reaction;
    };

} // namespace indigo

#endif
