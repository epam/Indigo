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

ReactionMultistepDetector::ReactionMultistepDetector(BaseMolecule& bmol, const LayoutOptions& options)
    : _bmol(bmol), _moleculeCount(0), _layout_options(options)
{
    _reaction_margin_size =
        options.fontSize > EPSILON ? options.getMarginSizeInAngstroms() : LayoutOptions::DEFAULT_BOND_LENGTH * options.getMarginSizeInAngstroms();
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
        auto& comp = _merged_components[i];
        comp.mol->getBoundingBox(bbox, MIN_MOL_SIZE);
        mol_tops.emplace_back(bbox.top(), i);
        mol_bottoms.emplace_back(bbox.bottom(), i);
        mol_lefts.emplace_back(bbox.left(), i);
        mol_rights.emplace_back(bbox.right(), i);
        auto mol = std::unique_ptr<BaseMolecule>(comp.mol->neu());
        mol->clone(*comp.mol);
        _reaction_components.emplace_back(ReactionComponent::MOLECULE, bbox, i, std::move(mol));
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

    std::list<MolSumm> component_summ_blocks_list;

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
                component_summ_blocks_list.emplace_back(bbox); // add merged boxes of both components
                auto& last_sb = component_summ_blocks_list.back();
                last_sb.indexes.push_back(plus_connection.first);
                last_sb.indexes.push_back(plus_connection.second);

                rc_connection.first.summ_block_idx = ReactionComponent::CONNECTED; // mark as connected
                rc_connection.second.summ_block_idx = ReactionComponent::CONNECTED;
                auto last_it = std::prev(component_summ_blocks_list.end());
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
                component_summ_blocks_list.erase(second_block_it); // remove block_second
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
    for (auto& csb : component_summ_blocks_list)
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

// TODO: we can't avoid the calculation like this, but it's optimizable to N(log(N)) complexity
void ReactionMultistepDetector::collectSortedDistances()
{
    _mol_distances.resize(_moleculeCount);
    for (int i = 0; i < _moleculeCount; ++i)
    {
        auto& mdi = _mol_distances[i];
        for (int j = i + 1; j < _moleculeCount; ++j)
        {
            float dist = computeConvexDistance(_components[i].hull, _components[j].hull);
            auto& mdj = _mol_distances[j];
            if (dist < LayoutOptions::DEFAULT_BOND_LENGTH * 2)
            {
                mdi.sorted_distances.emplace_back(j, dist);
                mdj.sorted_distances.emplace_back(i, dist);
            }
            mdj.distances_map.emplace(i, dist);
            mdi.distances_map.emplace(j, dist);
        }
        std::sort(mdi.sorted_distances.begin(), mdi.sorted_distances.end(), [](auto& lhs, auto& rhs) { return lhs.second < rhs.second; });
    }
}

void ReactionMultistepDetector::createSpecialZones()
{
    for (int i = 0; i < _bmol.meta().getMetaCount(ReactionArrowObject::CID); ++i)
    {
        auto& arrow = (const ReactionArrowObject&)_bmol.meta().getMetaObject(ReactionArrowObject::CID, i);
        addArrowZones(arrow.getTail(), arrow.getHead());
    }

    for (int i = 0; i < _bmol.meta().getMetaCount(ReactionPlusObject::CID); ++i)
    {
        auto& plus = (const ReactionPlusObject&)_bmol.meta().getMetaObject(ReactionPlusObject::CID, i);
        const auto& plus_pos = plus.getPos();
        addPlusZones(plus_pos);
    }

    for (int i = 0; i < _bmol.meta().getMetaCount(ReactionMultitailArrowObject::CID); ++i)
    {
        const auto& multi = (const ReactionMultitailArrowObject&)_bmol.meta().getMetaObject(ReactionMultitailArrowObject::CID, i);
        const auto& tails = multi.getTails();
        const auto& head = multi.getHead();
        const auto& spine_beg = multi.getSpineBegin();
        const auto& spine_end = multi.getSpineEnd();
        std::vector<Vec2f> tails_vec(tails.begin(), tails.end());
        addPathwayZones(head, spine_beg, spine_end, tails_vec);
    }
}

void ReactionMultistepDetector::addPlusZones(const Vec2f& pos)
{
    Rect2f bbox(pos - PLUS_BBOX_SHIFT / 2, pos + PLUS_BBOX_SHIFT / 2);
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

    // the order of sections is important. Odd and even indexes are opposite. 0 - left, 1 - right, 2 - top, 3 - bottom.
    // this used ReactionMultistepDetector::isMergeable during the switch between opposite sections with ^1 operation.
    szd.zone_sections.push_back(left);
    szd.zone_sections.push_back(right);
    szd.zone_sections.push_back(top);
    szd.zone_sections.push_back(bottom);
    _zones.push_back(szd);
}

std::vector<Vec2f> ReactionMultistepDetector::getArrowZone(const Vec2f& tail, const Vec2f& head)
{
    auto dir = head - tail;
    float length = dir.length();
    dir.normalize();
    Vec2f nB = {-dir.y, dir.x};
    Vec2f nT = {dir.y, -dir.x};
    std::vector<Vec2f> result;
    Vec2f pos = head + nT * ARROW_DETECTION_DISTANCE;
    result.push_back(pos);
    pos += dir * (ARROW_DETECTION_DISTANCE * 2 + _reaction_margin_size);
    result.push_back(pos);
    pos += nB * ARROW_DETECTION_DISTANCE * 2;
    result.push_back(pos);
    pos -= dir * (ARROW_DETECTION_DISTANCE * 2 + _reaction_margin_size);
    result.push_back(pos);
    pos += nT * ARROW_DETECTION_DISTANCE * 2;
    result.push_back(pos);
    return result;
}

void ReactionMultistepDetector::addArrowZones(const Vec2f& tail, const Vec2f& head)
{
    float dx = head.x - tail.x, dy = head.y - tail.y;
    float length = std::hypot(dx, dy), half_length = length * 0.5f;
    Vec2f dir = {dx, dy};
    dir.normalize();
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
    // the order of sections is important. Odd and even indexes are opposite. 0 - left, 1 - right, 2 - top, 3 - bottom
    // this used ReactionMultistepDetector::isMergeable during the switch between opposite sections with ^1 operation.
    szd.zone_sections.push_back(left);
    szd.zone_sections.push_back(right);
    szd.zone_sections.push_back(top);
    szd.zone_sections.push_back(bottom);
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
    // the order of sections is important. Odd and even indexes are opposite. 0 - top, 1 - bottom
    // this used ReactionMultistepDetector::isMergeable during the switch between opposite sections with ^1 operation.
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
    auto& hull = _components[mol_idx].hull;
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

// Find in which detection zone the molecule is primarily located.
std::optional<std::pair<int, int>> ReactionMultistepDetector::findMaxSpecialZone(size_t mol_idx, std::map<int, std::set<int>>& zones)
{
    std::optional<std::pair<int, int>> result{};
    float max_containment = 0.0f;
    auto& hull = _components[mol_idx].hull;
    for (int i = 0; i < static_cast<int>(_zones.size()); ++i)
    {
        auto& zone = _zones[i];
        for (int j = 0; j < static_cast<int>(zone.zone_sections.size()); ++j)
        {
            auto& section = zone.zone_sections[j];
            float cont = computeConvexContainment(hull, section);
            if (cont > max_containment)
            {
                max_containment = cont;
                result = {i, j};
            }
            if (cont > 0.0f)
            {
                auto zone_it = zones.find(i);
                if (zone_it != zones.end())
                    zone_it->second.insert(j);
                else
                    zones.insert({i, {j}});
            }
        }
    }
    return result;
}

void ReactionMultistepDetector::mergeCloseComponents()
{
    for (size_t i = 0; i < _mol_distances.size(); ++i)
    {
        auto& mdi = _mol_distances[i];
        if (mdi.sorted_distances.empty() || !_components[i].mol)
            continue;
        std::queue<std::pair<std::size_t, std::optional<std::pair<int, int>>>> bfs_queue;
        std::vector<std::size_t> cluster;
        bfs_queue.emplace(i, std::nullopt);
        cluster.push_back(i);
        while (!bfs_queue.empty())
        {
            auto qel = bfs_queue.front();
            auto& mdj = _mol_distances[qel.first];
            bfs_queue.pop();
            for (auto& sd : mdj.sorted_distances)
            {
                auto j = sd.first;
                if (!_components[j].mol || std::find(cluster.begin(), cluster.end(), j) != cluster.end())
                    continue;
                auto zone = isMergeable(qel.first, j, qel.second);
                if (zone.has_value())
                {
                    cluster.push_back(j);
                    bfs_queue.emplace(j, zone);
                }
            }
        }
        for (std::size_t k = 1; k < cluster.size(); ++k)
        {
            QS_DEF(Array<int>, mapping);
            if (_components[cluster[k]].mol)
            {
                _components[i].mol->mergeWithMolecule(*_components[cluster[k]].mol, &mapping, 0);
                _components[cluster[k]].idx = (int)i;
                _components[cluster[k]].mol.reset();
            }
        }
    }

    for (int i = 0; i < _moleculeCount; ++i)
    {
        if (_components[i].mol)
        {
            _merged_components.emplace_back(std::move(_components[i].mol), _components[i].hull, (int)_merged_components.size());
            _merged_components.back().mapped_idx = i;
            _components[i].mapped_idx = (int)_merged_components.size() - 1;
        }
    }
    _moleculeCount = (int)_merged_components.size();
}

void dumpHull(std::string name, const std::vector<Vec2f>& hull)
{
    std::cout << name << std::endl;
    for (auto& v : hull)
        std::cout << v.x << "," << v.y << std::endl;
}

std::optional<std::pair<int, int>> ReactionMultistepDetector::isMergeable(size_t mol_idx1, size_t mol_idx2, std::optional<std::pair<int, int>>& current_zone)
{
    auto& mdi = _mol_distances[mol_idx1];
    auto dist_it = mdi.distances_map.find(mol_idx2);
    if (dist_it != mdi.distances_map.end() && dist_it->second < LayoutOptions::DEFAULT_BOND_LENGTH * 2)
    {
        // collect surrounding zones for both molecules
        std::map<int, std::set<int>> other_zones1;
        std::map<int, std::set<int>> other_zones2;

        auto zone1 = findMaxSpecialZone(mol_idx1, other_zones1);
        auto zone2 = findMaxSpecialZone(mol_idx2, other_zones2);

        // no zone - no reason to merge
        if (zone1.has_value())
        {
            // if both molecules has the same zone and section - merge
            // we never merge molecules with different sections of the same zone
            if (zone2.has_value())
            {
                // disable catalysts merging
                if ((_zones[zone1.value().first].zone_type == ZoneType::EArrow && zone1.value().second > 1) ||
                    (_zones[zone2.value().first].zone_type == ZoneType::EArrow && zone2.value().second > 1))
                    return std::nullopt;

                // if both molecules are on the same zone - check if they are mergeable

                // check if there are opposite sections
                std::map<int, std::set<int>> comm_zones;
                std::set_intersection(other_zones1.begin(), other_zones1.end(), other_zones2.begin(), other_zones2.end(),
                                      std::inserter(comm_zones, comm_zones.begin()), [](auto& a, auto& b) { return a.first < b.first; });

                for (auto& cz : comm_zones)
                {
                    auto it_oz2 = other_zones2.find(cz.first);
                    if (it_oz2 != other_zones2.end())
                    {
                        for (auto& section : cz.second)
                        {
                            // meaning of section ^ 1: the sections are ELeft = 0, ERight, ETop, EBottom. ^ 1 switches between ELeft and ERight, ETop and
                            // EBottom. left = 0, right = 1, left = 2, right = 3. ^ 1 operation converts left->right, right->left, top->bottom, bottom->top.
                            if ((_zones[cz.first].zone_type == ZoneType::EPathWay && section > 1) || it_oz2->second.count(section ^ 1))
                                return std::nullopt;
                        }
                    }
                }

                // different zone types are not mergeable
                if (_zones[zone1.value().first].zone_type != _zones[zone2.value().first].zone_type && comm_zones.empty())
                    return std::nullopt;
            }

            if (!current_zone.has_value())
                current_zone = zone1;
        }
        else if (!current_zone.has_value())
            return std::nullopt;

        auto zidx = current_zone.value().first;
        // if molecules have different zones - check if they are mergeable
        auto& hull1 = _components[mol_idx1].hull;
        auto& hull2 = _components[mol_idx2].hull;

        const auto& coords = _zones[zidx].origin_coordinates;
        // check if both molecules are on the same zone continuation
        switch (_zones[zidx].zone_type)
        {
        case ZoneType::EPlus:
            if (((current_zone->second == (int)PlusSectionCode::ETop || current_zone->second == (int)PlusSectionCode::EBottom) &&
                 doesVerticalLineIntersectPolygon(coords[0].x, hull1) && doesVerticalLineIntersectPolygon(coords[0].x, hull2)) ||
                ((current_zone->second == (int)PlusSectionCode::ELeft || current_zone->second == (int)PlusSectionCode::ERight) &&
                 doesHorizontalLineIntersectPolygon(coords[0].y, hull1) && doesHorizontalLineIntersectPolygon(coords[0].y, hull2)))
                return zone1 ? zone1 : current_zone;
            break;
        case ZoneType::EArrow:
            if ((doesRayIntersectPolygon(coords[0], coords[1], hull1) && doesRayIntersectPolygon(coords[0], coords[1], hull2)) ||
                (doesRayIntersectPolygon(coords[1], coords[0], hull1) && doesRayIntersectPolygon(coords[1], coords[0], hull2)))
                return zone1 ? zone1 : current_zone;
            break;
        case ZoneType::EPathWay: {
            auto c_it = coords.begin();
            const Vec2f& head = *c_it++;
            const Vec2f& spine_beg = *c_it++;
            const Vec2f& spine_end = *c_it++; // we could skip it because only x is used which is the same as spine_beg
            Vec2f tail(spine_beg.x, head.y);
            if (doesRayIntersectPolygon(tail, head, hull1) && doesRayIntersectPolygon(tail, head, hull2))
                return zone1 ? zone1 : current_zone;
            for (; c_it != coords.end(); ++c_it)
            {
                Vec2f tail_start(spine_end.x, c_it->y);
                if (doesRayIntersectPolygon(tail_start, *c_it, hull1) && doesRayIntersectPolygon(tail_start, *c_it, hull2))
                    return zone1 ? zone1 : current_zone;
            }
        }
        break;
        }
    }
    return std::nullopt;
}

void ReactionMultistepDetector::sortSummblocks()
{
    // Create a list of original indices
    std::vector<int> indices(_component_summ_blocks.size());
    std::iota(indices.begin(), indices.end(), 0);

    // Sort the indices based on reaction_idx
    std::stable_sort(indices.begin(), indices.end(), [&](int a, int b) -> bool {
        auto& csba = _component_summ_blocks[a];
        auto& csbb = _component_summ_blocks[b];
        return csba.reaction_idx < csbb.reaction_idx;
    });

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
    // Update reaction components with new indices
    for (auto& rc : _reaction_components)
    {
        if (rc.summ_block_idx >= 0 && rc.summ_block_idx < static_cast<int>(old_to_new.size()))
            rc.summ_block_idx = old_to_new[rc.summ_block_idx];
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
        Rect2f bbox;
        component->getBoundingBox(bbox, Vec2f(LayoutOptions::DEFAULT_BOND_LENGTH, LayoutOptions::DEFAULT_BOND_LENGTH));
        // auto hull = component->getConvexHull(Vec2f(LayoutOptions::DEFAULT_BOND_LENGTH, LayoutOptions::DEFAULT_BOND_LENGTH));
        std::vector<Vec2f> hull;
        hull.push_back(bbox.leftTop());
        hull.push_back(bbox.leftBottom());
        hull.push_back(bbox.rightBottom());
        hull.push_back(bbox.rightTop());
        hull.push_back(bbox.leftTop());
        _components.emplace_back(std::move(component), hull, i);
    }

    collectSortedDistances();
    mergeCloseComponents();
    createSummBlocks();
    bool has_multistep = mapReactionComponents();
    bool has_multitail = mapMultitailReactionComponents();
    sortSummblocks();
    mergeUndefinedComponents();
    return has_multitail ? ReactionType::EPathwayReaction : (has_multistep ? ReactionType::EMutistepReaction : ReactionType::ESimpleReaction);
}

bool ReactionMultistepDetector::mapReactionComponents()
{
    int arrow_count = _bmol.meta().getMetaCount(ReactionArrowObject::CID);
    if (arrow_count > 0)
    {
        for (int reaction_index = 0; reaction_index < arrow_count; ++reaction_index)
        {
            auto& arrow = (const ReactionArrowObject&)_bmol.meta().getMetaObject(ReactionArrowObject::CID, reaction_index);
            int arrow_type = arrow.getArrowType();
            bool reverseReactionOrder = arrow_type == ReactionArrowObject::ERetrosynthetic;
            const Vec2f& arr_begin = !reverseReactionOrder ? arrow.getTail() : arrow.getHead();
            const Vec2f& arr_end = !reverseReactionOrder ? arrow.getHead() : arrow.getTail();

            auto right_arrow_zone = getArrowZone(arr_begin, arr_end);
            auto left_arrow_zone = getArrowZone(arr_end, arr_begin);

            float min_dist_prod = -1, min_dist_reac = -1;
            int idx_cs_min_prod = -1, idx_cs_min_reac = -1;
            for (int index_cs = 0; index_cs < static_cast<int>(_component_summ_blocks.size()); ++index_cs)
            {
                auto& csb = _component_summ_blocks[index_cs];
                std::array<int, KProductArea + 1> sides{};
                int side = -1;
                for (auto rc_idx : csb.indexes)
                {
                    auto& rc = _reaction_components[rc_idx];
                    if (rc.component_type == ReactionComponent::MOLECULE)
                        side = getMoleculeSide(arrow, *rc.molecule, sides);
                }

                std::vector<Vec2f> csb_bbox = {csb.bbox.leftTop(), csb.bbox.rightTop(), csb.bbox.rightBottom(), csb.bbox.leftBottom(), csb.bbox.leftTop()};
                if (csb.bbox.rayIntersectsRect(arr_end, arr_begin) ||
                    (arrow_count == 1 && side == KProductArea && convexPolygonsIntersect(csb_bbox, right_arrow_zone)))
                {
                    float dist = csb.bbox.pointDistance(arr_end);
                    if (min_dist_prod < 0 || dist < min_dist_prod)
                    {
                        min_dist_prod = dist;
                        idx_cs_min_prod = index_cs;
                    }
                }
                else if (csb.bbox.rayIntersectsRect(arr_begin, arr_end) ||
                         (arrow_count == 1 && side == KReactantArea && convexPolygonsIntersect(csb_bbox, left_arrow_zone)))
                {
                    float dist = csb.bbox.pointDistance(arr_begin);
                    if (min_dist_reac < 0 || dist < min_dist_reac)
                    {
                        min_dist_reac = dist;
                        idx_cs_min_reac = index_cs;
                    }
                }
                else
                {
                    for (auto rc_idx : csb.indexes)
                    {
                        auto& rc = _reaction_components[rc_idx];
                        if (rc.component_type == ReactionComponent::MOLECULE)
                        {
                            auto& top_zone = _zones[reaction_index].zone_sections[2];
                            auto& bottom_zone = _zones[reaction_index].zone_sections[3];
                            if (convexPolygonsIntersect(_merged_components[rc_idx].hull, top_zone) ||
                                convexPolygonsIntersect(_merged_components[rc_idx].hull, bottom_zone))
                                csb.role = BaseReaction::CATALYST;
                        }
                    }
                }
            }

            // TODO: add upper limit
            if (min_dist_prod >= 0 || min_dist_reac >= 0) // if both ends present
            {
                auto& rc_arrow = _reaction_components[_moleculeCount + _bmol.meta().getMetaCount(ReactionPlusObject::CID) + reaction_index];
                rc_arrow.summ_block_idx = ReactionComponent::CONNECTED; // mark arrow as connected

                if (min_dist_prod >= 0)
                {
                    auto& csb_min_prod = _component_summ_blocks[idx_cs_min_prod];
                    if (csb_min_prod.role == BaseReaction::UNDEFINED)
                        csb_min_prod.role = BaseReaction::PRODUCT;
                    else if (csb_min_prod.role == BaseReaction::REACTANT)
                        csb_min_prod.role = BaseReaction::INTERMEDIATE;
                    if (min_dist_reac >= 0)
                        csb_min_prod.arrows_from.push_back(idx_cs_min_reac);
                    csb_min_prod.reaction_idx = reaction_index;
                }

                if (min_dist_reac >= 0)
                {
                    auto& csb_min_reac = _component_summ_blocks[idx_cs_min_reac];
                    if (csb_min_reac.role == BaseReaction::UNDEFINED)
                        csb_min_reac.role = BaseReaction::REACTANT;
                    else if (csb_min_reac.role == BaseReaction::PRODUCT)
                        csb_min_reac.role = BaseReaction::INTERMEDIATE;
                    if (min_dist_prod >= 0)
                        csb_min_reac.arrows_to.push_back(idx_cs_min_prod);
                }
            }
        }
        return arrow_count > 1;
    }
    return false;
}

bool ReactionMultistepDetector::mapMultitailReactionComponents()
{
    int pathway_count = _bmol.meta().getMetaCount(ReactionMultitailArrowObject::CID);

    bool bad_pathway = pathway_count == 0;

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
                float dist = (csb.bbox.center() - arr_end).length();
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
                    auto dist = (csb.bbox.center() - arr_begin).length();
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
            if (reac.first >= 0)
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
            }
            else
                bad_pathway = true;
        }
    }
    return !bad_pathway;
}

void ReactionMultistepDetector::collectUndefinedDistances(const std::vector<std::pair<size_t, Rect2f>>& component_bboxes,
                                                          const std::vector<std::pair<size_t, Rect2f>>& undef_component_bboxes,
                                                          std::vector<MOL_DISTANCES_DESC>& undef_distances,
                                                          std::vector<MOL_DISTANCES_DESC>& undef_comp_distances)
{
    // collect undef_distances and undef_comp_distances
    undef_distances.clear();
    undef_comp_distances.clear();
    undef_distances.resize(undef_component_bboxes.size());
    undef_comp_distances.resize(undef_component_bboxes.size());
    for (size_t i = 0; i < undef_component_bboxes.size(); ++i)
    {
        // collect distances to other undefined components
        auto& mdi = undef_distances[i];
        for (size_t j = i + 1; j < undef_component_bboxes.size(); ++j)
        {
            float dist = undef_component_bboxes[i].second.distanceTo(undef_component_bboxes[j].second);
            auto& mdj = undef_distances[j];
            if (dist < LayoutOptions::DEFAULT_BOND_LENGTH * 2)
            {
                mdi.sorted_distances.emplace_back(j, dist);
                mdj.sorted_distances.emplace_back(i, dist);
            }
            mdj.distances_map.emplace(i, dist);
            mdi.distances_map.emplace(j, dist);
        }
        std::sort(mdi.sorted_distances.begin(), mdi.sorted_distances.end(), [](auto& lhs, auto& rhs) { return lhs.second < rhs.second; });

        // collect distances to detected components
        auto& mdic = undef_comp_distances[i];
        for (size_t j = 0; j < component_bboxes.size(); ++j)
        {
            float dist = undef_component_bboxes[i].second.distanceTo(component_bboxes[j].second);
            mdic.distances_map.emplace(j, dist);
            if (dist < LayoutOptions::DEFAULT_BOND_LENGTH * 2)
                mdic.sorted_distances.emplace_back(j, dist);
        }
        std::sort(mdic.sorted_distances.begin(), mdic.sorted_distances.end(), [](auto& lhs, auto& rhs) { return lhs.second < rhs.second; });
    }
}

void ReactionMultistepDetector::mergeUndefinedComponents()
{
    std::vector<std::pair<size_t, Rect2f>> component_bboxes;
    std::vector<std::pair<size_t, Rect2f>> undef_component_bboxes;
    MOL_DISTANCES mol_distances;

    for (size_t i = 0; i < _reaction_components.size(); ++i)
    {
        auto& rc = _reaction_components[i];
        if (rc.component_type == ReactionComponent::MOLECULE)
        {
            auto& csb = _component_summ_blocks[rc.summ_block_idx];
            Rect2f bbox;
            rc.molecule->getBoundingBox(bbox);
            if (csb.role == BaseReaction::UNDEFINED)
                undef_component_bboxes.emplace_back(i, bbox);
            else
                component_bboxes.emplace_back(i, bbox);
        }
    }

    bool has_merges = false;
    std::vector<MOL_DISTANCES_DESC> undef_distances;
    std::vector<MOL_DISTANCES_DESC> undef_comp_distances;
    std::vector<MERGE_CANDIDATE> merge_list_candidates;

    do
    {
        merge_list_candidates.clear();
        collectUndefinedDistances(component_bboxes, undef_component_bboxes, undef_distances, undef_comp_distances);

        // find minimal distance to other undefined components
        std::optional<MERGE_CANDIDATE> undef_min_dist;

        // iterate over all undefined components
        for (size_t i = 0; i < undef_component_bboxes.size(); ++i)
        {
            auto& mdi_comp = undef_comp_distances[i];
            auto& mdi_undef = undef_distances[i];
            // check if we have closest detected component
            if (mdi_comp.sorted_distances.size())
            {
                // check if we have a merge candidate
                if (mdi_undef.sorted_distances.empty() || mdi_comp.sorted_distances.front().second < mdi_undef.sorted_distances.front().second)
                    merge_list_candidates.emplace_back(i, mdi_comp.sorted_distances.front().first, mdi_comp.sorted_distances.front().second);
                else if (mdi_undef.sorted_distances.size() &&
                         (!undef_min_dist.has_value() || undef_min_dist.value().distance > mdi_undef.sorted_distances.front().second))
                    undef_min_dist = MERGE_CANDIDATE(i, mdi_comp.sorted_distances.front().first, mdi_undef.sorted_distances.front().second);
            }
        }

        // check if we have a merge candidate which overrides merge_list_candidates
        if (merge_list_candidates.empty() && undef_min_dist.has_value())
            merge_list_candidates.emplace_back(undef_min_dist.value());

        // merge all merge_list_candidates
        std::unordered_set<size_t> undefs_delete;
        has_merges = merge_list_candidates.size();
        for (auto& mc : merge_list_candidates)
        {
            undefs_delete.emplace(mc.undef_idx);
            auto& undef_bbox = undef_component_bboxes[mc.undef_idx];
            auto& comp_bbox = component_bboxes[mc.comp_idx];
            auto& rc_undef = _reaction_components[undef_bbox.first];
            auto& rc_target = _reaction_components[comp_bbox.first];
            if (rc_undef.molecule && rc_target.molecule)
            {
                rc_target.molecule->mergeWithMolecule(*rc_undef.molecule, nullptr, 0);
                _components[undef_bbox.first].idx = _components[comp_bbox.first].idx;
                rc_undef.molecule.reset();
                comp_bbox.second.extend(undef_bbox.second);
            }
        }
        size_t index = 0;
        undef_component_bboxes.erase(std::remove_if(undef_component_bboxes.begin(), undef_component_bboxes.end(),
                                                    [&](std::pair<size_t, Rect2f>&) { return undefs_delete.count(index++); }),
                                     undef_component_bboxes.end());

    } while (has_merges);
}

bool ReactionMultistepDetector::findPlusNeighbours(const Vec2f& plus_pos, const FLOAT_INT_PAIRS& mol_tops, const FLOAT_INT_PAIRS& mol_bottoms,
                                                   const FLOAT_INT_PAIRS& mol_lefts, const FLOAT_INT_PAIRS& mol_rights, std::pair<int, int>& connection)
{
    auto plus_pos_right = std::make_pair(plus_pos.x + PLUS_BBOX_SHIFT.x, -1);
    auto plus_pos_left = std::make_pair(plus_pos.x - PLUS_BBOX_SHIFT.x, -1);
    auto plus_pos_top = std::make_pair(plus_pos.y + PLUS_BBOX_SHIFT.y, -1);
    auto plus_pos_bottom = std::make_pair(plus_pos.y - PLUS_BBOX_SHIFT.y, -1);

    auto pair_comp_asc = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.first > a.first; };
    auto pair_comp_des = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.first < a.first; };
    auto pair_comp_mol_asc = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.second > a.second; };
    // look for mols where top > plus_pos_bottom
    auto tops_above_it = std::upper_bound(mol_tops.begin(), mol_tops.end(), plus_pos_bottom, pair_comp_asc);
    // look for mols where bottom < plus_pos_top
    auto bottoms_below_it = std::upper_bound(mol_bottoms.begin(), mol_bottoms.end(), plus_pos_top, pair_comp_des);
    // look for mols where right > plus_pos_left
    auto rights_after_it = std::upper_bound(mol_rights.begin(), mol_rights.end(), plus_pos_left, pair_comp_asc);
    // look for mols where left < plus_pos_right
    auto lefts_before_it = std::upper_bound(mol_lefts.begin(), mol_lefts.end(), plus_pos_right, pair_comp_des);

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
        auto dir = tb_box.center() - plus_pos;
        if (_reaction_components[kvp.second].component_type == ReactionComponent::MOLECULE && std::fabs(dir.x) > std::fabs(dir.y))
        {
            rights_row.emplace_back(tb_box.right(), kvp.second);
            lefts_row.emplace_back(tb_box.left(), kvp.second);
        }
    }

    // collect top-bottom
    for (const auto& kvp : intersection_left_right)
    {
        auto& lr_box = _reaction_components[kvp.second].bbox;
        auto dir = lr_box.center() - plus_pos;
        if (_reaction_components[kvp.second].component_type == ReactionComponent::MOLECULE && std::fabs(dir.x) < std::fabs(dir.y))
        {
            tops_col.emplace_back(lr_box.top(), kvp.second);
            bottoms_col.emplace_back(lr_box.bottom(), kvp.second);
        }
    }

    // sort to find the closest to the plus
    std::sort(lefts_row.begin(), lefts_row.end(), pair_comp_asc);
    std::sort(rights_row.begin(), rights_row.end(), pair_comp_des);
    std::sort(tops_col.begin(), tops_col.end(), pair_comp_asc);
    std::sort(bottoms_col.begin(), bottoms_col.end(), pair_comp_des);

    auto rights_row_it = std::upper_bound(rights_row.begin(), rights_row.end(), plus_pos_right, pair_comp_des);
    auto lefts_row_it = std::upper_bound(lefts_row.begin(), lefts_row.end(), plus_pos_left, pair_comp_asc);

    auto tops_col_it = std::upper_bound(tops_col.begin(), tops_col.end(), plus_pos_top, pair_comp_asc);
    auto bottoms_col_it = std::upper_bound(bottoms_col.begin(), bottoms_col.end(), plus_pos_bottom, pair_comp_des);

    float min_distance_h = 0, min_distance_v = 0;

    bool result = false;

    if (rights_row_it != rights_row.end() && lefts_row_it != lefts_row.end() && lefts_row_it->second != rights_row_it->second)
    {
        // TODO: distances limit?
        min_distance_h = std::min(std::fabs(rights_row_it->first - plus_pos_right.first), std::fabs(plus_pos_left.first - lefts_row_it->first));
        connection.second = lefts_row_it->second;
        connection.first = rights_row_it->second;
        result = _reaction_components[connection.first].component_type == ReactionComponent::MOLECULE &&
                 _reaction_components[connection.second].component_type == ReactionComponent::MOLECULE;
    }

    if (tops_col_it != tops_col.end() && bottoms_col_it != bottoms_col.end() && tops_col_it->second != bottoms_col_it->second)
    {
        // TODO: distances limit?
        min_distance_v = std::min(std::fabs(tops_col_it->first - plus_pos_top.first), std::fabs(bottoms_col_it->first - plus_pos_bottom.first));
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
            if (rc.molecule && !copied_components.count(idx))
            {
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
                case BaseReaction::CATALYST:
                    copied_components.emplace(idx, rxn.addCatalystCopy(*rc.molecule, 0, 0));
                    break;
                default:
                    break;
                }
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

int ReactionMultistepDetector::getMoleculeSide(const ReactionArrowObject& arrow, BaseMolecule& mol, std::array<int, KProductArea + 1>& sides)
{
    bool reverseReactionOrder = arrow.getArrowType() == ReactionArrowObject::ERetrosynthetic;
    bool has_element = false;
    for (int idx = mol.vertexBegin(); idx < mol.vertexEnd(); idx = mol.vertexNext(idx))
    {
        Vec3f& pt3d = mol.getAtomXyz(idx);
        Vec2f pt(pt3d.x, pt3d.y);
        int side = !reverseReactionOrder ? getPointSide(pt, arrow.getTail(), arrow.getHead()) : getPointSide(pt, arrow.getHead(), arrow.getTail());
        sides[side]++;
        has_element = true;
    }
    return has_element ? (int)std::distance(sides.begin(), std::max_element(sides.begin(), sides.end())) : -1;
}

void ReactionMultistepDetector::constructSimpleArrowReaction(BaseReaction& rxn)
{
    enum RecordIndexes
    {
        BBOX_IDX = 0,
        FRAGMENT_TYPE_IDX,
        MOLECULE_IDX
    };

    if (rxn.meta().getMetaCount(ReactionArrowObject::CID))
    {
        auto& arrow = (const ReactionArrowObject&)rxn.meta().getMetaObject(ReactionArrowObject::CID, 0);
        bool reverseReactionOrder = arrow.getArrowType() == ReactionArrowObject::ERetrosynthetic;

        if (reverseReactionOrder)
        {
            rxn.setIsRetrosyntetic();
        }

        for (int i = 0; i < rxn.meta().getMetaCount(SimpleTextObject::CID); ++i)
        {
            auto& text = (const SimpleTextObject&)rxn.meta().getMetaObject(SimpleTextObject::CID, i);
            Rect2f bbox(Vec2f(text._pos.x, text._pos.y), Vec2f(text._pos.x, text._pos.y)); // change to real text box later
            _reaction_components.emplace_back(ReactionComponent::TEXT, bbox, i, nullptr);
        }

        int text_meta_idx = 0;

        std::vector<std::pair<int, uint8_t>> undefined_components;
        std::vector<std::pair<int, int>> product_components, reactant_components;

        for (auto& csb : _component_summ_blocks)
        {
            switch (csb.role)
            {
            case BaseReaction::PRODUCT: {
                for (auto idx : csb.indexes)
                {
                    auto& rc = _reaction_components[idx];
                    product_components.emplace_back(idx, rxn.addProductCopy(*rc.molecule, 0, 0));
                }
            }
            break;
            case BaseReaction::REACTANT: {
                for (auto idx : csb.indexes)
                {
                    auto& rc = _reaction_components[idx];
                    reactant_components.emplace_back(idx, rxn.addReactantCopy(*rc.molecule, 0, 0));
                }
            }
            break;
            // simple reaction does not have intermediates
            case BaseReaction::INTERMEDIATE: {
                for (auto idx : csb.indexes)
                {
                    auto& rc = _reaction_components[idx];
                    rxn.addIntermediateCopy(*rc.molecule, 0, 0);
                }
            }
            break;
            case BaseReaction::CATALYST: {
                for (auto idx : csb.indexes)
                {
                    auto& rc = _reaction_components[idx];
                    rxn.addCatalystCopy(*rc.molecule, 0, 0);
                }
            }
            break;
            // undefined components are not allowed
            case BaseReaction::UNDEFINED: {
                for (auto idx : csb.indexes)
                {
                    auto& rc = _reaction_components[idx];
                    if (rc.molecule)
                    {
                        std::array<int, KProductArea + 1> sides{};
                        auto side = getMoleculeSide(arrow, *rc.molecule, sides);
                        int rem_up_count = rc.molecule->vertexCount() - sides[KReagentUpArea];
                        int rem_down_count = rc.molecule->vertexCount() - sides[KReagentDownArea];

                        if (side > -1 && (sides[KReagentUpArea] > rem_up_count || sides[KReagentDownArea] > rem_down_count))
                            rxn.addCatalystCopy(*rc.molecule, 0, 0);
                        else
                            rxn.addUndefinedCopy(*rc.molecule, 0, 0);
                    }
                }
            }
            break;
            default:
                break;
            }
        }

        for (const auto& comp : _reaction_components)
        {
            switch (comp.component_type)
            {
            case ReactionComponent::TEXT: {
                const auto& bbox = comp.bbox;
                Vec2f pt(bbox.center());
                int side = !reverseReactionOrder ? getPointSide(pt, arrow.getTail(), arrow.getHead()) : getPointSide(pt, arrow.getHead(), arrow.getTail());
                if (side == KReagentUpArea || side == KReagentDownArea)
                {
                    rxn.addSpecialCondition(text_meta_idx, bbox);
                    break;
                }
                text_meta_idx++;
            }
            break;
            default:
                break;
            }
        }
    }
    else
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
            case BaseReaction::CATALYST: {
                for (auto idx : csb.indexes)
                {
                    auto& rc = _reaction_components[idx];
                    rxn.addCatalystCopy(*rc.molecule, 0, 0);
                }
            }
            break;
            case BaseReaction::UNDEFINED: {
                for (auto idx : csb.indexes)
                {
                    auto& rc = _reaction_components[idx];
                    if (rc.molecule)
                        rxn.addUndefinedCopy(*rc.molecule, 0, 0);
                }
            }
            }
        }
}
