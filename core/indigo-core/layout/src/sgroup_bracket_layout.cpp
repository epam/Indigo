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

#include <algorithm>
#include <cmath>

#include "layout/atom_boxes.h"
#include "layout/metalayout.h"
#include "layout/sgroup_texts.h"
#include "molecule/base_molecule.h"

using namespace indigo;

SGroupBracketLayout::SGroupBracketLayout(BaseMolecule& molecule) : _molecule(molecule), _nesting(molecule)
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
    for (int idx : _nesting.bracketedDeepestFirst())
    {
        const SGroup& sgroup = _molecule.sgroups.getSGroup(idx);
        if (!sgroup.hasBrackets() || _hasMoved(sgroup))
            to_place.push_back(idx);
    }
    if (to_place.empty() || !_hasCoordinates())
        return;

    const float bond_length = _averageBondLength(_molecule);
    const AtomBoxes atoms(_molecule, bond_length);
    const SGroupTexts texts(_molecule, bond_length);
    for (int idx : to_place)
        _setBrackets(_molecule.sgroups.getSGroup(idx), _bracketsBoxAround(idx, _boundingBox(idx, atoms), texts, bond_length));
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

// The copies of a multiple group stand at one point until it is laid out: bonds of zero length do not count
float SGroupBracketLayout::_averageBondLength(BaseMolecule& molecule)
{
    float total = 0;
    int counted = 0;
    for (int i = molecule.edgeBegin(); i != molecule.edgeEnd(); i = molecule.edgeNext(i))
    {
        const Edge& edge = molecule.getEdge(i);
        Vec2f begin, end;
        Vec2f::projectZ(begin, molecule.getAtomXyz(edge.beg));
        Vec2f::projectZ(end, molecule.getAtomXyz(edge.end));
        const float length = Vec2f::dist(begin, end);
        if (length > EPSILON)
        {
            total += length;
            counted++;
        }
    }
    return counted > 0 ? total / counted : LayoutOptions::DEFAULT_BOND_LENGTH;
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

Rect2f SGroupBracketLayout::_boundingBox(int sgroup_idx, const AtomBoxes& atoms) const
{
    const SGroup& sgroup = _molecule.sgroups.getSGroup(sgroup_idx);
    Rect2f bbox = atoms.box(sgroup.atoms[0]);
    for (int atom : sgroup.atoms)
        bbox = Rect2f(bbox, atoms.box(atom));
    for (int child : _nesting.children(sgroup_idx))
        if (const std::optional<Rect2f> nested = _bracketsBox(_molecule.sgroups.getSGroup(child)))
            bbox = Rect2f(bbox, *nested);
    return bbox;
}

Rect2f SGroupBracketLayout::_bracketsBoxAround(int sgroup_idx, const Rect2f& bbox, const SGroupTexts& texts, float bond_length) const
{
    const float gap = GAP * bond_length, clearance = TEXT_CLEARANCE * bond_length;
    float left = bbox.left() - gap, right = bbox.right() + gap, bottom = bbox.bottom() - gap, top = bbox.top() + gap;
    for (int child : _nesting.children(sgroup_idx))
        if (const std::optional<Rect2f> text = texts.box(_molecule.sgroups.getSGroup(child)))
        {
            left = std::min(left, text->left() - clearance);
            right = std::max(right, text->right() + clearance);
            bottom = std::min(bottom, text->bottom() - clearance);
            top = std::max(top, text->top() + clearance);
        }
    const float shortfall = MIN_LENGTH * bond_length - (top - bottom);
    if (shortfall > 0)
    {
        bottom -= shortfall / 2;
        top += shortfall / 2;
    }
    return Rect2f(Vec2f(left, bottom), Vec2f(right, top));
}

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

std::optional<Rect2f> SGroupBracketLayout::_bracketsBox(const SGroup& sgroup)
{
    if (!sgroup.hasBrackets())
        return std::nullopt;
    Rect2f box(sgroup.brackets[0][0], sgroup.brackets[0][1]);
    for (int i = 1; i < sgroup.brackets.size(); ++i)
        box = Rect2f(box, Rect2f(sgroup.brackets[i][0], sgroup.brackets[i][1]));
    return box;
}
