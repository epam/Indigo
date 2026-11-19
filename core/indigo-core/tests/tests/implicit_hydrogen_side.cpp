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

// The side of an atom symbol on which its implicit hydrogens are drawn. Ketcher draws the same structures on its
// canvas, so the tables repeat the cases of its implicitHydrogenSide.test.ts: the same bonds must give the same side.

#include <cmath>
#include <initializer_list>
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

    struct Case
    {
        const char* name;
        std::vector<double> bond_angles;
        bool has_radical;
        Side expected;
    };

    // 0 degrees points right, 90 degrees up
    static Vec2f direction(double degrees, double length = 1.)
    {
        const double radians = degrees * M_PI / 180.;
        return Vec2f(static_cast<float>(length * std::cos(radians)), static_cast<float>(length * std::sin(radians)));
    }

    static Side sideOf(const std::vector<double>& bond_angles, bool has_radical, double bond_length = 1.)
    {
        std::vector<Vec2f> directions;
        for (double angle : bond_angles)
            directions.push_back(direction(angle, bond_length));
        return ImplicitHydrogenSide(directions, has_radical).side();
    }

    static void expectSides(const std::vector<Case>& cases, double bond_length = 1.)
    {
        for (const Case& tc : cases)
            EXPECT_EQ(tc.expected, sideOf(tc.bond_angles, tc.has_radical, bond_length)) << tc.name;
    }

    static int addAtomWithNeighbours(Molecule& mol, int element, std::initializer_list<double> bond_angles)
    {
        const int atom = mol.addAtom(element);
        mol.setAtomXyz(atom, 0.f, 0.f, 0.f);
        for (double angle : bond_angles)
        {
            const int neighbour = mol.addAtom(ELEM_C);
            const Vec2f position = direction(angle);
            mol.setAtomXyz(neighbour, position.x, position.y, 0.f);
            mol.addBond(atom, neighbour, BOND_SINGLE);
        }
        return atom;
    }

    static std::vector<Case> casesOfTopAndBottomBothFree()
    {
        return {
            {"peak, both bonds down", {210, 330}, false, Side::TOP},
            {"valley, both bonds up", {30, 150}, false, Side::BOTTOM},
            {"bonds at 20 and 160", {20, 160}, false, Side::BOTTOM},
            {"level bonds", {0, 180}, false, Side::TOP},
            {"one bond up, one down, in balance", {330, 150}, false, Side::TOP},
            {"bonds pointing down on balance", {350, 190}, false, Side::TOP},
            {"bonds pointing up on balance", {20, 190}, false, Side::BOTTOM},
            {"three bonds pointing up on balance", {10, 170, 190}, false, Side::BOTTOM},
            {"three bonds pointing down on balance", {350, 170, 190}, false, Side::TOP},
            {"a radical with bonds 30 and 150", {30, 150}, true, Side::BOTTOM},
            {"a radical with bonds 210 and 330", {210, 330}, true, Side::BOTTOM},
            // sin(179) is 0.0175, above the 0.01 tolerance; sin(179.5) is 0.0087, within it
            {"balance above the tolerance", {0, 179}, false, Side::BOTTOM},
            {"balance within the tolerance, bond above level", {0, 179.5}, false, Side::TOP},
            {"balance within the tolerance, bond below level", {0, 180.5}, false, Side::TOP},
        };
    }
};

TEST_F(IndigoCoreImplicitHydrogenSideTest, TerminalAtom_HydrogensAreOppositeTheBond_AndRightForVerticalOrNoBond)
{
    expectSides({
        {"no bond", {}, false, Side::RIGHT},
        {"bond up-right at 30 degrees", {30}, false, Side::LEFT},
        {"bond down-left at 210 degrees", {210}, false, Side::RIGHT},
        {"bond exactly right", {0}, false, Side::LEFT},
        {"bond exactly left", {180}, false, Side::RIGHT},
        {"bond exactly up", {90}, false, Side::RIGHT},
        {"bond exactly down", {270}, false, Side::RIGHT},
        {"bond 0.4 degrees right of up", {89.6}, false, Side::RIGHT},
        {"bond 0.4 degrees left of up", {90.4}, false, Side::RIGHT},
        {"bond 0.5 degrees right of up", {89.5}, false, Side::RIGHT},
        {"bond 0.4 degrees left of down", {269.6}, false, Side::RIGHT},
        {"bond 0.4 degrees right of down", {270.4}, false, Side::RIGHT},
        {"bond 1 degree right of up", {89}, false, Side::LEFT},
        {"bond 1 degree right of down", {271}, false, Side::LEFT},
        {"bond 1 degree left of up", {91}, false, Side::RIGHT},
        {"bond given as negative angle to the right", {-30}, false, Side::LEFT},
        {"bond given as angle above 360 to the right", {390}, false, Side::LEFT},
        {"one bond with a radical", {30}, true, Side::LEFT},
        {"no bond with a radical", {}, true, Side::RIGHT},
    });
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, InternalAtom_HydrogensTakeTheFirstFreeQuadrant_RightLeftTopBottom)
{
    expectSides({
        {"two bonds pointing down", {210, 330}, false, Side::TOP},
        {"bonds in top and bottom quadrants", {90, 270}, false, Side::RIGHT},
        {"right and left bonds with a radical", {0, 180}, true, Side::BOTTOM},
        {"all four quadrants occupied", {0, 90, 180, 270}, false, Side::RIGHT},
        {"three bonds leave top free", {30, 150, 270}, false, Side::TOP},
        {"six bonds leave no free quadrant", {0, 60, 120, 180, 240, 300}, false, Side::RIGHT},
        {"left is preferred over top when right is taken", {10, 80}, false, Side::LEFT},
        {"top is preferred over bottom", {0, 180, 270}, false, Side::TOP},
        {"radical with right, left and top bonds", {0, 180, 90}, true, Side::BOTTOM},
        {"radical with right, left and bottom bonds", {0, 180, 270}, true, Side::RIGHT},
    });
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, TopAndBottomBothFree_HydrogensGoAwayFromTheBonds)
{
    expectSides(casesOfTopAndBottomBothFree());
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, TopAndBottomBothFree_BondLengthDoesNotChangeTheSide)
{
    expectSides(casesOfTopAndBottomBothFree(), 5.);
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, TopAndBottomBothFree_EachBondCountsOnceWhateverItsLength)
{
    // Unit directions at 30 and 210 balance to zero. The sum of the raw vectors would point up.
    const std::vector<Vec2f> long_up_short_down = {direction(30, 10.), direction(210, 0.1)};

    EXPECT_EQ(Side::TOP, ImplicitHydrogenSide(long_up_short_down, false).side());
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, QuadrantBoundary_BelongsToTheQuadrantThatStartsThere)
{
    expectSides({
        {"45 is top, not right: right is free", {45, 180}, false, Side::RIGHT},
        // Were 135 top, left would be free and win. It is left, so top and bottom are free and the bond points up.
        {"135 is left, not top", {135, 0}, false, Side::BOTTOM},
        {"225 is bottom, not left: left is free", {225, 0}, false, Side::LEFT},
        {"315 is right, not bottom: top is free", {315, 180}, false, Side::TOP},
        {"just below 315 is bottom", {314.99, 180}, false, Side::RIGHT},
        {"rounded to 315 is right", {314.996, 180}, false, Side::TOP},
        {"-45 normalises to 315", {-45, 180}, false, Side::TOP},
        {"405 normalises to 45", {405, 180}, false, Side::RIGHT},
        {"360 normalises to 0", {360, 180}, false, Side::TOP},
    });
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, BondDirection_YPointsUp)
{
    const Vec2f right(1.f, 0.f), up(0.f, 1.f), left(-1.f, 0.f);

    // A reading with y pointing down would put the up bond at the bottom and return TOP.
    EXPECT_EQ(Side::BOTTOM, ImplicitHydrogenSide({right, up, left}, false).side());
    EXPECT_EQ(Side::LEFT, ImplicitHydrogenSide({Vec2f(1.f, 1.f)}, false).side());
    EXPECT_EQ(Side::RIGHT, ImplicitHydrogenSide({Vec2f(-1.f, -1.f)}, false).side());
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, BondDirection_LengthDoesNotChangeTheSide)
{
    EXPECT_EQ(Side::RIGHT, ImplicitHydrogenSide({Vec2f(-30.f, 0.f)}, false).side());
    EXPECT_EQ(Side::LEFT, ImplicitHydrogenSide({Vec2f(0.001f, 0.f)}, false).side());
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, IsStacked_OnlyForTopAndBottom)
{
    EXPECT_TRUE(ImplicitHydrogenSide::isStacked(Side::TOP));
    EXPECT_TRUE(ImplicitHydrogenSide::isStacked(Side::BOTTOM));
    EXPECT_FALSE(ImplicitHydrogenSide::isStacked(Side::LEFT));
    EXPECT_FALSE(ImplicitHydrogenSide::isStacked(Side::RIGHT));
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, AtomOfMolecule_BondsAreReadFromTheCoordinatesOfItsNeighbours)
{
    Molecule peak, valley, terminal;
    const int peak_atom = addAtomWithNeighbours(peak, ELEM_N, {210, 330});
    const int valley_atom = addAtomWithNeighbours(valley, ELEM_N, {30, 150});
    const int terminal_atom = addAtomWithNeighbours(terminal, ELEM_O, {0});

    EXPECT_EQ(Side::TOP, ImplicitHydrogenSide(peak, peak_atom).side());
    EXPECT_EQ(Side::BOTTOM, ImplicitHydrogenSide(valley, valley_atom).side());
    EXPECT_EQ(Side::LEFT, ImplicitHydrogenSide(terminal, terminal_atom).side());
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, AtomOfMolecule_BondsAreMeasuredFromTheAtomNotFromTheOrigin)
{
    Molecule mol;
    const int atom = mol.addAtom(ELEM_N);
    const int upper_left = mol.addAtom(ELEM_C);
    const int upper_right = mol.addAtom(ELEM_C);
    mol.setAtomXyz(atom, 10.f, -5.f, 0.f);
    mol.setAtomXyz(upper_left, 9.f, -4.5f, 0.f);
    mol.setAtomXyz(upper_right, 11.f, -4.5f, 0.f);
    mol.addBond(atom, upper_left, BOND_SINGLE);
    mol.addBond(atom, upper_right, BOND_SINGLE);

    EXPECT_EQ(Side::BOTTOM, ImplicitHydrogenSide(mol, atom).side());
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, AtomOfMolecule_RadicalOccupiesTheTop)
{
    Molecule mol;
    const int atom = addAtomWithNeighbours(mol, ELEM_C, {210, 330});
    mol.setAtomRadical(atom, RADICAL_DOUBLET);

    EXPECT_EQ(Side::BOTTOM, ImplicitHydrogenSide(mol, atom).side());
}

TEST_F(IndigoCoreImplicitHydrogenSideTest, AtomOfMolecule_WithoutBonds_IsRightForEveryElement)
{
    for (int element : {ELEM_O, ELEM_Cl, ELEM_S, ELEM_N, ELEM_C})
    {
        Molecule mol;
        const int atom = addAtomWithNeighbours(mol, element, {});

        EXPECT_EQ(Side::RIGHT, ImplicitHydrogenSide(mol, atom).side()) << Element::toString(element);
    }
}
