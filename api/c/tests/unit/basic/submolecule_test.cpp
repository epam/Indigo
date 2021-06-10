#include <gtest/gtest.h>

#include <base_cpp/exception.h>
#include <base_cpp/output.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;

TEST(IndigoSubmoleculeTest, sub_test_layout)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    try
    {
        int mol = indigoLoadMoleculeFromString("CC.NN.PP.OO");
        ArrayInt vertices;
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

TEST(IndigoSubmoleculeTest, sub_test_general)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    try
    {
        int mol = indigoLoadMoleculeFromString("C1=CC=CC=C1.C1=CC=CC=C1");
        ArrayInt vertices;
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
