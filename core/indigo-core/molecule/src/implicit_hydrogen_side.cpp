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

#include "molecule/implicit_hydrogen_side.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#include "molecule/base_molecule.h"

using namespace indigo;

ImplicitHydrogenSide::ImplicitHydrogenSide(BaseMolecule& molecule, int atom)
    : ImplicitHydrogenSide(_bondDirections(molecule, atom), _carriesRadical(molecule, atom))
{
}

ImplicitHydrogenSide::ImplicitHydrogenSide(const std::vector<Vec2f>& bond_directions, bool has_radical) : _hasRadical(has_radical)
{
    for (const Vec2f& direction : bond_directions)
        _bondCentidegrees.push_back(_centidegrees(direction));
}

ImplicitHydrogenSide::Side ImplicitHydrogenSide::side() const
{
    return _bondCentidegrees.size() <= 1 ? _terminalSide() : _internalSide();
}

bool ImplicitHydrogenSide::isStacked(Side side)
{
    return side == Side::TOP || side == Side::BOTTOM;
}

std::vector<Vec2f> ImplicitHydrogenSide::_bondDirections(BaseMolecule& molecule, int atom)
{
    std::vector<Vec2f> directions;
    const Vec2f position = molecule.getAtomXyz(atom).projectZ();
    const Vertex& vertex = molecule.getVertex(atom);
    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        Vec2f direction = molecule.getAtomXyz(vertex.neiVertex(i)).projectZ();
        direction.sub(position);
        directions.push_back(direction);
    }
    return directions;
}

bool ImplicitHydrogenSide::_carriesRadical(BaseMolecule& molecule, int atom)
{
    if (molecule.isRSite(atom) || molecule.isPseudoAtom(atom) || molecule.isTemplateAtom(atom))
        return false;
    return molecule.getAtomRadical_NoThrow(atom, 0) > 0;
}

int ImplicitHydrogenSide::_centidegrees(const Vec2f& direction)
{
    const double degrees = RAD2DEG(_2DOUBLE(ATAN2(direction.y, direction.x)));
    const double in_full_turn = std::fmod(degrees + 360., 360.);
    return static_cast<int>(std::lround(in_full_turn * 100.)) % FULL_TURN;
}

int ImplicitHydrogenSide::_axis(Side quadrant)
{
    switch (quadrant)
    {
    case Side::RIGHT:
        return RIGHTWARDS;
    case Side::TOP:
        return UP;
    case Side::LEFT:
        return LEFTWARDS;
    default:
        return DOWN;
    }
}

bool ImplicitHydrogenSide::_isVertical(int centidegrees)
{
    return std::abs(centidegrees - UP) <= VERTICAL_TOLERANCE || std::abs(centidegrees - DOWN) <= VERTICAL_TOLERANCE;
}

bool ImplicitHydrogenSide::_pointsRight(int centidegrees)
{
    return centidegrees < UP || centidegrees > DOWN;
}

ImplicitHydrogenSide::Side ImplicitHydrogenSide::_terminalSide() const
{
    if (_bondCentidegrees.empty() || _isVertical(_bondCentidegrees.front()))
        return Side::RIGHT;
    return _pointsRight(_bondCentidegrees.front()) ? Side::LEFT : Side::RIGHT;
}

ImplicitHydrogenSide::Side ImplicitHydrogenSide::_internalSide() const
{
    for (Side quadrant : {Side::RIGHT, Side::LEFT, Side::TOP, Side::BOTTOM})
    {
        if (!_isFree(quadrant))
            continue;
        if (quadrant == Side::TOP && _isFree(Side::BOTTOM) && _bondsPointUp())
            return Side::BOTTOM;
        return quadrant;
    }
    return Side::RIGHT;
}

bool ImplicitHydrogenSide::_isFree(Side quadrant) const
{
    if (quadrant == Side::TOP && _hasRadical)
        return false;
    const int start = _axis(quadrant) - QUARTER_TURN / 2;
    return std::none_of(_bondCentidegrees.begin(), _bondCentidegrees.end(),
                        [start](int centidegrees) { return (centidegrees - start + FULL_TURN) % FULL_TURN < QUARTER_TURN; });
}

bool ImplicitHydrogenSide::_bondsPointUp() const
{
    float balance = 0.f;
    for (int centidegrees : _bondCentidegrees)
        balance += SIN(DEG2RAD(centidegrees / 100.));
    return balance > UPWARD_BALANCE_TOLERANCE;
}
