/*
 * $Header: odci.h 07-sep-2001.10:38:44 yhu Exp $
 */

/* Copyright (c) 1998, 2001, Oracle Corporation.  All rights reserved.  */
 
/* 
   NAME 
     odci.h - Oracle Data Cartridge Interface definitions

   DESCRIPTION 
     This file contains Oracle Data Cartridge Interface definitions. These
     include the ODCI Types and Constants.

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
     None.

   PRIVATE FUNCTION(S)
     None.

   EXAMPLES

   NOTES
     - The constants defined here are replica of the constants defined 
       in ODCIConst Package defined as part of catodci.sql. If you change
       these do make the similar change in catodci.sql.

   MODIFIED   (MM/DD/YY)
   yhu         07/20/01 - add parallel degree in ODCIIndexInfo.
   abrumm      02/20/01 - ODCIExtTableInfo: add AccessParmBlob attribute
   abrumm      01/18/01 - ODCIExtTableInfo: add default directory
   spsundar    08/24/00 - Update attrbiute positions
   abrumm      08/04/00 - external tables changes: ODCIExtTableInfo, constants
   tchorma     09/11/00 - Add return code ODCI_FATAL
   tchorma     08/08/00 - Add Update Block References Option for Alter Index
   ayoaz       08/01/00 - Add ODCI_AGGREGATE_REUSE_CTX
   spsundar    06/19/00 - add ODCIEnv type
   abrumm      06/27/00 - add defines for ODCIExtTable flags
   abrumm      06/04/00 - external tables: ODCIExtTableInfo change; add ODCIEnv
   ddas        04/28/00 - extensible optimizer enhancements for 8.2
   yhu         06/05/00 - add a bit in IndexInfoFlags for trans. tblspc
   yhu         04/10/00 - add ODCIPartInfo & remove ODCIIndexPartList
   abrumm      03/29/00 - external table support
   spsundar    02/14/00 - update odci definitions for 8.2
   nagarwal    03/07/99 - bug# 838308 - set estimate_stats=1
   rmurthy     11/09/98 - add blocking flag
   ddas        10/31/98 - add ODCI_QUERY_SORT_ASC and ODCI_QUERY_SORT_DESC
   ddas        05/26/98 - fix ODCIPredInfo flag bits
   rmurthy     06/03/98 - add macro for RegularCall
   spsundar    05/08/98 - add constants related to ODCIIndexAlter options
   rmurthy     04/30/98 - remove include s.h
   rmurthy     04/20/98 - name fixes
   rmurthy     04/13/98 - add C mappings for odci types
   alsrivas    04/10/98 - adding defines for ODCI_INDEX1
   jsriniva    04/04/98 - Creation

*/

#ifndef OCI_ORACLE
# include <oci.h>
#endif
#ifndef ODCI_ORACLE
# define ODCI_ORACLE

/*---------------------------------------------------------------------------*/
/*                         SHORT NAMES SUPPORT SECTION                       */
/*---------------------------------------------------------------------------*/

#ifdef SLSHORTNAME

/* The following are short names that are only supported on IBM mainframes
 *   with the SLSHORTNAME defined.
 * With this all subsequent long names will actually be substituted with
 *  the short names here
 */

#define ODCIColInfo_ref             odcicir
#define ODCIColInfoList             odcicil
#define ODCIIndexInfo_ref           odciiir
#define ODCIPredInfo_ref            odcipir
#define ODCIRidList                 odcirl
#define ODCIIndexCtx_ref            odciicr
#define ODCIObject_ref              odcior
#define ODCIObjectList              odciol
#define ODCIQueryInfo_ref           odciqir
#define ODCIFuncInfo_ref            odcifir
#define ODCICost_ref                odcicr
#define ODCIArgDesc_ref             odciadr
#define ODCIArgDescList             odciadl
#define ODCIStatsOptions_ref        odcisor
#define ODCIColInfo                 odcici
#define ODCIColInfo_ind             odcicii
#define ODCIIndexInfo               odciii
#define ODCIIndexInfo_ind           odciiii
#define ODCIPredInfo                odcipi
#define ODCIPredInfo_ind            odcipii
#define ODCIIndexCtx                odciic
#define ODCIIndexCtx_ind            odciici
#define ODCIObject                  odcio
#define ODCIObject_ind              odcioi
#define ODCIQueryInfo               odciqi
#define ODCIQueryInfo_ind           odciqii
#define ODCIFuncInfo                odcifi
#define ODCIFuncInfo_infd           odcifii
#define ODCICost                    odcic
#define ODCICost_ind                odcici
#define ODCIArgDesc                 odciad
#define ODCIArgDesc_ind             odciadi
#define ODCIStatsOptions            odciso
#define ODCIStatsOptions_ind        odcisoi
#define ODCIPartInfo                odcipti
#define ODCIPartInfo_ind            odciptii
#define ODCIPartInfo_ref            odciptir
#define ODCIExtTableInfo            odcixt
#define ODCIExtTableInfo_ind        odcixti
#define ODCIExtTableInfo_ref        odcixtr
#define ODCIExtTableQCInfo          odcixq
#define ODCIExtTableQCInfo_ind      odcixqi
#define ODCIExtTableQCInfo_ref      odcixqr

#endif                                                        /* SLSHORTNAME */

/*---------------------------------------------------------------------------
                     PUBLIC TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/

/* Constants for Return Status */
#define ODCI_SUCCESS             0
#define ODCI_ERROR               1
#define ODCI_WARNING             2
#define ODCI_ERROR_CONTINUE      3
#define ODCI_FATAL               4

/* Constants for ODCIPredInfo.Flags */
#define ODCI_PRED_EXACT_MATCH    0x0001
#define ODCI_PRED_PREFIX_MATCH   0x0002
#define ODCI_PRED_INCLUDE_START  0x0004
#define ODCI_PRED_INCLUDE_STOP   0x0008
#define ODCI_PRED_OBJECT_FUNC    0x0010
#define ODCI_PRED_OBJECT_PKG     0x0020
#define ODCI_PRED_OBJECT_TYPE    0x0040
#define ODCI_PRED_MULTI_TABLE    0x0080

/* Constants for QueryInfo.Flags */
#define ODCI_QUERY_FIRST_ROWS    0x01
#define ODCI_QUERY_ALL_ROWS      0x02
#define ODCI_QUERY_SORT_ASC      0x04
#define ODCI_QUERY_SORT_DESC     0x08
#define ODCI_QUERY_BLOCKING      0x10

/* Constants for ScnFlg(Func /w Index Context) */
#define ODCI_CLEANUP_CALL        1
#define ODCI_REGULAR_CALL        2

/* Constants for ODCIFuncInfo.Flags */
#define ODCI_OBJECT_FUNC         0x01
#define ODCI_OBJECT_PKG          0x02
#define ODCI_OBJECT_TYPE         0x04

/* Constants for ODCIArgDesc.ArgType */
#define ODCI_ARG_OTHER           1
#define ODCI_ARG_COL             2                                 /* column */
#define ODCI_ARG_LIT             3                                /* literal */
#define ODCI_ARG_ATTR            4                       /* object attribute */
#define ODCI_ARG_NULL            5

/* Constants for ODCIStatsOptions.Options */
#define ODCI_PERCENT_OPTION      1
#define ODCI_ROW_OPTION          2

/* Constants for ODCIStatsOptions.Flags */
#define ODCI_ESTIMATE_STATS     0x01
#define ODCI_COMPUTE_STATS      0x02
#define ODCI_VALIDATE           0x04

/* Constants for ODCIIndexAlter parameter alter_option */
#define ODCI_ALTIDX_NONE               0
#define ODCI_ALTIDX_RENAME             1
#define ODCI_ALTIDX_REBUILD            2
#define ODCI_ALTIDX_REBUILD_ONL        3
#define ODCI_ALTIDX_MODIFY_COL         4
#define ODCI_ALTIDX_UPDATE_BLOCK_REFS  5

/* Constants for ODCIIndexInfo.IndexInfoFlags */
#define ODCI_INDEX_LOCAL         0x0001
#define ODCI_INDEX_RANGE_PARTN   0x0002
#define ODCI_INDEX_HASH_PARTN    0x0004
#define ODCI_INDEX_ONLINE        0x0008
#define ODCI_INDEX_PARALLEL      0x0010
#define ODCI_INDEX_UNUSABLE      0x0020
#define ODCI_INDEX_ONIOT         0x0040
#define ODCI_INDEX_TRANS_TBLSPC  0x0080
#define ODCI_INDEX_FUNCTION_IDX  0x0100

/* Constants for ODCIIndexInfo.IndexParaDegree */
#define ODCI_INDEX_DEFAULT_DEGREE 32767

/* Constants for ODCIEnv.CallProperty */
#define ODCI_CALL_NONE           0
#define ODCI_CALL_FIRST          1
#define ODCI_CALL_INTERMEDIATE   2
#define ODCI_CALL_FINAL          3

/* Constants for ODCIExtTableInfo.OpCode */
#define ODCI_EXTTABLE_INFO_OPCODE_FETCH    1
#define ODCI_EXTTABLE_INFO_OPCODE_POPULATE 2

/* Constants (bit definitions) for ODCIExtTableInfo.Flag */
    /* sampling type: row or block */
#define ODCI_EXTTABLE_INFO_FLAG_SAMPLE           0x00000001
#define ODCI_EXTTABLE_INFO_FLAG_SAMPLE_BLOCK     0x00000002
    /* AccessParmClob, AccessParmBlob discriminator */
#define ODCI_EXTTABLE_INFO_FLAG_ACCESS_PARM_CLOB 0x00000004
#define ODCI_EXTTABLE_INFO_FLAG_ACCESS_PARM_BLOB 0x00000008

/* Constants for ODCIExtTableInfo.IntraSourceConcurrency */
#define ODCI_TRUE  1
#define ODCI_FALSE 0

/* Constants (bit definitions) for ODCIExtTable{Open,Fetch,Populate,Close}
 * Flag argument.
 */
#define ODCI_EXTTABLE_OPEN_FLAGS_QC     0x00000001  /* caller is Query Coord */
#define ODCI_EXTTABLE_OPEN_FLAGS_SHADOW 0x00000002  /* caller is shadow proc */
#define ODCI_EXTTABLE_OPEN_FLAGS_SLAVE  0x00000004  /* caller is slave  proc */

#define ODCI_EXTTABLE_FETCH_FLAGS_EOS   0x00000001 /* end-of-stream on fetch */

/* Constants for Flags argument to ODCIAggregateTerminate */
#define ODCI_AGGREGATE_REUSE_CTX  1

/*---------------------------------------------------------------------------
                     ODCI TYPES
  ---------------------------------------------------------------------------*/
/*
 * These are C mappings for the OTS types defined in catodci.sql
 */

typedef OCIRef   ODCIColInfo_ref;
typedef OCIArray ODCIColInfoList;
typedef OCIRef   ODCIIndexInfo_ref;
typedef OCIRef   ODCIPredInfo_ref;
typedef OCIArray ODCIRidList;
typedef OCIRef   ODCIIndexCtx_ref;
typedef OCIRef   ODCIObject_ref;
typedef OCIArray ODCIObjectList;
typedef OCIRef   ODCIQueryInfo_ref;
typedef OCIRef   ODCIFuncInfo_ref;
typedef OCIRef   ODCICost_ref;
typedef OCIRef   ODCIArgDesc_ref;
typedef OCIArray ODCIArgDescList;
typedef OCIRef   ODCIStatsOptions_ref;
typedef OCIRef ODCIPartInfo_ref;
typedef OCIRef   ODCIEnv_ref;
typedef OCIRef   ODCIExtTableInfo_ref;             /* external table support */
typedef OCIArray ODCIGranuleList;                  /* external table support */
typedef OCIRef   ODCIExtTableQCInfo_ref;           /* external table support */
 
struct ODCIColInfo
{
   OCIString* TableSchema;
   OCIString* TableName;
   OCIString* ColName;
   OCIString* ColTypName;
   OCIString* ColTypSchema;
   OCIString* TablePartition;
};
typedef struct ODCIColInfo ODCIColInfo;
 
struct ODCIColInfo_ind
{
   OCIInd atomic;
   OCIInd TableSchema;
   OCIInd TableName;
   OCIInd ColName;
   OCIInd ColTypName;
   OCIInd ColTypSchema;
   OCIInd TablePartition;
};
typedef struct ODCIColInfo_ind ODCIColInfo_ind;
 
struct ODCIIndexInfo
{
   OCIString*       IndexSchema;
   OCIString*       IndexName;
   ODCIColInfoList* IndexCols;
   OCIString*       IndexPartition;
   OCINumber        IndexInfoFlags;
   OCINumber        IndexParaDegree;
};
typedef struct ODCIIndexInfo ODCIIndexInfo;
 
struct ODCIIndexInfo_ind
{
   OCIInd atomic;
   OCIInd IndexSchema;
   OCIInd IndexName;
   OCIInd IndexCols;
   OCIInd IndexPartition;
   OCIInd IndexInfoFlags;
   OCIInd IndexParaDegree;
};
typedef struct ODCIIndexInfo_ind ODCIIndexInfo_ind;
 
struct ODCIPredInfo
{
   OCIString* ObjectSchema;
   OCIString* ObjectName;
   OCIString* MethodName;
   OCINumber  Flags;
};
typedef struct ODCIPredInfo ODCIPredInfo;
 
struct ODCIPredInfo_ind
{
   OCIInd atomic;
   OCIInd ObjectSchema;
   OCIInd ObjectName;
   OCIInd MethodName;
   OCIInd Flags;
};
typedef struct ODCIPredInfo_ind ODCIPredInfo_ind;
 
struct ODCIIndexCtx
{
   struct ODCIIndexInfo IndexInfo;
   OCIString*           Rid;
};
typedef struct ODCIIndexCtx ODCIIndexCtx;
 
struct ODCIIndexCtx_ind
{
   OCIInd                   atomic;
   struct ODCIIndexInfo_ind IndexInfo;
   OCIInd                   Rid;
};
typedef struct ODCIIndexCtx_ind ODCIIndexCtx_ind;
 
struct ODCIObject
{
   OCIString* ObjectSchema;
   OCIString* ObjectName;
};
typedef struct ODCIObject ODCIObject;
 
struct ODCIObject_ind
{
   OCIInd atomic;
   OCIInd ObjectSchema;
   OCIInd ObjectName;
};
typedef struct ODCIObject_ind ODCIObject_ind;
 
struct ODCIQueryInfo
{
   OCINumber       Flags;
   ODCIObjectList* AncOps;
};
typedef struct ODCIQueryInfo ODCIQueryInfo;

 
struct ODCIQueryInfo_ind
{
   OCIInd atomic;
   OCIInd Flags;
   OCIInd AncOps;
};
typedef struct ODCIQueryInfo_ind ODCIQueryInfo_ind;
 
struct ODCIFuncInfo
{
   OCIString* ObjectSchema;
   OCIString* ObjectName;
   OCIString* MethodName;
   OCINumber Flags;
};
typedef struct ODCIFuncInfo ODCIFuncInfo;
 
struct ODCIFuncInfo_ind
{
   OCIInd atomic;
   OCIInd ObjectSchema;
   OCIInd ObjectName;
   OCIInd MethodName;
   OCIInd Flags;
};
typedef struct ODCIFuncInfo_ind ODCIFuncInfo_ind;
 
struct ODCICost
{
   OCINumber  CPUcost;
   OCINumber  IOcost;
   OCINumber  NetworkCost;
   OCIString* IndexCostInfo;
};
typedef struct ODCICost ODCICost;
 
struct ODCICost_ind
{
   OCIInd atomic;
   OCIInd CPUcost;
   OCIInd IOcost;
   OCIInd NetworkCost;
   OCIInd IndexCostInfo;
};
typedef struct ODCICost_ind ODCICost_ind;
 
struct ODCIArgDesc
{
   OCINumber  ArgType;
   OCIString* TableName;
   OCIString* TableSchema;
   OCIString* ColName;
   OCIString* TablePartitionLower;
   OCIString* TablePartitionUpper;
};
typedef struct ODCIArgDesc ODCIArgDesc;
 
struct ODCIArgDesc_ind
{
   OCIInd atomic;
   OCIInd ArgType;
   OCIInd TableName;
   OCIInd TableSchema;
   OCIInd ColName;
   OCIInd TablePartitionLower;
   OCIInd TablePartitionUpper;
};
typedef struct ODCIArgDesc_ind ODCIArgDesc_ind;
 
struct ODCIStatsOptions
{
   OCINumber Sample;
   OCINumber Options;
   OCINumber Flags;
};
typedef struct ODCIStatsOptions ODCIStatsOptions;
 
struct ODCIStatsOptions_ind
{
   OCIInd atomic;
   OCIInd Sample;
   OCIInd Options;
   OCIInd Flags;
};
typedef struct ODCIStatsOptions_ind ODCIStatsOptions_ind;

struct ODCIEnv
{
   OCINumber EnvFlags;
   OCINumber CallProperty;
};
typedef struct ODCIEnv ODCIEnv;

struct ODCIEnv_ind
{
   OCIInd _atomic;
   OCIInd EnvFlags;
   OCIInd CallProperty;
};
typedef struct ODCIEnv_ind ODCIEnv_ind;
 
struct ODCIPartInfo
{
   OCIString* TablePartition;
   OCIString* IndexPartition;
};
typedef struct ODCIPartInfo ODCIPartInfo;
 
struct ODCIPartInfo_ind
{
   OCIInd atomic;
   OCIInd TablePartition;
   OCIInd IndexPartition;
};
typedef struct ODCIPartInfo_ind ODCIPartInfo_ind;

/*---------- External Tables ----------*/
struct ODCIExtTableInfo
{
   OCIString*       TableSchema;
   OCIString*       TableName;
   ODCIColInfoList* RefCols;
   OCIClobLocator*  AccessParmClob;
   OCIBlobLocator*  AccessParmBlob;
   ODCIArgDescList* Locations;
   ODCIArgDescList* Directories;
   OCIString*       DefaultDirectory;
   OCIString*       DriverType;
   OCINumber        OpCode;
   OCINumber        AgentNum;
   OCINumber        GranuleSize;
   OCINumber        Flag;
   OCINumber        SamplePercent;
   OCINumber        MaxDoP;
   OCIRaw*          SharedBuf;
};
typedef struct ODCIExtTableInfo ODCIExtTableInfo;

struct ODCIExtTableInfo_ind
{
   OCIInd _atomic;
   OCIInd TableSchema;
   OCIInd TableName;
   OCIInd RefCols;
   OCIInd AccessParmClob;
   OCIInd AccessParmBlob;
   OCIInd Locations;
   OCIInd Directories;
   OCIInd DefaultDirectory;
   OCIInd DriverType;
   OCIInd OpCode;
   OCIInd AgentNum;
   OCIInd GranuleSize;
   OCIInd Flag;
   OCIInd SamplePercent;
   OCIInd MaxDoP;
   OCIInd SharedBuf;
};
typedef struct ODCIExtTableInfo_ind ODCIExtTableInfo_ind;

struct ODCIExtTableQCInfo
{
   OCINumber        NumGranules;
   OCINumber        NumLocations;
   ODCIGranuleList* GranuleInfo;
   OCINumber        IntraSourceConcurrency;
   OCINumber        MaxDoP;
   OCIRaw*          SharedBuf;
};
typedef struct ODCIExtTableQCInfo ODCIExtTableQCInfo;

struct ODCIExtTableQCInfo_ind
{
   OCIInd _atomic;
   OCIInd NumGranules;
   OCIInd NumLocations;
   OCIInd GranuleInfo;
   OCIInd IntraSourceConcurrency;
   OCIInd MaxDoP;
   OCIInd SharedBuf;
};
typedef struct ODCIExtTableQCInfo_ind ODCIExtTableQCInfo_ind;

/*---------------------------------------------------------------------------
                     PRIVATE TYPES AND CONSTANTS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                           PUBLIC FUNCTIONS
  ---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------
                          PRIVATE FUNCTIONS
  ---------------------------------------------------------------------------*/


#endif                                              /* ODCI_ORACLE */
