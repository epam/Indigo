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

// The component calls of the C API on a haptic complex (#3927). Every call that
// takes a component index has to read the same decomposition, or an index from one
// would pick atoms by another; so they are all checked on one structure.

#include <string>

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
    // The structure attached to #3927: a ferrocene whose two rings reach the iron
    // through ENDPTS records only. Loading absorbs the two star atoms, leaving eleven
    // atoms with a gap in their indices.
    static constexpr const char* FERROCENE = "\n"
                                             "  ACCLDraw09272617562D\n"
                                             "\n"
                                             "  0  0  0     0  0            999 V3000\n"
                                             "M  V30 BEGIN CTAB\n"
                                             "M  V30 COUNTS 13 12 0 0 0\n"
                                             "M  V30 BEGIN ATOM\n"
                                             "M  V30 1 C 3.1753 -2.1774 0 0 RAD=2 CFG=3\n"
                                             "M  V30 2 C 4.1303 -2.8712 0 0\n"
                                             "M  V30 3 C 3.7656 -3.9941 0 0\n"
                                             "M  V30 4 C 2.585 -3.9941 0 0\n"
                                             "M  V30 5 C 2.2203 -2.8712 0 0\n"
                                             "M  V30 6 Fe 2.9045 -6.4941 0 0\n"
                                             "M  V30 7 * 2.9045 -3.0566 0 0\n"
                                             "M  V30 8 * 2.9045 -9.9733 0 0\n"
                                             "M  V30 9 C 1.8453 -9.6212 0 0\n"
                                             "M  V30 10 C 2.8003 -8.9274 0 0 RAD=2 CFG=3\n"
                                             "M  V30 11 C 3.7553 -9.6212 0 0\n"
                                             "M  V30 12 C 3.3906 -10.7441 0 0\n"
                                             "M  V30 13 C 2.21 -10.7441 0 0\n"
                                             "M  V30 END ATOM\n"
                                             "M  V30 BEGIN BOND\n"
                                             "M  V30 1 1 2 1\n"
                                             "M  V30 2 2 3 2\n"
                                             "M  V30 3 1 4 3\n"
                                             "M  V30 4 2 5 4\n"
                                             "M  V30 5 1 1 5\n"
                                             "M  V30 6 9 6 7 ENDPTS=(5 4 5 1 2 3) ATTACH=ALL\n"
                                             "M  V30 7 9 6 8 ENDPTS=(5 9 10 11 12 13) ATTACH=ALL\n"
                                             "M  V30 8 1 10 9\n"
                                             "M  V30 9 1 11 10\n"
                                             "M  V30 10 2 12 11\n"
                                             "M  V30 11 1 13 12\n"
                                             "M  V30 12 2 9 13\n"
                                             "M  V30 END BOND\n"
                                             "M  V30 END CTAB\n"
                                             "M  END\n";

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

    static int countOccurrences(const std::string& text, const std::string& what)
    {
        int count = 0;
        for (size_t at = text.find(what); at != std::string::npos; at = text.find(what, at + what.size()))
            count++;
        return count;
    }
};

TEST_F(IndigoApiHapticComponentsTest, ferrocene_is_one_component)
{
    const int mol = indigoLoadMoleculeFromString(FERROCENE);
    ASSERT_EQ(11, indigoCountAtoms(mol));
    ASSERT_EQ(2, indigoCountHapticBonds(mol));

    ASSERT_EQ(1, indigoCountComponents(mol));

    const int component = indigoComponent(mol, 0);
    EXPECT_EQ(11, indigoCountAtoms(component));
    EXPECT_EQ(10, indigoCountBonds(component)); // the ring bonds; the haptic ones are not bonds of the graph
    EXPECT_EQ(11, countItems(indigoIterateAtoms(component)));
    EXPECT_EQ(10, countItems(indigoIterateBonds(component)));

    EXPECT_EQ(11, indigoCountComponentAtoms(mol, 0));
    EXPECT_EQ(10, indigoCountComponentBonds(mol, 0));
    EXPECT_EQ(11, countItems(indigoIterateComponentAtoms(mol, 0)));
    EXPECT_EQ(10, countItems(indigoIterateComponentBonds(mol, 0)));

    const int atoms = indigoIterateAtoms(mol);
    while (indigoHasNext(atoms))
    {
        const int atom = indigoNext(atoms);
        EXPECT_EQ(0, indigoComponentIndex(atom)) << "atom " << indigoIndex(atom);
        indigoFree(atom);
    }
    indigoFree(atoms);
}

// A clone of the component is the whole complex, haptic bonds and groups included.
TEST_F(IndigoApiHapticComponentsTest, component_clone_keeps_the_complex)
{
    const int mol = indigoLoadMoleculeFromString(FERROCENE);

    for (const int clone : {indigoClone(indigoComponent(mol, 0)), indigoCloneComponent(mol, 0)})
    {
        EXPECT_EQ(11, indigoCountAtoms(clone));
        EXPECT_EQ(2, indigoCountAttachmentGroups(clone));
        EXPECT_EQ(2, indigoCountHapticBonds(clone));
    }
}

TEST_F(IndigoApiHapticComponentsTest, fragmented_sdf_writes_the_complex_as_one_record)
{
    const int mol = indigoLoadMoleculeFromString(FERROCENE);

    EXPECT_EQ(1, countOccurrences(indigoFragmentedSdf(mol), "$$$$"));
}

// What the ticket asks for: a counter ion that no bond holds is a component of its
// own, next to the complex, however the atoms are numbered.
TEST_F(IndigoApiHapticComponentsTest, counter_ion_is_a_component_of_its_own)
{
    const int mol = indigoLoadMoleculeFromString(FERROCENE);
    indigoMerge(mol, indigoLoadMoleculeFromString("[Na+].[Cl-]"));

    ASSERT_EQ(3, indigoCountComponents(mol));
    EXPECT_EQ(11, indigoCountAtoms(indigoComponent(mol, 0)));
    EXPECT_EQ(1, indigoCountAtoms(indigoComponent(mol, 1)));
    EXPECT_EQ(1, indigoCountAtoms(indigoComponent(mol, 2)));
    EXPECT_EQ(3, countOccurrences(indigoFragmentedSdf(mol), "$$$$"));
}

TEST_F(IndigoApiHapticComponentsTest, bad_component_index_is_reported)
{
    indigoSetErrorHandler(nullptr, nullptr);
    const int mol = indigoLoadMoleculeFromString(FERROCENE);

    EXPECT_EQ(-1, indigoComponent(mol, 1));
    EXPECT_STREQ("core: indigoComponent(): bad index 1 (0-0 allowed)", indigoGetLastError());

    EXPECT_EQ(-1, indigoCountComponentAtoms(mol, 1));
    EXPECT_NE(std::string::npos, std::string(indigoGetLastError()).find("component 1 does not exist")) << indigoGetLastError();
}
