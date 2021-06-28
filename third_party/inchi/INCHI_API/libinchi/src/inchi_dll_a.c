/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.06
 * December 15, 2020
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
 * Copyright (C) IUPAC and InChI Trust
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
 * Licence No. 1.0 with this library; if not, please e-mail:
 *
 * info@inchi-trust.org
 *
 */


/*
    InChI - 'modularized' API ( since v. 1.02 )
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>
#include "../../../INCHI_BASE/src/incomdef.h"
#include "../../../INCHI_BASE/src/ichidrp.h"
#include "../../../INCHI_BASE/src/inpdef.h"
#include "../../../INCHI_BASE/src/ichi.h"
#include "../../../INCHI_BASE/src/strutil.h"
#include "../../../INCHI_BASE/src/util.h"
#include "../../../INCHI_BASE/src/ichierr.h"
#include "../../../INCHI_BASE/src/ichimain.h"
#include "../../../INCHI_BASE/src/extr_ct.h"
#include "../../../INCHI_BASE/src/ichi_io.h"
#include "../../../INCHI_BASE/src/mol_fmt.h"
#include "../../../INCHI_BASE/src/ichicomp.h"
#include "../../../INCHI_BASE/src/ichitaut.h"
#include "../../../INCHI_BASE/src/ichinorm.h"


#include "../../../INCHI_BASE/src/ichisize.h"
#include "../../../INCHI_BASE/src/ichitime.h"
#include "../../../INCHI_BASE/src/mode.h"
#include "../../../INCHI_BASE/src/inchi_api.h"

#include "inchi_dll_a.h" /* not inchi_api.h as it hides internal data types */
#include "inchi_dll.h"



/* Forward declaration */
struct tagINCHI_CLOCK;

/*
    Local prototypes.
*/

int parse_options_string( char *cmd, const char *argv[], int maxargs );
int ExtractOneStructure( STRUCT_DATA *sd,
                         INPUT_PARMS *ip,
                         char *szTitle,
                         inchi_InputEx *inp,
                         INCHI_IOSTREAM *log_file,
                         INCHI_IOSTREAM *out_file,
                         INCHI_IOSTREAM *prb_file,
                         ORIG_ATOM_DATA *orig_inp_data,
                         long *num_inp );
int NormOneStructureINChI( CANON_GLOBALS *pCG,
                           INCHI_CLOCK *ic,
                           INCHIGEN_DATA *pGenData,
                           INCHIGEN_CONTROL * HGen,
                           int iINChI,
                           INCHI_IOSTREAM *inp_file );
int CanonOneStructureINChI( CANON_GLOBALS *pCG,
                            struct tagINCHI_CLOCK *ic,
                            INCHIGEN_CONTROL *HGen,
                            int iINChI,
                            INCHI_IOSTREAM *inp_file );


/****************************************************************************

    InChI Generator: create generator
    Returns handle of generator object or NULL on failure

****************************************************************************/


/****************************************************************************/
INCHIGEN_HANDLE INCHI_DECL STDINCHIGEN_Create( void )
{
    return INCHIGEN_Create( );
}



/****************************************************************************/
INCHIGEN_HANDLE INCHI_DECL INCHIGEN_Create( void )
{
    INCHIGEN_CONTROL * HGen = NULL;


    HGen = (INCHIGEN_CONTROL *) inchi_malloc( sizeof( INCHIGEN_CONTROL ) );

    if (!HGen)
    {
        return (INCHIGEN_HANDLE) NULL;
    }

    memset( HGen, 0, sizeof( INCHIGEN_CONTROL ) );

    /* Set/init aliases */
    memset( &( HGen->InpParms ), 0, sizeof( INPUT_PARMS ) );
    memset( &( HGen->StructData ), 0, sizeof( STRUCT_DATA ) );

    HGen->ulTotalProcessingTime = 0;
    HGen->num_err = 0;
    HGen->num_inp = 0;
    HGen->szTitle[0] = '\0';


    /* Initialize output streams as string buffers */
    inchi_ios_init( &( HGen->inchi_file[0] ), INCHI_IOS_TYPE_STRING, NULL );
    inchi_ios_init( &( HGen->inchi_file[1] ), INCHI_IOS_TYPE_STRING, NULL );
    inchi_ios_init( &( HGen->inchi_file[2] ), INCHI_IOS_TYPE_STRING, NULL );


    memset( &( HGen->OrigInpData ), 0, sizeof( HGen->OrigInpData ) );
    memset( &( HGen->PrepInpData[0] ), 0, 2 * sizeof( HGen->PrepInpData[0] ) );

    memset( HGen->pINChI, 0, sizeof( HGen->pINChI ) );
    memset( HGen->pINChI_Aux, 0, sizeof( HGen->pINChI_Aux ) );

    /* Supply expandable string buffer */
    if (0 >= inchi_strbuf_init( &( HGen->strbuf_container ), INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT ))
    {
        inchi_free( HGen );
        return (INCHIGEN_HANDLE) NULL;
    }

    return (INCHIGEN_HANDLE) HGen;
}


/****************************************************************************

    InChI Generator: initialization stage (accepts a specific structure)

****************************************************************************/


/****************************************************************************/
int INCHI_DECL STDINCHIGEN_Setup( INCHIGEN_HANDLE _HGen,
                                 INCHIGEN_DATA * pGenData,
                                 inchi_Input * pInp )
{
    INCHIGEN_CONTROL *HGen = (INCHIGEN_CONTROL *) _HGen;
    INPUT_PARMS *ip = &( HGen->InpParms );
    STRUCT_DATA *sd = &( HGen->StructData );
    int retcode = inchi_Ret_OKAY;

    retcode = INCHIGEN_Setup( _HGen, pGenData, pInp );

    /* Ensure standardness */
    if (ip->bINChIOutputOptions & INCHI_OUT_SAVEOPT)
    {
        ip->bINChIOutputOptions &= ~INCHI_OUT_SAVEOPT;
        retcode = _IS_WARNING;
    }
    if (0 != ( ip->bTautFlags & TG_FLAG_RECONNECT_COORD ))
    {
        ip->bTautFlags &= ~TG_FLAG_RECONNECT_COORD;
        retcode = _IS_WARNING;
    }
    if (0 != ( ip->nMode & REQ_MODE_BASIC ))
    {
        ip->nMode &= ~REQ_MODE_BASIC;
        retcode = _IS_WARNING;
    }
    if (0 != ( ip->nMode & REQ_MODE_RELATIVE_STEREO ))
    {
        ip->nMode &= ~( REQ_MODE_RACEMIC_STEREO | REQ_MODE_RELATIVE_STEREO | REQ_MODE_CHIR_FLG_STEREO );
        retcode = _IS_WARNING;
    }
    if (0 != ( ip->nMode & REQ_MODE_RACEMIC_STEREO ))
    {
        ip->nMode &= ~( REQ_MODE_RACEMIC_STEREO | REQ_MODE_RELATIVE_STEREO | REQ_MODE_CHIR_FLG_STEREO );
        retcode = _IS_WARNING;
    }
    if (0 != ( ip->nMode & REQ_MODE_CHIR_FLG_STEREO ))
    {
        ip->nMode &= ~( REQ_MODE_RACEMIC_STEREO | REQ_MODE_RELATIVE_STEREO | REQ_MODE_CHIR_FLG_STEREO );
        retcode = _IS_WARNING;
    }
    if (0 != ( ip->nMode & REQ_MODE_DIFF_UU_STEREO ))
    {
        ip->nMode &= ~REQ_MODE_DIFF_UU_STEREO;
        retcode = _IS_WARNING;
    }
    if (0 == ( ip->nMode & ( REQ_MODE_SB_IGN_ALL_UU | REQ_MODE_SC_IGN_ALL_UU ) ))
    {
        ip->nMode |= REQ_MODE_SB_IGN_ALL_UU;
        ip->nMode |= REQ_MODE_SC_IGN_ALL_UU;
        retcode = _IS_WARNING;
    }
    if (0 != ( ip->bTautFlags & TG_FLAG_KETO_ENOL_TAUT ))
    {
        ip->bTautFlags &= ~TG_FLAG_KETO_ENOL_TAUT;
        retcode = _IS_WARNING;
    }
    if (0 != ( ip->bTautFlags & TG_FLAG_1_5_TAUT ))
    {
        ip->bTautFlags &= ~TG_FLAG_1_5_TAUT;
        retcode = _IS_WARNING;
    }

    /* And anyway... */
    ip->bINChIOutputOptions |= INCHI_OUT_STDINCHI;
    ip->bINChIOutputOptions &= ~INCHI_OUT_SAVEOPT;

    strcpy( pGenData->pStrErrStruct, sd->pStrErrStruct );

    return retcode;
}


/****************************************************************************/
int INCHI_DECL INCHIGEN_Setup( INCHIGEN_HANDLE _HGen,
                              INCHIGEN_DATA * pGenData,
                              inchi_Input * pInp )
{
    int retcode = inchi_Ret_OKAY;

    INCHIGEN_CONTROL *HGen = (INCHIGEN_CONTROL *) _HGen;

    ORIG_ATOM_DATA *orig_inp_data = &( HGen->OrigInpData );
    STRUCT_DATA *sd = &( HGen->StructData );
    INPUT_PARMS *ip = &( HGen->InpParms );
    INCHI_IOSTREAM *log_file = HGen->inchi_file + 1;
    INCHI_IOSTREAM prbstr, *prb_file = &prbstr;

    const char *argv[INCHI_MAX_NUM_ARG + 1];
    int   argc;
    char *szOptions = NULL;
    char szSdfDataValue[MAX_SDF_VALUE + 1];
    int bReleaseVersion = bRELEASE_VERSION;
    unsigned long  ulDisplTime = 0;    /*  infinite, milliseconds */
    int p;
    inchi_InputEx inpEx;
    inchi_InputEx *pInpEx = &inpEx;

    /* No '*' or 'Zz' elements are allowed in the input . */
    if ( input_erroneously_contains_pseudoatoms( pInp, NULL) )
    {
        AddErrorMessage(sd->pStrErrStruct, "Pseudoatoms are not supported in current API mode");
        sd->nStructReadError = 99;
        sd->nErrorType = _IS_ERROR;
        retcode = _IS_ERROR;
        goto ret;
    }

    pInpEx->atom = pInp->atom;
    pInpEx->num_atoms = pInp->num_atoms;
    pInpEx->num_stereo0D = pInp->num_stereo0D;
    pInpEx->stereo0D = pInp->stereo0D;
    pInpEx->szOptions = pInp->szOptions;
    pInpEx->polymer = NULL; /* v. 1.05 ext not supported in modularized API */
    pInpEx->v3000 = NULL; /* v. 1.05 ext not supported in modularized API */


    /* Allocate/init */
    if (!pGenData)
    {
        retcode = _IS_ERROR;
        goto ret;
    }
    memset( pGenData, 0, sizeof( *pGenData ) );

    /* Parse 'command-line' options and fill internal INPUT_PARMS structure */
    if (pInp && pInp->szOptions)
    {
        szOptions = (char*) inchi_malloc( strlen( pInp->szOptions ) + 1 );
        if (!szOptions)
            return _IS_FATAL;   /* Not enough memory.... */
        else
        {
            /* Parse. */
            strcpy( szOptions, pInp->szOptions );
            argc = parse_options_string( szOptions, argv, INCHI_MAX_NUM_ARG );
        }
    }
    else
    {
        /* Got NULL options string or NULL 'pInp', will use defaults. */
        argc = 1;
        argv[0] = "";
        argv[1] = NULL;
    }


    if (argc == 1
#ifdef TARGET_API_LIB
        && ( !pInp || pInp->num_atoms <= 0 || !pInp->atom )
#endif
        || argc == 2 && ( argv[1][0] == INCHI_OPTION_PREFX ) &&
        ( !strcmp( argv[1] + 1, "?" ) || !inchi_stricmp( argv[1] + 1, "help" ) ))
    {

        HelpCommandLineParms( log_file );
        memset( log_file, 0, sizeof( *log_file ) );
        return _IS_EOF;
    }

    memset( szSdfDataValue, 0, sizeof( szSdfDataValue ) );

    /* Decrypt command line    */
    retcode = ReadCommandLineParms( argc, argv, ip, szSdfDataValue, &ulDisplTime, bReleaseVersion, log_file );

    if (szOptions)
    {
        inchi_free( szOptions );
    }

    /* INChI DLL specific */
    ip->bNoStructLabels = 1;

    if (0 > retcode)
    {
        goto ret;
    }

    if (ip->bNoStructLabels)
    {
        ip->pSdfLabel = NULL;
        ip->pSdfValue = NULL;
    }
    else
    {
        if (ip->nInputType == INPUT_INCHI_XML ||
          ip->nInputType == INPUT_INCHI_PLAIN ||
          ip->nInputType == INPUT_CMLFILE)
        {
            /* the input may contain both the header and the label of the structure */
            if (!ip->pSdfLabel)
            {
                ip->pSdfLabel = ip->szSdfDataHeader;
            }
            if (!ip->pSdfValue)
            {
                ip->pSdfValue = szSdfDataValue;
            }
        }
    }

    if (retcode != inchi_Ret_OKAY) goto ret;

    PrintInputParms( log_file, ip );

    /* Extract the structure */
    retcode = ExtractOneStructure( sd, ip, HGen->szTitle, pInpEx, log_file,
                                   HGen->inchi_file, /* out_file */
                                   prb_file, orig_inp_data,
                                   &( HGen->num_inp ) );

ret:switch (retcode)
{
    case _IS_OKAY: retcode = inchi_Ret_OKAY; HGen->init_passed = 1; break;    /* Success; break; no errors or warnings */

    case _IS_ERROR: ( HGen->num_err )++;  retcode = inchi_Ret_ERROR; break;
                                                            /* Error: no INChI has been created */
    case _IS_FATAL: ( HGen->num_err )++;  retcode = inchi_Ret_FATAL; break;
                                                            /* Severe error: no INChI has been created
                                                            (typically; break; memory allocation failed) */
    case _IS_SKIP: retcode = inchi_Ret_SKIP; break;   /* not used in INChI dll */
    case _IS_EOF: retcode = inchi_Ret_EOF; break;   /* no structural data has been provided */
    case _IS_WARNING: retcode = inchi_Ret_WARNING; HGen->init_passed = 1; break;    /* Success; break; warning(s) issued */
    case _IS_UNKNOWN:
    default: retcode = inchi_Ret_UNKNOWN; break;   /* Unlnown program error */
}

    if (NULL!=pGenData)
    {
        strcpy( pGenData->pStrErrStruct, sd->pStrErrStruct );
        for (p = 0; p < INCHI_NUM; p++)
        {
            pGenData->num_components[p] = sd->num_components[p];
        }
    }

    return retcode;
}



/****************************************************************************

    Get normalized form of the structure

****************************************************************************/


/****************************************************************************/
int INCHI_DECL STDINCHIGEN_DoNormalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData )
{
    return INCHIGEN_DoNormalization( HGen, pGenData );
}


/****************************************************************************/
int INCHI_DECL INCHIGEN_DoNormalization( INCHIGEN_HANDLE _HGen, INCHIGEN_DATA *pGenData )
{
    int nRet = 0, nRet1 = 0;
    /* int maxINChI=0; */

    INCHIGEN_CONTROL * HGen = (INCHIGEN_CONTROL *) _HGen;
    INPUT_PARMS *ip = &( HGen->InpParms );
    STRUCT_DATA *sd = &( HGen->StructData );
    NORM_CANON_FLAGS *pncFlags = &( HGen->ncFlags );
    INCHI_IOSTREAM *out_file = HGen->inchi_file;
    INCHI_IOSTREAM inpstr, *inp_file = &inpstr;
    ORIG_ATOM_DATA *orig_inp_data = &( HGen->OrigInpData );
    ORIG_STRUCT      *pOrigStruct = NULL;
    INCHI_CLOCK ic;
    CANON_GLOBALS CG;
    int k;

    memset( &CG, 0, sizeof( CG ) );
    memset( &ic, 0, sizeof( ic ) );

#if ( RING2CHAIN == 1 || UNDERIVATIZE == 1 )
    int ret1 = 0, ret2 = 0;
#endif

/* Set debug output */
#if (TRACE_MEMORY_LEAKS == 1)

    _CrtSetDbgFlag( _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF );
/* for execution outside the VC++ debugger uncomment one of the following two */
#ifdef MY_REPORT_FILE
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_WARN, MY_REPORT_FILE );
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ERROR, MY_REPORT_FILE );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, MY_REPORT_FILE );
#else
    _CrtSetReportMode( _CRT_WARN | _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif
#if ( !defined(__STDC__) || __STDC__ != 1 )
    /* turn on floating point exceptions */
    {
        /* Get the default control word. */
        int cw = _controlfp( 0, 0 );
        /* Set the exception masks OFF, turn exceptions on. */
        /*cw &=~(EM_OVERFLOW|EM_UNDERFLOW|EM_INEXACT|EM_ZERODIVIDE|EM_DENORMAL);*/
        cw &= ~( EM_OVERFLOW | EM_UNDERFLOW | EM_ZERODIVIDE | EM_DENORMAL );
        /* Set the control word. */
        _controlfp( cw, MCW_EM );
    }
#endif /* ( !defined(__STDC__) || __STDC__ != 1 ) */

#endif /* (TRACE_MEMORY_LEAKS == 1) */

    if (HGen->init_passed == 0)
    {
        AddErrorMessage( sd->pStrErrStruct, "InChI generator not initialized" );
        sd->nStructReadError = 99;
        sd->nErrorType = _IS_ERROR;
        nRet = _IS_ERROR;
        goto exit_function;
    }

    inchi_ios_init( inp_file, INCHI_IOS_TYPE_FILE, NULL );

    sd->bUserQuitComponent = 0;
    sd->bUserQuitComponentDisplay = 0;
    memset( HGen->composite_norm_data, 0, sizeof( HGen->composite_norm_data ) );
    memset( pncFlags, 0, sizeof( *pncFlags ) );

    /* for testing only */
#if( REMOVE_ION_PAIRS_ORIG_STRU == 1 )
    fix_odd_things( orig_inp_data->num_inp_atoms, orig_inp_data->at, 0 );
#endif

#if 0 /*** FOR NOW, DISABLE UNDERIVATIZATION IN  MODULAR INTERFACE LIBINCHI ***/
#if( UNDERIVATIZE == 1 )  /***** post v.1 feature *****/
    /*if (ip->bUnderivatize && 0 > ( ret2 = underivatize( orig_inp_data ) ))*/

    if (ip->bUnderivatize
         && (ret2 = OAD_Edit_Underivatize( &ic, &CG, orig_inp_data, ( ip->bINChIOutputOptions & INCHI_OUT_SDFILE_ONLY ), ip->bUnderivatize & 2, ip->pSdfValue ) )
        )

    {
        long num_inp2 = HGen->num_inp;
        AddErrorMessage( sd->pStrErrStruct, "Underivatization error" );
        sd->nStructReadError = 99;
        sd->nErrorType = _IS_ERROR;
        nRet = _IS_ERROR;
        TreatReadTheStructureErrors( sd, ip, LOG_MASK_ALL, inp_file, log_file, out_file, prb_file,
                                        prep_inp_data, &num_inp2, HGen->pStr, PSTR_BUFFER_SIZE );
        goto exit_function; /* output only if derivatives found */
    }
#endif /* UNDERIVATIZE == 1 */
#if( RING2CHAIN == 1 )  /***** post v.1 feature *****/
    if (ip->bRing2Chain && 0 > ( ret1 = Ring2Chain( orig_inp_data ) ))
    {
        long num_inp2 = HGen->num_inp;
        AddErrorMessage( sd->pStrErrStruct, "Ring to chain error" );
        sd->nStructReadError = 99;
        sd->nErrorType = _IS_ERROR;
        nRet = _IS_ERROR;
        TreatReadTheStructureErrors( sd, ip, LOG_MASK_ALL, inp_file, log_file, out_file, prb_file,
                                        prep_inp_data, &num_inp2, HGen->pStr, PSTR_BUFFER_SIZE );
        goto exit_function; /* output only if derivatives found */
    }
#endif /* RING2CHAIN == 1 */
#if ( RING2CHAIN == 1 || UNDERIVATIZE == 1 )  /***** post v.1 feature *****/
    if (ip->bIngnoreUnchanged && !ret1 && !ret2)
    {
        goto exit_function; /* output only if derivatives or ring/chain found */
    }
#endif /* RING2CHAIN == 1 || UNDERIVATIZE == 1 */
#endif /*** FOR NOW, DISABLE UNDERIVATIZATION IN  MODULAR INTERFACE LIBINCHI ***/


    /***** output MOLfile ***************/
    if (ip->bINChIOutputOptions & INCHI_OUT_SDFILE_ONLY)
    {
        char szNumber[32];
        int ret1a = 0, ret2a = 0; /* for derivatives and ring-chain */
        ret1a = sprintf( szNumber, "Structure #%ld", HGen->num_inp );
        ret2a = OrigAtData_WriteToSDfile( orig_inp_data, out_file, szNumber, NULL,
            ( sd->bChiralFlag & FLAG_INP_AT_CHIRAL ) ? 1 : 0,
            ( ip->bINChIOutputOptions & INCHI_OUT_SDFILE_ATOMS_DT ) ? 1 : 0, ip->pSdfLabel, ip->pSdfValue );
        goto exit_function;
    }

    /******* create full reversibility information **************/
    if (!( ip->bINChIOutputOptions & ( INCHI_OUT_NO_AUX_INFO | INCHI_OUT_SHORT_AUX_INFO ) ))
    {
        pOrigStruct = &( HGen->OrigStruct );
        memset( pOrigStruct, 0, sizeof( *pOrigStruct ) );
        if (OrigStruct_FillOut( &CG, orig_inp_data, pOrigStruct, sd ))
        {
            AddErrorMessage( sd->pStrErrStruct, "Cannot interpret reversibility information" );
            sd->nStructReadError = 99;
            sd->nErrorType = _IS_ERROR;
            nRet = _IS_ERROR;
        }
    }

    sd->bUserQuit = 0;
    if (sd->bUserQuit)
    {
        goto exit_function;
    }

    /* Normalize the whole disconnected or original structure */
    if (nRet != _IS_FATAL && nRet != _IS_ERROR)
    {
        nRet1 = NormOneStructureINChI( &CG, &ic, pGenData, HGen, INCHI_BAS, inp_file );
        nRet = inchi_max( nRet, nRet1 );
    }

    /*
    if ( nRet != _IS_FATAL && nRet != _IS_ERROR )
        maxINChI = 1;
    */

    if (nRet != _IS_FATAL && nRet != _IS_ERROR &&
        ( sd->bTautFlagsDone[INCHI_BAS] & TG_FLAG_DISCONNECT_COORD_DONE ) &&
        ( ip->bTautFlags               & TG_FLAG_RECONNECT_COORD ))
    {
        /* Normalize  the whole reconnected structure */
        nRet1 = NormOneStructureINChI( &CG, &ic, pGenData, HGen, INCHI_REC, inp_file );
        nRet = inchi_max( nRet, nRet1 );
        /*
        if ( nRet != _IS_FATAL && nRet != _IS_ERROR )
                maxINChI = 2;
        */
    }

exit_function:

    if (nRet != _IS_FATAL && nRet != _IS_ERROR)
    {
        HGen->norm_passed = 1;
    }

    for (k = 0; k < INCHI_NUM; k++)
    {
        pGenData->num_components[k] = sd->num_components[k];
    }

    /* Emit normalization warnings */
    if (nRet != _IS_FATAL && nRet != _IS_ERROR)
    {
        int ics, istruct, itaut, nc[2];
        int warn_prot = 0, warn_neutr = 0;
        INP_ATOM_DATA *inp_norm_data[TAUT_NUM]; /*  = { &InpNormAtData, &InpNormTautData }; */
        nc[0] = pGenData->num_components[0];
        nc[1] = pGenData->num_components[1];
        for (istruct = 0; istruct < 2; istruct++)
        {
            if (nc[istruct] > 0)
            {
                for (ics = 0; ics < nc[istruct]; ics++)
                {
                    inp_norm_data[0] = &( HGen->InpNormAtData[istruct][ics] );
                    inp_norm_data[1] = &( HGen->InpNormTautData[istruct][ics] );
                    for (itaut = 0; itaut < 2; itaut++)
                    {
                        if (NULL != inp_norm_data[itaut])
                        {
                            if (inp_norm_data[itaut]->bTautomeric)
                            {
                                if (inp_norm_data[itaut]->bNormalizationFlags & ( FLAG_NORM_CONSIDER_TAUT &~FLAG_PROTON_CHARGE_CANCEL ))
                                {
                                    if (warn_prot == 0)
                                    {
                                        warn_prot++;
                                        WarningMessage( sd->pStrErrStruct, "Proton(s) added/removed" );
                                    }
                                }
                                if (inp_norm_data[itaut]->bNormalizationFlags & FLAG_PROTON_CHARGE_CANCEL)
                                {
                                    if (warn_neutr == 0)
                                    {
                                        warn_neutr++;
                                        WarningMessage( sd->pStrErrStruct, "Charges neutralized" );
                                    }
                                }
                            }
                        }
                    } /* itaut */
                }
            }
        }
    }

    strcpy( pGenData->pStrErrStruct, sd->pStrErrStruct );

    make_norm_atoms_from_inp_atoms( pGenData, HGen );

    return nRet;
}



/****************************************************************************

    Get canonicalized form of the structure

****************************************************************************/


/****************************************************************************/
int INCHI_DECL STDINCHIGEN_DoCanonicalization
( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData )
{
    return INCHIGEN_DoCanonicalization( HGen, pGenData );
}


/****************************************************************************/
int INCHI_DECL INCHIGEN_DoCanonicalization
( INCHIGEN_HANDLE _HGen, INCHIGEN_DATA *pGenData )
{
    int nRet = 0, nRet1 /*, maxINChI=0*/;
    INCHIGEN_CONTROL * HGen = (INCHIGEN_CONTROL *) _HGen;


    STRUCT_DATA *sd = &( HGen->StructData );
    INPUT_PARMS *ip = &( HGen->InpParms );
    INCHI_IOSTREAM *out_file = HGen->inchi_file, *log_file = HGen->inchi_file + 1;
    INCHI_IOSTREAM prbstr, *prb_file = &prbstr;
    INCHI_IOSTREAM inpstr, *inp_file = &inpstr;

    ORIG_ATOM_DATA *prep_inp_data = &( HGen->PrepInpData[0] );

    INCHI_CLOCK ic;
    CANON_GLOBALS CG;

    int k;

    memset( &ic, 0, sizeof( ic ) );
    memset( &CG, 0, sizeof( CG ) );

    /* Set debug output */
#if (TRACE_MEMORY_LEAKS == 1)

    _CrtSetDbgFlag( _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF );
    /* for execution outside the VC++ debugger uncomment one of the following two */
#ifdef MY_REPORT_FILE
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_WARN, MY_REPORT_FILE );
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ERROR, MY_REPORT_FILE );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, MY_REPORT_FILE );
#else
    _CrtSetReportMode( _CRT_WARN | _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif
#if ( !defined(__STDC__) || __STDC__ != 1 )
    /* turn on floating point exceptions */
    {
        /* Get the default control word. */
        int cw = _controlfp( 0, 0 );
        /* Set the exception masks OFF, turn exceptions on. */
        /*cw &=~(EM_OVERFLOW|EM_UNDERFLOW|EM_INEXACT|EM_ZERODIVIDE|EM_DENORMAL);*/
        cw &= ~( EM_OVERFLOW | EM_UNDERFLOW | EM_ZERODIVIDE | EM_DENORMAL );
        /* Set the control word. */
        _controlfp( cw, MCW_EM );
    }
#endif /* ( !defined(__STDC__) || __STDC__ != 1 ) */

#endif /* (TRACE_MEMORY_LEAKS == 1) */



    if (HGen->norm_passed == 0)
    {
        AddErrorMessage( sd->pStrErrStruct, "Got non-normalized structure" );
        sd->nStructReadError = 99;
        sd->nErrorType = _IS_ERROR;
        nRet = _IS_ERROR;
        goto exit_function;
    }


    inchi_ios_init( inp_file, INCHI_IOS_TYPE_FILE, NULL );
    inchi_ios_init( prb_file, INCHI_IOS_TYPE_FILE, NULL );

    sd->bUserQuit = 0;
    if (sd->bUserQuit)
    {
        goto exit_function;
    }

   /* create INChI for each connected component of the structure and optionally display them */
   /* output INChI for the whole structure */



    /* create INChI for each connected component of the structure and optionally display them */
    /* create INChI for the whole disconnected or original structure */
    if (nRet != _IS_FATAL && nRet != _IS_ERROR)
    {
        nRet1 = CanonOneStructureINChI( &CG, &ic, HGen, INCHI_BAS, inp_file );
        nRet = inchi_max( nRet, nRet1 );
    }

    /*
    if ( nRet != _IS_FATAL && nRet != _IS_ERROR )
        maxINChI = 1;
    */

    if (nRet != _IS_FATAL && nRet != _IS_ERROR &&
        ( sd->bTautFlagsDone[INCHI_BAS] & TG_FLAG_DISCONNECT_COORD_DONE ) &&
        ( ip->bTautFlags               & TG_FLAG_RECONNECT_COORD ))
    {
        /* create INChI for the whole reconnected structure */
        nRet1 = CanonOneStructureINChI( &CG, &ic, HGen, INCHI_REC, inp_file );
        nRet = inchi_max( nRet, nRet1 );
        /*
        if ( nRet != _IS_FATAL && nRet != _IS_ERROR )
                maxINChI = 2;
        */
    }

    if (nRet != _IS_FATAL && nRet != _IS_ERROR)
    {
        if (( sd->bChiralFlag & FLAG_INP_AT_CHIRAL ) &&
            ( ip->nMode & REQ_MODE_STEREO ) &&
              !( ip->nMode & ( REQ_MODE_RELATIVE_STEREO | REQ_MODE_RACEMIC_STEREO ) ) &&
              !bIsStructChiral( HGen->pINChI, sd->num_components ))
        {
            WarningMessage( sd->pStrErrStruct, "Not chiral" );
        }

        /*************************************/
        /*       Output err/warn messages    */
        /*************************************/
        if ( /*!sd->nErrorCode &&*/ !sd->bUserQuitComponent && !sd->bUserQuit)
        {
            /*  if successful then returns 0, otherwise returns _IS_FATAL */
            /*  extract the structure if requested */
            nRet1 = TreatCreateINChIWarning( sd, ip, prep_inp_data, HGen->num_inp,
                                 inp_file, log_file, out_file, prb_file );
            nRet = inchi_max( nRet, nRet1 );
        }
    }

    switch (nRet)
    {
        case _IS_SKIP: nRet = inchi_Ret_SKIP; break; /* not used in INChI dll */
        case _IS_EOF: nRet = inchi_Ret_EOF; break; /* no structural data has been provided */
        case _IS_OKAY: nRet = inchi_Ret_OKAY; HGen->canon_passed = 1; break;
                                                    /* Success; break; no errors or warnings */
        case _IS_WARNING: nRet = inchi_Ret_WARNING; HGen->canon_passed = 1; break;
                                                    /* Success; break; warning(s) issued */
        case _IS_ERROR: nRet = inchi_Ret_ERROR; break; /* Error: no INChI has been created */
        case _IS_FATAL: nRet = inchi_Ret_FATAL; break; /* Severe error: no INChI has been created (typically; break; memory allocation failed) */
        case _IS_UNKNOWN:
        default: nRet = inchi_Ret_UNKNOWN; break; /* Unknown program error */
    }
exit_function:

    strcpy( pGenData->pStrErrStruct, sd->pStrErrStruct );
    for (k = 0; k < INCHI_NUM; k++)
    {
        pGenData->num_components[k] = sd->num_components[k];
    }

    return nRet;
} /* INCHIGEN_DoCanonicalization */



/****************************************************************************

    Get serialized form (InChI string).

****************************************************************************/


/****************************************************************************/
int INCHI_DECL STDINCHIGEN_DoSerialization( INCHIGEN_HANDLE HGen,
                                                                     INCHIGEN_DATA * pGenData,
                                                                     inchi_Output * pResults )
{
    return INCHIGEN_DoSerialization( HGen, pGenData, pResults );
}



/****************************************************************************/
int INCHI_DECL INCHIGEN_DoSerialization( INCHIGEN_HANDLE _HGen,
                                        INCHIGEN_DATA * pGenData,
                                        inchi_Output * pResults )
{
    int nRet = 0, nRet1 = 0, i, k;



    INCHIGEN_CONTROL * HGen = (INCHIGEN_CONTROL *) _HGen;

    INPUT_PARMS *ip = &( HGen->InpParms );
    INCHI_IOSTREAM *out_file = HGen->inchi_file, *log_file = HGen->inchi_file + 1;
    INCHI_IOSTREAM inpstr, *inp_file = &inpstr;
    INCHI_IOSTREAM prbstr, *prb_file = &prbstr;

    STRUCT_DATA *sd = &( HGen->StructData );
    NORM_CANON_FLAGS *pncFlags = &( HGen->ncFlags );
    ORIG_ATOM_DATA *orig_inp_data = &( HGen->OrigInpData );
    ORIG_ATOM_DATA *prep_inp_data = &( HGen->PrepInpData[0] );
    ORIG_STRUCT      *pOrigStruct = &( HGen->OrigStruct );
    int bSortPrintINChIFlags = 0;
    unsigned char save_opt_bits = 0;
    int retcode = 0;

    CANON_GLOBALS CG;
    memset( &CG, 0, sizeof( CG ) );

    /* Post-1.02b - added initialization of pResults to 0; thanks to David Foss */
    memset( pResults, 0, sizeof( *pResults ) );
    pResults->szLog = log_file->s.pStr;
    inchi_ios_init( inp_file, INCHI_IOS_TYPE_FILE, NULL );
    inchi_ios_init( prb_file, INCHI_IOS_TYPE_FILE, NULL );

    /* Set debug output */

#if (TRACE_MEMORY_LEAKS == 1)

    _CrtSetDbgFlag( _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_ALLOC_MEM_DF );

/* for execution outside the VC++ debugger uncomment one of the following two */
#ifdef MY_REPORT_FILE
    _CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_WARN, MY_REPORT_FILE );
    _CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ERROR, MY_REPORT_FILE );
    _CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_FILE );
    _CrtSetReportFile( _CRT_ASSERT, MY_REPORT_FILE );
#else
    _CrtSetReportMode( _CRT_WARN | _CRT_ERROR, _CRTDBG_MODE_DEBUG );
#endif
#if ( !defined(__STDC__) || __STDC__ != 1 )
    /* turn on floating point exceptions */
    {
        /* Get the default control word. */
        int cw = _controlfp( 0, 0 );
        /* Set the exception masks OFF, turn exceptions on. */
        /*cw &=~(EM_OVERFLOW|EM_UNDERFLOW|EM_INEXACT|EM_ZERODIVIDE|EM_DENORMAL);*/
        cw &= ~( EM_OVERFLOW | EM_UNDERFLOW | EM_ZERODIVIDE | EM_DENORMAL );
        /* Set the control word. */
        _controlfp( cw, MCW_EM );
    }
#endif /* ( !defined(__STDC__) || __STDC__ != 1 ) */

#endif /* (TRACE_MEMORY_LEAKS == 1) */


/*****************************/


    if (HGen->canon_passed == 0)
    {
        AddErrorMessage( sd->pStrErrStruct, "Got non-canonicalized structure" );
        sd->nStructReadError = 99;
        sd->nErrorType = _IS_ERROR;
        retcode = _IS_ERROR;
        goto frees;
    }

    /************************************************/
    /*  sort and print INChI for the whole structure */
    /************************************************/

    /* Prepare SaveOpt bits */
    if (ip->bINChIOutputOptions & INCHI_OUT_SAVEOPT)
    {
        if (0 != ( ip->bTautFlags & TG_FLAG_RECONNECT_COORD ))
        {
            save_opt_bits |= SAVE_OPT_RECMET;
        }
        if (0 != ( ip->nMode & REQ_MODE_BASIC ))
        {
            save_opt_bits |= SAVE_OPT_FIXEDH;
        }
        if (0 != ( ip->nMode & REQ_MODE_DIFF_UU_STEREO ))
        {
            save_opt_bits |= SAVE_OPT_SLUUD;
        }
        if (0 == ( ip->nMode & ( REQ_MODE_SB_IGN_ALL_UU | REQ_MODE_SC_IGN_ALL_UU ) ))
        {
            save_opt_bits |= SAVE_OPT_SUU;
        }
        if (0 != ( ip->bTautFlags & TG_FLAG_KETO_ENOL_TAUT ))
        {
            save_opt_bits |= SAVE_OPT_KET;
        }
        if (0 != ( ip->bTautFlags & TG_FLAG_1_5_TAUT ))
        {
            save_opt_bits |= SAVE_OPT_15T;
        }
    }

    nRet = SortAndPrintINChI( &CG,out_file, &( HGen->strbuf_container ),
                              log_file, ip, orig_inp_data, prep_inp_data,
                              HGen->composite_norm_data, pOrigStruct,
                              sd->num_components, sd->num_non_taut, sd->num_taut,
                              sd->bTautFlags, sd->bTautFlagsDone,
                              pncFlags, HGen->num_inp,
                              HGen->pINChI, HGen->pINChI_Aux,
                              &bSortPrintINChIFlags, save_opt_bits );

    if (nRet != _IS_FATAL && nRet != _IS_ERROR)
    {
        /* Special mode: extract all good MOLfiles into the problem file
        * Do not extract any MOLfile that could not be processed (option /PGO)
        */
        if (prb_file->f && 0L <= sd->fPtrStart && sd->fPtrStart < sd->fPtrEnd && ip->bSaveAllGoodStructsAsProblem)
        {
            MolfileSaveCopy( inp_file, sd->fPtrStart, sd->fPtrEnd, prb_file->f, 0 );
        }
#if( /*bRELEASE_VERSION != 1 &&*/ EXTR_FLAGS == EXTR_TRANSPOSITION_EXAMPLES && EXTR_MASK == EXTR_FLAGS )
        else
            if (prb_file->f && ( bSortPrintINChIFlags &
                ( FLAG_SORT_PRINT_TRANSPOS_BAS | FLAG_SORT_PRINT_TRANSPOS_REC ) )
                )
            {
                MolfileSaveCopy( inp_file, sd->fPtrStart, sd->fPtrEnd, prb_file->f, 0 );
            }
#endif
    }

    for (i = 0; i < INCHI_NUM; i++)
    {
        for (k = 0; k < TAUT_NUM + 1; k++)
        {
            FreeCompAtomData( &( HGen->composite_norm_data[i][k] ) );
        }
    }

    /*****************************/

    /* Prepare output message(s). */

    /* Error/warning. */
    if (sd->pStrErrStruct[0])
    {
        if (pGenData && ( pResults->szMessage = (char *) inchi_malloc( strlen( sd->pStrErrStruct ) + 1 ) ))
        {
            strcpy( pResults->szMessage, sd->pStrErrStruct );
        }
    }

    /* InChI, AuxInfo  (go to  pResults->szInChI, pResults->szAuxInfo) */
    if (out_file->s.pStr && out_file->s.nUsedLength > 0 && pGenData)
    {
        char *p;
        pResults->szInChI = out_file->s.pStr;
        pResults->szAuxInfo = NULL;
        if (!( INCHI_OUT_SDFILE_ONLY & ip->bINChIOutputOptions )) /* do not remove last LF from SDF output - 2008-12-23 DT */
        {
            for (p = strchr( pResults->szInChI, '\n' ); p; p = strchr( p + 1, '\n' ))
            {
                if (!memcmp( p, "\nAuxInfo", 8 ))
                {
                    *p = '\0';            /* remove LF after INChI */
                    pResults->szAuxInfo = p + 1; /* save pointer to AuxInfo */
                }
                else
                {
                    if (pResults->szAuxInfo || !p[1])
                    {
                        /* remove LF after aux info or from the last char */
                        *p = '\0';
                        break;
                    }
                }
            }
        }
        out_file->s.pStr = NULL;
    }

    /* Log message. */
    if (log_file->s.pStr && log_file->s.nUsedLength > 0)
    {
        while (log_file->s.nUsedLength && '\n' == log_file->s.pStr[log_file->s.nUsedLength - 1])
            log_file->s.pStr[--log_file->s.nUsedLength] = '\0'; /* remove last LF */
        if (pGenData)
        {
            pResults->szLog = log_file->s.pStr;
            log_file->s.pStr = NULL;
        }
    }

    if (out_file->s.pStr)
    {
        inchi_free( out_file->s.pStr );
        out_file->s.pStr = NULL;
    }
    if (log_file->s.pStr)
    {
        inchi_free( log_file->s.pStr );
        log_file->s.pStr = NULL;
    }

    HGen->ulTotalProcessingTime += sd->ulStructTime;
    nRet = inchi_max( nRet, nRet1 );

    switch (nRet)
    {
        case _IS_FATAL:
        case _IS_ERROR:     HGen->num_err++;
    }

frees:
    /* Free all. */
    for (i = 0; i < MAX_NUM_PATHS; i++)
    {
        if (ip->path[i])
        {
            inchi_free( (char*) ip->path[i] ); /*  cast deliberately discards 'const' qualifier */
            ip->path[i] = NULL;
        }
    }
    SetBitFree( &CG );


    strcpy( pGenData->pStrErrStruct, sd->pStrErrStruct );
    for (k = 0; k < INCHI_NUM; k++)
    {
        pGenData->num_components[k] = sd->num_components[k];
    }

    return retcode;
}


/****************************************************************************

    InChI Generator: reset stage (use before get next structure)

****************************************************************************/


/****************************************************************************/
void INCHI_DECL STDINCHIGEN_Reset( INCHIGEN_HANDLE HGen,
                               INCHIGEN_DATA * pGenData,
                               inchi_Output * pResults )
{
    INCHIGEN_Reset( HGen, pGenData, pResults );
}


/****************************************************************************/
void INCHI_DECL INCHIGEN_Reset( INCHIGEN_HANDLE _HGen,
                                 INCHIGEN_DATA * pGenData,
                                 inchi_Output * pResults )
{
    int i, k, nc;
    INCHIGEN_CONTROL * HGen = (INCHIGEN_CONTROL *) _HGen;

    if (pResults->szInChI)
    {
        inchi_free( pResults->szInChI );
    }
    if (pResults->szLog)
    {
        inchi_free( pResults->szLog );
    }
    if (pResults->szMessage)
    {
        inchi_free( pResults->szMessage );
    }

    /* Free all data associated with components of disconn/conn structures */
    if (NULL != HGen)
    {

        /* Re-initialize output streams/string buffers */
        inchi_ios_close( &( HGen->inchi_file[0] ) );
        inchi_ios_close( &( HGen->inchi_file[1] ) );
        inchi_ios_close( &( HGen->inchi_file[2] ) );
        inchi_ios_init( &( HGen->inchi_file[0] ), INCHI_IOS_TYPE_STRING, NULL );
        inchi_ios_init( &( HGen->inchi_file[1] ), INCHI_IOS_TYPE_STRING, NULL );
        inchi_ios_init( &( HGen->inchi_file[2] ), INCHI_IOS_TYPE_STRING, NULL );

        inchi_strbuf_reset( &( HGen->strbuf_container ) );

        for (i = 0; i < MAX_NUM_PATHS; i++)
        {
            if (HGen->InpParms.path[i])
            {
                inchi_free( (char*) HGen->InpParms.path[i] ); /*  cast deliberately discards 'const' qualifier */
                HGen->InpParms.path[i] = NULL;
            }
        }
        memset( &( HGen->InpParms ), 0, sizeof( INPUT_PARMS ) );

        FreeOrigAtData( &( HGen->OrigInpData ) );
        memset( &( HGen->OrigInpData ), 0, sizeof( HGen->OrigInpData ) );


        FreeOrigAtData( &( HGen->PrepInpData[0] ) );
        FreeOrigAtData( &( HGen->PrepInpData[1] ) );
        memset( &( HGen->PrepInpData[0] ), 0, 2 * sizeof( HGen->PrepInpData[0] ) );

        OrigStruct_Free( &( HGen->OrigStruct ) );
        memset( &( HGen->OrigStruct ), 0, sizeof( HGen->OrigStruct ) );

        for (i = 0; i < INCHI_NUM; i++)
        {
            for (k = 0; k < TAUT_NUM + 1; k++)
            {
                FreeCompAtomData( &( HGen->composite_norm_data[i][k] ) );
            }
        }

        for (k = 0; k < INCHI_NUM; k++)
        {
            nc = HGen->StructData.num_components[k];

            if (HGen->InpCurAtData[k])
            {
                for (i = 0; i < nc; i++)
                {
                    FreeInpAtomData( &( HGen->InpCurAtData[k][i] ) );
                }
                inchi_free( HGen->InpCurAtData[k] );
                HGen->InpCurAtData[k] = NULL;
            }

            if (HGen->cti[k])
            {
                if (( HGen->cti[k] )->at[TAUT_YES])
                {
                    inchi_free( ( HGen->cti[k] )->at[TAUT_YES] );
                    ( HGen->cti[k] )->at[TAUT_YES] = NULL;
                }

                if (( HGen->cti[k] )->at[TAUT_NON])
                {
                    inchi_free( ( HGen->cti[k] )->at[TAUT_NON] );
                    ( HGen->cti[k] )->at[TAUT_NON] = NULL;
                }

                if (&( ( HGen->cti[k] )->vt_group_info ))
                {
                    free_t_group_info( &( ( HGen->cti[k] )->vt_group_info ) );
                }

                if (&( ( HGen->cti[k] )->vt_group_info_orig ))
                {
                    free_t_group_info( &( ( HGen->cti[k] )->vt_group_info_orig ) );
                }

                inchi_free( HGen->cti[k] );
                HGen->cti[k] = NULL;
            }
        }

        for (k = 0; k < INCHI_NUM; k++)
        {
            nc = HGen->StructData.num_components[k];
            if (HGen->InpNormAtData[k])
            {
                for (i = 0; i < nc; i++)
                {
                    FreeInpAtomData( &( HGen->InpNormAtData[k][i] ) );
                }
                inchi_free( HGen->InpNormAtData[k] );
                HGen->InpNormAtData[k] = NULL;
            }

            if (HGen->InpNormTautData[k])
            {
                for (i = 0; i < nc; i++)
                {
                    FreeInpAtomData( &( HGen->InpNormTautData[k][i] ) );
                }
                inchi_free( HGen->InpNormTautData[k] );
                HGen->InpNormTautData[k] = NULL;
            }

            if (pGenData->NormAtomsTaut[k])
            {
                /*
                for ( i = 0;  i < nc; i ++ )
                    FreeInpAtomData( &(pGenData->NormAtomsTaut[k][i]) );
                */
                inchi_free( pGenData->NormAtomsTaut[k] );
                pGenData->NormAtomsTaut[k] = NULL;
            }
            if (pGenData->NormAtomsNontaut[k])
            {
                /* for ( i = 0;  i < nc; i ++ )
                    FreeInpAtomData( &(pGenData->NormAtomsNontaut[k][i]) );
                */
                inchi_free( pGenData->NormAtomsNontaut[k] );
                pGenData->NormAtomsNontaut[k] = NULL;
            }
        }

        /*  free INChI memory */
        FreeAllINChIArrays( HGen->pINChI, HGen->pINChI_Aux, HGen->StructData.num_components );
        memset( HGen->pINChI, 0, sizeof( HGen->pINChI ) );
        memset( HGen->pINChI_Aux, 0, sizeof( HGen->pINChI_Aux ) );

        HGen->szTitle[0] = '\0';
    }

    memset( &( HGen->StructData ), 0, sizeof( STRUCT_DATA ) );
    memset( pResults, 0, sizeof( *pResults ) );
    memset( pGenData, 0, sizeof( *pGenData ) );

    return;
}



/****************************************************************************

    InChI Generator: destroy generator

****************************************************************************/


/****************************************************************************/
void INCHI_DECL STDINCHIGEN_Destroy( INCHIGEN_HANDLE HGen )
{
    INCHIGEN_Destroy( HGen );
}


/****************************************************************************/
void INCHI_DECL INCHIGEN_Destroy( INCHIGEN_HANDLE _HGen )
{
    INCHIGEN_CONTROL * HGen = (INCHIGEN_CONTROL *) _HGen;

    if (NULL != HGen)
    {
        inchi_strbuf_close( &( HGen->strbuf_container ) );

        inchi_ios_close( &( HGen->inchi_file[0] ) );
        inchi_ios_close( &( HGen->inchi_file[1] ) );
        inchi_ios_close( &( HGen->inchi_file[2] ) );

        inchi_free( HGen );
    }
}


#if( defined( _WIN32 ) && defined( _MSC_VER ) && _MSC_VER >= 800 && defined(_USRDLL) && defined(BUILD_LINK_AS_DLL) )
    /* Win32 & MS VC ++, compile and link as a DLL */
/*********************************************************/
/*   C calling conventions export from Win32 dll         */
/*********************************************************/
/* prototypes */
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif
    INCHIGEN_HANDLE cdecl_INCHIGEN_Create( void );
    INCHIGEN_HANDLE cdecl_STDINCHIGEN_Create( void );
    int  cdecl_INCHIGEN_Setup( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Input * pInp );
    int  cdecl_STDINCHIGEN_Setup( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Input * pInp );
    int  cdecl_INCHIGEN_DoNormalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData );
    int  cdecl_STDINCHIGEN_DoNormalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData );
    int  cdecl_INCHIGEN_DoCanonicalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData );
    int  cdecl_STDINCHIGEN_DoCanonicalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData );
    int  cdecl_INCHIGEN_DoSerialization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Output * pResults );
    int  cdecl_STDINCHIGEN_DoSerialization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Output * pResults );
    void cdecl_INCHIGEN_Reset( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Output * pResults );
    void cdecl_STDINCHIGEN_Reset( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Output * pResults );
    void cdecl_INCHIGEN_Destroy( INCHIGEN_HANDLE HGen );
    void cdecl_STDINCHIGEN_Destroy( INCHIGEN_HANDLE HGen );
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


/* Implementation */
/* libinchi.def provides export without cdecl_ prefixes */


/****************************************************************************/
INCHIGEN_HANDLE cdecl_INCHIGEN_Create( void )
{
    return INCHIGEN_Create( );
}


/****************************************************************************/
INCHIGEN_HANDLE cdecl_STDINCHIGEN_Create( void )
{
    return STDINCHIGEN_Create( );
}


/****************************************************************************/
int cdecl_INCHIGEN_Setup( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Input * pInp )
{
    return INCHIGEN_Setup( HGen, pGenData, pInp );
}


/****************************************************************************/
int cdecl_STDINCHIGEN_Setup( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Input * pInp )
{
    return STDINCHIGEN_Setup( HGen, pGenData, pInp );
}


/****************************************************************************/
int cdecl_INCHIGEN_DoNormalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData )
{
    return INCHIGEN_DoNormalization( HGen, pGenData );
}


/****************************************************************************/
int cdecl_STDINCHIGEN_DoNormalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData )
{
    return STDINCHIGEN_DoNormalization( HGen, pGenData );
}

/****************************************************************************/
int cdecl_INCHIGEN_DoCanonicalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData )
{
    return INCHIGEN_DoCanonicalization( HGen, pGenData );
}


/****************************************************************************/
int cdecl_STDINCHIGEN_DoCanonicalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData )
{
    return STDINCHIGEN_DoCanonicalization( HGen, pGenData );
}


/****************************************************************************/
int cdecl_INCHIGEN_DoSerialization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Output * pResults )
{
    return INCHIGEN_DoSerialization( HGen, pGenData, pResults );
}


/****************************************************************************/
int cdecl_STDINCHIGEN_DoSerialization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Output * pResults )
{
    return STDINCHIGEN_DoSerialization( HGen, pGenData, pResults );
}


/****************************************************************************/
void cdecl_INCHIGEN_Reset( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData, inchi_Output *pResults )
{
    INCHIGEN_Reset( HGen, pGenData, pResults );
}


/****************************************************************************/
void cdecl_STDINCHIGEN_Reset( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData, inchi_Output *pResults )
{
    STDINCHIGEN_Reset( HGen, pGenData, pResults );
}


/****************************************************************************/
void cdecl_INCHIGEN_Destroy( INCHIGEN_HANDLE HGen )
{
    INCHIGEN_Destroy( HGen );
}


/****************************************************************************/
void cdecl_STDINCHIGEN_Destroy( INCHIGEN_HANDLE HGen )
{
    STDINCHIGEN_Destroy( HGen );
}
#endif


#if( defined(__GNUC__) && __GNUC__ >= 3 && defined(__MINGW32__) && defined(_WIN32) )
#include <windows.h>
/*
    Pascal calling conventions export from Win32 dll
*/

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

/* Prototypes */
    INCHIGEN_HANDLE PASCAL pasc_INCHIGEN_Create( void );
    INCHIGEN_HANDLE PASCAL pasc_STDINCHIGEN_Create( void );
    int  PASCAL pasc_INCHIGEN_Setup( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Input * pInp );
    int  PASCAL pasc_STDINCHIGEN_Setup( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Input * pInp );
    int  PASCAL pasc_INCHIGEN_DoNormalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData );
    int  PASCAL pasc_STDINCHIGEN_DoNormalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData );
    int  PASCAL pasc_INCHIGEN_DoCanonicalization( INCHIGEN_HANDLE _HGen, INCHIGEN_DATA *pGenData );
    int  PASCAL pasc_STDINCHIGEN_DoCanonicalization( INCHIGEN_HANDLE _HGen, INCHIGEN_DATA *pGenData );
    int  PASCAL pasc_INCHIGEN_DoSerialization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Output * pResults );
    int  PASCAL pasc_STDINCHIGEN_DoSerialization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Output * pResults );
    void PASCAL pasc_INCHIGEN_Reset( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData, inchi_Output *pResults );
    void PASCAL pasc_STDINCHIGEN_Reset( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData, inchi_Output *pResults );
    void PASCAL pasc_INCHIGEN_Destroy( INCHIGEN_HANDLE HGen );
    void PASCAL pasc_STDINCHIGEN_Destroy( INCHIGEN_HANDLE HGen );
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

/* Implementation */
/* libinchi.def provides export without PASCAL pasc_ prefixes */

INCHIGEN_HANDLE PASCAL pasc_INCHIGEN_Create( void )
{
    return INCHIGEN_Create( );
}
INCHIGEN_HANDLE PASCAL pasc_STDINCHIGEN_Create( void )
{
    return STDINCHIGEN_Create( );
}
int PASCAL pasc_INCHIGEN_Setup( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Input * pInp )
{
    return INCHIGEN_Setup( HGen, pGenData, pInp );
}
int PASCAL pasc_STDINCHIGEN_Setup( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Input * pInp )
{
    return STDINCHIGEN_Setup( HGen, pGenData, pInp );
}
int PASCAL pasc_INCHIGEN_DoNormalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData )
{
    return INCHIGEN_DoNormalization( HGen, pGenData );
}
int PASCAL pasc_STDINCHIGEN_DoNormalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData )
{
    return STDINCHIGEN_DoNormalization( HGen, pGenData );
}
int PASCAL pasc_INCHIGEN_DoCanonicalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData )
{
    return INCHIGEN_DoCanonicalization( HGen, pGenData );
}
int PASCAL pasc_STDINCHIGEN_DoCanonicalization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData )
{
    return STDINCHIGEN_DoCanonicalization( HGen, pGenData );
}
int PASCAL pasc_INCHIGEN_DoSerialization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Output * pResults )
{
    return INCHIGEN_DoSerialization( HGen, pGenData, pResults );
}
int PASCAL pasc_STDINCHIGEN_DoSerialization( INCHIGEN_HANDLE HGen, INCHIGEN_DATA * pGenData, inchi_Output * pResults )
{
    return STDINCHIGEN_DoSerialization( HGen, pGenData, pResults );
}
void PASCAL pasc_INCHIGEN_Reset( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData, inchi_Output *pResults )
{
    INCHIGEN_Reset( HGen, pGenData, pResults );
}
void PASCAL pasc_STDINCHIGEN_Reset( INCHIGEN_HANDLE HGen, INCHIGEN_DATA *pGenData, inchi_Output *pResults )
{
    STDINCHIGEN_Reset( HGen, pGenData, pResults );
}
void PASCAL pasc_INCHIGEN_Destroy( INCHIGEN_HANDLE HGen )
{
    INCHIGEN_Destroy( HGen );
}
void PASCAL pasc_STDINCHIGEN_Destroy( INCHIGEN_HANDLE HGen )
{
    STDINCHIGEN_Destroy( HGen );
}

#endif
