#include <gtest/gtest.h>

#include <indigo-inchi.h>

#include "common.h"

using namespace indigo;

TEST(IndigoInChITest, basic)
{
    indigoSetErrorHandler(errorHandling, nullptr);

    const char* inchi = "InChI=1S/C10H20N2O2/c11-7-1-5-2-8(12)10(14)4-6(5)3-9(7)13/h5-10,13-14H,1-4,11-12H2";
    const auto m = indigoInchiLoadMolecule(inchi);
    ASSERT_EQ(strcmp(indigoCanonicalSmiles(m), "NC1CC2CC(N)C(O)CC2CC1O"), 0);
    const char* res_inchi = indigoInchiGetInchi(m);
    ASSERT_EQ(strcmp(res_inchi, inchi), 0);
}
