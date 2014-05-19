#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "indigo.h"
#include "bingo.h"

void onError (const char *message, void *context)
{
   fprintf(stderr, "Error: %s\n", message);
   exit(-1);
}

int main (void)
{
   indigoSetErrorHandler(onError, 0);
   printf("%s\n", indigoVersion());
   return 0;
}
