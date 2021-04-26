#include <gtest/gtest.h>

#include <indigo-inchi.h>

#include "common.h"

using namespace indigo;

TEST(IndigoInChITest, basic)
{
    int m;
    const char* inchi = "InChI=1S/C10H20N2O2/c11-7-1-5-2-8(12)10(14)4-6(5)3-9(7)13/h5-10,13-14H,1-4,11-12H2";
    const char* res_inchi;

    indigoSetErrorHandler(errorHandling, 0);
    printf("%s\n", indigoVersion());
    m = indigoInchiLoadMolecule(inchi);
    printf("%s\n", indigoCanonicalSmiles(m));

    res_inchi = indigoInchiGetInchi(m);
    ASSERT_EQ(strcmp(res_inchi, inchi), 0);
}
