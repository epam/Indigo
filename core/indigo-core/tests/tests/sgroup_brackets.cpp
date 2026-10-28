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

// The brackets of polymer S-groups: which groups nest, when a bracket counts as a placeholder, how rigid moves
// carry brackets, and where SGroupBracketLayout puts them. At font size 0 an atom's box is its centre, so every
// expected bracket is an exact sum of coordinates and constants.

#include <initializer_list>
#include <vector>

#include <gtest/gtest.h>

#include <base_cpp/exception.h>
#include <layout/sgroup_bracket_layout.h>
#include <layout/sgroup_nesting.h>
#include <molecule/elements.h>
#include <molecule/molecule.h>

#include "common.h"

using namespace indigo;

class IndigoCoreSGroupBracketsTest : public IndigoCoreTest
{
protected:
    static constexpr float NO_FONT = 0.f;

    // Carbons at (0, 0), (1, 0.5), (2, 0), (3, 0.5), ... joined in a chain
    static void zigzag(Molecule& mol, int count)
    {
        for (int i = 0; i < count; i++)
        {
            const int atom = mol.addAtom(ELEM_C);
            mol.setAtomXyz(atom, static_cast<float>(i), i % 2 == 0 ? 0.f : 0.5f, 0.f);
            if (i > 0)
                mol.addBond(atom - 1, atom, BOND_SINGLE);
        }
    }

    static int addGroup(Molecule& mol, int type, std::initializer_list<int> atoms)
    {
        const int idx = mol.sgroups.addSGroup(type);
        for (int atom : atoms)
            mol.sgroups.getSGroup(idx).atoms.push(atom);
        return idx;
    }

    static void setBrackets(SGroup& sgroup, std::initializer_list<std::pair<Vec2f, Vec2f>> brackets)
    {
        sgroup.brackets.clear();
        for (const auto& bracket : brackets)
        {
            Vec2f* points = sgroup.brackets.push();
            points[0] = bracket.first;
            points[1] = bracket.second;
        }
    }

    // The opening bracket runs up on the left, the closing one down on the right (invariant A12)
    static void expectBrackets(const SGroup& sgroup, float left, float bottom, float right, float top)
    {
        ASSERT_EQ(2, sgroup.brackets.size());
        EXPECT_FLOAT_EQ(left, sgroup.brackets[0][0].x);
        EXPECT_FLOAT_EQ(bottom, sgroup.brackets[0][0].y);
        EXPECT_FLOAT_EQ(left, sgroup.brackets[0][1].x);
        EXPECT_FLOAT_EQ(top, sgroup.brackets[0][1].y);
        EXPECT_FLOAT_EQ(right, sgroup.brackets[1][0].x);
        EXPECT_FLOAT_EQ(top, sgroup.brackets[1][0].y);
        EXPECT_FLOAT_EQ(right, sgroup.brackets[1][1].x);
        EXPECT_FLOAT_EQ(bottom, sgroup.brackets[1][1].y);
    }
};

// ---- SGroupNesting -------------------------------------------------------------------------------------------

TEST_F(IndigoCoreSGroupBracketsTest, Nesting_BracketedTypes_OnlyPolymerTypes)
{
    for (int type : {SGroup::SG_TYPE_SRU, SGroup::SG_TYPE_COP, SGroup::SG_TYPE_COM, SGroup::SG_TYPE_MIX, SGroup::SG_TYPE_FOR})
        EXPECT_TRUE(SGroupNesting::isBracketed(type)) << type;
    for (int type : {SGroup::SG_TYPE_GEN, SGroup::SG_TYPE_SUP, SGroup::SG_TYPE_MUL, SGroup::SG_TYPE_DAT})
        EXPECT_FALSE(SGroupNesting::isBracketed(type)) << type;
}

TEST_F(IndigoCoreSGroupBracketsTest, Nesting_NoLinks_SmallestContainingGroupIsParentAndChildrenComeFirst)
{
    Molecule mol;
    zigzag(mol, 5);
    const int mixture = addGroup(mol, SGroup::SG_TYPE_MIX, {0, 1, 2, 3, 4});
    const int unit = addGroup(mol, SGroup::SG_TYPE_SRU, {1, 2});
    const int copolymer = addGroup(mol, SGroup::SG_TYPE_COP, {0, 1, 2, 3});

    const SGroupNesting nesting(mol);

    EXPECT_EQ(std::vector<int>({unit, copolymer, mixture}), nesting.bracketedDeepestFirst());
    EXPECT_EQ(std::vector<int>({copolymer}), nesting.children(mixture));
    EXPECT_EQ(std::vector<int>({unit}), nesting.children(copolymer));
    EXPECT_TRUE(nesting.children(unit).empty());
}

TEST_F(IndigoCoreSGroupBracketsTest, Nesting_LinkToContainingGroup_WinsOverSmallest)
{
    Molecule mol;
    zigzag(mol, 4);
    const int unit = addGroup(mol, SGroup::SG_TYPE_SRU, {1, 2});
    const int inner = addGroup(mol, SGroup::SG_TYPE_COP, {0, 1, 2});
    const int outer = addGroup(mol, SGroup::SG_TYPE_COP, {0, 1, 2, 3});
    mol.sgroups.getSGroup(unit).parent_idx = outer;

    const SGroupNesting nesting(mol);

    EXPECT_EQ(std::vector<int>({unit, inner}), nesting.children(outer));
    EXPECT_TRUE(nesting.children(inner).empty());
}

TEST_F(IndigoCoreSGroupBracketsTest, Nesting_LinkToGroupNotContainingTheAtoms_IsIgnored)
{
    Molecule mol;
    zigzag(mol, 4);
    const int unit = addGroup(mol, SGroup::SG_TYPE_SRU, {0, 1});
    const int other = addGroup(mol, SGroup::SG_TYPE_COP, {2, 3});
    mol.sgroups.getSGroup(unit).parent_idx = other;

    const SGroupNesting nesting(mol);

    EXPECT_TRUE(nesting.children(other).empty());
}

TEST_F(IndigoCoreSGroupBracketsTest, Nesting_EqualAtomsWithoutLink_AreNotNested)
{
    Molecule mol;
    zigzag(mol, 3);
    const int first = addGroup(mol, SGroup::SG_TYPE_SRU, {0, 1, 2});
    const int second = addGroup(mol, SGroup::SG_TYPE_COP, {0, 1, 2});

    const SGroupNesting nesting(mol);

    EXPECT_TRUE(nesting.children(first).empty());
    EXPECT_TRUE(nesting.children(second).empty());
}

TEST_F(IndigoCoreSGroupBracketsTest, Nesting_ParentLinksInALoop_AreCut)
{
    Molecule mol;
    zigzag(mol, 3);
    const int first = addGroup(mol, SGroup::SG_TYPE_SRU, {0, 1, 2});
    const int second = addGroup(mol, SGroup::SG_TYPE_COP, {0, 1, 2});
    mol.sgroups.getSGroup(first).parent_idx = second;
    mol.sgroups.getSGroup(second).parent_idx = first;

    const SGroupNesting nesting(mol);

    EXPECT_EQ(2u, nesting.bracketedDeepestFirst().size());
    EXPECT_EQ(1u, nesting.children(first).size() + nesting.children(second).size());
}

TEST_F(IndigoCoreSGroupBracketsTest, Nesting_MultipleGroup_IsChildButNeverParent)
{
    Molecule mol;
    zigzag(mol, 4);
    const int unit = addGroup(mol, SGroup::SG_TYPE_SRU, {0, 1, 2, 3});
    const int multiple = addGroup(mol, SGroup::SG_TYPE_MUL, {1, 2});
    const int inner = addGroup(mol, SGroup::SG_TYPE_SRU, {1});

    const SGroupNesting nesting(mol);

    EXPECT_EQ(std::vector<int>({multiple, inner}), nesting.children(unit));
    EXPECT_TRUE(nesting.children(multiple).empty());
}

// ---- SGroup::hasBrackets ---------------------------------------------------------------------------------------

TEST_F(IndigoCoreSGroupBracketsTest, HasBrackets_ZeroLengthPlaceholder_MeansNone)
{
    Molecule mol;
    zigzag(mol, 2);
    SGroup& sgroup = mol.sgroups.getSGroup(addGroup(mol, SGroup::SG_TYPE_GEN, {0, 1}));
    EXPECT_FALSE(sgroup.hasBrackets());

    setBrackets(sgroup, {{Vec2f(-0.5f, -1.f), Vec2f(-0.5f, 1.f)}});
    EXPECT_TRUE(sgroup.hasBrackets());
    EXPECT_FALSE(sgroup.isZeroLengthBracket(0));

    setBrackets(sgroup, {{Vec2f(-0.5f, -1.f), Vec2f(-0.5f, 1.f)}, {Vec2f(0.f, 0.f), Vec2f(0.f, 0.f)}});
    EXPECT_FALSE(sgroup.hasBrackets());
    EXPECT_FALSE(sgroup.isZeroLengthBracket(0));
    EXPECT_TRUE(sgroup.isZeroLengthBracket(1));
}

// ---- MoleculeSGroups::transformBrackets ---------------------------------------------------------------------

TEST_F(IndigoCoreSGroupBracketsTest, RigidMoves_CarryBrackets_LeavePlaceholders)
{
    Molecule mol;
    zigzag(mol, 2);
    SGroup& real = mol.sgroups.getSGroup(addGroup(mol, SGroup::SG_TYPE_SRU, {0, 1}));
    setBrackets(real, {{Vec2f(-0.1f, -0.3f), Vec2f(-0.1f, 0.8f)}, {Vec2f(1.1f, 0.8f), Vec2f(1.1f, -0.3f)}});
    // A real bracket next to a placeholder moves; the placeholder stays
    SGroup& mixed = mol.sgroups.getSGroup(addGroup(mol, SGroup::SG_TYPE_GEN, {0, 1}));
    setBrackets(mixed, {{Vec2f(-0.5f, -1.f), Vec2f(-0.5f, 1.f)}, {Vec2f(0.f, 0.f), Vec2f(0.f, 0.f)}});

    mol.offsetCoordinates(Vec3f(1.f, 2.f, 0.f));
    expectBrackets(real, 0.9f, 1.7f, 2.1f, 2.8f);

    mol.scale(Vec2f(0.f, 0.f), 2.f);
    expectBrackets(real, 1.8f, 3.4f, 4.2f, 5.6f);

    EXPECT_FLOAT_EQ(1.f, mixed.brackets[0][0].x);
    EXPECT_FLOAT_EQ(2.f, mixed.brackets[0][0].y);
    EXPECT_FLOAT_EQ(6.f, mixed.brackets[0][1].y);
    EXPECT_FLOAT_EQ(0.f, mixed.brackets[1][0].x);
    EXPECT_FLOAT_EQ(0.f, mixed.brackets[1][1].y);
}

// ---- SGroupBracketLayout -------------------------------------------------------------------------------------

TEST_F(IndigoCoreSGroupBracketsTest, Layout_GroupWithoutChildren_StandsAtTheMargins)
{
    Molecule mol;
    zigzag(mol, 4);
    const int unit = addGroup(mol, SGroup::SG_TYPE_SRU, {1, 2});

    SGroupBracketLayout(mol, NO_FONT).update();

    expectBrackets(mol.sgroups.getSGroup(unit), 1.f - 0.1f, 0.f - 0.3f, 2.f + 0.1f, 0.5f + 0.3f);
}

TEST_F(IndigoCoreSGroupBracketsTest, Layout_Parent_StandsAtWhicheverReachesFurther)
{
    Molecule mol;
    zigzag(mol, 4);
    const int unit = addGroup(mol, SGroup::SG_TYPE_SRU, {1, 2});
    const int copolymer = addGroup(mol, SGroup::SG_TYPE_COP, {0, 1, 2, 3});

    SGroupBracketLayout(mol, NO_FONT).update();

    // Left and right: the margin past its own atoms 0 and 3. Top and bottom: the space past the child's brackets
    expectBrackets(mol.sgroups.getSGroup(copolymer), 0.f - 0.1f, -0.3f - 0.3f, 3.f + 0.1f, 0.8f + 0.3f);
    expectBrackets(mol.sgroups.getSGroup(unit), 0.9f, -0.3f, 2.1f, 0.8f);
}

TEST_F(IndigoCoreSGroupBracketsTest, Layout_ParentOfTheSameAtoms_StandsAtTheSpacePastTheChild)
{
    Molecule mol;
    zigzag(mol, 4);
    const int unit = addGroup(mol, SGroup::SG_TYPE_SRU, {1, 2});
    const int copolymer = addGroup(mol, SGroup::SG_TYPE_COP, {1, 2});
    mol.sgroups.getSGroup(unit).parent_idx = copolymer;

    SGroupBracketLayout(mol, NO_FONT).update();

    expectBrackets(mol.sgroups.getSGroup(copolymer), 0.9f - 0.3f, -0.3f - 0.3f, 2.1f + 0.3f, 0.8f + 0.3f);
}

TEST_F(IndigoCoreSGroupBracketsTest, Layout_LoadedBrackets_KeptWhileTheAtomsStay)
{
    Molecule mol;
    zigzag(mol, 4);
    SGroup& unit = mol.sgroups.getSGroup(addGroup(mol, SGroup::SG_TYPE_SRU, {1, 2}));
    setBrackets(unit, {{Vec2f(0.5f, -2.f), Vec2f(0.5f, 2.f)}, {Vec2f(2.5f, 2.f), Vec2f(2.5f, -2.f)}});

    SGroupBracketLayout untouched(mol, NO_FONT);
    untouched.update();
    expectBrackets(unit, 0.5f, -2.f, 2.5f, 2.f);

    SGroupBracketLayout moved(mol, NO_FONT);
    mol.setAtomXyz(2, 3.f, 0.f, 0.f);
    moved.update();
    expectBrackets(unit, 0.9f, -0.3f, 3.1f, 0.8f);
}

TEST_F(IndigoCoreSGroupBracketsTest, Layout_AtomsAtOnePoint_LeavePlaceholders)
{
    Molecule mol;
    zigzag(mol, 3);
    for (int i = 0; i < 3; i++)
        mol.setAtomXyz(i, 0.f, 0.f, 0.f);
    SGroup& unit = mol.sgroups.getSGroup(addGroup(mol, SGroup::SG_TYPE_SRU, {0, 1, 2}));
    setBrackets(unit, {{Vec2f(0.f, 0.f), Vec2f(0.f, 0.f)}});

    SGroupBracketLayout(mol, NO_FONT).update();

    ASSERT_EQ(1, unit.brackets.size());
    EXPECT_TRUE(unit.isZeroLengthBracket(0));
}

TEST_F(IndigoCoreSGroupBracketsTest, Layout_NoBracketedGroups_ChangesNothing)
{
    Molecule mol;
    zigzag(mol, 2);
    SGroup& generic = mol.sgroups.getSGroup(addGroup(mol, SGroup::SG_TYPE_GEN, {0, 1}));

    SGroupBracketLayout(mol, NO_FONT).update();

    EXPECT_EQ(0, generic.brackets.size());
}

TEST_F(IndigoCoreSGroupBracketsTest, Layout_AtomOutsideTheStructure_ThrowsWithMessage)
{
    Molecule mol;
    zigzag(mol, 2);
    addGroup(mol, SGroup::SG_TYPE_SRU, {0, 99});
    try
    {
        SGroupBracketLayout(mol, NO_FONT).update();
        FAIL() << "expected an exception";
    }
    catch (Exception& e)
    {
        EXPECT_STREQ("array: invalid index 99 (size=2)", e.message());
    }
}
