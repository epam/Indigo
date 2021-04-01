#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"

void testRPE()
{
    indigoSetOption("rpe-multistep-reactions", "1");
    indigoSetOption("rpe-mode", "grid");
    indigoSetOption("rpe-self-reaction", "0");
    indigoSetOption("rpe-max-depth", "3");
    indigoSetOption("rpe-max-products-count", "4");
    int reaction = indigoLoadQueryReactionFromString("[*]C([#17,#35,#53])=O.O[*]>>[*]C(O[*])=O |$_R1;;;;;_R2;_R1;;;_R2;$|");
    int m1 = indigoLoadMoleculeFromString("CC(Cl)=O");
    int m2 = indigoLoadMoleculeFromString("C1(CCCCC1)C(Cl)=O");
    int m3 = indigoLoadMoleculeFromString("[C@@H]1(O)[C@@H](O)[C@H](O)[C@@H](O)[C@H](O)[C@H]1O");
    int m_pair = indigoCreateArray();
    indigoArrayAdd( m_pair, m1 ); indigoArrayAdd( m_pair, m2 );
    int m_single = indigoCreateArray();
    indigoArrayAdd( m_single, m3 );
    int monomers = indigoCreateArray();
    indigoArrayAdd( monomers, m_pair );
    indigoArrayAdd( monomers, m_single );
    int rpe_reactions = indigoReactionProductEnumerate(reaction, monomers);
    printf( "rpe_reactions=%d\n", rpe_reactions);
}

void onError(const char* message, void* context)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(-1);
}

void testTransform()
{
    int molecule = indigoLoadMoleculeFromString("[O-][C+]1CCCC1[N+]([O-])=O");
    int transformation = indigoLoadReactionSmartsFromString("[*;+:1]-[*;-:2]>>[*:1]=[*:2]");

    printf("Input: %s\n", indigoSmiles(molecule));
    // This methods accepts a single molecule or Indigo array of molecules
    // and modifies the source molecule in-place.
    indigoTransform(transformation, molecule);
    printf("Output: %s\n", indigoSmiles(molecule));
    indigoFree(molecule);
    indigoFree(transformation);
}

int main(void)
{
    testRPE();
    int m;
    const char* simple_mol = "CCCCCCCCCCC";
    int r;
    int gf;

    r = indigoLoadReactionFromString("C");
    if (r < 0)
    {
        printf("Error handled: %s\n", indigoGetLastError());
    }
    indigoFree(r);

    indigoSetErrorHandler(onError, 0);
    printf("%s\n", indigoVersion());
    m = indigoLoadMoleculeFromString("COC1=CC2=C(NC(=C2)C(O)(CC2=CN=CC=C2)CC2=CN=CC=C2)C=C1");
    printf("%s\n", indigoCanonicalSmiles(m));
    indigoFree(m);

    m = indigoLoadMoleculeFromString(simple_mol);
    if (strcmp(indigoCanonicalSmiles(m), simple_mol) != 0)
    {
        printf("Canonical SMILES is invalid for a molecule: %s != %s\n", simple_mol, indigoCanonicalSmiles(m));
        exit(-1);
    }
    indigoFree(m);

    testTransform();

    r = indigoLoadReactionFromString("C.CC>>CC.C");
    gf = indigoGrossFormula(r);
    printf("%s\n", indigoToString(gf));
    indigoFree(gf);
    indigoFree(r);
    return 0;
}
