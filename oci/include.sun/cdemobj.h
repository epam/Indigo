/*
 * $Header: cdemobj.h 07-dec-2000.19:03:46 aliu Exp $
 */

/* Copyright (c) 1995, 1999, Oracle Corporation.  All rights reserved.
*/

/*
   NAME
     cdemobj.h

   DESCRIPTION
     This file contains the header information for the cdemobj.c

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
    aliu       12/07/00  - fix bug 1237851: add #include <stdio.h>.
    mjaeger    07/14/99 -  bug 808870: OCCS: convert tabs, no long lines
    skmishra   05/14/97 -  stdcc compatibility changes
    azhao      03/28/97 -  include ocikp.h
    echen      11/15/96 -  remove unnecessary header files
    dchatter   07/18/96 -  delete spurious .h files
    echen      07/16/96 -  Creation
*/

#ifndef CDEMO_OBJ_ORACLE
#define CDEMO_OBJ_ORACLE

#ifndef OCI_ORACLE
#include <oci.h>
#endif

#include <stdio.h>

/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                     PRIVATE TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/
struct address
{
   OCINumber  no;
   OCIString  *street;
   OCIString  *state;
   OCIString  *zip;
};
typedef struct address address;

struct null_address
{
  sb2    null_addr;
  sb2    null_no;
  sb2    null_street;
  sb2    null_state;
  sb2    null_zip;
};
typedef struct null_address null_address;

struct person
{
   OCIString *fname;
   OCIString *lname;
   OCINumber  age;
   OCINumber  salary;
   OCINumber  bonus;
   OCINumber  retirement_fund;
   OCINumber  number_of_kids;
   OCINumber  years_of_school;
   OCITable  *preaddr;
   OCIDate  birthday;
   OCINumber  number_of_pets;
   OCIRaw  *comment1;
   OCILobLocator   *comment2;
   OCIString *comment3;
   address addr;
};
typedef struct person person;

struct null_person
{
  OCIInd        null_per;
  OCIInd        null_fname;
  OCIInd        null_lname;
  OCIInd        null_age;
  OCIInd        null_salary;
  OCIInd        null_bonus;
  OCIInd        null_retirement_fund;
  OCIInd        null_number_of_kids;
  OCIInd        null_years_of_school;
  OCIInd        null_preaddr;
  OCIInd        null_birthday;
  OCIInd        null_number_of_pets;
  OCIInd        null_comment1;
  OCIInd        null_comment2;
  OCIInd        null_comment3;
  null_address  null_addr;
};
typedef struct null_person null_person;

struct customer
{
   OCIString  *account;
   OCIRef     *aperson;
};
typedef struct customer customer;

struct null_customer
{
  OCIInd            null_cus;
  OCIInd            null_account;
  OCIInd            null_aperson;
};
typedef struct null_customer null_customer;

int main(/*_ void _*/);

#endif                                              /* TKPOTTA_ORACLE */
