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

// Where SGroupDataLayout puts the values of data S-groups after their atoms moved. Every move is a quarter turn, a
// shift or a stretch along an axis, so every expected position is exact.

#include <initializer_list>

#include <gtest/gtest.h>

#include <layout/sgroup_data_layout.h>
#include <molecule/elements.h>
#include <molecule/molecule.h>

#include "common.h"

using namespace indigo;

class IndigoCoreSGroupDataLayoutTest : public IndigoCoreTest
{
protected:
    static int addAtom(Molecule& mol, float x, float y)
    {
        const int atom = mol.addAtom(ELEM_C);
        mol.setAtomXyz(atom, x, y, 0.f);
        return atom;
    }

    static int addGroup(Molecule& mol, int type, std::initializer_list<int> atoms)
    {
        const int idx = mol.sgroups.addSGroup(type);
        for (int atom : atoms)
            mol.sgroups.getSGroup(idx).atoms.push(atom);
        return idx;
    }

    static DataSGroup& addValue(Molecule& mol, std::initializer_list<int> atoms, float x, float y)
    {
        DataSGroup& value = static_cast<DataSGroup&>(mol.sgroups.getSGroup(addGroup(mol, SGroup::SG_TYPE_DAT, atoms)));
        value.detached = true;
        value.display_pos.set(Vec2f(x, y));
        return value;
    }

    static void expectPosition(const DataSGroup& value, float x, float y)
    {
        ASSERT_TRUE(value.display_pos.has_value());
        EXPECT_FLOAT_EQ(x, value.display_pos.value().x);
        EXPECT_FLOAT_EQ(y, value.display_pos.value().y);
    }

    // A quarter turn of the bond (0, 0)-(1, 0) about the origin, then a shift by (2, 3)
    static void turnAndShift(Molecule& mol, int first, int second)
    {
        mol.setAtomXyz(first, 2.f, 3.f, 0.f);
        mol.setAtomXyz(second, 2.f, 4.f, 0.f);
    }
};

TEST_F(IndigoCoreSGroupDataLayoutTest, Absolute_TurnedAndShifted_KeepsItsPlaceAgainstTheAtoms)
{
    Molecule mol;
    const int first = addAtom(mol, 0.f, 0.f), second = addAtom(mol, 1.f, 0.f);
    DataSGroup& value = addValue(mol, {first, second}, 0.5f, -1.f);

    SGroupDataLayout layout(mol);
    turnAndShift(mol, first, second);
    layout.update();

    expectPosition(value, 3.f, 3.5f);
}

TEST_F(IndigoCoreSGroupDataLayoutTest, Relative_TurnedAndShifted_OnlyTurns)
{
    Molecule mol;
    const int first = addAtom(mol, 0.f, 0.f), second = addAtom(mol, 1.f, 0.f);
    DataSGroup& value = addValue(mol, {first, second}, 0.5f, -1.f);
    value.relative = true;

    SGroupDataLayout layout(mol);
    turnAndShift(mol, first, second);
    layout.update();

    expectPosition(value, 1.f, 0.5f);
}

TEST_F(IndigoCoreSGroupDataLayoutTest, StretchedAtoms_ShiftTheValueWithoutScalingIt)
{
    Molecule mol;
    const int first = addAtom(mol, 0.f, 0.f), second = addAtom(mol, 1.f, 0.f);
    DataSGroup& value = addValue(mol, {first, second}, 0.5f, -1.f);

    SGroupDataLayout layout(mol);
    mol.setAtomXyz(second, 2.f, 0.f, 0.f);
    layout.update();

    expectPosition(value, 1.f, -1.f);
}

TEST_F(IndigoCoreSGroupDataLayoutTest, NoOwnAtoms_FollowsTheAtomsOfTheParent)
{
    Molecule mol;
    const int chain = addAtom(mol, 0.f, 0.f), water = addAtom(mol, 0.5f, -2.f);
    const int component = addGroup(mol, SGroup::SG_TYPE_COM, {water});
    DataSGroup& value = addValue(mol, {}, 1.5f, -2.f);
    value.parent_idx = component;

    SGroupDataLayout layout(mol);
    mol.setAtomXyz(chain, 10.f, 10.f, 0.f);
    mol.setAtomXyz(water, 3.5f, 0.f, 0.f);
    layout.update();

    expectPosition(value, 4.5f, 0.f);
}

TEST_F(IndigoCoreSGroupDataLayoutTest, NoOwnAtoms_ParentKnownByNumberOnly_IsFound)
{
    Molecule mol;
    const int water = addAtom(mol, 0.5f, -2.f);
    mol.sgroups.getSGroup(addGroup(mol, SGroup::SG_TYPE_COM, {water})).index = 2;
    DataSGroup& value = addValue(mol, {}, 1.5f, -2.f);
    value.parent_group = 2;
    ASSERT_EQ(-1, value.parent_idx.value());

    SGroupDataLayout layout(mol);
    mol.setAtomXyz(water, 3.5f, 0.f, 0.f);
    layout.update();

    expectPosition(value, 4.5f, 0.f);
}

TEST_F(IndigoCoreSGroupDataLayoutTest, NoOwnAtomsAndNoParent_StaysWhereItWas)
{
    Molecule mol;
    const int first = addAtom(mol, 0.f, 0.f), second = addAtom(mol, 1.f, 0.f);
    DataSGroup& value = addValue(mol, {}, 0.5f, -1.f);

    SGroupDataLayout layout(mol);
    turnAndShift(mol, first, second);
    layout.update();

    expectPosition(value, 0.5f, -1.f);
}

TEST_F(IndigoCoreSGroupDataLayoutTest, AtomsThatStayed_LeaveTheValueAsItIs)
{
    Molecule mol;
    const int first = addAtom(mol, 0.f, 0.f), second = addAtom(mol, 1.f, 0.f), other = addAtom(mol, 0.3f, 0.7f);
    DataSGroup& moved = addValue(mol, {first, second}, 0.5f, -1.f);
    DataSGroup& stayed = addValue(mol, {other}, 0.1f, 0.3f);

    SGroupDataLayout layout(mol);
    turnAndShift(mol, first, second);
    layout.update();

    expectPosition(moved, 3.f, 3.5f);
    EXPECT_EQ(0.1f, stayed.display_pos.value().x);
    EXPECT_EQ(0.3f, stayed.display_pos.value().y);
}

TEST_F(IndigoCoreSGroupDataLayoutTest, WithoutPosition_GetsNone)
{
    Molecule mol;
    const int first = addAtom(mol, 0.f, 0.f), second = addAtom(mol, 1.f, 0.f);
    DataSGroup& value = static_cast<DataSGroup&>(mol.sgroups.getSGroup(addGroup(mol, SGroup::SG_TYPE_DAT, {first, second})));

    SGroupDataLayout layout(mol);
    turnAndShift(mol, first, second);
    layout.update();

    EXPECT_FALSE(value.display_pos.has_value());
}
