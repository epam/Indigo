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


/*
    General processing procedures

*/

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <ctype.h>

#include "mode.h"
#include "ichitime.h"
#ifndef COMPILE_ANSI_ONLY
#include <conio.h>
#endif

#include "ichimain.h"
#include "ichi_io.h"
#include "mol_fmt.h"
#include "inchi_api.h"
#include "readinch.h"
#include "ichicant.h"
#ifdef TARGET_LIB_FOR_WINCHI
#include "../../../IChI_lib/src/ichi_lib.h"
#include "inchi_api.h"
#endif
#include "inchi_gui.h"
#include "readinch.h"


extern int DisplayTheWholeStructure( struct tagCANON_GLOBALS *pCG,
                                      struct tagINCHI_CLOCK *ic,
                                      struct tagStructData *sd,
                                      INPUT_PARMS *ip,
                                      char *szTitle,
                                      INCHI_IOSTREAM *inp_file,
                                      INCHI_IOSTREAM *log_file,
                                      ORIG_ATOM_DATA *orig_inp_data,
                                      long num_inp,
                                      int iINChI,
                                      int bShowStruct,
                                      int bINCHI_LIB_Flag );
extern int DisplayStructure( struct tagCANON_GLOBALS *pCG,
                              inp_ATOM *at,
                              int num_at, int num_removed_H,
                              int bAdd_DT_to_num_H,
                              int nNumRemovedProtons,
                              NUM_H *nNumRemovedProtonsIsotopic,
                              int bIsotopic,
                              int j /*bTautomeric*/,
                              INChI **cur_INChI, INChI_Aux **cur_INChI_Aux,
                              int bAbcNumbers,
                              DRAW_PARMS *dp,
                              INCHI_MODE nMode,
                              char *szTitle );

/* Local functions */
static int DoOneStructureEarlyPreprocessing( long num_inp, STRUCT_DATA *sd, INPUT_PARMS *ip,
                                             INCHI_IOSTREAM *inp_file, INCHI_IOSTREAM *log_file,
                                             INCHI_IOSTREAM *out_file, INCHI_IOSTREAM *prb_file,
                                             ORIG_ATOM_DATA *orig_inp_data, ORIG_ATOM_DATA *prep_inp_data );
static int    OrigAtData_SaveMolfile( ORIG_ATOM_DATA *orig_inp_data,
                                    STRUCT_DATA *sd,
                                    INPUT_PARMS *ip,
                                    long num_inp,
                                    INCHI_IOSTREAM *out_file );
ORIG_STRUCT *OrigAtData_StoreNativeInput( CANON_GLOBALS *pCG, int *nRet,
                                          STRUCT_DATA *sd, INPUT_PARMS *ip,
                                          ORIG_ATOM_DATA *orig_inp_data,
                                          ORIG_STRUCT *pOrigStruct);
void prepare_saveopt_bits( unsigned char *save_opt_bits, INPUT_PARMS *ip );
void DisplayOrigAndResultStructuresAndComponents( int nRet, INCHI_CLOCK *ic, CANON_GLOBALS *pCG,
                                                  STRUCT_DATA *sd, INPUT_PARMS *ip, char *szTitle,
                                                  PINChI2 *pINChI[INCHI_NUM],
                                                  PINChI_Aux2 *pINChI_Aux[INCHI_NUM],
                                                  INCHI_IOSTREAM *inp_file, INCHI_IOSTREAM *log_file,
                                                  INCHI_IOSTREAM *out_file,
                                                  ORIG_ATOM_DATA *orig_inp_data, ORIG_ATOM_DATA *prep_inp_data,
                                                  long num_inp, int maxINChI,
                                                  COMP_ATOM_DATA composite_norm_data[INCHI_NUM][TAUT_NUM+1]);
void SaveOkProcessedMolfile( int nRet, STRUCT_DATA *sd, INPUT_PARMS *ip,
                             INCHI_IOSTREAM *prb_file, INCHI_IOSTREAM *inp_file);
static int snapshot_data_structs_passed_to_ProcessOneStructure(
                                            struct tagINCHI_CLOCK *ic, struct tagINCHI_CLOCK *ic2,
                                            struct tagCANON_GLOBALS *CG, struct tagCANON_GLOBALS *CG2,
                                            STRUCT_DATA *sd, STRUCT_DATA *sd2,
                                            INPUT_PARMS *ip, INPUT_PARMS *ip2,
                                            PINChI2 *pINChI2[INCHI_NUM], PINChI2 *pINChI22[INCHI_NUM],
                                            PINChI_Aux2 *pINChI_Aux2[INCHI_NUM], PINChI_Aux2 *pINChI_Aux22[INCHI_NUM],
                                            INCHI_IOSTREAM *inp_file, INCHI_IOSTREAM *inp_file2,
                                            INCHI_IOSTREAM *log_file, INCHI_IOSTREAM *log_file2,
                                            INCHI_IOSTREAM *out_file, INCHI_IOSTREAM *out_file2,
                                            INCHI_IOSTREAM *prb_file, INCHI_IOSTREAM *prb_file2,
                                            ORIG_ATOM_DATA *orig_inp_data, ORIG_ATOM_DATA *orig_inp_data2,
                                            ORIG_ATOM_DATA *prep_inp_data, ORIG_ATOM_DATA *prep_inp_data2,
                                            INCHI_IOSTREAM_STRING *strbuf, INCHI_IOSTREAM_STRING *strbuf2
                                            );


/* Callbacks */
/*  Console user issued CTRL+C etc. */
int (*ConsoleQuit)(void) = NULL;
int (*UserAction)(void)  = NULL;


/**********************************************
 * output " L=V" or " L missing" or ""
 * The fprintf format string must contain %s%s%s%s
 */
const char gsMissing[] = "is missing";
const char gsEmpty[]   = "";
const char gsSpace[]   = " ";
const char gsEqual[]   = "=";


/*
    Process a portion of input data (molecule, InChI string, ...)
    in a relevant way (generate InChI, restore molecule by InChI )
*/
int ProcessOneStructure( INCHI_CLOCK *ic,
                         CANON_GLOBALS *pCG,
                         STRUCT_DATA *sd,
                         INPUT_PARMS *ip,
                         char *szTitle,
                         PINChI2 *pINChI[INCHI_NUM],
                         PINChI_Aux2 *pINChI_Aux[INCHI_NUM],
                         INCHI_IOSTREAM *inp_file,
                         INCHI_IOSTREAM *log_file,
                         INCHI_IOSTREAM *out_file,
                         INCHI_IOSTREAM *prb_file,
                         ORIG_ATOM_DATA *orig_inp_data,
                         ORIG_ATOM_DATA *prep_inp_data,
                         long num_inp,
                         INCHI_IOSTREAM_STRING *strbuf,
                         unsigned char save_opt_bits)
{
    int nRet = 0,
        nRet1, i, k,
        maxINChI=0,
        bSortPrintINChIFlags=0;
    COMP_ATOM_DATA
        composite_norm_data[INCHI_NUM][TAUT_NUM+1];    /*    [0]:non-taut,
                                                        [1]:taut,
                                                        [2]:intermediate taut struct */
    NORM_CANON_FLAGS ncFlags;
    NORM_CANON_FLAGS *pncFlags = &ncFlags;
    ORIG_STRUCT OrigStruct;
    ORIG_STRUCT *pOrigStruct = NULL;
    int err, ret1 = 0;

#if ( RING2CHAIN == 1 || UNDERIVATIZE == 1 )
    int /*ret1=0,*/ret2=0;
#endif

    int is_polymer            =    orig_inp_data &&
                                orig_inp_data->polymer &&
                                orig_inp_data->polymer->n > 0 &&
                                orig_inp_data->polymer->valid;
    int is_polymer2inchi    =    is_polymer &&
                                (ip->nInputType == INPUT_MOLFILE || ip->nInputType == INPUT_SDFILE);


    /*    1. Preliminary work */

    sd->bUserQuitComponent = 0;
    sd->bUserQuitComponentDisplay = 0;
    memset( composite_norm_data, 0, sizeof(composite_norm_data) );
    memset( pncFlags, 0, sizeof(*pncFlags) );

    /* For experimental purposes only */
    ret1 = DoOneStructureEarlyPreprocessing(     num_inp, sd, ip,
                                     inp_file, log_file, out_file, prb_file,
                                     orig_inp_data, prep_inp_data);
    if ( ret1 )
        goto exit_function;

    if ( is_polymer && !is_polymer2inchi )
        /* Do this when restoring polymer structure from InChI */
        OrigAtData_CheckAndMakePolymerPhaseShifts( orig_inp_data->polymer,
                                                    orig_inp_data->at,
                                                    orig_inp_data->num_inp_atoms,
                                                    &orig_inp_data->num_inp_bonds);

    ret1 = OrigAtData_SaveMolfile( orig_inp_data, sd, ip, num_inp, out_file );
    if ( ret1 )
        goto exit_function;

    pOrigStruct = &OrigStruct;
    memset( pOrigStruct, 0, sizeof(*pOrigStruct));

    OrigAtData_StoreNativeInput( pCG, &nRet, sd, ip, orig_inp_data, pOrigStruct );


    /*    2. Create INChI for the whole disconnected or original structure */

    if ( nRet != _IS_FATAL && nRet != _IS_ERROR )
    {
        nRet1 = CreateOneStructureINChI( pCG, ic, sd, ip,
                                         szTitle, pINChI,pINChI_Aux, INCHI_BAS,
                                         inp_file, log_file, out_file, prb_file,
                                         orig_inp_data, prep_inp_data,
                                         composite_norm_data,
                                         num_inp, strbuf, pncFlags );

        nRet = inchi_max(nRet, nRet1);

        if ( is_polymer2inchi )
        {
            int polymer_repr_type =  OrigAtDataPolymer_GetRepresentation( orig_inp_data->polymer );

            if ( polymer_repr_type == POLYMER_REPRESENTATION_STRUCTURE_BASED )
            {
                /* Temporarily copy ptr to polymer data to prep_inp_data */
                prep_inp_data->polymer = orig_inp_data->polymer;


                OrigAtDataPolymer_CollectPhaseShiftBonds( prep_inp_data, /* NB: not orig_inp_data! */
                                                          &(composite_norm_data[INCHI_BAS][TAUT_YES]),
                                                          &err, sd->pStrErrStruct );
                if ( err )
                    ret1 = _IS_ERROR;
                nRet = inchi_max(nRet, ret1);
                prep_inp_data->polymer = NULL;    /* remove temp copied */
            }
        }
    }
    if ( nRet != _IS_FATAL && nRet != _IS_ERROR )
        maxINChI = 1;


    /* 3. Create INChI for the whole metal-reconnected structure */

    if ( nRet != _IS_FATAL && nRet != _IS_ERROR &&
        (sd->bTautFlagsDone[INCHI_BAS] & TG_FLAG_DISCONNECT_COORD_DONE) &&
        (ip->bTautFlags & TG_FLAG_RECONNECT_COORD) )
    {

        nRet1 = CreateOneStructureINChI( pCG, ic, sd, ip,
                                         szTitle, pINChI, pINChI_Aux, INCHI_REC,
                                         inp_file, log_file, out_file, prb_file,
                                         orig_inp_data, prep_inp_data,
                                         composite_norm_data,
                                         num_inp, strbuf,  pncFlags);
        nRet = inchi_max(nRet, nRet1);

        if ( is_polymer2inchi )
        {
            int err, ret1=0;
            /* temporarily copy ptr to polymer data to prep_inp_data */
            prep_inp_data->polymer = orig_inp_data->polymer;

            OrigAtDataPolymer_CollectPhaseShiftBonds( prep_inp_data, /* NB: not orig_inp_data! */
                                                      &(composite_norm_data[INCHI_REC][TAUT_YES]),
                                                      &err, sd->pStrErrStruct );
            if ( err )
                ret1 = _IS_ERROR;
            nRet = inchi_max(nRet, ret1);
            prep_inp_data->polymer = NULL;    /* remove temp copied */
        }
        if ( nRet != _IS_FATAL && nRet != _IS_ERROR )
            maxINChI = 2;
    }

    if ( nRet != _IS_FATAL && nRet != _IS_ERROR )
    {
        if ( (sd->bChiralFlag & FLAG_INP_AT_CHIRAL) &&
             (ip->nMode & REQ_MODE_STEREO) &&
             !(ip->nMode & (REQ_MODE_RELATIVE_STEREO | REQ_MODE_RACEMIC_STEREO)) &&
             !bIsStructChiral( pINChI, sd->num_components ) )
        {
            WarningMessage(sd->pStrErrStruct, "Not chiral");
        }
         if ( !sd->bUserQuitComponent && !sd->bUserQuit )
        {
            nRet1 = TreatCreateINChIWarning( sd, ip, prep_inp_data, num_inp,
                                             inp_file, log_file, out_file, prb_file );
            nRet = inchi_max(nRet, nRet1);
        }
    }


    /*    4. Sort and print INChI for the whole structure */

    prepare_saveopt_bits( &save_opt_bits, ip);
    if ( nRet != _IS_FATAL && nRet != _IS_ERROR )
    {
        nRet1 = SortAndPrintINChI( pCG, out_file, strbuf, log_file, ip,
                                   orig_inp_data, prep_inp_data,
                                   composite_norm_data,
                                   pOrigStruct,
                                   sd->num_components, sd->num_non_taut,  sd->num_taut,
                                   sd->bTautFlags, sd->bTautFlagsDone, pncFlags,
                                   num_inp, pINChI, pINChI_Aux,
                                   &bSortPrintINChIFlags, save_opt_bits);
    }


    /*    5. Post-process */

    DisplayOrigAndResultStructuresAndComponents( nRet,ic, pCG, sd, ip, szTitle,
                                                 pINChI, pINChI_Aux,
                                                 inp_file, log_file, out_file,
                                                 orig_inp_data, prep_inp_data,
                                                 num_inp,maxINChI,
                                                 composite_norm_data );

    SaveOkProcessedMolfile( nRet, sd, ip, prb_file, inp_file );

    /* Cleanup */
    for ( i = 0; i < INCHI_NUM; i ++ )
        for ( k = 0; k < TAUT_NUM+1; k ++ )
            FreeCompAtomData( &composite_norm_data[i][k] );
    FreeOrigStruct( pOrigStruct);

exit_function:
    return nRet;
}


/*
    Early preprocessing stage:
    used if defined REMOVE_ION_PAIRS_ORIG_STRU or UNDERIVATIZE or RING2CHAIN
*/
int DoOneStructureEarlyPreprocessing( long num_inp, STRUCT_DATA *sd, INPUT_PARMS *ip,
                            INCHI_IOSTREAM *inp_file, INCHI_IOSTREAM *log_file,
                            INCHI_IOSTREAM *out_file, INCHI_IOSTREAM *prb_file,
                            ORIG_ATOM_DATA *orig_inp_data, ORIG_ATOM_DATA *prep_inp_data )
{
#if ( REMOVE_ION_PAIRS_ORIG_STRU == 1 )
    fix_odd_things( orig_inp_data->num_inp_atoms, orig_inp_data->at, 0, ip->bFixNonUniformDraw );
#endif
#if ( UNDERIVATIZE == 1 )
    if ( ip->bUnderivatize && 0 > (ret2=underivatize( orig_inp_data )) )
    {
        long num_inp2 = num_inp;
        AddErrorMessage(sd->pStrErrStruct, "Underivatization error");
        sd->nStructReadError =  99;
        sd->nErrorType = _IS_ERROR;
        TreatErrorsInReadTheStructure( sd, ip, LOG_MASK_ALL, inp_file, log_file, out_file, prb_file,
                                    prep_inp_data, &num_inp2 );
        return _IS_ERROR; /* output only if derivatives found */
    }
#endif /* UNDERIVATIZE == 1 */
#if ( RING2CHAIN == 1 )
    if ( ip->bRing2Chain && 0 > (ret1 = Ring2Chain( orig_inp_data )) )
    {
        long num_inp2 = num_inp;
        AddErrorMessage(sd->pStrErrStruct, "Ring to chain error");
        sd->nStructReadError =  99;
        sd->nErrorType = _IS_ERROR;
        nRet = _IS_ERROR;
        TreatErrorsInReadTheStructure( sd, ip,
                                       LOG_MASK_ALL,
                                       inp_file, log_file,
                                       out_file, prb_file,
                                       prep_inp_data, &num_inp2 );
         return _IS_ERROR; /* output only if derivatives found */
    }
#endif /* RING2CHAIN == 1 */
#if ( RING2CHAIN == 1 || UNDERIVATIZE == 1 )  /***** post v.1 feature *****/
    if ( ip->bIngnoreUnchanged && !ret1 && !ret2 )
    {
        return _IS_SKIP; /* output only if derivatives or ring/chain found */
    }
#endif /* RING2CHAIN == 1 || UNDERIVATIZE == 1 */
    return 0;
}


/*
    If requested, save input data to a Molfile instead of creating INChI
    Also used for output in case of combination of options 'InChI2Struct' and 'OutputSDF'
*/
int OrigAtData_SaveMolfile( ORIG_ATOM_DATA *orig_inp_data,
                            STRUCT_DATA *sd,
                            INPUT_PARMS *ip,
                            long num_inp,
                            INCHI_IOSTREAM *out_file )
{
int ret=0;

    if ( ! (ip->bINChIOutputOptions & INCHI_OUT_SDFILE_ONLY)  )
        return _IS_OKAY;
    else
    {
        char szNumber[32];

        sprintf( szNumber, "Structure #%ld", num_inp );
        ret = OrigAtData_WriteToSDfile( orig_inp_data, out_file, szNumber, NULL,
                                         (sd->bChiralFlag&FLAG_INP_AT_CHIRAL) ? 1 : 0,
                                         (ip->bINChIOutputOptions&INCHI_OUT_SDFILE_ATOMS_DT) ? 1 : 0,
                                         ip->pSdfLabel, ip->pSdfValue );
    }
    return ret;
}


/*
    Optionally save native input data as 'OrigStruct' data package
*/
ORIG_STRUCT * OrigAtData_StoreNativeInput(    CANON_GLOBALS *pCG, int *nRet,
                                            STRUCT_DATA *sd, INPUT_PARMS *ip,
                                            ORIG_ATOM_DATA *orig_inp_data,
                                            ORIG_STRUCT *pOrigStruct)
{

    /*    v. 1.05 always create and fill OrigStruc as it may be used to store e.g. polymer info    */
    /*    If normal AuxInfo is requested, create full reversibility information from native inp data
    if ( ip->bINChIOutputOptions & (INCHI_OUT_NO_AUX_INFO | INCHI_OUT_SHORT_AUX_INFO))
        return NULL; */

    if ( OrigStruct_FillOut( pCG, orig_inp_data, pOrigStruct, sd ) )
    {
        AddErrorMessage(sd->pStrErrStruct, "Cannot interpret reversibility information");
        sd->nStructReadError =  99;
        sd->nErrorType = _IS_ERROR;
        *nRet = _IS_ERROR;
    }
    return pOrigStruct;
}


/*
    Prepare SaveOpt bits
*/
void prepare_saveopt_bits( unsigned char *save_opt_bits, INPUT_PARMS *ip )
{
    if ( ip->nInputType != INPUT_INCHI )
    {
        *save_opt_bits = 0;
        if ( ip->bINChIOutputOptions & INCHI_OUT_SAVEOPT )
        {
            if ( 0 != ( ip->bTautFlags & TG_FLAG_RECONNECT_COORD) )
                (*save_opt_bits) |= SAVE_OPT_RECMET;
            if ( 0 != ( ip->nMode & REQ_MODE_BASIC) )
                (*save_opt_bits) |= SAVE_OPT_FIXEDH;
            if ( 0 != ( ip->nMode & REQ_MODE_DIFF_UU_STEREO) )
                (*save_opt_bits) |= SAVE_OPT_SLUUD;
            if ( 0 == (ip->nMode & (REQ_MODE_SB_IGN_ALL_UU | REQ_MODE_SC_IGN_ALL_UU)) )
                (*save_opt_bits) |= SAVE_OPT_SUU;
            if ( 0 != (ip->bTautFlags & TG_FLAG_KETO_ENOL_TAUT) )
                (*save_opt_bits) |= SAVE_OPT_KET;
            if ( 0 != (ip->bTautFlags & TG_FLAG_1_5_TAUT) )
                (*save_opt_bits) |= SAVE_OPT_15T;
            /* Check if /SNon requested and turn OFF stereo bits if so */
            if ( ! (ip->nMode & REQ_MODE_STEREO) )
            {
                (*save_opt_bits) &= ~SAVE_OPT_SUU;
                (*save_opt_bits) &= ~SAVE_OPT_SLUUD;
            }
        }
    }
}


/*
    Display structures/components on screen
*/
void DisplayOrigAndResultStructuresAndComponents( int nRet,
                                                  INCHI_CLOCK *ic,
                                                  CANON_GLOBALS *pCG,
                                                  STRUCT_DATA *sd,
                                                  INPUT_PARMS *ip,
                                                  char *szTitle,
                                                  PINChI2 *pINChI[INCHI_NUM],
                                                  PINChI_Aux2 *pINChI_Aux[INCHI_NUM],
                                                  INCHI_IOSTREAM *inp_file,
                                                  INCHI_IOSTREAM *log_file,
                                                  INCHI_IOSTREAM *out_file,
                                                  ORIG_ATOM_DATA *orig_inp_data,
                                                  ORIG_ATOM_DATA *prep_inp_data,
                                                  long num_inp,
                                                  int maxINChI,
                                                  COMP_ATOM_DATA composite_norm_data[INCHI_NUM][TAUT_NUM+1])
{


    if ( ip->bDisplay )    ip->bDisplayCompositeResults = 1;    /* v. 1.05 */

#ifndef COMPILE_ANSI_ONLY /* { */

    /* Display equivalent components on original or preprocessed structure(s) */
#ifndef TARGET_LIB_FOR_WINCHI
    if ( nRet != _IS_FATAL && nRet != _IS_ERROR && /*ip->bDisplay &&*/
         (ip->bCompareComponents & CMP_COMPONENTS) && !sd->bUserQuit && !sd->bUserQuitComponent )
    {
        int j, ret, ord;
        int bDisplaySaved = ip->bDisplay;
        ORIG_ATOM_DATA *inp_data;
        AT_NUMB         nEquSet;
        for ( ord = -1; ord < INCHI_NUM; ord ++ )
        {
            switch( ord ) {
            case -1:
                j = INCHI_BAS;  /* preprocessed non-tautomeric */
                break;
            case 0:
                j = INCHI_REC;  /* preprocessed tautomeric */
                break;
            case 1:
                j = -1;        /* original input */
                break;
            default:
                continue;
            }
            inp_data   = j < 0? orig_inp_data : prep_inp_data+j;
            if ( inp_data && inp_data->num_inp_atoms && inp_data->at &&
                 inp_data->nEquLabels &&
                 inp_data->nNumEquSets ) {
                for ( nEquSet = 1; nEquSet <= inp_data->nNumEquSets; nEquSet ++ ) {
                    ip->dp.nEquLabels   = inp_data->nEquLabels;
                    ip->dp.nCurEquLabel = nEquSet;
                    ip->dp.nNumEquSets  = inp_data->nNumEquSets;
                    ip->bDisplay = 1; /* force display if it was not requested */
                    ret = DisplayTheWholeStructure( pCG,ic, sd, ip, szTitle,
                                                    inp_file, log_file, inp_data,
                                                    num_inp, j, 1 /*bShowStructure*/, 0 );
                    ip->dp.nEquLabels   = NULL;
                    ip->dp.nCurEquLabel = 0;
                    ip->dp.nNumEquSets  = 0;
                    ip->bDisplay = bDisplaySaved; /* restore display option */
                    if ( ret ) {
                        /* user pressed Esc */
                        goto exit_loop;
                    }
                }
            }
        }
exit_loop:;
    }
#endif


    /* Display composite results and equivalent components on composite results */
    if ( nRet != _IS_FATAL && nRet != _IS_ERROR && /*ip->bDisplay &&*/ ip->bDisplayCompositeResults )
    {
        int iINChI;
        for ( iINChI = 0; iINChI < maxINChI && !sd->bUserQuitComponentDisplay; iINChI ++ )
        {
            DisplayTheWholeCompositeStructure( pCG, ic, ip, sd, num_inp,
                                               iINChI, pINChI[iINChI], pINChI_Aux[iINChI],
                                               orig_inp_data, prep_inp_data, composite_norm_data[iINChI] );
        }
#ifndef TARGET_LIB_FOR_WINCHI
        if( !ip->bDisplay && sd->bUserQuitComponentDisplay )
        {
            sd->bUserQuit = 1;
        }
#endif
    }

#endif /* } COMPILE_ANSI_ONLY */

    return;
}


/*
    Special mode (option /PGO) : extract all good MOLfiles into the problem file;
    do not extract any MOLfile that could not be processed.
*/
void SaveOkProcessedMolfile( int nRet, STRUCT_DATA *sd, INPUT_PARMS *ip,
                                             INCHI_IOSTREAM *prb_file, INCHI_IOSTREAM *inp_file)
{
    if (    ip->bSaveAllGoodStructsAsProblem    &&
            nRet != _IS_FATAL                    &&
            nRet != _IS_ERROR                    &&
            prb_file                            &&
            prb_file->f                            &&
            0L <= sd->fPtrStart                    &&
            sd->fPtrStart < sd->fPtrEnd                )
            MolfileSaveCopy( inp_file, sd->fPtrStart, sd->fPtrEnd, prb_file->f, 0);
    return;
}


/*
    Generate InChI for the whole (may be multi-component) structure
*/
int CreateOneStructureINChI( CANON_GLOBALS *pCG,
                             INCHI_CLOCK *ic,
                             STRUCT_DATA *sd,
                             INPUT_PARMS *ip,
                             char *szTitle,
                             PINChI2 *pINChI2[INCHI_NUM],
                             PINChI_Aux2 *pINChI_Aux2[INCHI_NUM],
                             int iINChI,
                             INCHI_IOSTREAM *inp_file,
                             INCHI_IOSTREAM *log_file,
                             INCHI_IOSTREAM *out_file,
                             INCHI_IOSTREAM *prb_file,
                             ORIG_ATOM_DATA *orig_inp_data,
                             ORIG_ATOM_DATA *prep_inp_data,
                             COMP_ATOM_DATA composite_norm_data2[][TAUT_NUM+1],
                             long num_inp,
                             INCHI_IOSTREAM_STRING *strbuf,
                             NORM_CANON_FLAGS *pncFlags )
{
    int i, j, k, nRet=0, n=0l;
    int err=0;


    PINChI2     *pINChI     = NULL;
    PINChI_Aux2 *pINChI_Aux = NULL;

    INP_ATOM_DATA InpCurAtData;
    INP_ATOM_DATA *inp_cur_data;

    INP_ATOM_DATA InpNormAtData, InpNormTautData;
    INP_ATOM_DATA *inp_norm_data[TAUT_NUM]; /*  = { &InpNormAtData, &InpNormTautData }; */
    ORIG_ATOM_DATA *cur_prep_inp_data = prep_inp_data + iINChI;
    inchiTime      ulTStart;

/* Always create info data structures (but do not display them always )
#ifndef COMPILE_ANSI_ONLY
*/
    int            bShowStructure = 0;
    int            bStructurePreprocessed = 0; /* All changes except disconnection */
    int            bStructureDisconnected = 0;
    int            bAlsoOutputReconnected = 0, bINCHI_LIB_Flag = 0;
    COMP_ATOM_DATA *composite_norm_data = composite_norm_data2[iINChI];
    INP_ATOM_DATA2 *all_inp_norm_data = NULL;
/*#endif*/

/*        Order of actions:

        if ( orig_inp_data is NOT empty AND
             prep_inp_data[0] IS empty ) then do
             in PreprocessOneStructure()        :

            1. copy orig_inp_data --> prep_inp_data[0]
            2. fix odd things in prep_inp_data[0]
            3. if( orig_inp_data->bDisconnectSalts ) then
                  -- disconnect salts in prep_inp_data[0]
            4. move protons to neutralize charges on heteroatoms
            5. if( orig_inp_data->bDisconnectCoord ) then
                  -- copy prep_inp_data[0] --> prep_inp_data[1]
                  -- disconnect metals in prep_inp_data[0]

        iINChI = 0
        =========
        (normal/disconnected layer)

            1. normalize prep_inp_data[0] in inp_norm_data[0,1]
            2. create INChI[ iINChI ] out of inp_norm_data[0,1]


        iINChI = 1 AND orig_inp_data->bDisconnectCoord > 0
        =================================================
        (reconnected layer)

            1. normalize prep_inp_data[1] in inp_norm_data[0,1]
            2. create INChI[ iINChI ] out of inp_norm_data[0,1]

*/

    ip->msec_LeftTime = ip->msec_MaxTime; /* start timeout countdown for each component */

    inp_cur_data     = &InpCurAtData;
    inp_norm_data[TAUT_NON] = &InpNormAtData;
    inp_norm_data[TAUT_YES] = &InpNormTautData;

    memset( inp_cur_data      , 0, sizeof( *inp_cur_data     ) );
    memset( inp_norm_data[TAUT_NON], 0, sizeof( *inp_norm_data[0] ) );
    memset( inp_norm_data[TAUT_YES], 0, sizeof( *inp_norm_data[0] ) );

    {    /*#ifndef COMPILE_ANSI_ONLY*/
    memset( composite_norm_data+TAUT_NON, 0, sizeof( composite_norm_data[0] ) );
    memset( composite_norm_data+TAUT_YES, 0, sizeof( composite_norm_data[0] ) );
    memset( composite_norm_data+TAUT_INI, 0, sizeof( composite_norm_data[0] ) );
    }    /*#endif*/

    if ( ip->bAllowEmptyStructure && !orig_inp_data->at && !orig_inp_data->num_inp_atoms )
    {
        ;
    }
    else if ( !orig_inp_data->at || !orig_inp_data->num_inp_atoms )
    {
        return 0; /* nothing to do */
    }
    if ( iINChI == 1 && orig_inp_data->bDisconnectCoord <= 0 )
    {
        return 0;
    }

   /* m = iINChI; */ /* orig_inp_data index */

    if ( iINChI != INCHI_BAS && iINChI != INCHI_REC )
    {
        AddErrorMessage(sd->pStrErrStruct, "Fatal undetermined program error");
        sd->nStructReadError =  97;
        nRet = sd->nErrorType = _IS_FATAL;
        goto exit_function;
    }

    /*******************************************************************
     *                                                                 *
     *                                                                 *
     *  Whole structure preprocessing: 1st step of the normalization   *
     *                                                                 *
     *  Happen only on the first call to CreateOneStructureINChI()      *
     *                                                                 *
     *                                                                 *
     *******************************************************************/

    if ( (!prep_inp_data->at || !prep_inp_data->num_inp_atoms) &&
         orig_inp_data->num_inp_atoms > 0 )
    {
        /* the structure has not been preprocessed */
        if ( ip->msec_MaxTime )
        {
            InchiTimeGet( &ulTStart );
        }


        PreprocessOneStructure( ic, sd, ip, orig_inp_data, prep_inp_data );

        pncFlags->bTautFlags[iINChI][TAUT_YES] =
                            pncFlags->bTautFlags[iINChI][TAUT_NON] =
                                sd->bTautFlags[INCHI_BAS] | ip->bTautFlags;

        pncFlags->bTautFlagsDone[iINChI][TAUT_YES] =
                            pncFlags->bTautFlagsDone[iINChI][TAUT_NON] =
                                sd->bTautFlagsDone[INCHI_BAS] | ip->bTautFlagsDone;

        {    /*#ifndef COMPILE_ANSI_ONLY*/
        /* in this location the call happens once for each input structure, before preprocessing */
        bStructurePreprocessed = (0 != (sd->bTautFlagsDone[INCHI_BAS] & (
                                        TG_FLAG_MOVE_HPLUS2NEUTR_DONE  |
                                        TG_FLAG_DISCONNECT_SALTS_DONE  |
                                        TG_FLAG_MOVE_POS_CHARGES_DONE  |
                                        TG_FLAG_FIX_ODD_THINGS_DONE    )));

        bStructureDisconnected = (0 != (sd->bTautFlagsDone[INCHI_BAS] & TG_FLAG_DISCONNECT_COORD_DONE));

        bShowStructure = ( bStructurePreprocessed ||
                           bStructureDisconnected ||
                           prep_inp_data[0].num_components > 1);

        /* sd->bTautFlags[] contains output flags
           ip->bTautFlags   contains input flags
        */
        bAlsoOutputReconnected = (sd->bTautFlagsDone[INCHI_BAS] & TG_FLAG_DISCONNECT_COORD_DONE) &&
                                 (ip->bTautFlags               & TG_FLAG_RECONNECT_COORD);
        bINCHI_LIB_Flag = 0;

        /*************** output structures to TARGET_LIB_FOR_WINCHI conditions *********************
         *
         *  Send to TARGET_LIB_FOR_WINCHI:
         *
         *  type                      component  conditions
         *
         *  COMPONENT_ORIGINAL              #0:  (num_components > 1)
         *  COMPONENT_ORIGINAL_PREPROCESSED #0:  (num_components > 1) && (preprocessed)
         *  COMPONENT_ORIGINAL              #1:  (num_components = 1) && (preprocessed)
         *
         *  Flags explanation:
         *        MAIN => iINChI=0,  RECN => iINChI=1 (Reconnected)
         *        ORIG => Original, PREP => Preprocessed
         *
         *  Possible flags:           k
         *
         *  COMP_ORIG_0_MAIN  0x0001  0  COMPONENT_ORIGINAL, bMain, component #0
         *  COMP_ORIG_0_RECN  0x0002  1  COMPONENT_ORIGINAL, bRecn, component #0
         *
         *  COMP_PREP_0_MAIN  0x0004  2  COMPONENT_ORIGINAL_PREPROCESSED, bMain, component #0
         *  COMP_PREP_0_RECN  0x0008  3  COMPONENT_ORIGINAL_PREPROCESSED, bRecn, component #0
         *
         *  COMP_ORIG_1_MAIN  0x0010  4  COMPONENT_ORIGINAL, bMain, component #1
         *  COMP_ORIG_1_RECN  0x0020  5  COMPONENT_ORIGINAL, bRecn, component #1
         *
         *  bReconnected  = k%2     (0 or 1)
         *  nComponent    = k/4     (0 or 1)
         *  bPreprocessed = (k/2)%2 (0 or 1)
         *
         ******************************************************************************/

        /* Original -> Main, component #0, Original */
        if ( prep_inp_data[INCHI_BAS].num_components > 1 )
        {
            bINCHI_LIB_Flag |= COMP_ORIG_0_MAIN;
        }
        else
        /* Original -> Main, component #1, Original */
        if ( prep_inp_data[INCHI_BAS].num_components == 1 && bStructurePreprocessed )
        {
            bINCHI_LIB_Flag |= COMP_ORIG_1_MAIN;
            /* preprocessed will be added when output canonicalization results */
        }

        if ( bAlsoOutputReconnected )
        {
            /* Original -> Reconnected, component #0, Original */
            if ( prep_inp_data[INCHI_REC].num_components > 1 )
            {
                bINCHI_LIB_Flag |= COMP_ORIG_0_RECN;
            }
            else if ( prep_inp_data[INCHI_BAS].num_components == 1 && bStructurePreprocessed )
            {
                /* Original -> Reconnected, component #1, Original */
                bINCHI_LIB_Flag |= COMP_ORIG_1_RECN;
                /* preprocessed will be added when output canonicalization results */
            }
        }
        if ( ip->msec_MaxTime )
        {
            ip->msec_LeftTime -= InchiTimeElapsed( ic, &ulTStart );
        }

        /* display the ORIGINAL, UN-PREPROCESSED structure */

        if ( ip->bDisplay )
        {
            if ( DisplayTheWholeStructure( pCG, ic, sd, ip, szTitle,
                                           inp_file, log_file, orig_inp_data, num_inp,
                                           -1, bShowStructure, bINCHI_LIB_Flag ) )
            {
                goto exit_function;
            }
        }
        } /*#endif */

        switch (sd->nErrorType)
        {
        case _IS_ERROR:
        case _IS_FATAL:
            /* error message */
            nRet = TreatErrorsInReadTheStructure( sd, ip,
                                                  LOG_MASK_ALL,
                                                  inp_file, log_file, out_file, prb_file,
                                                  prep_inp_data, &num_inp );
            goto exit_cycle;
        }
    }
    /* tranfer flags from INChI_Aux to sd */


    { /*#ifndef COMPILE_ANSI_ONLY */ /* { */

    /******************************************/
    /*      Displaying the structures         */
    /*          Only under WIN32              */
    /******************************************/
    if ( /*
         ip->bDisplayCompositeResults &&
        !sd->bUserQuitComponentDisplay && */
        prep_inp_data[iINChI].num_components > 1)
    {
        all_inp_norm_data = (INP_ATOM_DATA2 *)inchi_calloc( prep_inp_data[iINChI].num_components, sizeof(all_inp_norm_data[0]));
    }

    /* Display the input structure AFTER PREPROCESSING */
    switch ( iINChI )
    {
    case INCHI_BAS:
        /*------------ Possibly disconnected structure -------------------*/
        bStructurePreprocessed = 0 != (sd->bTautFlagsDone[iINChI] & (
                                        TG_FLAG_MOVE_HPLUS2NEUTR_DONE  |
                                        TG_FLAG_DISCONNECT_SALTS_DONE  |
                                        TG_FLAG_MOVE_POS_CHARGES_DONE  |
                                        TG_FLAG_MOVE_CHARGE_COORD_DONE |
                                        TG_FLAG_DISCONNECT_COORD_DONE  |
                                        TG_FLAG_FIX_ODD_THINGS_DONE    ));
        bINCHI_LIB_Flag = 0;
        /* Preprocessed/Main -> Main, component #0, Preprocessed */
        if ( prep_inp_data[iINChI].num_components > 1 &&
             bStructurePreprocessed )
        {
            bINCHI_LIB_Flag |= COMP_PREP_0_MAIN;
        }
        bShowStructure = ( bStructurePreprocessed &&
                           prep_inp_data[iINChI].num_components > 1);
        break;

    case INCHI_REC:
        /*------------ Reconnected structure ------------------------------*/
        bAlsoOutputReconnected =
            (sd->bTautFlagsDone[INCHI_BAS] & TG_FLAG_DISCONNECT_COORD_DONE) &&
            (ip->bTautFlags               & TG_FLAG_RECONNECT_COORD);

        if ( !bAlsoOutputReconnected )
        {
            break;
        }

        bStructurePreprocessed = 0 != (sd->bTautFlagsDone[iINChI] & (
                                        TG_FLAG_MOVE_HPLUS2NEUTR_DONE  |
                                        TG_FLAG_DISCONNECT_SALTS_DONE  |
                                        TG_FLAG_MOVE_POS_CHARGES_DONE  |
                                        TG_FLAG_FIX_ODD_THINGS_DONE    ));
        bINCHI_LIB_Flag = 0;
        /* Preprocessed/Reconnected -> Reconnected, component #0, Preprocessed */
        if ( prep_inp_data[iINChI].num_components > 1 && bStructurePreprocessed ) {
            bINCHI_LIB_Flag |= COMP_PREP_0_RECN;
        }
        bShowStructure = ( bStructurePreprocessed &&
                           prep_inp_data[iINChI].num_components > 1 );
        break;

    default:
        bShowStructure = 0;
    }


    if ( ip->bDisplay && prep_inp_data[iINChI].num_inp_atoms > 0 )
    {
        if ( DisplayTheWholeStructure( pCG, ic, sd, ip, szTitle,
                                       inp_file, log_file,
                                       prep_inp_data+iINChI,
                                       num_inp,
                                       iINChI,
                                       bShowStructure,
                                       bINCHI_LIB_Flag ) )
                                                goto exit_function;
    }
    } /* #endif */ /* } ifndef COMPILE_ANSI_ONLY */


    /* allocate pINChI[iINChI] and pINChI_Aux2[iINChI] -- arrays of pointers to INChI and INChI_Aux */
    /* assign values to sd->num_components[]                                                  */
    MYREALLOC2(PINChI2, PINChI_Aux2, pINChI2[iINChI], pINChI_Aux2[iINChI], sd->num_components[iINChI], cur_prep_inp_data->num_components, k);

    if ( k )
    {
        AddErrorMessage(sd->pStrErrStruct, "Cannot allocate output data. Terminating");
        sd->nStructReadError =  99;
        sd->nErrorType = _IS_FATAL;
        goto exit_function;
    }

    pINChI     = pINChI2[iINChI];
    pINChI_Aux = pINChI_Aux2[iINChI];

    /**************************************************************************/
    /*                                                                        */
    /*                                                                        */
    /*   M A I N   C Y C L E:   P R O C E S S    C O M P O N E N T S          */
    /*                                                                        */
    /*                                                                        */
    /*                     O N E   B Y   O N E                                */
    /*                                                                        */
    /*                                                                        */
    /**************************************************************************/

    for ( i = 0, nRet = 0;
            !sd->bUserQuitComponent && i < cur_prep_inp_data->num_components;
                i ++ )
    {
        if ( ip->msec_MaxTime )
        {
            InchiTimeGet( &ulTStart );
        }

#ifndef TARGET_LIB_FOR_WINCHI  /* { */
#if ( bREUSE_INCHI == 1 )

        if ( iINChI == INCHI_REC &&
             /*( !ip->bDisplay &&
               !ip->bDisplayCompositeResults && */
               !(ip->bCompareComponents & CMP_COMPONENTS) ||
               sd->bUserQuitComponentDisplay )
        {
            /* Reconnected structure (06-20-2005: added "&& !ip->bDisplayCompositeResults" to display composite structure) */
            int m = iINChI-1;

            /* Find whether we have already calculated this INChI in basic (disconnected) layer */
            for ( j = n = 0; j < prep_inp_data[m].num_components; j ++ )
            {
                if ( i+1 == prep_inp_data[m].nOldCompNumber[j] &&
                     (pINChI2[m][j][TAUT_NON] || pINChI2[m][j][TAUT_YES]) )
                {
                    /* Yes, we have already done this */
                    if ( !n++ )
                    {
                        memcpy( pINChI    +i, pINChI2    [m]+j, sizeof(pINChI[0]));
                        memcpy( pINChI_Aux+i, pINChI_Aux2[m]+j, sizeof(pINChI_Aux[0]));
                        for ( k = 0; k < TAUT_NUM; k ++ )
                        {
                            if ( pINChI[i][k] )
                            {
                                pINChI[i][k]->nRefCount ++;
                                if ( pINChI[i][k]->nNumberOfAtoms > 0 )
                                {
                                    switch( k )
                                    {
                                    case TAUT_NON:
                                        sd->num_non_taut[iINChI] ++;
                                        break;
                                    case TAUT_YES:
                                        if ( pINChI[i][k]->lenTautomer > 0 )
                                        {
                                            sd->num_taut[iINChI] ++;
                                        }
                                        else
                                        if ( !pINChI[i][TAUT_NON] ||
                                             !pINChI[i][TAUT_NON]->nNumberOfAtoms )
                                        {
                                            sd->num_non_taut[iINChI] ++;
                                        }
                                        break;
                                    }
                                }
                            }
                            if ( pINChI_Aux[i][k] )
                            {
                                pINChI_Aux[i][k]->nRefCount ++;
                            }
                        }
                    }
                }
            }

            if ( n == 1 )
            {
                continue;
            }
            if ( n > 1 )
            {
                /* ith component is equivalent to more than one another component */
                AddErrorMessage(sd->pStrErrStruct, "Cannot distinguish components");
                sd->nStructReadError =  99;
                sd->nErrorType = _IS_ERROR;
                goto exit_function;
            }
        }

#endif
#endif /* } TARGET_LIB_FOR_WINCHI */

        /*****************************************************/
        /*  a) Allocate memory and extract current component */
        /*****************************************************/

        nRet = GetOneComponent( ic, sd, ip,
                                log_file, out_file,
                                inp_cur_data, cur_prep_inp_data,
                                i, num_inp );

        if ( ip->msec_MaxTime )
            ip->msec_LeftTime -= InchiTimeElapsed( ic, &ulTStart );

        switch ( nRet )
        {
        case _IS_ERROR:
        case _IS_FATAL:
            goto exit_cycle;
        }

#if !defined(TARGET_API_LIB) && !defined(COMPILE_ANSI_ONLY)
        /*  console request: Display the component? */
        if ( ip->bDisplay && inp_file->f != stdin )
        {
            if ( user_quit(ic, "Enter=Display Component, Esc=Stop ?", ip->ulDisplTime) )
            {
                sd->bUserQuitComponent = 1;
                break;
            }
        }
#endif

        /*#ifndef COMPILE_ANSI_ONLY  /* { */

        /*  b) Display the extracted original component structure */
        if ( ip->bDisplay && inp_cur_data->at && !sd->bUserQuitComponentDisplay )
        {
            if ( cur_prep_inp_data->num_components == 1 )
            {
                sprintf( szTitle, "%sInput Structure #%ld.%s%s%s%s%s",
                                  bStructurePreprocessed? "Preprocessed ":"",
                                  num_inp, SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue), iINChI? " (Reconnected)":"");
            }
            else
            {
                sprintf( szTitle, "Component #%d of %d, Input Structure #%ld.%s%s%s%s%s",
                                  i+1, cur_prep_inp_data->num_components,
                                  num_inp, SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue), iINChI? " (Reconnected)":"");
            }

#ifndef TARGET_LIB_FOR_WINCHI

            err = DisplayStructure( pCG,
                                    inp_cur_data->at,
                                    inp_cur_data->num_at,
                                    0,
                                    1,
                                    0,
                                    NULL,
                                    1/*isotopic*/,
                                    0/*taut*/,
                                    NULL,
                                    NULL,
                                    ip->bAbcNumbers,
                                    &ip->dp,
                                    ip->nMode,
                                    szTitle );

            sd->bUserQuitComponentDisplay = (err==ESC_KEY);

            if ( !err )
            {
                inchi_fprintf( stderr, "Cannot display the structure\n");
            }

#else

            if(DRAWDATA && DRAWDATA_EXISTS)
            {
                struct DrawData vDrawData;
                int    nType = COMPONENT_ORIGINAL;
                vDrawData.pWindowData = CreateWinData_( pCG,
                                                        inp_cur_data->at,
                                                        inp_cur_data->num_at,
                                                        0,
                                                        1 /* bAdd_DT_to_num_H */,
                                                        0,
                                                        NULL,
                                                        1 /* display isotopic if present */,
                                                        0,
                                                        NULL,
                                                        NULL,
                                                        ip->bAbcNumbers,
                                                        &ip->dp,
                                                        ip->nMode );
                if( vDrawData.pWindowData != NULL )
                {
                    if ( DRAWDATA_EXISTS ( i+1, nType, iINChI ) )
                    {
                        /* i = component number */
                        nType = COMPONENT_ORIGINAL_PREPROCESSED;
                    }
                    vDrawData.nComponent   = i+1;
                    vDrawData.nType        = nType;
                       vDrawData.bReconnected = iINChI; /* 0=>main; 1=>reconnected */
                    vDrawData.szTitle              = inchi__strdup(szTitle);
                    vDrawData.pWindowData->szTitle = inchi__strdup(szTitle);
                    DRAWDATA(&vDrawData);
                }
            }
#endif
        }
        /*#endif */  /* } COMPILE_ANSI_ONLY */


        /*******************************************************************************/
        /*                                                                             */
        /*  N O R M A L I Z A T I O N    a n d     C A N O N I C A L I Z A T I O N     */
        /*                                                                             */
        /*         (both tautomeric and non-tautomeric if requested)                   */
        /*                                                                             */
        /*******************************************************************************/
        /*  c) Create the component's INChI ( copies ip->bTautFlags into sd->bTautFlags)*/
        /*******************************************************************************/

        nRet = CreateOneComponentINChI( pCG, ic, sd, ip,
                                        inp_cur_data, orig_inp_data,
                                        pINChI/*2[iINChI]*/,
                                        pINChI_Aux/*2[iINChI]*/,
                                        iINChI, i, num_inp,
                                        inp_norm_data,
                                        pncFlags,
                                        log_file );



        /*  d) Display one component structure and/or INChI results only if there was no error */

        /* #ifndef COMPILE_ANSI_ONLY */ /* { */
        if ( !nRet )
        {
            /*  output one component INChI to the stdout if requested */
            /*
            if ( ip->bDisplayEachComponentINChI ) {
                int cur_num_non_taut = (pINChI[i][TAUT_NON] && pINChI[i][TAUT_NON]->nNumberOfAtoms>0);
                int cur_num_taut     = (pINChI[i][TAUT_YES] && pINChI[i][TAUT_YES]->nNumberOfAtoms>0);
                if ( ip->bDisplayEachComponentINChI && cur_num_non_taut + cur_num_taut ) {
                    SortAndPrintINChI( pCG, stdout, pStr, nStrLen, NULL,
                                       ip, 1, cur_num_non_taut, cur_num_taut,
                                       num_inp, pINChI+i, pINChI_Aux+i,
                                       save_opt_bits);
                }
            }
            */
            /**************************************************************************
             * display from one up to 4 structure pictures-results for each component *
             * Enable buttons:                                                        *
             * BN (non-tautomeric non-isotopic): inp_norm_data[0]->bExists            *
             * TN (tautomeric non-isotopic):     inp_norm_data[1]->bExists            *
             * BI (non-tautomeric isotopic):     inp_norm_data[0]->bExists &&         *
             *                                   inp_norm_data[0]->bHasIsotopicLayer  *
             * TI (tautomeric isotopic):         inp_norm_data[1]->bExists &&         *
             *                                   inp_norm_data[1]->bHasIsotopicLayer  *
             **************************************************************************/

            int bIsotopic, bTautomeric, bDisplayTaut, bHasIsotopicLayer, bFixedBondsTaut, m_max, m, nNumDisplayedFixedBondTaut=0;

            for ( j = 0;
                    ip->bDisplay &&
                    !sd->bUserQuitComponentDisplay &&
                    j < TAUT_NUM;
                        j ++ )
            {
                if ( inp_norm_data[j]->bExists && !inp_norm_data[j]->bDeleted )
                {
                    bTautomeric = (pINChI[i][j]->lenTautomer > 0);
                        /* same as (inp_norm_data[j]->bTautomeric > 0) */

                    /* If requested tautomeric and no tautmerism found then do not say mobile or fixed H. 2004-10-27 */
                    bDisplayTaut = (!(ip->nMode & REQ_MODE_BASIC) && !bTautomeric)? -1 : bTautomeric;
                    bHasIsotopicLayer = (inp_norm_data[j]->bHasIsotopicLayer > 0);

                    for ( k = 0; k <= bHasIsotopicLayer; k ++ )
                    {
                        bIsotopic = (k > 0);
                        m_max = inp_norm_data[j]->at_fixed_bonds && inp_norm_data[j]->bTautPreprocessed? 1 : 0;
                        for ( m = m_max; 0 <= m; m -- )
                        {
                            bFixedBondsTaut = (m>0);
                            nNumDisplayedFixedBondTaut += bFixedBondsTaut;
                                /* display only one time */

                            /*  Added number of components, added another format for a single component case - DCh */
                            if ( cur_prep_inp_data->num_components > 1 )
                            {
                                sprintf( szTitle, "%s Component #%d of %d, Structure #%ld%s%s.%s%s%s%s%s",
                                              bFixedBondsTaut? "Preprocessed":"Result for",
                                              i+1, cur_prep_inp_data->num_components, num_inp,
                                              bDisplayTaut==1? ", mobile H": bDisplayTaut==0?", fixed H":"",
                                              bIsotopic? ", isotopic":"",
                                              SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue), iINChI? " (Reconnected)":"");
                            }
                            else
                            {
                                sprintf( szTitle, "%s Structure #%ld%s%s.%s%s%s%s%s",
                                              bFixedBondsTaut? "Preprocessed":"Result for",
                                              num_inp,
                                              bDisplayTaut==1? ", mobile H": bDisplayTaut==0?", fixed H":"",
                                              bIsotopic? ", isotopic":"",
                                              SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue), iINChI? " (Reconnected)":"");
                            }

#ifndef TARGET_LIB_FOR_WINCHI
                            if ( bFixedBondsTaut && nNumDisplayedFixedBondTaut != 1 )
                                continue;
                            if ( ip->bDisplay )
                            {
                                if ( bFixedBondsTaut )
                                {
                                    err = DisplayStructure( pCG,
                                                            inp_norm_data[j]->at_fixed_bonds,
                                                            inp_norm_data[j]->num_at,
                                                            inp_norm_data[j]->num_removed_H,
                                                            0 /*bAdd_DT_to_num_H*/,
                                                            inp_norm_data[j]->nNumRemovedProtons,
                                                            inp_norm_data[j]->nNumRemovedProtonsIsotopic,
                                                            bHasIsotopicLayer,
                                                            j,
                                                            NULL,
                                                            NULL,
                                                            ip->bAbcNumbers,
                                                            &ip->dp,
                                                            ip->nMode,
                                                            szTitle );
                                }
                                else
                                {
                                    err = DisplayStructure( pCG,
                                                            inp_norm_data[j]->at,
                                                            inp_norm_data[j]->num_at,
                                                            0,
                                                            0 /*bAdd_DT_to_num_H*/,
                                                            0,
                                                            NULL,
                                                            k,
                                                            j,
                                                            pINChI[i],
                                                            pINChI_Aux[i],
                                                            ip->bAbcNumbers,
                                                            &ip->dp,
                                                            ip->nMode, szTitle );
                                }
                                if ( sd->bUserQuitComponentDisplay = (err==ESC_KEY) )
                                {
                                    break;
                                }
                            }

#else

                            if(DRAWDATA && !bFixedBondsTaut)
                            {
                                struct DrawData vDrawData;
                                vDrawData.pWindowData =
                                        CreateWinData_( pCG,
                                                        inp_norm_data[j]->at,
                                                        inp_norm_data[j]->num_at,
                                                        0,
                                                        0 /* bAdd_DT_to_num_H */,
                                                        0,
                                                        NULL,
                                                        k,
                                                        j,
                                                        pINChI[i],
                                                        pINChI_Aux[i],
                                                        ip->bAbcNumbers,
                                                        &ip->dp,
                                                        ip->nMode );

                                if( vDrawData.pWindowData != NULL )
                                {
                                    int nType;
                                    vDrawData.nComponent = i+1;
                                    if( bTautomeric == 0 )
                                        nType = (bIsotopic == 0) ? COMPONENT_BN
                                                                 : COMPONENT_BI;
                                    else
                                        nType = (bIsotopic == 0) ? COMPONENT_TN
                                                                 : COMPONENT_TI;
                                    vDrawData.nType        = nType;

                                    vDrawData.bReconnected = iINChI; /* 0=>main; 1=>reconnected */
                                    vDrawData.szTitle              = inchi__strdup(szTitle);
                                    vDrawData.pWindowData->szTitle = inchi__strdup(szTitle);
                                    DRAWDATA(&vDrawData);
                                }
                            }
                            else if(DRAWDATA && bFixedBondsTaut)
                            {
                                struct DrawData vDrawData;
                                if ( (ip->bCompareComponents & CMP_COMPONENTS) &&
                                     !(ip->bCompareComponents & CMP_COMPONENTS_NONTAUT) &&
                                     !bIsotopic == !inp_norm_data[j]->bHasIsotopicLayer ) {

                                    vDrawData.pWindowData =
                                         CreateWinData_( pCG,
                                                         inp_norm_data[j]->at_fixed_bonds,
                                                         inp_norm_data[j]->num_at,
                                                         inp_norm_data[j]->num_removed_H,
                                                         0 /* bAdd_DT_to_num_H */,
                                                         inp_norm_data[j]->nNumRemovedProtons,
                                                         inp_norm_data[j]->nNumRemovedProtonsIsotopic,
                                                         k,
                                                         j,
                                                         NULL,
                                                         NULL,
                                                         ip->bAbcNumbers,
                                                         &ip->dp,
                                                         ip->nMode );
                                }
                                else
                                {
                                    continue;
                                }
                                if( vDrawData.pWindowData != NULL )
                                {
                                    vDrawData.nComponent = i+1;
                                    vDrawData.nType        = COMPONENT_ORIGINAL_PREPROCESSED;
                                    vDrawData.bReconnected = iINChI; /* 0=>main; 1=>reconnected */
                                    vDrawData.szTitle              = inchi__strdup(szTitle);
                                    vDrawData.pWindowData->szTitle = inchi__strdup(szTitle);
                                    DRAWDATA(&vDrawData);
                                }
                            }
#endif
                        }
                    }
                }
            }

            /* Save normalized components for composite display */
            if    ( /*ip->bDisplayCompositeResults && */
                 all_inp_norm_data
                )
            {
                for ( j = 0; j < TAUT_NUM; j ++ )
                {
                    if ( inp_norm_data[j]->bExists )
                    {
                        all_inp_norm_data[i][j] = *inp_norm_data[j];
                        memset( inp_norm_data[j], 0, sizeof(*inp_norm_data[0]) );
                    }
                }
            }
        }

        /* #endif */ /* } COMPILE_ANSI_ONLY */


        if ( nRet )
        {
            nRet = TreatErrorsInCreateOneComponentINChI( sd,
                                                         ip,
                                                         cur_prep_inp_data,
                                                         i,
                                                         num_inp,
                                                         inp_file,
                                                         log_file, out_file, prb_file );
            break;
        }
    }
    /**************************************************************************/
    /*                                                                        */
    /*                                                                        */
    /*   E N D   O F   T H E    M A I N   C Y C L E   P R O C E S S I N G     */
    /*                                                                        */
    /*          C O M P O N E N T S    O N E   B Y   O N E                    */
    /*                                                                        */
    /*                                                                        */
    /**************************************************************************/


exit_cycle:
    switch ( nRet )
    {
    case _IS_FATAL:
    case _IS_ERROR:
        break;
    default:

    /* #ifndef COMPILE_ANSI_ONLY *//* { */
        /* composite results picture(s) */
        if ( all_inp_norm_data )
        {
             CreateCompositeNormAtom( composite_norm_data,
                                      all_inp_norm_data,
                                      prep_inp_data[iINChI].num_components );
             /*
             for ( i = 0; i < prep_inp_data[iINChI].num_components; i ++ ) {
                 for ( k = 0; k < TAUT_NUM; k ++ ) {
                    FreeInpAtomData( &all_inp_norm_data[i][k] );
                 }
             }
             inchi_free( all_inp_norm_data );
             all_inp_norm_data = NULL;
             */
        }
        /* #endif */ /* } COMPILE_ANSI_ONLY */

        break;
    }

    /*#ifndef COMPILE_ANSI_ONLY*/ /* { */

        /* avoid memory leaks in case of error */
        if ( all_inp_norm_data )
        {
             for ( i = 0; i < prep_inp_data[iINChI].num_components; i ++ )
             {
                 for ( k = 0; k < TAUT_NUM; k ++ )
                 {
                    FreeInpAtomData( &all_inp_norm_data[i][k] );
                 }
             }
             inchi_free( all_inp_norm_data );
             all_inp_norm_data = NULL;
        }
    /*#endif */ /* } COMPILE_ANSI_ONLY */

    FreeInpAtomData( inp_cur_data     );
    for ( i = 0; i < TAUT_NUM; i ++ )
    {
        FreeInpAtomData( inp_norm_data[i] );
    }


exit_function:
    return nRet;
}


/*
    Generate InChI for one connected component (of possibly multi-component structure)
*/
int CreateOneComponentINChI( CANON_GLOBALS *pCG,
                             INCHI_CLOCK *ic,
                             STRUCT_DATA *sd,
                             INPUT_PARMS *ip,
                             INP_ATOM_DATA *inp_cur_data,
                             ORIG_ATOM_DATA *orig_inp_data,
                             PINChI2 *pINChI,
                             PINChI_Aux2 *pINChI_Aux,
                             int iINChI,
                             int i, long num_inp,
                             INP_ATOM_DATA **inp_norm_data,
                             NORM_CANON_FLAGS *pncFlags,
                             INCHI_IOSTREAM *log_file )
{
    inchiTime     ulTStart, ulTEnd, *pulTEnd = NULL;
    int           k, num_at, ret = 0;
    int           bOrigCoord;
    INCHI_MODE     bTautFlags     = ip->bTautFlags;
    INCHI_MODE     bTautFlagsDone = (ip->bTautFlagsDone | sd->bTautFlagsDone[INCHI_BAS]);
    INChI       *cur_INChI[TAUT_NUM];
    INChI_Aux   *cur_INChI_Aux[TAUT_NUM];
    long          lElapsedTime;

    InchiTimeGet( &ulTStart );
    bOrigCoord =
        !(ip->bINChIOutputOptions & (INCHI_OUT_NO_AUX_INFO | INCHI_OUT_SHORT_AUX_INFO));

    for ( k = 0; k < TAUT_NUM; k ++ )
    {
        cur_INChI[k]      = NULL;
        cur_INChI_Aux[k]  = NULL;
    }

    /*  Allocate memory for non-tautomeric (k=0) and tautomeric (k=1) results */
    for ( k = 0; k < TAUT_NUM; k ++ )
    {
        int nAllocMode = (k==TAUT_YES? REQ_MODE_TAUT:0) |
                         (bTautFlagsDone & ( TG_FLAG_FOUND_ISOTOPIC_H_DONE |
                                             TG_FLAG_FOUND_ISOTOPIC_ATOM_DONE ))
                                                  ? (ip->nMode & REQ_MODE_ISO) :0;

        if ( k==TAUT_NON && (ip->nMode & REQ_MODE_BASIC ) ||
             k==TAUT_YES && (ip->nMode & REQ_MODE_TAUT )     )
        {
            /*  alloc INChI and INChI_Aux */
            cur_INChI[k]     = Alloc_INChI( inp_cur_data->at,
                                            inp_cur_data->num_at,
                                            &inp_cur_data->num_bonds,
                                            &inp_cur_data->num_isotopic,
                                            nAllocMode );
            cur_INChI_Aux[k] = Alloc_INChI_Aux( inp_cur_data->num_at,
                                                inp_cur_data->num_isotopic,
                                                nAllocMode,
                                                bOrigCoord );
            if ( cur_INChI_Aux[k] )
            {
                cur_INChI_Aux[k]->bIsIsotopic = inp_cur_data->num_isotopic;
            }
            /*  alloc memory for the output structure: non-tautomeric and tautomeric (for displaying) */

            CreateInpAtomData( inp_norm_data[k], inp_cur_data->num_at, k );
        }
        else
        {
            FreeInpAtomData( inp_norm_data[k] );
        }
    }

    lElapsedTime = InchiTimeElapsed(  ic, &ulTStart );
    if ( ip->msec_MaxTime )
    {
        ip->msec_LeftTime -= lElapsedTime;
    }
    sd->ulStructTime += lElapsedTime;


/*^^^#if ( !defined( TARGET_LIB_FOR_WINCHI ) && !defined( TARGET_API_LIB ) ) */
#if ( !defined( TARGET_LIB_FOR_WINCHI ) && !defined( TARGET_API_LIB ) && !defined(TARGET_EXE_STANDALONE) )
#endif


    /******************************************************
     *
     *  Get one component canonical numberings, etc.
     *
     ******************************************************/

    /*
     * Create_INChI() return value:
     * num_at <= 0: error code
     * num_at >  0: number of atoms (excluding terminal hydrogen atoms)
     * inp_norm_data[0] => non-tautomeric, inp_norm_data[1] => tautomeric
     */

    InchiTimeGet( &ulTStart );

    if ( ip->msec_MaxTime )
    {
        ulTEnd = ulTStart;
        pulTEnd = &ulTEnd;
        if ( ip->msec_LeftTime > 0 )
        {
            InchiTimeAddMsec( ic, pulTEnd, ip->msec_LeftTime );
        }
    }

    num_at = Create_INChI( pCG, ic, cur_INChI, cur_INChI_Aux,
                           orig_inp_data/* not used */,
                           inp_cur_data->at, inp_norm_data, inp_cur_data->num_at,
                           ip->nMode,
                           ip->bLargeMolecules,
                           ip->bPolymers,
                           &bTautFlags, &bTautFlagsDone,
                           pulTEnd, NULL, sd->pStrErrStruct);

    SetConnectedComponentNumber( inp_cur_data->at,
                                 inp_cur_data->num_at,
                                 i+1 );
                    /*  normalization alters structure component number */

    for ( k = 0; k < TAUT_NUM; k ++ )
    {
        if ( cur_INChI_Aux[k] && cur_INChI_Aux[k]->nNumberOfAtoms > 0 )
        {
            pncFlags->bNormalizationFlags[iINChI][k] |=
                                        cur_INChI_Aux[k]->bNormalizationFlags;
            pncFlags->bTautFlags[iINChI][k]          |=
                                        cur_INChI_Aux[k]->bTautFlags;
            pncFlags->bTautFlagsDone[iINChI][k]      |=
                                        cur_INChI_Aux[k]->bTautFlagsDone;
            pncFlags->nCanonFlags[iINChI][k]         |=
                                        cur_INChI_Aux[k]->nCanonFlags;
        }
    }

    /*  Detect errors */
    if ( num_at < 0 )
    {
        sd->nErrorCode = num_at;
    }
    else if ( num_at == 0 )
    {
        sd->nErrorCode = -1;
    }
    else if ( cur_INChI[TAUT_NON] && cur_INChI[TAUT_NON]->nErrorCode )
    {
        /*  non-tautomeric error */
        sd->nErrorCode = cur_INChI[TAUT_NON]->nErrorCode;
    }
    else if ( cur_INChI[TAUT_YES] && cur_INChI[TAUT_YES]->nErrorCode )
    {
        /*  tautomeric error */
        sd->nErrorCode = cur_INChI[TAUT_YES]->nErrorCode;
    }

#if ( bRELEASE_VERSION == 0 )
    if ( cur_INChI[TAUT_NON] ) sd->bExtract |= cur_INChI[TAUT_NON]->bExtract;
    if ( cur_INChI[TAUT_YES] ) sd->bExtract |= cur_INChI[TAUT_YES]->bExtract;
    if ( (TG_FLAG_TEST_TAUT3_SALTS_DONE & bTautFlagsDone) ) {
        sd->bExtract |= EXTR_TEST_TAUT3_SALTS_DONE;
    }
#endif

    /*  Detect and store stereo warnings */
    if ( !sd->nErrorCode )
    {
        GetProcessingWarningsOneComponentInChI(cur_INChI, inp_norm_data, sd);
    }

    lElapsedTime = InchiTimeElapsed( ic, &ulTStart );
    if ( ip->msec_MaxTime )
    {
        ip->msec_LeftTime -= lElapsedTime;
    }
    sd->ulStructTime += lElapsedTime;

#if !defined(TARGET_API_LIB) && !defined(COMPILE_ANSI_ONLY)
    /*  Display the results */
    if ( ip->bDisplay )
        eat_keyboard_input();
#endif

    /*  a) No matter what happened save the allocated INChI pointers */
    /*  save the INChI of the current component */

    InchiTimeGet( &ulTStart );
    for ( k = 0; k < TAUT_NUM; k ++ )
    {
        pINChI[i][k]     = cur_INChI[k];
        pINChI_Aux[i][k] = cur_INChI_Aux[k];
        cur_INChI[k]     = NULL;
        cur_INChI_Aux[k] = NULL;
    }

    /*  b) Count one component structure and/or INChI results only
           if there was no error
           Set inp_norm_data[j]->num_removed_H = number of removed explicit H
    */

    if ( !sd->nErrorCode )
    {
        /*  find where the current processed structure is located */
        int cur_is_in_non_taut = ( pINChI[i][TAUT_NON] &&
                                   pINChI[i][TAUT_NON]->nNumberOfAtoms>0);
        int cur_is_in_taut     = ( pINChI[i][TAUT_YES] &&
                                   pINChI[i][TAUT_YES]->nNumberOfAtoms>0);

        int cur_is_non_taut = cur_is_in_non_taut && 0 == pINChI[i][TAUT_NON]->lenTautomer ||
                              cur_is_in_taut     && 0 == pINChI[i][TAUT_YES]->lenTautomer;
        int cur_is_taut     = cur_is_in_taut     && 0 <  pINChI[i][TAUT_YES]->lenTautomer;

        /*
        sd->bTautFlags[iINChI]     |= bTautFlags;
        sd->bTautFlagsDone[iINChI] |= bTautFlagsDone;
        */

        if ( cur_is_non_taut + cur_is_taut )
        {
            /*  count tautomeric and non-tautomeric components of the structures */
            int j1 = cur_is_in_non_taut ? TAUT_NON : TAUT_YES;
            int j2 = cur_is_in_taut        ? TAUT_YES : TAUT_NON;
            int j;
            sd->num_non_taut[iINChI] += cur_is_non_taut;
            sd->num_taut[iINChI]     += cur_is_taut;
            for ( j = j1; j <= j2; j ++ )
            {
                int bIsotopic = ( pINChI[i][j]->nNumberOfIsotopicAtoms ||
                                  pINChI[i][j]->nNumberOfIsotopicTGroups ||
                                  pINChI[i][j]->nPossibleLocationsOfIsotopicH && pINChI[i][j]->nPossibleLocationsOfIsotopicH[0]>1);
                if ( j == TAUT_YES )
                {
                    bIsotopic |= (0 < pINChI_Aux[i][j]->nNumRemovedIsotopicH[0] +
                                      pINChI_Aux[i][j]->nNumRemovedIsotopicH[1] +
                                      pINChI_Aux[i][j]->nNumRemovedIsotopicH[2]);
                }

                inp_norm_data[j]->bExists = 1; /*  j=0: non-taut exists, j=1: taut exists */
                inp_norm_data[j]->bHasIsotopicLayer = bIsotopic;
                /*inp_norm_data[j]->num_removed_H = inp_norm_data[j]->num_at - num_at;*/
            }
        }
    }

/*
    return (sd->nErrorCode==CT_OUT_OF_RAM || sd->nErrorCode==CT_USER_QUIT_ERR)? _IS_FATAL :
            sd->nErrorCode? _IS_ERROR : 0;
*/

    if ( sd->nErrorCode==CT_OUT_OF_RAM || sd->nErrorCode==CT_USER_QUIT_ERR )
    {
        ret = _IS_FATAL;
    }
    else if ( sd->nErrorCode )
    {
        ret = _IS_ERROR;
    }

    lElapsedTime = InchiTimeElapsed( ic, &ulTStart );
    if ( ip->msec_MaxTime )
    {
        ip->msec_LeftTime -= lElapsedTime;
    }
    sd->ulStructTime += lElapsedTime;

    return ret;
}


int        ProcessOneStructureEx(    struct tagINCHI_CLOCK *ic,
                                struct tagCANON_GLOBALS *CG,
                                STRUCT_DATA *sd,
                                INPUT_PARMS *ip,
                                char *szTitle,
                                PINChI2 *pINChI2[INCHI_NUM],
                                PINChI_Aux2 *pINChI_Aux2[INCHI_NUM],
                                INCHI_IOSTREAM *inp_file,
                                INCHI_IOSTREAM *log_file,
                                INCHI_IOSTREAM *out_file,
                                INCHI_IOSTREAM *prb_file,
                                ORIG_ATOM_DATA *orig_inp_data,
                                ORIG_ATOM_DATA *prep_inp_data,
                                long num_inp,
                                INCHI_IOSTREAM_STRING *strbuf,
                                unsigned char save_opt_bits)
{
int res;


    int    is_polymer    =    orig_inp_data &&
                        orig_inp_data->polymer &&
                        orig_inp_data->polymer->n > 0 &&
                        orig_inp_data->polymer->valid != 0 &&
                        (ip->nInputType == INPUT_MOLFILE || ip->nInputType == INPUT_SDFILE);

#ifdef TARGET_LIB_FOR_WINCHI
    inchi_ios_reset( out_file );
#endif

    if ( is_polymer )
    {
        /* Determine the kind of polymer units and polymer as a whole */
        res = OrigAtDataPolymer_ParseAndValidate( orig_inp_data, ip->bPolymers, sd->pStrErrStruct );

        if ( res  )
        {
            sd->nErrorCode = res;

            inchi_ios_eprint( log_file,
                              "Error %d (%s) structure #%ld.%s%s%s%s\n",
                              sd->nErrorCode, sd->pStrErrStruct,
                              num_inp, SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue) );
            res = _IS_ERROR;
            orig_inp_data->num_inp_atoms = -1;
            goto exitf;
        }
        OrigAtData_DebugTrace( orig_inp_data );

        /*    Polymer-specific preprocessing

            Analyze and modify canonical CRU if applicable.
            Use the canonical numbers from ready InChI.
        */
        res = OrigAtDataPolymer_CyclizeCloseableUnits( orig_inp_data, sd->pStrErrStruct );

        if ( res  )
        {
            AddErrorMessage(sd->pStrErrStruct, "Error while processing polymer-related input");
            res = _IS_ERROR;
            orig_inp_data->num_inp_atoms = -1;
            goto exitf;
        }
        OrigAtData_DebugTrace( orig_inp_data );
    }

    res = ProcessOneStructure( ic, CG, sd, ip, szTitle,
                               pINChI2, pINChI_Aux2,
                               inp_file, log_file,
                               out_file, prb_file,
                               orig_inp_data, prep_inp_data,
                               num_inp, strbuf,
                               save_opt_bits);


exitf:
    return res;
}
