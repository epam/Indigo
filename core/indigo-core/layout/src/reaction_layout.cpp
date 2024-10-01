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
#include "molecule/ket_commons.h"
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
      max_iterations(0)
{
    _options.bondLength = bond_length;
    _options.reactionComponentMarginSize = reaction_margin_size;
}

ReactionLayout::ReactionLayout(BaseReaction& r, bool smart_layout, const LayoutOptions& options)
    : bond_length(LayoutOptions::DEFAULT_BOND_LENGTH), default_plus_size(LayoutOptions::DEFAULT_PLUS_SIZE),
      default_arrow_size(LayoutOptions::DEFAULT_BOND_LENGTH * 2), preserve_molecule_layout(false), _r(r), _smart_layout(smart_layout),
      reaction_margin_size(options.getMarginSizeInAngstroms()), atom_label_margin(LayoutOptions::DEFAULT_BOND_LENGTH / 2), layout_orientation(UNCPECIFIED),
      max_iterations(0), _options(options)
{
}

void ReactionLayout::fixLayout()
{
    int arrows_count = _r.meta().getMetaCount(KETReactionArrow::CID);
    int simple_count = _r.meta().getMetaCount(KETSimpleObject::CID) + _r.meta().getMetaCount(KETTextObject::CID);
    int multi_count = _r.meta().getMetaCount(KETReactionMultitailArrow::CID);
    if (arrows_count > 1 || simple_count || multi_count)
        return;

    Rect2f bb;
    // Calculate rightTop of reactant bounding box
    bool invalid_layout = false;
    float cur_left = 0, cur_right = 0;
    for (int i = _r.isRetrosyntetic() ? _r.productBegin() : _r.reactantBegin(); i != (_r.isRetrosyntetic() ? _r.productEnd() : _r.reactantEnd());
         i = _r.isRetrosyntetic() ? _r.productNext(i) : _r.reactantNext(i))
    {
        _r.getBaseMolecule(i).getBoundingBox(bb);
        if (i == 0 || (bb.left() > cur_left && bb.right() > cur_right))
        {
            cur_left = bb.left();
            cur_right = bb.right();
        }
        else
            invalid_layout = true;
    }

    // Calculate leftBottom of product bounding box
    for (int i = _r.isRetrosyntetic() ? _r.reactantBegin() : _r.productBegin(); i != (_r.isRetrosyntetic() ? _r.reactantEnd() : _r.productEnd());
         i = _r.isRetrosyntetic() ? _r.reactantNext(i) : _r.productNext(i))
    {
        _r.getBaseMolecule(i).getBoundingBox(bb);
        if (bb.left() > cur_left && bb.right() > cur_right)
        {
            cur_left = bb.left();
            cur_right = bb.right();
        }
        else
            invalid_layout = true;
    }

    // if left side of product bb at left of right side of reactant bb - fix layout
    if (invalid_layout)
    {
        ReactionLayout rl(_r, true);
        rl.preserve_molecule_layout = true;
        rl.make();
    }
    else if (_r.meta().getMetaCount(KETReactionArrow::CID) == 0 && _r.meta().getMetaCount(KETReactionMultitailArrow::CID) == 0)
        _updateMetadata();
}

void ReactionLayout::_updateMetadata()
{
    float arrow_height = 0;
    int arrow_type = KETReactionArrow::EOpenAngle;
    if (_r.meta().getMetaCount(KETReactionArrow::CID) > 0)
    {
        auto& ra = static_cast<const KETReactionArrow&>(_r.meta().getMetaObject(KETReactionArrow::CID, 0));
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
    if (_r.reactantsCount() > 0)
    {
        processSideBoxes(pluses, react_box, BaseReaction::REACTANT);
        for (int i = _r.reactantBegin(); i != _r.reactantEnd(); i = _r.reactantNext(i))
            last_single_reactant = _r.getBaseMolecule(i).vertexCount() == 1;
    }

    if (_r.productsCount() > 0)
    {
        processSideBoxes(pluses, product_box, BaseReaction::PRODUCT);
        first_single_product = _r.getBaseMolecule(_r.productBegin()).vertexCount() == 1;
    }

    if (_r.catalystCount() > 0)
        processSideBoxes(pluses, catalyst_box, BaseReaction::CATALYST);

    for (const auto& plus_offset : pluses)
        _r.meta().addMetaObject(new KETReactionPlus(plus_offset));

    // calculate arrow size and position
    Vec2f arrow_head(0, 0);
    Vec2f arrow_tail(0, 0);
    if (_r.productsCount() == 0)
    {
        arrow_tail.x = react_box.right() + reaction_margin_size + atom_label_margin;
        arrow_tail.y = react_box.middleY();
        arrow_head.x = arrow_tail.x + default_arrow_size + atom_label_margin;
        arrow_head.y = arrow_tail.y;
    }
    else if (_r.reactantsCount() == 0)
    {
        arrow_head.x = product_box.left() - reaction_margin_size - atom_label_margin;
        arrow_head.y = product_box.middleY();
        arrow_tail.x = arrow_head.x - default_arrow_size - atom_label_margin;
        arrow_tail.y = arrow_head.y;
    }
    else
    {
        const float ptab = /*first_single_product ? reaction_margin_size * 2 :*/ reaction_margin_size + atom_label_margin;
        const float rtab = /*last_single_reactant ? reaction_margin_size * 2 :*/ reaction_margin_size + atom_label_margin;

        arrow_head.y = product_box.middleY();
        arrow_tail.y = react_box.middleY();

        if (product_box.left() > react_box.right())
        {
            arrow_head.x = product_box.left() - ptab;
            arrow_tail.x = react_box.right() + rtab;
        }
        else
        {
            arrow_head.x = react_box.right() + rtab;
            arrow_tail.x = product_box.left() - ptab;
        }
    }
    _r.meta().addMetaObject(new KETReactionArrow(arrow_type, arrow_tail, arrow_head, arrow_height));
}

void ReactionLayout::processSideBoxes(std::vector<Vec2f>& pluses, Rect2f& type_box, int side)
{
    int begin = _r.sideBegin(side);
    std::vector<Rect2f> boxes;

    for (int i = begin; i != _r.sideEnd(); i = _r.sideNext(side, i))
    {
        BaseMolecule& mol = _r.getBaseMolecule(i);

        Rect2f box;
        mol.getBoundingBox(box);
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
    int arrows_count = _r.meta().getMetaCount(KETReactionArrow::CID);
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

    processReactionElements(_r.reactantBegin(), _r.reactantEnd(), &BaseReaction::reactantNext);

    if (_r.catalystCount())
    {

        _pushSpace(line, reaction_margin_size);
        _pushSpace(line, reaction_margin_size);
        for (int i = _r.catalystBegin(); i < _r.catalystEnd(); i = _r.catalystNext(i))
        {
            auto& mol = _getMol(i);
            Rect2f bbox;
            mol.getBoundingBox(bbox, Vec2f(bond_length, bond_length));
            if (i != _r.catalystBegin())
                _pushSpace(line, reaction_margin_size);
            _pushMol(line, i, true);
        }
        _pushSpace(line, reaction_margin_size);
        _pushSpace(line, reaction_margin_size);
    }
    else
        _pushSpace(line, default_arrow_size + reaction_margin_size * 2);

    processReactionElements(_r.productBegin(), _r.productEnd(), &BaseReaction::productNext);

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
    mol.getBoundingBox(bbox);
    item.min.copy(bbox.leftBottom());
    item.max.copy(bbox.rightTop());
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
