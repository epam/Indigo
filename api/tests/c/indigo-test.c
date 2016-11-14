#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"

void onError (const char *message, void *context)
{
    fprintf(stderr, "Error: %s\n", message);
    exit(-1);
}

void testTransform ()
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

int main (void)
{
    int m;
    const char *simple_mol = "CCCCCCCCCCC";
    int r;
    int gf;

    // r = indigoLoadReactionFromString("C");
    if (r < 0)
    {
        printf("Error handled: %s\n", indigoGetLastError());
    }
    // indigoFree(r);

    // indigoSetErrorHandler(onError, 0);
    // printf("%s\n", indigoVersion());
    // m = indigoLoadMoleculeFromString("COC1=CC2=C(NC(=C2)C(O)(CC2=CN=CC=C2)CC2=CN=CC=C2)C=C1");
    // printf("%s\n", indigoCanonicalSmiles(m));
    // indigoFree(m);

    // m = indigoLoadMoleculeFromString(simple_mol);
    // if (strcmp(indigoCanonicalSmiles(m), simple_mol) != 0)
    // {
    //     printf("Canonical SMILES is invalid for a molecule: %s != %s\n", simple_mol, indigoCanonicalSmiles(m));
    //     exit(-1);
    // }
    // indigoFree(m);

    // testTransform();

    // r = indigoLoadReactionFromString("C.CC>>CC.C");
    // gf = indigoGrossFormula(r);
    // printf("%s\n", indigoToString(gf));
    // indigoFree(gf);
    // indigoFree(r);
    m = indigoLoadMoleculeFromString("\n  Ketcher 11081615222D 1   1.00000     0.00000     0\n\n  5  5  0     0  0            999 V2000\n   -3.9392   -0.1000    0.0000 F   0  0  0  0  0  0  0  0  0  0  0  0\n   -0.2571    1.5821    0.0000 N   0  0  0  0  0  0  0  0  0  0  0  0\n    1.3250    1.0500    0.0000 P   0  0  0  0  0  0  0  0  0  0  0  0\n    4.9250   -0.3000    0.0000 C   0  0  0  0  0  0  0  0  0  0  0  0\n    0.1429   -2.4821    0.0000 O   0  0  0  0  0  0  0  0  0  0  0  0\n  1  2  2  0     0  0\n  2  3  2  0     0  0\n  3  4  1  0     0  0\n  3  5  2  0     0  0\n  5  1  2  0     0  0\nM  END\n");
    indigoClean2d(m);
    printf("%s\n", indigoMolfile(m));
    return 0;
}
