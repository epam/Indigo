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

// Tests for the components a molecule reports when haptic bonds join atoms (#3927):
// what holds a component together, how components are numbered, and when the cached
// answer is rebuilt.

#include <gtest/gtest.h>

#include <list>
#include <unordered_set>

#include <graph/filter.h>
#include <molecule/base_molecule.h>
#include <molecule/molecule.h>
#include <molecule/molecule_components.h>
#include <molecule/molecule_haptic_bonds.h>
#include <molecule/molecule_standardize_options.h>

#include "common.h"

using namespace indigo;

class IndigoCoreMoleculeComponentsTest : public IndigoCoreTest
{
protected:
    using Endpoint = HapticBond::Endpoint;

    static void addRing(Molecule& mol, int first)
    {
        for (int i = 0; i < 5; i++)
            mol.addAtom(ELEM_C);
        for (int i = 0; i < 5; i++)
            mol.addBond(first + i, first + (i + 1) % 5, BOND_SINGLE);
    }

    static int addGroup(Molecule& mol, int first)
    {
        const int group = mol.attachment_groups.addGroup();
        mol.attachment_groups.group(group).setAtoms({first, first + 1, first + 2, first + 3, first + 4});
        return group;
    }

    // Ferrocene as #3927 attached it: two rings reach the iron (10) through haptic
    // bonds only, so the graph has three pieces and the molecule one.
    static void makeFerrocene(Molecule& mol)
    {
        addRing(mol, 0);
        addRing(mol, 5);
        const int metal = mol.addAtom(ELEM_Fe);
        mol.addHapticBond(Endpoint::group(addGroup(mol, 0)), Endpoint::atom(metal));
        mol.addHapticBond(Endpoint::group(addGroup(mol, 5)), Endpoint::atom(metal));
    }
};

TEST_F(IndigoCoreMoleculeComponentsTest, FerroceneIsOneComponent)
{
    Molecule mol;
    makeFerrocene(mol);

    const MoleculeComponents& components = mol.moleculeComponents();
    ASSERT_EQ(1, components.count());
    EXPECT_EQ(11, components.atomCount(0));
    EXPECT_EQ(10, components.bondCount(0)); // the ring bonds; haptic bonds are counted apart
    for (int atom : mol.vertices())
        EXPECT_EQ(0, components.componentOf(atom)) << "atom " << atom;
}

// A counter ion drawn between the atoms of the complex: it stays apart, and the
// complex, holding the lowest atom, comes first — the order the graph uses.
TEST_F(IndigoCoreMoleculeComponentsTest, CounterIonStaysApartOfTheComplex)
{
    Molecule mol;
    addRing(mol, 0);
    const int ion = mol.addAtom(ELEM_Cl);
    addRing(mol, 6);
    const int metal = mol.addAtom(ELEM_Fe);
    mol.addHapticBond(Endpoint::group(addGroup(mol, 0)), Endpoint::atom(metal));
    mol.addHapticBond(Endpoint::group(addGroup(mol, 6)), Endpoint::atom(metal));

    const MoleculeComponents& components = mol.moleculeComponents();
    ASSERT_EQ(2, components.count());
    EXPECT_EQ(0, components.componentOf(metal));
    EXPECT_EQ(0, components.componentOf(7));
    EXPECT_EQ(1, components.componentOf(ion));
    EXPECT_EQ(11, components.atomCount(0));
    EXPECT_EQ(1, components.atomCount(1));
    EXPECT_EQ(0, components.bondCount(1));
}

TEST_F(IndigoCoreMoleculeComponentsTest, AtomToAtomHapticBondJoinsItsAtoms)
{
    Molecule mol;
    mol.addAtom(ELEM_C);
    mol.addAtom(ELEM_Fe);
    mol.addHapticBond(Endpoint::atom(0), Endpoint::atom(1));

    EXPECT_EQ(1, mol.moleculeComponents().count());
}

// A variable attachment still bonds the substituent to the ring; only the position
// on the ring is left open.
TEST_F(IndigoCoreMoleculeComponentsTest, VariableAttachmentJoinsItsAtoms)
{
    Molecule mol;
    addRing(mol, 0);
    const int substituent = mol.addAtom(ELEM_Cl);
    mol.addHapticBond(Endpoint::group(addGroup(mol, 0)), Endpoint::atom(substituent), _BOND_VARIABLE_ATTACHMENT);

    EXPECT_EQ(1, mol.moleculeComponents().count());
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

    EXPECT_EQ(2, mol.moleculeComponents().count());
}

// Without haptic bonds the answer is the graph's, component by component and atom by
// atom — including a molecule whose atom indices have a hole.
TEST_F(IndigoCoreMoleculeComponentsTest, WithoutHapticBondsTheAnswerIsTheGraphs)
{
    for (const char* smiles : {"C", "CC.O.[Na+].[Cl-]", "C1CC1.C1.N.C1", "c1ccccc1CC(=O)O.[K+]"})
    {
        Molecule mol;
        loadMolecule(smiles, mol);
        mol.removeAtom(mol.vertexBegin());
        const MoleculeComponents& components = mol.moleculeComponents();

        ASSERT_EQ(mol.countComponents(), components.count()) << smiles;
        for (int atom : mol.vertices())
            EXPECT_EQ(mol.vertexComponent(atom), components.componentOf(atom)) << smiles << ", atom " << atom;
        for (int component = 0; component < components.count(); component++)
        {
            EXPECT_EQ(mol.countComponentVertices(component), components.atomCount(component)) << smiles;
            EXPECT_EQ(mol.countComponentEdges(component), components.bondCount(component)) << smiles;
        }
    }
}

TEST_F(IndigoCoreMoleculeComponentsTest, SelectedAtomsCarryTheWholeComplex)
{
    Molecule mol;
    makeFerrocene(mol);

    Filter filter;
    mol.moleculeComponents().selectAtoms(0, filter);
    Molecule component;
    component.makeSubmolecule(mol, filter, nullptr, nullptr);

    EXPECT_EQ(11, component.vertexCount());
    EXPECT_EQ(2, component.attachment_groups.groupCount());
    EXPECT_EQ(2, component.haptic_bonds.count());
}

// ---- when the answer is rebuilt ---------------------------------------------

TEST_F(IndigoCoreMoleculeComponentsTest, RemovingAGroupSplitsTheComplex)
{
    Molecule mol;
    makeFerrocene(mol);
    ASSERT_EQ(1, mol.moleculeComponents().count());

    mol.removeAttachmentGroup(mol.attachment_groups.begin()); // its haptic bond goes with it

    EXPECT_EQ(2, mol.moleculeComponents().count()); // one ring apart, the other still on the iron
}

// The graph keeps its own decomposition when a vertex is added; this answer must not.
TEST_F(IndigoCoreMoleculeComponentsTest, AnAtomAddedAfterAQueryIsSeen)
{
    Molecule mol;
    makeFerrocene(mol);
    ASSERT_EQ(1, mol.moleculeComponents().count());

    const int lone = mol.addAtom(ELEM_Na);

    EXPECT_EQ(2, mol.moleculeComponents().count());
    EXPECT_EQ(1, mol.moleculeComponents().componentOf(lone));
}

// The graph's cache may hold a decomposition made with s-group neighbours, as any
// caller of countComponents(neighbors) leaves it; the component answer must not pick
// that up.
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

    EXPECT_EQ(2, mol.moleculeComponents().count());
}

// And the other way round: SMILES, InChI and substructure search keep seeing the graph.
TEST_F(IndigoCoreMoleculeComponentsTest, TheGraphKeepsItsOwnAnswer)
{
    Molecule mol;
    makeFerrocene(mol);
    ASSERT_EQ(1, mol.moleculeComponents().count());

    EXPECT_EQ(3, mol.countComponents());
}

// ---- errors -----------------------------------------------------------------

TEST_F(IndigoCoreMoleculeComponentsTest, NoComponentForARemovedAtom)
{
    Molecule mol;
    loadMolecule("CCO", mol);
    mol.removeAtom(1);

    const MoleculeComponents& components = mol.moleculeComponents();
    EXPECT_THROW(components.componentOf(1), Exception);
    EXPECT_THROW(components.componentOf(-1), Exception);
    EXPECT_THROW(components.componentOf(3), Exception);
}

TEST_F(IndigoCoreMoleculeComponentsTest, NoComponentBeyondTheCount)
{
    Molecule mol;
    loadMolecule("C.O", mol);

    const MoleculeComponents& components = mol.moleculeComponents();
    Filter filter;
    EXPECT_THROW(components.atomCount(2), Exception);
    EXPECT_THROW(components.bondCount(-1), Exception);
    EXPECT_THROW(components.selectAtoms(2, filter), Exception);
}

TEST_F(IndigoCoreMoleculeComponentsTest, EmptyMoleculeHasNoComponents)
{
    Molecule mol;
    EXPECT_EQ(0, mol.moleculeComponents().count());
}

// ---- standardize ------------------------------------------------------------

// A ferrocene (atoms 0-10) beside a sodium and a chloride nothing bonds to: the
// fragment options must treat the complex as one fragment of eleven atoms.
class IndigoCoreStandardizeHapticTest : public IndigoCoreMoleculeComponentsTest
{
protected:
    static int standardize(bool StandardizeOptions::*option, Molecule& mol)
    {
        makeFerrocene(mol);
        mol.addAtom(ELEM_Na);
        mol.addAtom(ELEM_Cl);

        StandardizeOptions options;
        options.*option = true;
        mol.standardize(options);
        return mol.vertexCount();
    }
};

TEST_F(IndigoCoreStandardizeHapticTest, RemoveSingleAtomsKeepsTheMetalOfAComplex)
{
    Molecule mol;
    EXPECT_EQ(11, standardize(&StandardizeOptions::remove_single_atom_fragments, mol));
    EXPECT_EQ(2, mol.haptic_bonds.count());
}

TEST_F(IndigoCoreStandardizeHapticTest, KeepLargestKeepsTheWholeComplex)
{
    Molecule mol;
    EXPECT_EQ(11, standardize(&StandardizeOptions::keep_largest_fragment, mol));
    EXPECT_EQ(2, mol.haptic_bonds.count());
}

TEST_F(IndigoCoreStandardizeHapticTest, KeepSmallestDropsTheWholeComplex)
{
    Molecule mol;
    EXPECT_EQ(1, standardize(&StandardizeOptions::keep_smallest_fragment, mol));
    EXPECT_EQ(ELEM_Na, mol.getAtomNumber(mol.vertexBegin()));
}

// A chloride is a chlorine nothing holds; one on a variable attachment is a substituent.
TEST_F(IndigoCoreStandardizeHapticTest, ChargesLeaveABoundHalogenNeutral)
{
    Molecule mol;
    addRing(mol, 0);
    const int substituent = mol.addAtom(ELEM_Cl);
    mol.addHapticBond(Endpoint::group(addGroup(mol, 0)), Endpoint::atom(substituent), _BOND_VARIABLE_ATTACHMENT);
    const int lone = mol.addAtom(ELEM_Cl);

    StandardizeOptions options;
    options.standardize_charges = true;
    mol.standardize(options);

    EXPECT_EQ(0, mol.getAtomCharge(substituent));
    EXPECT_EQ(-1, mol.getAtomCharge(lone));
}

TEST_F(IndigoCoreStandardizeHapticTest, RemoveLargestDropsTheWholeComplex)
{
    Molecule mol;
    EXPECT_EQ(2, standardize(&StandardizeOptions::remove_largest_fragment, mol));
    EXPECT_TRUE(mol.haptic_bonds.isEmpty());
}
