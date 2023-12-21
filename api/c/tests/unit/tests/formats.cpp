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

#include <molecule/molecule_mass.h>

#include <indigo.h>

#include "common.h"

using namespace std;
using namespace indigo;

class IndigoApiFormatsTest : public IndigoApiTest
{
};

TEST_F(IndigoApiFormatsTest, molecule)
{
    try
    {
        string mStr = "c1ccccc1N";
        int obj = indigoLoadStructureFromString(mStr.c_str(), "");
        // EXPECT_STREQ("c1c(N)cccc1", indigoSmiles(obj));
        EXPECT_STREQ("Nc1ccccc1", indigoCanonicalSmiles(obj));

        obj = indigoLoadStructureFromString(mStr.c_str(), "query");
        EXPECT_STREQ("C1:C(N):C:C:C:C:1", indigoSmiles(obj));
        EXPECT_EQ(7, indigoCountAtoms(obj));
        EXPECT_EQ(7, indigoCountBonds(obj));

        // 1
        obj = indigoLoadStructureFromString("", "");
        EXPECT_STREQ("", indigoCanonicalSmiles(obj));

        mStr = "C1=C(*)C=CC=C1";
        string expected = "C1C=CC=CC=1* |$;;;;;;A$|";

        // 2
        obj = indigoLoadStructureFromString(mStr.c_str(), "");
        EXPECT_STREQ(expected.c_str(), indigoSmiles(obj));
        EXPECT_STREQ("*C1C=CC=CC=1", indigoCanonicalSmiles(obj));
        EXPECT_EQ(7, indigoCountAtoms(obj));
        EXPECT_EQ(7, indigoCountBonds(obj));

        // 3
        const string expectedSmarts = "C1-C=C-C=C-C=1-[*]";
        obj = indigoLoadStructureFromString(mStr.c_str(), "smarts");
        EXPECT_STREQ(expectedSmarts.c_str(), indigoSmarts(obj));
        EXPECT_EQ(7, indigoCountAtoms(obj));
        EXPECT_EQ(7, indigoCountBonds(obj));

        // 4
        const string expectedQuery = "[#6]1-[#6]=[#6]-[#6]=[#6]-[#6]=1-[*]";
        obj = indigoLoadStructureFromString(mStr.c_str(), "query");
        EXPECT_STREQ(expectedQuery.c_str(), indigoSmarts(obj));
        EXPECT_EQ(7, indigoCountAtoms(obj));
        EXPECT_EQ(7, indigoCountBonds(obj));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiFormatsTest, reaction)
{
    const string react = "C1=C(*)C=CC=C1>>C1=CC=CC(*)=C1";
    const string expected = "C1C=CC=CC=1*>>C1C=C(*)C=CC=1";
    const string expected_smarts = "C1-C=C-C=C-C=1-[*]>>C1-C=C(-[*])-C=C-C=1";

    try
    {
        int obj = -1;
        obj = indigoLoadStructureFromString(react.c_str(), "smarts");
        EXPECT_STREQ(expected_smarts.c_str(), indigoSmarts(obj));
        EXPECT_EQ(1, indigoCountReactants(obj));
        EXPECT_EQ(1, indigoCountProducts(obj));
        EXPECT_EQ(2, indigoCountMolecules(obj));

        obj = indigoLoadStructureFromString(react.c_str(), "query");
        EXPECT_STREQ(expected.c_str(), indigoSmiles(obj));
        EXPECT_STREQ(expected.c_str(), indigoCanonicalSmiles(obj));
        EXPECT_EQ(1, indigoCountReactants(obj));
        EXPECT_EQ(1, indigoCountProducts(obj));
        EXPECT_EQ(2, indigoCountMolecules(obj));

        //        obj = indigoLoadStructureFromString(react.c_str(), "");
        //        EXPECT_STREQ(expected.c_str(), indigoSmiles(obj));
        //        EXPECT_EQ(1, indigoCountReactants(obj));
        //        EXPECT_EQ(1, indigoCountProducts(obj));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiFormatsTest, smarts)
{
    const string mStr = "[OH]c1ccccc1";

    try
    {
        int obj = -1;
        obj = indigoLoadStructureFromString(mStr.c_str(), "");
        EXPECT_EQ(7, indigoCountAtoms(obj));
        EXPECT_EQ(7, indigoCountBonds(obj));

        obj = indigoLoadStructureFromString(mStr.c_str(), "smarts");
        EXPECT_EQ(7, indigoCountAtoms(obj));
        EXPECT_EQ(7, indigoCountBonds(obj));
        //        EXPECT_STREQ(expectedSmarts.c_str(), indigoSmarts(obj));

        obj = indigoLoadStructureFromString(mStr.c_str(), "query");
        EXPECT_EQ(7, indigoCountAtoms(obj));
        EXPECT_EQ(7, indigoCountBonds(obj));
        //        EXPECT_STREQ(expectedQuery.c_str(), indigoSmarts(obj));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiFormatsTest, query)
{
    const string mStr = "c1[nH]c2c(c(N)[n+]([O-])c[n]2)[n]1";

    try
    {
        int obj = -1;
        obj = indigoLoadStructureFromString(mStr.c_str(), "query");
        EXPECT_EQ(1, indigoCheckQuery(obj));
        EXPECT_EQ(11, indigoCountAtoms(obj));
        EXPECT_EQ(12, indigoCountBonds(obj));

        obj = indigoLoadStructureFromString(mStr.c_str(), "smarts query");
        EXPECT_EQ(1, indigoCheckQuery(obj));
        EXPECT_EQ(11, indigoCountAtoms(obj));
        EXPECT_EQ(12, indigoCountBonds(obj));

        obj = indigoLoadStructureFromString(mStr.c_str(), "");
        EXPECT_EQ(11, indigoCountAtoms(obj));
        EXPECT_EQ(12, indigoCountBonds(obj));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

// TODO: #417
TEST_F(IndigoApiFormatsTest, loadAssert)
{
    const auto* mStr = "C1=C(*)C=?C=C1";
    const auto* expectedError = "molecule auto loader: SMILES loader: Character #63 is unexpected during bond parsing";
    indigoSetErrorHandler(nullptr, nullptr);
    const auto result = indigoLoadStructureFromString(mStr, "");
    ASSERT_EQ(result, -1);
    ASSERT_STREQ(expectedError, indigoGetLastError());
}

TEST_F(IndigoApiFormatsTest, fromBuffer)
{
    const uint8_t mStr[] = "[CX4H3][#6]";
    const int buffSize = sizeof(mStr);

    try
    {
        int obj = -1;
        obj = indigoLoadStructureFromBuffer(mStr, sizeof(mStr), "query");
        // EXPECT_EQ(1, indigoCheckQuery(obj));
        EXPECT_EQ(2, indigoCountAtoms(obj));
        EXPECT_EQ(1, indigoCountBonds(obj));

        const uint8_t react2[] = "C1CC[NH:2]CC1.C1CC[S:1]CC1>>C1CC2CC[S:2]CC2C[NH:1]1";
        obj = indigoLoadStructureFromBuffer(react2, sizeof(react2), "");
        EXPECT_EQ(3, indigoCountMolecules(obj));

        const uint8_t react[] = "C[12CH2:1]C(CCCC)[CH]CCCCCCC>>C[13CH2:1]C(CCCC)[C]CCCCCCCC |^1:7,^4:22|";
        obj = indigoLoadStructureFromBuffer(react, sizeof(react), "query");
        EXPECT_EQ(2, indigoCountMolecules(obj));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiFormatsTest, fromFile)
{
    try
    {
        int obj = -1;
        obj = indigoLoadStructureFromFile(dataPath("molecules/affine/2-bromobenzenethiol-rot.mol").c_str(), "query");
        // EXPECT_EQ(1, indigoCheckQuery(obj));
        EXPECT_EQ(8, indigoCountAtoms(obj));
        EXPECT_EQ(8, indigoCountBonds(obj));

        obj = indigoLoadStructureFromFile(dataPath("reactions/other/epoxy.rxn").c_str(), "query");
        // EXPECT_EQ(1, indigoCheckQuery(obj));
        EXPECT_EQ(4, indigoCountMolecules(obj));
        EXPECT_EQ(1, indigoCountReactants(obj));
        EXPECT_EQ(1, indigoCountProducts(obj));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiFormatsTest, cdx)
{
    try
    {
        int obj = -1;
        obj = indigoLoadMoleculeFromFile(dataPath("molecules/stereo/enhanced_stereo1.mol").c_str());
        const char* ptr = indigoCdxBase64(obj);
        indigoLoadMoleculeFromString(ptr);
        obj = indigoLoadMoleculeFromFile(dataPath("molecules/stereo/enhanced_stereo2.mol").c_str());
        ptr = indigoCdxBase64(obj);
        indigoLoadMoleculeFromString(ptr);
        obj = indigoLoadMoleculeFromFile(dataPath("molecules/stereo/enhanced_stereo3.mol").c_str());
        ptr = indigoCdxBase64(obj);
        indigoLoadMoleculeFromString(ptr);
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiFormatsTest, fromGzFile)
{
    try
    {
        int obj = indigoLoadStructureFromFile(dataPath("molecules/basic/benzene.mol.gz").c_str(), "query");
        // EXPECT_EQ(1, indigoCheckQuery(obj));
        EXPECT_EQ(6, indigoCountAtoms(obj));
        EXPECT_EQ(6, indigoCountBonds(obj));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }

    try
    {
        int obj = indigoLoadStructureFromFile(dataPath("molecules/basic/Compound_0000001_0000250.sdf.gz").c_str(), "query");
        EXPECT_EQ(8454, indigoCountAtoms(obj));
        EXPECT_EQ(8478, indigoCountBonds(obj));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiFormatsTest, noFile)
{
    ASSERT_THROW(
        {
            int obj = -1;
            obj = indigoLoadStructureFromFile("/wrong/path/to/non/existent/file", "");
        },
        Exception);
}
