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

#include <indigo-inchi.h>
#include <indigo.h>

#include "common.h"

using namespace indigo;

class IndigoApiInchiTest : public IndigoApiTest
{
protected:
    void SetUp() final
    {
        IndigoApiTest::SetUp();
        indigoInchiInit(session);
    }

    void TearDown() final
    {
        indigoInchiDispose(session);
        IndigoApiTest::TearDown();
    }
};

TEST_F(IndigoApiInchiTest, test_inchi)
{
    int mol = indigoInchiLoadMolecule("InChI=1S/C18H18/c1-2-4-6-8-10-12-14-16-18-17-15-13-11-9-7-5-3-1/h1-18H/"
                                      "b2-1-,3-1+,4-2+,5-3+,6-4+,7-5-,8-6-,9-7+,10-8+,11-9+,12-10+,13-11-,14-12-,15-13+,16-14+,17-15+,18-16+,18-17-");
    ASSERT_GT(mol, 0);
}

TEST_F(IndigoApiInchiTest, basic)
{
    try
    {
        const char* inchi = "InChI=1S/C10H20N2O2/c11-7-1-5-2-8(12)10(14)4-6(5)3-9(7)13/h5-10,13-14H,1-4,11-12H2";
        const auto m = indigoInchiLoadMolecule(inchi);
        ASSERT_EQ(strcmp(indigoCanonicalSmiles(m), "NC1CC2CC(N)C(O)CC2CC1O"), 0);
        const char* res_inchi = indigoInchiGetInchi(m);
        ASSERT_EQ(strcmp(res_inchi, inchi), 0);
        const char* empty_inchi = "InChI=1S//";
        const auto empty = indigoInchiLoadMolecule(empty_inchi);
        EXPECT_ANY_THROW(indigoInchiGetInchi(empty));
    }
    catch (const std::exception& e)
    {
        ASSERT_STREQ("inchi-wrapper: Wrong InChI format", e.what());
    }
}

TEST_F(IndigoApiInchiTest, incorrect_symbols)
{
    try
    {
        const char* inchi = "InChI=1S/C7H16O/c1-2-3-4-5-6-7-8$h8H,2-7H2,1H3";
        const auto m = indigoInchiLoadMolecule(inchi);
        EXPECT_ANY_THROW(indigoInchiGetInchi(m));
    }
    catch (const std::exception& e)
    {
        ASSERT_STREQ("inchi-wrapper: Wrong InChI format", e.what());
    }
}

TEST_F(IndigoApiInchiTest, incorrect_symbols_123)
{
    try
    {
        const char* inchi = "InChI=123";
        const auto m = indigoInchiLoadMolecule(inchi);
        EXPECT_ANY_THROW(indigoInchiGetInchi(m));
    }
    catch (const std::exception& e)
    {
        ASSERT_STREQ("inchi-wrapper: Wrong InChI format", e.what());
    }
}

TEST_F(IndigoApiInchiTest, without_options)
{
    int m = indigoLoadMoleculeFromString("c1ccccc1");
    const char* inchi = indigoInchiGetInchi(m);
    ASSERT_STREQ("InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H", inchi);
}

TEST_F(IndigoApiInchiTest, without_options_nullptr)
{
    int m = indigoLoadMoleculeFromString("c1ccccc1");
    const char* inchi = indigoInchiGetInchiWithForcedOptions(m, nullptr);
    ASSERT_STREQ("InChI=1S/C6H6/c1-2-4-6-5-3-1/h1-6H", inchi);
}

TEST_F(IndigoApiInchiTest, srel)
{
    int m = indigoLoadMoleculeFromString("c1ccccc1");
    const char* inchi = indigoInchiGetInchiWithForcedOptions(m, "/SRel");
    ASSERT_STREQ("InChI=1/C6H6/c1-2-4-6-5-3-1/h1-6H", inchi);
}

TEST_F(IndigoApiInchiTest, srac)
{
    int m = indigoLoadMoleculeFromString("c1ccccc1");
    const char* inchi = indigoInchiGetInchiWithForcedOptions(m, "/SRac");
    ASSERT_STREQ("InChI=1/C6H6/c1-2-4-6-5-3-1/h1-6H", inchi);
}

const char* orEnantiomer = "\n"
                           "  TestStruct 2\n\n"
                           "  0  0  0     0  0            999 V3000\n"
                           "M  V30 BEGIN CTAB\n"
                           "M  V30 COUNTS 8 8 0 0 1\n"
                           "M  V30 BEGIN ATOM\n"
                           "M  V30 1 C 9.5653 -6.4929 0 0\n"
                           "M  V30 2 C 9.5653 -7.3196 0 0\n"
                           "M  V30 3 C 10.2813 -7.733 0 0\n"
                           "M  V30 4 C 10.9972 -7.3196 0 0 CFG=2\n"
                           "M  V30 5 C 10.9972 -6.4929 0 0 CFG=1\n"
                           "M  V30 6 C 10.2813 -6.0795 0 0\n"
                           "M  V30 7 O 11.7131 -6.0795 0 0\n"
                           "M  V30 8 N 11.7131 -7.7329 0 0\n"
                           "M  V30 END ATOM\n"
                           "M  V30 BEGIN BOND\n"
                           "M  V30 1 1 1 6\n"
                           "M  V30 2 1 1 2\n"
                           "M  V30 3 1 2 3\n"
                           "M  V30 4 1 3 4\n"
                           "M  V30 5 1 4 5\n"
                           "M  V30 6 1 5 6\n"
                           "M  V30 7 1 5 7 CFG=1\n"
                           "M  V30 8 1 4 8 CFG=1\n"
                           "M  V30 END BOND\n"
                           "M  V30 BEGIN COLLECTION\n"
                           "M  V30 MDLV30/STEREL1 ATOMS=(2 5 4)\n"
                           "M  V30 END COLLECTION\n"
                           "M  V30 END CTAB\n"
                           "M  END\n";

TEST_F(IndigoApiInchiTest, or_enantiomer_backward_compat1)
{
    int m = indigoLoadMoleculeFromString(orEnantiomer);
    const char* inchi = indigoInchiGetInchiWithForcedOptions(m, nullptr);
    ASSERT_STREQ("InChI=1/C6H13NO/c7-5-3-1-2-4-6(5)8/h5-6,8H,1-4,7H2/t5-,6+/s2", inchi);
}

TEST_F(IndigoApiInchiTest, or_enantiomer_backward_compat2)
{
    int m = indigoLoadMoleculeFromString(orEnantiomer);
    const char* inchi = indigoInchiGetInchi(m);
    ASSERT_STREQ("InChI=1/C6H13NO/c7-5-3-1-2-4-6(5)8/h5-6,8H,1-4,7H2/t5-,6+/s2", inchi);
}

TEST_F(IndigoApiInchiTest, or_enantiomer_standard)
{
    int m = indigoLoadMoleculeFromString(orEnantiomer);
    const char* inchi = indigoInchiGetInchiWithForcedOptions(m, "");
    const char* expected = "InChI=1S/C6H13NO/c7-5-3-1-2-4-6(5)8/h5-6,8H,1-4,7H2/t5-,6+/m0/s1";

    ASSERT_STREQ(expected, inchi);
    inchi = indigoInchiGetInchiWithForcedOptions(m, "/WarnOnEmptyStructure");
    ASSERT_STREQ(expected, inchi);
    inchi = indigoInchiGetInchiWithForcedOptions(m, "-WarnOnEmptyStructure");
    ASSERT_STREQ(expected, inchi);
}

TEST_F(IndigoApiInchiTest, or_enantiomer_srel)
{
    int m = indigoLoadMoleculeFromString(orEnantiomer);
    const char* inchi = indigoInchiGetInchiWithForcedOptions(m, "/SRel");
    ASSERT_STREQ("InChI=1/C6H13NO/c7-5-3-1-2-4-6(5)8/h5-6,8H,1-4,7H2/t5-,6+/s2", inchi);
}

TEST_F(IndigoApiInchiTest, or_enantiomer_srac)
{
    int m = indigoLoadMoleculeFromString(orEnantiomer);
    const char* inchi = indigoInchiGetInchiWithForcedOptions(m, "/SRac");
    ASSERT_STREQ("InChI=1/C6H13NO/c7-5-3-1-2-4-6(5)8/h5-6,8H,1-4,7H2/t5-,6+/s3", inchi);
}

TEST_F(IndigoApiInchiTest, or_enantiomer_srac_srel)
{
    int m = indigoLoadMoleculeFromString(orEnantiomer);
    const char* inchi = indigoInchiGetInchiWithForcedOptions(m, "/SRac /SRel");
    ASSERT_STREQ("InChI=1/C6H13NO/c7-5-3-1-2-4-6(5)8/h5-6,8H,1-4,7H2/t5-,6+/s2", inchi);
}
