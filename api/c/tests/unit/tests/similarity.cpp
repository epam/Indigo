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

class IndigoSimilarityTest : public IndigoApiTest
{
public:
    int m1;
    int m2; // `m1` with a small chain
    int m3; // `m2` written in a non-canonical way
    int m4; // contains explicit hydrogen atoms

protected:
    void SetUp() final
    {
        IndigoApiTest::SetUp();
        m1 = indigoLoadMoleculeFromString("C1C=C(OCC)C=CC=1");
        m2 = indigoLoadMoleculeFromString("C1C=C(OCCCC)C=CC=1");
        m3 = indigoLoadMoleculeFromString("C1=CC=C(OCCCC)C=C1");
        m4 = indigoLoadMoleculeFromString("Cl[2H].OC(=O)[C@@H](N)CC1=CC([2H])=C(O)C([2H])=C1");
    }

    void TearDown() final
    {
        IndigoApiTest::TearDown();
    }
};

TEST_F(IndigoSimilarityTest, generate_fingerprints)
{
    for (int mol : {m1, m2, m4})
    {
        for (const auto& type : {"sim", "sub", "sub-res", "sub-tau", "full"})
        {
            ASSERT_NE(-1, indigoFingerprint(mol, type));
        }

        const char* similarity_type_names[] = {
            "sim",   "chem", "ecfp2", "ecfp4",
            "ecfp6", "ecfp8"

            //      TODO: implement FCFP fingerprints
            //            "fcfp2", "fcfp4", "fcfp6", "fcfp8"
        };

        for (auto& mode : similarity_type_names)
        {
            indigoSetOption("similarity-type", mode);
            ASSERT_NE(-1, indigoFingerprint(mol, "sim"));
        }
    }
}

TEST_F(IndigoSimilarityTest, similarity_sim)
{
    const char* type = "sim";
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);
    int f3 = indigoFingerprint(m3, type);

    EXPECT_LT(0.85, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_EQ(1.00, indigoSimilarity(f2, f3, "tanimoto"));
}

TEST_F(IndigoSimilarityTest, similarity_sub)
{
    const char* type = "sub";
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);
    int f3 = indigoFingerprint(m3, type);

    EXPECT_LT(0.85, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_EQ(1.00, indigoSimilarity(f2, f3, "tanimoto"));
}

TEST_F(IndigoSimilarityTest, similarity_subres)
{
    const char* type = "sub-res";
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);
    int f3 = indigoFingerprint(m3, type);

    EXPECT_LT(0.85, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_EQ(1.00, indigoSimilarity(f2, f3, "tanimoto"));
}

TEST_F(IndigoSimilarityTest, similarity_subtau)
{
    const char* type = "sub-tau";
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);
    int f3 = indigoFingerprint(m3, type);

    EXPECT_LT(0.85, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_EQ(1.00, indigoSimilarity(f2, f3, "tanimoto"));
}

TEST_F(IndigoSimilarityTest, similarity_full)
{
    const char* type = "full";
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);
    int f3 = indigoFingerprint(m3, type);

    EXPECT_LT(0.85, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_EQ(1.00, indigoSimilarity(f2, f3, "tanimoto"));
}

TEST_F(IndigoSimilarityTest, similarity_normalized_edit)
{
    indigoSetErrorHandler(nullptr, nullptr);
    const char* type = "full";
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);

    EXPECT_LT(0.80, indigoSimilarity(m1, m2, "normalized-edit"));
    EXPECT_EQ(1.00, indigoSimilarity(m2, m3, "normalized-edit"));
    EXPECT_EQ(-1, indigoSimilarity(f1, f2, "normalized-edit"));
}

TEST_F(IndigoSimilarityTest, similarity_sim_chem_mode)
{
    const char* type = "sim";
    indigoSetOption("similarity-type", "chem");
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);
    int f3 = indigoFingerprint(m3, type);

    EXPECT_LT(0.80, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_GT(0.99, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_EQ(1.00, indigoSimilarity(f2, f3, "tanimoto"));
}

TEST_F(IndigoSimilarityTest, similarity_sim_ECFP2_mode)
{
    const char* type = "sim";
    indigoSetOption("similarity-type", "ecfp2");
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);
    int f3 = indigoFingerprint(m3, type);

    EXPECT_LT(0.06, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_GT(0.99, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_EQ(1.00, indigoSimilarity(f2, f3, "tanimoto"));
}

TEST_F(IndigoSimilarityTest, similarity_sim_ECFP4_mode)
{
    const char* type = "sim";
    indigoSetOption("similarity-type", "ecfp4");
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);
    int f3 = indigoFingerprint(m3, type);

    EXPECT_LT(0.5, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_GT(0.99, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_EQ(1.00, indigoSimilarity(f2, f3, "tanimoto"));
}

TEST_F(IndigoSimilarityTest, similarity_sim_ECFP6_mode)
{
    const char* type = "sim";
    indigoSetOption("similarity-type", "ecfp6");
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);
    int f3 = indigoFingerprint(m3, type);

    EXPECT_LT(0.4, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_GT(0.99, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_EQ(1.00, indigoSimilarity(f2, f3, "tanimoto"));
}

TEST_F(IndigoSimilarityTest, similarity_sim_ECFP8_mode)
{
    const char* type = "sim";
    indigoSetOption("similarity-type", "ecfp8");
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);
    int f3 = indigoFingerprint(m3, type);

    EXPECT_LT(0.4, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_GT(0.99, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_EQ(1.00, indigoSimilarity(f2, f3, "tanimoto"));
}

TEST_F(IndigoSimilarityTest, similarity_sim_FCFP2_mode)
{
    const char* type = "sim";
    indigoSetOption("similarity-type", "fcfp2");
    int f1 = indigoFingerprint(m1, type);
    int f2 = indigoFingerprint(m2, type);
    int f3 = indigoFingerprint(m3, type);
    
    EXPECT_LT(0.06, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_GT(0.99, indigoSimilarity(f1, f2, "tanimoto"));
    EXPECT_EQ(1.00, indigoSimilarity(f2, f3, "tanimoto"));
}
