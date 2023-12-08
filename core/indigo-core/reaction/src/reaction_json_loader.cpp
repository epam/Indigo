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

#include <algorithm>
#include <array>
#include <iterator>
#include <memory>
#include <string>
#include <tuple>
#include <vector>

#include "reaction/query_reaction.h"
#include "reaction/reaction.h"
#include "reaction/reaction_json_loader.h"

using namespace indigo;
using namespace rapidjson;

IMPL_ERROR(ReactionJsonLoader, "reaction KET loader");

ReactionJsonLoader::ReactionJsonLoader(Document& ket)
    : _loader(ket), _molecule(kArrayType), _prxn(nullptr), _pqrxn(nullptr), ignore_noncritical_query_features(false)
{
    ignore_bad_valence = false;
}

ReactionJsonLoader::~ReactionJsonLoader()
{
}

void ReactionJsonLoader::loadReaction(BaseReaction& rxn)
{
    _loader.stereochemistry_options = stereochemistry_options;
    _loader.ignore_noncritical_query_features = ignore_noncritical_query_features;
    _loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
    _loader.ignore_no_chiral_flag = ignore_no_chiral_flag;

    if (rxn.isQueryReaction())
        _pqrxn = &rxn.asQueryReaction();
    else
        _prxn = &rxn.asReaction();

    if (_prxn)
    {
        _pmol = &_mol;
        _loader.loadMolecule(_mol, true);
    }
    else if (_pqrxn)
    {
        _loader.loadMolecule(_qmol, true);
        _pmol = &_qmol;
    }
    else
        throw Error("unknown reaction type: %s", typeid(rxn).name());

    rxn.original_format = BaseMolecule::KET;

    rxn.meta().clone(_pmol->meta());
    _pmol->meta().resetMetaData();

    int arrow_count = rxn.meta().getMetaCount(KETReactionArrow::CID);
    if (arrow_count == 0)
        throw Error("No arrow in the reaction");

    if (arrow_count > 1)
        parseMultipleArrowReaction(rxn);
    else
        parseOneArrowReaction(rxn);
}

bool ReactionJsonLoader::findPlusNeighbours(const Vec2f& plus_pos, const FLOAT_INT_PAIRS& mol_tops, const FLOAT_INT_PAIRS& mol_bottoms,
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
        connection.first = lefts_row_it->second;
        connection.second = rights_row_it->second;
        result = _reaction_components[connection.first].component_type == ReactionComponent::MOLECULE &&
                 _reaction_components[connection.second].component_type == ReactionComponent::MOLECULE;
    }

    if (tops_col_it != tops_col.end() && bottoms_col_it != bottoms_col.end())
    {
        min_distance_v = std::min(tops_col_it->first - plus_pos_y.first, bottoms_col_it->first - plus_pos_y.first);
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

void ReactionJsonLoader::constructMultipleArrowReaction(BaseReaction& rxn)
{
    // _reaction_components -> allMolecules
    // _component_summ_blocks ->
    for (auto& rc : _reaction_components)
    {
        if (rc.component_type == ReactionComponent::MOLECULE)
        {
            auto& csb = _component_summ_blocks[rc.summ_block_idx];
            switch (csb.role)
            {
            case BaseReaction::REACTANT:
                rxn.addReactantCopy(*rc.molecule, 0, 0);
                break;
            case BaseReaction::PRODUCT:
                rxn.addProductCopy(*rc.molecule, 0, 0);
                break;
            case BaseReaction::INTERMEDIATE:
                rxn.addIntermediateCopy(*rc.molecule, 0, 0);
                break;
            case BaseReaction::UNDEFINED:
                rxn.addUndefinedCopy(*rc.molecule, 0, 0);
                break;
            case BaseReaction::CATALYST:
                rxn.addCatalystCopy(*rc.molecule, 0, 0);
                break;
            }
        }
    }
}

void ReactionJsonLoader::parseMultipleArrowReaction(BaseReaction& rxn)
{
    auto pair_comp_asc = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.first > a.first; };
    auto pair_comp_des = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.first < a.first; };
    auto pair_comp_mol_asc = [](const FLOAT_INT_PAIR& a, const FLOAT_INT_PAIR& b) { return b.second > a.second; };
    std::list<std::unordered_set<int>> s_neighbors;
    getSGroupAtoms(*_pmol, s_neighbors);
    int count = _pmol->countComponents(s_neighbors);
    _reaction_components.reserve(count);
    FLOAT_INT_PAIRS mol_tops, mol_bottoms, mol_lefts, mol_rights;

    // collect components
    for (int i = 0; i < count; ++i)
    {
        Filter filter(_pmol->getDecomposition().ptr(), Filter::EQ, i);
        std::unique_ptr<BaseMolecule> component;
        if (rxn.isQueryReaction())
            component = std::make_unique<QueryMolecule>();
        else
            component = std::make_unique<Molecule>();

        BaseMolecule& mol = *component;
        mol.makeSubmolecule(*_pmol, filter, 0, 0);
        Rect2f bbox;
        mol.getBoundingBox(bbox, MIN_MOL_SIZE);

        mol_tops.emplace_back(bbox.top(), i);
        mol_bottoms.emplace_back(bbox.bottom(), i);
        mol_lefts.emplace_back(bbox.left(), i);
        mol_rights.emplace_back(bbox.right(), i);
        _reaction_components.emplace_back(ReactionComponent::MOLECULE, bbox, i, std::move(component));
    }

    for (int i = 0; i < rxn.meta().getMetaCount(KETReactionPlus::CID); ++i)
    {
        auto& plus = (const KETReactionPlus&)rxn.meta().getMetaObject(KETReactionPlus::CID, i);
        const Vec2f& plus_pos = plus._pos;
        Rect2f bbox(plus_pos - PLUS_BBOX_SHIFT, plus_pos + PLUS_BBOX_SHIFT);
        _reaction_components.emplace_back(ReactionComponent::PLUS, bbox, i, std::unique_ptr<BaseMolecule>(nullptr));
        _reaction_components.back().coordinates.push_back(plus_pos);
        int index = _reaction_components.size() - 1;
        mol_tops.emplace_back(bbox.top(), index);
        mol_bottoms.emplace_back(bbox.bottom(), index);
        mol_lefts.emplace_back(bbox.left(), index);
        mol_rights.emplace_back(bbox.right(), index);
    }

    for (int i = 0; i < rxn.meta().getMetaCount(KETReactionArrow::CID); ++i)
    {
        auto& arrow = (const KETReactionArrow&)rxn.meta().getMetaObject(KETReactionArrow::CID, i);
        int arrow_type = arrow._arrow_type;
        const Vec2f& arr_begin = arrow._begin;
        const Vec2f& arr_end = arrow._end;
        Rect2f bbox(arr_begin - ARROW_BBOX_SHIFT, arr_end + ARROW_BBOX_SHIFT);
        _reaction_components.emplace_back(arrow_type, bbox, i, std::unique_ptr<BaseMolecule>(nullptr));
        _reaction_components.back().coordinates.push_back(arr_begin);
        _reaction_components.back().coordinates.push_back(arr_end);
        int index = _reaction_components.size() - 1;
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

    for (int i = 0; i < rxn.meta().getMetaCount(KETReactionPlus::CID); ++i)
    {
        auto& plus = (const KETReactionPlus&)rxn.meta().getMetaObject(KETReactionPlus::CID, i);
        const Vec2f& plus_pos = plus._pos;
        std::pair<int, int> plus_connection; // (component1_index, component2_index)

        if (findPlusNeighbours(plus_pos, mol_tops, mol_bottoms, mol_lefts, mol_rights, plus_connection))
        {
            _reaction_components[count + i].summ_block_idx = ReactionComponent::CONNECTED; // mark plus as connected

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
            _reaction_components[v].summ_block_idx = _component_summ_blocks.size();
        _component_summ_blocks.push_back(csb);
    }

    // add all single molecules to _component_summ_blocks
    for (int i = 0; i < _reaction_components.size(); ++i)
    {
        auto& rc = _reaction_components[i];
        if (rc.component_type != ReactionComponent::MOLECULE)
            break;
        if (rc.summ_block_idx == ReactionComponent::NOT_CONNECTED)
        {
            rc.summ_block_idx = _component_summ_blocks.size();
            _component_summ_blocks.push_back(_reaction_components[i].bbox);
            _component_summ_blocks.back().indexes.push_back(i);
        }
    }

    // handle arrows
    for (int i = 0; i < rxn.meta().getMetaCount(KETReactionArrow::CID); ++i)
    {
        auto& arrow = (const KETReactionArrow&)rxn.meta().getMetaObject(KETReactionArrow::CID, i);
        int arrow_type = arrow._arrow_type;
        const Vec2f& arr_begin = arrow._begin;
        const Vec2f& arr_end = arrow._end;
        float min_dist_prod = -1, min_dist_reac = -1;
        int idx_cs_min_prod = -1, idx_cs_min_reac = -1;
        for (int index_cs = 0; index_cs < _component_summ_blocks.size(); ++index_cs)
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

        auto& rb = rxn.addReactionBlock();
        rb.arrow_index = i;

        if (min_dist_prod > 0 && min_dist_reac > 0) // if both ends present
        {
            auto& rc_arrow = _reaction_components[count + rxn.meta().getMetaCount(KETReactionPlus::CID) + i];
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

            // idx_cs_min_reac -> idx_cs_min_prod
            csb_min_reac.arrows_to.push_back(idx_cs_min_prod);
            for (auto ri : csb_min_reac.indexes)
                rb.reactants.push(_reaction_components[ri].index);
            for (auto pi : csb_min_prod.indexes)
                rb.products.push(_reaction_components[pi].index);
        }
    }

    // _component_summ_blocks, _reaction_components - result
    constructMultipleArrowReaction(rxn);
}

void ReactionJsonLoader::parseOneArrowReaction(BaseReaction& rxn)
{
    enum RecordIndexes
    {
        BBOX_IDX = 0,
        FRAGMENT_TYPE_IDX,
        MOLECULE_IDX
    };

    enum class ReactionFragmentType
    {
        MOLECULE,
        PLUS,
        ARROW,
        TEXT
    };

    using ReactionComponent = std::tuple<Rect2f, ReactionFragmentType, std::unique_ptr<BaseMolecule>>;

    std::vector<ReactionComponent> components;

    std::list<std::unordered_set<int>> s_neighbors;
    getSGroupAtoms(*_pmol, s_neighbors);

    for (int index = 0; index < _pmol->countComponents(s_neighbors); ++index)
    {
        std::unique_ptr<BaseMolecule> mol;
        if (_pmol->isQueryMolecule())
            mol = std::make_unique<QueryMolecule>();
        else
            mol = std::make_unique<Molecule>();

        Filter filter(_pmol->getDecomposition().ptr(), Filter::EQ, index);

        mol->makeSubmolecule(*_pmol, filter, 0, 0);
        Rect2f bbox;
        mol->getBoundingBox(bbox);
        components.emplace_back(bbox, ReactionFragmentType::MOLECULE, std::move(mol));
    }

    auto& arrow = (const KETReactionArrow&)rxn.meta().getMetaObject(KETReactionArrow::CID, 0);

    for (int i = 0; i < rxn.meta().getMetaCount(KETTextObject::CID); ++i)
    {
        auto& text = (const KETTextObject&)rxn.meta().getMetaObject(KETTextObject::CID, i);
        Rect2f bbox(Vec2f(text._pos.x, text._pos.y), Vec2f(text._pos.x, text._pos.y)); // change to real text box later
        components.emplace_back(bbox, ReactionFragmentType::TEXT, nullptr);
    }

    int text_meta_idx = 0;
    for (const auto& comp : components)
    {
        switch (std::get<FRAGMENT_TYPE_IDX>(comp))
        {
        case ReactionFragmentType::MOLECULE: {
            auto& cmol = *std::get<MOLECULE_IDX>(comp);
            for (int idx = cmol.vertexBegin(); idx < cmol.vertexEnd(); idx = cmol.vertexNext(idx))
            {
                Vec3f& pt3d = cmol.getAtomXyz(idx);
                Vec2f pt(pt3d.x, pt3d.y);
                int side = getPointSide(pt, arrow._begin, arrow._end);
                switch (side)
                {
                case KETReagentUpArea:
                case KETReagentDownArea:
                    rxn.addCatalystCopy(cmol, 0, 0);
                    break;
                case KETProductArea:
                    rxn.addProductCopy(cmol, 0, 0);
                    break;
                default:
                    rxn.addReactantCopy(cmol, 0, 0);
                    break;
                }
                break;
            }
        }
        break;
        case ReactionFragmentType::TEXT: {
            const auto& bbox = std::get<BBOX_IDX>(comp);
            Vec2f pt(bbox.center());
            int side = getPointSide(pt, arrow._begin, arrow._end);
            if (side == KETReagentUpArea || side == KETReagentDownArea)
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
