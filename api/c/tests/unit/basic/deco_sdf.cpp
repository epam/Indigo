#include <fstream>

#include <gtest/gtest.h>

#include <indigo.h>

#include "common.h"

using namespace indigo;

TEST(IndigoDecoSDFTest, deco_sdf_test)
{
    qword session = indigoAllocSessionId();
    indigoSetSessionId(session);

    indigoSetErrorHandler(errorHandling, 0);

    indigoClearTautomerRules();
    indigoSetTautomerRule(1, "N,O,P,S,As,Se,Sb,Te", "N,O,P,S,As,Se,Sb,Te");
    indigoSetTautomerRule(2, "0C", "N,O,P,S");
    indigoSetTautomerRule(3, "1C", "N,O");
    int mol1 = indigoLoadQueryMoleculeFromString("CC1(C)NC(=O)C2=CC=CC=C2N1");
    int mol2 = indigoLoadMoleculeFromString("CC(=C)NC1=CC=CC=C1C(N)=O");
    indigoAromatize(mol2);
    int sm = indigoSubstructureMatcher(mol2, "TAU R-C");
    bool res = indigoMatch(sm, mol1);
    printf("tau test: %s\n", res ? "passed" : "failed");
    ASSERT_TRUE(res);
}
