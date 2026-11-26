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

#include "molecule/base_molecule.h"
#include "molecule/elements.h"

using namespace indigo;

ImplicitHydrogenSide::ImplicitHydrogenSide(BaseMolecule& molecule, int atom)
    : _hasRadical(_carriesRadical(molecule, atom)), _element(molecule.getAtomNumber(atom))
{
    const Vec2f position = molecule.getAtomXyz(atom).projectZ();
    const Vertex& vertex = molecule.getVertex(atom);
    _bondCentidegrees.reserve(vertex.degree());
    for (int i = vertex.neiBegin(); i != vertex.neiEnd(); i = vertex.neiNext(i))
    {
        Vec2f direction = molecule.getAtomXyz(vertex.neiVertex(i)).projectZ();
        direction.sub(position);
        _bondCentidegrees.push_back(_centidegrees(direction));
    }
}

ImplicitHydrogenSide::Side ImplicitHydrogenSide::side() const
{
    if (_bondCentidegrees.empty())
        return _loneSide();
    return _bondCentidegrees.size() == 1 ? _terminalSide() : _internalSide();
}

bool ImplicitHydrogenSide::_bondOccupies(Side quadrant) const
{
    const int counter_clockwise_end = _axis(quadrant) + QUARTER_TURN / 2;
    return std::any_of(_bondCentidegrees.begin(), _bondCentidegrees.end(),
                       [counter_clockwise_end](int centidegrees) { return (counter_clockwise_end - centidegrees + FULL_TURN) % FULL_TURN < QUARTER_TURN; });
}

bool ImplicitHydrogenSide::_carriesRadical(BaseMolecule& molecule, int atom)
{
    if (molecule.isRSite(atom) || molecule.isPseudoAtom(atom) || molecule.isTemplateAtom(atom))
        return false;
    return molecule.getAtomRadical_NoThrow(atom, 0) > 0;
}

int ImplicitHydrogenSide::_centidegrees(const Vec2f& direction)
{
    const double centidegrees = RAD2DEG(_2DOUBLE(ATAN2(direction.y, direction.x))) * CENTIDEGREES_PER_DEGREE;
    return static_cast<int>(std::lround(std::fmod(centidegrees + FULL_TURN, FULL_TURN))) % FULL_TURN;
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

ImplicitHydrogenSide::Side ImplicitHydrogenSide::_opposite(Side quadrant)
{
    switch (quadrant)
    {
    case Side::RIGHT:
        return Side::LEFT;
    case Side::TOP:
        return Side::BOTTOM;
    case Side::LEFT:
        return Side::RIGHT;
    default:
        return Side::TOP;
    }
}

bool ImplicitHydrogenSide::_liesInRightHalf(int centidegrees)
{
    return centidegrees <= UP || centidegrees > DOWN;
}

ImplicitHydrogenSide::Side ImplicitHydrogenSide::_loneSide() const
{
    return ElementHydrogenOnLeft(_element) ? Side::LEFT : Side::RIGHT;
}

ImplicitHydrogenSide::Side ImplicitHydrogenSide::_terminalSide() const
{
    return _liesInRightHalf(_bondCentidegrees.front()) ? Side::LEFT : Side::RIGHT;
}

ImplicitHydrogenSide::Side ImplicitHydrogenSide::_internalSide() const
{
    for (Side quadrant : {Side::RIGHT, Side::LEFT, Side::TOP, Side::BOTTOM})
    {
        if (!_isFree(quadrant))
            continue;
        const Side opposite = _opposite(quadrant);
        return _isFree(opposite) && _bondsPointTowards(quadrant) ? opposite : quadrant;
    }
    return Side::RIGHT;
}

bool ImplicitHydrogenSide::_isFree(Side quadrant) const
{
    if (quadrant == Side::TOP && _hasRadical)
        return false;
    return !_bondOccupies(quadrant);
}

bool ImplicitHydrogenSide::_bondsPointTowards(Side quadrant) const
{
    float balance = 0.f;
    for (int centidegrees : _bondCentidegrees)
        balance += COS(DEG2RAD(_2DOUBLE(centidegrees - _axis(quadrant)) / CENTIDEGREES_PER_DEGREE));
    return balance > BALANCE_TOLERANCE;
}
