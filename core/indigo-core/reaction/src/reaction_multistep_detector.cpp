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
#include <numeric>

#include "layout/pathway_layout.h"
#include "reaction/pathway_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_multistep_detector.h"

using namespace indigo;

inline void merge_bbox(Rect2f& bb1, const Rect2f& bb2)
{
    Vec2f lb, rt;
    lb.x = std::min(bb1.left(), bb2.left());
    rt.x = std::max(bb1.right(), bb2.right());
    lb.y = std::min(bb1.bottom(), bb2.bottom());
    rt.y = std::max(bb1.top(), bb2.top());
    bb1 = Rect2f(lb, rt);
}

IMPL_ERROR(ReactionMultistepDetector, "reaction multistep detector");

ReactionMultistepDetector::~ReactionMultistepDetector()
{
}

ReactionMultistepDetector::ReactionMultistepDetector(BaseMolecule& bmol) : _bmol(bmol), _moleculeCount(0)
{
}

void ReactionMultistepDetector::createSummBlocks()
{
    auto pair_comp_asc = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.first > a.first; };
    auto pair_comp_des = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.first < a.first; };
    auto pair_comp_mol_asc = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.second > a.second; };
    std::list<std::unordered_set<int>> s_neighbors;
    getSGroupAtoms(_bmol, s_neighbors);
    _moleculeCount = _bmol.countComponents(s_neighbors);
    _reaction_components.reserve(_moleculeCount);
    FLOAT_INT_PAIRS mol_tops, mol_bottoms, mol_lefts, mol_rights;

    // collect components
    for (int i = 0; i < _moleculeCount; ++i)
    {
        Filter filter(_bmol.getDecomposition().ptr(), Filter::EQ, i);
        std::unique_ptr<BaseMolecule> component;
        if (_bmol.isQueryMolecule())
            component = std::make_unique<QueryMolecule>();
        else
            component = std::make_unique<Molecule>();

        BaseMolecule& mol = *component;
        mol.makeSubmolecule(_bmol, filter, 0, 0);
        Rect2f bbox;
        mol.getBoundingBox(bbox, MIN_MOL_SIZE);

        mol_tops.emplace_back(bbox.top(), i);
        mol_bottoms.emplace_back(bbox.bottom(), i);
        mol_lefts.emplace_back(bbox.left(), i);
        mol_rights.emplace_back(bbox.right(), i);
        _reaction_components.emplace_back(ReactionComponent::MOLECULE, bbox, i, std::move(component));
    }

    for (int i = 0; i < _bmol.meta().getMetaCount(ReactionPlusObject::CID); ++i)
    {
        auto& plus = (const ReactionPlusObject&)_bmol.meta().getMetaObject(ReactionPlusObject::CID, i);
        const auto& plus_pos = plus.getPos();
        Rect2f bbox(plus_pos - PLUS_BBOX_SHIFT, plus_pos + PLUS_BBOX_SHIFT);
        _reaction_components.emplace_back(ReactionComponent::PLUS, bbox, i, std::unique_ptr<BaseMolecule>(nullptr));
        _reaction_components.back().coordinates.push_back(plus_pos);
        int index = static_cast<int>(_reaction_components.size() - 1);
        mol_tops.emplace_back(bbox.top(), index);
        mol_bottoms.emplace_back(bbox.bottom(), index);
        mol_lefts.emplace_back(bbox.left(), index);
        mol_rights.emplace_back(bbox.right(), index);
    }

    for (int i = 0; i < _bmol.meta().getMetaCount(ReactionArrowObject::CID); ++i)
    {
        auto& arrow = (const ReactionArrowObject&)_bmol.meta().getMetaObject(ReactionArrowObject::CID, i);
        int arrow_type = arrow.getArrowType();
        bool reverseReactionOrder = arrow_type == ReactionArrowObject::ERetrosynthetic;
        const Vec2f& arr_begin = !reverseReactionOrder ? arrow.getTail() : arrow.getHead();
        const Vec2f& arr_end = !reverseReactionOrder ? arrow.getHead() : arrow.getTail();
        Rect2f bbox(arr_begin - ARROW_BBOX_SHIFT, arr_end + ARROW_BBOX_SHIFT);
        _reaction_components.emplace_back(arrow_type, bbox, i, std::unique_ptr<BaseMolecule>(nullptr));
        _reaction_components.back().coordinates.push_back(arr_begin);
        _reaction_components.back().coordinates.push_back(arr_end);
        int index = static_cast<int>(_reaction_components.size() - 1);
        mol_tops.emplace_back(bbox.top(), index);
        mol_bottoms.emplace_back(bbox.bottom(), index);
        mol_lefts.emplace_back(bbox.left(), index);
        mol_rights.emplace_back(bbox.right(), index);
    }

    for (int i = 0; i < _bmol.meta().getMetaCount(ReactionMultitailArrowObject::CID); ++i)
    {
        auto& multi = (const ReactionMultitailArrowObject&)_bmol.meta().getMetaObject(ReactionMultitailArrowObject::CID, i);
        auto& tails = multi.getTails();
        Rect2f bbox(Vec2f(tails.top().x, tails.top().y), Vec2f(multi.getHead().x, multi.getSpineBegin().y));
        _reaction_components.emplace_back(ReactionComponent::ARROW_MULTITAIL, bbox, i, std::unique_ptr<BaseMolecule>(nullptr));
        _reaction_components.back().coordinates.push_back(multi.getHead());
        for (auto& tail : tails)
            _reaction_components.back().coordinates.push_back(tail);
        _reaction_components.back().coordinates.push_back(multi.getSpineBegin());
        _reaction_components.back().coordinates.push_back(multi.getSpineEnd());

        int index = static_cast<int>(_reaction_components.size() - 1);
        mol_tops.emplace_back(bbox.top(), index);
        mol_bottoms.emplace_back(bbox.bottom(), index);
        mol_lefts.emplace_back(bbox.left(), index);
        mol_rights.emplace_back(bbox.right(), index);
    }

    // sort components
    std::sort(mol_tops.begin(), mol_tops.end(), pair_comp_asc);
    std::sort(mol_bottoms.begin(), mol_bottoms.end(), pair_comp_des);
    std::sort(mol_lefts.begin(), mol_lefts.end(), pair_comp_des);
    std::sort(mol_rights.begin(), mol_rights.end(), pair_comp_asc);

    for (int i = 0; i < _bmol.meta().getMetaCount(ReactionPlusObject::CID); ++i)
    {
        auto& plus = static_cast<const ReactionPlusObject&>(_bmol.meta().getMetaObject(ReactionPlusObject::CID, i));
        auto& plus_pos = plus.getPos();
        std::pair<int, int> plus_connection; // (component1_index, component2_index)

        if (findPlusNeighbours(plus_pos, mol_tops, mol_bottoms, mol_lefts, mol_rights, plus_connection))
        {
            _reaction_components[_moleculeCount + i].summ_block_idx = ReactionComponent::CONNECTED; // mark plus as connected

            auto rc_connection = std::make_pair(std::ref(_reaction_components[plus_connection.first]),
                                                std::ref(_reaction_components[plus_connection.second])); // component1, component2

            enum
            {
                LI_NEW_CONNECTION = 0,
                LI_FIRST_ONLY,
                LI_SECOND_ONLY,
                LI_BOTH
            };

            int logic_index = (rc_connection.first.summ_block_idx == ReactionComponent::NOT_CONNECTED ? 0 : 1) +
                              (rc_connection.second.summ_block_idx == ReactionComponent::NOT_CONNECTED ? 0 : 2);

            switch (logic_index)
            {
            case LI_NEW_CONNECTION: {
                // create brand new connection
                Rect2f bbox = rc_connection.first.bbox;
                merge_bbox(bbox, rc_connection.second.bbox);
                _component_summ_blocks_list.emplace_back(bbox); // add merged boxes of both components
                auto& last_sb = _component_summ_blocks_list.back();
                last_sb.indexes.push_back(plus_connection.first);
                last_sb.indexes.push_back(plus_connection.second);

                rc_connection.first.summ_block_idx = ReactionComponent::CONNECTED; // mark as connected
                rc_connection.second.summ_block_idx = ReactionComponent::CONNECTED;
                auto last_it = std::prev(_component_summ_blocks_list.end());
                rc_connection.first.summ_block_it = last_it; // bind to summ blocks list
                rc_connection.second.summ_block_it = last_it;
            }
            break;

            case LI_BOTH: {
                // merge two blocks
                auto& block_first = *rc_connection.first.summ_block_it;
                auto& block_second = *rc_connection.second.summ_block_it;
                auto second_block_it = rc_connection.second.summ_block_it;
                merge_bbox(block_first.bbox, block_second.bbox);
                for (int v : block_second.indexes)
                {
                    block_first.indexes.push_back(v);                                          // copy all indexes of block_second to block_first
                    _reaction_components[v].summ_block_it = rc_connection.first.summ_block_it; // patch copied components. now they belong to block_first.
                }
                _component_summ_blocks_list.erase(second_block_it); // remove block_second
            }
            break;

            case LI_FIRST_ONLY: {
                // connect second to the existing first block
                auto& block = *rc_connection.first.summ_block_it;
                block.indexes.push_back(plus_connection.second);                        // add second component
                merge_bbox(block.bbox, rc_connection.second.bbox);                      // merge second box with block box
                rc_connection.second.summ_block_it = rc_connection.first.summ_block_it; // bind second to the first block
                rc_connection.second.summ_block_idx = ReactionComponent::CONNECTED;     // mark second as connected
            }
            break;

            case LI_SECOND_ONLY: {
                // connect first to the existing second block
                auto& block = *rc_connection.second.summ_block_it;
                block.indexes.push_back(plus_connection.first);                         // add second component
                merge_bbox(block.bbox, rc_connection.first.bbox);                       // merge second box with block box
                rc_connection.first.summ_block_it = rc_connection.second.summ_block_it; // bind second to the first block
                rc_connection.first.summ_block_idx = ReactionComponent::CONNECTED;      // mark second as connected
            }
            break;
            }
        }
    }

    // copy list to vector and set summ block indexes
    for (auto& csb : _component_summ_blocks_list)
    {
        for (int v : csb.indexes)
            _reaction_components[v].summ_block_idx = static_cast<int>(_component_summ_blocks.size());
        _component_summ_blocks.push_back(csb);
    }

    // add all single molecules to _component_summ_blocks
    for (size_t i = 0; i < _reaction_components.size(); ++i)
    {
        auto& rc = _reaction_components[i];
        if (rc.component_type != ReactionComponent::MOLECULE)
            break;
        if (rc.summ_block_idx == ReactionComponent::NOT_CONNECTED)
        {
            rc.summ_block_idx = static_cast<int>(_component_summ_blocks.size());
            _component_summ_blocks.push_back(_reaction_components[i].bbox);
            _component_summ_blocks.back().indexes.push_back(static_cast<int>(i));
        }
    }
}

void ReactionMultistepDetector::sortSummblocks()
{
    // Create a list of original indices
    std::vector<int> indices(_component_summ_blocks.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Sort the indices based on reaction_idx
    std::sort(indices.begin(), indices.end(),
              [&](int a, int b) -> bool { return _component_summ_blocks[a].reaction_idx < _component_summ_blocks[b].reaction_idx; });

    // Create a mapping from old index to new index
    std::vector<int> old_to_new(_component_summ_blocks.size());
    for (int new_idx = 0; new_idx < static_cast<int>(indices.size()); ++new_idx)
        old_to_new[indices[new_idx]] = new_idx;

    // Sort the blocks based on the sorted indices
    std::vector<MolSumm> sorted_blocks;
    sorted_blocks.reserve(_component_summ_blocks.size());
    for (auto idx : indices)
        sorted_blocks.emplace_back(std::move(_component_summ_blocks[idx]));
    _component_summ_blocks = std::move(sorted_blocks);

    // Update arrows_to and arrows_from with new indices
    for (auto& block : _component_summ_blocks)
    {
        auto update_arrow = [&](int& arrow) {
            if (arrow >= 0 && arrow < static_cast<int>(old_to_new.size()))
                arrow = old_to_new[arrow];
        };

        std::for_each(block.arrows_to.begin(), block.arrows_to.end(), update_arrow);
        std::for_each(block.arrows_from.begin(), block.arrows_from.end(), update_arrow);
    }
}

ReactionMultistepDetector::ReactionType ReactionMultistepDetector::detectReaction()
{
    createSummBlocks();
    bool has_multistep = mapReactionComponents();
    bool has_multitail = mapMultitailReactionComponents();
    sortSummblocks();
    return has_multitail ? ReactionType::EPathwayReaction : (has_multistep ? ReactionType ::EMutistepReaction : ReactionType::ESimpleReaction);
}

bool ReactionMultistepDetector::mapReactionComponents()
{
    int arrow_count = _bmol.meta().getMetaCount(ReactionArrowObject::CID);
    if (arrow_count == 0)
        return false;
    for (int reaction_index = 0; reaction_index < arrow_count; ++reaction_index)
    {
        auto& arrow = (const ReactionArrowObject&)_bmol.meta().getMetaObject(ReactionArrowObject::CID, reaction_index);
        int arrow_type = arrow.getArrowType();
        bool reverseReactionOrder = arrow_type == ReactionArrowObject::ERetrosynthetic;
        const Vec2f& arr_begin = !reverseReactionOrder ? arrow.getTail() : arrow.getHead();
        const Vec2f& arr_end = !reverseReactionOrder ? arrow.getHead() : arrow.getTail();

        float min_dist_prod = -1, min_dist_reac = -1;
        int idx_cs_min_prod = -1, idx_cs_min_reac = -1;
        for (int index_cs = 0; index_cs < static_cast<int>(_component_summ_blocks.size()); ++index_cs)
        {
            auto& csb = _component_summ_blocks[index_cs];
            if (csb.bbox.rayIntersectsRect(arr_end, arr_begin))
            {
                float dist = csb.bbox.pointDistance(arr_end);
                if (min_dist_prod < 0 || dist < min_dist_prod)
                {
                    min_dist_prod = dist;
                    idx_cs_min_prod = index_cs;
                }
            }
            else if (csb.bbox.rayIntersectsRect(arr_begin, arr_end))
            {
                float dist = csb.bbox.pointDistance(arr_begin);
                if (min_dist_reac < 0 || dist < min_dist_reac)
                {
                    min_dist_reac = dist;
                    idx_cs_min_reac = index_cs;
                }
            }
        }

        // TODO: add upper limit
        if (min_dist_prod > 0 && min_dist_reac > 0) // if both ends present
        {
            auto& rc_arrow = _reaction_components[_moleculeCount + _bmol.meta().getMetaCount(ReactionPlusObject::CID) + reaction_index];
            rc_arrow.summ_block_idx = ReactionComponent::CONNECTED; // mark arrow as connected
            auto& csb_min_prod = _component_summ_blocks[idx_cs_min_prod];
            if (csb_min_prod.role == BaseReaction::UNDEFINED)
                csb_min_prod.role = BaseReaction::PRODUCT;
            else if (csb_min_prod.role == BaseReaction::REACTANT)
                csb_min_prod.role = BaseReaction::INTERMEDIATE;

            auto& csb_min_reac = _component_summ_blocks[idx_cs_min_reac];
            if (csb_min_reac.role == BaseReaction::UNDEFINED)
                csb_min_reac.role = BaseReaction::REACTANT;
            else if (csb_min_reac.role == BaseReaction::PRODUCT)
                csb_min_reac.role = BaseReaction::INTERMEDIATE;

            // idx_cs_min_reac <-> idx_cs_min_prod
            csb_min_reac.arrows_to.push_back(idx_cs_min_prod);
            csb_min_prod.arrows_from.push_back(idx_cs_min_reac);
            // csb_min_reac.reaction_idx = reaction_index;
            csb_min_prod.reaction_idx = reaction_index;
        }
    }
    return arrow_count > 1;
}

bool ReactionMultistepDetector::mapMultitailReactionComponents()
{
    int pathway_count = _bmol.meta().getMetaCount(ReactionMultitailArrowObject::CID);

    if (pathway_count == 0)
        return false;

    bool bad_pathway = false;

    for (int pathway_idx = 0; pathway_idx < pathway_count; ++pathway_idx)
    {
        auto& multi = (const ReactionMultitailArrowObject&)_bmol.meta().getMetaObject(ReactionMultitailArrowObject::CID, pathway_idx);
        float min_dist_prod = -1;
        int idx_cs_min_prod = -1;
        auto arr_begin = multi.getHead();
        auto arr_end = arr_begin;
        arr_end.x = multi.getSpineBegin().x;
        auto& tails = multi.getTails();
        std::vector<std::pair<float, int>> min_dist_reactants;
        min_dist_reactants.resize(tails.size(), std::make_pair(-1.0f, -1));

        for (int index_cs = 0; index_cs < static_cast<int>(_component_summ_blocks.size()); ++index_cs)
        {
            auto& csb = _component_summ_blocks[index_cs];
            if (csb.bbox.rayIntersectsRect(arr_begin, arr_end))
            {
                float dist = csb.bbox.pointDistance(arr_end);
                if (min_dist_prod < 0 || dist < min_dist_prod)
                {
                    min_dist_prod = dist;
                    idx_cs_min_prod = index_cs;
                }
            }

            for (int j = 0; j < tails.size(); ++j)
            {
                auto arr_begin = tails[j];
                auto arr_end = arr_begin;
                arr_begin.x = multi.getSpineBegin().x;

                if (csb.bbox.rayIntersectsRect(arr_end, arr_begin))
                {
                    auto dist = csb.bbox.pointDistance(arr_begin);
                    if (min_dist_reactants[j].first < 0 || dist < min_dist_reactants[j].first)
                    {
                        min_dist_reactants[j].first = dist;
                        min_dist_reactants[j].second = index_cs;
                    }
                }
            }
        }

        // TODO: add upper limit
        if (idx_cs_min_prod < 0)
            bad_pathway = true;
        else
        {
            auto& csb_min_prod = _component_summ_blocks[idx_cs_min_prod];
            csb_min_prod.reaction_idx = pathway_idx + _bmol.meta().getMetaCount(ReactionArrowObject::CID);
            auto& rc_arrow = _reaction_components[_moleculeCount + _bmol.meta().getMetaCount(ReactionPlusObject::CID) +
                                                  _bmol.meta().getMetaCount(ReactionArrowObject::CID) + pathway_idx];
            rc_arrow.summ_block_idx = ReactionComponent::CONNECTED; // mark arrow as connected
            if (csb_min_prod.role == BaseReaction::UNDEFINED)
                csb_min_prod.role = BaseReaction::PRODUCT;
            else if (csb_min_prod.role == BaseReaction::REACTANT)
                csb_min_prod.role = BaseReaction::INTERMEDIATE;
        }

        PathwayComponent pwc;

        for (int j = 0; j < (int)min_dist_reactants.size(); ++j)
        {
            auto& reac = min_dist_reactants[j];
            if (reac.first > 0)
            {
                auto& csb_min_reac = _component_summ_blocks[reac.second];
                if (csb_min_reac.role == BaseReaction::UNDEFINED)
                    csb_min_reac.role = BaseReaction::REACTANT;
                else if (csb_min_reac.role == BaseReaction::PRODUCT)
                    csb_min_reac.role = BaseReaction::INTERMEDIATE;

                // idx_cs_min_reac <-> idx_cs_min_prod

                if (idx_cs_min_prod >= 0)
                {
                    csb_min_reac.arrows_to.push_back(idx_cs_min_prod);
                    _component_summ_blocks[idx_cs_min_prod].arrows_from.push_back(reac.second);
                }
                // csb_min_reac.reaction_idx = pathway_idx + _bmol.meta().getMetaCount(ReactionArrowObject::CID);
            }
            else
                bad_pathway = true;
        }
    }
    for (auto& csb : _component_summ_blocks)
    {
        // no undefined components allowed
        if (csb.role == BaseReaction::UNDEFINED)
        {
            csb.role = BaseReaction::REACTANT;
            bad_pathway = true;
        }
    }
    return !bad_pathway;
}

bool ReactionMultistepDetector::findPlusNeighbours(const Vec2f& plus_pos, const FLOAT_INT_PAIRS& mol_tops, const FLOAT_INT_PAIRS& mol_bottoms,
                                                   const FLOAT_INT_PAIRS& mol_lefts, const FLOAT_INT_PAIRS& mol_rights, std::pair<int, int>& connection)
{
    auto plus_pos_y = std::make_pair(plus_pos.y, 0);
    auto plus_pos_x = std::make_pair(plus_pos.x, 0);

    auto pair_comp_asc = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.first > a.first; };
    auto pair_comp_des = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.first < a.first; };
    auto pair_comp_mol_asc = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.second > a.second; };
    // look for mols where top > y
    auto tops_above_it = std::upper_bound(mol_tops.begin(), mol_tops.end(), plus_pos_y, pair_comp_asc);
    // look for mols where bottom < y
    auto bottoms_below_it = std::upper_bound(mol_bottoms.begin(), mol_bottoms.end(), plus_pos_y, pair_comp_des);
    // look for mols where right > x
    auto rights_after_it = std::upper_bound(mol_rights.begin(), mol_rights.end(), plus_pos_x, pair_comp_asc);
    // look for mols where left < x
    auto lefts_before_it = std::upper_bound(mol_lefts.begin(), mol_lefts.end(), plus_pos_x, pair_comp_des);

    FLOAT_INT_PAIRS tops_above, bottoms_below, lefts_before, rights_after;

    std::copy(tops_above_it, mol_tops.end(), std::back_inserter(tops_above));
    std::copy(bottoms_below_it, mol_bottoms.end(), std::back_inserter(bottoms_below));
    std::copy(rights_after_it, mol_rights.end(), std::back_inserter(rights_after));
    std::copy(lefts_before_it, mol_lefts.end(), std::back_inserter(lefts_before));

    std::sort(tops_above.begin(), tops_above.end(), pair_comp_mol_asc);
    std::sort(bottoms_below.begin(), bottoms_below.end(), pair_comp_mol_asc);
    std::sort(rights_after.begin(), rights_after.end(), pair_comp_mol_asc);
    std::sort(lefts_before.begin(), lefts_before.end(), pair_comp_mol_asc);

    // build intersections
    FLOAT_INT_PAIRS intersection_top_bottom, intersection_left_right;
    std::set_intersection(tops_above.begin(), tops_above.end(), bottoms_below.begin(), bottoms_below.end(), std::back_inserter(intersection_top_bottom),
                          pair_comp_mol_asc);

    std::set_intersection(lefts_before.begin(), lefts_before.end(), rights_after.begin(), rights_after.end(), std::back_inserter(intersection_left_right),
                          pair_comp_mol_asc);

    // collect left-right
    FLOAT_INT_PAIRS rights_row, lefts_row, tops_col, bottoms_col;
    for (const auto& kvp : intersection_top_bottom)
    {
        auto& tb_box = _reaction_components[kvp.second].bbox;
        if (tb_box.pointInRect(plus_pos))
            continue;
        rights_row.emplace_back(tb_box.right(), kvp.second);
        lefts_row.emplace_back(tb_box.left(), kvp.second);
    }

    // collect top-bottom
    for (const auto& kvp : intersection_left_right)
    {
        auto& lr_box = _reaction_components[kvp.second].bbox;
        if (lr_box.pointInRect(plus_pos))
            continue;
        tops_col.emplace_back(lr_box.top(), kvp.second);
        bottoms_col.emplace_back(lr_box.bottom(), kvp.second);
    }

    std::sort(lefts_row.begin(), lefts_row.end(), pair_comp_asc);
    std::sort(rights_row.begin(), rights_row.end(), pair_comp_des);
    std::sort(tops_col.begin(), tops_col.end(), pair_comp_asc);
    std::sort(bottoms_col.begin(), bottoms_col.end(), pair_comp_des);

    auto rights_row_it = std::upper_bound(rights_row.begin(), rights_row.end(), plus_pos_x, pair_comp_des);
    auto lefts_row_it = std::upper_bound(lefts_row.begin(), lefts_row.end(), plus_pos_x, pair_comp_asc);
    auto tops_col_it = std::upper_bound(tops_col.begin(), tops_col.end(), plus_pos_y, pair_comp_asc);
    auto bottoms_col_it = std::upper_bound(bottoms_col.begin(), bottoms_col.end(), plus_pos_y, pair_comp_des);

    float min_distance_h = 0, min_distance_v = 0;

    bool result = false;

    if (rights_row_it != rights_row.end() && lefts_row_it != lefts_row.end())
    {
        min_distance_h = std::min(std::fabs(rights_row_it->first - plus_pos_x.first), std::fabs(plus_pos_x.first - lefts_row_it->first));
        connection.second = lefts_row_it->second;
        connection.first = rights_row_it->second;
        result = _reaction_components[connection.first].component_type == ReactionComponent::MOLECULE &&
                 _reaction_components[connection.second].component_type == ReactionComponent::MOLECULE;
    }

    if (tops_col_it != tops_col.end() && bottoms_col_it != bottoms_col.end())
    {
        min_distance_v = std::min(std::fabs(tops_col_it->first - plus_pos_y.first), std::fabs(bottoms_col_it->first - plus_pos_y.first));
        if (!result || min_distance_v < min_distance_h)
        {
            connection.first = tops_col_it->second;
            connection.second = bottoms_col_it->second;
            result = _reaction_components[connection.first].component_type == ReactionComponent::MOLECULE &&
                     _reaction_components[connection.second].component_type == ReactionComponent::MOLECULE;
        }
    }
    return result;
}

void ReactionMultistepDetector::constructPathwayReaction(PathwayReaction& rxn)
{
    std::unordered_map<int, int> rc_to_molecule;
    std::vector<std::pair<std::vector<int>, std::vector<int>>> csb_reactions;

    std::vector<int> csb_product_to_reaction;
    std::vector<std::unordered_map<int, int>> csb_to_reactant_indexes;

    for (int i = 0; i < (int)_component_summ_blocks.size(); ++i)
    {
        csb_product_to_reaction.push_back(-1);
        auto& csb = _component_summ_blocks[i];
        if (csb.role == BaseReaction::PRODUCT || csb.role == BaseReaction::INTERMEDIATE)
        {
            // one product = one reaction, one reaction node
            // map reactionNode <> csb_reaction
            auto& rcidx_to_reactant = csb_to_reactant_indexes.emplace_back();
            csb_product_to_reaction.back() = (int)csb_reactions.size();
            csb_reactions.emplace_back().second.push_back(i); // add csb as product

            auto [sri, sr] = rxn.addReaction();
            sr.arrowMetaIndex = csb.reaction_idx;
            // add products
            for (auto rc_idx : csb.indexes)
            {
                auto& rc = _reaction_components[rc_idx];
                auto it_copied = rc_to_molecule.find(rc_idx);
                int mol_idx = -1;
                if (it_copied != rc_to_molecule.end())
                    mol_idx = it_copied->second;
                else
                {
                    mol_idx = rxn.addMolecule(*rc.molecule);
                    rc_to_molecule.emplace(rc_idx, mol_idx);
                }
                sr.productIndexes.push(mol_idx);
            }

            // add reactants
            for (auto reactant_idx : csb.arrows_from)
            {
                csb_reactions.back().first.push_back(reactant_idx);
                auto& csb_reactant = _component_summ_blocks[reactant_idx];
                for (auto rc_idx : csb_reactant.indexes)
                {
                    auto& rc = _reaction_components[rc_idx];
                    auto it_copied = rc_to_molecule.find(rc_idx);
                    int mol_idx = -1;
                    if (it_copied != rc_to_molecule.end())
                        mol_idx = it_copied->second;
                    else
                    {
                        mol_idx = rxn.addMolecule(*rc.molecule);
                        rc_to_molecule.emplace(rc_idx, mol_idx);
                    }
                    rcidx_to_reactant.emplace(rc_idx, (int)sr.reactantIndexes.size());
                    sr.reactantIndexes.push(mol_idx);
                }
            }
        }
    }

    // fill reaction nodes
    for (int i = 0; i < (int)csb_reactions.size(); ++i)
    {
        auto& csb_reaction = csb_reactions[i];
        auto& rn = rxn.addReactionNode();
        // normally we have only one summblock here
        for (auto csb_product_idx : csb_reaction.second)
        {
            auto& csb_product = _component_summ_blocks[csb_product_idx];
            for (auto reactant_csb_idx : csb_product.arrows_to)
            {
                // look up for reactions where the product plays a role of reactant
                auto reac_idx = csb_product_to_reaction[reactant_csb_idx];
                if (reac_idx >= 0 && reac_idx != i)
                    rn.successorReactionIndexes.push(reac_idx);
            }
        }

        // iterate reactant summ blocks
        for (auto csb_reactant_idx : csb_reaction.first)
        {
            auto& csb_reactant = _component_summ_blocks[csb_reactant_idx];
            // find reactions where the reactant plays a role of product
            auto reac_idx = csb_product_to_reaction[csb_reactant_idx];
            if (reac_idx >= 0 && reac_idx != i)
            {
                rn.precursorReactionIndexes.push(reac_idx);
                auto& rcidx_to_reactant = csb_to_reactant_indexes[i];
                for (auto ridx : csb_reactant.indexes)
                {
                    auto rcidx_it = rcidx_to_reactant.find(ridx);
                    if (rcidx_it != rcidx_to_reactant.end())
                        rn.connectedReactants.insert(rcidx_it->second, reac_idx);
                }
            }
        }
    }
    detectPathwayMetadata(rxn);
}

void ReactionMultistepDetector::detectPathwayMetadata(PathwayReaction& rxn)
{
    auto arrow_count = rxn.meta().getMetaCount(ReactionArrowObject::CID);
    auto multi_count = rxn.meta().getMetaCount(ReactionMultitailArrowObject::CID);

    for (int i = 0; i < rxn.getReactionCount(); ++i)
    {
        auto& sr = rxn.getReaction(i);
        if (sr.arrowMetaIndex >= 0)
        {
            if (arrow_count && sr.arrowMetaIndex < arrow_count)
            {
                auto& arrow = static_cast<const ReactionArrowObject&>(rxn.meta().getMetaObject(ReactionArrowObject::CID, sr.arrowMetaIndex));
                Vec2f box_rt = arrow.getHead();
                box_rt.y += (arrow.getHead() - arrow.getTail()).length() / 2;
                Rect2f lookup_box(arrow.getTail(), box_rt);
                collectMetadata(i, rxn, lookup_box);
            }
            else
            {
                auto& multi_arrow = static_cast<const ReactionMultitailArrowObject&>(
                    rxn.meta().getMetaObject(ReactionMultitailArrowObject::CID, sr.arrowMetaIndex - arrow_count));
                Rect2f lookup_box(multi_arrow.getHead(), multi_arrow.getSpineBegin());
                collectMetadata(i, rxn, lookup_box);
            }
        }
    }
}

void ReactionMultistepDetector::collectMetadata(int reaction_idx, PathwayReaction& rxn, const Rect2f& bbox)
{
    auto& sr = rxn.getReaction(reaction_idx);
    float min_dist;
    int text_idx = -1;
    for (int i = 0; i < rxn.meta().getMetaCount(SimpleTextObject::CID); ++i)
    {
        auto& text = static_cast<const SimpleTextObject&>(rxn.meta().getMetaObject(SimpleTextObject::CID, i));
        Rect2f text_bbox;
        text.getBoundingBox(text_bbox);
        if (bbox.intersects(text_bbox))
        {
            auto dist = Vec2f::dist(bbox.leftBottom(), text_bbox.leftBottom());
            if (text_idx < 0 || dist < min_dist)
            {
                text_idx = i;
                min_dist = dist;
            }
        }
    }
    if (text_idx >= 0)
    {
        collectProperties(sr, static_cast<const SimpleTextObject&>(rxn.meta().getMetaObject(SimpleTextObject::CID, text_idx)));
        int meta_id = rxn.meta().getMetaObjectIndex(SimpleTextObject::CID, text_idx);
        rxn.meta().addExplicitReactionObjectIndex(meta_id);
    }
}

void ReactionMultistepDetector::collectProperties(PathwayReaction::SimpleReaction& sr, const SimpleTextObject& text_obj)
{
    std::string name, condition;
    bool is_condition = false;
    for (const auto& line : text_obj.getLines())
    {
        if (line.text.size())
        {
            if (is_condition)
            {
                if (condition.size())
                    condition += "\n";
                condition += line.text;
            }
            else
            {
                if (name.size())
                    name += "\n";
                name += line.text;
            }
        }
        else
            is_condition = true;
    }

    int id = sr.properties.insert(PathwayLayout::REACTION_NAME);
    sr.properties.value(id).readString(name.empty() ? PathwayLayout::REACTION_PROPERTY_NA : name.c_str(), true);

    id = sr.properties.insert(PathwayLayout::REACTION_CONDITIONS);
    sr.properties.value(id).readString(condition.empty() ? PathwayLayout::REACTION_PROPERTY_NA : condition.c_str(), true);
}

void ReactionMultistepDetector::constructMultipleArrowReaction(BaseReaction& rxn)
{
    // _reaction_components -> allMolecules
    // _component_summ_blocks ->
    std::unordered_map<int, int> copied_components;
    for (int i = 0; i < (int)_component_summ_blocks.size(); ++i)
    {
        auto& csb = _component_summ_blocks[i];
        for (auto idx : csb.indexes)
        {
            auto& rc = _reaction_components[idx];
            if (!copied_components.count(idx))
                switch (csb.role)
                {
                case BaseReaction::INTERMEDIATE:
                    copied_components.emplace(idx, rxn.addIntermediateCopy(*rc.molecule, 0, 0));
                    break;
                case BaseReaction::REACTANT:
                    copied_components.emplace(idx, rxn.addReactantCopy(*rc.molecule, 0, 0));
                    break;
                case BaseReaction::PRODUCT:
                    copied_components.emplace(idx, rxn.addProductCopy(*rc.molecule, 0, 0));
                    break;
                case BaseReaction::UNDEFINED:
                    copied_components.emplace(idx, rxn.addUndefinedCopy(*rc.molecule, 0, 0));
                    break;
                default:
                    break;
                }
        }
    }

    for (int i = 0; i < (int)_component_summ_blocks.size(); ++i)
    {
        auto& csb = _component_summ_blocks[i];
        for (auto csb_index : csb.arrows_to)
        {
            auto& rb = rxn.addReactionBlock();
            // add reactants
            for (auto ridx : csb.indexes)
            {
                auto r_it = copied_components.find(ridx);
                if (r_it != copied_components.end())
                    rb.reactants.push(r_it->second);
            }

            // add products
            auto& csb_product = _component_summ_blocks[csb_index];
            for (auto pidx : csb_product.indexes)
            {
                auto p_it = copied_components.find(pidx);
                if (p_it != copied_components.end())
                    rb.products.push(p_it->second);
            }
        }
    }
}

void ReactionMultistepDetector::constructSimpleArrowReaction(BaseReaction& rxn)
{
    for (auto& csb : _component_summ_blocks)
    {
        switch (csb.role)
        {
        case BaseReaction::PRODUCT: {
            for (auto idx : csb.indexes)
            {
                auto& rc = _reaction_components[idx];
                rxn.addProductCopy(*rc.molecule, 0, 0);
            }
        }
        break;
        case BaseReaction::REACTANT: {
            for (auto idx : csb.indexes)
            {
                auto& rc = _reaction_components[idx];
                rxn.addReactantCopy(*rc.molecule, 0, 0);
            }
        }
        break;
        case BaseReaction::INTERMEDIATE: {
            for (auto idx : csb.indexes)
            {
                auto& rc = _reaction_components[idx];
                rxn.addIntermediateCopy(*rc.molecule, 0, 0);
            }
        }
        break;
        case BaseReaction::UNDEFINED: {
            for (auto idx : csb.indexes)
            {
                auto& rc = _reaction_components[idx];
                rxn.addUndefinedCopy(*rc.molecule, 0, 0);
            }
        }
        break;
        default:
            break;
        }
    }
}