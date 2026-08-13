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

// Tests for the attachment-group model that backs haptic bonds (#3233, #3837).
//
// Two contracts are pinned: the container itself (stable indices, slot reuse,
// bonds owned by their group, unbounded multiplicity) and its contract with the
// molecule — a group must not perturb any chemistry, since BIOVIA classes a
// haptic bond with the "zero-order bonds, which are bonds that do not affect
// valence". That last one guards the isolation from the valence model of #3617.

#include <gtest/gtest.h>

#include <molecule/molecule.h>
#include <molecule/molecule_attachment_groups.h>

#include "common.h"

using namespace indigo;

class IndigoCoreAttachmentGroupsTest : public IndigoCoreTest
{
protected:
    // Cyclopentadienyl ring (atoms 0..4) plus an iron atom (5), the shape of
    // the ferrocene half that the ticket's example is made of.
    static void makeRingAndMetal(Molecule& mol)
    {
        for (int i = 0; i < 5; i++)
            mol.addAtom(ELEM_C);
        mol.addAtom(ELEM_Fe);
        for (int i = 0; i < 5; i++)
            mol.addBond(i, (i + 1) % 5, BOND_SINGLE);
    }

    static int addRingGroup(Molecule& mol)
    {
        const int idx = mol.attachment_groups.addGroup();
        AttachmentGroup& ag = mol.attachment_groups.group(idx);
        for (int i = 0; i < 5; i++)
            ag.addAtom(i);
        return idx;
    }
};

// ---- container contract ---------------------------------------------------

TEST_F(IndigoCoreAttachmentGroupsTest, AddAssignsIndicesAndCounts)
{
    MoleculeAttachmentGroups groups;
    EXPECT_EQ(0, groups.groupCount());
    EXPECT_TRUE(groups.isEmpty());

    const int a = groups.addGroup();
    const int b = groups.addGroup(AttachmentMode::Any);
    EXPECT_EQ(0, a);
    EXPECT_EQ(1, b);
    EXPECT_EQ(2, groups.groupCount());
    EXPECT_FALSE(groups.isEmpty());
    EXPECT_EQ(AttachmentMode::All, groups.group(a).mode());
    EXPECT_EQ(AttachmentMode::Any, groups.group(b).mode());
}

TEST_F(IndigoCoreAttachmentGroupsTest, RemoveFreesSlotAndReusesItClean)
{
    MoleculeAttachmentGroups groups;
    groups.addGroup();
    const int b = groups.addGroup();
    groups.addGroup();
    groups.group(b).addAtom(42);
    groups.group(b).addBond(7, _BOND_COORDINATION);

    groups.removeGroup(b);
    EXPECT_EQ(2, groups.groupCount());
    EXPECT_FALSE(groups.hasGroup(b));

    const int reused = groups.addGroup();
    EXPECT_EQ(b, reused); // stable indices: the freed slot comes back
    EXPECT_TRUE(groups.group(reused).atoms().empty());
    EXPECT_TRUE(groups.group(reused).bonds().empty()); // no stale bonds
}

TEST_F(IndigoCoreAttachmentGroupsTest, IterationSkipsRemovedGroups)
{
    MoleculeAttachmentGroups groups;
    const int a = groups.addGroup();
    const int b = groups.addGroup();
    const int c = groups.addGroup();
    groups.removeGroup(b);

    std::vector<int> seen;
    for (int i = groups.begin(); i != groups.end(); i = groups.next(i))
        seen.push_back(i);
    EXPECT_EQ(std::vector<int>({a, c}), seen);
}

TEST_F(IndigoCoreAttachmentGroupsTest, AccessToMissingGroupThrows)
{
    MoleculeAttachmentGroups groups;
    EXPECT_THROW(groups.group(0), Exception);
    EXPECT_THROW(groups.removeGroup(0), Exception);
}

TEST_F(IndigoCoreAttachmentGroupsTest, MembershipIsASet)
{
    MoleculeAttachmentGroups groups;
    AttachmentGroup& ag = groups.group(groups.addGroup());
    ag.addAtom(3);
    ag.addAtom(3);
    ag.setAtoms({1, 2, 2, 1});
    EXPECT_EQ(std::vector<int>({1, 2}), ag.atoms());
    EXPECT_TRUE(ag.hasAtom(1));
    EXPECT_FALSE(ag.hasAtom(3)); // setAtoms replaces, it does not append
}

// The ticket lets one group take part in several haptic bonds and one atom
// take part in several (ketcher#8390 req. 4.6/4.7), so neither side may be
// capped at one.
TEST_F(IndigoCoreAttachmentGroupsTest, MultiplicityIsUnbounded)
{
    MoleculeAttachmentGroups groups;
    AttachmentGroup& shared = groups.group(groups.addGroup());
    shared.addBond(10, _BOND_COORDINATION);
    shared.addBond(11, _BOND_COORDINATION);
    EXPECT_EQ(2u, shared.bonds().size());

    AttachmentGroup& second = groups.group(groups.addGroup());
    second.addBond(10, _BOND_COORDINATION); // same partner atom as the first group
    EXPECT_EQ(1u, second.bonds().size());
    EXPECT_EQ(10, second.bonds()[0].atom);
}

TEST_F(IndigoCoreAttachmentGroupsTest, RemovingGroupTakesItsBondsWithIt)
{
    MoleculeAttachmentGroups groups;
    const int a = groups.addGroup();
    const int b = groups.addGroup();
    groups.group(a).addBond(10, _BOND_COORDINATION);
    groups.group(b).addBond(11, _BOND_COORDINATION);

    groups.removeGroup(a);
    ASSERT_TRUE(groups.hasGroup(b));
    EXPECT_EQ(1u, groups.group(b).bonds().size()); // untouched
    EXPECT_EQ(11, groups.group(b).bonds()[0].atom);
}

TEST_F(IndigoCoreAttachmentGroupsTest, HapticMarksOnBonds)
{
    MoleculeAttachmentGroups groups;
    EXPECT_FALSE(groups.isBondHaptic(3));
    groups.setBondHaptic(3);
    EXPECT_TRUE(groups.isBondHaptic(3));
    EXPECT_EQ(1, groups.hapticBondCount());
    EXPECT_FALSE(groups.isEmpty()); // a lone mark still means the feature is in use

    groups.setBondHaptic(3, false);
    EXPECT_FALSE(groups.isBondHaptic(3));
    EXPECT_TRUE(groups.isEmpty());
}

TEST_F(IndigoCoreAttachmentGroupsTest, ConnectivitySetsJoinGroupAndPartners)
{
    MoleculeAttachmentGroups groups;
    AttachmentGroup& ag = groups.group(groups.addGroup());
    ag.setAtoms({0, 1, 2});
    ag.addBond(9, _BOND_COORDINATION);

    std::list<std::unordered_set<int>> neighbors;
    groups.collectConnectivitySets(neighbors);
    ASSERT_EQ(1u, neighbors.size());
    EXPECT_EQ(std::unordered_set<int>({0, 1, 2, 9}), neighbors.front());
}

// ---- contract with the molecule -------------------------------------------

// The isolation invariant: adding a group and a haptic bond must leave every
// atom's valence and implicit hydrogen count exactly as it was.
TEST_F(IndigoCoreAttachmentGroupsTest, ZeroOrderInvariantValenceUntouched)
{
    Molecule plain;
    makeRingAndMetal(plain);

    Molecule with_group;
    makeRingAndMetal(with_group);
    AttachmentGroup& ag = with_group.attachment_groups.group(addRingGroup(with_group));
    ag.addBond(5, _BOND_COORDINATION); // haptic bond from the ring to the metal
    ag.setCharge(-1);

    for (int i = plain.vertexBegin(); i != plain.vertexEnd(); i = plain.vertexNext(i))
    {
        EXPECT_EQ(plain.getAtomValence(i), with_group.getAtomValence(i)) << "valence changed for atom " << i;
        EXPECT_EQ(plain.getImplicitH(i), with_group.getImplicitH(i)) << "implicit H changed for atom " << i;
    }
}

TEST_F(IndigoCoreAttachmentGroupsTest, HapticMarkOnBondDoesNotChangeValence)
{
    Molecule mol;
    mol.addAtom(ELEM_C);
    mol.addAtom(ELEM_Fe);
    const int bond = mol.addBond(0, 1, _BOND_COORDINATION);

    const int valence_before = mol.getAtomValence(0);
    const int hydrogens_before = mol.getImplicitH(0);
    mol.attachment_groups.setBondHaptic(bond);
    EXPECT_EQ(valence_before, mol.getAtomValence(0));
    EXPECT_EQ(hydrogens_before, mol.getImplicitH(0));
}

// The graph pool hands a freed bond index straight back to the next addBond, so a
// mark left on a removed bond is not stale data — it silently marks another bond.
TEST_F(IndigoCoreAttachmentGroupsTest, HapticMarkDoesNotOutliveItsBond)
{
    Molecule mol;
    mol.addAtom(ELEM_C);
    mol.addAtom(ELEM_Fe);
    mol.addAtom(ELEM_C);
    const int bond = mol.addBond(0, 1, _BOND_COORDINATION);
    mol.attachment_groups.setBondHaptic(bond);

    mol.removeBond(bond);
    EXPECT_FALSE(mol.attachment_groups.isBondHaptic(bond));
    EXPECT_EQ(0, mol.attachment_groups.hapticBondCount());

    const int reused = mol.addBond(1, 2, BOND_SINGLE);
    ASSERT_EQ(bond, reused);
    EXPECT_FALSE(mol.attachment_groups.isBondHaptic(reused));
}

TEST_F(IndigoCoreAttachmentGroupsTest, HapticMarkDropsWithTheRemovedAtom)
{
    Molecule mol;
    mol.addAtom(ELEM_C);
    mol.addAtom(ELEM_Fe);
    mol.addAtom(ELEM_C);
    const int bond = mol.addBond(0, 1, _BOND_COORDINATION);
    mol.attachment_groups.setBondHaptic(bond);

    mol.removeAtom(1); // takes the marked bond with it
    EXPECT_EQ(0, mol.attachment_groups.hapticBondCount());

    const int reused = mol.addBond(0, 2, BOND_SINGLE);
    ASSERT_EQ(bond, reused);
    EXPECT_FALSE(mol.attachment_groups.isBondHaptic(reused));
}

TEST_F(IndigoCoreAttachmentGroupsTest, GroupIsDroppedWholeWhenAMemberAtomIsRemoved)
{
    Molecule mol;
    makeRingAndMetal(mol);
    mol.attachment_groups.group(addRingGroup(mol)).addBond(5, _BOND_COORDINATION);

    mol.removeAtom(2); // one of the five ring atoms
    EXPECT_EQ(0, mol.attachment_groups.groupCount());
    EXPECT_TRUE(mol.attachment_groups.isEmpty());
}

TEST_F(IndigoCoreAttachmentGroupsTest, RemovingThePartnerAtomDropsOnlyTheBond)
{
    Molecule mol;
    makeRingAndMetal(mol);
    const int group = addRingGroup(mol);
    mol.attachment_groups.group(group).addBond(5, _BOND_COORDINATION);

    mol.removeAtom(5); // the metal: a partner, not a member
    ASSERT_TRUE(mol.attachment_groups.hasGroup(group));
    EXPECT_EQ(5u, mol.attachment_groups.group(group).atoms().size());
    EXPECT_TRUE(mol.attachment_groups.group(group).bonds().empty());
}

TEST_F(IndigoCoreAttachmentGroupsTest, CloneKeepsGroupsAndMarks)
{
    Molecule source;
    makeRingAndMetal(source);
    AttachmentGroup& ag = source.attachment_groups.group(addRingGroup(source));
    ag.addBond(5, _BOND_COORDINATION);
    ag.setCharge(-1);
    ag.setRadical(2);
    source.attachment_groups.setBondHaptic(0);

    Molecule copy;
    copy.clone(source);

    ASSERT_EQ(1, copy.attachment_groups.groupCount());
    const AttachmentGroup& copied = copy.attachment_groups.group(copy.attachment_groups.begin());
    EXPECT_EQ(5u, copied.atoms().size());
    ASSERT_EQ(1u, copied.bonds().size());
    EXPECT_EQ(5, copied.bonds()[0].atom);
    EXPECT_EQ(_BOND_COORDINATION, copied.bonds()[0].order);
    EXPECT_EQ(-1, copied.charge());
    EXPECT_EQ(2, copied.radical());
    EXPECT_EQ(1, copy.attachment_groups.hapticBondCount());
}

TEST_F(IndigoCoreAttachmentGroupsTest, SkipFlagDropsGroupsAndMarks)
{
    Molecule source;
    makeRingAndMetal(source);
    source.attachment_groups.group(addRingGroup(source)).addBond(5, _BOND_COORDINATION);
    source.attachment_groups.setBondHaptic(0);

    Molecule copy;
    copy.clone(source, nullptr, nullptr, SKIP_ATTACHMENT_GROUPS);

    EXPECT_EQ(0, copy.attachment_groups.groupCount());
    EXPECT_EQ(0, copy.attachment_groups.hapticBondCount());
}

// A haptic bond means "all atoms of this group at once", so a group that would
// survive only in part is dropped whole rather than silently narrowed.
TEST_F(IndigoCoreAttachmentGroupsTest, PartialGroupIsDroppedWhole)
{
    Molecule source;
    makeRingAndMetal(source);
    source.attachment_groups.group(addRingGroup(source)).addBond(5, _BOND_COORDINATION);

    Array<int> vertices;
    for (int i = 0; i < 4; i++) // four of the five ring atoms
        vertices.push(i);

    Molecule partial;
    partial.makeSubmolecule(source, vertices, nullptr);
    EXPECT_EQ(0, partial.attachment_groups.groupCount());
}

TEST_F(IndigoCoreAttachmentGroupsTest, CompleteGroupSurvivesSubmoleculeAndIsRemapped)
{
    Molecule source;
    makeRingAndMetal(source);
    source.attachment_groups.group(addRingGroup(source)).addBond(5, _BOND_COORDINATION);

    Array<int> vertices; // whole ring plus the metal, in reverse order
    for (int i = 5; i >= 0; i--)
        vertices.push(i);

    Array<int> mapping;
    Molecule sub;
    sub.makeSubmolecule(source, vertices, &mapping);

    ASSERT_EQ(1, sub.attachment_groups.groupCount());
    const AttachmentGroup& ag = sub.attachment_groups.group(sub.attachment_groups.begin());
    ASSERT_EQ(5u, ag.atoms().size());
    for (int i = 0; i < 5; i++)
        EXPECT_TRUE(ag.hasAtom(mapping[i])) << "member atom " << i << " was not remapped";
    ASSERT_EQ(1u, ag.bonds().size());
    EXPECT_EQ(mapping[5], ag.bonds()[0].atom); // the partner followed the mapping
}

// A bond whose partner atom did not survive is dropped, while the group itself
// (all members present) stays.
TEST_F(IndigoCoreAttachmentGroupsTest, BondToDroppedPartnerIsRemoved)
{
    Molecule source;
    makeRingAndMetal(source);
    source.attachment_groups.group(addRingGroup(source)).addBond(5, _BOND_COORDINATION);

    Array<int> vertices; // ring only, metal left out
    for (int i = 0; i < 5; i++)
        vertices.push(i);

    Molecule sub;
    sub.makeSubmolecule(source, vertices, nullptr);

    ASSERT_EQ(1, sub.attachment_groups.groupCount());
    EXPECT_TRUE(sub.attachment_groups.group(sub.attachment_groups.begin()).bonds().empty());
}

TEST_F(IndigoCoreAttachmentGroupsTest, ClearRemovesEverything)
{
    Molecule mol;
    makeRingAndMetal(mol);
    mol.attachment_groups.group(addRingGroup(mol)).addBond(5, _BOND_COORDINATION);
    mol.attachment_groups.setBondHaptic(0);

    mol.clear();
    EXPECT_TRUE(mol.attachment_groups.isEmpty());
    EXPECT_EQ(0, mol.attachment_groups.groupCount());
}
