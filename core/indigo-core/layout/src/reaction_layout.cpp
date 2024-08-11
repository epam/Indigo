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

#include "layout/reaction_layout.h"
#include "layout/molecule_layout.h"
#include "molecule/ket_commons.h"
#include "molecule/molecule.h"
#include "reaction/reaction.h"
#include <numeric>
#include <stdio.h>

using namespace indigo;

ReactionLayout::ReactionLayout(BaseReaction& r, bool smart_layout)
    : bond_length(MoleculeLayout::DEFAULT_BOND_LENGTH), plus_interval_factor(1), arrow_interval_factor(2), preserve_molecule_layout(false), _r(r),
      _smart_layout(smart_layout), horizontal_interval_factor(DEFAULT_HOR_INTERVAL_FACTOR), atom_label_width(1.3f), layout_orientation(UNCPECIFIED),
      max_iterations(0)
{
}

void ReactionLayout::fixLayout()
{
    int arrows_count = _r.meta().getMetaCount(KETReactionArrow::CID);
    int simple_count = _r.meta().getMetaCount(KETSimpleObject::CID) + _r.meta().getMetaCount(KETTextObject::CID);
    if (arrows_count > 1 || simple_count)
        return;

    Vec2f rmax{Vec2f::min_coord(), Vec2f::min_coord()}, pmin{Vec2f::max_coord(), Vec2f::max_coord()};
    Rect2f bb;
    // Calculate rightTop of reactant bounding box
    for (int i = _r.reactantBegin(); i != _r.reactantEnd(); i = _r.reactantNext(i))
    {
        _r.getBaseMolecule(i).getBoundingBox(bb);
        rmax.max(bb.rightTop());
    }

    // Calculate leftBottom of product bounding box
    for (int i = _r.productBegin(); i != _r.productEnd(); i = _r.productNext(i))
    {
        _r.getBaseMolecule(i).getBoundingBox(bb);
        pmin.min(bb.leftBottom());
    }

    // if left side of product bb at left of right side of reactant bb - fix layout
    if (rmax.x > pmin.x)
    {
        ReactionLayout rl(_r, true);
        rl.preserve_molecule_layout = true;
        rl.make();
    }
    else if (_r.meta().getMetaCount(KETReactionArrow::CID) == 0)
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

    Vec2f arrow_head(0, 0);
    Vec2f arrow_tail(0, 0);

    constexpr float shift = 1.0f;
    if (_r.productsCount() == 0)
    {
        arrow_tail.x = react_box.right() + shift;
        arrow_tail.y = react_box.middleY();
        arrow_head.x = arrow_tail.x + shift;
        arrow_head.y = arrow_tail.y;
    }
    else if (_r.reactantsCount() == 0)
    {
        arrow_head.x = product_box.left() - shift;
        arrow_head.y = product_box.middleY();
        arrow_tail.x = arrow_head.x - shift;
        arrow_tail.y = arrow_head.y;
    }
    else
    {
        const float ptab = first_single_product ? 2.0f : 1.0f;
        const float rtab = last_single_reactant ? 2.0f : 1.0f;

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

void ReactionLayout::make()
{
    int arrows_count = _r.meta().getMetaCount(KETReactionArrow::CID);
    int simple_count = _r.meta().getNonChemicalMetaCount();
    if (arrows_count > 1 || simple_count)
        return; // not implemented yet

    const auto kHalfBondLength = bond_length / 2;
    const auto kDoubleBondLength = bond_length * 2;
    // update layout of molecules, if needed
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
    for (int i = _r.reactantBegin(); i < _r.reactantEnd(); i = _r.reactantNext(i))
    {
        bool single_atom = _getMol(i).vertexCount() == 1;
        if (i != _r.reactantBegin())
            _pushSpace(line, plus_interval_factor);
        _pushMol(line, i);
    }

    if (_r.catalystCount())
    {
        for (int i = _r.catalystBegin(); i < _r.catalystEnd(); i = _r.catalystNext(i))
        {
            auto& mol = _getMol(i);
            Rect2f bbox;
            mol.getBoundingBox(bbox, Vec2f(kDoubleBondLength, kDoubleBondLength));
            _pushSpace(line, bbox.width() / 2);
            _pushMol(line, i, true);
        }
        _pushSpace(line, bond_length);
    }
    else
        _pushSpace(line, arrow_interval_factor);

    _pushSpace(line, bond_length);

    for (int i = _r.productBegin(); i < _r.productEnd(); i = _r.productNext(i))
    {
        bool single_atom = _getMol(i).vertexCount() == 1;
        if (i != _r.productBegin())
            _pushSpace(line, plus_interval_factor);
        _pushMol(line, i);
    }

    _ml.bondLength = bond_length;
    _ml.horizontalIntervalFactor = horizontal_interval_factor;
    _ml.cb_getMol = cb_getMol;
    _ml.cb_process = cb_process;
    _ml.context = this;
    _ml.prepare();
    _ml.scaleSz();
    _ml.calcContentSize();
    _ml.process();
    _updateMetadata();
}

void ReactionLayout::_pushMol(Metalayout::LayoutLine& line, int id, bool is_agent)
{
    // Molecule label alligned to atom center by non-hydrogen
    // Hydrogen may be at left or at right H2O, PH3 - so add space before and after molecule
    _pushSpace(line, atom_label_width);
    Metalayout::LayoutItem& item = line.items.push();
    item.type = 0;
    item.fragment = true;
    item.id = id;
    auto& mol = _getMol(id);
    if (is_agent)
    {
        item.verticalAlign = Metalayout::LayoutItem::ItemVerticalAlign::ETop;
    }

    Rect2f bbox;
    mol.getBoundingBox(bbox);
    item.min.copy(bbox.leftBottom());
    item.max.copy(bbox.rightTop());
    _pushSpace(line, atom_label_width);
}

void ReactionLayout::_pushSpace(Metalayout::LayoutLine& line, float size)
{
    Metalayout::LayoutItem& item = line.items.push();
    item.type = 1;
    item.fragment = false;
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
    Vec2f pos2;
    pos2.copy(pos);
    pos2.y -= item.scaledSize.y / 2;
    if (item.fragment)
    {
        ReactionLayout* layout = (ReactionLayout*)context;
        layout->_ml.adjustMol(layout->_getMol(item.id), item.min, pos2);
    }
}
