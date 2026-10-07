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

// The component calls of the C API on a haptic complex (#3927). A component number
// taken from one call is given to another, so every call is asked about the same
// component and has to give the same answer.

#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;

// Exported, though no header declares them.
CEXPORT int indigoCloneComponent(int molecule, int index);
CEXPORT int indigoCountComponentAtoms(int molecule, int index);
CEXPORT int indigoCountComponentBonds(int molecule, int index);
CEXPORT int indigoIterateComponentAtoms(int molecule, int index);
CEXPORT int indigoIterateComponentBonds(int molecule, int index);

class IndigoApiHapticComponentsTest : public IndigoApiTest
{
protected:
    static constexpr int RING_SIZE = 5;
    static constexpr int FERROCENE_RINGS = 2;                               // each an attachment group with a haptic bond to the iron
    static constexpr int FERROCENE_ATOMS = FERROCENE_RINGS * RING_SIZE + 1; // the rings and the iron
    static constexpr int FERROCENE_BONDS = FERROCENE_RINGS * RING_SIZE;     // the ring bonds: a haptic bond is not among the bonds

    // The structure attached to the ticket: a ferrocene whose rings reach the iron
    // through ENDPTS records only. Loading absorbs the two star atoms.
    static constexpr const char* FERROCENE = R"(
  ACCLDraw09272617562D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 13 12 0 0 0
M  V30 BEGIN ATOM
M  V30 1 C 3.1753 -2.1774 0 0 RAD=2 CFG=3
M  V30 2 C 4.1303 -2.8712 0 0
M  V30 3 C 3.7656 -3.9941 0 0
M  V30 4 C 2.585 -3.9941 0 0
M  V30 5 C 2.2203 -2.8712 0 0
M  V30 6 Fe 2.9045 -6.4941 0 0
M  V30 7 * 2.9045 -3.0566 0 0
M  V30 8 * 2.9045 -9.9733 0 0
M  V30 9 C 1.8453 -9.6212 0 0
M  V30 10 C 2.8003 -8.9274 0 0 RAD=2 CFG=3
M  V30 11 C 3.7553 -9.6212 0 0
M  V30 12 C 3.3906 -10.7441 0 0
M  V30 13 C 2.21 -10.7441 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 2 1
M  V30 2 2 3 2
M  V30 3 1 4 3
M  V30 4 2 5 4
M  V30 5 1 1 5
M  V30 6 9 6 7 ENDPTS=(5 4 5 1 2 3) ATTACH=ALL
M  V30 7 9 6 8 ENDPTS=(5 9 10 11 12 13) ATTACH=ALL
M  V30 8 1 10 9
M  V30 9 1 11 10
M  V30 10 2 12 11
M  V30 11 1 13 12
M  V30 12 2 9 13
M  V30 END BOND
M  V30 END CTAB
M  END
)";

    static int loadFerroceneWithIons()
    {
        const int mol = indigoLoadMoleculeFromString(FERROCENE);
        indigoMerge(mol, indigoLoadMoleculeFromString("[Na+].[Cl-]"));
        return mol;
    }

    static int countItems(int iterator)
    {
        int count = 0;
        while (indigoHasNext(iterator))
        {
            indigoFree(indigoNext(iterator));
            count++;
        }
        indigoFree(iterator);
        return count;
    }

    // Asks every call that takes a component number about component `index`.
    static void expectComponent(int mol, int index, int atom_count, int bond_count)
    {
        SCOPED_TRACE("component " + std::to_string(index));

        const int component = indigoComponent(mol, index);
        EXPECT_EQ(index, indigoIndex(component));
        EXPECT_EQ(atom_count, indigoCountAtoms(component));
        EXPECT_EQ(bond_count, indigoCountBonds(component));
        EXPECT_EQ(bond_count, countItems(indigoIterateBonds(component)));

        EXPECT_EQ(atom_count, indigoCountComponentAtoms(mol, index));
        EXPECT_EQ(bond_count, indigoCountComponentBonds(mol, index));
        EXPECT_EQ(atom_count, countItems(indigoIterateComponentAtoms(mol, index)));
        EXPECT_EQ(bond_count, countItems(indigoIterateComponentBonds(mol, index)));

        EXPECT_EQ(atom_count, indigoCountAtoms(indigoClone(component)));
        EXPECT_EQ(atom_count, indigoCountAtoms(indigoCloneComponent(mol, index)));

        int atoms_seen = 0;
        const int atoms = indigoIterateAtoms(component);
        while (indigoHasNext(atoms))
        {
            const int atom = indigoNext(atoms);
            EXPECT_EQ(index, indigoComponentIndex(atom)) << "atom " << indigoIndex(atom);
            indigoFree(atom);
            atoms_seen++;
        }
        indigoFree(atoms);
        EXPECT_EQ(atom_count, atoms_seen);
    }

    static std::vector<std::string> sdfRecords(const std::string& sdf)
    {
        static const std::string terminator = "$$$$\n";
        std::vector<std::string> records;
        size_t begin = 0;
        for (size_t end = sdf.find(terminator); end != std::string::npos; end = sdf.find(terminator, begin))
        {
            records.push_back(sdf.substr(begin, end - begin));
            begin = end + terminator.size();
        }
        return records;
    }
};

TEST_F(IndigoApiHapticComponentsTest, ferrocene_is_one_component)
{
    const int mol = indigoLoadMoleculeFromString(FERROCENE);
    ASSERT_EQ(FERROCENE_ATOMS, indigoCountAtoms(mol));
    ASSERT_EQ(FERROCENE_RINGS, indigoCountHapticBonds(mol));

    ASSERT_EQ(1, indigoCountComponents(mol));
    expectComponent(mol, 0, FERROCENE_ATOMS, FERROCENE_BONDS);
}

// What the ticket asks for: a counter ion that no bond holds is told apart from the
// parts of the complex, which haptic bonds hold.
TEST_F(IndigoApiHapticComponentsTest, counter_ions_are_components_of_their_own)
{
    const int mol = loadFerroceneWithIons();

    ASSERT_EQ(3, indigoCountComponents(mol));
    expectComponent(mol, 0, FERROCENE_ATOMS, FERROCENE_BONDS);
    expectComponent(mol, 1, 1, 0);
    expectComponent(mol, 2, 1, 0);

    std::vector<int> atom_counts;
    const int components = indigoIterateComponents(mol);
    while (indigoHasNext(components))
    {
        const int component = indigoNext(components);
        EXPECT_EQ(static_cast<int>(atom_counts.size()), indigoIndex(component));
        atom_counts.push_back(indigoCountAtoms(component));
    }
    EXPECT_EQ(std::vector<int>({FERROCENE_ATOMS, 1, 1}), atom_counts);
}

// A clone of the component is the whole complex, haptic bonds and groups included.
TEST_F(IndigoApiHapticComponentsTest, component_clone_keeps_the_complex)
{
    const int mol = indigoLoadMoleculeFromString(FERROCENE);

    for (const int clone : {indigoClone(indigoComponent(mol, 0)), indigoCloneComponent(mol, 0)})
    {
        EXPECT_EQ(FERROCENE_ATOMS, indigoCountAtoms(clone));
        EXPECT_EQ(FERROCENE_RINGS, indigoCountAttachmentGroups(clone));
        EXPECT_EQ(FERROCENE_RINGS, indigoCountHapticBonds(clone));
    }
}

// A query takes the same path; its component clones into a query that keeps the complex.
TEST_F(IndigoApiHapticComponentsTest, query_ferrocene_is_one_component)
{
    const int query = indigoLoadQueryMoleculeFromString(FERROCENE);
    ASSERT_EQ(1, indigoCountComponents(query));

    const int clone = indigoClone(indigoComponent(query, 0));
    EXPECT_EQ(FERROCENE_ATOMS, indigoCountAtoms(clone));
    EXPECT_EQ(FERROCENE_RINGS, indigoCountHapticBonds(clone));
}

TEST_F(IndigoApiHapticComponentsTest, fragmented_sdf_writes_a_record_per_component)
{
    indigoSetOption("molfile-saving-mode", "3000"); // V2000 has no form for a haptic bond
    const int mol = loadFerroceneWithIons();

    const std::vector<std::string> records = sdfRecords(indigoFragmentedSdf(mol));

    ASSERT_EQ(3u, records.size());
    const int complex = indigoLoadMoleculeFromString(records[0].c_str());
    EXPECT_EQ(FERROCENE_ATOMS, indigoCountAtoms(complex));
    EXPECT_EQ(FERROCENE_RINGS, indigoCountHapticBonds(complex));
    EXPECT_EQ(1, indigoCountAtoms(indigoLoadMoleculeFromString(records[1].c_str())));
    EXPECT_EQ(1, indigoCountAtoms(indigoLoadMoleculeFromString(records[2].c_str())));
}

TEST_F(IndigoApiHapticComponentsTest, component_number_beyond_the_count_is_reported)
{
    indigoSetErrorHandler(nullptr, nullptr);
    const int mol = indigoLoadMoleculeFromString(FERROCENE);
    const int beyond = 1; // the ferrocene is component 0 and there is no other

    EXPECT_EQ(-1, indigoComponent(mol, beyond));
    EXPECT_STREQ("core: indigoComponent(): bad index 1 (0-0 allowed)", indigoGetLastError());
    EXPECT_EQ(-1, indigoCloneComponent(mol, beyond));
    EXPECT_STREQ("core: indigoCloneComponent(): bad index 1 (0-0 allowed)", indigoGetLastError());
    EXPECT_EQ(-1, indigoIterateComponentAtoms(mol, beyond));
    EXPECT_STREQ("core: 1 is not a valid component number (0-0 allowed)", indigoGetLastError());
    EXPECT_EQ(-1, indigoIterateComponentBonds(mol, beyond));
    EXPECT_STREQ("core: 1 is not a valid component number (0-0 allowed)", indigoGetLastError());
    EXPECT_EQ(-1, indigoCountComponentAtoms(mol, beyond));
    EXPECT_STREQ("core: 1 is not a valid component number (0-0 allowed)", indigoGetLastError());
    EXPECT_EQ(-1, indigoCountComponentBonds(mol, beyond));
    EXPECT_STREQ("core: 1 is not a valid component number (0-0 allowed)", indigoGetLastError());
}

TEST_F(IndigoApiHapticComponentsTest, removed_atom_has_no_component)
{
    indigoSetErrorHandler(nullptr, nullptr);
    const int mol = indigoLoadMoleculeFromString("CC.O");
    int oxygen_index = 2;
    const int oxygen = indigoGetAtom(mol, oxygen_index);
    ASSERT_EQ(1, indigoComponentIndex(oxygen));

    ASSERT_EQ(1, indigoRemoveAtoms(mol, 1, &oxygen_index));

    EXPECT_EQ(-1, indigoComponentIndex(oxygen));
    EXPECT_STREQ("core: indigoComponentIndex(): atom 2 is not in the molecule", indigoGetLastError());
}
