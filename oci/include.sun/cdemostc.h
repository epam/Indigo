/* Copyright (c) 2002, Oracle Corporation.  All rights reserved.  */
 
/* 
   NAME 
     cdemostc.h - OCI Statement caching


   MODIFIED   (MM/DD/YY)
   sudsrini    01/30/02 - Rename ocisc -> cdemostc
   gkirsur     01/25/02 - Merged gkirsur_stcache_tests
   sudsrini    01/24/02 - Removed argc and argv from main definition
   sprabhak    01/22/02 - Incorporated Review comments  
   sprabhak    01/10/02 - Creation

*/

#ifndef ORATYPES
# include <oratypes.h>
#endif

#ifndef OCI_ORACLE
# include <oci.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef OCISC_ORACLE
# define OCISC_ORACLE

/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                     PRIVATE TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                           EXPORT FUNCTIONS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                          INTERNAL FUNCTIONS
  ---------------------------------------------------------------------------*/

int main(void);
static void checkerr (OCIError *errhp, sword status);
static void queryRows(OCISvcCtx *svchp);

#endif                                              /* OCISC_ORACLE */
