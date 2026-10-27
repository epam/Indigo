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

#include "layout/sgroup_bracket_layout.h"

#include <cmath>

#include "molecule/base_molecule.h"

using namespace indigo;

SGroupBracketLayout::SGroupBracketLayout(BaseMolecule& molecule, float font_size, LABEL_MODE label_mode)
    : _molecule(molecule), _nesting(molecule), _fontSize(font_size), _labelMode(label_mode)
{
    if (_nesting.bracketedDeepestFirst().empty())
        return;
    _initialPositions.resize(molecule.vertexEnd());
    for (int i = molecule.vertexBegin(); i != molecule.vertexEnd(); i = molecule.vertexNext(i))
        Vec2f::projectZ(_initialPositions[i], molecule.getAtomXyz(i));
}

void SGroupBracketLayout::update()
{
    std::vector<int> to_place;
    for (int sgroup_idx : _nesting.bracketedDeepestFirst())
    {
        const SGroup& sgroup = _molecule.sgroups.getSGroup(sgroup_idx);
        if (!sgroup.hasBrackets() || _hasMoved(sgroup))
            to_place.push_back(sgroup_idx);
    }
    if (to_place.empty() || !_hasCoordinates())
        return;
    for (int sgroup_idx : to_place)
        _setBrackets(_molecule.sgroups.getSGroup(sgroup_idx), _bracketsBox(sgroup_idx));
}

bool SGroupBracketLayout::_hasMoved(const SGroup& sgroup) const
{
    for (int atom : sgroup.atoms)
    {
        Vec2f now;
        Vec2f::projectZ(now, _molecule.getAtomXyz(atom));
        if (Vec2f::distSqr(now, _initialPositions.at(atom)) > EPSILON)
            return true;
    }
    return false;
}

// A structure without coordinates has all its atoms at one point
bool SGroupBracketLayout::_hasCoordinates() const
{
    if (_molecule.vertexCount() < 2)
        return true;
    const Vec3f& first = _molecule.getAtomXyz(_molecule.vertexBegin());
    for (int i = _molecule.vertexNext(_molecule.vertexBegin()); i != _molecule.vertexEnd(); i = _molecule.vertexNext(i))
    {
        const Vec3f& other = _molecule.getAtomXyz(i);
        if (std::abs(other.x - first.x) > EPSILON || std::abs(other.y - first.y) > EPSILON)
            return true;
    }
    return false;
}

Rect2f SGroupBracketLayout::_bracketsBox(int sgroup_idx) const
{
    const float bond_length = LayoutOptions::DEFAULT_BOND_LENGTH;
    Rect2f box = _enlarged(_atomsBox(_molecule.sgroups.getSGroup(sgroup_idx)), HORIZONTAL_MARGIN * bond_length, VERTICAL_MARGIN * bond_length);
    for (int child_idx : _nesting.children(sgroup_idx))
        if (const std::optional<Rect2f> child_brackets = _spannedByBrackets(_molecule.sgroups.getSGroup(child_idx)))
            box.extend(_enlarged(*child_brackets, HORIZONTAL_SPACE * bond_length, VERTICAL_SPACE * bond_length));
    return box;
}

Rect2f SGroupBracketLayout::_atomsBox(const SGroup& sgroup) const
{
    const auto atom_box = [this](int atom) {
        Vec2f bottom_left, top_right;
        _molecule.getAtomBoundingBox(atom, _fontSize, _labelMode, bottom_left, top_right);
        return Rect2f(bottom_left, top_right);
    };
    Rect2f box = atom_box(sgroup.atoms[0]);
    for (int atom : sgroup.atoms)
        box.extend(atom_box(atom));
    return box;
}

std::optional<Rect2f> SGroupBracketLayout::_spannedByBrackets(const SGroup& sgroup)
{
    if (!sgroup.hasBrackets())
        return std::nullopt;
    Rect2f box(sgroup.brackets[0][0], sgroup.brackets[0][1]);
    for (int i = 1; i < sgroup.brackets.size(); ++i)
        box.extend(Rect2f(sgroup.brackets[i][0], sgroup.brackets[i][1]));
    return box;
}

Rect2f SGroupBracketLayout::_enlarged(const Rect2f& box, float horizontal, float vertical)
{
    return Rect2f(Vec2f(box.left() - horizontal, box.bottom() - vertical), Vec2f(box.right() + horizontal, box.top() + vertical));
}

// The renderer turns the hooks of a bracket to the right of its direction from the first point to the second and puts
// the texts at the last bracket: so the opening bracket runs up and comes first, the closing one runs down
void SGroupBracketLayout::_setBrackets(SGroup& sgroup, const Rect2f& box)
{
    sgroup.brackets.clear();
    Vec2f* opening = sgroup.brackets.push();
    opening[0].set(box.left(), box.bottom());
    opening[1].set(box.left(), box.top());
    Vec2f* closing = sgroup.brackets.push();
    closing[0].set(box.right(), box.top());
    closing[1].set(box.right(), box.bottom());
}
