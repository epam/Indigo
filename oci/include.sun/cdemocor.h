/*
 * $Header: cdemocor.h 14-jul-99.12:47:46 mjaeger Exp $
 */

/* Copyright (c) 1995, 1999, Oracle Corporation.  All rights reserved.
*/

/*
   NAME
     cdemocor.h

   DESCRIPTION
     This file contains the header information for the cdemocor.c

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
    mjaeger   07/14/99 -  bug 808870: OCCS: convert tabs, no long lines
    echen     06/05/97 -  remove ifdefed code
    echen     05/30/97 -  Creation
*/

#ifndef CDEMOCOR_ORACLE
#define CDEMOCOR_ORACLE

#ifndef OCI_ORACLE
#include <oci.h>
#endif


#ifdef __FILE__
#ifdef __LINE__
#define checkerr(a, b) checkerr1(a, b, __FILE__, __LINE__)
#else
#define checkerr(a, b) checkerr1(a, b, __FILE__, 0)
#endif
#else
#define checkerr(a, b) checkerr1(a, b, "???", 0)
#endif


/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                     PRIVATE TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/
struct purchase_order
{
  OCINumber     po_number;
  OCIRef        *cust;
  OCIRef        *related_orders;
};
typedef struct purchase_order purchase_order;

struct customer
{
  OCIString     *name;
  OCINumber     age;
  OCITable      *addr;
};
typedef struct customer customer;

struct address
{
  OCIString *state; /* text   state[3]; */
  OCIString *zip; /* text   zip[11]; */
};
typedef struct address address;

struct null_address
{
  sb2    null_address;
  sb2    null_state;
  sb2    null_zip;
};
typedef struct null_address null_address;

#endif /* CDEMOCOR_ORACLE */
