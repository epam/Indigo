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
#include "molecule/molecule.h"
#include "reaction/reaction.h"

using namespace indigo;

ReactionLayout::ReactionLayout(BaseReaction& r, bool smart_layout)
    : bond_length(1), plus_interval_factor(2), arrow_interval_factor(2), preserve_molecule_layout(false), _r(r), _smart_layout(smart_layout),
      horizontal_interval_factor(0.5f)
{
    max_iterations = 0;
}

void ReactionLayout::make()
{
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
        {
            _pushSpace(line, plus_interval_factor + (single_atom ? bond_length : 0));
        }
        _pushMol(line, i);
    }

    if (_r.catalystCount())
    {
        for (int i = _r.catalystBegin(); i < _r.catalystEnd(); i = _r.catalystNext(i))
        {
            auto& mol = _getMol(i);
            Rect2f bbox;
            mol.getBoundingBox(bbox);
            _pushSpace(line, bond_length + bbox.width() / 2);
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
            _pushSpace(line, plus_interval_factor + (single_atom ? bond_length : 0));
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
}

Metalayout::LayoutItem& ReactionLayout::_pushMol(Metalayout::LayoutLine& line, int id, bool is_agent)
{
    const int kMaxSymbols = 3;
    const auto kMinHeight = bond_length / 8;
    Metalayout::LayoutItem& item = line.items.push();
    item.type = 0;
    item.fragment = true;
    item.id = id;
    auto& mol = _getMol(id);
    Metalayout::getBoundRect(item.min, item.max, mol);
    if (is_agent)
    {
        item.verticalAlign = Metalayout::LayoutItem::ItemVerticalAlign::ETop;
    }

    if (mol.vertexCount() == 1)
    {
        int max_h = mol.getAtomMaxH(0);
        int mult = max_h + 1;
        if (mult > kMaxSymbols)
            mult = kMaxSymbols;
        item.minScaledSize.set(0, kMinHeight);
    }
    else
        item.minScaledSize.set(bond_length, bond_length);

    return item;
}

Metalayout::LayoutItem& ReactionLayout::_pushSpace(Metalayout::LayoutLine& line, float size)
{
    Metalayout::LayoutItem& item = line.items.push();
    item.type = 1;
    item.fragment = false;
    item.scaledSize.set(size, 0);
    return item;
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
