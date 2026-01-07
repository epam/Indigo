#include <gtest/gtest.h>

#include "common.h"
#include <indigo.h>
#include <indigo_internal.h>

using namespace indigo;

class IndigoApiTautomersTest : public IndigoApiTest
{
};

TEST_F(IndigoApiTautomersTest, testExactTau)
{
    try
    {
        const char* flags = "TAU STRICT";
        // 2-Penten-2-ol
        int mol1 = indigoLoadMoleculeFromString("CC(O)=CCC");

        // 2-Pentanone
        int mol2 = indigoLoadMoleculeFromString("CC(CCC)=O");

        int match = indigoExactMatch(mol1, mol2, flags);
        ASSERT_NE(0, match) << "Expected match between 2-Penten-2-ol and 2-Pentanone";
        if (match)
            indigoFree(match);

        // 3-Penten-2-ol
        int mol3 = indigoLoadMoleculeFromString("CC(O)/C=C/C");
        match = indigoExactMatch(mol1, mol3, flags);
        ASSERT_EQ(0, match) << "Expected NO match between 2-Penten-2-ol and 3-Penten-2-ol with restricted rules";
        int mol4 = indigoLoadMoleculeFromString("CC(/C=C(\\O)/C)O");
        match = indigoExactMatch(mol1, mol4, flags);
        ASSERT_EQ(0, match) << "Expected NO match between 2-Penten-2-ol and 3-Penten-2-ol with restricted rules";
        if (match)
            indigoFree(match);

        indigoFree(mol1);
        indigoFree(mol2);
        indigoFree(mol3);
    }
    catch (Exception& e)
    {
        ASSERT_STREQ("", e.message());
    }
}

TEST_F(IndigoApiTautomersTest, testSpecificTautomers)
{
    const char* flags = "TAU STRICT";
    int mol1 = indigoLoadMoleculeFromString("C(/C)(\\O)=C\\C(CC)O");
    int mol2 = indigoLoadMoleculeFromString("C(C)(O)/C=C(/CC)\\O");

    int match = indigoExactMatch(mol1, mol2, flags);
    ASSERT_NE(0, match) << "Expected match between specified tautomers";

    if (match)
        indigoFree(match);

    indigoFree(mol1);
    indigoFree(mol2);
}

TEST_F(IndigoApiTautomersTest, testExpandedTautomerRules)
{
    const char* flags = "TAU STRICT";

    // 1. NOT MATCHED: Allylic Isomerization (Simple Alkene Shift)
    // C=CCCC (1-Pentene) vs CC=CCC (2-Pentene)
    // Shift C1-C2 to C2-C3. Proton shift involved. Neither C1 nor C3 activated.
    int mol1 = indigoLoadMoleculeFromString("C=CCCC");
    int mol2 = indigoLoadMoleculeFromString("CC=CCC");
    int match = indigoExactMatch(mol1, mol2, flags);
    ASSERT_EQ(0, match) << "Allylic Isomerization should NOT match";
    if (match)
        indigoFree(match);
    indigoFree(mol1);
    indigoFree(mol2);

    // 2. NOT MATCHED: Allylic Alcohol Isomerization
    // CC(O)=CCC (Enol) vs CC(O)C=CC (Allylic Alcohol)
    mol1 = indigoLoadMoleculeFromString("CC(O)=CCC");
    mol2 = indigoLoadMoleculeFromString("CC(O)C=CC");
    match = indigoExactMatch(mol1, mol2, flags);
    ASSERT_EQ(0, match) << "Allylic Alcohol Isomerization should NOT match";
    if (match)
        indigoFree(match);
    indigoFree(mol1);
    indigoFree(mol2);

    // 3. MATCHED: Keto-Enol Tautomerism
    // CC(=O)C (Acetone) vs CC(O)=C (Propen-2-ol)
    mol1 = indigoLoadMoleculeFromString("CC(=O)C");
    mol2 = indigoLoadMoleculeFromString("CC(O)=C");
    match = indigoExactMatch(mol1, mol2, flags);
    ASSERT_NE(0, match) << "Keto-Enol should MATCH";
    if (match)
        indigoFree(match);
    indigoFree(mol1);
    indigoFree(mol2);

    // 4. MATCHED: Imine-Enamine Tautomerism
    // CC=N (Imine) vs C=CN (Enamine)
    mol1 = indigoLoadMoleculeFromString("CC=N");
    mol2 = indigoLoadMoleculeFromString("C=CN");
    match = indigoExactMatch(mol1, mol2, flags);
    ASSERT_NE(0, match) << "Imine-Enamine should MATCH";
    if (match)
        indigoFree(match);
    indigoFree(mol1);
    indigoFree(mol2);

    // 5. MATCHED: Enediol Tautomerism
    // O-C=C(O) (Enediol) vs O=C-C(O) (Hydroxyketone)
    mol1 = indigoLoadMoleculeFromString("OC=C(O)");
    mol2 = indigoLoadMoleculeFromString("O=CC(O)");
    match = indigoExactMatch(mol1, mol2, flags);
    ASSERT_NE(0, match) << "Enediol should MATCH";
    if (match)
        indigoFree(match);
    indigoFree(mol1);
    indigoFree(mol2);

    // 6. MATCHED: 1,3-Dicarbonyl Tautomerism (Activated Methylene)
    // CC(=O)CC(=O)C (Pentane-2,4-dione) vs CC(=O)C=C(O)C (Enol form)
    mol1 = indigoLoadMoleculeFromString("CC(=O)CC(=O)C");
    mol2 = indigoLoadMoleculeFromString("CC(=O)C=C(O)C");
    match = indigoExactMatch(mol1, mol2, flags);
    ASSERT_NE(0, match) << "1,3-Dicarbonyl should MATCH";
    if (match)
        indigoFree(match);
    indigoFree(mol1);
    indigoFree(mol2);

    // 7. MATCHED: Nitro-Aci-Nitro Tautomerism
    // [O-][N+](=O)C (Nitromethane) vs [O-][N+](=C)O (Aci-nitro zwitterion)
    // Ensures valence compatibility (N+ in both).
    mol1 = indigoLoadMoleculeFromString("[O-][N+](=O)C");
    mol2 = indigoLoadMoleculeFromString("[O-][N+](=C)O");
    match = indigoExactMatch(mol1, mol2, flags);
    ASSERT_NE(0, match) << "Nitro-Aci-Nitro should MATCH";
    if (match)
        indigoFree(match);
    indigoFree(mol1);
    indigoFree(mol2);
}
