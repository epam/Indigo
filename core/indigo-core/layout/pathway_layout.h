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
#include <algorithm>
#include <cmath>
#include <vector>

#include <algorithm>
#include <cmath>
#include <vector>

    class PathwayLayoutItem
    {
    public:
        PathwayLayoutItem(PathwayReaction& reaction, int nodeIdx, int reactantIdx = -1)
            : number(-2), prelim(0.0), mod(0.0), shift(0.0), change(0.0), width(0.0), height(0.0), x(0.0), y(0.0), ancestor(this),
              thread(nullptr), children(), parent(nullptr), nextSibling(nullptr), prevSibling(nullptr)
        {
            auto& rn = reaction.getReactionNode(nodeIdx);
            auto& sr = reaction.getReaction(rn.reactionIdx);
            // create as a final reactant child
            if (reactantIdx != -1)
			{
				auto& mol = reaction.getMolecule(reactantIdx);
				Rect2f bbox;
				mol.getBoundingBox(bbox);
				molecules.push_back(std::make_pair(reactantIdx, bbox));
				width += bbox.width();
				height = std::max(bbox.height(), height);
            }
            else
            {
                for (auto pidx : sr.productIndexes)
                {
                    auto& mol = reaction.getMolecule(pidx);
                    Rect2f bbox;
                    mol.getBoundingBox(bbox);
                    molecules.push_back(std::make_pair(pidx, bbox));
                    width += bbox.width();
                    height = std::max(bbox.height(), height);
                }

                // enumerate only final reactants here
                // how to check if it is a final reactant?
                // rn.precursorReactionsIndexes
                for ( int i = 0; i < sr.reactantIndexes.size(); ++i)
				{
                    // check if it is a final reactant
                    auto ridx = sr.reactantIndexes[i];
                    PathwayLayoutItem* item = new PathwayLayoutItem(reaction, nodeIdx, ridx);
                    children.push_back(item);
                    item->parent = this;
				}
            }
            // fill parent and children
        }

        PathwayLayoutItem* getFirstChild()
        {
            return children.empty() ? nullptr : children.front();
        }

        PathwayLayoutItem* getLastChild()
        {
            return children.empty() ? nullptr : children.back();
        }

        void clear()
        {
            number = -2;
            prelim = 0.0;
            mod = shift = change = 0.0;
            ancestor = thread = nullptr;
        }

        // required for the layout algorithm
        float width, height;
        std::vector<PathwayLayoutItem*> children;
        PathwayLayoutItem* parent;
        PathwayLayoutItem* nextSibling;
        PathwayLayoutItem* prevSibling;

        // computed fields
        int number;
        float prelim, mod, shift, change;
        float x, y;
        PathwayLayoutItem* ancestor;
        PathwayLayoutItem* thread;

        // some data
        std::vector< std::pair<int, Rect2f>> molecules;
    };

    class PathwayLayout
    {
        DECL_ERROR;

    public:
        PathwayLayout(PathwayReaction& reaction) : _reaction(reaction), _depths(10, 0), _maxDepth(0)
        {
        }

        void make();
        void buildLayoutTree();

    private:
        float spacing(PathwayLayoutItem* l, PathwayLayoutItem* r, bool /*siblings*/);
        PathwayLayoutItem* nextLeft(PathwayLayoutItem* n);
        PathwayLayoutItem* nextRight(PathwayLayoutItem* n);
        PathwayLayoutItem* ancestor(PathwayLayoutItem* vim, PathwayLayoutItem* v, PathwayLayoutItem* a);
        void updateDepths(int depth, PathwayLayoutItem* item);
        void determineDepths();
        void firstWalk(PathwayLayoutItem* v, int num, int depth);
        void secondWalk(PathwayLayoutItem* v, PathwayLayoutItem* p, float m, int depth);

        PathwayLayoutItem* apportion(PathwayLayoutItem* v, PathwayLayoutItem* a);
        void moveSubtree(PathwayLayoutItem* wm, PathwayLayoutItem* wp, float shift);
        void executeShifts(PathwayLayoutItem* v);
        PathwayReaction& _reaction;
        std::vector<float> _depths;
        int _maxDepth;
        std::vector<PathwayLayoutItem> _layoutItems;
    };
}

#endif
