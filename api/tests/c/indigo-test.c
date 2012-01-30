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
   
   testTransform();
   
   return 0;
}
