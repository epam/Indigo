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

#ifndef __implicit_hydrogen_side_h__
#define __implicit_hydrogen_side_h__

#include <vector>

#include "base_c/defs.h"
#include "math/algebra.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class BaseMolecule;

    // The side of an atom symbol on which its implicit hydrogens are drawn, decided from the bonds of the atom as
    // drawn: 0 degrees points right, 90 up, and angles are compared in hundredths of a degree.
    // - No bonds: on the left for an element written with its hydrogens first (H2O, HCl), otherwise on the right.
    // - One bond: on the left when the bond lies in the right half, which includes straight up and excludes
    //   straight down, otherwise on the right.
    // - More bonds: the first quadrant without a bond in the order right, left, top, bottom, and the right one when
    //   none is free. A quadrant reaches 45 degrees to both sides of its axis and includes its counter-clockwise
    //   end: a bond at 45 degrees is in the right one. A radical takes the top quadrant.
    // - When the quadrant opposite to the chosen one is free as well, the hydrogens stand in the one the bonds
    //   point away from: below a carbon whose two bonds go up, left of a nitrogen whose two bonds go right. The
    //   bonds point towards a quadrant when the cosines of their angles to its axis add up to more than
    //   BALANCE_TOLERANCE; otherwise they balance and the chosen quadrant stays.
    class DLLEXPORT ImplicitHydrogenSide
    {
    public:
        enum class Side
        {
            RIGHT,
            LEFT,
            TOP,
            BOTTOM
        };

        ImplicitHydrogenSide(BaseMolecule& molecule, int atom);

        Side side() const;

    private:
        static constexpr int CENTIDEGREES_PER_DEGREE = 100;
        static constexpr int FULL_TURN = 360 * CENTIDEGREES_PER_DEGREE;
        static constexpr int QUARTER_TURN = FULL_TURN / 4;
        static constexpr int RIGHTWARDS = 0;
        static constexpr int UP = QUARTER_TURN;
        static constexpr int LEFTWARDS = 2 * QUARTER_TURN;
        static constexpr int DOWN = 3 * QUARTER_TURN;
        static constexpr float BALANCE_TOLERANCE = 0.01f;

        static bool _carriesRadical(BaseMolecule& molecule, int atom);
        static int _centidegrees(const Vec2f& direction);
        static int _axis(Side quadrant);
        static Side _opposite(Side quadrant);
        static bool _liesInRightHalf(int centidegrees);

        Side _loneSide() const;
        Side _terminalSide() const;
        Side _internalSide() const;
        bool _bondOccupies(Side quadrant) const;
        bool _isFree(Side quadrant) const;
        bool _bondsPointTowards(Side quadrant) const;

        // In [0, FULL_TURN)
        std::vector<int> _bondCentidegrees;
        bool _hasRadical;
        int _element;
    };
} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
