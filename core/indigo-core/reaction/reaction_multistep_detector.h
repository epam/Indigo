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

#ifndef __reaction_multistep_detector__
#define __reaction_multistep_detector__

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

#include <list>
#include <utility>
#include <vector>

#include "base_cpp/exception.h"
#include "layout/metalayout.h"
#include "molecule/meta_commons.h"

namespace indigo
{

    class BaseMolecule;
    class BaseReaction;
    class PathwayReaction;

    class ReactionMultistepDetector
    {
    public:
        using MOL_DISTANCES = std::vector<std::pair<size_t, float>>;
        using MOL_DISTANCES_MAP = std::unordered_map<size_t, float>;

        struct MOL_DISTANCES_DESC
        {
            MOL_DISTANCES sorted_distances;
            MOL_DISTANCES_MAP distances_map;
        };

        struct MERGE_CANDIDATE
        {
            MERGE_CANDIDATE(size_t idx1, size_t idx2, float distance) : undef_idx(idx1), comp_idx(idx2), distance(distance)
            {
            }
            size_t undef_idx;
            size_t comp_idx;
            float distance;
        };

        enum class ReactionType
        {
            ESimpleReaction,
            EMutistepReaction,
            EPathwayReaction
        };

        enum class ZoneType
        {
            EPlus,
            EArrow,
            EPathWay
        };

        enum class PlusSectionCode : int
        {
            ELeft = 0,
            ERight,
            ETop,
            EBottom,
        };

        enum class ArrowSectionCode : int
        {
            ELeft = 0,
            ERight,
            ETop,
            EBottom,
        };

        enum class PathwaySectionCode : int
        {
            ERight,
            ETop,
            EBottom,
        };

        struct SPECIAL_ZONE_DESC
        {
            ZoneType zone_type;
            std::vector<std::vector<Vec2f>> zone_sections;
            std::vector<Vec2f> origin_coordinates;
        };

        struct COMPONENT_DESC
        {
            COMPONENT_DESC(std::unique_ptr<BaseMolecule> mol, std::vector<Vec2f> poly, int idx) : mol(std::move(mol)), hull(poly), idx(idx), mapped_idx(-1)
            {
            }
            std::unique_ptr<BaseMolecule> mol;
            std::vector<Vec2f> hull;
            int idx;
            int mapped_idx;
        };

        ReactionMultistepDetector(BaseMolecule& mol, const LayoutOptions& options);
        ~ReactionMultistepDetector();
        ReactionType detectReaction();
        void constructMultipleArrowReaction(BaseReaction& rxn);
        void constructSimpleArrowReaction(BaseReaction& rxn);
        int getMoleculeSide(const ReactionArrowObject& arrow, BaseMolecule& mol, std::array<int, KProductArea + 1>& sides);

        void constructPathwayReaction(PathwayReaction& rxn);
        void detectPathwayMetadata(PathwayReaction& rxn);
        void collectMetadata(int reaction_idx, PathwayReaction& rxn, const Rect2f& bbox);
        void collectProperties(PathwayReaction::SimpleReaction& sr, const SimpleTextObject& text_obj);

        typedef std::pair<float, int> FLOAT_INT_PAIR;
        typedef std::vector<FLOAT_INT_PAIR> FLOAT_INT_PAIRS;
        const Vec2f PLUS_BBOX_SHIFT = {1.0f, 1.0f};
        const Vec2f ARROW_BBOX_SHIFT = {0.0f, 0.9f};
        const float PLUS_DETECTION_DISTANCE = LayoutOptions::DEFAULT_BOND_LENGTH * 2.0f;
        const float ARROW_DETECTION_DISTANCE = LayoutOptions::DEFAULT_BOND_LENGTH * 2.0f;

        DECL_ERROR;

    private:
        void createSummBlocks();
        // collect molecules' distances
        void collectSortedDistances();
        void createSpecialZones();
        void addPlusZones(const Vec2f& pos);
        void addArrowZones(const Vec2f& tail, const Vec2f& head);
        std::vector<Vec2f> getArrowZone(const Vec2f& tail, const Vec2f& head);

        void addPathwayZones(const Vec2f& head, const Vec2f& sp_beg, const Vec2f& sp_end, const std::vector<Vec2f>& tails);
        std::map<int, std::unordered_set<int>> findSpecialZones(size_t mol_idx);
        std::optional<std::pair<int, int>> findMaxSpecialZone(size_t mol_idx, std::map<int, std::set<int>>& other_zones);

        void mergeCloseComponents();
        std::optional<std::pair<int, int>> isMergeable(size_t mol_idx1, size_t mol_idx2, std::optional<std::pair<int, int>>& current_zone);
        std::unique_ptr<BaseMolecule> extractComponent(int index);
        void sortSummblocks();

        bool mapReactionComponents();
        bool mapMultitailReactionComponents();
        void mergeUndefinedComponents();
        void collectUndefinedDistances(const std::vector<std::pair<size_t, Rect2f>>& component_bboxes,
                                       const std::vector<std::pair<size_t, Rect2f>>& undef_component_bboxes, std::vector<MOL_DISTANCES_DESC>& undef_distances,
                                       std::vector<MOL_DISTANCES_DESC>& undef_comp_distances);

        bool findPlusNeighbours(const Vec2f& plus_pos, const FLOAT_INT_PAIRS& mol_tops, const FLOAT_INT_PAIRS& mol_bottoms, const FLOAT_INT_PAIRS& mol_lefts,
                                const FLOAT_INT_PAIRS& mol_rights, std::pair<int, int>& connection);

        BaseMolecule& _bmol;
        std::vector<ReactionComponent> _reaction_components;
        std::vector<MolSumm> _component_summ_blocks;
        std::vector<COMPONENT_DESC> _components;
        std::vector<COMPONENT_DESC> _merged_components;
        std::vector<MOL_DISTANCES_DESC> _mol_distances;
        std::vector<SPECIAL_ZONE_DESC> _zones;
        int _moleculeCount;
        const LayoutOptions& _layout_options;
        float _reaction_margin_size;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
