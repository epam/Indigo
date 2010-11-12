/* Copyright (c) Oracle Corporation 1995.  All Rights Reserved. */

/*
  NAME
    i_residence - Demo program which modifies an inherited type in a table and
    displays a record from the table.

  DESCRIPTION
    This program pins an inherited instance in the object cache and displays
    the attributes in it. It also updates a record of a table.

  RELATED DOCUMENTS
  

  NOTES 
  MODIFIED
     rdwajan   08/11/00   Created
*/
#include <stdio.h>
#include <string.h>

#ifndef CDEMOIN1_ORACLE
# define CDEMOIN1_ORACLE

#ifndef OCI_ORACLE
# include <oci.h>
#endif

typedef OCIRef i_manager_ref;
typedef OCIArray i_residence_arr;
typedef OCIRef i_people_ref;
typedef OCIRef i_residence_ref;
typedef OCITable i_residence_nest;

struct i_residence
{
   OCINumber hno;
   OCIString * street;
};
typedef struct i_residence i_residence;

struct i_residence_ind
{
   OCIInd _atomic;
   OCIInd hno;
   OCIInd street;
};
typedef struct i_residence_ind i_residence_ind;

struct i_people
{
   OCIString * name;
   OCINumber ssn;
   struct i_residence addr;
   i_residence_nest * altadrs;
};
typedef struct i_people i_people;

struct i_people_ind
{
   OCIInd _atomic;
   OCIInd name;
   OCIInd ssn;
   struct i_residence_ind addr;
   OCIInd altadrs;
};
typedef struct i_people_ind i_people_ind;

struct i_manager
{
   i_people _super;
   OCINumber empno;
   i_residence_arr * workadd;
};
typedef struct i_manager i_manager;

struct i_manager_ind
{
   i_people _super;
   OCIInd empno;
   OCIInd workadd;
};
typedef struct i_manager_ind i_manager_ind;

#endif
