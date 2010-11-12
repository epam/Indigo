/* Copyright (c) 1999, 2001, Oracle Corporation.  All rights reserved.  */
 
/* 
   NAME 
     ocixad.h - OCI eXtensible Access Driver (for external tables)

   DESCRIPTION 
     Provides handles for XAD support.

   RELATED DOCUMENTS 
     External Tables Design specification (external_tab_ds.doc)
 
   EXPORT FUNCTION(S) 

   INTERNAL FUNCTION(S)

   EXAMPLES

   NOTES
     Currently these interfaces are for Oracle internal use only.

   MODIFIED   (MM/DD/YY)
   cmlim       08/10/01 - date cache: add XADSESSION attrs: DCACHE_SIZE 
   abrumm      04/18/01 - define interface method signatures in OCIXAD
   abrumm      02/20/01 - add attributes for ACCESS_PARM_TYPE
   abrumm      01/18/01 - more OCI_ATTR_XADSESSION attributes
   abrumm      10/09/00 - use oratypes.h, not s.h
   abrumm      03/30/00 - external table support
   abrumm      03/30/00 - Creation

*/

#ifndef OCIXAD_ORACLE
#define OCIXAD_ORACLE

#ifndef ORATYPES
#include <oratypes.h>
#endif

#ifndef OCIDFN
#include <ocidfn.h>
#endif

#ifndef OCI_ORACLE
#include <oci.h>
#endif

#ifndef OCIEXTP_ORACLE
#include <ociextp.h>
#endif

/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/

/*----- Handles and descriptors for access driver operations (OCIXAD*)  -----*/
typedef struct OCIXADSession      OCIXADSession;           /* session handle */
typedef struct OCIXADTable        OCIXADTable;               /* table handle */
typedef struct OCIXADField        OCIXADField;               /* field handle */
typedef struct OCIXADGranule      OCIXADGranule;           /* granule handle */

/*---------------- Access Driver method interface signatures ----------------*/

/* Each of the following access driver methods are logically member functions
 * of the OCIXADSession handle.  As such, the first argument to the method is
 * the OCIXADSession handle (i.e. "this").
 *
 * An access driver writer must provide these entry points at configure
 * time via the OCIXADMethodEntry structure.
 */

typedef sword (*OCIXADMethodOpen)(OCIXADSession *xadses,
                                  OCIExtProcContext *withCtx,
                                  OCIXADTable *xadtbl,
                                  OCILobLocator *accessParm);

typedef sword (*OCIXADMethodFetchInit)(OCIXADSession *xadses,
                                       OCIExtProcContext *withCtx,
                                       OCIXADTable *xadtbl,
                                       OCINumber *gnum, ub4 maxRowCnt);

typedef sword (*OCIXADMethodFetch)(OCIXADSession *xadses,
                                   OCIExtProcContext *withCtx,
                                   OCIXADTable *xadtbl, void *opaqueCtx,
                                   ub4 rowCnt, sb4 rejctLmt,
                                   sb4 *rejctCntp);

typedef void (*OCIXADMethodPopulateInit)(OCIXADSession *xadses,
                                         OCIExtProcContext *withCtx);

typedef void (*OCIXADMethodPopulate)(OCIXADSession *xadses,
                                     OCIExtProcContext *withCtx);

typedef void (*OCIXADMethodPopulateTerm)(OCIXADSession *xadses,
                                         OCIExtProcContext *withCtx);

typedef sword  (*OCIXADMethodClose)(OCIXADSession *xadses,
                                    OCIExtProcContext *withCtx,
                                    OCIXADTable *xadtbl);

typedef void (*OCIXADMethodErrorCallback)(OCIXADSession *xadses,
                                          OCIExtProcContext *withCtx,
                                          void *opaqueCtx, OCIError *errhp,
                                          ub4 rowidx, ub2 colIdx);

/* Access Driver Method Entry points provided at "configure" time. */
struct OCIXADMethodEntry
{
  ub4                       Version_OCIXADMethodEntry;
#define OCIXAD_METHOD_ENTRY_VERSION_1   100
#define OCIXAD_METHOD_ENTRY_VERSION_CUR OCIXAD_METHOD_ENTRY_VERSION_1

  OCIXADMethodOpen          Open_OCIXADMethodEntry;
  OCIXADMethodFetchInit     FetchInit_OCIXADMethodEntry;
  OCIXADMethodFetch         Fetch_OCIXADMethodEntry;
  OCIXADMethodPopulateInit  PopulateInit_OCIXADMethodEntry;
  OCIXADMethodPopulate      Populate_OCIXADMethodEntry;
  OCIXADMethodPopulateTerm  PopulateTerm_OCIXADMethodEntry;
  OCIXADMethodClose         Close_OCIXADMethodEntry;
  OCIXADMethodErrorCallback ErrorCallback_OCIXADMethodEntry;
};
typedef struct OCIXADMethodEntry OCIXADMethodEntry;

/* Each access driver type (e.g. "ORACLE_LOADER") must provide a
 * configure function (currently via the kpxdconf[] array).
 * The configure function must set the
 * OCI_ATTR_XADSESSION_METHOD_ENTRY_POINTS attribute of the passed in
 * 'hndl' argument.  The OCI_ATTR_XADSESSION_METHOD_ENTRY_POINTS attribute
 * is a pointer to an OCIXADMethodEntry structure.
 */
typedef sword (*OCIXADConfig)(OraText *driverType,       /* driver type name */
                              void     *hndl,        /* OCIXADSession handle */
                              ub4       hndlType,             /* handle type */
                              OCIError *errhp);          /* OCI error handle */

/* Granule handle client callback prototypes for external tables.
 * The granulesPerSrc vector is allocated with numsrc_kpxg entries.
 * The access drivers granule info method (if present) should populate
 * each entry of the granulesPerSrc vector with the number of granules
 * for the corresponding source number.
 */
typedef void (*OCIXADMethodGranuleInfo)(OCIXADGranule *xadgran,
                                        OCIExtProcContext *withCtx,
                                        ub4 *granulesPerSrc);

/*------------------------------ OCIXADSession ------------------------------*/
               /*----- Defines for OCIXADSession Attributes -----*/
#define OCI_ATTR_XADSESSION_TABLE                 1
#define OCI_ATTR_XADSESSION_LOCATIONS             2
#define OCI_ATTR_XADSESSION_NAMES                 2 /* synonym for locations */
#define OCI_ATTR_XADSESSION_DIRECTORIES           3
#define OCI_ATTR_XADSESSION_GRANULE               4
#define OCI_ATTR_XADSESSION_OPCODE                5
#define OCI_ATTR_XADSESSION_CALLERID              6
#define OCI_ATTR_XADSESSION_GRANULESIZE           7
#define OCI_ATTR_XADSESSION_DATAMODE              8
#define OCI_ATTR_XADSESSION_AGENT_NUMBER          9
#define OCI_ATTR_XADSESSION_OPAQUECTX            10

      /*----- OCIXADSession virtual methods (methods as attributes) -----*/
#define OCI_ATTR_XADSESSION_METHOD_ENTRY_POINTS  11
                                                        /* type for GET, SET */
                              /* (OCIXADMethodEntry **, OCIXADMethodEntry *) */

      /* more OCIXADSession Attributes (numbering starts after entry points) */
                                                        /* type for GET, SET */
#define OCI_ATTR_XADSESSION_DEFAULT_DIRECTORY    12     /* (text **, text *) */
#define OCI_ATTR_XADSESSION_DRIVER_TYPE          13     /* (text **, text *) */
#define OCI_ATTR_XADSESSION_SAMPLE_TYPE          14     /* (ub4 *,   ub4 *)  */
#define OCI_ATTR_XADSESSION_SAMPLE_PERCENT       15     /* (ub4 *,   ub4 *)  */
#define OCI_ATTR_XADSESSION_ACCESS_PARM_TYPE     16     /* (ub1 *,   ub1 *)  */
#define OCI_ATTR_XADSESSION_DCACHE_SIZE          17     /* (ub4 *,   ub4 *)  */
#define OCI_ATTR_XADSESSION_DCACHE_NUM           18     /* (ub4 *),  n/a  )  */
#define OCI_ATTR_XADSESSION_DCACHE_DISABLE       19     /* (ub1 *),  n/a  )  */
#define OCI_ATTR_XADSESSION_DCACHE_HITS          20     /* (ub4 *),  n/a  )  */
#define OCI_ATTR_XADSESSION_DCACHE_MISSES        21     /* (ub4 *),  n/a  )  */


             /*----- Values for OCI_ATTR_XADSESSION_OPCODE -----*/
#define OCI_XADSESSION_OPCODE_FETCH               1
#define OCI_XADSESSION_OPCODE_POPULATE            2

            /*----- Values for OCI_ATTR_XADSESSION_CALLERID -----*/
#define OCI_XADSESSION_CALLERID_QC                1     /* query coordinator */
#define OCI_XADSESSION_CALLERID_SHADOW            2
#define OCI_XADSESSION_CALLERID_SLAVE             3

            /*----- Values for OCI_ATTR_XADSESSION_DATAMODE -----*/
#define OCI_XADSESSION_DATAMODE_STREAM            1
#define OCI_XADSESSION_DATAMODE_FIELD             2

            /*----- Values for OCI_ATTR_XADSESSION_SAMPLE_TYPE -----*/
#define OCI_XADSESSION_SAMPLE_NONE                0          /* not sampling */
#define OCI_XADSESSION_SAMPLE_ROW                 1    /* row level sampling */
#define OCI_XADSESSION_SAMPLE_BLOCK               2  /* block level sampling */

           /*----- Values for OCI_ATTR_XADSESSION_ACCESS_PARM_TYPE -----*/
#define OCI_XADSESSION_ACCESS_PARM_TYPE_CLOB      1    /* accessParm is CLOB */
#define OCI_XADSESSION_ACCESS_PARM_TYPE_BLOB      2    /* accessParm is BLOB */

/*------------------------------- OCIXADTable -------------------------------*/
                /*----- Defines for OCIXADTable Attributes -----*/
#define OCI_ATTR_XADTABLE_SESSION                 1
#define OCI_ATTR_XADTABLE_NAME                    2
#define OCI_ATTR_XADTABLE_COLUMNS                 3
#define OCI_ATTR_XADTABLE_REF_COLUMNS             4
#define OCI_ATTR_XADTABLE_FIELDS                  5
#define OCI_ATTR_XADTABLE_NUM_FIELDS              6
#define OCI_ATTR_XADTABLE_OPAQUECTX               7
#define OCI_ATTR_XADTABLE_NUM_COLS                8
#define OCI_ATTR_XADTABLE_NUM_REF_COLS            9
#define OCI_ATTR_XADTABLE_SCHEMA                 10

/*------------------------------- OCIXADField -------------------------------*/
                /*----- Defines for OCIXADField Attributes -----*/
#define OCI_ATTR_XADFIELD_COLUMN_NUM              1
#define OCI_ATTR_XADFIELD_DATA_TYPE               2
#define OCI_ATTR_XADFIELD_PRECISION               3
#define OCI_ATTR_XADFIELD_SCALE                   4
#define OCI_ATTR_XADFIELD_CHARSET_ID              5
#define OCI_ATTR_XADFIELD_FORMAT_MASK             6
#define OCI_ATTR_XADFIELD_ADDR                    7
#define OCI_ATTR_XADFIELD_LENGTH                  8
#define OCI_ATTR_XADFIELD_ISNULL                  9
#define OCI_ATTR_XADFIELD_ISPARTIAL              10
#define OCI_ATTR_XADFIELD_OPAQUECTX              11

/*------------------------------ OCIXADGranule ------------------------------*/
               /*----- Defines for OCIXADGranule Attributes -----*/
#define OCI_ATTR_XADGRANULE_NUM_GRANULES          1
#define OCI_ATTR_XADGRANULE_NUM_SRC               2
#define OCI_ATTR_XADGRANULE_INTRA_SRC_CONCURRENCY 3
#define OCI_ATTR_XADGRANULE_OPAQUECTX             4

      /*----- OCIXADGranule virtual methods (methods as attributes) -----*/
#define OCI_ATTR_XADGRANULE_INFO_METHOD           5


/*---------------------------------------------------------------------------
                     PRIVATE TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/
/* NONE */

/*---------------------------------------------------------------------------
                           EXPORT FUNCTIONS
  ---------------------------------------------------------------------------*/

/*-------------------------- OCIXADSession Methods --------------------------*/
/* NONE */

/*--------------------------- OCIXADTable Methods ---------------------------*/
/*
  NAME
    OCIXADTableSetRowValues

  DESCRIPTION
    To set the individual column values for a row by iterating through the
    OCIXADField handle vector which is attached to the passed in OCIXADTable
    handle.  The OCIXADTableSetRowValues method is called once for complete
    rows, multiple times for pieced rows.
    The column array row index is returned as an OUT parameter (*rowIdxp).
    
  RETURNS
    OCI_SUCCESS:  all column array entries set.
    OCI_CONTINUE: a partial Field was encountered.
    OCI_ERROR:    a partial field encountered for a column which
                  does not allow partial fields (partials are only allowed
                  for LOB and LONG columns).
  NOTES
 */
sword
#if defined(__STDC__) || defined(__cplusplus)
OCIXADTableSetRowValues(OCIXADTable *tblhp, ub4 *rowIdxp,OCIError *errhp);
#else
OCIXADTableSetRowValues(/*_ OCIXADTable *tblhp, ub4 *rowIdxp ,
                            OCIError *errhp _*/);
#endif


/*--------------------------- OCIXADField Methods ---------------------------*/
/*
  NAME
    OCIXADFieldSet

  DESCRIPTION
    Sets run-time (data dependent) attributes of the OCIXADField handle.
    
  RETURNS
    OCI_SUCCESS:  Field attributes successfully set.
    OCI_ERROR:    Invalid combination of flags,
                  or NULL addr value for a non-NULL field,
                  or length of zero for a non-NULL field,
  NOTES
    Basically for convenience and efficiency, so the application does
    not have to do four OCIAttrSet calls to set these attributes.
 */
sword
#if defined(__STDC__) || defined(__cplusplus)
OCIXADFieldSet(OCIXADField *fldhp, ub1 *addr, ub4 length, ub1 isnull,
               ub1 ispartial);
#else
OCIXADFieldSet(/*_ OCIXADField *fldhp, ub1 *addr, ub4 length, ub1 isnull,
                   ub1 ispartial _*/);
#endif

/*-------------------------- OCIXADGranule Methods --------------------------*/
/* NONE */


/*---------------------------------------------------------------------------
                          INTERNAL FUNCTIONS
  ---------------------------------------------------------------------------*/
/* NONE */


#endif                                                      /* OCIXAD_ORACLE */
