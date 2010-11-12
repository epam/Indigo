/*
 * $Header: extdemo5.h 30-apr-2001.16:05:04 hdnguyen Exp $
 */

/* Copyright (c) 2001, Oracle Corporation.  All rights reserved. 
*/

/*
   NAME
     extdemo5.h - Extensible Indexing example implemented as C routines
                  for local domain index on varchar2 column of a 
                  range partitioned table.

   DESCRIPTION
     This file contains the definitions of the DML and Query routines
     for the extensible indexing example that implements a simple btree
     (sbtree). See extdemo5.sql for the SQL script that defines the
     indextype.

   RELATED DOCUMENTS

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
     qxiqtbps - QXIQT Btree Start routine
     qxiqtbpf - QXIQT Btree Fetch routine
     qxiqtbpc - QXIQT Btree Close routine
     qxiqtbpi - QXIQT Btree Insert routine
     qxiqtbpd - QXIQT Btree Delete routine
     qxiqtbpu - QXIQT Btree Update routine

   PRIVATE FUNCTION(S)
     qxiqtbe - QXIQT error reporting routine

   EXAMPLES

   NOTES
     <other useful comments, qualifications, etc.>

   MODIFIED   (MM/DD/YY)
   hdnguyen    04/27/99 - misc. modification
   spsundar    04/25/01 - Creation

*/


#ifndef EXTDEMO5_ORACLE
# define EXTDEMO5_ORACLE

#ifndef OCI_ORACLE
# include <oci.h>
#endif
#ifndef ODCI_ORACLE
# include <odci.h>
#endif

/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
                     PRIVATE TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/

#ifdef WIN32COMMON
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

/* index scan context - should be stored in "statement" duration memory
 * and used by start, fetch and close routines.
 */
struct qxiqtcx
{
  OCIStmt *stmthp;
  OCIDefine *defnp;
  OCIBind *bndp;
  char ridp[19];
};
typedef struct qxiqtcx qxiqtcx;

/* The index implementation type is an ADT with a single RAW attribute
 * which will be used to store the context key value.
 * C mapping of the implementation type :
 */
struct qxiqtim
{
   OCIRaw *sctx_qxiqtim;
};
typedef struct qxiqtim qxiqtim;

struct qxiqtin
{
  short atomic_qxiqtin;
  short scind_qxiqtin;
};
typedef struct qxiqtin qxiqtin;

/*---------------------------------------------------------------------------
                           PUBLIC FUNCTIONS
  ---------------------------------------------------------------------------*/
/* ODCIIndexStart */
OCINumber DLLEXPORT *qxiqtbps(/*_ OCIExtProcContext *ctx,
                     struct qxiqtim *sctx, struct qxiqtin *sctx_ind,
                     ODCIIndexInfo *ix, ODCIIndexInfo_ind *ix_ind,
                     ODCIPredInfo *pr, ODCIPredInfo_ind *pr_ind,
                     ODCIQueryInfo *qy, ODCIQueryInfo_ind *qy_ind,
                     OCINumber *strt, short strt_ind,
                     OCINumber *stop, short stop_ind,
                     char *cmpval, short cmpval_ind, 
                     ODCIEnv *env, ODCIEnv_ind *env_ind _*/);

/* ODCIIndexFetch */
OCINumber DLLEXPORT *qxiqtbpf(/*_ OCIExtProcContext *ctx,
                     struct qxiqtim *self, struct qxiqtin *self_ind,
                     OCINumber *nrows, short nrows_ind,
                     OCIArray **rids, short *rids_ind, 
                     ODCIEnv *env, ODCIEnv_ind *env_ind _*/);

/* ODCIIndexClose */
OCINumber DLLEXPORT *qxiqtbpc(/*_ OCIExtProcContext *ctx,
                     struct qxiqtim *self, struct qxiqtin *self_ind, 
                     ODCIEnv *env, ODCIEnv_ind *env_ind _*/);

/* ODCIIndexInsert */
OCINumber DLLEXPORT *qxiqtbpi(/*_ OCIExtProcContext *ctx,
                     ODCIIndexInfo *ix,
                     ODCIIndexInfo_ind *ix_ind,
                     char *rid,
                     short rid_ind,
                     char *newval,
                     short newval_ind,  
                     ODCIEnv *env, ODCIEnv_ind *env_ind _*/);

/* ODCIIndexDelete  */
OCINumber DLLEXPORT *qxiqtbpd(/*_ OCIExtProcContext *ctx,
                     ODCIIndexInfo *ix,
                     ODCIIndexInfo_ind *ix_ind,
                     char *rid,
                     short rid_ind,
                     char  *oldval,
                     short oldval_ind, 
                     ODCIEnv *env, ODCIEnv_ind *env_ind _*/);

/* ODCIIndexUpdate  */
OCINumber DLLEXPORT *qxiqtbpu(/*_ OCIExtProcContext *ctx,
                     ODCIIndexInfo *ix,
                     ODCIIndexInfo_ind *ix_ind,
                     char *rid,
                     short rid_ind,
                     char *oldval,
                     short oldval_ind,
                     char *newval,
                     short newval_ind, 
                     ODCIEnv *env, ODCIEnv_ind *env_ind _*/);

/*---------------------------------------------------------------------------
                          PRIVATE FUNCTIONS
  ---------------------------------------------------------------------------*/
static int qxiqtce(/*_ OCIExtProcContext *ctx, OCIError *errhp,
                      sword status _*/);

#endif                                              /* EXTDEMO5_ORACLE */
