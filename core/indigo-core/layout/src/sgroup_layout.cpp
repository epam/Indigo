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

#include "layout/sgroup_layout.h"

#include <cmath>

#include "molecule/base_molecule.h"

using namespace indigo;

SGroupLayout::SGroupLayout(BaseMolecule& molecule, float font_size, LABEL_MODE label_mode)
    : _molecule(molecule), _nesting(molecule), _fontSize(font_size), _labelMode(label_mode)
{
    MoleculeSGroups& sgroups = molecule.sgroups;
    for (int i = sgroups.begin(); i != sgroups.end(); i = sgroups.next(i))
    {
        const SGroup& sgroup = sgroups.getSGroup(i);
        if (sgroup.sgroup_type != SGroup::SG_TYPE_DAT)
            continue;
        const DataSGroup& value = static_cast<const DataSGroup&>(sgroup);
        if (value.display_pos.has_value())
            _initialValues.emplace(i, value.display_pos.value());
    }
    if (_nesting.bracketedDeepestFirst().empty() && _initialValues.empty())
        return;
    _initialAtoms.resize(molecule.vertexEnd());
    for (int i = molecule.vertexBegin(); i != molecule.vertexEnd(); i = molecule.vertexNext(i))
        _initialAtoms[i] = _position(i);
}

float SGroupLayout::labelFontSizeInAngstroms(const LayoutOptions& options)
{
    return options.fontSize > EPSILON ? options.getFontSizeInAngstroms() : DEFAULT_FONT_SIZE;
}

void SGroupLayout::update()
{
    _placeBrackets();
    _moveValues();
}

void SGroupLayout::_placeBrackets()
{
    std::vector<int> to_place;
    for (int sgroup_idx : _nesting.bracketedDeepestFirst())
    {
        const SGroup& sgroup = _molecule.sgroups.getSGroup(sgroup_idx);
        if (!sgroup.hasBrackets() || _hasMoved(sgroup))
            to_place.push_back(sgroup_idx);
    }
    if (to_place.empty() || _allAtomsAtOnePoint())
        return;
    for (int sgroup_idx : to_place)
        _setBrackets(_molecule.sgroups.getSGroup(sgroup_idx), _bracketsBox(sgroup_idx));
}

void SGroupLayout::_moveValues()
{
    if (_initialValues.empty())
        return;
    const std::vector<int> parents = _molecule.sgroups.resolveParents();
    for (const auto& [sgroup_idx, initial] : _initialValues)
    {
        DataSGroup& value = static_cast<DataSGroup&>(_molecule.sgroups.getSGroup(sgroup_idx));
        const SGroup* reference = _referenceGroup(value, parents[sgroup_idx]);
        if (reference == nullptr || !_hasMoved(*reference))
            continue;
        // A whole-molecule move has carried it with the scale, which the fit of a single atom could not give
        if (_standApart(value.display_pos.value(), initial))
            continue;
        _moveValue(value, *reference);
    }
}

void SGroupLayout::_moveValue(DataSGroup& value, const SGroup& reference) const
{
    Vec3f point = value.displayPoint([this](int atom) { return _initialAtoms.at(atom); });
    point.transformPoint(_fitOfAtoms(reference));
    value.setDisplayPoint(point.projectZ(), [this](int atom) { return _position(atom); });
}

bool SGroupLayout::_hasMoved(const SGroup& sgroup) const
{
    for (int atom : sgroup.atoms)
        if (_standApart(_position(atom), _initialAtoms.at(atom)))
            return true;
    return false;
}

bool SGroupLayout::_standApart(const Vec2f& first, const Vec2f& second)
{
    return Vec2f::distSqr(first, second) > EPSILON;
}

Vec2f SGroupLayout::_position(int atom) const
{
    Vec2f position;
    Vec2f::projectZ(position, _molecule.getAtomXyz(atom));
    return position;
}

const SGroup* SGroupLayout::_referenceGroup(const DataSGroup& value, int parent) const
{
    if (value.originAtom() >= 0)
        return &value;
    if (value.relative)
        return nullptr;

    if (parent < 0 || _molecule.sgroups.getSGroup(parent).atoms.size() == 0)
        return nullptr;
    return &_molecule.sgroups.getSGroup(parent);
}

Transform3f SGroupLayout::_fitOfAtoms(const SGroup& sgroup) const
{
    std::vector<Vec3f> before, after;
    before.reserve(sgroup.atoms.size());
    after.reserve(sgroup.atoms.size());
    for (int atom : sgroup.atoms)
    {
        before.emplace_back(_initialAtoms.at(atom));
        after.emplace_back(_position(atom));
    }
    Transform3f fit;
    fit.bestFit(sgroup.atoms.size(), before.data(), after.data(), nullptr);
    return fit;
}

bool SGroupLayout::_allAtomsAtOnePoint() const
{
    if (_molecule.vertexCount() < 2)
        return false;
    const Vec3f& first = _molecule.getAtomXyz(_molecule.vertexBegin());
    for (int i = _molecule.vertexNext(_molecule.vertexBegin()); i != _molecule.vertexEnd(); i = _molecule.vertexNext(i))
    {
        const Vec3f& other = _molecule.getAtomXyz(i);
        if (std::abs(other.x - first.x) > EPSILON || std::abs(other.y - first.y) > EPSILON)
            return false;
    }
    return true;
}

Rect2f SGroupLayout::_bracketsBox(int sgroup_idx) const
{
    const float bond_length = LayoutOptions::DEFAULT_BOND_LENGTH;
    Rect2f box = _enlarged(_atomsBox(_molecule.sgroups.getSGroup(sgroup_idx)), HORIZONTAL_MARGIN * bond_length, VERTICAL_MARGIN * bond_length);
    for (int child_idx : _nesting.children(sgroup_idx))
        if (const std::optional<Rect2f> child_brackets = _spannedByBrackets(_molecule.sgroups.getSGroup(child_idx)))
            box.extend(_enlarged(*child_brackets, HORIZONTAL_SPACE * bond_length, VERTICAL_SPACE * bond_length));
    return box;
}

Rect2f SGroupLayout::_atomsBox(const SGroup& sgroup) const
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

std::optional<Rect2f> SGroupLayout::_spannedByBrackets(const SGroup& sgroup)
{
    if (!sgroup.hasBrackets())
        return std::nullopt;
    Rect2f box(sgroup.brackets[0][0], sgroup.brackets[0][1]);
    for (int i = 1; i < sgroup.brackets.size(); ++i)
        box.extend(Rect2f(sgroup.brackets[i][0], sgroup.brackets[i][1]));
    return box;
}

Rect2f SGroupLayout::_enlarged(const Rect2f& box, float horizontal, float vertical)
{
    return Rect2f(Vec2f(box.left() - horizontal, box.bottom() - vertical), Vec2f(box.right() + horizontal, box.top() + vertical));
}

// The order and direction of the files Indigo writes: the opening bracket runs up and comes first, the closing one
// runs down. The renderer depends on neither: it finds the closing bracket by position
void SGroupLayout::_setBrackets(SGroup& sgroup, const Rect2f& box)
{
    sgroup.brackets.clear();
    Vec2f* opening = sgroup.brackets.push();
    opening[0].set(box.left(), box.bottom());
    opening[1].set(box.left(), box.top());
    Vec2f* closing = sgroup.brackets.push();
    closing[0].set(box.right(), box.top());
    closing[1].set(box.right(), box.bottom());
}
