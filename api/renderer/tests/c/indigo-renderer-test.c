#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"
#include "indigo-renderer.h"

void onError (const char *message, void *context)
{
   fprintf(stderr, "Error: %s\n", message);
   exit(-1);
}


int main (void)
{
   int m;
   
   indigoSetErrorHandler(onError, 0);
   printf("%s\n", indigoVersion());
   m = indigoLoadMoleculeFromString("COC1=CC2=C(NC(=C2)C(O)(CC2=CN=CC=C2)CC2=CN=CC=C2)C=C1");
   printf("%s\n", indigoCanonicalSmiles(m));

   indigoSetOption("render-output-format", "png");
   indigoSetOption("render-atom-ids-visible", "true"); 
   indigoSetOption("render-bond-ids-visible", "true"); 
   indigoSetOption("render-background-color", "255, 255, 255"); 
   indigoRenderToFile(m, "indigo-renderer-test.png"); 

   return 0;
}
