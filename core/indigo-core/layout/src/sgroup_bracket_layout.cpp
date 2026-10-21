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
#include "layout/bracket_spacing.h"
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
    for (int sgroup_idx : _nesting.bracketedDeepestFirst())
    {
        const SGroup& sgroup = _molecule.sgroups.getSGroup(sgroup_idx);
        if (!sgroup.hasBrackets() || _hasMoved(sgroup))
            to_place.push_back(sgroup_idx);
    }
    if (to_place.empty() || !_hasCoordinates())
        return;

    const float bond_length = _averageBondLength(_molecule);
    const AtomBoxes atoms(_molecule, bond_length);
    const SGroupTexts texts(_molecule, bond_length);
    for (int sgroup_idx : to_place)
        _setBrackets(_molecule.sgroups.getSGroup(sgroup_idx), _bracketsBoxAround(sgroup_idx, _boundingBox(sgroup_idx, atoms), atoms, texts, bond_length));
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
    Rect2f bounding_box = atoms.box(sgroup.atoms[0]);
    for (int atom : sgroup.atoms)
        bounding_box.extend(atoms.box(atom));
    for (int child_idx : _nesting.children(sgroup_idx))
        if (const std::optional<Rect2f> child_brackets = _spannedByBrackets(_molecule.sgroups.getSGroup(child_idx)))
            bounding_box.extend(*child_brackets);
    return bounding_box;
}

Rect2f SGroupBracketLayout::_bracketsBoxAround(int sgroup_idx, const Rect2f& bounding_box, const AtomBoxes& atoms, const SGroupTexts& texts,
                                               float bond_length) const
{
    const BracketSpacing spacing(texts, bond_length);
    Sides sides;
    for (Side side : {LEFT, BOTTOM, RIGHT, TOP})
        sides[side] = _sidePosition(sgroup_idx, side, bounding_box, atoms, spacing, bond_length);
    _keepClearOfChildren(sides, sgroup_idx, texts, bond_length);
    _movePastLoneAtoms(sides, sgroup_idx, atoms, bond_length);
    const float shortfall = MIN_BRACKET_LENGTH * bond_length - (sides[TOP] - sides[BOTTOM]);
    if (shortfall > 0)
    {
        sides[BOTTOM] -= shortfall / 2;
        sides[TOP] += shortfall / 2;
    }
    return Rect2f(Vec2f(sides[LEFT], sides[BOTTOM]), Vec2f(sides[RIGHT], sides[TOP]));
}

float SGroupBracketLayout::_sidePosition(int sgroup_idx, Side side, const Rect2f& bounding_box, const AtomBoxes& atoms, const BracketSpacing& spacing,
                                         float bond_length) const
{
    const float outward = _outwardSign(side);
    const float step_position = _sideOf(bounding_box, side) + outward * BracketSpacing::STEP * bond_length;
    const SGroup& sgroup = _molecule.sgroups.getSGroup(sgroup_idx);
    const int outermost = _outermostAtom(sgroup, side, atoms);
    const float gap_start = _sideOf(atoms.box(outermost), side);
    const int nearest_outside = _nearestAtomOutside(sgroup, side, bounding_box, gap_start, atoms, bond_length);
    if (nearest_outside < 0)
        return step_position;

    const bool is_bracket_side = side == LEFT || side == RIGHT;
    const std::vector<int> groups_in_gap = is_bracket_side ? _nesting.groupsBetween(outermost, nearest_outside) : std::vector<int>{sgroup_idx};
    BracketSpacing::Gap gap;
    gap.length = outward * (_sideOf(atoms.box(nearest_outside), _opposite(side)) - gap_start);
    gap.rightwards = outward > 0;
    for (int gap_group : groups_in_gap)
    {
        const Side bracket_side = _molecule.sgroups.getSGroup(gap_group).atoms.find(outermost) >= 0 ? side : _opposite(side);
        gap.brackets.push_back({gap_group, bracket_side == RIGHT});
    }
    const size_t own_index = static_cast<size_t>(std::find(groups_in_gap.begin(), groups_in_gap.end(), sgroup_idx) - groups_in_gap.begin());
    const std::optional<float> distance = spacing.distances(gap).at(own_index);
    return distance ? gap_start + outward * *distance : step_position;
}

// The nearest atom outside the group on this side, level with the bounding box (one step wider) and not behind the gap
// start; -1 when there is none
int SGroupBracketLayout::_nearestAtomOutside(const SGroup& sgroup, Side side, const Rect2f& bounding_box, float gap_start, const AtomBoxes& atoms,
                                             float bond_length) const
{
    const float step = BracketSpacing::STEP * bond_length;
    const float outward = _outwardSign(side);
    const Side low_end = side == LEFT || side == RIGHT ? BOTTOM : LEFT;
    const Side high_end = _opposite(low_end);
    const float level_low = _sideOf(bounding_box, low_end) - step, level_high = _sideOf(bounding_box, high_end) + step;
    const auto near_side = [&](int atom) { return _sideOf(atoms.box(atom), _opposite(side)); };
    int nearest = -1;
    for (int i = _molecule.vertexBegin(); i != _molecule.vertexEnd(); i = _molecule.vertexNext(i))
    {
        const Rect2f box = atoms.box(i);
        if (sgroup.atoms.find(i) >= 0 || _sideOf(box, low_end) >= level_high || _sideOf(box, high_end) <= level_low)
            continue;
        if (outward * (near_side(i) - gap_start) < -EPSILON)
            continue;
        if (nearest < 0 || outward * (near_side(i) - near_side(nearest)) < 0)
            nearest = i;
    }
    return nearest;
}

// A distance from BracketSpacing is counted from the group's own atoms, so it can fall short of the brackets of a child;
// and the bounding box leaves out the texts of the children
void SGroupBracketLayout::_keepClearOfChildren(Sides& sides, int sgroup_idx, const SGroupTexts& texts, float bond_length) const
{
    const float clearance = BracketSpacing::TEXT_CLEARANCE * bond_length;
    for (int child_idx : _nesting.children(sgroup_idx))
    {
        const SGroup& child = _molecule.sgroups.getSGroup(child_idx);
        std::optional<Rect2f> box = _spannedByBrackets(child);
        if (!box)
            continue;
        if (const std::optional<Rect2f> text = texts.box(child))
            box->extend(*text);
        sides[LEFT] = std::min(sides[LEFT], box->left() - clearance);
        sides[RIGHT] = std::max(sides[RIGHT], box->right() + clearance);
        sides[BOTTOM] = std::min(sides[BOTTOM], box->bottom() - clearance);
        sides[TOP] = std::max(sides[TOP], box->top() + clearance);
    }
}

void SGroupBracketLayout::_movePastLoneAtoms(Sides& sides, int sgroup_idx, const AtomBoxes& atoms, float bond_length) const
{
    const float step = BracketSpacing::STEP * bond_length, radius = UNLABELED_ATOM_RADIUS * bond_length;
    const std::vector<bool> lone = _nesting.loneAtoms(sgroup_idx);
    std::vector<Rect2f> lone_boxes;
    for (int i = _molecule.vertexBegin(); i != _molecule.vertexEnd(); i = _molecule.vertexNext(i))
        if (lone[i])
        {
            const Rect2f box = atoms.box(i);
            const float padding = box.width() < EPSILON ? radius : 0;
            lone_boxes.emplace_back(Vec2f(box.left() - padding, box.bottom() - padding), Vec2f(box.right() + padding, box.top() + padding));
        }
    for (Side side : {LEFT, RIGHT})
    {
        // A side moved past one atom can land in the box of another
        bool moved;
        do
        {
            moved = false;
            for (const Rect2f& box : lone_boxes)
                if (box.left() <= sides[side] && sides[side] <= box.right() && box.bottom() < sides[TOP] && box.top() > sides[BOTTOM])
                {
                    sides[side] = _sideOf(box, side) + _outwardSign(side) * step;
                    moved = true;
                }
        } while (moved);
    }
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

std::optional<Rect2f> SGroupBracketLayout::_spannedByBrackets(const SGroup& sgroup)
{
    if (!sgroup.hasBrackets())
        return std::nullopt;
    Rect2f box(sgroup.brackets[0][0], sgroup.brackets[0][1]);
    for (int i = 1; i < sgroup.brackets.size(); ++i)
        box.extend(Rect2f(sgroup.brackets[i][0], sgroup.brackets[i][1]));
    return box;
}

int SGroupBracketLayout::_outermostAtom(const SGroup& sgroup, Side side, const AtomBoxes& atoms)
{
    int outermost = sgroup.atoms[0];
    for (int atom : sgroup.atoms)
        if (_outwardSign(side) * (_sideOf(atoms.box(atom), side) - _sideOf(atoms.box(outermost), side)) > 0)
            outermost = atom;
    return outermost;
}

float SGroupBracketLayout::_sideOf(const Rect2f& box, Side side)
{
    switch (side)
    {
    case LEFT:
        return box.left();
    case BOTTOM:
        return box.bottom();
    case RIGHT:
        return box.right();
    default:
        return box.top();
    }
}

SGroupBracketLayout::Side SGroupBracketLayout::_opposite(Side side)
{
    switch (side)
    {
    case LEFT:
        return RIGHT;
    case BOTTOM:
        return TOP;
    case RIGHT:
        return LEFT;
    default:
        return BOTTOM;
    }
}

float SGroupBracketLayout::_outwardSign(Side side)
{
    return side == RIGHT || side == TOP ? 1.f : -1.f;
}
