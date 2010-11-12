/*
 * $Header: extdemo2.h 14-jul-99.12:48:29 mjaeger Exp $
 */

/* Copyright (c) 1998, 1999, Oracle Corporation.  All rights reserved.
*/

/*
   NAME
     extdemo2.h - Extensible Indexing example implemented as C routines

   DESCRIPTION
     This file contains the definitions of the DML and Query routines
     for the extensible indexing example that implements a simple btree
     (sbtree). See extdemo2.sql for the SQL script that defines the
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
     qxiqtbs - QXIQT Btree Start routine
     qxiqtbf - QXIQT Btree Fetch routine
     qxiqtbc - QXIQT Btree Close routine
     qxiqtbi - QXIQT Btree Insert routine
     qxiqtbd - QXIQT Btree Delete routine
     qxiqtbu - QXIQT Btree Update routine

   PRIVATE FUNCTION(S)
     qxiqtbe - QXIQT error reporting routine

   EXAMPLES

   NOTES
     <other useful comments, qualifications, etc.>

   MODIFIED   (MM/DD/YY)
   mjaeger     07/14/99 - bug 808870: OCCS: convert tabs, no long lines
   hdnguyen    04/02/99 - merged khackel fix in
   khackel     02/03/99 - WIN32COMMON: added dllexport for public functions
   rmurthy     06/16/98 - Creation

*/


#ifndef EXTDEMO2_ORACLE
# define EXTDEMO2_ORACLE

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
OCINumber DLLEXPORT *qxiqtbs(/*_ OCIExtProcContext *ctx,
                     struct qxiqtim *sctx, struct qxiqtin *sctx_ind,
                     ODCIIndexInfo *ix, dvoid *ix_ind,
                     ODCIPredInfo *pr, dvoid *pr_ind,
                     ODCIQueryInfo *qy, dvoid *qy_ind,
                     OCINumber *strt, short strt_ind,
                     OCINumber *stop, short stop_ind,
                     char *cmpval, short cmpval_ind _*/);

/* ODCIIndexFetch */
OCINumber DLLEXPORT *qxiqtbf(/*_ OCIExtProcContext *ctx,
                     struct qxiqtim *self, struct qxiqtin *self_ind,
                     OCINumber *nrows, short nrows_ind,
                     OCIArray **rids, short *rids_ind _*/);

/* ODCIIndexClose */
OCINumber DLLEXPORT *qxiqtbc(/*_ OCIExtProcContext *ctx,
                     struct qxiqtim *self, struct qxiqtin *self_ind _*/);

/* ODCIIndexInsert */
OCINumber DLLEXPORT *qxiqtbi(/*_ OCIExtProcContext *ctx,
                     ODCIIndexInfo *ix,
                     ODCIIndexInfo_ind *ix_ind,
                     char *rid,
                     short rid_ind,
                     char *newval,
                     short newval_ind _*/);

/* ODCIIndexDelete  */
OCINumber DLLEXPORT *qxiqtbd(/*_ OCIExtProcContext *ctx,
                     ODCIIndexInfo *ix,
                     ODCIIndexInfo_ind *ix_ind,
                     char *rid,
                     short rid_ind,
                     char  *oldval,
                     short oldval_ind _*/);

/* ODCIIndexUpdate  */
OCINumber DLLEXPORT *qxiqtbu(/*_ OCIExtProcContext *ctx,
                     ODCIIndexInfo *ix,
                     ODCIIndexInfo_ind *ix_ind,
                     char *rid,
                     short rid_ind,
                     char *oldval,
                     short oldval_ind,
                     char *newval,
                     short newval_ind _*/);

/*---------------------------------------------------------------------------
                          PRIVATE FUNCTIONS
  ---------------------------------------------------------------------------*/
static int qxiqtce(/*_ OCIExtProcContext *ctx, OCIError *errhp,
                      sword status _*/);

#endif                                              /* EXTDEMO2_ORACLE */
