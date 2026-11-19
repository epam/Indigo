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

    // The side of an atom symbol on which its implicit hydrogens are drawn. Ketcher draws the same structures on its
    // canvas, so the rule and its constants are those of getImplicitHydrogenSide in ketcher-core
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
        // The bonds of the atom as drawn: x to the right, y up
        ImplicitHydrogenSide(const std::vector<Vec2f>& bond_directions, bool has_radical);

        Side side() const;
        static bool isStacked(Side side);

    private:
        static constexpr int FULL_TURN = 36000;
        static constexpr int QUARTER_TURN = FULL_TURN / 4;
        static constexpr int RIGHTWARDS = 0;
        static constexpr int UP = QUARTER_TURN;
        static constexpr int LEFTWARDS = 2 * QUARTER_TURN;
        static constexpr int DOWN = 3 * QUARTER_TURN;
        static constexpr int VERTICAL_TOLERANCE = 50;
        static constexpr float UPWARD_BALANCE_TOLERANCE = 0.01f;

        static std::vector<Vec2f> _bondDirections(BaseMolecule& molecule, int atom);
        static bool _carriesRadical(BaseMolecule& molecule, int atom);
        static int _centidegrees(const Vec2f& direction);
        static int _axis(Side quadrant);
        static bool _isVertical(int centidegrees);
        static bool _pointsRight(int centidegrees);

        Side _terminalSide() const;
        Side _internalSide() const;
        bool _isFree(Side quadrant) const;
        bool _bondsPointUp() const;

        // In [0, FULL_TURN): 0 points right, UP points up
        std::vector<int> _bondCentidegrees;
        bool _hasRadical;
    };
} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
