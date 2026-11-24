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

// The side of an atom symbol on which its implicit hydrogens are drawn. Angles in the tables are as drawn: 0 degrees
// points right, 90 up.

#include <cmath>
#include <ostream>
#include <vector>

#include <gtest/gtest.h>

#include <math/algebra.h>
#include <molecule/elements.h>
#include <molecule/implicit_hydrogen_side.h>
#include <molecule/molecule.h>

#include "common.h"

using namespace indigo;

namespace indigo
{
    void PrintTo(ImplicitHydrogenSide::Side side, std::ostream* output)
    {
        switch (side)
        {
        case ImplicitHydrogenSide::Side::RIGHT:
            *output << "RIGHT";
            break;
        case ImplicitHydrogenSide::Side::LEFT:
            *output << "LEFT";
            break;
        case ImplicitHydrogenSide::Side::TOP:
            *output << "TOP";
            break;
        case ImplicitHydrogenSide::Side::BOTTOM:
            *output << "BOTTOM";
            break;
        }
    }
} // namespace indigo

class IndigoCoreImplicitHydrogenSideTest : public IndigoCoreTest
{
protected:
    using Side = ImplicitHydrogenSide::Side;

    static constexpr int NO_RADICAL = 0;

    struct Case
    {
        const char* name;
        std::vector<double> bond_angles;
        int radical;
        Side expected;
    };

    static Vec2f direction(double degrees, double length = 1.)
    {
        const double radians = degrees * M_PI / 180.;
        return Vec2f(static_cast<float>(length * std::cos(radians)), static_cast<float>(length * std::sin(radians)));
    }

    static void addNeighbour(Molecule& mol, int atom, const Vec2f& offset)
    {
        const Vec3f position = mol.getAtomXyz(atom);
        const int neighbour = mol.addAtom(ELEM_C);
        mol.setAtomXyz(neighbour, position.x + offset.x, position.y + offset.y, 0.f);
        mol.addBond(atom, neighbour, BOND_SINGLE);
    }

    static Side sideOf(const std::vector<double>& bond_angles, int radical = NO_RADICAL, int element = ELEM_N)
    {
        Molecule mol;
        const int atom = mol.addAtom(element);
        mol.setAtomXyz(atom, 0.f, 0.f, 0.f);
        for (double angle : bond_angles)
            addNeighbour(mol, atom, direction(angle));
        if (radical != NO_RADICAL)
            mol.setAtomRadical(atom, radical);
        return ImplicitHydrogenSide(mol, atom).side();
    }

    static void expectSides(const std::vector<Case>& cases)
    {
        for (const Case& tc : cases)
            EXPECT_EQ(tc.expected, sideOf(tc.bond_angles, tc.radical)) << tc.name;
    }
};

TEST_F(IndigoCoreImplicitHydrogenSideTest, AtomWithoutBonds_HydrogensAreOnTheLeftForAnElementWrittenHydrogensFirst)
{
    EXPECT_EQ(Side::LEFT, sideOf({}, NO_RADICAL, ELEM_O));
    EXPECT_EQ(Side::RIGHT, sideOf({}, NO_RADICAL, ELEM_N));
    EXPECT_EQ(Side::LEFT, sideOf({}, RADICAL_DOUBLET, ELEM_O));
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, AtomWithBonds_ElementDoesNotDecideTheSide)
{
    EXPECT_EQ(Side::RIGHT, sideOf({180}, NO_RADICAL, ELEM_O));
    EXPECT_EQ(Side::RIGHT, sideOf({90, 270}, NO_RADICAL, ELEM_O));
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, TerminalAtom_HydrogensAreOnTheLeftWhenTheBondLiesInTheRightHalf)
{
    expectSides({
        {"bond up-right at 30 degrees", {30}, NO_RADICAL, Side::LEFT},
        {"bond down-right at 330 degrees", {330}, NO_RADICAL, Side::LEFT},
        {"bond up-left at 150 degrees", {150}, NO_RADICAL, Side::RIGHT},
        {"bond down-left at 210 degrees", {210}, NO_RADICAL, Side::RIGHT},
        {"bond exactly right", {0}, NO_RADICAL, Side::LEFT},
        {"bond exactly left", {180}, NO_RADICAL, Side::RIGHT},
        {"one bond with a radical", {30}, RADICAL_DOUBLET, Side::LEFT},
    });
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, TerminalAtom_StraightUpIsInTheRightHalf_StraightDownInTheLeftHalf)
{
    expectSides({
        {"bond exactly up", {90}, NO_RADICAL, Side::LEFT},
        {"bond 0.01 degrees right of up", {89.99}, NO_RADICAL, Side::LEFT},
        {"bond 0.01 degrees left of up", {90.01}, NO_RADICAL, Side::RIGHT},
        {"bond 0.4 degrees right of up", {89.6}, NO_RADICAL, Side::LEFT},
        {"bond rounded to exactly up", {90.004}, NO_RADICAL, Side::LEFT},
        {"bond exactly down", {270}, NO_RADICAL, Side::RIGHT},
        {"bond 0.01 degrees left of down", {269.99}, NO_RADICAL, Side::RIGHT},
        {"bond 0.01 degrees right of down", {270.01}, NO_RADICAL, Side::LEFT},
        {"bond 0.4 degrees right of down", {270.4}, NO_RADICAL, Side::LEFT},
        {"bond rounded to exactly down", {270.004}, NO_RADICAL, Side::RIGHT},
    });
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, InternalAtom_HydrogensTakeTheFirstFreeQuadrant_RightLeftTopBottom)
{
    expectSides({
        {"right is free", {100, 190}, NO_RADICAL, Side::RIGHT},
        {"left is preferred over top", {0, 270}, NO_RADICAL, Side::LEFT},
        {"left when right and top are taken", {10, 80}, NO_RADICAL, Side::LEFT},
        {"right when left is taken, though the bonds point right", {50, 180, 310}, NO_RADICAL, Side::RIGHT},
        {"top when bottom is taken, though the bonds point up", {40, 140, 270}, NO_RADICAL, Side::TOP},
        {"top when it alone is free", {0, 180, 270}, NO_RADICAL, Side::TOP},
        {"bottom when it alone is free", {0, 90, 180}, NO_RADICAL, Side::BOTTOM},
        {"no free quadrant", {0, 90, 180, 270}, NO_RADICAL, Side::RIGHT},
        {"a radical takes the top", {0, 180}, RADICAL_DOUBLET, Side::BOTTOM},
        {"a radical and bonds leave no free quadrant", {0, 180, 270}, RADICAL_DOUBLET, Side::RIGHT},
    });
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, TopAndBottomBothFree_HydrogensGoAwayFromTheBonds)
{
    expectSides({
        {"peak, both bonds down", {210, 330}, NO_RADICAL, Side::TOP},
        {"valley, both bonds up", {30, 150}, NO_RADICAL, Side::BOTTOM},
        {"level bonds", {0, 180}, NO_RADICAL, Side::TOP},
        {"one bond up, one down, in balance", {330, 150}, NO_RADICAL, Side::TOP},
        {"three bonds pointing up on balance", {10, 170, 190}, NO_RADICAL, Side::BOTTOM},
        // sin(40) - sin(5) - sin(10) is 0.38: a count of the bonds above and below would say they point down
        {"one bond up outweighs two bonds slightly down", {40, 185, 190}, NO_RADICAL, Side::BOTTOM},
        {"a radical at a peak", {210, 330}, RADICAL_DOUBLET, Side::BOTTOM},
        // sin(179) is 0.0175, above the 0.01 tolerance; sin(179.5) is 0.0087, within it
        {"balance above the tolerance", {0, 179}, NO_RADICAL, Side::BOTTOM},
        {"balance within the tolerance, bond above level", {0, 179.5}, NO_RADICAL, Side::TOP},
        {"balance within the tolerance, bond below level", {0, 180.5}, NO_RADICAL, Side::TOP},
    });
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, TopAndBottomBothFree_EachBondCountsOnceWhateverItsLength)
{
    Molecule mol;
    const int atom = mol.addAtom(ELEM_N);
    mol.setAtomXyz(atom, 0.f, 0.f, 0.f);
    // Unit directions at 30 and 210 balance to zero. The sum of the raw vectors would point up.
    addNeighbour(mol, atom, direction(30, 10.));
    addNeighbour(mol, atom, direction(210, 0.1));

    EXPECT_EQ(Side::TOP, ImplicitHydrogenSide(mol, atom).side());
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, RightAndLeftBothFree_HydrogensGoAwayFromTheBonds)
{
    expectSides({
        {"a ring vertex on the left, both bonds to the right", {60, 300}, NO_RADICAL, Side::LEFT},
        {"a ring vertex on the right, both bonds to the left", {120, 240}, NO_RADICAL, Side::RIGHT},
        {"vertical bonds, in balance", {90, 270}, NO_RADICAL, Side::RIGHT},
        // cos(271) is 0.0175, above the 0.01 tolerance; cos(270.5) is 0.0087, within it
        {"balance above the tolerance", {90, 271}, NO_RADICAL, Side::LEFT},
        {"balance within the tolerance, bond right of down", {90, 270.5}, NO_RADICAL, Side::RIGHT},
        {"balance within the tolerance, bond left of down", {90, 269.5}, NO_RADICAL, Side::RIGHT},
    });
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, BondOnDiagonal_BelongsToTheQuadrantClockwiseFromIt)
{
    expectSides({
        // Were 45 top, right would be free and win. It is right, so top and bottom are free and the bond points up.
        {"45 is right, not top", {45, 180}, NO_RADICAL, Side::BOTTOM},
        {"135 is top, not left: left is free", {135, 0}, NO_RADICAL, Side::LEFT},
        // Were 225 bottom, left would be free and win. It is left, so top and bottom are free and the bond points down.
        {"225 is left, not bottom", {225, 0}, NO_RADICAL, Side::TOP},
        {"315 is bottom, not right: right is free", {315, 180}, NO_RADICAL, Side::RIGHT},
        {"just above 45 is top", {45.01, 180}, NO_RADICAL, Side::RIGHT},
        {"rounded to 45 is right", {45.004, 180}, NO_RADICAL, Side::BOTTOM},
        {"a ring standing on its corner, bonds at 225 and 315", {225, 315}, NO_RADICAL, Side::RIGHT},
        {"two bonds on one line at 45 and 225", {45, 225}, NO_RADICAL, Side::TOP},
    });
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, BondDirection_YPointsUp)
{
    // A reading with y pointing down would put the up bond at the bottom and return TOP.
    EXPECT_EQ(Side::BOTTOM, sideOf({0, 90, 180}));
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, BondDirection_IsMeasuredFromTheAtomNotFromTheOrigin)
{
    Molecule mol;
    const int atom = mol.addAtom(ELEM_N);
    mol.setAtomXyz(atom, 10.f, -5.f, 0.f);
    addNeighbour(mol, atom, Vec2f(-1.f, 0.5f));
    addNeighbour(mol, atom, Vec2f(1.f, 0.5f));

    EXPECT_EQ(Side::BOTTOM, ImplicitHydrogenSide(mol, atom).side());
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, Radical_OfAnyKindTakesTheTop)
{
    for (int radical : {RADICAL_SINGLET, RADICAL_DOUBLET, RADICAL_TRIPLET})
        EXPECT_EQ(Side::BOTTOM, sideOf({210, 330}, radical, ELEM_C)) << "radical " << radical;
}
