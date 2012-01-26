#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"

void onError (const char *message, void *context)
{
   fprintf(stderr, "Error: %s\n", message);
   exit(-1);
}


int main (void)
{
   int m;
   const char *simple_mol = "CCCCCCCCCCC";
   
   indigoSetErrorHandler(onError, 0);
   printf("%s\n", indigoVersion());
   m = indigoLoadMoleculeFromString("COC1=CC2=C(NC(=C2)C(O)(CC2=CN=CC=C2)CC2=CN=CC=C2)C=C1");
   printf("%s\n", indigoCanonicalSmiles(m));

   m = indigoLoadMoleculeFromString(simple_mol);
   if (strcmp(indigoCanonicalSmiles(m), simple_mol) != 0)
   {
      printf("Canonical SMILES is invalid for a molecule: %s != %s\n", simple_mol, indigoCanonicalSmiles(m));
      exit(-1);
   }
   return 0;
}
