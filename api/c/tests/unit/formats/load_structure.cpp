#include <string>

#include <gtest/gtest.h>

#include <base_cpp/exception.h>

#include <indigo.h>

#include "common.h"

using namespace std;
using namespace indigo;

TEST(IndigoLoadTest, molecule)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

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
        EXPECT_STREQ("*C1C=CC=CC=1 |$A;;;;;;$|", indigoCanonicalSmiles(obj));
        EXPECT_EQ(7, indigoCountAtoms(obj));
        EXPECT_EQ(7, indigoCountBonds(obj));

        // 3
        const string expectedSmarts = "[#6;A]1-[#6;A]=[#6;A]-[#6;A]=[#6;A]-[#6;A]=1-[!#1]";
        obj = indigoLoadStructureFromString(mStr.c_str(), "smarts");
        EXPECT_STREQ(expectedSmarts.c_str(), indigoSmarts(obj));
        EXPECT_EQ(7, indigoCountAtoms(obj));
        EXPECT_EQ(7, indigoCountBonds(obj));

        // 4
        const string expectedQuery = "[#6]1-[#6]=[#6]-[#6]=[#6]-[#6]=1-[!#1]";
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

TEST(IndigoLoadTest, reaction)
{
    const string react = "C1=C(*)C=CC=C1>>C1=CC=CC(*)=C1";
    const string expected = "C1C=CC=CC=1*>>C1C=C(*)C=CC=1";

    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    try
    {
        int obj = -1;
        obj = indigoLoadStructureFromString(react.c_str(), "smarts");
        EXPECT_STREQ(expected.c_str(), indigoSmiles(obj));
        EXPECT_STREQ(expected.c_str(), indigoCanonicalSmiles(obj));
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

TEST(IndigoLoadTest, smarts)
{
    const string mStr = "[OH]c1ccccc1";
    //    const string expectedQuery  = "[#8;H]-[#6;a]1:[#6;a]:[#6;a]:[#6;a]:[#6;a]:[#6;a]:1";
    //    const string expectedSmarts = "[#8;A;H]-[#6;a]1-,:[#6;a]-,:[#6;a]-,:[#6;a]-,:[#6;a]-,:[#6;a]-,:1";

    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

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

TEST(IndigoLoadTest, query)
{
    const string mStr = "c1[nH]c2c(c(N)[n+]([O-])c[n]2)[n]1";
    //"[#8;A]-[*]-[#6;A](-[#9])(-[#9])-[#9]"

    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

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

TEST(IndigoLoadTest, loadAssert)
{
    const string mStr = "C1=C(*)C=?C=C1";
    const string expectedError = "molecule auto loader: SMILES loader: Character #63 is unexpected during bond parsing";

    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    try
    {
        int obj = -1;
        obj = indigoLoadStructureFromString(mStr.c_str(), "");
        EXPECT_EQ(false, true);
    }
    catch (Exception& e)
    {
        ASSERT_STREQ(expectedError.c_str(), e.message());
    }
}

TEST(IndigoLoadTest, fromBuffer)
{
    const byte mStr[] = "[CX4H3][#6]";
    const int buffSize = sizeof(mStr);

    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    try
    {
        int obj = -1;
        obj = indigoLoadStructureFromBuffer(mStr, sizeof(mStr), "query");
        // EXPECT_EQ(1, indigoCheckQuery(obj));
        EXPECT_EQ(2, indigoCountAtoms(obj));
        EXPECT_EQ(1, indigoCountBonds(obj));

        const byte react2[] = "C1CC[NH:2]CC1.C1CC[S:1]CC1>>C1CC2CC[S:2]CC2C[NH:1]1";
        obj = indigoLoadStructureFromBuffer(react2, sizeof(react2), "");
        EXPECT_EQ(3, indigoCountMolecules(obj));

        const byte react[] = "C[12CH2:1]C(CCCC)[CH]CCCCCCC>>C[13CH2:1]C(CCCC)[C]CCCCCCCC |^1:7,^4:22|";
        obj = indigoLoadStructureFromBuffer(react, sizeof(react), "query");
        EXPECT_EQ(2, indigoCountMolecules(obj));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST(IndigoLoadTest, fromFile)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);
    indigoSetErrorHandler(errorHandling, 0);

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

TEST(IndigoLoadTest, fromGzFile)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);
    indigoSetErrorHandler(errorHandling, 0);

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
        int obj = indigoLoadStructureFromFile(dataPath("molecules/basic/pharmapendium.sdf.gz").c_str(), "query");
        EXPECT_EQ(68, indigoCountAtoms(obj));
        EXPECT_EQ(71, indigoCountBonds(obj));
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST(IndigoLoadTest, noFile)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    ASSERT_THROW(
    {
        int obj = -1;
        obj = indigoLoadStructureFromFile("/wrong/path/to/non/existent/file", "");
    }, Exception);
}
