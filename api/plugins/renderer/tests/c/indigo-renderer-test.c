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

#ifdef WIN32
#include <Windows.h>
#endif

void testHDC ()
{
   int buffer_object;
   char *raw_ptr;
   int size;
#ifdef WIN32
   HDC hdc, hdc2;
#endif
   int hdc_buffer_object;
   int molecule;

   molecule = indigoLoadMoleculeFromString("C1=CC=CC=C1");
   indigoSetOption("render-output-format", "png");
   indigoSetOption("render-background-color", "255, 255, 255");
   indigoRenderToFile(molecule, "indigo-renderer-test.png");

   buffer_object = indigoWriteBuffer();
   indigoRender(molecule, buffer_object);

   indigoToBuffer(buffer_object, &raw_ptr, &size);
   //<Copy the raw_ptr data anywhere>
   indigoFree(buffer_object);

#ifdef WIN32
   hdc = GetWindowDC(NULL);
   hdc2 = CreateCompatibleDC(hdc);

   hdc_buffer_object = indigoRenderWriteHDC(hdc2, 0);
   indigoRender(molecule, hdc_buffer_object);
   indigoFree(hdc_buffer_object);
#endif

   indigoFree(molecule);

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

   testHDC();
   return 0;
}
