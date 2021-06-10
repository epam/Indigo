#include <gtest/gtest.h>

#include <molecule/molecule_mass.h>

#include <indigo-inchi.h>
#include <indigo.h>
#include <indigo_internal.h>

#include "common.h"

using namespace indigo;

TEST(IndigoBasicApiTest, test_mass)
{
    Molecule t_mol;

    loadMolecule("[81Br]", t_mol);

    MoleculeMass mm;

    auto m = mm.monoisotopicMass(t_mol);
    ASSERT_NEAR(80.9163, m, 0.01);
}

TEST(IndigoBasicApiTest, test_inchi)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, nullptr);

    int mol = indigoInchiLoadMolecule("InChI=1S/C18H18/c1-2-4-6-8-10-12-14-16-18-17-15-13-11-9-7-5-3-1/h1-18H/"
                                      "b2-1-,3-1+,4-2+,5-3+,6-4+,7-5-,8-6-,9-7+,10-8+,11-9+,12-10+,13-11-,14-12-,15-13+,16-14+,17-15+,18-16+,18-17-");
    ASSERT_GT(mol, 0);
}

int sum(int s, int x)
{
    if (x > 5)
    {
        return s;
    }
    QS_DEF(ArrayInt, xs);
    xs.push(x);

    int top = xs[0];

    for (int i = 0; i < x; ++i)
    {
        sum(s, x + 1);
    }

    return s + (top - x);
}

TEST(IndigoBasicApiTest, test_qsdef)
{
    int res = sum(0, 0);
    ASSERT_EQ(res, 0);
}

TEST(IndigoBasicApiTest, test_smarts_match)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

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

TEST(IndigoBasicApiTest, test_rgroup_dearom)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

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

TEST(IndigoBasicApiTest, test_valence)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

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

TEST(IndigoBasicApiTest, test_sgroup_utf)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

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
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST(IndigoBasicApiTest, test_reset_options)
{

    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    indigoSetOption("ignore-noncritical-query-features", "true");
    indigoSetOption("max-embeddings", "10");

    Indigo& self = indigoGetInstance();

    ASSERT_EQ(self.ignore_noncritical_query_features, true);
    ASSERT_EQ(self.max_embeddings, 10);

    indigoResetOptions();

    ASSERT_EQ(self.ignore_noncritical_query_features, false);
    ASSERT_EQ(self.max_embeddings, 10000);
}

TEST(IndigoBasicApiTest, test_getter_function)
{

    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

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
}