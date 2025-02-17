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
#include <functional>
#include <numeric>
#include <stdio.h>

#include "layout/molecule_layout.h"
#include "layout/reaction_layout.h"
#include "molecule/meta_commons.h"
#include "molecule/molecule.h"
#include "reaction/pathway_reaction_builder.h"
#include "reaction/reaction.h"
#include "reaction/reaction_multistep_detector.h"

#ifdef _MSC_VER
#pragma warning(push, 4)
#endif

using namespace std::placeholders;
using namespace indigo;

ReactionLayout::ReactionLayout(BaseReaction& r, bool smart_layout)
    : bond_length(LayoutOptions::DEFAULT_BOND_LENGTH), default_plus_size(1), default_arrow_size(2), preserve_molecule_layout(false), _r(r),
      _smart_layout(smart_layout), reaction_margin_size(DEFAULT_HOR_INTERVAL_FACTOR), atom_label_margin(1.3f), layout_orientation(UNCPECIFIED),
      max_iterations(0), _font_size(-1)
{
    _options.bondLength = bond_length;
    _options.reactionComponentMarginSize = reaction_margin_size;
}

ReactionLayout::ReactionLayout(BaseReaction& r, bool smart_layout, const LayoutOptions& options)
    : bond_length(options.fontSize > EPSILON ? 1 : LayoutOptions::DEFAULT_BOND_LENGTH), default_plus_size(LayoutOptions::DEFAULT_PLUS_SIZE),
      default_arrow_size(LayoutOptions::DEFAULT_BOND_LENGTH), preserve_molecule_layout(false), _r(r), _smart_layout(smart_layout),
      reaction_margin_size(options.fontSize > EPSILON ? options.getMarginSizeInAngstroms()
                                                      : LayoutOptions::DEFAULT_BOND_LENGTH * options.getMarginSizeInAngstroms()),
      atom_label_margin(LayoutOptions::DEFAULT_BOND_LENGTH / 2), layout_orientation(UNCPECIFIED), max_iterations(0), _options(options),
      _font_size(options.getFontSizeInAngstroms())
{
}

bool ReactionLayout::hasAnyIntersect(const std::vector<Rect2f>& bblist)
{
    std::vector<SweepEvent> events;
    events.reserve(bblist.size() * 2);
    for (const auto& rect : bblist)
    {
        events.emplace_back(SweepEvent{rect.left(), true, rect.bottom(), rect.top()});
        events.emplace_back(SweepEvent{rect.left() + rect.width(), false, rect.bottom(), rect.top()});
    }
    std::sort(events.begin(), events.end());
    std::set<std::pair<float, float>> active;
    for (const auto& event : events)
    {
        if (event.is_start)
        {
            auto it = active.lower_bound({event.y_start, event.y_end});
            if (it != active.begin())
            {
                auto prev = std::prev(it);
                if (prev->second > event.y_start)
                    return true;
            }
            if (it != active.end() && it->first < event.y_end)
                return true;
            active.emplace(event.y_start, event.y_end);
        }
        else
        {
            active.erase({event.y_start, event.y_end});
        }
    }
    return false;
}

bool ReactionLayout::validVerticalRange(const std::vector<Rect2f>& bblist)
{
    if (bblist.empty())
        return true;

    float max_start = std::max_element(bblist.begin(), bblist.end(), [](const Rect2f& a, const Rect2f& b) { return a.bottom() < b.bottom(); })->bottom();
    float min_end = std::min_element(bblist.begin(), bblist.end(), [](const Rect2f& a, const Rect2f& b) { return a.top() < b.top(); })->top();
    return max_start <= min_end;
}

void ReactionLayout::fixLayout()
{
    int arrows_count = _r.meta().getMetaCount(ReactionArrowObject::CID);
    int simple_count = _r.meta().getMetaCount(SimpleGraphicsObject::CID) + _r.meta().getMetaCount(SimpleTextObject::CID);
    int multi_count = _r.meta().getMetaCount(ReactionMultitailArrowObject::CID);
    if (arrows_count || simple_count || multi_count)
        return;

    Vec2f rmax{Vec2f::min_coord(), Vec2f::min_coord()}, pmin{Vec2f::max_coord(), Vec2f::max_coord()};
    Rect2f bb;
    std::vector<Rect2f> bboxes;
    // Calculate rightTop of reactant bounding box
    bool invalid_layout = false;
    float cur_left = 0, cur_right = 0;
    for (int i = _r.isRetrosyntetic() ? _r.productBegin() : _r.reactantBegin(); i != (_r.isRetrosyntetic() ? _r.productEnd() : _r.reactantEnd());
         i = _r.isRetrosyntetic() ? _r.productNext(i) : _r.reactantNext(i))
    {
        _r.getBaseMolecule(i).getBoundingBox(bb);
        bboxes.push_back(bb);
        rmax.max(bb.rightTop());
        if (i == 0 || (bb.left() > cur_left && bb.right() > cur_right))
        {
            cur_left = bb.left();
            cur_right = bb.right();
        }
        else
            invalid_layout = true;
    }

    bool first_after_arrow = true;
    // Calculate leftBottom of product bounding box
    for (int i = _r.isRetrosyntetic() ? _r.reactantBegin() : _r.productBegin(); i != (_r.isRetrosyntetic() ? _r.reactantEnd() : _r.productEnd());
         i = _r.isRetrosyntetic() ? _r.reactantNext(i) : _r.productNext(i))
    {
        _r.getBaseMolecule(i).getBoundingBox(bb);
        bboxes.push_back(bb);
        pmin.min(bb.leftBottom());
        if (bb.left() > cur_left && bb.right() > cur_right)
        {
            if (first_after_arrow)
            {
                first_after_arrow = false;
                if (bb.left() - cur_left < default_arrow_size)
                {
                    invalid_layout = true;
                    break;
                }
            }
            cur_left = bb.left();
            cur_right = bb.right();
        }
        else
        {
            invalid_layout = true;
            break;
        }
    }

    if (!invalid_layout)
        invalid_layout = hasAnyIntersect(bboxes) || !validVerticalRange(bboxes);

    float arrow_len = pmin.x - rmax.x - (2 * ReactionMarginSize());
    // if left side of product bb at left of right side of reactant bb - fix layout
    if (invalid_layout || arrow_len != default_arrow_size)
    {
        ReactionLayout rl(_r, true, _options);
        rl.preserve_molecule_layout = true;
        for (int i = _r.begin(); i < _r.end(); i = _r.next(i))
        {
            auto& mol = _r.getBaseMolecule(i);
            if (mol.vertexCount() > 1 && Metalayout::getTotalMoleculeBondLength(mol) <= +0.0f)
            {
                rl.preserve_molecule_layout = false;
                break;
            }
        }
        rl.make();
    }
    else if (_r.meta().getMetaCount(ReactionArrowObject::CID) == 0 && _r.meta().getMetaCount(ReactionMultitailArrowObject::CID) == 0)
        _updateMetadata();
}

void ReactionLayout::_updateMetadata()
{
    float arrow_height = 0;
    int arrow_type = ReactionArrowObject::EOpenAngle;
    if (_r.meta().getMetaCount(ReactionArrowObject::CID) > 0)
    {
        auto& ra = static_cast<const ReactionArrowObject&>(_r.meta().getMetaObject(ReactionArrowObject::CID, 0));
        // remember arrow type & height
        arrow_type = ra.getArrowType();
        arrow_height = ra.getHeight();
        // reset pluses and arrows
        _r.meta().resetReactionData();
    }

    std::vector<Vec2f> pluses;
    Rect2f react_box, product_box, catalyst_box;
    bool last_single_reactant = false;
    bool first_single_product = false;
    bool is_retrosyntetic = _r.isRetrosyntetic();
    if (_r.reactantsCount() > 0)
    {
        if (is_retrosyntetic)
        {
            processSideBoxes(pluses, react_box, BaseReaction::PRODUCT);
            for (int i = _r.productBegin(); i != _r.productEnd(); i = _r.productNext(i))
                last_single_reactant = _r.getBaseMolecule(i).vertexCount() == 1;
        }
        else
        {
            processSideBoxes(pluses, react_box, BaseReaction::REACTANT);
            for (int i = _r.reactantBegin(); i != _r.reactantEnd(); i = _r.reactantNext(i))
                last_single_reactant = _r.getBaseMolecule(i).vertexCount() == 1;
        }
    }

    if (_r.productsCount() > 0)
    {
        if (is_retrosyntetic)
        {
            processSideBoxes(pluses, product_box, BaseReaction::REACTANT);
            first_single_product = _r.getBaseMolecule(_r.reactantBegin()).vertexCount() == 1;
        }
        else
        {
            processSideBoxes(pluses, product_box, BaseReaction::PRODUCT);
            first_single_product = _r.getBaseMolecule(_r.productBegin()).vertexCount() == 1;
        }
    }

    float arrow_length = default_arrow_size;
    if (_r.catalystCount() > 0)
    {
        processSideBoxes(pluses, catalyst_box, BaseReaction::CATALYST);
        arrow_length = catalyst_box.width();
    }

    for (const auto& plus_offset : pluses)
        _r.meta().addMetaObject(new ReactionPlusObject(plus_offset));

    // calculate arrow size and position
    Vec2f arrow_head(0, 0);
    Vec2f arrow_tail(0, 0);

    int prod_count = is_retrosyntetic ? _r.reactantsCount() : _r.productsCount();
    int react_count = is_retrosyntetic ? _r.productsCount() : _r.reactantsCount();
    if (prod_count == 0)
    {
        arrow_tail.x = react_box.right() + ReactionMarginSize();
        arrow_tail.y = react_box.middleY();
        arrow_head.x = arrow_tail.x + arrow_length + ReactionMarginSize() * 2;
        arrow_head.y = arrow_tail.y;
    }
    else if (react_count == 0)
    {
        arrow_head.x = product_box.left() - ReactionMarginSize();
        arrow_head.y = product_box.middleY();
        arrow_tail.x = arrow_head.x - arrow_length - ReactionMarginSize() * 2;
        arrow_tail.y = arrow_head.y;
    }
    else
    {
        arrow_head.y = product_box.middleY();
        arrow_tail.y = react_box.middleY();

        if (product_box.left() > react_box.right())
        {
            arrow_head.x = product_box.left() - ReactionMarginSize();
            arrow_tail.x = react_box.right() + ReactionMarginSize();
        }
        else
        {
            arrow_head.x = react_box.right() + ReactionMarginSize();
            arrow_tail.x = product_box.left() - ReactionMarginSize();
        }
    }
    _r.meta().addMetaObject(new ReactionArrowObject(arrow_type, arrow_tail, arrow_head, arrow_height));
}

void ReactionLayout::processSideBoxes(std::vector<Vec2f>& pluses, Rect2f& type_box, int side)
{
    int begin = _r.sideBegin(side);
    std::vector<Rect2f> boxes;

    for (int i = begin; i != _r.sideEnd(); i = _r.sideNext(side, i))
    {
        BaseMolecule& mol = _r.getBaseMolecule(i);

        Rect2f box;
        // If have font size calc bounding box with labes
        if (_font_size < EPSILON)
            mol.getBoundingBox(box, Vec2f(atom_label_margin, atom_label_margin));
        else
            mol.getBoundingBox(_font_size, _options.labelMode, box);
        if (i == begin)
            type_box.copy(box);
        else
            type_box.extend(box);

        if (side != BaseReaction::CATALYST)
            boxes.emplace_back(box);
    }

    // For REACTANT and PRODUCT insert pluses between boxes
    if (side != BaseReaction::CATALYST && boxes.size() > 1)
    {
        std::ignore = std::accumulate(std::next(boxes.begin()), boxes.end(), boxes[0], [&pluses](Rect2f left, Rect2f right) {
            pluses.emplace_back(right.between_left_box(left), left.middleY());
            return right;
        });
    }
}

void ReactionLayout::makePathwayFromSimple()
{
    std::deque<Reaction> reactions;
    for (int i = 0; i < _r.reactionBlocksCount(); i++)
    {
        auto& rb = _r.reactionBlock(i);
        if (rb.products.size() || rb.reactants.size())
        {
            auto& rc = reactions.emplace_back();
            for (int j = 0; j < rb.reactants.size(); j++)
                rc.addReactantCopy(_r.getBaseMolecule(rb.reactants[j]), 0, 0);
            for (int j = 0; j < rb.products.size(); j++)
                rc.addProductCopy(_r.getBaseMolecule(rb.products[j]), 0, 0);
        }
    }
    PathwayReactionBuilder prb;
    auto pwr = prb.buildPathwayReaction(reactions, _options);
    _r.meta().resetReactionData();
    pwr->meta().append(_r.meta());
    pwr->copyToReaction(_r);
}

void ReactionLayout::make()
{
    int arrows_count = _r.meta().getMetaCount(ReactionArrowObject::CID);
    int simple_count = _r.meta().getNonChemicalMetaCount();
    if (simple_count)
        return;

    /*
    int multi_count = _r.meta().getMetaCount(KETReactionMultitailArrow::CID);
    if (_r.reactionBlocksCount() > 1 && _r.intermediateCount() == 0 && multi_count == 0)
    {
        makePathwayFromSimple();
        return;
    }
    */

    if (arrows_count > 1)
        return; // not implemented yet

    //  update layout of molecules, if needed
    if (!preserve_molecule_layout)
    {
        for (int i = _r.begin(); i < _r.end(); i = _r.next(i))
        {
            MoleculeLayout molLayout(_r.getBaseMolecule(i), _smart_layout);
            molLayout.max_iterations = max_iterations;
            molLayout.layout_orientation = layout_orientation;
            molLayout.bond_length = bond_length;
            molLayout.multiple_distance = MOL_COMPONENT_INTERVAL;
            molLayout.make();
        }
    }

    // layout molecules in a row with the intervals specified
    Metalayout::LayoutLine& line = _ml.newLine();
    auto processReactionElements = [this, &line](int begin, int end, std::function<int(BaseReaction&, int)> next) {
        for (int i = begin; i < end; i = next(_r, i))
        {
            // bool single_atom = _getMol(i).vertexCount() == 1;
            if (i != begin)
            {
                _pushSpace(line, reaction_margin_size);
                _pushSpace(line, default_plus_size);
                _pushSpace(line, reaction_margin_size);
            }
            _pushMol(line, i);
        }
    };

    if (_r.isRetrosyntetic())
        processReactionElements(_r.productBegin(), _r.productEnd(), &BaseReaction::productNext);
    else
    {
        processReactionElements(_r.reactantBegin(), _r.reactantEnd(), &BaseReaction::reactantNext);
    }

    if (_r.catalystCount())
    {
        _pushSpace(line, reaction_margin_size);
        _pushSpace(line, reaction_margin_size);
        for (int i = _r.catalystBegin(); i < _r.catalystEnd(); i = _r.catalystNext(i))
        {
            if (i != _r.catalystBegin())
                _pushSpace(line, reaction_margin_size);
            _pushMol(line, i, true);
        }
        _pushSpace(line, reaction_margin_size);
        _pushSpace(line, reaction_margin_size);
    }
    else
        _pushSpace(line, default_arrow_size + reaction_margin_size * 2);

    if (_r.isRetrosyntetic())
    {
        processReactionElements(_r.reactantBegin(), _r.reactantEnd(), &BaseReaction::reactantNext);
    }
    else
        processReactionElements(_r.productBegin(), _r.productEnd(), &BaseReaction::productNext);

    if (_r.undefinedCount())
    {
        _ml.verticalIntervalFactor += reaction_margin_size + DEFAULT_VER_INTERVAL_FACTOR;
        float first_margin = 0, last_margin = 0;

        if (_r.reactantBegin() != _r.reactantEnd())
        {
            Rect2f bb;
            _r.getBaseMolecule(_r.reactantBegin()).getBoundingBox(bb);
            first_margin = bb.width();
        }
        else if (_r.productBegin() != _r.productEnd())
        {
            Rect2f bb;
            _r.getBaseMolecule(_r.productBegin()).getBoundingBox(bb);
            last_margin = bb.width();
        }

        for (int i = _r.undefinedBegin(); i < _r.undefinedEnd(); i = _r.undefinedNext(i))
        {
            Rect2f bbox;
            _r.getBaseMolecule(i).getBoundingBox(bbox, MIN_MOL_SIZE);
            Metalayout::LayoutLine& line_undef = _ml.newLine();
            line_undef.offset = first_margin - bbox.width();
            if (_r.reactantsCount() == 0)
                line_undef.offset -= last_margin;
            _pushMol(line_undef, i, false);
        }
    }

    _ml.bondLength = bond_length;
    _ml.reactionComponentMarginSize = reaction_margin_size;
    _ml.cb_getMol = cb_getMol;
    _ml.cb_process = cb_process;
    _ml.context = this;
    _ml.prepare();
    _ml.scaleMoleculesSize();
    _ml.calcContentSize();
    _ml.process();
    _updateMetadata();
}

void ReactionLayout::_pushMol(Metalayout::LayoutLine& line, int id, bool is_catalyst)
{
    // Molecule label alligned to atom center by non-hydrogen
    // Hydrogen may be at left or at right H2O, PH3 - so add space before and after molecule
    if (_font_size < EPSILON)
        _pushSpace(line, atom_label_margin);
    Metalayout::LayoutItem& item = line.items.push();
    item.type = Metalayout::LayoutItem::Type::EMolecule;
    item.isMoleculeFragment = true;
    item.id = id;
    auto& mol = _getMol(id);
    if (is_catalyst)
    {
        item.verticalAlign = Metalayout::LayoutItem::ItemVerticalAlign::ETop;
    }
    Rect2f bbox;
    // If have font size calc bounding box with labes
    if (_font_size < EPSILON)
        mol.getBoundingBox(bbox, Vec2f(atom_label_margin, atom_label_margin));
    else
        mol.getBoundingBox(_font_size, _options.labelMode, bbox);
    item.min.copy(bbox.leftBottom());
    item.max.copy(bbox.rightTop());
    if (_font_size < EPSILON)
        _pushSpace(line, atom_label_margin);
}

void ReactionLayout::_pushSpace(Metalayout::LayoutLine& line, float size)
{
    Metalayout::LayoutItem& item = line.items.push();
    item.type = Metalayout::LayoutItem::Type::ESpace;
    item.isMoleculeFragment = false;
    item.scaledSize.set(size, 0);
}

BaseMolecule& ReactionLayout::_getMol(int id)
{
    return _r.getBaseMolecule(id);
}

BaseMolecule& ReactionLayout::cb_getMol(int id, void* context)
{
    return ((ReactionLayout*)context)->_getMol(id);
}

void ReactionLayout::cb_process(Metalayout::LayoutItem& item, const Vec2f& pos, void* context)
{
    if (item.isMoleculeFragment)
    {
        Vec2f pos2;
        pos2.copy(pos);
        pos2.y -= item.scaledSize.y / 2;
        auto layout = (ReactionLayout*)context;
        layout->_ml.adjustMol(layout->_getMol(item.id), item.min, pos2);
    }
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
