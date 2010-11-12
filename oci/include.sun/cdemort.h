/*
 * $Header: cdemort.h 14-jul-99.12:52:18 mjaeger Exp $
 */

/* Copyright (c) 1995, 1999, Oracle Corporation.  All rights reserved.
*/

/*
   NAME
     cdemort.h

   DESCRIPTION
     This file contains the header information for the cdemort.c

   RELATED DOCUMENTS
     None.

   INSPECTION STATUS
     Inspection date:
     Inspection status:
     Estimated increasing cost defects per page:
     Rule sets:

   ACCEPTANCE REVIEW STATUS
     Review date:
     Review status:
     Reviewers:

   PUBLIC FUNCTION(S)
     None.

   PRIVATE FUNCTION(S)
     As defined below.

   EXAMPLES

   NOTES
     <other useful comments, qualifications, etc.>
   MODIFIED   (MM/DD/YY)
    mjaeger    07/14/99 -  bug 808870: OCCS: convert tabs, no long lines
    echen      11/15/96 -  oci beautification
    dchatter   07/18/96 -  delete spurious .h files
    echen      08/07/95 -  Creation
*/

#ifndef CDEMO_OBJ_ORACLE
#define CDEMO_OBJ_ORACLE

#ifndef OCI_ORACLE
#include <oci.h>
#endif


/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                     PRIVATE TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/
#define RUNTEST TRUE
#define USERNAME "internal"
#define SERVER "ORACLE"
#define SCHEMA "SYS"
#define ADDRESS_TYPE_NAME "ADDRESS_OBJECT"
#define RETURN_ON_ERROR(error) if (error) return (error)

static void unit_test_type_access(/*_ OCIEnv *envhp,  OCIError *errhp,
                                      OCISvcCtx *svchp,  char *type_name  _*/);

int main(/*_ int argc, char *argv[] _*/);

#endif                                              /* CDEMO_OBJ_ORACLE */

