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

#include <molecule/molecule_substructure_matcher.h>

#include "common.h"

using namespace std;
using namespace indigo;

class IndigoCoreSmartsTest : public IndigoCoreTest
{
};

TEST_F(IndigoCoreSmartsTest, PiBondedAtom)
{
    EXPECT_FALSE(substructureMatch("C", "[i]"));
    EXPECT_TRUE(substructureMatch("C=C", "[i]"));
    EXPECT_TRUE(substructureMatch("C#C", "[i]"));
    EXPECT_TRUE(substructureMatch("c1ccccc1", "[i]"));
}

TEST_F(IndigoCoreSmartsTest, NotPiBondedAtom)
{
    EXPECT_TRUE(substructureMatch("C", "[!i]"));
    EXPECT_FALSE(substructureMatch("C=C", "[!i]=[!i]"));
    EXPECT_FALSE(substructureMatch("C#C", "[!i]#[!i]"));
    EXPECT_FALSE(substructureMatch("c1ccccc1", "[!i]:[!i]"));
}

TEST_F(IndigoCoreSmartsTest, SharpX)
{
    EXPECT_FALSE(substructureMatch("[H]", "[#X]"));
    EXPECT_FALSE(substructureMatch("C", "[#X]"));
    EXPECT_TRUE(substructureMatch("O", "[#X]"));
    EXPECT_TRUE(substructureMatch("c1ccccc1O", "a[#X]"));
}

TEST_F(IndigoCoreSmartsTest, NotSharpX)
{
    EXPECT_TRUE(substructureMatch("[H]", "[!#X]"));
    EXPECT_TRUE(substructureMatch("C", "[!#X]"));
    EXPECT_FALSE(substructureMatch("O=O", "[!#X]"));
    EXPECT_FALSE(substructureMatch("c(O)1c(O)c(O)c(O)c(O)c1O", "a[!#X]"));
}

TEST_F(IndigoCoreSmartsTest, SharpN)
{
    EXPECT_FALSE(substructureMatch("[H]", "[#N]"));
    EXPECT_FALSE(substructureMatch("C", "[#N]"));
    EXPECT_TRUE(substructureMatch("O", "[#N]"));
    EXPECT_TRUE(substructureMatch("N", "[#N]"));
    EXPECT_TRUE(substructureMatch("F", "[#N]"));
    EXPECT_TRUE(substructureMatch("Cl", "[#N]"));
    EXPECT_TRUE(substructureMatch("Br", "[#N]"));
    EXPECT_FALSE(substructureMatch("I", "[#N]"));
}

TEST_F(IndigoCoreSmartsTest, NotSharpN)
{
    EXPECT_TRUE(substructureMatch("[H]", "[!#N]"));
    EXPECT_TRUE(substructureMatch("C", "[!#N]"));
    EXPECT_FALSE(substructureMatch("O=O", "[!#N]"));
    EXPECT_FALSE(substructureMatch("N#N", "[!#N]"));
    EXPECT_FALSE(substructureMatch("ClCl", "[!#N]"));
    EXPECT_FALSE(substructureMatch("BrBr", "[!#N]"));
}

TEST_F(IndigoCoreSmartsTest, q)
{
    EXPECT_FALSE(substructureMatch("[H]", "[q3]"));
    EXPECT_TRUE(substructureMatch("C1C2CCCC3CCCC(CC1)C23", "[q3]"));
}

TEST_F(IndigoCoreSmartsTest, SharpG)
{
    EXPECT_TRUE(substructureMatch("[H]", "[#G1]"));
    EXPECT_FALSE(substructureMatch("[H]", "[#G4]"));
    EXPECT_FALSE(substructureMatch("C=C", "[#G1]=[#G1]"));
    EXPECT_TRUE(substructureMatch("C=C", "[#G4]=[#G4]"));
}

TEST_F(IndigoCoreSmartsTest, NotSharpG)
{
    EXPECT_FALSE(substructureMatch("[H]", "[!#G1]"));
    EXPECT_TRUE(substructureMatch("[H]", "[!#G4]"));
    EXPECT_TRUE(substructureMatch("C=C", "[!#G1]=[!#G1]"));
    EXPECT_FALSE(substructureMatch("C=C", "[!#G4]=[!#G4]"));
}

TEST_F(IndigoCoreSmartsTest, h)
{
    EXPECT_TRUE(substructureMatch("C", "[Ch]"));
    EXPECT_TRUE(substructureMatch("C", "[Ch4]"));
    EXPECT_FALSE(substructureMatch("C", "[Ch1]"));
    EXPECT_FALSE(substructureMatch("C=C", "[Ch3]"));
    EXPECT_FALSE(substructureMatch("C", "[*h5]"));
    EXPECT_FALSE(substructureMatch("C([H])([H])([H])[H]", "[Ch]"));

    EXPECT_STREQ(smilesLoadSaveLoad("[#6;h]", true).c_str(), "[#6;h]");
    EXPECT_STREQ(smilesLoadSaveLoad("[#6;h2]", true).c_str(), "[#6;h2]");
}

TEST_F(IndigoCoreSmartsTest, connectivity)
{
    EXPECT_STREQ(smilesLoadSaveLoad("[#7;X3]", true).c_str(), "[#7;X3]");
    EXPECT_STREQ(smilesLoadSaveLoad("[#7X3]", true).c_str(), "[#7;X3]");
    EXPECT_STREQ(smilesLoadSaveLoad("[NX3]", true).c_str(), "[#7;X3]");
}

TEST_F(IndigoCoreSmartsTest, aliases)
{
    EXPECT_STREQ(smilesLoadSaveLoad("C", true).c_str(), "[#6]");
    EXPECT_TRUE(substructureMatch("C", "C"));

    EXPECT_STREQ(smilesLoadSaveLoad("C |$Carbon$|", true).c_str(), "[#6] |$Carbon$|");
    EXPECT_TRUE(substructureMatch("C", "C |$Carbon$|"));
    EXPECT_FALSE(substructureMatch("N", "C |$Carbon$|"));

    EXPECT_STREQ(smilesLoadSaveLoad("CC |$Carbon;$|", true).c_str(), "[#6]-[#6] |$Carbon;$|");

    EXPECT_STREQ(smilesLoadSaveLoad("* |$Pseudo$|", true).c_str(), "[*] |$Pseudo$|");
    EXPECT_TRUE(substructureMatch("* |$Pseudo$|", "* |$Pseudo$|"));
}


TEST_F(IndigoCoreSmartsTest, smiles)
{
    EXPECT_STREQ(smilesLoadSaveLoad("C", false).c_str(), "C");
}
