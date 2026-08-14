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

// Tests for the haptic bond container (#3233, model #3837): stable indices, slot
// reuse, the all-or-nothing rule on both mappings, and the vertex view a consumer
// gets instead of a graph edge.

#include <gtest/gtest.h>

#include <molecule/base_molecule.h>
#include <molecule/molecule_attachment_groups.h>
#include <molecule/molecule_haptic_bonds.h>

#include "common.h"

using namespace indigo;

class IndigoCoreHapticBondsTest : public IndigoCoreTest
{
protected:
    using Endpoint = HapticBond::Endpoint;

    // A group of the five ring atoms, the shape of the ferrocene half.
    static int addRingGroup(MoleculeAttachmentGroups& groups)
    {
        const int idx = groups.addGroup();
        groups.group(idx).setAtoms({0, 1, 2, 3, 4});
        return idx;
    }

    static Array<int> identity(int size)
    {
        Array<int> mapping;
        mapping.clear_resize(size);
        for (int i = 0; i < size; i++)
            mapping[i] = i;
        return mapping;
    }
};

TEST_F(IndigoCoreHapticBondsTest, AddAssignsIndicesAndCounts)
{
    MoleculeHapticBonds bonds;
    EXPECT_EQ(0, bonds.count());
    EXPECT_TRUE(bonds.isEmpty());

    const int a = bonds.add(Endpoint::group(0), Endpoint::atom(5), _BOND_HAPTIC);
    const int b = bonds.add(Endpoint::atom(1), Endpoint::atom(2), _BOND_VARIABLE_ATTACHMENT);
    EXPECT_EQ(0, a);
    EXPECT_EQ(1, b);
    EXPECT_EQ(2, bonds.count());
    EXPECT_FALSE(bonds.isEmpty());

    EXPECT_TRUE(bonds.at(a).begin().isGroup());
    EXPECT_EQ(0, bonds.at(a).begin().index());
    EXPECT_FALSE(bonds.at(a).end().isGroup());
    EXPECT_EQ(5, bonds.at(a).end().index());
    EXPECT_EQ(_BOND_HAPTIC, bonds.at(a).type());
    EXPECT_EQ(_BOND_VARIABLE_ATTACHMENT, bonds.at(b).type());
}

TEST_F(IndigoCoreHapticBondsTest, RemoveFreesSlotAndReusesItClean)
{
    MoleculeHapticBonds bonds;
    bonds.add(Endpoint::atom(0), Endpoint::atom(1), _BOND_HAPTIC);
    const int b = bonds.add(Endpoint::group(3), Endpoint::atom(7), _BOND_HAPTIC);
    bonds.add(Endpoint::atom(2), Endpoint::atom(3), _BOND_HAPTIC);

    bonds.remove(b);
    EXPECT_EQ(2, bonds.count());
    EXPECT_FALSE(bonds.has(b));

    const int reused = bonds.add(Endpoint::atom(8), Endpoint::atom(9), _BOND_HAPTIC);
    EXPECT_EQ(b, reused); // stable indices: the freed slot comes back
    EXPECT_FALSE(bonds.at(reused).begin().isGroup());
    EXPECT_EQ(8, bonds.at(reused).begin().index());
}

TEST_F(IndigoCoreHapticBondsTest, IterationSkipsRemovedBonds)
{
    MoleculeHapticBonds bonds;
    const int a = bonds.add(Endpoint::atom(0), Endpoint::atom(1), _BOND_HAPTIC);
    const int b = bonds.add(Endpoint::atom(2), Endpoint::atom(3), _BOND_HAPTIC);
    const int c = bonds.add(Endpoint::atom(4), Endpoint::atom(5), _BOND_HAPTIC);
    bonds.remove(b);

    std::vector<int> seen;
    for (int i = bonds.begin(); i != bonds.end(); i = bonds.next(i))
        seen.push_back(i);
    EXPECT_EQ(std::vector<int>({a, c}), seen);
}

TEST_F(IndigoCoreHapticBondsTest, AccessToMissingBondThrows)
{
    MoleculeHapticBonds bonds;
    EXPECT_THROW(bonds.at(0), Exception);
    EXPECT_THROW(bonds.remove(0), Exception);
}

// One group may take part in several haptic bonds and one atom in several
// (ketcher#8390 req. 4.6/4.7), so neither side may be capped at one.
TEST_F(IndigoCoreHapticBondsTest, MultiplicityIsUnbounded)
{
    MoleculeHapticBonds bonds;
    bonds.add(Endpoint::group(0), Endpoint::atom(10), _BOND_HAPTIC);
    bonds.add(Endpoint::group(0), Endpoint::atom(11), _BOND_HAPTIC); // same group, another metal
    bonds.add(Endpoint::group(1), Endpoint::atom(10), _BOND_HAPTIC); // same metal, another group
    EXPECT_EQ(3, bonds.count());
}

TEST_F(IndigoCoreHapticBondsTest, VerticesRunFromTheBeginEndpointToTheEndOne)
{
    MoleculeAttachmentGroups groups;
    const int group = addRingGroup(groups);

    MoleculeHapticBonds bonds;
    const int bond = bonds.add(Endpoint::group(group), Endpoint::atom(5), _BOND_HAPTIC);

    std::vector<int> vertices;
    bonds.collectVertices(bond, groups, vertices);
    EXPECT_EQ(std::vector<int>({0, 1, 2, 3, 4, 5}), vertices);
    EXPECT_EQ(0, bonds.firstAtom(bond, groups));
    EXPECT_EQ(5, bonds.lastAtom(bond, groups));
}

TEST_F(IndigoCoreHapticBondsTest, VerticesOfAnAtomToAtomBondAreTheTwoAtoms)
{
    MoleculeAttachmentGroups groups;
    MoleculeHapticBonds bonds;
    const int bond = bonds.add(Endpoint::atom(3), Endpoint::atom(8), _BOND_HAPTIC);

    std::vector<int> vertices;
    bonds.collectVertices(bond, groups, vertices);
    EXPECT_EQ(std::vector<int>({3, 8}), vertices);
    EXPECT_EQ(3, bonds.firstAtom(bond, groups));
    EXPECT_EQ(8, bonds.lastAtom(bond, groups));
}

TEST_F(IndigoCoreHapticBondsTest, ConnectivitySetsJoinBothEndpoints)
{
    MoleculeAttachmentGroups groups;
    const int group = addRingGroup(groups);

    MoleculeHapticBonds bonds;
    bonds.add(Endpoint::group(group), Endpoint::atom(5), _BOND_HAPTIC);
    bonds.add(Endpoint::atom(6), Endpoint::atom(7), _BOND_HAPTIC);

    std::list<std::unordered_set<int>> neighbors;
    bonds.collectConnectivitySets(groups, neighbors);
    ASSERT_EQ(2u, neighbors.size());
    EXPECT_EQ(std::unordered_set<int>({0, 1, 2, 3, 4, 5}), neighbors.front());
    EXPECT_EQ(std::unordered_set<int>({6, 7}), neighbors.back());
}

TEST_F(IndigoCoreHapticBondsTest, RemovedAtomDropsTheWholeBond)
{
    MoleculeHapticBonds bonds;
    const int kept = bonds.add(Endpoint::atom(0), Endpoint::atom(1), _BOND_HAPTIC);
    bonds.add(Endpoint::atom(0), Endpoint::atom(2), _BOND_HAPTIC);

    Array<int> mapping = identity(3);
    mapping[2] = -1; // atom 2 is gone
    bonds.onAtomsRemoved(mapping);

    EXPECT_EQ(1, bonds.count());
    ASSERT_TRUE(bonds.has(kept));
    EXPECT_EQ(1, bonds.at(kept).end().index());
}

TEST_F(IndigoCoreHapticBondsTest, RemappingFollowsTheAtomMapping)
{
    MoleculeHapticBonds bonds;
    const int bond = bonds.add(Endpoint::group(2), Endpoint::atom(1), _BOND_HAPTIC);

    Array<int> mapping = identity(3);
    mapping[1] = 7;
    bonds.onAtomsRemoved(mapping);

    EXPECT_EQ(7, bonds.at(bond).end().index());
    EXPECT_TRUE(bonds.at(bond).begin().isGroup());
    EXPECT_EQ(2, bonds.at(bond).begin().index()); // a group endpoint is not an atom index
}

// The group pool recycles freed indices, so a bond left behind would silently
// attach itself to the next group to take the index.
TEST_F(IndigoCoreHapticBondsTest, BondDoesNotOutliveItsGroup)
{
    MoleculeHapticBonds bonds;
    bonds.add(Endpoint::group(1), Endpoint::atom(5), _BOND_HAPTIC);
    const int other = bonds.add(Endpoint::group(2), Endpoint::atom(5), _BOND_HAPTIC);

    bonds.onGroupRemoved(1);
    EXPECT_EQ(1, bonds.count());
    ASSERT_TRUE(bonds.has(other));
    EXPECT_EQ(2, bonds.at(other).begin().index());
}

TEST_F(IndigoCoreHapticBondsTest, MergeCopiesBondsThroughBothMappings)
{
    MoleculeHapticBonds source;
    source.add(Endpoint::group(0), Endpoint::atom(5), _BOND_HAPTIC);
    source.add(Endpoint::atom(1), Endpoint::atom(2), _BOND_VARIABLE_ATTACHMENT);

    Array<int> atom_mapping = identity(6);
    atom_mapping[5] = 15;
    atom_mapping[1] = 11;
    atom_mapping[2] = 12;
    Array<int> group_mapping = identity(1);
    group_mapping[0] = 3;

    MoleculeHapticBonds target;
    target.mergeWithSubmolecule(source, atom_mapping, group_mapping);

    ASSERT_EQ(2, target.count());
    const HapticBond& first = target.at(target.begin());
    EXPECT_EQ(3, first.begin().index());
    EXPECT_EQ(15, first.end().index());
    const HapticBond& second = target.at(target.next(target.begin()));
    EXPECT_EQ(11, second.begin().index());
    EXPECT_EQ(12, second.end().index());
    EXPECT_EQ(_BOND_VARIABLE_ATTACHMENT, second.type());
}

TEST_F(IndigoCoreHapticBondsTest, MergeDropsBondsWhoseEndpointDidNotSurvive)
{
    MoleculeHapticBonds source;
    source.add(Endpoint::group(0), Endpoint::atom(5), _BOND_HAPTIC); // group dropped
    source.add(Endpoint::group(1), Endpoint::atom(6), _BOND_HAPTIC); // partner dropped
    source.add(Endpoint::group(1), Endpoint::atom(5), _BOND_HAPTIC); // survives

    Array<int> atom_mapping = identity(7);
    atom_mapping[6] = -1;
    Array<int> group_mapping = identity(2);
    group_mapping[0] = -1;

    MoleculeHapticBonds target;
    target.mergeWithSubmolecule(source, atom_mapping, group_mapping);

    ASSERT_EQ(1, target.count());
    const HapticBond& survivor = target.at(target.begin());
    EXPECT_EQ(1, survivor.begin().index());
    EXPECT_EQ(5, survivor.end().index());
}

TEST_F(IndigoCoreHapticBondsTest, ClearRemovesEverything)
{
    MoleculeHapticBonds bonds;
    bonds.add(Endpoint::group(0), Endpoint::atom(5), _BOND_HAPTIC);
    bonds.clear();
    EXPECT_TRUE(bonds.isEmpty());
    EXPECT_EQ(0, bonds.count());
}
