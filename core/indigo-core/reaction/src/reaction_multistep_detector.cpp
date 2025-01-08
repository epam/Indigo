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
#include <queue>

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
    _reaction_components.reserve(_moleculeCount);
    FLOAT_INT_PAIRS mol_tops, mol_bottoms, mol_lefts, mol_rights;

    // collect components
    for (int i = 0; i < _moleculeCount; ++i)
    {
        Rect2f bbox;
        _components[i].first->getBoundingBox(bbox, MIN_MOL_SIZE);

        mol_tops.emplace_back(bbox.top(), i);
        mol_bottoms.emplace_back(bbox.bottom(), i);
        mol_lefts.emplace_back(bbox.left(), i);
        mol_rights.emplace_back(bbox.right(), i);
        _reaction_components.emplace_back(ReactionComponent::MOLECULE, bbox, i, std::move(_components[i].first));
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

std::unique_ptr<BaseMolecule> ReactionMultistepDetector::extractComponent(int index)
{
    Filter filter(_bmol.getDecomposition().ptr(), Filter::EQ, index);
    std::unique_ptr<BaseMolecule> component;
    if (_bmol.isQueryMolecule())
        component = std::make_unique<QueryMolecule>();
    else
        component = std::make_unique<Molecule>();
    component->makeSubmolecule(_bmol, filter, 0, 0);
    return component;
}

void ReactionMultistepDetector::collectSortedDistances()
{
    _mol_distances.resize(_moleculeCount);
    for (int i = 0; i < _moleculeCount; ++i)
    {
        for (int j = i + 1; j < _moleculeCount; ++j)
        {
            float dist = computeConvexDistance(_components[i].second, _components[j].second);
            auto& mdi = _mol_distances[i];
            auto it = std::lower_bound(mdi.sorted_distances.begin(), mdi.sorted_distances.end(), std::make_pair(j, dist),
                                       [](auto& lhs, auto& rhs) { return lhs.second < rhs.second; });

            mdi.sorted_distances.insert(it, {j, dist});
            mdi.distances_map[j] = dist;

            auto& mdj = _mol_distances[j];
            it = std::lower_bound(mdj.sorted_distances.begin(), mdj.sorted_distances.end(), std::make_pair(i, dist),
                                  [](auto& lhs, auto& rhs) { return lhs.second < rhs.second; });

            mdj.sorted_distances.insert(it, {i, dist});
            mdj.distances_map[i] = dist;
        }
    }
}

void ReactionMultistepDetector::createSpecialZones()
{
    for (int i = 0; i < _bmol.meta().getMetaCount(ReactionPlusObject::CID); ++i)
    {
        auto& plus = (const ReactionPlusObject&)_bmol.meta().getMetaObject(ReactionPlusObject::CID, i);
        const auto& plus_pos = plus.getPos();
        addPlusZones(plus_pos);
    }

    for (int i = 0; i < _bmol.meta().getMetaCount(ReactionArrowObject::CID); ++i)
    {
        auto& arrow = (const ReactionArrowObject&)_bmol.meta().getMetaObject(ReactionArrowObject::CID, i);
        addArrowZones(arrow.getTail(), arrow.getHead());
    }

    for (int i = 0; i < _bmol.meta().getMetaCount(ReactionMultitailArrowObject::CID); ++i)
    {
        auto& multi = (const ReactionMultitailArrowObject&)_bmol.meta().getMetaObject(ReactionMultitailArrowObject::CID, i);
        auto& tails = multi.getTails();
        auto& head = multi.getHead();
        auto& spine_beg = multi.getSpineBegin();
        auto& spine_end = multi.getSpineEnd();
        std::vector<Vec2f> tails_vec(tails.begin(), tails.end());
        addPathwayZones(head, spine_beg, spine_end, tails_vec);
    }
}

void ReactionMultistepDetector::addPlusZones(const Vec2f& pos)
{
    Rect2f bbox(pos - PLUS_BBOX_SHIFT, pos + PLUS_BBOX_SHIFT);
    SPECIAL_ZONE_DESC szd;
    szd.zone_type = ZoneType::EPlus;
    szd.origin_coordinates.push_back(pos);
    std::vector<Vec2f> left, right, bottom, top;

    // left zone
    left.push_back(pos);
    left.push_back(bbox.leftBottom());
    left.push_back(Vec2f(left.back().x - PLUS_DETECTION_DISTANCE, left.back().y));
    left.push_back(Vec2f(left.back().x, left.back().y + bbox.height()));
    left.push_back(bbox.leftTop());
    left.push_back(pos);

    // right zone
    right.push_back(pos);
    right.push_back(bbox.rightTop());
    right.push_back(Vec2f(right.back().x + PLUS_DETECTION_DISTANCE, right.back().y));
    right.push_back(Vec2f(right.back().x, right.back().y - bbox.height()));
    right.push_back(bbox.rightBottom());
    right.push_back(pos);

    // bottom zone
    bottom.push_back(pos);
    bottom.push_back(bbox.rightBottom());
    bottom.push_back(Vec2f(bottom.back().x, bottom.back().y - PLUS_DETECTION_DISTANCE));
    bottom.push_back(Vec2f(bottom.back().x - bbox.width(), bottom.back().y));
    bottom.push_back(bbox.leftBottom());
    bottom.push_back(pos);

    // top zone
    top.push_back(pos);
    top.push_back(bbox.leftTop());
    top.push_back(Vec2f(top.back().x, top.back().y + PLUS_DETECTION_DISTANCE));
    top.push_back(Vec2f(top.back().x + bbox.width(), top.back().y));
    top.push_back(bbox.rightTop());
    top.push_back(pos);
    szd.zone_sections.push_back(left);
    szd.zone_sections.push_back(right);
    szd.zone_sections.push_back(bottom);
    szd.zone_sections.push_back(top);
    _zones.push_back(szd);
}

void ReactionMultistepDetector::addArrowZones(const Vec2f& tail, const Vec2f& head)
{
    float dx = head.x - tail.x, dy = head.y - tail.y;
    Vec2f dir = {dx, dy};
    float length = std::hypot(dx, dy), half_length = length * 0.5f;
    float inv = 1.0f / length;
    Vec2f nT = {-dy * inv, dx * inv};
    Vec2f nB = {dy * inv, -dx * inv};

    Vec2f nTop = nT * half_length;
    Vec2f nBottom = nB * half_length;

    std::vector<Vec2f> top{{head.x, head.y}, {tail.x, tail.y}, {tail.x + nTop.x, tail.y + nTop.y}, {head.x + nTop.x, head.y + nTop.y}};
    std::vector<Vec2f> bottom{{tail.x, tail.y}, {head.x, head.y}, {head.x + nBottom.x, head.y + nBottom.y}, {tail.x + nBottom.x, tail.y + nBottom.y}};

    std::vector<Vec2f> left, right;
    Vec2f pos = tail + nB / 2;
    left.push_back(pos);
    pos -= dir * ARROW_DETECTION_DISTANCE;
    left.push_back(pos);
    pos += nT;
    left.push_back(pos);
    pos += dir * ARROW_DETECTION_DISTANCE;
    left.push_back(pos);
    pos += nB;
    left.push_back(pos);

    pos = head + nT / 2;
    right.push_back(pos);
    pos += dir * ARROW_DETECTION_DISTANCE;
    right.push_back(pos);
    pos += nB;
    right.push_back(pos);
    pos -= dir * ARROW_DETECTION_DISTANCE;
    right.push_back(pos);
    pos += nT;
    right.push_back(pos);

    SPECIAL_ZONE_DESC szd;
    szd.zone_type = ZoneType::EArrow;
    szd.zone_sections.push_back(top);
    szd.zone_sections.push_back(bottom);
    szd.zone_sections.push_back(left);
    szd.zone_sections.push_back(right);
    szd.origin_coordinates.push_back(tail);
    szd.origin_coordinates.push_back(head);
    _zones.push_back(szd);
}

void ReactionMultistepDetector::addPathwayZones(const Vec2f& head, const Vec2f& sp_beg, const Vec2f& sp_end, const std::vector<Vec2f>& tails)
{
    std::vector<Vec2f> right, top, bottom;

    // form products zone
    Vec2f pos(head);
    pos.y += LayoutOptions::DEFAULT_BOND_LENGTH / 2.0f;
    right.push_back(pos);
    pos.x += ARROW_DETECTION_DISTANCE;
    right.push_back(pos);
    pos.y -= LayoutOptions::DEFAULT_BOND_LENGTH;
    right.push_back(pos);
    pos.x -= ARROW_DETECTION_DISTANCE;
    right.push_back(pos);
    pos.y += LayoutOptions::DEFAULT_BOND_LENGTH;
    right.push_back(pos);

    // form top agents zone
    pos = head;
    top.push_back(pos);
    pos.x = sp_beg.x;
    top.push_back(pos);
    pos.y = sp_beg.y;
    top.push_back(pos);
    pos.x = head.x;
    top.push_back(pos);
    pos = head;
    top.push_back(pos);
    bottom.push_back(pos);
    pos.y = sp_end.y;
    bottom.push_back(pos);
    pos.x = sp_end.x;
    bottom.push_back(pos);
    pos.y = head.y;
    bottom.push_back(pos);
    bottom.push_back(head);
    SPECIAL_ZONE_DESC szd;
    szd.zone_type = ZoneType::EPathWay;
    szd.zone_sections.push_back(top);
    szd.zone_sections.push_back(bottom);
    szd.zone_sections.push_back(right);
    szd.origin_coordinates.push_back(head);
    szd.origin_coordinates.push_back(sp_beg);
    szd.origin_coordinates.push_back(sp_end);
    for (const auto& tail : tails)
    {
        std::vector<Vec2f> left;
        pos = tail;
        pos.y -= LayoutOptions::DEFAULT_BOND_LENGTH / 2.0f;
        left.push_back(pos);
        pos.x -= ARROW_DETECTION_DISTANCE;
        left.push_back(pos);
        pos.y += LayoutOptions::DEFAULT_BOND_LENGTH;
        left.push_back(pos);
        pos.x += ARROW_DETECTION_DISTANCE;
        left.push_back(pos);
        pos.y -= LayoutOptions::DEFAULT_BOND_LENGTH;
        left.push_back(pos);
        szd.zone_sections.push_back(left);
        szd.origin_coordinates.push_back(tail);
    }
    _zones.push_back(szd);
}

std::map<int, std::unordered_set<int>> ReactionMultistepDetector::findSpecialZones(size_t mol_idx)
{
    auto& hull = _components[mol_idx].second;
    std::map<int, std::unordered_set<int>> result;
    for (int i = 0; i < static_cast<int>(_zones.size()); ++i)
    {
        auto& zone = _zones[i];
        std::pair<int, std::unordered_set<int>> cur_zone(i, {});
        for (int j = 0; j < static_cast<int>(zone.zone_sections.size()); ++j)
        {
            auto& section = zone.zone_sections[j];
            if (convexPolygonsIntersect(hull, section))
                cur_zone.second.insert(j);
        }
        if (cur_zone.second.size())
            result.insert(cur_zone);
    }
    return result;
}

std::optional<std::pair<int, int>> ReactionMultistepDetector::findMaxSpecialZone(size_t mol_idx)
{
    std::optional<std::pair<int, int>> result{};
    float max_containment = 0.0f;
    auto& hull = _components[mol_idx].second;
    for (int i = 0; i < static_cast<int>(_zones.size()); ++i)
    {
        auto& zone = _zones[i];
        std::pair<int, std::unordered_set<int>> cur_zone(i, {});
        for (int j = 0; j < static_cast<int>(zone.zone_sections.size()); ++j)
        {
            auto& section = zone.zone_sections[j];
            float cont = computeConvexContainment(hull, section);
            if (cont > max_containment)
            {
                max_containment = cont;
                result = {i, j};
            }
        }
    }
    return result;
}

void ReactionMultistepDetector::mergeCloseComponents()
{
    for (std::size_t i = 0; i < _components.size(); ++i)
    {
        if (!_components[i].first)
            continue;
        std::queue<std::size_t> bfs_queue;
        std::vector<std::size_t> cluster;
        std::optional<std::pair<int, int>> current_zone{};
        bfs_queue.push(i);
        cluster.push_back(i);
        while (!bfs_queue.empty())
        {
            auto idx = bfs_queue.front();
            bfs_queue.pop();
            for (std::size_t j = 0; j < _components.size(); ++j)
            {
                if (!_components[j].first || j == idx)
                    continue;
                if (std::find(cluster.begin(), cluster.end(), j) != cluster.end())
                    continue;
                auto zone = isMergeable(idx, j, current_zone);
                if (zone.has_value())
                {
                    current_zone = zone;
                    cluster.push_back(j);
                    bfs_queue.push(j);
                }
            }
        }
        for (std::size_t k = 1; k < cluster.size(); ++k)
        {
            QS_DEF(Array<int>, mapping);
            if (_components[cluster[k]].first)
            {
                _components[i].first->mergeWithMolecule(*_components[cluster[k]].first, &mapping, 0);
                _components[cluster[k]].first.reset();
            }
        }
    }
    _components.erase(std::remove_if(_components.begin(), _components.end(), [](auto& p) { return !p.first; }), _components.end());
    _moleculeCount = (int)_components.size();
}

std::optional<std::pair<int, int>> ReactionMultistepDetector::isMergeable(size_t mol_idx1, size_t mol_idx2, std::optional<std::pair<int, int>> current_zone)
{
    auto& mdi = _mol_distances[mol_idx1];
    auto dist_it = mdi.distances_map.find(mol_idx2);
    if (dist_it != mdi.distances_map.end() && dist_it->second < LayoutOptions::DEFAULT_BOND_LENGTH * 2)
    {
        // collect surrounding zones for both molecules
        auto zone1 = findMaxSpecialZone(mol_idx1);
        auto zone2 = findMaxSpecialZone(mol_idx2);
        // no zone - no reason to merge
        if (zone1.has_value())
        {
            // if both molecules has the same zone and section - merge
            // we never merge molecules with different sections of the same zone
            if (zone1.value().first == zone2.value().first)
                return zone1.value().second == zone2.value().second ? zone1 : std::nullopt;
            if (!current_zone.has_value())
                current_zone = zone1;
        }
        else if (!current_zone.has_value())
            return std::nullopt;

        auto zidx = current_zone.value().first;
        // if molecules have different zones - check if they are mergeable
        auto& hull1 = _components[mol_idx1].second;
        auto& hull2 = _components[mol_idx2].second;

        const auto& coords = _zones[zidx].origin_coordinates;
        // check if both molecules are on the same zone continuation
        switch (_zones[zidx].zone_type)
        {
        case ZoneType::EPlus:
            if ((doesVerticalLineIntersectPolygon(coords[0].x, hull1) && doesVerticalLineIntersectPolygon(coords[0].x, hull2)) ||
                (doesHorizontalLineIntersectPolygon(coords[0].x, hull1) && doesHorizontalLineIntersectPolygon(coords[0].x, hull2)))
                return zone1;
            break;
        case ZoneType::EArrow:
            if ((doesRayIntersectPolygon(coords[0], coords[1], hull1) && doesRayIntersectPolygon(coords[0], coords[1], hull2)) ||
                (doesRayIntersectPolygon(coords[1], coords[0], hull1) && doesRayIntersectPolygon(coords[1], coords[0], hull2)))
                return zone1;
            break;
        case ZoneType::EPathWay: {
            auto c_it = coords.begin();
            const Vec2f& head = *c_it++;
            const Vec2f& spine_beg = *c_it++;
            const Vec2f& spine_end = *c_it++;
            Vec2f tail(spine_beg.x, head.y);
            if (doesRayIntersectPolygon(tail, head, hull1) && doesRayIntersectPolygon(tail, head, hull2))
                return zone1;
            for (; c_it != coords.end(); ++c_it)
            {
                Vec2f tail_start(spine_end.x, c_it->y);
                if (doesRayIntersectPolygon(tail_start, *c_it, hull1) && doesRayIntersectPolygon(tail_start, *c_it, hull2))
                    return zone1;
            }
        }
        break;
        }
    }
    return std::nullopt;
}

bool ReactionMultistepDetector::checkForOppositeSections(ZoneType zt, const std::unordered_set<int>& sections1, const std::unordered_set<int>& sections2)
{
    for (int section1 : sections1)
    {
        if (zt == ZoneType::EPathWay && section1 > 1) // pathway has only 2 opposite agents sections
            break;
        if (sections2.count(section1 ^ 1))
            return true;
    }
    return false;
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
    createSpecialZones();
    std::list<std::unordered_set<int>> s_neighbors;
    getSGroupAtoms(_bmol, s_neighbors);
    _moleculeCount = _bmol.countComponents(s_neighbors);
    for (int i = 0; i < _moleculeCount; ++i)
    {
        auto component = extractComponent(i);
        auto hull = component->getConvexHull(Vec2f(LayoutOptions::DEFAULT_BOND_LENGTH, LayoutOptions::DEFAULT_BOND_LENGTH));
        _components.emplace_back(std::move(component), hull);
    }

    collectSortedDistances();
    mergeCloseComponents();
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
    // TODO: add bounding box for plus
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

    // sort to find the closest to the plus
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
        // TODO: distances limit
        min_distance_h = std::min(std::fabs(rights_row_it->first - plus_pos_x.first), std::fabs(plus_pos_x.first - lefts_row_it->first));
        connection.second = lefts_row_it->second;
        connection.first = rights_row_it->second;
        result = _reaction_components[connection.first].component_type == ReactionComponent::MOLECULE &&
                 _reaction_components[connection.second].component_type == ReactionComponent::MOLECULE;
    }

    if (tops_col_it != tops_col.end() && bottoms_col_it != bottoms_col.end())
    {
        // TODO: distances limit
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

    if (name.size())
    {
        int id = sr.properties.insert(PathwayLayout::REACTION_NAME);
        sr.properties.value(id).readString(name.c_str(), true);
    }

    if (condition.size())
    {
        int id = sr.properties.insert(PathwayLayout::REACTION_CONDITIONS);
        sr.properties.value(id).readString(condition.c_str(), true);
    }
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