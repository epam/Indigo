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
#define __pathway_layout_h__molecules

#include <algorithm>
#include <cmath>
#include <list>
#include <numeric>
#include <vector>

#include "reaction/pathway_reaction.h"

namespace indigo
{
    class PathwayLayout
    {
        DECL_ERROR;

    public:
        PathwayLayout(PathwayReaction& reaction) : _reaction(reaction), _depths(10, 0), _maxDepth(0)
        {
        }

        void make();

    private:
        struct PathwayLayoutItem
        {
            PathwayLayoutItem(PathwayReaction& pwr, int nodeIdx, int reactantIdx = -1)
                : number(-2), prelim(0.0), mod(0.0), shift(0.0), change(0.0), width(0.0), height(0.0), x(0.0), y(0.0), ancestor(this), thread(nullptr),
                  children(), parent(nullptr), nextSibling(nullptr), prevSibling(nullptr), reaction(pwr)
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
                    width = bbox.width();
                    height = bbox.height();
                }
                else
                {
                    for (auto pidx : sr.productIndexes)
                    {
                        auto& mol = reaction.getMolecule(pidx);
                        Rect2f bbox;
                        mol.getBoundingBox(bbox);
                        molecules.push_back(std::make_pair(pidx, bbox));
                        width += bbox.width(); // add some spacing for plus
                        height = std::max(bbox.height(), height);
                    }

                    // add precursors free reactants as children
                    for (int i = 0; i < sr.reactantIndexes.size(); ++i)
                    {
                        // check if it is a final reactant
                        if (!rn.successorReactants.find(i))
                        {
                            auto ridx = sr.reactantIndexes[i];
                            reactants_no_precursors.emplace_back(reaction, nodeIdx, ridx);
                            PathwayLayoutItem* item = &reactants_no_precursors.back();
                            children.push_back(item);
                            item->parent = this;
                            if (children.size() > 1)
                            {
                                children[children.size() - 2]->nextSibling = item;
                                item->prevSibling = children[children.size() - 2];
                            }
                        }
                    }
                }
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
                prelim = mod = shift = change = 0.0;
                ancestor = thread = nullptr;
            }

            void applyLayout(const Vec2f& offset)
            {
                if (molecules.size())
                {
                    float totalWidth = std::accumulate(molecules.begin(), molecules.end(), 0.0f,
                                                       [](float acc, const std::pair<int, Rect2f>& r) { return acc + r.second.width(); });

                    float spacing = molecules.size() > 1 ? (width - totalWidth) / (molecules.size() - 1) : (width - totalWidth) / 2;
                    float currentX = x - width / 2;
                    float currentY = y - height / 2;
                    for (auto& mol_desc : molecules)
                    {
                        auto& mol = reaction.getMolecule(mol_desc.first);
                        Vec2f item_offset(currentX - mol_desc.second.left(), currentY - mol_desc.second.bottom());
                        mol.offsetCoordinates(Vec3f(item_offset.x + offset.x, item_offset.y + offset.y, 0));
                        currentX += mol_desc.second.width() + spacing;
                    }
                }
            }

            // required for the layout algorithm
            float width, height;
            std::vector<PathwayLayoutItem*> children;
            std::list<PathwayLayoutItem> reactants_no_precursors;
            PathwayLayoutItem* parent;
            PathwayLayoutItem* nextSibling;
            PathwayLayoutItem* prevSibling;

            // computed fields
            int number;
            float prelim, mod, shift, change;
            float x, y;
            PathwayLayoutItem* ancestor;
            PathwayLayoutItem* thread;

            // other data
            std::vector<std::pair<int, Rect2f>> molecules;
            PathwayReaction& reaction;
        };

        struct PathwayLayoutRootItem
        {
            PathwayLayoutRootItem(int root) : root_index(root)
            {
            }
            int root_index;
            Vec2f offset;
            std::vector<PathwayLayout::PathwayLayoutItem*> li_items;
        };

        std::vector<PathwayLayoutItem*> traverse(PathwayLayoutItem* root);

        void dumpLayoutItem(const PathwayLayout::PathwayLayoutItem& li)
        {
            std::cout << "Layout Item: " << li.x << " " << li.y << " " << li.width << " " << li.height << std::endl;
        }

        float spacing(PathwayLayoutItem* top, PathwayLayoutItem* bottom, bool siblings)
        {
            return (top->height + bottom->height) + 0.5f;
            // return siblings ? (top->height + bottom->height) : std::max(top->width, bottom->width);  
            // return (top->height + bottom->height) / 2.0f;
        }

        void updateDepths(int depth, PathwayLayoutItem* item);

        void determineDepths();

        void buildLayoutTree();

        void firstWalk(PathwayLayoutItem* n, int num, int depth);

        PathwayLayoutItem* apportion(PathwayLayoutItem* v, PathwayLayoutItem* a);

        PathwayLayoutItem* nextTop(PathwayLayoutItem* n);

        PathwayLayoutItem* nextBottom(PathwayLayoutItem* n);

        void moveSubtree(PathwayLayoutItem* wm, PathwayLayoutItem* wp, float shift);

        void executeShifts(PathwayLayoutItem* n);

        PathwayLayoutItem* ancestor(PathwayLayoutItem* vim, PathwayLayoutItem* v, PathwayLayoutItem* a);

        void secondWalk(PathwayLayoutItem* n, PathwayLayoutItem* p, float m, int depth);

        void applyLayout();

        std::vector<float> _depths;
        int _maxDepth = 0;
        PathwayReaction& _reaction;
        std::vector<PathwayLayoutItem> _layoutItems;
        std::vector<PathwayLayoutRootItem> _layoutRootItems;
    };

}

#endif
