/* Copyright (c) 2002, Oracle Corporation.  All rights reserved.  */
 
/* 
   NAME 
     cdemosp.h - OCI Session Pool demo program header


   MODIFIED   (MM/DD/YY)
   jchai       01/28/02 - Merged jchai_change_oci_sp_sc_cp_names
   sudsrini    01/08/02 - Merged sudsrini_enable_sp_sc_suite
   sudsrini    01/03/02 - Creation

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

#ifndef OCISP_ORACLE
# define OCISP_ORACLE

/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                     PRIVATE TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                           EXPORT FUNCTIONS
  ---------------------------------------------------------------------------*/

int main (void);

/*---------------------------------------------------------------------------
                          INTERNAL FUNCTIONS
  ---------------------------------------------------------------------------*/


#endif                                              /* OCISP_ORACLE */
