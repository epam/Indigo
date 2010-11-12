/* Copyright (c) Oracle Corporation 1995.  All Rights Reserved. */

/*
  NAME
    cdemoin2 - Demo program to perform attribute substitutability.

  DESCRIPTION
    This program demonstrates attribute substitutability, wherein a column
    which is of REF to a supertype is substituted with a REF to a subtype.
    All the data from the table are then displayed.

  NOTES
    dependent files :
      cdemoin2.c    - Source file.
      cdemoin2.sql  - SQL File to be run before execution of the test.
      cdemoin2.tsc  - Optional, test script file.

  MODIFIED
     rdwajan   08/11/00   Created
*/

#ifndef CDEMOIN2_ORACLE
# define CDEMOIN2_ORACLE

#ifndef OCI_ORACLE
# include <oci.h>
#endif

typedef OCIRef cdemoin2_sec_address_ref;
typedef OCIRef cdemoin2_address_ref;

struct cdemoin2_address
{
   OCINumber hno;
   OCIString * street;
};
typedef struct cdemoin2_address cdemoin2_address;

struct cdemoin2_address_ind
{
   OCIInd _atomic;
   OCIInd hno;
   OCIInd street;
};
typedef struct cdemoin2_address_ind cdemoin2_address_ind;

struct cdemoin2_sec_address
{
   cdemoin2_address _super;
   OCIString * city;
   OCIString * state;
};
typedef struct cdemoin2_sec_address cdemoin2_sec_address;

struct cdemoin2_sec_address_ind
{
   cdemoin2_address _super;
   OCIInd city;
   OCIInd state;
};
typedef struct cdemoin2_sec_address_ind cdemoin2_sec_address_ind;

#endif
