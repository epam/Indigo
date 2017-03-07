/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.05
 * January 27, 2017
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST.
 * Modifications and additions by IUPAC and the InChI Trust.
 * Some portions of code were developed/changed by external contributors
 * (either contractor or volunteer) which are listed in the file
 * 'External-contributors' included in this distribution.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the
 * International Chemical Identifier (InChI)
 * Copyright (C) IUPAC and InChI Trust Limited
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0,
 * or any later version.
 *
 * Please note that this library is distributed WITHOUT ANY WARRANTIES
 * whatsoever, whether expressed or implied.
 * See the IUPAC/InChI-Trust InChI Licence No.1.0 for more details.
 *
 * You should have received a copy of the IUPAC/InChI Trust InChI
 * Licence No. 1.0 with this library; if not, please write to:
 *
 * The InChI Trust
 * 8 Cavendish Avenue
 * Cambridge CB1 7US
 * UK
 *
 * or e-mail to alan@inchi-trust.org
 *
 */


#ifndef _ICHIDRP_H_
#define _ICHIDRP_H_


#include "incomdef.h"

/********************************************
 * Parameters for the structure drawing
 ********************************************/
#define TDP_LEN_LBL      16  /* length of a label (label: Req., Shown, Found) */
/* #define TDP_NUM_LBL 3  */ /* number of labels */
/* #define TDP_NUM_PAR 3  */ /* number of types per label (types: B/T, I/N, S) */
typedef enum tagTblTypes {itBASIC, itISOTOPIC, itSTEREO, TDP_NUM_PAR} TBL_TYPES; /*  types */
typedef enum tagTblLabels{ ilSHOWN,  TDP_NUM_LBL} TBL_LABELS; /*  labels */
typedef struct tagTblDrawPatms {
    char   ReqShownFoundTxt[TDP_NUM_LBL][TDP_LEN_LBL];
    char   ReqShownFound[TDP_NUM_LBL][TDP_NUM_PAR];
    int    nOrientation;  /* 10*degrees: 0 or 2700 */
    int    bDrawTbl;
} TBL_DRAW_PARMS;
/*********************************************/
typedef struct tagDrawParmsSettings {
    TBL_DRAW_PARMS *tdp;
    unsigned long  ulDisplTime;
    int            bOrigAtom;
    int            nFontSize;
} SET_DRAW_PARMS;  /* input only: how to draw or calculate */
/*********************************************/
typedef struct tagReturnedDrawParms {
    int       bEsc;
} RET_DRAW_PARMS;
/*********************************************/
typedef struct tagPersistDrawParms {
    int rcPict[4];
} PER_DRAW_PARMS; /* saved between displaying different structures */
/*********************************************/
typedef struct tagDrawParms {
    SET_DRAW_PARMS  sdp;   /* how to draw: fill on the 1st call */
    RET_DRAW_PARMS  rdp;   /* returned when drawing window is closed */
    PER_DRAW_PARMS *pdp;   /* persistent: save between calls (window size) */
#ifndef TARGET_LIB_FOR_WINCHI
#ifndef COMPILE_ANSI_ONLY
    AT_NUMB   *nEquLabels; /* num_inp_atoms elements, value>0 marks atoms in the set #value  */
    AT_NUMB    nNumEquSets;  /* max mark value */
    AT_NUMB    nCurEquLabel; /* current mark */
#endif
#endif
} DRAW_PARMS; /* Settings: How to draw the structure */

/* @@@ #endif */ /* } COMPILE_ANSI_ONLY */

#define MAX_NUM_PATHS 4


typedef enum tagInputType
{
        INPUT_NONE=0,
        INPUT_MOLFILE=1,
        INPUT_SDFILE=2,
        INPUT_INCHI_XML=3, /* obsolete */
        INPUT_INCHI_PLAIN=4,
        INPUT_CMLFILE=5, /* obsolete */
        INPUT_INCHI=6,
        INPUT_MAX
}
INPUT_TYPE;

/* bCalcInChIHash values */
typedef enum tagInChIHashCalc
{
    INCHIHASH_NONE=0,
    INCHIHASH_KEY=1,
    INCHIHASH_KEY_XTRA1=2,
    INCHIHASH_KEY_XTRA2=3,
    INCHIHASH_KEY_XTRA1_XTRA2=4
}
INCHI_HASH_CALC;

typedef struct tagInputParms {
    char            szSdfDataHeader[MAX_SDF_HEADER+1];
    char           *pSdfLabel;
    char           *pSdfValue;
    long            lSdfId;
    long            lMolfileNumber;

/* @@@
#if ( defined( TARGET_LIB_FOR_WINCHI ) || defined(TARGET_EXE_STANDALONE) )
#ifndef COMPILE_ANSI_ONLY
*/
    DRAW_PARMS      dp;
    PER_DRAW_PARMS  pdp;
    TBL_DRAW_PARMS  tdp;
/* @@@ #endif
#endif
*/

/*
  -- Files --
  ip->path[0] => Input
  ip->path[1] => Output (INChI)
  ip->path[2] => Log
  ip->path[3] => Problem structures
  ip->path[4] => Errors file (ACD)

*/
    const char     *path[MAX_NUM_PATHS];
    int             num_paths;
    long            first_struct_number;
    long            last_struct_number;
    INPUT_TYPE      nInputType;
    INCHI_MODE      nMode;
    int             bAbcNumbers;
    int             bINChIOutputOptions; /* !(ip->bINChIOutputOptions & INCHI_OUT_PLAIN_TEXT) */
    int             bINChIOutputOptions2; /* v. 1.05 */
    int             bCtPredecessors;
    int             bDisplayEachComponentINChI;
    long            msec_MaxTime;   /* was ulMaxTime; max time to run ProsessOneStructure */
    long            msec_LeftTime;
    long            ulDisplTime; /* not used: max structure or question display time */
    int             bDisplay;
    int             bDisplayIfRestoreWarnings; /* InChI->Struct debug */
    int             bMergeAllInputStructures;
    int             bSaveWarningStructsAsProblem;
    int             bSaveAllGoodStructsAsProblem;
    int             bGetSdfileId;
    int             bGetMolfileNumber;  /* read molfile number from the name line like "Structure #22" */
    int             bCompareComponents; /* see flags CMP_COMPONENTS, etc. */
    int             bDisplayCompositeResults;
    int             bDoNotAddH;
    int             bNoStructLabels;
    int             bChiralFlag;
    int             bAllowEmptyStructure;
    int                bLargeMolecules;    /* v. 1.05 */
    int                bPolymers;            /* v. 1.05 */
    int             bCalcInChIHash;
    int             bFixNonUniformDraw; /* correct non-uniformly drawn oxoanions and amidinium cations. */
    /* */
    INCHI_MODE      bTautFlags;
    INCHI_MODE      bTautFlagsDone;

#if ( READ_INCHI_STRING == 1 )
    int             bReadInChIOptions;
#endif

/* post v.1 features */
#if ( UNDERIVATIZE == 1 )
    int             bUnderivatize;
#endif
#if ( RING2CHAIN == 1 )
    int             bRing2Chain;
#endif
#if ( RING2CHAIN == 1 || UNDERIVATIZE == 1 )
    int             bIngnoreUnchanged;
#endif
} INPUT_PARMS;


#endif /* _ICHIDRP_H_ */
