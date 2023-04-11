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

#include <indigo-renderer.h>
#include <indigo.h>
#include <indigo_internal.h>

#include "common.h"

using namespace indigo;

class IndigoApiBasicTest : public IndigoApiTest
{
};

TEST_F(IndigoApiBasicTest, arom_test_merge)
{
    try
    {
        int m = indigoLoadMoleculeFromString("c1ccccc1.c1ccccc1");
        int c = indigoComponent(m, 0);
        int cc = indigoClone(c);
        indigoDearomatize(cc);
        Array<int> vertices;
        for (int i = 0; i < 6; ++i)
            vertices.push(i);

        indigoRemoveAtoms(m, vertices.size(), vertices.ptr());
        //      printf("%s\n", indigoSmiles(cc));

        indigoMerge(m, cc);
        ASSERT_STREQ("C1C=CC=CC=1.c1ccccc1", indigoSmiles(m));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiBasicTest, test_smarts_match)
{
    try
    {
        {
            int mol = indigoLoadMoleculeFromFile(dataPath("molecules/sss/arom_het_5_21.mol").c_str());
            int q = indigoLoadQueryMoleculeFromString("[CX4H3][#6]");
            int match = indigoSubstructureMatcher(mol, "");
            ASSERT_EQ(2, indigoCountMatches(match, q));
        }
        {
            int mol = indigoLoadMoleculeFromFile(dataPath("molecules/sss/arom_het_5_35.mol").c_str());
            int q = indigoLoadQueryMoleculeFromString("[a;!c]");
            int match = indigoSubstructureMatcher(mol, "");
            ASSERT_EQ(3, indigoCountMatches(match, q));
        }
        {
            int mol = indigoLoadMoleculeFromFile(dataPath("molecules/sss/arom_het_5_79.mol").c_str());
            int q = indigoLoadQueryMoleculeFromString("[!#6;!R0]");
            int match = indigoSubstructureMatcher(mol, "");
            ASSERT_EQ(3, indigoCountMatches(match, q));
        }
        {
            int mol = indigoLoadMoleculeFromString("c1ccccc1");
            int q = indigoLoadQueryMoleculeFromString("[!#6;!R0]");
            int match = indigoSubstructureMatcher(mol, "");
            ASSERT_EQ(0, indigoCountMatches(match, q));
        }
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiBasicTest, test_rgroup_dearom)
{
    try
    {

        int mol = indigoLoadMoleculeFromFile(dataPath("molecules/rgroups/Rgroup_for_Dearomatize.mol").c_str());
        indigoAromatize(mol);
        indigoDearomatize(mol);
        int iter = indigoIterateRGroups(mol);
        while (indigoHasNext(iter) > 0)
        {
            int r = indigoNext(iter);
            int riter = indigoIterateRGroupFragments(r);
            while (indigoHasNext(riter) > 0)
            {
                int rf = indigoNext(riter);
                ASSERT_STREQ("NC1=NC=NC2NC=NC=21", indigoCanonicalSmiles(rf));
            }
        }
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiBasicTest, test_valence)
{
    try
    {

        int mol = indigoLoadMoleculeFromString("CC(C)(C)(C)(C)CC");
        ASSERT_STREQ("element: bad valence on C having 6 drawn bonds, charge 0, and 0 radical electrons", indigoCheckBadValence(mol));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiBasicTest, test_sgroup_utf)
{
    try
    {

        int mol = indigoLoadMoleculeFromFile(dataPath("molecules/sgroups/sgroups_utf8.mol").c_str());

        int iter = indigoIterateDataSGroups(mol);

        while (indigoHasNext(iter) > 0)
        {
            int sg = indigoNext(iter);

            //         flog << indigoData(sg);
            ASSERT_STREQ("single-value-бензол", indigoData(sg));
        }

        indigoFree(iter);
        indigoFree(mol);
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiBasicTest, test_reset_options)
{
    indigoSetOption("ignore-noncritical-query-features", "true");
    indigoSetOption("max-embeddings", "10");

    Indigo& self = indigoGetInstance();

    ASSERT_EQ(self.ignore_noncritical_query_features, true);
    ASSERT_EQ(self.max_embeddings, 10);

    indigoResetOptions();

    ASSERT_EQ(self.ignore_noncritical_query_features, false);
    ASSERT_EQ(self.max_embeddings, 10000);
}

TEST_F(IndigoApiBasicTest, test_getter_function)
{
    int bl;
    indigoSetOption("ignore-noncritical-query-features", "true");
    indigoGetOptionBool("ignore-noncritical-query-features", &bl);
    ASSERT_EQ((bool)bl, true);
    const char* chBool = indigoGetOption("ignore-noncritical-query-features");
    ASSERT_STREQ(chBool, "true");

    int i;
    indigoSetOption("max-embeddings", "10");
    indigoGetOptionInt("max-embeddings", &i);
    ASSERT_EQ(i, 10);
    const char* chInt = indigoGetOption("max-embeddings");
    ASSERT_STREQ(chInt, "10");

    indigoSetOption("filename-encoding", "ASCII");
    const char* ch = indigoGetOption("filename-encoding");
    ASSERT_STREQ(ch, "ASCII");

    float f;
    indigoSetOptionFloat("layout-horintervalfactor", 20.5f);
    indigoGetOptionFloat("layout-horintervalfactor", &f);
    ASSERT_NEAR(f, 20.5f, 0.1f);
    const char* chFloat = indigoGetOption("layout-horintervalfactor");
    ASSERT_STREQ(chFloat, "20.5");

    float r, g, b;
    indigoRendererInit(session);
    indigoSetOptionColor("render-background-color", 50, 100, 150);
    indigoGetOptionColor("render-background-color", &r, &g, &b);
    ASSERT_EQ(r, 50);
    ASSERT_EQ(g, 100);
    ASSERT_EQ(b, 150);
    const char* chColor = indigoGetOption("render-background-color");
    ASSERT_STREQ(chColor, "[50, 100, 150]");

    int x, y;
    indigoSetOptionXY("render-image-size", 250, 400);
    indigoGetOptionXY("render-image-size", &x, &y);
    ASSERT_EQ(x, 250);
    ASSERT_EQ(y, 400);
    const char* chXY = indigoGetOption("render-image-size");
    ASSERT_STREQ(chXY, "[250, 400]");

    indigoRendererDispose(session);
}

TEST_F(IndigoApiBasicTest, test_exact_match)
{
    int mol = indigoLoadMoleculeFromFile(dataPath("molecules/other/39004.1src.mol").c_str());

    byte* buf;
    int size;
    indigoSerialize(mol, &buf, &size);
    Array<char> buffer;
    buffer.copy((const char*)buf, size);
    int mol2 = indigoUnserialize((const byte*)buffer.ptr(), buffer.size());

    int res = indigoExactMatch(mol, mol2, "");

    ASSERT_NE(0, res);
}

TEST_F(IndigoApiBasicTest, name2structure_alkanes)
{
    const char* data = "ethane";
    int molecule = indigoNameToStructure(data, nullptr);
    EXPECT_NE(-1, molecule);

    const char* smiles = indigoCanonicalSmiles(molecule);
    EXPECT_STREQ(smiles, "CC");
}

TEST_F(IndigoApiBasicTest, submolecule_test_layout)
{
    try
    {
        int mol = indigoLoadMoleculeFromString("CC.NN.PP.OO");
        Array<int> vertices;
        vertices.push(6);
        vertices.push(7);
        int sub_mol = indigoGetSubmolecule(mol, vertices.size(), vertices.ptr());
        indigoLayout(sub_mol);
        indigoClean2d(sub_mol);
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiBasicTest, submolecule_test_general)
{
    try
    {
        int mol = indigoLoadMoleculeFromString("C1=CC=CC=C1.C1=CC=CC=C1");
        Array<int> vertices;
        for (int i = 0; i < 6; ++i)
            vertices.push(i);

        int sub_mol = indigoGetSubmolecule(mol, vertices.size(), vertices.ptr());

        ASSERT_STREQ("C1C=CC=CC=1", indigoCanonicalSmiles(sub_mol));
        ASSERT_TRUE(strlen(indigoMolfile(sub_mol)) < 650);

        indigoAromatize(sub_mol);
        ASSERT_STREQ("c1ccccc1", indigoCanonicalSmiles(sub_mol));

        ASSERT_NEAR(78.1118, indigoMolecularWeight(sub_mol), 0.1);
        ASSERT_NEAR(78.1118, indigoMostAbundantMass(sub_mol), 0.1);
        ASSERT_NEAR(78.1118, indigoMonoisotopicMass(sub_mol), 0.1);
        ASSERT_STREQ("C 92.26 H 7.74", indigoMassComposition(sub_mol));
        ASSERT_STREQ("C6 H6", indigoToString(indigoGrossFormula(sub_mol)));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiBasicTest, memory_leak)
{
    int m = indigoLoadMoleculeFromString("C1CCCCC1");
}