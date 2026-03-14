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

static constexpr char Mol[] = R"(
  XXXX

  4  2  0  0  0  0  0  0  0  0999 V2000
   13.4520   -7.5745    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.1657   -7.9874    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.1347  -10.0969    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
   14.8501  -10.5060    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  4  3  2  0  0  0  0
M  STY  5   1 FOR   2 COM   3 COM   4 DAT   5 DAT
M  SPL  2   4   2   5   3
M  SAL   1  4   1   2   3   4
M  SDI   1  4    7.3843  -12.7547    7.3843   -5.3847
M  SDI   1  4   17.8743   -5.3847   17.8743  -12.7547
M  SAL   2  2   1   2
M  SDI   2  4   13.0243   -8.4047   13.0243   -5.9047
M  SDI   2  4   16.5743   -5.9047   16.5743   -8.4047
M  SAL   3  2   3   4
M  SDI   3  4   12.9943  -12.1647   12.9943   -9.6647
M  SDI   3  4   16.5443   -9.6647   16.5443  -12.1647
M  SDT   4 WEIGHT_PERCENT                F
M  SDD   4    16.7243   -6.1647    DA    ALL  1       5
M  SED   4 30
M  SDT   5 WEIGHT_PERCENT                F
M  SDD   5    16.6543  -10.0647    DA    ALL  1       5
M  SED   5 22.5
M  END
)";

TEST_F(IndigoApiBasicTest, molecule_iterate_components_and_render)
{
    try
    {
        const int mol = indigoLoadMoleculeFromString(Mol);
        const int iterator = indigoIterateComponents(mol);

        while (indigoHasNext(iterator))
        {
            int component = indigoNext(iterator);
            int cloned = indigoClone(component);

            auto molfile = indigoMolfile(cloned);
            auto smiles = indigoSmiles(cloned);

            indigoFree(cloned);
        }
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

// SiF5(2-) — silicon pentafluoride dianion from rand_queries_small.sdf molecule #50.
// Element::calcValence now handles Si with charge=-2 for conn==5 (SiF5^2-)
// in addition to conn==6 (SiF6^2-).
// See: https://github.com/epam/Indigo — bingo-elastic FullUsageMoleculeTest.substructureSearch
static constexpr char SiF5_charge_minus2_mol[] = R"(
  -INDIGO-10082014522D

  6  5  0  0  0  0  0  0  0  0999 V2000
   -1.0862   -0.0414    0.0000 Si  0  0  0  0  0  0  0  0  0  0  0  0
   -2.1862   -0.9552    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
   -1.3241    1.3690    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
    0.0172    0.8586    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
   -2.4310    0.4552    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
    0.2552   -0.5517    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  1  3  1  0  0  0  0
  1  4  1  0  0  0  0
  1  5  1  0  0  0  0
  1  6  1  0  0  0  0
M  CHG  1   1  -2
M  END
)";

// Reproduces the failing test from CI: FullUsageMoleculeTest.substructureSearch
// After fix: SiF5^2- should produce valid canonical SMILES
TEST_F(IndigoApiBasicTest, canonical_smiles_SiF5_charge_minus2)
{
    const int mol = indigoLoadMoleculeFromString(SiF5_charge_minus2_mol);
    ASSERT_NE(-1, mol);

    // Verify the molecule was loaded correctly
    ASSERT_EQ(6, indigoCountAtoms(mol));
    ASSERT_EQ(5, indigoCountBonds(mol));

    // After fix: calcValence handles Si charge=-2, conn=5 → valence=5, hyd=0
    const char* smiles = indigoCanonicalSmiles(mol);
    ASSERT_NE(nullptr, smiles);
    ASSERT_STRNE("", smiles);
}

// Verify that the related SiF6^2- (hexafluorosilicate) works correctly,
// since Element::calcValence explicitly handles charge=-2, conn=6.
TEST_F(IndigoApiBasicTest, canonical_smiles_SiF6_charge_minus2)
{
    // SiF6^2- — this case IS handled in Element::calcValence
    int mol = indigoLoadMoleculeFromString("[Si-2](F)(F)(F)(F)(F)F");
    ASSERT_NE(-1, mol);
    ASSERT_EQ(7, indigoCountAtoms(mol));

    const char* smiles = indigoCanonicalSmiles(mol);
    ASSERT_NE(nullptr, smiles);
    ASSERT_STRNE("", smiles);
}

// Scenario 3: Invalid molecule in MOL format — Si with charge=-2 and only 3 connections
// checkBadValence returns descriptive error message with element, charge, bonds
static constexpr char SiF3_charge_minus2_invalid_mol[] = R"(
  -INDIGO-test

  4  3  0  0  0  0  0  0  0  0999 V2000
    0.0000    0.0000    0.0000 Si  0  0  0  0  0  0  0  0  0  0  0  0
    1.0000    0.0000    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
    0.0000    1.0000    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
   -1.0000    0.0000    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  1  3  1  0  0  0  0
  1  4  1  0  0  0  0
M  CHG  1   1  -2
M  END
)";

TEST_F(IndigoApiBasicTest, canonical_smiles_invalid_molecule_descriptive_error)
{
    // Si with charge=-2 and 3 fluorines from MOL — no valid calcValence rule
    int mol = indigoLoadMoleculeFromString(SiF3_charge_minus2_invalid_mol);
    ASSERT_NE(-1, mol);

    // checkBadValence should return descriptive error with element, charge, bonds
    const char* valenceErr = indigoCheckBadValence(mol);
    ASSERT_NE(nullptr, valenceErr);
    std::string errStr(valenceErr);
    EXPECT_NE(std::string::npos, errStr.find("Si")) << "Error should mention Si, got: " << errStr;
    EXPECT_NE(std::string::npos, errStr.find("-2")) << "Error should mention charge -2, got: " << errStr;
    EXPECT_NE(std::string::npos, errStr.find("3 drawn bonds")) << "Error should mention 3 drawn bonds, got: " << errStr;
}