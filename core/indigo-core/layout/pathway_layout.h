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
#include <fstream>
#include <functional>
#include <iostream>
#include <list>
#include <numeric>
#include <vector>

#include "layout/metalayout.h"
#include "layout/molecule_layout.h"
#include "molecule/meta_commons.h"
#include "reaction/pathway_reaction.h"

namespace indigo
{
    // The algorithm is based on the following paper:
    // https://www.researchgate.net/publication/30508504_Improving_Walker's_Algorithm_to_Run_in_Linear_Time
    // The original Walker's algorithm is described here:
    // https://www.researchgate.net/publication/220853707_An_Optimal_Algorithm_for_Computing_the_Trees_of_a_Directed_Graph

    class PathwayLayout
    {
        DECL_ERROR;

    public:
        static constexpr float COMPONENTS_MARGIN = 1.0f;
        static constexpr float TEXT_LINE_HEIGHT = 0.36f;
        static constexpr float ARROW_TAIL_LENGTH = 0.5f;
        static constexpr float VERTICAL_SPACING = 2.5f;
        static constexpr float MULTIPATHWAY_VERTICAL_SPACING = 1.5f;
        static constexpr float ARROW_LENGTH_FACTOR = 7.0f;
        static constexpr float MIN_BOND_MEAN = 0.01f;
        static constexpr float TEXT_ADJUSTMENT = 1.5f;

        static constexpr int MAX_DEPTHS = 10;
        static constexpr int MAX_SYMBOLS = 30;
        static constexpr int MIN_LINES_COUNT = 9;
        static constexpr int ROUNDING_FACTOR = 1000;
        static constexpr auto REACTION_CONDITIONS = "Reaction Conditions";
        static constexpr auto REACTION_NAME = "Name";
        static constexpr auto REACTION_PROPERTY_NA = "Not available";

        PathwayLayout(PathwayReaction& reaction, const LayoutOptions& options)
            : _reaction(reaction), _depths(MAX_DEPTHS, 0), _maxDepth(0), _bond_length(options.DEFAULT_BOND_LENGTH),
              _default_arrow_size((float)options.DEFAULT_BOND_LENGTH * ARROW_LENGTH_FACTOR),
              _reaction_margin_size(options.reactionComponentMarginSize / options.ppi), _preserve_molecule_layout(true),
              _text_line_height((float)options.DEFAULT_BOND_LENGTH * TEXT_LINE_HEIGHT)
        {
        }

        void make();
        void setPreserveMoleculeLayout(bool preserve)
        {
            _preserve_molecule_layout = preserve;
        };

        bool isPreserveMoleculeLayout() const
        {
            return _preserve_molecule_layout;
        };

        void generateTextBlocks(SimpleTextObjectBuilder& tob, const ObjArray<Array<char>>& props, const std::string& style, float& height);

    private:
        struct PathwayLayoutItem
        {
            PathwayLayoutItem(PathwayReaction& pwr, const PathwayLayout& pwl, int nodeIdx, float bondLength, int reactantIdx = -1)
                : levelTree(-1), prelim(0.0), mod(0.0), shift(0.0), change(0.0), width(0.0), height(0.0), ancestor(this), thread(nullptr), children(),
                  parent(nullptr), nextSibling(nullptr), prevSibling(nullptr), reaction(pwr), boundingBox()
            {
                auto& reactionNode = reaction.getReactionNode(nodeIdx);
                reactionIndex = reactionNode.reactionIdx;
                // create as a final reactant child
                if (reactantIdx != -1)
                {
                    auto& mol = reaction.getMolecule(reactantIdx);
                    auto mean = mol.getBondsMeanLength();
                    if (!pwl.isPreserveMoleculeLayout() || mean < MIN_BOND_MEAN)
                    {
                        MoleculeLayout ml(mol, true);
                        ml.bond_length = bondLength;
                        ml.make();
                    }
                    else
                    {
                        Vec2f center;
                        mol.getAtomsCenterPoint(center);
                        mol.scale(center, bondLength / mean);
                    }
                    Rect2f boundingBox;
                    mol.getBoundingBox(boundingBox, Vec2f(bondLength, bondLength));
                    molecules.push_back(std::make_pair(reactantIdx, boundingBox));
                    width = boundingBox.width();
                    height = boundingBox.height();
                }
                else
                {
                    auto& simpleReaction = reaction.getReaction(reactionIndex);
                    for (auto pidx : simpleReaction.productIndexes)
                    {
                        auto& mol = reaction.getMolecule(pidx);
                        auto mean = mol.getBondsMeanLength();
                        if (!pwl.isPreserveMoleculeLayout() || mean < MIN_BOND_MEAN)
                        {
                            MoleculeLayout ml(mol, true);
                            ml.bond_length = bondLength;
                            ml.make();
                        }
                        else
                        {
                            Vec2f center;
                            mol.getAtomsCenterPoint(center);
                            mol.scale(center, bondLength / mean);
                        }

                        Rect2f boundingBox;
                        mol.getBoundingBox(boundingBox);
                        molecules.push_back(std::make_pair(pidx, boundingBox));
                        width += boundingBox.width(); // add some spacing for plus
                        height = std::max(boundingBox.height(), height);
                    }
                }
                width += COMPONENTS_MARGIN * molecules.size();
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
                levelTree = -1;
                prelim = mod = shift = change = 0.0;
                ancestor = thread = nullptr;
            }

            void applyLayout()
            {
                if (!molecules.empty())
                {
                    float totalWidth = std::accumulate(molecules.begin(), molecules.end(), 0.0f,
                                                       [](float acc, const std::pair<int, Rect2f>& r) { return acc + r.second.width(); });
                    float margin = COMPONENTS_MARGIN * (molecules.size() - 1);
                    float blockWidth = totalWidth + margin;

                    float startX = boundingBox.left() + (boundingBox.width() - blockWidth) / 2;
                    float currentX = startX;
                    float currentY = boundingBox.center().y;

                    for (auto& mol_desc : molecules)
                    {
                        auto& mol = reaction.getMolecule(mol_desc.first);
                        Vec2f item_offset(currentX + mol_desc.second.width() / 2 - mol_desc.second.center().x, currentY - mol_desc.second.center().y);
                        mol.offsetCoordinates(Vec3f(item_offset.x, item_offset.y, 0));
                        currentX += mol_desc.second.width() + COMPONENTS_MARGIN;
                    }
                }
            }

            void setXY(float x, float y)
            {
                boundingBox = Rect2f(Vec2f(x - width, y - height / 2), Vec2f(x, y + height / 2));
            }

            // required for the layout algorithm
            float width, height;
            std::vector<PathwayLayoutItem*> children;
            std::list<PathwayLayoutItem> reactantsNoPrecursors;
            PathwayLayoutItem* parent;
            PathwayLayoutItem* nextSibling;
            PathwayLayoutItem* prevSibling;

            // computed fields
            int levelTree;
            float prelim, mod, shift, change;
            PathwayLayoutItem* ancestor;
            PathwayLayoutItem* thread;

            // other data
            std::vector<std::pair<int, Rect2f>> molecules;
            PathwayReaction& reaction;
            Rect2f boundingBox;
            int reactionIndex;
        };

        struct PathwayLayoutRootItem
        {
            PathwayLayoutRootItem(int root) : rootIndex(root)
            {
            }
            int rootIndex;
            Rect2f boundingBox;
            std::vector<PathwayLayout::PathwayLayoutItem*> layoutItems;
        };

        void traverse(PathwayLayoutItem* root, std::function<void(PathwayLayoutItem*, int)> node_processor);

        float spacing(PathwayLayoutItem* top, PathwayLayoutItem* bottom, bool siblings)
        {
            return VERTICAL_SPACING + (top->height + bottom->height) / 2.0f;
        }

        void updateDepths(int depth, PathwayLayoutItem* item);

        void determineDepths();

        void buildLayoutTree();

        void copyTextPropertiesToNode(const PathwayReaction::SimpleReaction& reaction, PathwayReaction::ReactionNode& node);

        void firstWalk(PathwayLayoutItem* node, int num, int depth);

        PathwayLayoutItem* apportion(PathwayLayoutItem* currentNode, PathwayLayoutItem* ancestorNode);

        PathwayLayoutItem* nextUpper(PathwayLayoutItem* node);

        PathwayLayoutItem* nextLower(PathwayLayoutItem* node);

        void moveSubtree(PathwayLayoutItem* parent, PathwayLayoutItem* wp, float shift);

        void executeShifts(PathwayLayoutItem* node);

        PathwayLayoutItem* ancestor(PathwayLayoutItem* node1, PathwayLayoutItem* node2, PathwayLayoutItem* ancestor);

        void secondWalk(PathwayLayoutItem* node, PathwayLayoutItem* parent, float modifier, int depth);

        void applyLayout();
        void addMetaText(PathwayReaction::ReactionNode& node, const Vec2f text_pos_bl, float text_height_limit);
        std::vector<std::string> splitText(const std::string& text, float max_width, std::function<float(char ch)> symbol_width);

        std::vector<float> _depths;
        std::vector<float> _shifts;

        int _maxDepth = 0;
        PathwayReaction& _reaction;
        std::vector<PathwayLayoutItem> _layoutItems;
        std::vector<PathwayLayoutRootItem> _layoutRootItems;

        const float _bond_length;
        const float _text_line_height;
        const float _default_arrow_size;
        const float _reaction_margin_size;
        bool _preserve_molecule_layout;
    };
}

#endif
