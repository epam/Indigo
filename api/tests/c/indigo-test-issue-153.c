#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"

int main(void)
{
    indigoSetOption("pKa-model", "advanced");
    indigoSetOptionInt("pKa-model-level", 1);

    int m = indigoLoadMoleculeFromString("[2H]C1=C([2H])C([2H])=C(C2N=C(N3CCCCCC3)SC=2C(=O)C2=CC=CC=C2)C([2H])=C1[2H]");
    indigoIonize(m, 7.4, 0);
}
