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

// indigoCheckSalt and indigoStripSalt (#3927): what they return, what they change and
// what they refuse. Which structures are salts is listed in the integration test
// basic/salts.py, which runs through every wrapper.

#include <gtest/gtest.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;

class IndigoApiSaltsTest : public IndigoApiTest
{
protected:
    static constexpr int RING_SIZE = 5;

    // Joins the ring that starts at atom `first` to the metal by a haptic bond.
    static void bindRing(int mol, int first, int metal)
    {
        int ring[RING_SIZE];
        for (int i = 0; i < RING_SIZE; i++)
            ring[i] = first + i;
        indigoAddHapticBond(mol, indigoAddAttachmentGroup(mol, RING_SIZE, ring), indigoGetAtom(mol, metal));
    }
};

TEST_F(IndigoApiSaltsTest, check_tells_whether_there_is_a_salt)
{
    EXPECT_EQ(1, indigoCheckSalt(indigoLoadMoleculeFromString("CCO.[Na+]")));
    EXPECT_EQ(0, indigoCheckSalt(indigoLoadMoleculeFromString("CCO")));
}

TEST_F(IndigoApiSaltsTest, strip_removes_the_salts_from_the_molecule_itself)
{
    // The sodium stands between the two carbons of an ethane in the numbering.
    const int mol = indigoLoadMoleculeFromString("C1.[Na+].C1");

    EXPECT_EQ(1, indigoStripSalt(mol));
    EXPECT_STREQ("CC", indigoSmiles(mol));
    EXPECT_EQ(0, indigoStripSalt(mol)) << "nothing is left to remove";
    EXPECT_STREQ("CC", indigoSmiles(mol));
}

TEST_F(IndigoApiSaltsTest, a_metal_held_by_haptic_bonds_is_not_a_lone_ion)
{
    const int rings = 2;
    const int iron = rings * RING_SIZE;
    const int ferrocene = indigoLoadMoleculeFromString("[cH-]1cccc1.[cH-]1cccc1.[Fe+2]");
    bindRing(ferrocene, 0, iron);
    bindRing(ferrocene, RING_SIZE, iron);

    EXPECT_EQ(0, indigoCheckSalt(ferrocene));
    EXPECT_EQ(0, indigoStripSalt(ferrocene));
    EXPECT_EQ(rings * RING_SIZE + 1, indigoCountAtoms(ferrocene));
    EXPECT_EQ(rings, indigoCountHapticBonds(ferrocene));
}

// Zeise's salt. By its ordinary bonds alone the platinum with three chlorides is a
// small inorganic ion; the haptic bond to the ethylene makes it a part of the complex.
TEST_F(IndigoApiSaltsTest, a_counter_ion_goes_and_the_complex_stays)
{
    const int complex_atoms = 6; // the platinum, three chlorides and the ethylene
    const int zeise = indigoLoadMoleculeFromString("Cl[Pt-](Cl)Cl.C=C.[K+]");
    int ethylene[] = {4, 5};
    indigoAddHapticBond(zeise, indigoAddAttachmentGroup(zeise, 2, ethylene), indigoGetAtom(zeise, 1));

    EXPECT_EQ(1, indigoCheckSalt(zeise)) << "the potassium";
    EXPECT_EQ(1, indigoStripSalt(zeise));
    EXPECT_EQ(complex_atoms, indigoCountAtoms(zeise));
    EXPECT_EQ(1, indigoCountHapticBonds(zeise));
    EXPECT_EQ(0, indigoCheckSalt(zeise));
}

TEST_F(IndigoApiSaltsTest, a_query_molecule_is_refused)
{
    indigoSetErrorHandler(nullptr, nullptr);
    const int query = indigoLoadQueryMoleculeFromString("CCO.[Na+]");

    EXPECT_EQ(-1, indigoCheckSalt(query));
    EXPECT_STREQ("core: <query molecule> is not a molecule", indigoGetLastError());
    EXPECT_EQ(-1, indigoStripSalt(query));
    EXPECT_STREQ("core: <query molecule> is not a molecule", indigoGetLastError());
}
