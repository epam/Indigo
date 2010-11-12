/*
 * $Header: extdemo4.h 08-feb-2001.18:14:55 ayoaz Exp $
 */

/* Copyright (c) Oracle Corporation 1998, 2000. All Rights Reserved. */

/*
   NAME
     extdemo4.h - user defined aggregates using C safe callouts
                  with external aggregation context

   DESCRIPTION 
     This file contains the OCI declarations of the SQL types used
   by the SumVector aggregate function.

   MODIFIED   (MM/DD/YY)
   ayoaz       02/08/01 - Merged ayoaz_udag_demo
   ayoaz       02/06/01 - Creation

*/

#ifndef EXTDEMO4_ORACLE
#define EXTDEMO4_ORACLE
 
#ifndef OCI_ORACLE
# include <oci.h>
#endif
#ifndef ODCI_ORACLE
# include <odci.h>
#endif

/*-------------------------------------------------------------------------
                 OCI REPRESENTATION OF SQL TYPES
  -----------------------------------------------------------------------*/

/* Vector_t - this is the OCI representation of the Vector_t SQL type */

struct Vector_t
{
  OCINumber length;
  OCINumber angle;
};
typedef struct Vector_t Vector_t;

/* Indicator struct for Vector_t type */

struct Vector_Ind_t
{
  OCIInd _atomic;
  OCIInd length;
  OCIInd angle;
};
typedef struct Vector_Ind_t Vector_Ind_t;

/* AggCtx_t - OCI representation of the AggCtx_t SQL type */

struct AggCtx_t
{
  OCINumber x;
  OCINumber y;
};
typedef struct AggCtx_t AggCtx_t;

/* Indicator struct for AggCtx_t */

struct AggCtx_Ind_t
{
  OCIInd _atomic;
  OCIInd x;
  OCIInd y;
};
typedef struct AggCtx_Ind_t AggCtx_Ind_t;

/* Imp_t - OCI representation of the Imp_t SQL type */

struct Imp_t
{
  OCIRaw* key;     /* a key identifying the external context (null if none) */
  AggCtx_t aggCtx;  /* the aggregation context (null if context is external */
};
typedef struct Imp_t Imp_t;

/* Indicator struct for Imp_t */

struct Imp_Ind_t
{
  OCIInd _atomic;
  OCIInd key;
  AggCtx_Ind_t aggCtx;
};
typedef struct Imp_Ind_t Imp_Ind_t;

/*------------------------------------------------------------------------
                           PUBLIC FUNCTIONS
  ----------------------------------------------------------------------*/

/* C implementation of ODCIAgregateInitialize */
int Initialize(OCIExtProcContext* extProcCtx, 
               Imp_t* self, Imp_Ind_t* self_ind);

/* C implementation of ODCIAgregateIterate */
int Iterate(OCIExtProcContext* extProcCtx,
            Imp_t* self, Imp_Ind_t* self_ind,
            Vector_t* arg, Vector_Ind_t* arg_ind);

/* C implementation of ODCIAgregateTerminate */
int Terminate(OCIExtProcContext* extProcCtx,
              Imp_t* self, Imp_Ind_t* self_ind,
              Vector_t* result, Vector_Ind_t* result_ind,
              OCINumber* flags, OCIInd flags_ind);

/* C implementation of ODCIAggregateMerge */
int Merge(OCIExtProcContext* extProcCtx,
          Imp_t* self, Imp_Ind_t* self_ind,
          Imp_t* sctx2, Imp_Ind_t* sctx2_ind);

/* C implementation of ODCIAgregateDelete */
int Delete(OCIExtProcContext* extProcCtx,
           Imp_t* self, Imp_Ind_t* self_ind,
           Vector_t* arg, Vector_Ind_t* arg_ind);

/* C implementation of ODCIAgregateWrapContext */
int WrapContext(OCIExtProcContext* extProcCtx,
                Imp_t* self, Imp_Ind_t* self_ind);

#endif

