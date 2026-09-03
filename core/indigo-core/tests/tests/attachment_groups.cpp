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

// Tests for the attachment groups behind haptic bonds (#3233, #3837): stable
// indices, slot reuse, membership as a set, the all-or-nothing rule on removal,
// and the anchor atom a group points at without owning it. The bonds that
// address these groups are tested in haptic_bonds.cpp.

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
        mol.attachment_groups.group(idx).setAtoms({0, 1, 2, 3, 4});
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
    const int b = groups.addGroup();
    EXPECT_EQ(0, a);
    EXPECT_EQ(1, b);
    EXPECT_EQ(2, groups.groupCount());
    EXPECT_FALSE(groups.isEmpty());
}

TEST_F(IndigoCoreAttachmentGroupsTest, RemoveFreesSlotAndReusesItClean)
{
    Molecule mol;
    mol.addAtom(ELEM_C);
    auto& groups = mol.attachment_groups;
    groups.addGroup();
    const int b = groups.addGroup();
    groups.addGroup();
    groups.group(b).addAtom(0);
    groups.group(b).setAnchorAtom(0);

    mol.removeAttachmentGroup(b);
    EXPECT_EQ(2, groups.groupCount());
    EXPECT_FALSE(groups.hasGroup(b));

    const int reused = groups.addGroup();
    EXPECT_EQ(b, reused); // stable indices: the freed slot comes back
    EXPECT_TRUE(groups.group(reused).atoms().empty());
    EXPECT_EQ(-1, groups.group(reused).anchorAtom()); // no stale anchor
}

TEST_F(IndigoCoreAttachmentGroupsTest, IterationSkipsRemovedGroups)
{
    Molecule mol;
    auto& groups = mol.attachment_groups;
    const int a = groups.addGroup();
    const int b = groups.addGroup();
    const int c = groups.addGroup();
    mol.removeAttachmentGroup(b);

    std::vector<int> seen;
    for (int i = groups.begin(); i != groups.end(); i = groups.next(i))
        seen.push_back(i);
    EXPECT_EQ(std::vector<int>({a, c}), seen);
}

TEST_F(IndigoCoreAttachmentGroupsTest, AccessToMissingGroupThrows)
{
    Molecule mol;
    EXPECT_THROW(mol.attachment_groups.group(0), Exception);
    EXPECT_THROW(mol.removeAttachmentGroup(0), Exception);
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

TEST_F(IndigoCoreAttachmentGroupsTest, RemapKeepsAllMembersOrNone)
{
    MoleculeAttachmentGroups groups;
    AttachmentGroup& ag = groups.group(groups.addGroup());
    ag.setAtoms({0, 1, 2});

    Array<int> mapping;
    mapping.clear_resize(3);
    mapping[0] = 5;
    mapping[1] = 6;
    mapping[2] = 7;
    ASSERT_TRUE(ag.remapAtoms(mapping));
    EXPECT_EQ(std::vector<int>({5, 6, 7}), ag.atoms());

    mapping[1] = -1;
    EXPECT_FALSE(ag.remapAtoms(mapping));
    EXPECT_EQ(std::vector<int>({5, 6, 7}), ag.atoms()); // left untouched for the caller to drop
}

// ---- contract with the molecule -------------------------------------------

// A haptic bond means "all atoms of this group at once", so a group that would
// survive only in part is dropped whole rather than silently narrowed.
TEST_F(IndigoCoreAttachmentGroupsTest, GroupIsDroppedWholeWhenAMemberAtomIsRemoved)
{
    Molecule mol;
    makeRingAndMetal(mol);
    addRingGroup(mol);

    mol.removeAtom(2); // one of the five ring atoms
    EXPECT_EQ(0, mol.attachment_groups.groupCount());
    EXPECT_TRUE(mol.attachment_groups.isEmpty());
}

TEST_F(IndigoCoreAttachmentGroupsTest, RemovingANonMemberAtomLeavesTheGroupAlone)
{
    Molecule mol;
    makeRingAndMetal(mol);
    const int group = addRingGroup(mol);

    mol.removeAtom(5); // the metal is not a member
    ASSERT_TRUE(mol.attachment_groups.hasGroup(group));
    EXPECT_EQ(5u, mol.attachment_groups.group(group).atoms().size());
}

// A group is a set of atoms and nothing else: the charge of the pi system stays on
// the atom the file put it on, and cloning must not invent one anywhere.
TEST_F(IndigoCoreAttachmentGroupsTest, GroupCarriesNoChargeOfItsOwn)
{
    Molecule source;
    makeRingAndMetal(source);
    source.setAtomCharge(0, -1); // as a V3000 file may write it, on a ring atom
    addRingGroup(source);

    Molecule copy;
    copy.clone(source);

    ASSERT_EQ(1, copy.attachment_groups.groupCount());
    EXPECT_EQ(5u, copy.attachment_groups.group(copy.attachment_groups.begin()).atoms().size());

    int total = 0;
    for (int i = copy.vertexBegin(); i != copy.vertexEnd(); i = copy.vertexNext(i))
        total += copy.getAtomCharge(i);
    EXPECT_EQ(-1, total) << "the charge must stay exactly where the file put it";
}

// The anchor is a reference to an atom, so it follows the atoms through every
// mapping — otherwise a saver would write the ENDPTS of one group next to the star
// of another.
TEST_F(IndigoCoreAttachmentGroupsTest, AnchorAtomFollowsTheAtomMapping)
{
    Molecule source;
    makeRingAndMetal(source);
    const int star = source.addAtom(-1); // the star of a V3000 ENDPTS record
    source.setPseudoAtom(star, "*");
    source.attachment_groups.group(addRingGroup(source)).setAnchorAtom(star);

    Molecule copy;
    copy.clone(source);
    EXPECT_EQ(star, copy.attachment_groups.group(copy.attachment_groups.begin()).anchorAtom());

    Array<int> vertices; // whole ring, the metal and the star, in reverse order
    for (int i = star; i >= 0; i--)
        vertices.push(i);

    Array<int> mapping;
    Molecule sub;
    sub.makeSubmolecule(source, vertices, &mapping);
    ASSERT_EQ(1, sub.attachment_groups.groupCount());
    EXPECT_EQ(mapping[star], sub.attachment_groups.group(sub.attachment_groups.begin()).anchorAtom());
}

// Losing the anchor is not losing the group: the group is still a set of atoms,
// and a saver that needs a star puts a fresh one at their centre.
TEST_F(IndigoCoreAttachmentGroupsTest, GroupOutlivesItsAnchorAtom)
{
    Molecule mol;
    makeRingAndMetal(mol);
    const int star = mol.addAtom(-1);
    mol.setPseudoAtom(star, "*");
    const int group = addRingGroup(mol);
    mol.attachment_groups.group(group).setAnchorAtom(star);

    mol.removeAtom(star);

    ASSERT_TRUE(mol.attachment_groups.hasGroup(group));
    EXPECT_EQ(5u, mol.attachment_groups.group(group).atoms().size());
    EXPECT_EQ(-1, mol.attachment_groups.group(group).anchorAtom());
}

TEST_F(IndigoCoreAttachmentGroupsTest, SkipFlagDropsGroups)
{
    Molecule source;
    makeRingAndMetal(source);
    addRingGroup(source);

    Molecule copy;
    copy.clone(source, nullptr, nullptr, SKIP_ATTACHMENT_GROUPS);

    EXPECT_EQ(0, copy.attachment_groups.groupCount());
}

TEST_F(IndigoCoreAttachmentGroupsTest, PartialGroupIsDroppedWhole)
{
    Molecule source;
    makeRingAndMetal(source);
    addRingGroup(source);

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
    addRingGroup(source);

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
}

TEST_F(IndigoCoreAttachmentGroupsTest, ClearRemovesEverything)
{
    Molecule mol;
    makeRingAndMetal(mol);
    addRingGroup(mol);

    mol.clear();
    EXPECT_TRUE(mol.attachment_groups.isEmpty());
    EXPECT_EQ(0, mol.attachment_groups.groupCount());
}
