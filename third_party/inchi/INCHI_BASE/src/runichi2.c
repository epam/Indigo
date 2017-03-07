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
    Reading input data

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
#ifdef TARGET_LIB_FOR_WINCHI
#include "../../../IChI_lib/src/ichi_lib.h"
#include "inchi_api.h"
#else
#include "inchi_gui.h"
#endif
#include "readinch.h"


/*
    Get (the next) one portion of input data of any possible kind
    (Molfile, InChI string, ...) from a sequential input stream
*/
int GetOneStructure( INCHI_CLOCK *ic,
                     STRUCT_DATA *sd,
                     INPUT_PARMS *ip,
                     char *szTitle,
                     INCHI_IOSTREAM *inp_file,
                     INCHI_IOSTREAM *log_file,
                     INCHI_IOSTREAM *out_file,
                     INCHI_IOSTREAM *prb_file,
                     ORIG_ATOM_DATA *orig_inp_data,
                     long *num_inp,
                     STRUCT_FPTRS *struct_fptrs )
{
int nRet, inp_index, out_index, bUseFptr = (NULL != struct_fptrs);


    FreeOrigAtData( orig_inp_data );


    /* added for TARGET_LIB_FOR_WINCHI early EOF detection */

    inp_index = -1;
    out_index = -1;


    if ( struct_fptrs )
    {

        if ( inp_file->f == stdin )
            return _IS_FATAL;

        if ( ip->nInputType == INPUT_CMLFILE )
            bUseFptr = 0;


        /* initially allocate or increase length of struct_fptrs->fptr array */

        if ( !struct_fptrs->fptr ||
              struct_fptrs->len_fptr <= struct_fptrs->cur_fptr+1 )
        {

            INCHI_FPTR *new_fptr = (INCHI_FPTR *)
                                        inchi_calloc( struct_fptrs->len_fptr + ADD_LEN_STRUCT_FPTRS,
                                                      sizeof(new_fptr[0]) );

            if ( new_fptr )
            {
                if ( struct_fptrs->fptr )
                {
                    if ( struct_fptrs->len_fptr )
                        memcpy( new_fptr,
                                struct_fptrs->fptr,
                                struct_fptrs->len_fptr*sizeof(new_fptr[0]));
                    inchi_free( struct_fptrs->fptr );
                }
                else
                {
                    struct_fptrs->len_fptr = 0;
                    struct_fptrs->cur_fptr = 0;
                    struct_fptrs->max_fptr = 0;
                }
                struct_fptrs->len_fptr += ADD_LEN_STRUCT_FPTRS;
                struct_fptrs->fptr = new_fptr;
            }
            else
            {
                return _IS_FATAL;  /* new_fptr allocation error */
            }
        }

        if ( struct_fptrs->fptr[struct_fptrs->cur_fptr] == EOF )
            return _IS_EOF;
        else
        {

            if ( bUseFptr )
            {
                if( fseek( inp_file->f,
                           struct_fptrs->fptr[struct_fptrs->cur_fptr],
                           SEEK_SET) )
                {
                    return _IS_FATAL;
                }
                if ( struct_fptrs->cur_fptr &&
                     struct_fptrs->max_fptr <= struct_fptrs->cur_fptr )
                {
                    return _IS_FATAL;
                }
            }
            else
            {
                inp_index = struct_fptrs->fptr[struct_fptrs->cur_fptr];
                out_index = EOF;
            }
        }

        *num_inp = struct_fptrs->cur_fptr; /* set structure count */
    }



    nRet = ReadTheStructure( ic, sd, ip, inp_file,
                             orig_inp_data,
                             inp_index, &out_index );


    if ( !nRet )
    {
        if ( ip->nInputType == INPUT_INCHI_PLAIN ||
             ip->nInputType == INPUT_MOLFILE     || ip->nInputType == INPUT_SDFILE)
        {
            if ( ip->lMolfileNumber )
                *num_inp = ip->lMolfileNumber;
            else
                *num_inp += 1;
        }
        else
        {
            *num_inp += 1;
        }

        nRet = TreatErrorsInReadTheStructure( sd, ip,
                                              LOG_MASK_ALL, inp_file,
                                              log_file, out_file, prb_file,
                                              orig_inp_data, num_inp );
    }


    /************************************************************/
    /* added for TARGET_LIB_FOR_WINCHI:
       look ahead for end of file detection                        */

    /************************************************************/

    if ( inp_file->type==INCHI_IOSTREAM_TYPE_FILE &&
         inp_file->f &&
         struct_fptrs &&
         struct_fptrs->fptr &&
         struct_fptrs->fptr[struct_fptrs->cur_fptr+1] <= 0 )
    {

        int nRet2=0;
        INCHI_FPTR next_fptr=0;
        STRUCT_DATA sd2;

        if ( nRet != _IS_EOF && nRet != _IS_FATAL )
        {
            if ( inp_file->f == stdin || struct_fptrs->len_fptr <= struct_fptrs->cur_fptr+1 )
            {
                return _IS_FATAL;
            }
            /* get next structure fptr */
            if ( bUseFptr )
            {
                next_fptr = ftell( inp_file->f );
            }
            else
            {
                inp_index = out_index;
                out_index = EOF;
            }


            /* Read the next structure */

            nRet2 = ReadTheStructure( ic,
                                      &sd2, ip, inp_file, NULL,
                                      inp_index, &out_index );

            /* restore fptr to the next structure */
            if ( bUseFptr )
            {
                if ( next_fptr != -1L )
                {
                    fseek( inp_file->f, next_fptr, SEEK_SET);
                }
            }
        }
        else
        {
            /* treat current fatal error as end of file */
            struct_fptrs->fptr[struct_fptrs->cur_fptr] = EOF;
        }

        /* next is end of file or fatal */
        if ( nRet  == _IS_EOF || nRet  == _IS_FATAL ||
             nRet2 == _IS_EOF || nRet2 == _IS_FATAL )
        {
            struct_fptrs->fptr[struct_fptrs->cur_fptr+1] = EOF;
        }
        else
        {
            struct_fptrs->fptr[struct_fptrs->cur_fptr+1] = bUseFptr? sd->fPtrEnd : inp_index;
        }

        /* update struct_fptrs->max_fptr */
        if ( struct_fptrs->max_fptr <= struct_fptrs->cur_fptr+1  )
        {
            struct_fptrs->max_fptr = struct_fptrs->cur_fptr+2;
        }
    }



    switch ( nRet )
    {
    case _IS_EOF:
        *num_inp -= 1;
    case _IS_FATAL:
    case _IS_ERROR:
    case _IS_SKIP:
        goto exit_function;
    }


    /*
    if ( !orig_inp_data->num_dimensions ) {
        WarningMessage(sd->pStrErrStruct, "0D"); */ /* 0D-structure: no coordinates
    }
    */


exit_function:
    return nRet;
}


/*
    Extract one connected component from the input structure
*/
int GetOneComponent( INCHI_CLOCK *ic,
                     STRUCT_DATA *sd,
                     INPUT_PARMS *ip,
                     INCHI_IOSTREAM *log_file,
                     INCHI_IOSTREAM *out_file,
                     INP_ATOM_DATA *inp_cur_data,
                     ORIG_ATOM_DATA *orig_inp_data,
                     int i, long num_inp )
{
inchiTime ulTStart;

    InchiTimeGet( &ulTStart );

    CreateInpAtomData( inp_cur_data, orig_inp_data->nCurAtLen[i], 0 );

    inp_cur_data->num_at = ExtractConnectedComponent( orig_inp_data->at, orig_inp_data->num_inp_atoms, i+1, inp_cur_data->at );

    sd->ulStructTime += InchiTimeElapsed( ic, &ulTStart );


    /*  error processing */

    if ( inp_cur_data->num_at <= 0 || orig_inp_data->nCurAtLen[i] != inp_cur_data->num_at )
    {
        /*  log error message */
        AddErrorMessage(sd->pStrErrStruct, "Cannot extract Component");
        inchi_ios_eprint( log_file, "%s #%d structure #%ld.%s%s%s%s\n", sd->pStrErrStruct, i+1, num_inp, SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue));
        sd->nErrorCode = inp_cur_data->num_at < 0? inp_cur_data->num_at : (orig_inp_data->nCurAtLen[i] != inp_cur_data->num_at)? CT_ATOMCOUNT_ERR : CT_UNKNOWN_ERR;
        /* num_err ++; */
        sd->nErrorType = _IS_ERROR;

#ifdef TARGET_LIB_FOR_WINCHI
        if (( ip->bINChIOutputOptions & INCHI_OUT_WINCHI_WINDOW) &&
            (ip->bINChIOutputOptions & INCHI_OUT_PLAIN_TEXT) )
        {
            sd->nErrorType = ProcessStructError( out_file,
                                                 log_file,
                                                 sd->pStrErrStruct,
                                                 sd->nErrorType,
                                                 num_inp,
                                                 ip );
        }
#endif
    }

    return sd->nErrorType;
}


/* Read input data of any possible kind (Molfile, InChI string, ...)    */
int ReadTheStructure( struct tagINCHI_CLOCK *ic,
                      STRUCT_DATA *sd,
                      INPUT_PARMS *ip,
                      INCHI_IOSTREAM  *inp_file,
                      ORIG_ATOM_DATA *orig_inp_data,
                      /* deprecated, for CML support: */
                      int inp_index,  int *out_index )
{
    inchiTime ulTStart;
    int nRet = 0, nRet2 = 0;

    int bGetOrigCoord = ! (ip->bINChIOutputOptions &
                            (INCHI_OUT_NO_AUX_INFO | INCHI_OUT_SHORT_AUX_INFO));

    INCHI_MODE InpAtomFlags = 0;
    /* NB: reading Molfile may set FLAG_INP_AT_CHIRAL bit */

    int vABParityUnknown = AB_PARITY_UNDF;
        /* vABParityUnknown holds actual value of an internal constant */
        /* signifying unknown parity: either the same as for undefined */
        /* parity (default==standard)  or a specific one (non-std;       */
        /* requested by SLUUD switch).                                   */

    if ( 0 != ( ip->nMode & REQ_MODE_DIFF_UU_STEREO) )
    {
        /* Make labels for unknown and undefined stereo different */
        vABParityUnknown = AB_PARITY_UNKN;
    }


    if ( ip->bLargeMolecules )
        InpAtomFlags |= FLAG_SET_INP_LARGE_ALLOWED;
    if ( ip->bPolymers )
        InpAtomFlags |= FLAG_SET_INP_POLYMERS_RECOGNIZED;


    memset( sd, 0, sizeof(*sd) );


    switch ( ip->nInputType )
    {

    case INPUT_MOLFILE:
    case INPUT_SDFILE:

        /*  Read the original input structure from Molfile */

        if ( orig_inp_data )
        {

            if ( ip->pSdfValue && ip->pSdfValue[0] )
            {
                /* Added 07-29-2003 to avoid inheriting exact value from prev.
                    structure and to make reference to a (bad) structure
                    with unknown ID Value                                     */
                char *p, *q;  /* q shadows prev declaration of const char *q */
                int  n;
                if ( ( p = strrchr( ip->pSdfValue, '+' ) ) &&
                     '[' == *(p-1) &&
                     0 < ( n=strtol(p+1,&q,10) ) &&
                     q[0] &&
                     ']' == q[0] &&
                     !q[1] )
                {
                    sprintf( p+1, "%d]", n+1 );
                }
                else
                {
                    strcat( ip->pSdfValue, " [+1]" );
                }
            }

            InchiTimeGet( &ulTStart );

            if ( inp_file->type==INCHI_IOSTREAM_TYPE_FILE && inp_file->f )
                sd->fPtrStart = (inp_file->f == stdin)? -1 : ftell( inp_file->f );


            nRet2 = CreateOrigInpDataFromMolfile( inp_file,
                                                  orig_inp_data,
                                                  ip->bMergeAllInputStructures,
                                                  bGetOrigCoord,
                                                  ip->bDoNotAddH,
                                                  ip->pSdfLabel,
                                                  ip->pSdfValue,
                                                  &ip->lSdfId,
                                                  &ip->lMolfileNumber,
                                                  &InpAtomFlags,
                                                  &sd->nStructReadError,
                                                  sd->pStrErrStruct );



            if ( !ip->bGetSdfileId || ip->lSdfId == 999999)
                ip->lSdfId = 0;

            if ( !ip->bGetMolfileNumber || ip->lMolfileNumber < 0 )
                ip->lMolfileNumber = 0;

            if ( inp_file->type==INCHI_IOSTREAM_TYPE_FILE && inp_file->f )
                    sd->fPtrEnd = (inp_file->f == stdin)    ? -1  : ftell( inp_file->f );

            sd->ulStructTime+= InchiTimeElapsed( ic, &ulTStart );

#if ( bRELEASE_VERSION == 0 )
            sd->bExtract |= orig_inp_data->bExtract;
#endif

            /* 2004-11-16: added Molfile Chiral Flag Mode */
            /* ********************************************
             * Chiral flags are set in:
             * - RunICHI.c -- ReadTheStructure()
             * - e_IchiMain.c -- main()
             * - inchi_dll.c -- ExtractOneStructure
             **********************************************/

            /* 1. Highest precedence: Chiral Flag set by the user */
            if ( ip->bChiralFlag & FLAG_SET_INP_AT_CHIRAL )
            {
                InpAtomFlags = FLAG_INP_AT_CHIRAL; /* forced by the user */
            }

            else if ( ip->bChiralFlag & FLAG_SET_INP_AT_NONCHIRAL )
            {
                InpAtomFlags = FLAG_INP_AT_NONCHIRAL; /* forced by the user */
            }

            else if ( (InpAtomFlags & FLAG_INP_AT_CHIRAL) && (InpAtomFlags && FLAG_INP_AT_NONCHIRAL) )
            {
                InpAtomFlags &= ~FLAG_INP_AT_NONCHIRAL;
            }

            /* Save requested flags in the AuxInfo */
            sd->bChiralFlag &= ~( FLAG_INP_AT_CHIRAL | FLAG_INP_AT_NONCHIRAL );
            sd->bChiralFlag |= InpAtomFlags & ( FLAG_INP_AT_CHIRAL | FLAG_INP_AT_NONCHIRAL );

            /* Quick fix: modify ip->nMode on the fly */

            /* 2. The user requested both Stereo AND Chiral flag */
            if ( (ip->nMode & REQ_MODE_CHIR_FLG_STEREO) && (ip->nMode & REQ_MODE_STEREO) )
            {
                if ( InpAtomFlags & FLAG_INP_AT_CHIRAL )
                {
                    /* structure has chiral flag or the user said it is chiral */
                    ip->nMode &= ~(REQ_MODE_RELATIVE_STEREO | REQ_MODE_RACEMIC_STEREO);
                    sd->bChiralFlag |= FLAG_INP_AT_CHIRAL; /* write AuxInfo as chiral */
                }
                else
                {
                    ip->nMode &= ~REQ_MODE_RACEMIC_STEREO;
                    ip->nMode |=  REQ_MODE_RELATIVE_STEREO;
                    sd->bChiralFlag |= FLAG_INP_AT_NONCHIRAL; /* write AuxInfo as explicitly not chiral */
                }
            }
        } /* if ( orig_inp_data )  */

        else    /* !orig_inp_data */
        {
            /*  read the next original structure */
            int nStructReadError=0;

            if ( !ip->bMergeAllInputStructures )
            {
                nRet2 = CreateOrigInpDataFromMolfile( inp_file,
                                                      NULL,
                                                      0,
                                                      0,
                                                      0,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      NULL,
                                                      &InpAtomFlags, /*NULL, */
                                                      &nStructReadError,
                                                      NULL );
                if ( nRet2 <= 0 &&
                     10 < nStructReadError && nStructReadError < 20 )
                        return _IS_EOF;
            }
            else
            {
                return _IS_EOF;
            }
        }

        break;



    case INPUT_INCHI_PLAIN:

        /*  Read the original input data  as text (InChI string ) */
        if ( orig_inp_data )
        {
            if ( ip->pSdfValue && ip->pSdfValue[0] )
            {
                /* Added 07-29-2003 to avoid inheriting exact value from prev. structure
                   and to make reference to a (bad) structure with unknown ID Value */
                char *p, *q;
                int  n;
                if ( ( p = strrchr( ip->pSdfValue, '+' )) &&
                     '[' == *(p-1) &&
                     0 < (n=strtol(p+1,&q,10)) &&
                     q[0] &&
                     ']'==q[0] &&
                     !q[1] )
                {
                    sprintf( p+1, "%d]", n+1 );
                }
                else
                {
                    strcat( ip->pSdfValue, " [+1]" );
                }
            }

            InchiTimeGet( &ulTStart );

            if ( inp_file->type==INCHI_IOSTREAM_TYPE_FILE && inp_file->f )
                    sd->fPtrStart = (inp_file->f == stdin) ? -1 : ftell( inp_file->f );


            /*  Read and make internal molecular  data */
            nRet2 = InchiToOrigAtom( inp_file,
                                     orig_inp_data,
                                     ip->bMergeAllInputStructures,
                                     bGetOrigCoord,
                                     ip->bDoNotAddH,
                                     vABParityUnknown,
                                     ip->nInputType,
                                     ip->pSdfLabel,
                                     ip->pSdfValue,
                                     &ip->lMolfileNumber,
                                     &InpAtomFlags,
                                     &sd->nStructReadError,
                                     sd->pStrErrStruct );

            /*if ( !ip->bGetSdfileId || ip->lSdfId == 999999) ip->lSdfId = 0;*/
            if ( inp_file->type==INCHI_IOSTREAM_TYPE_FILE && inp_file->f )
                sd->fPtrEnd = (inp_file->f == stdin)? -1 : ftell( inp_file->f );

            sd->ulStructTime += InchiTimeElapsed( ic, &ulTStart );

#if ( bRELEASE_VERSION == 0 )
            sd->bExtract |= orig_inp_data->bExtract;
#endif

            /* 2004-11-16: added Molfile Chiral Flag Mode */
            if ( ip->bChiralFlag & FLAG_SET_INP_AT_CHIRAL )
            {
                InpAtomFlags = FLAG_INP_AT_CHIRAL; /* forced by the user */
            }
            else if ( ip->bChiralFlag & FLAG_SET_INP_AT_NONCHIRAL )
            {
                InpAtomFlags = FLAG_INP_AT_NONCHIRAL; /* forced by the user */
            }
            else if ( (InpAtomFlags & FLAG_INP_AT_CHIRAL) && (InpAtomFlags && FLAG_INP_AT_NONCHIRAL) )
            {
                InpAtomFlags &= ~FLAG_INP_AT_NONCHIRAL;
            }

            sd->bChiralFlag |= InpAtomFlags; /* copy chiral flag to AuxInfo */

            /* Quick fix: modify ip->nMode on the fly */
            if ( (ip->nMode & REQ_MODE_CHIR_FLG_STEREO) && (ip->nMode & REQ_MODE_STEREO) )
            {
                if ( InpAtomFlags & FLAG_INP_AT_CHIRAL )
                {
                    ip->nMode &= ~(REQ_MODE_RELATIVE_STEREO | REQ_MODE_RACEMIC_STEREO);
                }
                else
                {
                    ip->nMode &= ~REQ_MODE_RACEMIC_STEREO;
                    ip->nMode |=  REQ_MODE_RELATIVE_STEREO;
                }
            }
        }
        else
        {
            /*  Read the next original structure */
            int           nStructReadError=0;
            if ( !ip->bMergeAllInputStructures )
            {
                nRet2 = InchiToOrigAtom( inp_file,
                                         NULL, 0, 0, 0, 0,
                                         ip->nInputType,
                                         NULL, NULL, NULL, NULL,
                                         &nStructReadError,
                                         NULL );
                if ( nRet2 <= 0 && 10 < nStructReadError && nStructReadError < 20 )
                    return _IS_EOF;
            }
            else
            {
                return _IS_EOF;
            }
        }
        break;

    default:
        nRet = _IS_FATAL; /*  wrong file type */
    }

    return nRet;
}


/*
    Interpret and treat input reading errors/warnings
*/
int TreatErrorsInReadTheStructure(    STRUCT_DATA *sd,
                                    INPUT_PARMS *ip,
                                    int nLogMask,
                                    INCHI_IOSTREAM *inp_file,
                                    INCHI_IOSTREAM *log_file,
                                    INCHI_IOSTREAM *out_file,
                                    INCHI_IOSTREAM *prb_file,
                                    ORIG_ATOM_DATA *orig_inp_data,
                                    long *num_inp )
{
int nRet = _IS_OKAY;

    if ( 10 < sd->nStructReadError && sd->nStructReadError < 20 )
    {
        /*  End of file */
        if ( sd->pStrErrStruct[0] )
            inchi_ios_eprint( log_file, "%s inp structure #%ld: End of file.%s%s%s%s    \n", sd->pStrErrStruct, *num_inp, SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue) );

        inchi_ios_eprint( log_file, "End of file detected after structure #%ld.   \n", *num_inp-1 );

        nRet = _IS_EOF;
        goto exit_function; /*  end of file */
    }

    /*(*num_inp) ++;*/

    if ( *num_inp < ip->first_struct_number )
    {
        /*  Skip the structure */

#if ( !defined(TARGET_API_LIB) && !defined(TARGET_EXE_STANDALONE) )
/* #ifndef TARGET_API_LIB */
        if ( log_file->f != stderr )
            inchi_fprintf( stderr, "\rSkipping structure #%ld.%s%s%s%s...", *num_inp, SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue));
#endif

        nRet = sd->nErrorType = _IS_SKIP;
        goto exit_function;
    }


    sd->nErrorType = GetInpStructErrorType( ip, sd->nStructReadError,
                                            sd->pStrErrStruct,
                                            orig_inp_data->num_inp_atoms );


    if ( sd->nErrorType == _IS_FATAL )
    {
        /*  Fatal error */

        if ( nLogMask & LOG_MASK_FATAL )
            inchi_ios_eprint( log_file, "Fatal Error %d (aborted; %s) inp structure #%ld.%s%s%s%s\n",
                    sd->nStructReadError, sd->pStrErrStruct, *num_inp, SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue) );

#if ( bRELEASE_VERSION == 1 || EXTR_FLAGS == 0 )
        if ( prb_file->f && 0L <= sd->fPtrStart && sd->fPtrStart < sd->fPtrEnd && !ip->bSaveAllGoodStructsAsProblem ) {
            MolfileSaveCopy(inp_file, sd->fPtrStart, sd->fPtrEnd, prb_file->f, *num_inp);
        }
#endif
        /* goto exit_function; */
    }

    if ( sd->nErrorType == _IS_ERROR )
    {
        /*  Non-fatal errors: do not produce INChI */

        /*  70 => too many atoms */
        if ( nLogMask & LOG_MASK_ERR )
            inchi_ios_eprint( log_file, "Error %d (no %s; %s) inp structure #%ld.%s%s%s%s\n",
                    sd->nStructReadError, (ip->bINChIOutputOptions & INCHI_OUT_SDFILE_ONLY)?"Molfile":INCHI_NAME,
                    sd->pStrErrStruct, *num_inp, SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue) );

#if ( bRELEASE_VERSION == 1 || EXTR_FLAGS == 0 )
        if ( prb_file->f && 0L <= sd->fPtrStart && sd->fPtrStart < sd->fPtrEnd && !ip->bSaveAllGoodStructsAsProblem) {
            MolfileSaveCopy(inp_file, sd->fPtrStart, sd->fPtrEnd, prb_file->f, *num_inp);
        }
#endif
    }

    if ( sd->nErrorType == _IS_WARNING )
    {
        /*  Warnings: try to produce INChI */

        if ( nLogMask & LOG_MASK_WARN )
            inchi_ios_eprint( log_file, "Warning: (%s) inp structure #%ld.%s%s%s%s\n",
                    sd->pStrErrStruct, *num_inp, SDF_LBL_VAL(ip->pSdfLabel,ip->pSdfValue) );
    }



#ifdef TARGET_LIB_FOR_WINCHI
    if ( (ip->bINChIOutputOptions & INCHI_OUT_WINCHI_WINDOW) &&
         (ip->bINChIOutputOptions & INCHI_OUT_PLAIN_TEXT) )
    {
        if ( sd->nErrorType != _IS_OKAY && sd->nErrorType != _IS_WARNING )
        {
            sd->nErrorType =
                ProcessStructError( out_file, log_file, sd->pStrErrStruct, sd->nErrorType, *num_inp, ip );
        }
    }
#endif

exit_function:
    if ( nRet <= _IS_OKAY && sd->nErrorType > 0 )
        nRet = sd->nErrorType;

    return nRet;
}



#ifdef TARGET_EXE_USING_API

/*

*/
int InchiToInchi_Input( INCHI_IOSTREAM *inp_file,
                        inchi_Input *orig_at_data,
                        int bMergeAllInputStructures,
                        int bDoNotAddH,
                        int vABParityUnknown,
                        INPUT_TYPE nInputType,
                        char *pSdfLabel,
                        char *pSdfValue,
                        long *lSdfId,
                        INCHI_MODE *pInpAtomFlags,
                        int *err,
                        char *pStrErr )
{
/* inp_ATOM       *at = NULL; */
int             num_dimensions_new;
int             num_inp_bonds_new;
int             num_inp_atoms_new;
int             num_inp_0D_new;
inp_ATOM     *at_new     = NULL;
inp_ATOM     *at_old     = NULL;
inchi_Stereo0D *stereo0D_new = NULL;
inchi_Stereo0D *stereo0D_old = NULL;
int             nNumAtoms  = 0, nNumStereo0D = 0;
MOL_COORD      *szCoordNew = NULL;
MOL_COORD      *szCoordOld = NULL;
int            i, j;
int               max_num_at;


    max_num_at = MAX_ATOMS;
    if ( *pInpAtomFlags  & FLAG_SET_INP_LARGE_ALLOWED )
        max_num_at = MAX_ATOMS_LARGE_MOL;

    if ( pStrErr )
        pStrErr[0] = '\0';


    /*FreeOrigAtData( orig_at_data );*/
    if ( lSdfId )
        *lSdfId = 0;

    do
    {
        at_old       = orig_at_data? orig_at_data->atom      : NULL; /*  save pointer to the previous allocation */

        stereo0D_old = orig_at_data? orig_at_data->stereo0D  : NULL;

        szCoordOld = NULL;

        num_inp_atoms_new =
            InchiToinp_ATOM( inp_file, orig_at_data? &stereo0D_new:NULL, &num_inp_0D_new,
                          bDoNotAddH, vABParityUnknown, nInputType,
                          orig_at_data? &at_new:NULL, MAX_ATOMS,
                          &num_dimensions_new, &num_inp_bonds_new,
                          pSdfLabel, pSdfValue, lSdfId, pInpAtomFlags, err, pStrErr );

        if ( num_inp_atoms_new <= 0 && !*err )
        {
            TREAT_ERR (*err, 0, "Empty structure");
            *err = 98;
        }
        else if ( orig_at_data &&
                  !num_inp_atoms_new &&
                  10 < *err && *err < 20 &&
                  orig_at_data->num_atoms > 0
                  && bMergeAllInputStructures )
        {
            *err = 0; /* end of file */
            break;
        }
        else if ( num_inp_atoms_new > 0 && orig_at_data )
        {
            /*  merge pOrigDataTmp + orig_at_data => pOrigDataTmp; */
            nNumAtoms    = num_inp_atoms_new + orig_at_data->num_atoms;
            nNumStereo0D = num_inp_0D_new    + orig_at_data->num_stereo0D;

            if ( nNumAtoms >= max_num_at ) /*MAX_ATOMS ) */
            {
                TREAT_ERR (*err, 0, "Too many atoms [check  'LargeMolecules' switch]");
                *err = 70;
                orig_at_data->num_atoms = -1;
            }
            else if ( !at_old )
            {
                /* the first structure */

                orig_at_data->atom         = at_new;            at_new            = NULL;
                orig_at_data->num_atoms    = num_inp_atoms_new; num_inp_atoms_new = 0;
                orig_at_data->stereo0D     = stereo0D_new;      stereo0D_new      = NULL;
                orig_at_data->num_stereo0D = num_inp_0D_new;    num_inp_0D_new    = 0;
            }
            else if ( orig_at_data->atom = Createinp_ATOM( nNumAtoms ) )
            {
                /*  switch at_new <--> orig_at_data->at; */

                if ( orig_at_data->num_atoms )
                {
                    memcpy( orig_at_data->atom, at_old, orig_at_data->num_atoms * sizeof(orig_at_data->atom[0]) );
                    /*  adjust numbering in the newly read structure */
                    for ( i = 0; i < num_inp_atoms_new; i ++ )
                    {
                        for ( j = 0; j < at_new[i].num_bonds; j ++ )
                        {
                            at_new[i].neighbor[j] += orig_at_data->num_atoms;
                        }
                    }
                }
                Freeinp_ATOM( &at_old );

                /*  copy newly read structure */
                memcpy( orig_at_data->atom + orig_at_data->num_atoms,
                        at_new,
                        num_inp_atoms_new * sizeof(orig_at_data->atom[0]) );

                /*  copy newly read 0D stereo */
                if ( num_inp_0D_new > 0 && stereo0D_new )
                {
                    if ( orig_at_data->stereo0D = CreateInchi_Stereo0D( nNumStereo0D ) )
                    {
                        int ncopy = orig_at_data->num_stereo0D * sizeof(orig_at_data->stereo0D[0]);
                        memcpy( orig_at_data->stereo0D, stereo0D_old, ncopy );

                        /*  adjust numbering in the newly read structure */
                        for ( i = 0; i < num_inp_0D_new; i ++ )
                        {
                            if ( stereo0D_new[i].central_atom >= 0 )
                                stereo0D_new[i].central_atom += orig_at_data->num_atoms;

                            for ( j = 0; j < 4; j ++ )
                                stereo0D_new[i].neighbor[j] += orig_at_data->num_atoms;
                        }

                        FreeInchi_Stereo0D( &stereo0D_old );

                        int ncopy =  num_inp_0D_new * sizeof(orig_at_data->stereo0D[0]);
                        memcpy( orig_at_data->stereo0D+orig_at_data->num_stereo0D,
                                stereo0D_new,
                                ncopy );
                    }
                    else
                    {
                        num_inp_0D_new = 0;
                        TREAT_ERR (*err, 0, "Out of RAM");
                        *err = -1;
                    }
                }
                else
                {
                    num_inp_0D_new = 0;
                }

                /* update lengths */
                orig_at_data->num_atoms    += num_inp_atoms_new;
                orig_at_data->num_stereo0D += num_inp_0D_new;
            }
            else
            {
                TREAT_ERR (*err, 0, "Out of RAM");
                *err = -1;
            }
        }
        else if ( num_inp_atoms_new > 0 )
        {
            nNumAtoms += num_inp_atoms_new;
        }

        Freeinp_ATOM( &at_new );
        num_inp_atoms_new = 0;
        FreeInchi_Stereo0D( &stereo0D_new );
        num_inp_0D_new = 0;
    } while ( !*err && bMergeAllInputStructures );

    /*
    if ( !*err )
    {
        orig_at_data->num_components =
            MarkDisconnectedComponents( orig_at_data );
        if ( orig_at_data->num_components == 0 )
        {
            TREAT_ERR (*err, 0, "No components found");
            *err = 99;
        }
        if ( orig_at_data->num_components < 0 )
        {
            TREAT_ERR (*err, 0, "Too many components");
            *err = 99;
        }
    }
    */

    if ( szCoordNew )
        inchi_free( szCoordNew );

    if ( at_new )
        inchi_free( at_new );

    /*
    if ( !*err )
    {
        if ( ReconcileAllCmlBondParities( orig_at_data->atom, orig_at_data->num_atoms ) )
        {
            TREAT_ERR (*err, 0, "Cannot reconcile stereobond parities");
            if (!orig_at_data->num_atoms)
            {
                *err = 1;
            }
        }
    }
    */

    if ( *err )
        FreeInchi_Input( orig_at_data );

    if ( *err && !(10 < *err && *err < 20) && pStrErr && !pStrErr[0] )
    {
        TREAT_ERR (*err, 0, "Unknown error");  /*   <BRKPT> */
    }

    return orig_at_data? orig_at_data->num_atoms : nNumAtoms;
}

#endif /* #ifdef TARGET_EXE_USING_API */




/*
    Read input InChI string and create/fill internal data structures
*/
int InchiToOrigAtom( INCHI_IOSTREAM *inp_molfile,
                     ORIG_ATOM_DATA *orig_at_data,
                     int bMergeAllInputStructures,
                     int bGetOrigCoord,
                     int bDoNotAddH,
                     int vABParityUnknown,
                     INPUT_TYPE nInputType,
                     char *pSdfLabel,
                     char *pSdfValue,
                     long *lSdfId,
                     INCHI_MODE *pInpAtomFlags,
                     int *err,
                     char *pStrErr )
{
/* inp_ATOM       *at = NULL; */
int            num_dimensions_new;
int            num_inp_bonds_new;
int            num_inp_atoms_new;
inp_ATOM      *at_new     = NULL;
inp_ATOM      *at_old     = NULL;
int            nNumAtoms  = 0;
MOL_COORD     *szCoordNew = NULL;
MOL_COORD     *szCoordOld = NULL;
int            i, j;
int               max_num_at;


    max_num_at = MAX_ATOMS;
    if ( !( *pInpAtomFlags  & FLAG_SET_INP_LARGE_ALLOWED ) )
        max_num_at = NORMALLY_ALLOWED_INP_MAX_ATOMS;

    if ( pStrErr )
    {
        pStrErr[0] = '\0';
    }

    /*FreeOrigAtData( orig_at_data );*/
    if ( lSdfId )
        *lSdfId = 0;
    do {
        at_old     = orig_at_data ? orig_at_data->at
                                  : NULL; /*  save pointer to the previous allocation */
        szCoordOld = orig_at_data ? orig_at_data->szCoord
                                  : NULL;

        num_inp_atoms_new = InchiToInpAtom( inp_molfile,
                                            (bGetOrigCoord && orig_at_data)? &szCoordNew : NULL,
                                            bDoNotAddH, vABParityUnknown,
                                            nInputType,
                                            orig_at_data? &at_new : NULL,
                                            MAX_ATOMS, &num_dimensions_new, &num_inp_bonds_new,
                                            pSdfLabel, pSdfValue, lSdfId,
                                            pInpAtomFlags, err, pStrErr );

        if ( num_inp_atoms_new <= 0 && !*err )
        {
            TREAT_ERR (*err, 0, "Empty structure");
            *err = 98;
        }
        else if ( orig_at_data && !num_inp_atoms_new &&
                  10 < *err && *err < 20 &&
                  orig_at_data->num_inp_atoms > 0 &&
                  bMergeAllInputStructures )
        {
            *err = 0; /* end of file */
            break;
        }
        else if ( num_inp_atoms_new > 0 && orig_at_data )
        {
            /*  merge pOrigDataTmp + orig_at_data => pOrigDataTmp; */
            nNumAtoms = num_inp_atoms_new + orig_at_data->num_inp_atoms;
            if ( nNumAtoms >= max_num_at ) /*MAX_ATOMS ) */
            {
                TREAT_ERR (*err, 0, "Too many atoms [check  'LargeMolecules' switch]");
                *err = 70;
                orig_at_data->num_inp_atoms = -1;
            }
            else if ( !at_old )
            {
                /* the first structure */
                orig_at_data->at      = at_new;
                orig_at_data->szCoord = szCoordNew;
                at_new = NULL;
                szCoordNew = NULL;
                orig_at_data->num_inp_atoms  = num_inp_atoms_new;
                orig_at_data->num_inp_bonds  = num_inp_bonds_new;
                orig_at_data->num_dimensions = num_dimensions_new;
            }
            else if ( (orig_at_data->at = ( inp_ATOM* ) inchi_calloc( nNumAtoms, sizeof(inp_ATOM) )) &&
                      (!szCoordNew || (orig_at_data->szCoord = (MOL_COORD *) inchi_calloc( nNumAtoms, sizeof(MOL_COORD) ))) )
            {
                /*  switch at_new <--> orig_at_data->at; */
                if ( orig_at_data->num_inp_atoms )
                {
                    memcpy( orig_at_data->at,
                            at_old,
                            orig_at_data->num_inp_atoms * sizeof(orig_at_data->at[0]) );
                    /*  adjust numbering in the newly read structure */
                    for ( i = 0; i < num_inp_atoms_new; i ++ )
                    {
                        for ( j = 0; j < at_new[i].valence; j ++ )
                        {
                            at_new[i].neighbor[j] += orig_at_data->num_inp_atoms;
                        }
                        at_new[i].orig_at_number += orig_at_data->num_inp_atoms; /* 12-19-2003 */
                    }
                    if ( orig_at_data->szCoord && szCoordOld )
                    {
                        memcpy( orig_at_data->szCoord,
                                szCoordOld,
                                orig_at_data->num_inp_atoms * sizeof(MOL_COORD) );
                    }
                }
                if ( at_old )
                {
                    inchi_free( at_old );
                    at_old = NULL;
                }
                if ( szCoordOld )
                {
                    inchi_free( szCoordOld );
                    szCoordOld = NULL;
                }
                /*  copy newly read structure */
                memcpy( orig_at_data->at + orig_at_data->num_inp_atoms,
                        at_new,
                        num_inp_atoms_new * sizeof(orig_at_data->at[0]) );
                if ( orig_at_data->szCoord && szCoordNew )
                {
                    memcpy( orig_at_data->szCoord + orig_at_data->num_inp_atoms,
                            szCoordNew,
                            num_inp_atoms_new * sizeof(MOL_COORD) );
                }
                /*  add other things */
                orig_at_data->num_inp_atoms += num_inp_atoms_new;
                orig_at_data->num_inp_bonds += num_inp_bonds_new;
                orig_at_data->num_dimensions = inchi_max(num_dimensions_new, orig_at_data->num_dimensions);
            }
            else
            {
                TREAT_ERR (*err, 0, "Out of RAM");
                *err = -1;
            }
        }
        else if ( num_inp_atoms_new > 0 )
        {
            nNumAtoms += num_inp_atoms_new;
        }
        if ( at_new )
        {
            inchi_free( at_new );
            at_new = NULL;
        }
    } while ( !*err && bMergeAllInputStructures );
    /*
    if ( !*err ) {
        orig_at_data->num_components =
            MarkDisconnectedComponents( orig_at_data );
        if ( orig_at_data->num_components == 0 ) {
            TREAT_ERR (*err, 0, "No components found");
            *err = 99;
        }
        if ( orig_at_data->num_components < 0 ) {
            TREAT_ERR (*err, 0, "Too many components");
            *err = 99;
        }
    }
    */
    if ( szCoordNew )
    {
        inchi_free( szCoordNew );
    }
    if ( at_new )
    {
        inchi_free( at_new );
    }
    if ( !*err && orig_at_data )
    {
        /* added testing (orig_at_data != NULL) */
        if ( ReconcileAllCmlBondParities( orig_at_data->at,
                                          orig_at_data->num_inp_atoms, 0 ) )
        {
            TREAT_ERR (*err, 0, "Cannot reconcile stereobond parities");  /*   <BRKPT> */
            if (!orig_at_data->num_dimensions) {
                *err = 1;
            }
        }
    }

    if ( *err )
    {
        FreeOrigAtData( orig_at_data );
    }
    if ( *err && !(10 < *err && *err < 20) && pStrErr && !pStrErr[0] )
    {
        TREAT_ERR (*err, 0, "Unknown error");  /*   <BRKPT> */
    }

    return orig_at_data ? orig_at_data->num_inp_atoms
                        : nNumAtoms;
}
