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
#include <molecule/molecule.h>
#include <molecule/molecule_attachment_groups.h>
#include <molecule/molecule_haptic_bonds.h>
#include <molecule/query_molecule.h>

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

    // Cyclopentadienyl ring (atoms 0..4) plus an iron atom (5): the ferrocene
    // half the ticket's example is made of.
    static void makeRingAndMetal(Molecule& mol)
    {
        for (int i = 0; i < 5; i++)
            mol.addAtom(ELEM_C);
        mol.addAtom(ELEM_Fe);
        for (int i = 0; i < 5; i++)
            mol.addBond(i, (i + 1) % 5, BOND_SINGLE);
    }

    static int addRingGroup(BaseMolecule& mol)
    {
        const int idx = mol.attachment_groups.addGroup();
        mol.attachment_groups.group(idx).setAtoms({0, 1, 2, 3, 4});
        return idx;
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

// ---- contract with the molecule -------------------------------------------

TEST_F(IndigoCoreHapticBondsTest, AddHapticBondValidatesItsEndpoints)
{
    Molecule mol;
    makeRingAndMetal(mol);
    const int group = addRingGroup(mol);
    const int empty_group = mol.attachment_groups.addGroup();

    EXPECT_THROW(mol.addHapticBond(Endpoint::group(group), Endpoint::atom(42)), Exception);                    // no such atom
    EXPECT_THROW(mol.addHapticBond(Endpoint::group(42), Endpoint::atom(5)), Exception);                        // no such group
    EXPECT_THROW(mol.addHapticBond(Endpoint::group(empty_group), Endpoint::atom(5)), Exception);               // empty group
    EXPECT_THROW(mol.addHapticBond(Endpoint::group(group), Endpoint::group(empty_group)), Exception);          // two groups
    EXPECT_THROW(mol.addHapticBond(Endpoint::group(group), Endpoint::atom(0)), Exception);                     // partner is a member
    EXPECT_THROW(mol.addHapticBond(Endpoint::atom(5), Endpoint::atom(5)), Exception);                          // one atom twice
    EXPECT_THROW(mol.addHapticBond(Endpoint::group(group), Endpoint::atom(5), _BOND_COORDINATION), Exception); // format code

    EXPECT_EQ(0, mol.haptic_bonds.count());
}

TEST_F(IndigoCoreHapticBondsTest, AddHapticBondStoresTheBondAndMovesTheEditRevision)
{
    Molecule mol;
    makeRingAndMetal(mol);
    const int group = addRingGroup(mol);

    const int revision_before = mol.getEditRevision();
    const int bond = mol.addHapticBond(Endpoint::group(group), Endpoint::atom(5));

    ASSERT_TRUE(mol.haptic_bonds.has(bond));
    EXPECT_EQ(_BOND_HAPTIC, mol.haptic_bonds.at(bond).type());
    EXPECT_EQ(0, mol.haptic_bonds.firstAtom(bond, mol.attachment_groups));
    EXPECT_EQ(5, mol.haptic_bonds.lastAtom(bond, mol.attachment_groups));
    EXPECT_NE(revision_before, mol.getEditRevision());
}

// The method is not virtual, so every subclass gets exactly the same behaviour.
TEST_F(IndigoCoreHapticBondsTest, AddHapticBondWorksOnAQueryMoleculeToo)
{
    QueryMolecule qmol;
    for (int i = 0; i < 3; i++)
        qmol.addAtom(new QueryMolecule::Atom(QueryMolecule::ATOM_NUMBER, ELEM_C));

    const int group = qmol.attachment_groups.addGroup();
    qmol.attachment_groups.group(group).setAtoms({0, 1});

    const int bond = qmol.addHapticBond(Endpoint::group(group), Endpoint::atom(2));
    ASSERT_TRUE(qmol.haptic_bonds.has(bond));
    EXPECT_EQ(2, qmol.haptic_bonds.lastAtom(bond, qmol.attachment_groups));
}

// The isolation invariant: a haptic bond must leave every atom's valence and
// implicit hydrogen count exactly as it was — BIOVIA classes it with the
// "zero-order bonds, which are bonds that do not affect valence".
TEST_F(IndigoCoreHapticBondsTest, ZeroOrderInvariantValenceUntouched)
{
    Molecule plain;
    makeRingAndMetal(plain);

    Molecule with_bond;
    makeRingAndMetal(with_bond);
    with_bond.addHapticBond(Endpoint::group(addRingGroup(with_bond)), Endpoint::atom(5));

    for (int i = plain.vertexBegin(); i != plain.vertexEnd(); i = plain.vertexNext(i))
    {
        EXPECT_EQ(plain.getAtomValence(i), with_bond.getAtomValence(i)) << "valence changed for atom " << i;
        EXPECT_EQ(plain.getImplicitH(i), with_bond.getImplicitH(i)) << "implicit H changed for atom " << i;
    }
}

TEST_F(IndigoCoreHapticBondsTest, AtomToAtomHapticBondIsNotAGraphEdge)
{
    Molecule mol;
    mol.addAtom(ELEM_C);
    mol.addAtom(ELEM_Fe);

    const int edges_before = mol.edgeCount();
    mol.addHapticBond(Endpoint::atom(0), Endpoint::atom(1));

    EXPECT_EQ(edges_before, mol.edgeCount());
    EXPECT_EQ(-1, mol.findEdgeIndex(0, 1));
}

TEST_F(IndigoCoreHapticBondsTest, RemovingAMemberAtomDropsTheGroupAndItsBonds)
{
    Molecule mol;
    makeRingAndMetal(mol);
    mol.addHapticBond(Endpoint::group(addRingGroup(mol)), Endpoint::atom(5));

    mol.removeAtom(2); // one of the five ring atoms
    EXPECT_EQ(0, mol.attachment_groups.groupCount());
    EXPECT_EQ(0, mol.haptic_bonds.count());
}

TEST_F(IndigoCoreHapticBondsTest, RemovingThePartnerAtomDropsOnlyTheBond)
{
    Molecule mol;
    makeRingAndMetal(mol);
    const int group = addRingGroup(mol);
    mol.addHapticBond(Endpoint::group(group), Endpoint::atom(5));

    mol.removeAtom(5); // the metal: a partner, not a member
    ASSERT_TRUE(mol.attachment_groups.hasGroup(group));
    EXPECT_EQ(5u, mol.attachment_groups.group(group).atoms().size());
    EXPECT_EQ(0, mol.haptic_bonds.count());
}

// The group pool hands a freed index straight back to the next addGroup, so a
// bond left behind would not be stale data — it would attach to another group.
TEST_F(IndigoCoreHapticBondsTest, BondDoesNotOutliveTheGroupItAddresses)
{
    Molecule mol;
    makeRingAndMetal(mol);
    const int group = addRingGroup(mol);
    mol.addHapticBond(Endpoint::group(group), Endpoint::atom(5));

    mol.removeAttachmentGroup(group);
    EXPECT_EQ(0, mol.haptic_bonds.count());

    const int reused = mol.attachment_groups.addGroup();
    ASSERT_EQ(group, reused); // the freed slot comes back
    EXPECT_EQ(0, mol.haptic_bonds.count());
}

TEST_F(IndigoCoreHapticBondsTest, CloneKeepsBondsAndTheirGroups)
{
    Molecule source;
    makeRingAndMetal(source);
    const int group = addRingGroup(source);
    source.addHapticBond(Endpoint::group(group), Endpoint::atom(5));
    source.addHapticBond(Endpoint::atom(0), Endpoint::atom(5), _BOND_VARIABLE_ATTACHMENT);

    Molecule copy;
    copy.clone(source);

    ASSERT_EQ(1, copy.attachment_groups.groupCount());
    ASSERT_EQ(2, copy.haptic_bonds.count());

    const int copied_group = copy.attachment_groups.begin();
    const HapticBond& first = copy.haptic_bonds.at(copy.haptic_bonds.begin());
    ASSERT_TRUE(first.begin().isGroup());
    EXPECT_EQ(copied_group, first.begin().index()); // the group reference followed the merge
    EXPECT_EQ(5, first.end().index());

    const HapticBond& second = copy.haptic_bonds.at(copy.haptic_bonds.next(copy.haptic_bonds.begin()));
    EXPECT_EQ(_BOND_VARIABLE_ATTACHMENT, second.type());
}

TEST_F(IndigoCoreHapticBondsTest, SkipFlagDropsBondsAndGroups)
{
    Molecule source;
    makeRingAndMetal(source);
    source.addHapticBond(Endpoint::group(addRingGroup(source)), Endpoint::atom(5));

    Molecule copy;
    copy.clone(source, nullptr, nullptr, SKIP_ATTACHMENT_GROUPS);

    EXPECT_EQ(0, copy.attachment_groups.groupCount());
    EXPECT_EQ(0, copy.haptic_bonds.count());
}

TEST_F(IndigoCoreHapticBondsTest, PartialGroupTakesItsBondsWithIt)
{
    Molecule source;
    makeRingAndMetal(source);
    source.addHapticBond(Endpoint::group(addRingGroup(source)), Endpoint::atom(5));

    Array<int> vertices;
    for (int i = 0; i < 4; i++) // four of the five ring atoms
        vertices.push(i);

    Molecule partial;
    partial.makeSubmolecule(source, vertices, nullptr);
    EXPECT_EQ(0, partial.attachment_groups.groupCount());
    EXPECT_EQ(0, partial.haptic_bonds.count());
}

TEST_F(IndigoCoreHapticBondsTest, CompleteGroupSurvivesSubmoleculeAndIsRemapped)
{
    Molecule source;
    makeRingAndMetal(source);
    source.addHapticBond(Endpoint::group(addRingGroup(source)), Endpoint::atom(5));

    Array<int> vertices; // whole ring plus the metal, in reverse order
    for (int i = 5; i >= 0; i--)
        vertices.push(i);

    Array<int> mapping;
    Molecule sub;
    sub.makeSubmolecule(source, vertices, &mapping);

    ASSERT_EQ(1, sub.attachment_groups.groupCount());
    ASSERT_EQ(1, sub.haptic_bonds.count());
    const HapticBond& bond = sub.haptic_bonds.at(sub.haptic_bonds.begin());
    EXPECT_EQ(sub.attachment_groups.begin(), bond.begin().index());
    EXPECT_EQ(mapping[5], bond.end().index()); // the partner followed the mapping
}

TEST_F(IndigoCoreHapticBondsTest, BondToADroppedPartnerIsRemovedButTheGroupStays)
{
    Molecule source;
    makeRingAndMetal(source);
    source.addHapticBond(Endpoint::group(addRingGroup(source)), Endpoint::atom(5));

    Array<int> vertices; // ring only, metal left out
    for (int i = 0; i < 5; i++)
        vertices.push(i);

    Molecule sub;
    sub.makeSubmolecule(source, vertices, nullptr);

    EXPECT_EQ(1, sub.attachment_groups.groupCount());
    EXPECT_EQ(0, sub.haptic_bonds.count());
}

TEST_F(IndigoCoreHapticBondsTest, ClearRemovesBondsFromTheMolecule)
{
    Molecule mol;
    makeRingAndMetal(mol);
    mol.addHapticBond(Endpoint::group(addRingGroup(mol)), Endpoint::atom(5));

    mol.clear();
    EXPECT_TRUE(mol.haptic_bonds.isEmpty());
    EXPECT_TRUE(mol.attachment_groups.isEmpty());
}

// ---- connectivity ---------------------------------------------------------

// Ferrocene: two rings that reach the iron through haptic bonds only. The graph
// has three pieces; the molecule is one.
static void makeFerrocene(Molecule& mol)
{
    for (int ring = 0; ring < 2; ring++)
    {
        const int first = ring * 5;
        for (int i = 0; i < 5; i++)
            mol.addAtom(ELEM_C);
        for (int i = 0; i < 5; i++)
            mol.addBond(first + i, first + (i + 1) % 5, BOND_SINGLE);
    }
    const int metal = mol.addAtom(ELEM_Fe);

    for (int ring = 0; ring < 2; ring++)
    {
        const int group = mol.attachment_groups.addGroup();
        mol.attachment_groups.group(group).setAtoms({ring * 5, ring * 5 + 1, ring * 5 + 2, ring * 5 + 3, ring * 5 + 4});
        mol.addHapticBond(HapticBond::Endpoint::group(group), HapticBond::Endpoint::atom(metal));
    }
}

TEST_F(IndigoCoreHapticBondsTest, ExternalNeighborsKeepTheComplexWhole)
{
    Molecule mol;
    makeFerrocene(mol);

    std::list<std::unordered_set<int>> neighbors;
    mol.collectExternalNeighbors(neighbors);
    EXPECT_EQ(1, mol.countComponents(neighbors));
}

// Without them the graph is what it is: three disconnected pieces. That is the
// answer InChI and the standardizer need, since a haptic bond is dropped on
// export and the disconnection is then expected.
TEST_F(IndigoCoreHapticBondsTest, PlainComponentCountLeavesTheComplexApart)
{
    Molecule mol;
    makeFerrocene(mol);
    EXPECT_EQ(3, mol.countComponents());
}

// Both overloads share one cache, so without a mode flag the answer would
// depend on which of them ran first.
TEST_F(IndigoCoreHapticBondsTest, DecompositionDoesNotDependOnTheOrderOfTheCalls)
{
    Molecule mol;
    makeFerrocene(mol);

    ASSERT_EQ(3, mol.countComponents()); // fills the cache without external neighbours

    std::list<std::unordered_set<int>> neighbors;
    mol.collectExternalNeighbors(neighbors);
    EXPECT_EQ(1, mol.countComponents(neighbors));

    // and the decomposition that follows is the one just computed
    const Array<int>& decomposition = mol.getDecomposition();
    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        EXPECT_EQ(0, decomposition[i]) << "atom " << i << " landed in another component";
}

TEST_F(IndigoCoreHapticBondsTest, AtomToAtomHapticBondAlsoHoldsTheFragmentTogether)
{
    Molecule mol;
    mol.addAtom(ELEM_C);
    mol.addAtom(ELEM_Fe);
    mol.addHapticBond(Endpoint::atom(0), Endpoint::atom(1));

    std::list<std::unordered_set<int>> neighbors;
    mol.collectExternalNeighbors(neighbors);
    EXPECT_EQ(2, mol.countComponents());
    EXPECT_EQ(1, mol.countComponents(neighbors));
}
