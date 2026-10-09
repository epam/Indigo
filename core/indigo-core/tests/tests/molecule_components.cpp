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

// Tests for BaseMolecule::moleculeComponents(), the components a molecule reports when
// haptic bonds join atoms (#3927): what holds a component together, how components
// are numbered, and which edits change the answer after it has been asked for once.

#include <gtest/gtest.h>

#include <list>
#include <unordered_set>
#include <vector>

#include <base_cpp/scanner.h>
#include <graph/filter.h>
#include <graph/graph_decomposer.h>
#include <molecule/base_molecule.h>
#include <molecule/molecule.h>
#include <molecule/molecule_haptic_bonds.h>
#include <molecule/molecule_standardize_options.h>
#include <molecule/query_molecule.h>
#include <molecule/smiles_loader.h>

#include "common.h"

using namespace indigo;

class IndigoCoreMoleculeComponentsTest : public IndigoCoreTest
{
protected:
    using Endpoint = HapticBond::Endpoint;

    static constexpr int RING_SIZE = 5;
    static constexpr int FERROCENE_ATOMS = 2 * RING_SIZE + 1; // two rings and the iron
    static constexpr int FERROCENE_BONDS = 2 * RING_SIZE;     // the ring bonds: a haptic bond is not an edge

    // A ring bound to a metal, by the names a test needs to edit the binding.
    struct Ligand
    {
        std::vector<int> atoms;
        int group = -1;
        int haptic_bond = -1;
    };

    struct Ferrocene
    {
        Ligand first;
        Ligand second;
        int iron = -1;
    };

    static std::vector<int> addRing(Molecule& mol)
    {
        std::vector<int> ring;
        for (int i = 0; i < RING_SIZE; i++)
            ring.push_back(mol.addAtom(ELEM_C));
        for (int i = 0; i < RING_SIZE; i++)
            mol.addBond(ring[i], ring[(i + 1) % RING_SIZE], BOND_SINGLE);
        return ring;
    }

    static Ligand bind(Molecule& mol, const std::vector<int>& ring, int atom, int type = _BOND_HAPTIC)
    {
        Ligand ligand;
        ligand.atoms = ring;
        ligand.group = mol.attachment_groups.addGroup();
        mol.attachment_groups.group(ligand.group).setAtoms(ring);
        ligand.haptic_bond = mol.addHapticBond(Endpoint::group(ligand.group), Endpoint::atom(atom), type);
        return ligand;
    }

    // Two rings that reach the iron through haptic bonds only, so the graph has
    // three pieces and the molecule one.
    static Ferrocene addFerrocene(Molecule& mol)
    {
        const std::vector<int> first_ring = addRing(mol);
        const std::vector<int> second_ring = addRing(mol);

        Ferrocene ferrocene;
        ferrocene.iron = mol.addAtom(ELEM_Fe);
        ferrocene.first = bind(mol, first_ring, ferrocene.iron);
        ferrocene.second = bind(mol, second_ring, ferrocene.iron);
        return ferrocene;
    }

    static bool isAlone(const GraphDecomposer& components, int atom)
    {
        return components.getComponentVerticesCount(components.getComponent(atom)) == 1;
    }
};

// ---- what holds a component together ----------------------------------------

TEST_F(IndigoCoreMoleculeComponentsTest, FerroceneIsOneComponent)
{
    Molecule mol;
    addFerrocene(mol);

    const GraphDecomposer& components = mol.moleculeComponents();
    ASSERT_EQ(1, components.getComponentsCount());
    EXPECT_EQ(FERROCENE_ATOMS, components.getComponentVerticesCount(0));
    EXPECT_EQ(FERROCENE_BONDS, components.getComponentEdgesCount(0));
    for (int atom : mol.vertices())
        EXPECT_EQ(0, components.getComponent(atom)) << "atom " << atom;
}

// The decomposer joins the atoms of a set without taking the set for edges: every
// pair of a ring and the iron would otherwise count, four times the bonds there are.
TEST_F(IndigoCoreMoleculeComponentsTest, ExternalNeighboursJoinAtomsWithoutAddingEdges)
{
    Molecule mol;
    addFerrocene(mol);
    std::list<std::unordered_set<int>> haptic_sets;
    mol.haptic_bonds.collectConnectivitySets(mol.attachment_groups, haptic_sets);

    GraphDecomposer decomposer(mol);
    ASSERT_EQ(1, decomposer.decompose(nullptr, nullptr, &haptic_sets));
    EXPECT_EQ(FERROCENE_ATOMS, decomposer.getComponentVerticesCount(0));
    EXPECT_EQ(FERROCENE_BONDS, decomposer.getComponentEdgesCount(0));
}

// A counter ion drawn between the atoms of the complex stays apart, and the complex,
// holding the lowest atom, comes first - the order the graph uses.
TEST_F(IndigoCoreMoleculeComponentsTest, CounterIonStaysApartOfTheComplex)
{
    Molecule mol;
    const std::vector<int> first_ring = addRing(mol);
    const int ion = mol.addAtom(ELEM_Cl);
    const std::vector<int> second_ring = addRing(mol);
    const int iron = mol.addAtom(ELEM_Fe);
    bind(mol, first_ring, iron);
    bind(mol, second_ring, iron);

    const GraphDecomposer& components = mol.moleculeComponents();
    ASSERT_EQ(2, components.getComponentsCount());
    const int complex = components.getComponent(first_ring.front());
    const int salt = components.getComponent(ion);
    EXPECT_EQ(0, complex);
    EXPECT_EQ(1, salt);
    EXPECT_EQ(complex, components.getComponent(second_ring.back()));
    EXPECT_EQ(complex, components.getComponent(iron));
    EXPECT_EQ(FERROCENE_ATOMS, components.getComponentVerticesCount(complex));
    EXPECT_EQ(1, components.getComponentVerticesCount(salt));
    EXPECT_EQ(0, components.getComponentEdgesCount(salt));
    EXPECT_TRUE(isAlone(components, ion));
    EXPECT_FALSE(isAlone(components, iron)) << "no edge reaches the iron, but two haptic bonds do";
}

TEST_F(IndigoCoreMoleculeComponentsTest, AtomToAtomHapticBondJoinsItsAtoms)
{
    Molecule mol;
    const int carbon = mol.addAtom(ELEM_C);
    const int iron = mol.addAtom(ELEM_Fe);
    mol.addHapticBond(Endpoint::atom(carbon), Endpoint::atom(iron));

    EXPECT_EQ(1, mol.moleculeComponents().getComponentsCount());
}

// A variable attachment still bonds the substituent to the ring; only the position
// on the ring is left open.
TEST_F(IndigoCoreMoleculeComponentsTest, VariableAttachmentJoinsItsAtoms)
{
    Molecule mol;
    const std::vector<int> ring = addRing(mol);
    const int substituent = mol.addAtom(ELEM_Cl);
    bind(mol, ring, substituent, _BOND_VARIABLE_ATTACHMENT);

    EXPECT_EQ(1, mol.moleculeComponents().getComponentsCount());
}

// A group is a set of atoms a bond may refer to. Until one does, it holds nothing.
TEST_F(IndigoCoreMoleculeComponentsTest, AttachmentGroupWithoutABondJoinsNothing)
{
    Molecule mol;
    loadMolecule("C.C", mol);
    const int group = mol.attachment_groups.addGroup();
    mol.attachment_groups.group(group).setAtoms({0, 1});

    EXPECT_EQ(2, mol.moleculeComponents().getComponentsCount());
}

// An s-group annotates atoms and bonds none of them: two fragments marked by one data
// s-group stay two components, although the KET saver keeps them in one node.
TEST_F(IndigoCoreMoleculeComponentsTest, SGroupJoinsNothing)
{
    Molecule mol;
    loadMolecule("CC.O", mol);
    SGroup& sgroup = mol.sgroups.getSGroup(mol.sgroups.addSGroup(SGroup::SG_TYPE_DAT));
    sgroup.atoms.push(0);
    sgroup.atoms.push(2);

    EXPECT_EQ(2, mol.moleculeComponents().getComponentsCount());
}

// A SMARTS component group says where its atoms may match; it bonds none of them.
TEST_F(IndigoCoreMoleculeComponentsTest, SmartsComponentGroupJoinsNothing)
{
    QueryMolecule query;
    BufferScanner scanner("(C.C)");
    SmilesLoader loader(scanner);
    loader.loadSMARTS(query);

    std::list<std::unordered_set<int>> neighbors;
    query.collectExternalNeighbors(neighbors);
    ASSERT_EQ(1, query.countComponents(neighbors)) << "the group is there and the KET saver follows it";

    EXPECT_EQ(2, query.moleculeComponents().getComponentsCount());
}

// Without haptic bonds the answer is the graph's, component by component and atom by
// atom - including a molecule whose atom indices have a hole.
TEST_F(IndigoCoreMoleculeComponentsTest, WithoutHapticBondsTheAnswerIsTheGraphs)
{
    for (const char* smiles : {"C", "CC.O.[Na+].[Cl-]", "C1CC1.C1.N.C1", "c1ccccc1CC(=O)O.[K+]"})
    {
        Molecule mol;
        loadMolecule(smiles, mol);
        mol.removeAtom(mol.vertexBegin());
        const GraphDecomposer& components = mol.moleculeComponents();

        ASSERT_EQ(mol.countComponents(), components.getComponentsCount()) << smiles;
        for (int atom : mol.vertices())
            EXPECT_EQ(mol.vertexComponent(atom), components.getComponent(atom)) << smiles << ", atom " << atom;
        for (int component = 0; component < components.getComponentsCount(); component++)
        {
            EXPECT_EQ(mol.countComponentVertices(component), components.getComponentVerticesCount(component)) << smiles;
            EXPECT_EQ(mol.countComponentEdges(component), components.getComponentEdgesCount(component)) << smiles;
        }
    }
}

// What the API does to clone a component: a filter over the decomposition.
TEST_F(IndigoCoreMoleculeComponentsTest, AComponentClonesIntoTheWholeComplex)
{
    Molecule mol;
    const std::vector<int> first_ring = addRing(mol);
    mol.addAtom(ELEM_Cl);
    const std::vector<int> second_ring = addRing(mol);
    const int iron = mol.addAtom(ELEM_Fe);
    bind(mol, first_ring, iron);
    bind(mol, second_ring, iron);

    const GraphDecomposer& components = mol.moleculeComponents();
    Filter atoms_of_the_complex(components.getDecomposition().ptr(), Filter::EQ, components.getComponent(iron));
    Molecule complex;
    complex.makeSubmolecule(mol, atoms_of_the_complex, nullptr, nullptr);

    EXPECT_EQ(FERROCENE_ATOMS, complex.vertexCount());
    EXPECT_EQ(2, complex.attachment_groups.groupCount());
    EXPECT_EQ(2, complex.haptic_bonds.count());
}

// ---- which edits change an answer already given -----------------------------

// The graph keeps its own decomposition when a vertex is added; this one does not.
TEST_F(IndigoCoreMoleculeComponentsTest, AnAtomAddedAfterAQueryIsSeen)
{
    Molecule mol;
    addFerrocene(mol);
    ASSERT_EQ(1, mol.moleculeComponents().getComponentsCount());

    const int lone = mol.addAtom(ELEM_Na);

    const GraphDecomposer& components = mol.moleculeComponents();
    EXPECT_EQ(2, components.getComponentsCount());
    EXPECT_TRUE(isAlone(components, lone));
}

// The index of a removed atom is handed to the next atom added. What was known of
// the old atom is not.
TEST_F(IndigoCoreMoleculeComponentsTest, AnAtomTakingAFreedIndexIsNotTheAtomThatHadIt)
{
    Molecule mol;
    loadMolecule("CC.O", mol);
    const int removed = 1;
    ASSERT_EQ(mol.moleculeComponents().getComponent(0), mol.moleculeComponents().getComponent(removed));

    mol.removeAtom(removed);
    ASSERT_EQ(2, mol.moleculeComponents().getComponentsCount());
    const int added = mol.addAtom(ELEM_Cl);
    ASSERT_EQ(removed, added);

    const GraphDecomposer& components = mol.moleculeComponents();
    EXPECT_EQ(3, components.getComponentsCount());
    EXPECT_TRUE(isAlone(components, added));
    EXPECT_NE(components.getComponent(0), components.getComponent(added));
}

TEST_F(IndigoCoreMoleculeComponentsTest, ABondAddedAfterAQueryJoinsItsAtoms)
{
    Molecule mol;
    loadMolecule("C.C", mol);
    ASSERT_EQ(2, mol.moleculeComponents().getComponentsCount());

    mol.addBond(0, 1, BOND_SINGLE);

    EXPECT_EQ(1, mol.moleculeComponents().getComponentsCount());
}

TEST_F(IndigoCoreMoleculeComponentsTest, ABondRemovedAfterAQueryPartsItsAtoms)
{
    Molecule mol;
    loadMolecule("CC", mol);
    ASSERT_EQ(1, mol.moleculeComponents().getComponentsCount());

    mol.removeBond(mol.edgeBegin());

    EXPECT_EQ(2, mol.moleculeComponents().getComponentsCount());
}

// flipBondWithDirection() moves the end of a bond without removing the bond, past the
// calls of Graph that add and remove edges.
TEST_F(IndigoCoreMoleculeComponentsTest, ABondMovedToAnotherAtomIsSeen)
{
    Molecule mol;
    loadMolecule("CC.C", mol);
    const int kept = 0;
    const int left = 1;
    const int taken = 2;
    ASSERT_EQ(mol.moleculeComponents().getComponent(kept), mol.moleculeComponents().getComponent(left));

    const int no_leaving_atom = -1;
    mol.flipBondWithDirection(kept, left, taken, no_leaving_atom);

    const GraphDecomposer& components = mol.moleculeComponents();
    EXPECT_EQ(components.getComponent(kept), components.getComponent(taken));
    EXPECT_TRUE(isAlone(components, left));
}

TEST_F(IndigoCoreMoleculeComponentsTest, AMergedMoleculeBringsItsComponents)
{
    Molecule mol;
    addFerrocene(mol);
    ASSERT_EQ(1, mol.moleculeComponents().getComponentsCount());

    Molecule ions;
    loadMolecule("[Na+].[Cl-]", ions);
    mol.mergeWithMolecule(ions, nullptr);
    ASSERT_EQ(3, mol.moleculeComponents().getComponentsCount()) << "two atoms and no bond were added";

    Molecule another;
    addFerrocene(another);
    Array<int> mapping;
    mol.mergeWithMolecule(another, &mapping);

    const GraphDecomposer& components = mol.moleculeComponents();
    ASSERT_EQ(4, components.getComponentsCount());
    EXPECT_EQ(FERROCENE_ATOMS, components.getComponentVerticesCount(components.getComponent(mapping[another.vertexBegin()])));
}

TEST_F(IndigoCoreMoleculeComponentsTest, ACloneReplacesTheAnswerOfItsTarget)
{
    Molecule source;
    addFerrocene(source);
    Molecule target;
    loadMolecule("C.C.C", target);
    ASSERT_EQ(3, target.moleculeComponents().getComponentsCount());

    target.clone(source);

    EXPECT_EQ(1, target.moleculeComponents().getComponentsCount());
    EXPECT_EQ(1, source.moleculeComponents().getComponentsCount());
}

// clone_KeepIndices() fills the graph of an empty molecule by a path of its own.
TEST_F(IndigoCoreMoleculeComponentsTest, AnIndexKeepingCloneReplacesTheAnswerOfItsTarget)
{
    Molecule source;
    addFerrocene(source);
    source.addAtom(ELEM_Na);
    Molecule target;
    ASSERT_EQ(0, target.moleculeComponents().getComponentsCount());

    target.clone_KeepIndices(source);

    EXPECT_EQ(2, target.moleculeComponents().getComponentsCount());
}

TEST_F(IndigoCoreMoleculeComponentsTest, AClearedMoleculeHasNoComponents)
{
    Molecule mol;
    addFerrocene(mol);
    ASSERT_EQ(1, mol.moleculeComponents().getComponentsCount());

    mol.clear();

    EXPECT_EQ(0, mol.moleculeComponents().getComponentsCount());
}

TEST_F(IndigoCoreMoleculeComponentsTest, AHapticBondAddedAfterAQueryJoinsItsAtoms)
{
    Molecule mol;
    const std::vector<int> ring = addRing(mol);
    const int iron = mol.addAtom(ELEM_Fe);
    ASSERT_EQ(2, mol.moleculeComponents().getComponentsCount());

    bind(mol, ring, iron);

    EXPECT_EQ(1, mol.moleculeComponents().getComponentsCount());
}

TEST_F(IndigoCoreMoleculeComponentsTest, ARemovedHapticBondFreesItsRing)
{
    Molecule mol;
    const Ferrocene ferrocene = addFerrocene(mol);
    ASSERT_EQ(1, mol.moleculeComponents().getComponentsCount());

    mol.removeHapticBond(ferrocene.first.haptic_bond);

    const GraphDecomposer& components = mol.moleculeComponents();
    EXPECT_EQ(2, components.getComponentsCount());
    EXPECT_NE(components.getComponent(ferrocene.iron), components.getComponent(ferrocene.first.atoms.front()));
    EXPECT_EQ(components.getComponent(ferrocene.iron), components.getComponent(ferrocene.second.atoms.front()));
}

TEST_F(IndigoCoreMoleculeComponentsTest, ARemovedGroupTakesItsBondAndFreesItsRing)
{
    Molecule mol;
    const Ferrocene ferrocene = addFerrocene(mol);
    ASSERT_EQ(1, mol.moleculeComponents().getComponentsCount());

    mol.removeAttachmentGroup(ferrocene.first.group);

    const GraphDecomposer& components = mol.moleculeComponents();
    EXPECT_EQ(2, components.getComponentsCount());
    EXPECT_NE(components.getComponent(ferrocene.iron), components.getComponent(ferrocene.first.atoms.front()));
}

TEST_F(IndigoCoreMoleculeComponentsTest, NewAtomsOfAGroupTakeTheBondFromTheOldOnes)
{
    Molecule mol;
    const Ferrocene ferrocene = addFerrocene(mol);
    const std::vector<int> third_ring = addRing(mol);
    ASSERT_EQ(2, mol.moleculeComponents().getComponentsCount());

    mol.setAttachmentGroupAtoms(ferrocene.first.group, third_ring);

    const GraphDecomposer& components = mol.moleculeComponents();
    EXPECT_EQ(2, components.getComponentsCount());
    EXPECT_EQ(components.getComponent(ferrocene.iron), components.getComponent(third_ring.front()));
    EXPECT_NE(components.getComponent(ferrocene.iron), components.getComponent(ferrocene.first.atoms.front()));
}

// A group that loses a member is dropped whole with its bond (invariant A7), so the
// rest of the ring is no longer held.
TEST_F(IndigoCoreMoleculeComponentsTest, ARemovedGroupMemberFreesTheRestOfTheRing)
{
    Molecule mol;
    const Ferrocene ferrocene = addFerrocene(mol);
    ASSERT_EQ(1, mol.moleculeComponents().getComponentsCount());

    mol.removeAtom(ferrocene.first.atoms.front());

    const GraphDecomposer& components = mol.moleculeComponents();
    ASSERT_EQ(2, components.getComponentsCount());
    const int freed = components.getComponent(ferrocene.first.atoms.back());
    EXPECT_NE(components.getComponent(ferrocene.iron), freed);
    EXPECT_EQ(RING_SIZE - 1, components.getComponentVerticesCount(freed));
}

// ---- the two decompositions stay apart --------------------------------------

// The graph's cache may hold a decomposition made with s-group neighbours, as any
// caller of countComponents(neighbors) leaves it. That is not this answer.
TEST_F(IndigoCoreMoleculeComponentsTest, TheGraphsCacheDoesNotLeakIn)
{
    Molecule mol;
    loadMolecule("CC.O", mol);
    SGroup& sgroup = mol.sgroups.getSGroup(mol.sgroups.addSGroup(SGroup::SG_TYPE_DAT));
    sgroup.atoms.push(0);
    sgroup.atoms.push(2);

    std::list<std::unordered_set<int>> neighbors;
    mol.collectExternalNeighbors(neighbors);
    ASSERT_EQ(1, mol.countComponents(neighbors));

    EXPECT_EQ(2, mol.moleculeComponents().getComponentsCount());
}

// And the other way round: SMILES, InChI and substructure search keep seeing the graph.
TEST_F(IndigoCoreMoleculeComponentsTest, TheGraphKeepsItsOwnAnswer)
{
    Molecule mol;
    addFerrocene(mol);
    ASSERT_EQ(1, mol.moleculeComponents().getComponentsCount());

    EXPECT_EQ(3, mol.countComponents());
}

TEST_F(IndigoCoreMoleculeComponentsTest, EmptyMoleculeHasNoComponents)
{
    Molecule mol;
    EXPECT_EQ(0, mol.moleculeComponents().getComponentsCount());
}

// ---- standardize ------------------------------------------------------------

// A ferrocene beside a sodium and a chloride that nothing bonds: to the fragment
// options the complex is one fragment of eleven atoms, and its iron is not alone.
class IndigoCoreStandardizeHapticTest : public IndigoCoreMoleculeComponentsTest
{
protected:
    static constexpr int ION_COUNT = 2;

    static void addFerroceneWithIons(Molecule& mol)
    {
        addFerrocene(mol);
        mol.addAtom(ELEM_Na);
        mol.addAtom(ELEM_Cl);
    }
};

TEST_F(IndigoCoreStandardizeHapticTest, RemoveSingleAtomsKeepsTheMetalOfAComplex)
{
    Molecule mol;
    addFerroceneWithIons(mol);

    StandardizeOptions options;
    options.remove_single_atom_fragments = true;
    mol.standardize(options);

    EXPECT_EQ(FERROCENE_ATOMS, mol.vertexCount());
    EXPECT_EQ(2, mol.haptic_bonds.count());
}

TEST_F(IndigoCoreStandardizeHapticTest, KeepLargestKeepsTheWholeComplex)
{
    Molecule mol;
    addFerroceneWithIons(mol);

    StandardizeOptions options;
    options.keep_largest_fragment = true;
    mol.standardize(options);

    EXPECT_EQ(FERROCENE_ATOMS, mol.vertexCount());
    EXPECT_EQ(2, mol.haptic_bonds.count());
}

TEST_F(IndigoCoreStandardizeHapticTest, KeepSmallestDropsTheWholeComplex)
{
    Molecule mol;
    addFerroceneWithIons(mol);

    StandardizeOptions options;
    options.keep_smallest_fragment = true;
    mol.standardize(options);

    ASSERT_EQ(1, mol.vertexCount());
    EXPECT_NE(ELEM_Fe, mol.getAtomNumber(mol.vertexBegin())) << "an ion is the smallest fragment; the iron is a part of the largest";
    EXPECT_TRUE(mol.haptic_bonds.isEmpty());
}

TEST_F(IndigoCoreStandardizeHapticTest, RemoveLargestDropsTheWholeComplex)
{
    Molecule mol;
    addFerroceneWithIons(mol);

    StandardizeOptions options;
    options.remove_largest_fragment = true;
    mol.standardize(options);

    EXPECT_EQ(ION_COUNT, mol.vertexCount());
    EXPECT_TRUE(mol.haptic_bonds.isEmpty());
}

// A chloride is a chlorine nothing holds; one on a variable attachment is a substituent.
TEST_F(IndigoCoreStandardizeHapticTest, ChargesLeaveABoundHalogenNeutral)
{
    Molecule mol;
    const std::vector<int> ring = addRing(mol);
    const int substituent = mol.addAtom(ELEM_Cl);
    bind(mol, ring, substituent, _BOND_VARIABLE_ATTACHMENT);
    const int lone = mol.addAtom(ELEM_Cl);

    StandardizeOptions options;
    options.standardize_charges = true;
    mol.standardize(options);

    EXPECT_EQ(0, mol.getAtomCharge(substituent));
    EXPECT_EQ(-1, mol.getAtomCharge(lone));
}
