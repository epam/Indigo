#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"
#include "indigo-inchi.h"

void onError (const char *message, void *context)
{
   fprintf(stderr, "Error: %s\n", message);
   exit(-1);
}

int main (void)
{
   int m;
   const char *inchi = "InChI=1S/C10H20N2O2/c11-7-1-5-2-8(12)10(14)4-6(5)3-9(7)13/h5-10,13-14H,1-4,11-12H2";
   const char *res_inchi;
   
   indigoSetErrorHandler(onError, 0);
   printf("%s\n", indigoVersion());
   m = indigoInchiLoadMolecule(inchi);
   printf("%s\n", indigoCanonicalSmiles(m));

   res_inchi = indigoInchiGetInchi(m);
   if (strcmp(res_inchi, inchi) != 0)
   {
      printf("Converted Inchi is invalid: %s != %s\n", inchi, res_inchi);
      exit(-1);
   }
   return 0;
}
