#include <gtest/gtest.h>

#include <indigo.h>

TEST(NameToStructure, alkanesTest)
{
    const char* data = "ethane";
    int molecule = indigoNameToStructure(data, nullptr);
    EXPECT_NE(-1, molecule);

    const char* smiles = indigoCanonicalSmiles(molecule);
    EXPECT_STREQ(smiles, "CC");
}
