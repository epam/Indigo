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

#include <gtest/gtest.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;

class IndigoApiAbbreviationsTest : public IndigoApiTest
{
};

namespace
{
    const char* kCustomAbbreviationXml = "<abbreviations><item name=\"Zzz\" expansion=\"[*]CCCC\"/></abbreviations>";

    int makeMoleculeWithPseudoatom(const char* label)
    {
        int mol = indigoCreateMolecule();
        int c = indigoAddAtom(mol, "C");
        int pseudo = indigoAddAtom(mol, label);
        indigoAddBond(c, pseudo, 1);
        return mol;
    }
}

TEST_F(IndigoApiAbbreviationsTest, custom_abbreviation_expands_after_load)
{
    ASSERT_EQ(1, indigoLoadAbbreviationsFromString(kCustomAbbreviationXml));

    int mol = makeMoleculeWithPseudoatom("Zzz");
    ASSERT_EQ(1, indigoExpandAbbreviations(mol));
    // Zzz expands to a 4-carbon chain attached to the existing atom, 5 carbons total.
    ASSERT_EQ(5, indigoCountAtoms(mol));
}

TEST_F(IndigoApiAbbreviationsTest, unknown_abbreviation_is_left_unexpanded)
{
    int mol = makeMoleculeWithPseudoatom("Zzz");
    // Without a custom load, Zzz is not a known abbreviation, so nothing is expanded.
    ASSERT_EQ(0, indigoExpandAbbreviations(mol));
    ASSERT_EQ(2, indigoCountAtoms(mol));
}

TEST_F(IndigoApiAbbreviationsTest, builtin_abbreviation_still_expands_after_custom_load)
{
    ASSERT_EQ(1, indigoLoadAbbreviationsFromString(kCustomAbbreviationXml));

    int mol = makeMoleculeWithPseudoatom("Ph");
    ASSERT_EQ(1, indigoExpandAbbreviations(mol));
    // Ph (phenyl) expands to a benzene ring attached to the existing atom, 7 carbons total.
    ASSERT_EQ(7, indigoCountAtoms(mol));
}

TEST_F(IndigoApiAbbreviationsTest, reset_drops_custom_abbreviation)
{
    ASSERT_EQ(1, indigoLoadAbbreviationsFromString(kCustomAbbreviationXml));
    ASSERT_EQ(1, indigoResetAbbreviations());

    int mol = makeMoleculeWithPseudoatom("Zzz");
    ASSERT_EQ(0, indigoExpandAbbreviations(mol));
    ASSERT_EQ(2, indigoCountAtoms(mol));
}

TEST_F(IndigoApiAbbreviationsTest, accumulates_across_multiple_loads)
{
    ASSERT_EQ(1, indigoLoadAbbreviationsFromString(kCustomAbbreviationXml));
    ASSERT_EQ(1, indigoLoadAbbreviationsFromString("<abbreviations><item name=\"Www\" expansion=\"[*]CC\"/></abbreviations>"));

    int mol1 = makeMoleculeWithPseudoatom("Zzz");
    ASSERT_EQ(1, indigoExpandAbbreviations(mol1));
    ASSERT_EQ(5, indigoCountAtoms(mol1));

    int mol2 = makeMoleculeWithPseudoatom("Www");
    ASSERT_EQ(1, indigoExpandAbbreviations(mol2));
    ASSERT_EQ(3, indigoCountAtoms(mol2));
}
