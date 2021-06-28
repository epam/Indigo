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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <math.h>

#include "../../../INCHI_BASE/src/mode.h"
#include "../../../INCHI_BASE/src/inchi_api.h"
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
#include "../../../INCHI_BASE/src/ichicomp.h"
#include "../../../INCHI_BASE/src/ichitime.h"
#include "../../../INCHI_BASE/src/ichicant.h"
#include "../../../INCHI_BASE/src/readinch.h"

#include "inchi_dll.h"
#include "inchi_dll_b.h"


static
int PrepareToMakeINCHI( STRUCT_DATA *sd,
                        INPUT_PARMS *ip,
                        ORIG_ATOM_DATA *orig_inp_data,
                        ORIG_ATOM_DATA *prep_inp_data,
                        PINChI2 *pINChI[INCHI_NUM],
                        PINChI_Aux2 *pINChI_Aux[INCHI_NUM],
                        INCHI_IOSTREAM *pout,
                        INCHI_IOSTREAM *plog,
                        INCHI_IOSTREAM *pprb,
                        INCHI_IOSTREAM *inp_file,
                        const char *moltext,
                        char *options,
                        INCHI_IOS_STRING *strbuf );
static
int PostMakeINCHICleanup( struct tagCANON_GLOBALS *pCG,
                          STRUCT_DATA *sd,
                          INPUT_PARMS *ip,
                          ORIG_ATOM_DATA *orig_inp_data,
                          ORIG_ATOM_DATA *prep_inp_data,
                          PINChI2 *pINChI[INCHI_NUM],
                          PINChI_Aux2 *pINChI_Aux[INCHI_NUM],
                          INCHI_IOSTREAM *pout,
                          INCHI_IOSTREAM *plog,
                          INCHI_IOSTREAM *pprb,
                          INCHI_IOSTREAM *inp_file,
                          const char *moltext,
                          INCHI_IOS_STRING *strbuf );



/****************************************************************************/
void FreeInchi_Atom( inchi_Atom **at )
{
    if (at && *at)
    {
        inchi_free( *at );
        *at = NULL;
    }
}


/****************************************************************************/
inchi_Atom *CreateInchiAtom( int num_atoms )
{
    inchi_Atom *p = (inchi_Atom*) inchi_calloc( num_atoms, sizeof( inchi_Atom ) );
    return p;
}



/*****************************************************************************/
EXPIMP_TEMPLATE INCHI_API
int INCHI_DECL MakeINCHIFromMolfileText( const char *moltext,
                                         char *szOptions,
                                         inchi_Output *result )
{
    int retcode = 0, retcode2 = 0;
    long num_inp = 0, num_err = 0;
    char szTitle[MAX_SDF_HEADER + MAX_SDF_VALUE + 256];

    STRUCT_FPTRS *pStructPtrs = NULL;    /* dummy in this context */
    INPUT_PARMS inp_parms;
    INPUT_PARMS *ip = &inp_parms;
    STRUCT_DATA struct_data;
    STRUCT_DATA *sd = &struct_data;
    ORIG_ATOM_DATA OrigAtData;
    ORIG_ATOM_DATA *orig_inp_data = &OrigAtData;
    ORIG_ATOM_DATA PrepAtData[2];
    ORIG_ATOM_DATA *prep_inp_data = PrepAtData;
    PINChI2     *pINChI[INCHI_NUM];
    PINChI_Aux2 *pINChI_Aux[INCHI_NUM];
    INCHI_IOSTREAM outputstr, logstr, prbstr, instr;
    INCHI_IOSTREAM *pout = &outputstr, *plog = &logstr, *pprb = &prbstr, *inp_file = &instr;
    int output_error_inchi = 0;
    int have_err_in_GetOneStructure = 0;

    INCHI_IOS_STRING temp_string_container;
    INCHI_IOS_STRING *strbuf = &temp_string_container;

    CANON_GLOBALS CG;
    INCHI_CLOCK ic;
    memset( &CG, 0, sizeof( CG ) );
    memset( &ic, 0, sizeof( ic ) );

    retcode = PrepareToMakeINCHI( sd, ip, orig_inp_data, prep_inp_data,
                                   pINChI, pINChI_Aux,
                                   pout, plog, pprb, inp_file,
                                   moltext, szOptions, strbuf );

    output_error_inchi =
        ip->bINChIOutputOptions2 & INCHI_OUT_INCHI_GEN_ERROR;

    if (retcode)
    {
        if (plog && plog->s.pStr)
        {
            AddErrorMessage( plog->s.pStr, "Error while preparing to make InChI" );
        }
        retcode = mol2inchi_Ret_ERROR;
        num_err++;
        goto ret;
    }

    have_err_in_GetOneStructure = 0;
    retcode = GetOneStructure( &ic, sd, ip, szTitle,
                               inp_file, plog, pout, pprb,
                               orig_inp_data,
                               &num_inp, pStructPtrs );

    if (retcode == _IS_FATAL || retcode == _IS_ERROR)
    {
        retcode = mol2inchi_Ret_ERROR;
        num_err++;
        have_err_in_GetOneStructure = 1;
        /*
        if ( plog && plog->s.pStr )
                AddErrorMessage( plog->s.pStr, "Error while reading/parsing structure" );
        */
        if (!output_error_inchi)
        {
            goto ret;
        }
    }
    else if (retcode == _IS_EOF)
    {
        goto ret;
    }
    else if (retcode == _IS_SKIP)
    {
        goto ret;
    }

    /* Always enable polymer extensions */
    if (orig_inp_data->polymer)
    {
        orig_inp_data->valid_polymer = 1;
    }

    retcode = ProcessOneStructureEx( &ic, &CG, sd, ip, szTitle,
                                     pINChI, pINChI_Aux,
                                     inp_file, plog, pout, pprb,
                                     orig_inp_data, prep_inp_data,
                                     num_inp, strbuf, 0 /* save_opt_bits */ );


    if (retcode != _IS_FATAL && retcode != _IS_ERROR)
    {
        /* output */
        produce_generation_output( result, sd, ip, plog, pout );
        if (retcode == _IS_WARNING)
        {
            retcode = mol2inchi_Ret_WARNING;
        }
    }
    else
    {
        retcode = mol2inchi_Ret_ERROR;
        num_err++;
        if (output_error_inchi)
        {
            produce_generation_output( result, sd, ip, plog, pout );
            if (!result->szInChI)
            {
                /* As OutErrInchi was requested, we must fill an
                   InChI string anyway, here is the last chance  */
                result->szInChI = (char *) inchi_malloc( 12 * sizeof( char ) );
                if (ip->bINChIOutputOptions & INCHI_OUT_STDINCHI)
                {
                    strcpy( result->szInChI, "InChI=1S//" );
                }
                else
                {
                    strcpy( result->szInChI, "InChI=1//" );
                }
            }
        }
    }

ret:
    retcode2 = PostMakeINCHICleanup( &CG, sd, ip,
                                     orig_inp_data, prep_inp_data,
                                     pINChI, pINChI_Aux,
                                     pout, plog, pprb, inp_file,
                                     moltext, strbuf );
    if (retcode2)
    {
        if (plog && plog->s.pStr)
        {
            AddErrorMessage( plog->s.pStr, "Failed while cleaning things after InChI produced" );
        }
        retcode2 = mol2inchi_Ret_WARNING;
        num_err++;
    }

    copy_corrected_log_tail( result, plog );

    if (retcode < retcode2)
    {
        retcode = retcode2;
    }

    return retcode;
}


/****************************************************************************/
int PrepareToMakeINCHI( STRUCT_DATA *sd,
                        INPUT_PARMS *ip,
                        ORIG_ATOM_DATA *orig_inp_data,
                        ORIG_ATOM_DATA *prep_inp_data,
                        PINChI2 *pINChI[INCHI_NUM],
                        PINChI_Aux2 *pINChI_Aux[INCHI_NUM],
                        INCHI_IOSTREAM *pout,
                        INCHI_IOSTREAM *plog,
                        INCHI_IOSTREAM *pprb,
                        INCHI_IOSTREAM *inp_file,
                        const char *moltext,
                        char *options,
                        INCHI_IOS_STRING *strbuf )
{
    int retcode = 0;
    unsigned long  ulDisplTime = 0;
    int bReleaseVersion = bRELEASE_VERSION;
    char szSdfDataValue[MAX_SDF_VALUE + 1];


    const char *quasi_argv[INCHI_MAX_NUM_ARG + 1];
    int   quasi_argc;
    char *quasi_options = NULL;

    if (options)
    {
        quasi_options = (char*) inchi_malloc( strlen( options ) + 1 );
    }
    if (quasi_options)
    {
        strcpy( quasi_options, options );
        quasi_argc = parse_options_string( quasi_options, quasi_argv, INCHI_MAX_NUM_ARG );
    }
    else
    {
        quasi_argc = 1;
        quasi_argv[0] = "";
        quasi_argv[1] = NULL;
    }

    /* I/O streams */
    inchi_ios_init( pout, INCHI_IOS_TYPE_STRING, NULL );
    inchi_ios_init( plog, INCHI_IOS_TYPE_STRING, NULL );
    inchi_ios_init( pprb, INCHI_IOS_TYPE_STRING, NULL );


    /* input ( string of Molfile )*/
    inchi_ios_init( inp_file, INCHI_IOS_TYPE_STRING, NULL );
    inp_file->s.pStr = (char *) moltext;
    inp_file->s.nPtr = 0;
    inp_file->s.nUsedLength = strlen( moltext ) + 1;
    inp_file->f = NULL;

    memset( szSdfDataValue, 0, sizeof( szSdfDataValue ) );


    /* data structs */
    memset( sd, 0, sizeof( *sd ) );
    memset( ip, 0, sizeof( *ip ) );

    memset( orig_inp_data, 0, sizeof( *orig_inp_data ) );
    memset( prep_inp_data, 0, 2 * sizeof( *prep_inp_data ) );

    pINChI[0] = pINChI[1] = NULL;
    pINChI_Aux[0] = pINChI_Aux[1] = NULL;

    /* Parse command line */
    if (0 > ReadCommandLineParms( quasi_argc,
        quasi_argv,
        ip,
        szSdfDataValue,
        &ulDisplTime,
        bReleaseVersion,
        plog ))
    {
        return  MOL2INCHI_BAD_COMMAND_LINE;
    }

    ip->nInputType = INPUT_MOLFILE;
    ip->bNoStructLabels = 1;
    ip->pSdfLabel = NULL;
    ip->pSdfValue = NULL;
    /* ip->bINChIOutputOptions |= INCHI_OUT_NO_AUX_INFO; */

    /* Supply expandable string buffer */
    if (0 >= inchi_strbuf_init( strbuf, INCHI_STRBUF_INITIAL_SIZE, INCHI_STRBUF_SIZE_INCREMENT ))
    {
        if (plog && plog->s.pStr)
        {
            inchi_ios_eprint( plog, "Cannot allocate output string buffer. Terminating\n" );
        }
        retcode = MOL2INCHI_NO_RAM;
        goto ret;
    }

ret:
    if (quasi_options)
    {
        inchi_free( quasi_options );
    }

    return retcode;
}


/****************************************************************************/
int PostMakeINCHICleanup( struct tagCANON_GLOBALS *pCG,
                          STRUCT_DATA *sd,
                          INPUT_PARMS *ip,
                          ORIG_ATOM_DATA *orig_inp_data,
                          ORIG_ATOM_DATA *prep_inp_data,
                          PINChI2 *pINChI[INCHI_NUM],
                          PINChI_Aux2 *pINChI_Aux[INCHI_NUM],
                          INCHI_IOSTREAM *pout,
                          INCHI_IOSTREAM *plog,
                          INCHI_IOSTREAM *pprb,
                          INCHI_IOSTREAM *inp_file,
                          const char *moltext,
                          INCHI_IOS_STRING *strbuf )
{
    int retcode = 0;
    int i;


    /* Free structure data */

    /*  Free INChI memory */
    FreeAllINChIArrays( pINChI, pINChI_Aux, sd->num_components );

    FreeOrigAtData( orig_inp_data );
    FreeOrigAtData( prep_inp_data );
    FreeOrigAtData( prep_inp_data + 1 );

    inchi_ios_close( pout );
    inchi_ios_close( pprb );

    inchi_strbuf_close( strbuf );

    for (i = 0; i < MAX_NUM_PATHS; i++)
    {
        if (ip->path[i])
        {
            inchi_free( (void*) ip->path[i] );
            /*  cast deliberately discards 'const' qualifier */
            ip->path[i] = NULL;
        }
    }
    SetBitFree( pCG );

    return retcode;
}


/****************************************************************************/
void FreeInchi_Input( inchi_Input *inp_at_data )
{
    FreeInchi_Atom( &inp_at_data->atom );
    FreeInchi_Stereo0D( &inp_at_data->stereo0D );
    memset( inp_at_data, 0, sizeof( *inp_at_data ) );
}


/*****************************************************************************/

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


/****************************************************************************/
int cdecl_MakeINCHIFromMolfileText( const char *moltext,
                                    char *szOptions,
                                    inchi_Output *out );
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

/* implementation */
/* libinchi.def provides export without cdecl_ prefixes */

/****************************************************************************/
int cdecl_MakeINCHIFromMolfileText( const char *moltext,
                                    char *options,
                                    inchi_Output *out )
{
    return
        MakeINCHIFromMolfileText( moltext,
                                  options,
                                  out );
}

#endif


#if( defined(__GNUC__) && __GNUC__ >= 3 && defined(__MINGW32__) && defined(_WIN32) )

#include <windows.h>
/*********************************************************/
/*   Pacal calling conventions export from Win32 dll     */
/*********************************************************/

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

/* prototypes */
    int  PASCAL pasc_MakeINCHIFromMolfileText( const char *moltext,
                                               char *options,
                                               inchi_Output *out );
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

/* implementation */
/* libinchi.def provides export without PASCAL pasc_ prefixes */
/********************************************************/
int  PASCAL pasc_MakeINCHIFromMolfileText( const char *moltext,
                                           char *options,
                                           inchi_Output *out )
{
    return
        MakeINCHIFromMolfileText( moltext,
                                  options,
                                  out );
}

#endif



/*****************************************************************************/
S_SHORT *is_in_the_slist( S_SHORT *pathAtom, S_SHORT nNextAtom, int nPathLen )
{
    for (; nPathLen && *pathAtom != nNextAtom; nPathLen--, pathAtom++)
    {
        ;
    }

    return nPathLen ? pathAtom : NULL;
}


/*****************************************************************************/
int is_element_a_metal( char szEl[] )
{
    static const char szMetals[] = "K;V;Y;W;U;"
        "Li;Be;Na;Mg;Al;Ca;Sc;Ti;Cr;Mn;Fe;Co;Ni;Cu;Zn;Ga;Rb;Sr;Zr;"
        "Nb;Mo;Tc;Ru;Rh;Pd;Ag;Cd;In;Sn;Sb;Cs;Ba;La;Ce;Pr;Nd;Pm;Sm;"
        "Eu;Gd;Tb;Dy;Ho;Er;Tm;Yb;Lu;Hf;Ta;Re;Os;Ir;Pt;Au;Hg;Tl;Pb;"
        "Bi;Po;Fr;Ra;Ac;Th;Pa;Np;Pu;Am;Cm;Bk;Cf;Es;Fm;Md;No;Lr;Rf;";
    const int len = (int) strlen( szEl );
    const char *p;

    if (0 < len && len <= 2 &&
         isalpha( UCINT szEl[0] ) && isupper( szEl[0] ) &&
         ( p = strstr( szMetals, szEl ) ) && p[len] == ';')
    {

        return 1; /*return AtType_Metal;*/
    }

    return 0;
}


/*****************************************************************************/

#define inchi_NUMH2(AT,CUR_AT) ((AT[CUR_AT].num_iso_H[0]>0?AT[CUR_AT].num_iso_H[0]:0) +AT[CUR_AT].num_iso_H[1]+AT[CUR_AT].num_iso_H[2]+AT[CUR_AT].num_iso_H[3])

#define AT_NUM_BONDS(AT)    (AT).num_bonds
#define ATOM_NUMBER         AT_NUM
#define IN_NEIGH_LIST       is_in_the_slist
#define Create_Atom         CreateInchi_Atom
#define AT_BONDS_VAL(AT,I)  AT[I].num_iso_H[0]
#define ISOLATED_ATOM       (-15)
#define NUM_ISO_Hk(AT,I,K)  AT[I].num_iso_H[K+1]
#define IS_METAL_ATOM(AT,I) is_element_a_metal( AT[I].elname )

/*****************************************************************************/



/****************************************************************************/
int InchiToInchiAtom( INCHI_IOSTREAM *inp_file,
                      inchi_Stereo0D **stereo0D,
                      int *num_stereo0D,
                      int bDoNotAddH,
                      int vABParityUnknown,
                      INPUT_TYPE nInputType,
                      inchi_Atom **at,
                      int max_num_at,
                      int *num_dimensions,
                      int *num_bonds,
                      char *pSdfLabel,
                      char *pSdfValue,
                      long *Id,
                      INCHI_MODE *pInpAtomFlags,
                      int *err,
                      char *pStrErr )
{
    int      num_atoms = 0, bFindNext = 0, len, bHeaderRead, bItemIsOver, bErrorMsg, bRestoreInfo;
    int      bFatal = 0, num_struct = 0;
    int      i, k, k2, res, bond_type, bond_stereo1, bond_stereo2, bond_char, neigh, bond_parity, bond_parityNM;
    int      bTooLongLine, res2, bTooLongLine2, pos, hlen, hk;
    long     longID;
    char     szLine[INCHI_LINE_LEN], szNextLine[INCHI_LINE_ADD], *p, *q, *s, parity;
    int      b2D = 0, b3D = 0, b23D, nNumBonds = 0, bNonZeroXYZ, bNonMetal;
    int      len_stereo0D = 0, max_len_stereo0D = 0;
    inchi_Stereo0D  *atom_stereo0D = NULL;
    inchi_Atom      *atom = NULL;
    MOL_COORD       *pszCoord = NULL;
    INCHI_MODE InpAtomFlags = 0; /* 0 or FLAG_INP_AT_NONCHIRAL or FLAG_INP_AT_CHIRAL */
    static const char szIsoH[] = "hdt";
    /* plain tags */
    static const char sStructHdrPln[] = "Structure:";
    static const char sStructHdrPlnNoLblVal[] = " is missing";
    static char sStructHdrPlnAuxStart[64] = ""; /*"$1.1Beta/";*/
    static int  lenStructHdrPlnAuxStart = 0;
    static const char sStructHdrPlnRevAt[] = "/rA:";
    static const char sStructHdrPlnRevBn[] = "/rB:";
    static const char sStructHdrPlnRevXYZ[] = "/rC:";
    const  char *sToken;
    int  lToken;

    if (!lenStructHdrPlnAuxStart)
    {
        lenStructHdrPlnAuxStart = sprintf( sStructHdrPlnAuxStart, "AuxInfo=" );
    }

    if (at)
    {

        if (*at && max_num_at)
        {
            memset( *at, 0, max_num_at * sizeof( **at ) );
        }

        if (stereo0D && num_stereo0D)
        {
            if (*stereo0D && *num_stereo0D)
            {
                max_len_stereo0D = *num_stereo0D;
                memset( *stereo0D, 0, max_len_stereo0D * sizeof( **stereo0D ) );
            }
            else
            {
                max_len_stereo0D = 0;
            }
        }
    }
    else  /* if ( at )  */
    {
        bFindNext = 1;
    }

    bHeaderRead = bErrorMsg = bRestoreInfo = 0;
    *num_dimensions = *num_bonds = 0;

    /*************************************************************/
    /*   extract reversibility info from plain text INChI format */
    /*************************************************************/

    if (nInputType == INPUT_INCHI_PLAIN)
    {

        bHeaderRead = hk = 0;

        while (0 < ( res = inchi_ios_getsTab( szLine, sizeof( szLine ) - 1, inp_file, &bTooLongLine ) ))
        {

            /********************* find and interpret structure header ************/
            if (!bTooLongLine &&
                ( hlen = sizeof( sStructHdrPln ) - 1, !memcmp( szLine, sStructHdrPln, hlen ) ))
            {
                p = szLine + hlen;
                longID = 0;
                num_atoms = 0;

                /* structure number */
                longID = strtol( p, &q, 10 );
                if (q && q[0] == '.' && q[1] == ' ')
                    p = q + 2;
                p = p + strspn( p, " \n\r" );

                if (pSdfLabel)
                {
                    pSdfLabel[0] = '\0';
                }

                if (pSdfValue)
                {
                    pSdfValue[0] = '\0';
                }


                if (*p)
                {
                    /* has label name */

                    /*p ++;*/
                    if (q = strchr( p, '=' ))
                    {

                        /* '=' separates label name from the value */
                        len = inchi_min( q - p + 1, MAX_SDF_HEADER - 1 );

                        if (pSdfLabel)
                        {
                            mystrncpy( pSdfLabel, p, len );
                            lrtrim( pSdfLabel, &len );
                        }

                        p = q + 1;
                        q = p + (int) strlen( p );

                        if (q - p > 0)
                        {
                            len = inchi_min( q - p + 1, MAX_SDF_VALUE - 1 );
                            if (pSdfValue)
                            {
                                mystrncpy( pSdfValue, p, len );
                            }
                            p = q;
                        }
                    }
                    else if (q = strstr( p, sStructHdrPlnNoLblVal ))
                    {
                        len = inchi_min( q - p + 1, MAX_SDF_HEADER - 1 );
                        if (pSdfLabel)
                        {
                            mystrncpy( pSdfLabel, p, len );
                        }
                        p = q + 1;
                    }
                }

                if (Id)
                {
                    *Id = longID;
                }

                bHeaderRead = 1;
                bErrorMsg = bRestoreInfo = 0;
            }
            else if (!memcmp( szLine, sStructHdrPlnAuxStart, lenStructHdrPlnAuxStart ))
            {
                /* found the header of the AuxInfo, read AuxInfo head of the line */

                if (!bHeaderRead)
                {
                    longID = 0;
                    if (Id)
                    {
                        *Id = longID;
                    }
                    if (pSdfLabel)
                    {
                        pSdfLabel[0] = '\0';
                    }
                    if (pSdfValue)
                    {
                        pSdfValue[0] = '\0';
                    }
                }

                bHeaderRead = 0;

                /* check for empty "AuxInfo=ver//" */

                p = strchr( szLine + lenStructHdrPlnAuxStart, '/' );

                if (p && p[1] == '/' && ( !p[2] || '\n' == p[2] ))
                {
                    goto bypass_end_of_INChI_plain;
                }

                /***************** search for atoms block (plain) **********************/

                p = szLine;
                sToken = sStructHdrPlnRevAt;
                lToken = sizeof( sStructHdrPlnRevAt ) - 1;

                /* search for sToken in the line; load next segments of the line if sToken has not found */

                p = FindToken( inp_file, &bTooLongLine, sToken, lToken,
                               szLine, sizeof( szLine ), p, &res );

                if (!p)
                {
                    *err = INCHI_INP_ERROR_ERR;
                    num_atoms = INCHI_INP_ERROR_RET;
                    TREAT_ERR( *err, 0, "Missing atom data" );
                    goto bypass_end_of_INChI_plain;
                }
                else
                {
                    /* atoms block started */

                    i = 0;
                    res2 = bTooLongLine2 = -1;
                    bItemIsOver = ( s = strchr( p, '/' ) ) || !bTooLongLine;

                    while (1)
                    {

                        p = LoadLine( inp_file, &bTooLongLine, &bItemIsOver, &s,
                                      szLine, sizeof( szLine ), INCHI_LINE_ADD, p, &res );

                        if (!i)
                        {
                            /* allocate atom */
                            num_atoms = strtol( p, &q, 10 );

                            if (!num_atoms || !q || !*q)
                            {
                                num_atoms = 0; /* no atom data */
                                goto bypass_end_of_INChI_plain;
                            }
                            p = q;

                            /* Molfile chirality flag */
                            switch (*p)
                            {
                                case 'c':
                                    InpAtomFlags |= FLAG_INP_AT_CHIRAL;
                                    p++;
                                    break;
                                case 'n':
                                    InpAtomFlags |= FLAG_INP_AT_NONCHIRAL;
                                    p++;
                                    break;
                            }

                            if (at && *at)
                            {
                                if (num_atoms > max_num_at)
                                {
                                    inchi_free( *at );
                                    *at = NULL;
                                }
                                else
                                {
                                    memset( *at, 0, max_num_at * sizeof( **at ) );
                                    atom = *at;
                                }
                            }

                            if (!at || !*at)
                            {

                                atom = CreateInchiAtom( num_atoms + 1 );

                                if (!atom)
                                {
                                    num_atoms = INCHI_INP_FATAL_RET; /* was -1; error */
                                    *err = INCHI_INP_FATAL_ERR;
                                    TREAT_ERR( *err, 0, "Out of RAM" );
                                    goto bypass_end_of_INChI_plain;
                                }
                            }

                            if (stereo0D && *stereo0D)
                            {
                                if (num_atoms > max_len_stereo0D)
                                {
                                    FreeInchi_Stereo0D( stereo0D );
                                }
                                else
                                {
                                    memset( *stereo0D, 0, max_len_stereo0D * sizeof( **stereo0D ) );
                                    atom_stereo0D = *stereo0D;
                                }
                            }

                            if (!stereo0D || !*stereo0D)
                            {
                                max_len_stereo0D = num_atoms + 1;

                                atom_stereo0D = CreateInchi_Stereo0D( max_len_stereo0D );

                                if (!atom_stereo0D)
                                {
                                    num_atoms = INCHI_INP_FATAL_RET; /* fatal error: cannot allocate */
                                    *err = INCHI_INP_FATAL_ERR;
                                    TREAT_ERR( *err, 0, "Out of RAM" );
                                    goto bypass_end_of_INChI_plain;
                                }
                            }
                        }

                        /* element, first char */
                        if (!isalpha( UCINT *p ) || !isupper( UCINT *p ) || i >= num_atoms)
                        {
                            break; /* end of atoms block */
                        }

                        atom[i].elname[0] = *p++;

                        /* element, second char */
                        if (isalpha( UCINT *p ) && islower( UCINT *p ))
                        {
                            atom[i].elname[1] = *p++;
                        }

                        /* bonds' valence + number of non-isotopic H */
                        if (isdigit( UCINT *p ))
                        {
                            AT_BONDS_VAL( atom, i ) = (char) strtol( p, &q, 10 );
                            if (!AT_BONDS_VAL( atom, i ))
                                AT_BONDS_VAL( atom, i ) = ISOLATED_ATOM; /* same convention as in MOLfile, found zero bonds valence */
                            p = q;
                        }

                        /* charge */
                        atom[i].charge = ( *p == '+' ) ? 1 : ( *p == '-' ) ? -1 : 0;
                        if (atom[i].charge)
                        {
                            p++;
                            if (isdigit( UCINT *p ))
                            {
                                atom[i].charge *= (S_CHAR) ( strtol( p, &q, 10 ) & CHAR_MASK );
                                p = q;
                            }
                        }

                        /* radical */
                        if (*p == '.')
                        {
                            p++;
                            if (isdigit( UCINT *p ))
                            {
                                atom[i].radical = (S_CHAR) strtol( p, &q, 10 );
                                p = q;
                            }
                        }

                        /* isotopic mass */
                        if (*p == 'i')
                        {
                            p++;
                            if (isdigit( UCINT *p ))
                            {
                                int mw = strtol( p, &q, 10 );
                                p = q;

                                atom[i].isotopic_mass = mw;
                            }
                        }

                        /* parity */
                        switch (*p)
                        {
                            case 'o':
                                parity = INCHI_PARITY_ODD;
                                p++;
                                break;
                            case 'e':
                                parity = INCHI_PARITY_EVEN;
                                p++;
                                break;
                            case 'u':
                                parity = INCHI_PARITY_UNKNOWN;
                                p++;
                                break;
                            case '?':
                                parity = INCHI_PARITY_UNDEFINED;
                                p++;
                                break;
                            default:
                                parity = 0;
                                break;
                        }

                        if (parity)
                        {
                            atom_stereo0D[len_stereo0D].central_atom = i;
                            atom_stereo0D[len_stereo0D].parity = parity;
                            atom_stereo0D[len_stereo0D].type = INCHI_StereoType_Tetrahedral;
                            len_stereo0D++;
                        }

                        /* isotopic h, d, t */
                        for (k = 0; k < NUM_H_ISOTOPES; k++)
                        {
                            if (*p == szIsoH[k])
                            {
                                NUM_ISO_Hk( atom, i, k ) = 1;
                                p++;
                                if (isdigit( UCINT *p ))
                                {
                                    NUM_ISO_Hk( atom, i, k ) = (char) strtol( p, &q, 10 );
                                    p = q;
                                }
                            }
                        }

                        i++;
                    }


                    if (!bItemIsOver || i != num_atoms || s && p != s)
                    {
                        num_atoms = INCHI_INP_ERROR_RET; /* error */
                        *err = INCHI_INP_ERROR_ERR;
                        TREAT_ERR( *err, 0, "Wrong number of atoms" );
                        goto bypass_end_of_INChI_plain;
                    }
                }

                /***************** search for bonds block (plain) and read it *****************/

                /*p = szLine;*/
                sToken = sStructHdrPlnRevBn;
                lToken = sizeof( sStructHdrPlnRevBn ) - 1;

                /* search for sToken in the line; load next segments of the line if sToken has not found */

                p = FindToken( inp_file, &bTooLongLine, sToken, lToken,
                               szLine, sizeof( szLine ), p, &res );

                if (!p)
                {
                    num_atoms = INCHI_INP_ERROR_RET; /* error */
                    *err = INCHI_INP_ERROR_ERR;
                    TREAT_ERR( *err, 0, "Missing bonds data" );
                    goto bypass_end_of_INChI_plain;
                }
                else
                {
                    /* bonds block started */

                    i = 1;

                    res2 = bTooLongLine2 = -1;

                    bItemIsOver = ( s = strchr( p, '/' ) ) || !bTooLongLine;

                    if (1 == num_atoms)
                    {
                        /* needed because the next '/' may be still out of szLine */

                        p = LoadLine( inp_file, &bTooLongLine, &bItemIsOver, &s,
                                      szLine, sizeof( szLine ), INCHI_LINE_ADD, p, &res );
                    }

                    while (i < num_atoms)
                    {

                        p = LoadLine( inp_file, &bTooLongLine, &bItemIsOver, &s,
                                      szLine, sizeof( szLine ), INCHI_LINE_ADD, p, &res );

                        if (i >= num_atoms || s && p >= s)
                        {
                            break; /* end of bonds (plain) */
                        }

                        /* bond, first char */
                        if (*p == ';')
                        {
                            p++;
                            i++;
                            continue;
                        }

                        if (!isalpha( UCINT *p ))
                        {
                            num_atoms = INCHI_INP_ERROR_RET; /* error */
                            *err = INCHI_INP_ERROR_ERR;
                            TREAT_ERR( *err, 0, "Wrong bonds data" );
                            goto bypass_end_of_INChI_plain;
                        }

                        bond_char = *p++;

                        /* bond parity */
                        switch (*p)
                        {
                            case '-':
                                bond_parity = INCHI_PARITY_ODD;
                                p++;
                                break;
                            case '+':
                                bond_parity = INCHI_PARITY_EVEN;
                                p++;
                                break;
                            case 'u':
                                bond_parity = INCHI_PARITY_UNKNOWN;
                                p++;
                                break;
                            case '?':
                                bond_parity = INCHI_PARITY_UNDEFINED;
                                p++;
                                break;
                            default:
                                bond_parity = 0;
                                break;
                        }

                        if (bond_parity)
                        {
                            switch (*p)
                            {
                                case '-':
                                    bond_parityNM = INCHI_PARITY_ODD;
                                    p++;
                                    break;
                                case '+':
                                    bond_parityNM = INCHI_PARITY_EVEN;
                                    p++;
                                    break;
                                case 'u':
                                    bond_parityNM = INCHI_PARITY_UNKNOWN;
                                    p++;
                                    break;
                                case '?':
                                    bond_parityNM = INCHI_PARITY_UNDEFINED;
                                    p++;
                                    break;
                                default:
                                    bond_parityNM = 0;
                                    break;
                            }
                        }
                        else
                        {
                            bond_parityNM = 0;
                        }

                        /* neighbor of the current atom */
                        if (!isdigit( UCINT *p ))
                        {
                            num_atoms = INCHI_INP_ERROR_RET; /* error */
                            *err = INCHI_INP_ERROR_ERR;
                            TREAT_ERR( *err, 0, "Wrong bonds data" );
                            goto bypass_end_of_INChI_plain;
                        }

                        neigh = (int) strtol( p, &q, 10 ) - 1;

                        if (i >= num_atoms || neigh >= num_atoms)
                        {
                            num_atoms = INCHI_INP_ERROR_RET; /* error */
                            *err = INCHI_INP_ERROR_ERR;
                            TREAT_ERR( *err, 0, "Bond to nonexistent atom" );
                            goto bypass_end_of_INChI_plain;
                        }

                        p = q;
                        bond_stereo1 = bond_stereo2 = 0;

                        /* bond type & 2D stereo */
                        switch (bond_char)
                        {
                            case 'v':
                                bond_type = INCHI_BOND_TYPE_SINGLE;
                                bond_stereo1 = INCHI_BOND_STEREO_SINGLE_1EITHER;
                                bond_stereo2 = INCHI_BOND_STEREO_SINGLE_2EITHER;
                                break;
                            case 'V':
                                bond_type = INCHI_BOND_TYPE_SINGLE;
                                bond_stereo1 = INCHI_BOND_STEREO_SINGLE_2EITHER;
                                bond_stereo2 = INCHI_BOND_STEREO_SINGLE_1EITHER;
                                break;
                            case 'w':
                                bond_type = INCHI_BOND_TYPE_DOUBLE;
                                bond_stereo1 =
                                    bond_stereo2 = INCHI_BOND_STEREO_DOUBLE_EITHER;
                                break;
                            case 's':
                                bond_type = INCHI_BOND_TYPE_SINGLE;
                                break;
                            case 'd':
                                bond_type = INCHI_BOND_TYPE_DOUBLE;
                                break;
                            case 't':
                                bond_type = INCHI_BOND_TYPE_TRIPLE;
                                break;
                            case 'a':
                                bond_type = INCHI_BOND_TYPE_ALTERN;
                                break;
                            case 'p':
                                bond_type = INCHI_BOND_TYPE_SINGLE;
                                bond_stereo1 = INCHI_BOND_STEREO_SINGLE_1UP;
                                bond_stereo2 = INCHI_BOND_STEREO_SINGLE_2UP;
                                break;
                            case 'P':
                                bond_type = INCHI_BOND_TYPE_SINGLE;
                                bond_stereo1 = INCHI_BOND_STEREO_SINGLE_2UP;
                                bond_stereo2 = INCHI_BOND_STEREO_SINGLE_1UP;
                                break;
                            case 'n':
                                bond_type = INCHI_BOND_TYPE_SINGLE;
                                bond_stereo1 = INCHI_BOND_STEREO_SINGLE_1DOWN;
                                bond_stereo2 = INCHI_BOND_STEREO_SINGLE_2DOWN;
                                break;
                            case 'N':
                                bond_type = INCHI_BOND_TYPE_SINGLE;
                                bond_stereo1 = INCHI_BOND_STEREO_SINGLE_2DOWN;
                                bond_stereo2 = INCHI_BOND_STEREO_SINGLE_1DOWN;
                                break;
                            default:
                                num_atoms = INCHI_INP_ERROR_RET; /* error */
                                *err = INCHI_INP_ERROR_ERR;
                                TREAT_ERR( *err, 0, "Wrong bond type" );
                                goto bypass_end_of_INChI_plain;
                        }

                        k = AT_NUM_BONDS( atom[i] )++;

                        atom[i].bond_type[k] = bond_type;
                        atom[i].bond_stereo[k] = bond_stereo1;
                        atom[i].neighbor[k] = (ATOM_NUMBER) neigh;

                        k2 = AT_NUM_BONDS( atom[neigh] )++;
                        atom[neigh].bond_type[k2] = bond_type;
                        atom[neigh].bond_stereo[k2] = bond_stereo2;
                        atom[neigh].neighbor[k2] = (ATOM_NUMBER) i;

                        bond_parity |= ( bond_parityNM << SB_PARITY_SHFT );

                        if (bond_parity)
                        {
                            if (max_len_stereo0D <= len_stereo0D)
                            {
                                /* realloc atom_Stereo0D */

                                inchi_Stereo0D *new_atom_stereo0D = CreateInchi_Stereo0D( max_len_stereo0D + num_atoms );

                                if (!new_atom_stereo0D)
                                {
                                    num_atoms = INCHI_INP_FATAL_RET; /* fatal error: cannot allocate */
                                    *err = INCHI_INP_FATAL_ERR;
                                    TREAT_ERR( *err, 0, "Out of RAM" );
                                    goto bypass_end_of_INChI_plain;
                                }

                                memcpy( new_atom_stereo0D, atom_stereo0D, len_stereo0D * sizeof( *atom_stereo0D ) );
                                FreeInchi_Stereo0D( &atom_stereo0D );
                                atom_stereo0D = new_atom_stereo0D;
                                max_len_stereo0D += num_atoms;
                            }

                            /* (a) i may be allene endpoint and     neigh = allene middle point or
                               (b) i may be allene middle point and neigh = allene endpoint
                               !!!!! CURRENTLY ONLY (b) IS ALLOWED !!!!!
                            */

                            atom_stereo0D[len_stereo0D].neighbor[1] = neigh; /* neigh < i */
                            atom_stereo0D[len_stereo0D].neighbor[2] = i;
                            atom_stereo0D[len_stereo0D].parity = bond_parity;
                            atom_stereo0D[len_stereo0D].type = INCHI_StereoType_DoubleBond; /* incl allenes & cumulenes */
                            len_stereo0D++;
                        }
                    }

                    if (!bItemIsOver || i != num_atoms || s && p != s)
                    {
                        num_atoms = INCHI_INP_ERROR_RET; /* error */
                        *err = INCHI_INP_ERROR_ERR;
                        TREAT_ERR( *err, 0, "Wrong number of bonds" );
                        goto bypass_end_of_INChI_plain;
                    }
                }

                /***************** search for coordinates block (plain) **********************/
                /*p = szLine;*/

                sToken = sStructHdrPlnRevXYZ;
                lToken = sizeof( sStructHdrPlnRevXYZ ) - 1;

                /* search for sToken in the line; load next segments of the line if sToken has not found */

                p = FindToken( inp_file, &bTooLongLine, sToken, lToken,
                               szLine, sizeof( szLine ), p, &res );

                if (!p)
                {
                    num_atoms = INCHI_INP_ERROR_RET; /* error */
                    *err = INCHI_INP_ERROR_ERR;
                    TREAT_ERR( *err, 0, "Missing atom coordinates data" );
                    goto bypass_end_of_INChI_plain;
                }
                else
                {
                    /* coordinates block started */
                    if (pszCoord = (MOL_COORD*) inchi_malloc( inchi_max( num_atoms, 1 ) * sizeof( MOL_COORD ) ))
                    {
                        memset( pszCoord, ' ', inchi_max( num_atoms, 1 ) * sizeof( MOL_COORD ) );
                    }
                    else
                    {
                        num_atoms = INCHI_INP_FATAL_RET; /* allocation error */
                        *err = INCHI_INP_FATAL_ERR;
                        TREAT_ERR( *err, 0, "Out of RAM" );
                        goto bypass_end_of_INChI_plain;
                    }

                    i = 0;
                    res2 = bTooLongLine2 = -1;
                    bItemIsOver = ( s = strchr( p, '/' ) ) || !bTooLongLine;

                    while (i < num_atoms)
                    {

                        p = LoadLine( inp_file, &bTooLongLine, &bItemIsOver, &s,
                                      szLine, sizeof( szLine ), INCHI_LINE_ADD, p, &res );

                        if (i >= num_atoms || s && p >= s)
                        {
                            break; /* end of bonds (plain) */
                        }

                        /* coord, first char */
                        if (*p == ';')
                        {
                            for (k = 0; k < NUM_COORD; k++)
                            {
                                pszCoord[i][LEN_COORD*k + 4] = '0';
                            }
                            p++;
                            i++;
                            continue;
                        }

                        for (k = 0; k < 3; k++)
                        {
                            double xyz;
                            bNonZeroXYZ = 0;
                            if (*p == ';')
                            {
                                pszCoord[i][LEN_COORD*k + 4] = '0';
                                xyz = 0.0;
                            }
                            else
                            {
                                if (*p == ',')
                                {
                                    /* empty */
                                    pszCoord[i][LEN_COORD*k + 4] = '0';
                                    xyz = 0.0;
                                    p++;
                                }
                                else
                                {
                                    xyz = strtod( p, &q );
                                    bNonZeroXYZ = fabs( xyz ) > MIN_BOND_LENGTH;
                                    if (q != NULL)
                                    {
                                        memcpy( pszCoord[i] + LEN_COORD*k, p, q - p );
                                        if (*q == ',')
                                            q++;
                                        p = q;
                                    }
                                    else
                                        pszCoord[i][LEN_COORD*k + 4] = '0';
                                }
                            }

                            switch (k)
                            {
                                case 0:
                                    atom[i].x = xyz;
                                    b2D |= bNonZeroXYZ;
                                    break;
                                case 1:
                                    atom[i].y = xyz;
                                    b2D |= bNonZeroXYZ;
                                    break;
                                case 2:
                                    b3D |= bNonZeroXYZ;
                                    atom[i].z = xyz;
                                    break;
                            }
                        }

                        if (*p == ';')
                        {
                            p++; /* end of this triple of coordinates */
                            i++;
                        }
                        else
                        {
                            num_atoms = INCHI_INP_ERROR_RET; /* error in input data: atoms, bonds & coord must be present together */
                            *err = INCHI_INP_ERROR_ERR;
                            TREAT_ERR( *err, 0, "Wrong atom coordinates data" );
                            goto bypass_end_of_INChI_plain;
                        }
                    }

                    if (!bItemIsOver || s && p != s || i != num_atoms)
                    {
                        num_atoms = INCHI_INP_ERROR_RET; /* error */
                        *err = INCHI_INP_ERROR_ERR;
                        TREAT_ERR( *err, 0, "Wrong number of coordinates" );
                        goto bypass_end_of_INChI_plain;
                    }
                } /* end of coordinates */


                /* set special valences and implicit H (xml) */

                b23D = b2D | b3D;
                b2D = b3D = 0;
                if (at)
                {
                    if (!*at)
                    {
                        int a1, a2, n1, n2, valence;
                        int chem_bonds_valence;
                        int    nX = 0, nY = 0, nZ = 0, nXYZ;
                        *at = atom;

                        /* special valences */

                        for (bNonMetal = 0; bNonMetal < 1; bNonMetal++)
                        {

                            for (a1 = 0; a1 < num_atoms; a1++)
                            {

                                int num_bond_type[MAX_INPUT_BOND_TYPE - MIN_INPUT_BOND_TYPE + 1];

                                memset( num_bond_type, 0, sizeof( num_bond_type ) );

                                valence = AT_BONDS_VAL( atom, a1 ); /*  save atom valence if available */
                                AT_BONDS_VAL( atom, a1 ) = 0;


                                nX = nY = nZ = 0;

                                for (n1 = 0; n1 < AT_NUM_BONDS( atom[a1] ); n1++)
                                {
                                    bond_type = atom[a1].bond_type[n1] - MIN_INPUT_BOND_TYPE;
                                    if (bond_type < 0 || bond_type > MAX_INPUT_BOND_TYPE - MIN_INPUT_BOND_TYPE)
                                    {
                                        bond_type = 0;
                                        TREAT_ERR( *err, 0, "Unknown bond type in InChI aux assigned as a single bond" );
                                    }

                                    num_bond_type[bond_type] ++;
                                    nNumBonds++;
                                    if (b23D)
                                    {
                                        neigh = atom[a1].neighbor[n1];
                                        nX |= ( fabs( atom[a1].x - atom[neigh].x ) > MIN_BOND_LENGTH );
                                        nY |= ( fabs( atom[a1].y - atom[neigh].y ) > MIN_BOND_LENGTH );
                                        nZ |= ( fabs( atom[a1].z - atom[neigh].z ) > MIN_BOND_LENGTH );
                                    }
                                }

                                chem_bonds_valence = 0;
                                for (n1 = 0; MIN_INPUT_BOND_TYPE + n1 <= 3 && MIN_INPUT_BOND_TYPE + n1 <= MAX_INPUT_BOND_TYPE; n1++)
                                {
                                    chem_bonds_valence += ( MIN_INPUT_BOND_TYPE + n1 ) * num_bond_type[n1];
                                }

                                if (MIN_INPUT_BOND_TYPE <= INCHI_BOND_TYPE_ALTERN && INCHI_BOND_TYPE_ALTERN <= MAX_INPUT_BOND_TYPE &&
                                    ( n2 = num_bond_type[INCHI_BOND_TYPE_ALTERN - MIN_INPUT_BOND_TYPE] ))
                                {

                                    /* accept input aromatic bonds for now */

                                    switch (n2)
                                    {
                                        case 2:
                                            chem_bonds_valence += 3;  /* =A- */
                                            break;

                                        case 3:
                                            chem_bonds_valence += 4;  /* =A< */
                                            break;

                                        default:
                                            /*  if 1 or >= 4 aromatic bonds then replace such bonds with single bonds */
                                            for (n1 = 0; n1 < AT_NUM_BONDS( atom[a1] ); n1++)
                                            {
                                                if (atom[a1].bond_type[n1] == INCHI_BOND_TYPE_ALTERN)
                                                {
                                                    ATOM_NUMBER *p1;
                                                    a2 = atom[a1].neighbor[n1];
                                                    p1 = IN_NEIGH_LIST( atom[a2].neighbor, (ATOM_NUMBER) a1, AT_NUM_BONDS( atom[a2] ) );
                                                    if (p1)
                                                    {
                                                        atom[a1].bond_type[n1] =
                                                            atom[a2].bond_type[p1 - atom[a2].neighbor] = INCHI_BOND_TYPE_SINGLE;
                                                    }
                                                    else
                                                    {
                                                        *err = -2;  /*  Program error */
                                                        TREAT_ERR( *err, 0, "Program error interpreting InChI aux" );
                                                        num_atoms = 0;
                                                        goto bypass_end_of_INChI_plain; /*  no structure */
                                                    }
                                                }
                                            }

                                            chem_bonds_valence += n2;
                                            *err |= 32; /*  Unrecognized aromatic bond(s) replaced with single */
                                            TREAT_ERR( *err, 0, "Atom has 1 or more than 3 aromatic bonds" );
                                            break;
                                    }
                                }
                                /********************************
                                 *
                                 *  Set number of hydrogen atoms
                                 */
                                {
                                    int num_iso_H;
                                    num_iso_H = atom[a1].num_iso_H[1] + atom[a1].num_iso_H[2] + atom[a1].num_iso_H[3];
                                    if (valence == ISOLATED_ATOM)
                                    {
                                        atom[a1].num_iso_H[0] = 0;
                                    }
                                    else
                                    {
                                        if (valence && valence >= chem_bonds_valence)
                                        {
                                            atom[a1].num_iso_H[0] = valence - chem_bonds_valence;
                                        }
                                        else
                                        {
                                            if (valence || bDoNotAddH)
                                            {
                                                atom[a1].num_iso_H[0] = 0;
                                            }
                                            else
                                            {
                                                if (!bDoNotAddH)
                                                {
                                                    atom[a1].num_iso_H[0] = -1; /* auto add H */
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        nNumBonds /= 2;

                        if (b23D && nNumBonds)
                        {
                            nXYZ = nX + nY + nZ;
                            b2D = ( nXYZ > 0 );
                            b3D = ( nXYZ == 3 );
                            *num_dimensions = b3D ? 3 : b2D ? 2 : 0;
                            *num_bonds = nNumBonds;
                        }

                        /*======= 0D parities =================================*/
                        if (len_stereo0D > 0 && atom_stereo0D && stereo0D)
                        {
                            *stereo0D = atom_stereo0D;
                            *num_stereo0D = len_stereo0D;
                        }
                        else
                        {
                            FreeInchi_Stereo0D( &atom_stereo0D );
                            *num_stereo0D = len_stereo0D = 0;
                        }

                        for (i = 0; i < len_stereo0D; i++)
                        {
                            ATOM_NUMBER *p1, *p2;
                            int     sb_ord_from_a1 = -1, sb_ord_from_a2 = -1, bEnd1 = 0, bEnd2 = 0;

                            switch (atom_stereo0D[i].type)
                            {

                                case INCHI_StereoType_Tetrahedral:
                                    a1 = atom_stereo0D[i].central_atom;
                                    if (atom_stereo0D[i].parity && ( AT_NUM_BONDS( atom[a1] ) == 3 || AT_NUM_BONDS( atom[a1] ) == 4 ))
                                    {
                                        int ii, kk = 0;
                                        if (AT_NUM_BONDS( atom[a1] ) == 3)
                                            atom_stereo0D[i].neighbor[kk++] = a1;
                                        for (ii = 0; ii < AT_NUM_BONDS( atom[a1] ); ii++)
                                            atom_stereo0D[i].neighbor[kk++] = atom[a1].neighbor[ii];
                                    }

                                    break;

                                case INCHI_StereoType_DoubleBond:
#define MAX_CHAIN_LEN 20
                                    a1 = atom_stereo0D[i].neighbor[1];
                                    a2 = atom_stereo0D[i].neighbor[2];
                                    p1 = IN_NEIGH_LIST( atom[a1].neighbor, (ATOM_NUMBER) a2, AT_NUM_BONDS( atom[a1] ) );
                                    p2 = IN_NEIGH_LIST( atom[a2].neighbor, (ATOM_NUMBER) a1, AT_NUM_BONDS( atom[a2] ) );
                                    if (!p1 || !p2)
                                    {
                                        atom_stereo0D[i].type = INCHI_StereoType_None;
                                        atom_stereo0D[i].central_atom = NO_ATOM;
                                        atom_stereo0D[i].neighbor[0] =
                                            atom_stereo0D[i].neighbor[3] = -1;
                                        *err |= 64; /* Error in cumulene stereo */
                                        TREAT_ERR( *err, 0, "0D stereobond not recognized" );
                                        break;
                                    }

                                    /* streobond, allene, or cumulene */

                                    sb_ord_from_a1 = p1 - atom[a1].neighbor;
                                    sb_ord_from_a2 = p2 - atom[a2].neighbor;

                                    if (AT_NUM_BONDS( atom[a1] ) == 2 &&
                                          atom[a1].bond_type[0] + atom[a1].bond_type[1] == 2 * INCHI_BOND_TYPE_DOUBLE &&
                                          0 == inchi_NUMH2( atom, a1 ) &&
                                          ( AT_NUM_BONDS( atom[a2] ) != 2 ||
                                              atom[a2].bond_type[0] + atom[a2].bond_type[1] != 2 * INCHI_BOND_TYPE_DOUBLE ))
                                    {
                                        bEnd2 = 1; /* a2 is the end-atom, a1 is middle atom */
                                    }

                                    if (AT_NUM_BONDS( atom[a2] ) == 2 &&
                                          atom[a2].bond_type[0] + atom[a2].bond_type[1] == 2 * INCHI_BOND_TYPE_DOUBLE &&
                                          0 == inchi_NUMH2( atom, a2 ) &&
                                          ( AT_NUM_BONDS( atom[a1] ) != 2 ||
                                              atom[a1].bond_type[0] + atom[a1].bond_type[1] != 2 * INCHI_BOND_TYPE_DOUBLE ))
                                    {
                                        bEnd1 = 1; /* a1 is the end-atom, a2 is middle atom */
                                    }

                                    if (bEnd2 + bEnd1 == 1)
                                    {
                                        /* allene or cumulene */

                                        ATOM_NUMBER  chain[MAX_CHAIN_LEN + 1], prev, cur, next;

                                        if (bEnd2 && !bEnd1)
                                        {
                                            cur = a1;
                                            a1 = a2;
                                            a2 = cur;
                                            sb_ord_from_a1 = sb_ord_from_a2;
                                        }

                                        sb_ord_from_a2 = -1;
                                        cur = a1;
                                        next = a2;
                                        len = 0;
                                        chain[len++] = cur;
                                        chain[len++] = next;

                                        while (len < MAX_CHAIN_LEN)
                                        {
                                            /* arbitrary very high upper limit to prevent infinite loop */

                                            prev = cur;
                                            cur = next;
                                                /* follow double bond path && avoid going back */
                                            if (AT_NUM_BONDS( atom[cur] ) == 2 &&
                                                 atom[cur].bond_type[0] + atom[cur].bond_type[1] == 2 * INCHI_BOND_TYPE_DOUBLE &&
                                                 0 == inchi_NUMH2( atom, cur ))
                                            {
                                                next = atom[cur].neighbor[atom[cur].neighbor[0] == prev];
                                                chain[len++] = next;
                                            }
                                            else
                                            {
                                                break;
                                            }
                                        }
                                        if (len > 2 &&
                                            ( p2 = IN_NEIGH_LIST( atom[cur].neighbor, (ATOM_NUMBER) prev, AT_NUM_BONDS( atom[cur] ) ) ))
                                        {
                                            sb_ord_from_a2 = p2 - atom[cur].neighbor;
                                            a2 = cur;
                                            /* by design we need to pick up the first non-stereo-bond-neighbor as "sn"-atom */
                                            atom_stereo0D[i].neighbor[0] = atom[a1].neighbor[sb_ord_from_a1 == 0];
                                            atom_stereo0D[i].neighbor[1] = a1;
                                            atom_stereo0D[i].neighbor[2] = a2;
                                            atom_stereo0D[i].neighbor[3] = atom[a2].neighbor[sb_ord_from_a2 == 0];

                                            if (len % 2)
                                            {
                                                atom_stereo0D[i].central_atom = chain[len / 2];
                                                atom_stereo0D[i].type = INCHI_StereoType_Allene;
                                            }
                                            else
                                            {
                                                atom_stereo0D[i].central_atom = NO_ATOM;
                                            }
                                        }
                                        else
                                        {
                                            /* error */
                                            atom_stereo0D[i].type = INCHI_StereoType_None;
                                            atom_stereo0D[i].central_atom = NO_ATOM;
                                            atom_stereo0D[i].neighbor[0] =
                                                atom_stereo0D[i].neighbor[3] = -1;
                                            *err |= 64; /* Error in cumulene stereo */
                                            TREAT_ERR( *err, 0, "Cumulene stereo not recognized (0D)" );
                                        }
#undef MAX_CHAIN_LEN
                                    }
                                    else
                                    {
                                        /****** a normal possibly stereogenic bond -- not an allene or cumulene *******/
                                        /* by design we need to pick up the first non-stereo-bond-neighbor as "sn"-atom */
                                        sb_ord_from_a1 = p1 - atom[a1].neighbor;
                                        sb_ord_from_a2 = p2 - atom[a2].neighbor;
                                        atom_stereo0D[i].neighbor[0] = atom[a1].neighbor[p1 == atom[a1].neighbor];
                                        atom_stereo0D[i].neighbor[3] = atom[a2].neighbor[p2 == atom[a2].neighbor];
                                        atom_stereo0D[i].central_atom = NO_ATOM;
                                    }

                                    if (atom_stereo0D[i].type != INCHI_StereoType_None &&
                                         sb_ord_from_a1 >= 0 && sb_ord_from_a2 >= 0 &&
                                         ATOM_PARITY_WELL_DEF( SB_PARITY_2( atom_stereo0D[i].parity ) ))
                                    {
                                        /* Detected well-defined disconnected stereo
                                         * locate first non-metal neighbors */

                                        int    a, n, j, /* k,*/ sb_ord, cur_neigh, min_neigh;

                                        for (k = 0; k < 2; k++)
                                        {
                                            a = k ? atom_stereo0D[i].neighbor[2] : atom_stereo0D[i].neighbor[1];
                                            sb_ord = k ? sb_ord_from_a2 : sb_ord_from_a1;
                                            min_neigh = num_atoms;
                                            for (n = j = 0; j < AT_NUM_BONDS( atom[a] ); j++)
                                            {
                                                cur_neigh = atom[a].neighbor[j];
                                                if (j != sb_ord && !IS_METAL_ATOM( atom, cur_neigh ))
                                                {
                                                    min_neigh = inchi_min( cur_neigh, min_neigh );
                                                }
                                            }
                                            if (min_neigh < num_atoms)
                                            {
                                                atom_stereo0D[i].neighbor[k ? 3 : 0] = min_neigh;
                                            }
                                            else
                                            {
                                                TREAT_ERR( *err, 0, "Cannot find non-metal stereobond neighor (0D)" );
                                            }
                                        }
                                    }

                                    break;
                            }
                        }
                        /* end of 0D parities extraction */
/*exit_cycle:;*/
                    }

                    if (pInpAtomFlags)
                    {
                        /* save chirality flag */
                        *pInpAtomFlags |= InpAtomFlags;
                    }
                }
                else if (atom)
                {
                    inchi_free( atom );
                    atom = NULL;
                }

                if (pszCoord)
                {
                    inchi_free( pszCoord );
                    pszCoord = NULL;
                }

                goto bypass_end_of_INChI_plain;
                /*return num_atoms;*/
            }
        }

        if (atom_stereo0D)
        {
            FreeInchi_Stereo0D( &atom_stereo0D );
        }


        /* end of structure reading cycle */

        if (res <= 0)
        {
            if (*err == INCHI_INP_ERROR_ERR)
                return num_atoms;

            *err = INCHI_INP_EOF_ERR;
            return INCHI_INP_EOF_RET; /* no more data */
        }


    bypass_end_of_INChI_plain:

            /* cleanup */
        if (num_atoms == INCHI_INP_ERROR_RET && atom_stereo0D)
        {
            if (stereo0D && *stereo0D == atom_stereo0D)
            {
                *stereo0D = NULL;
                *num_stereo0D = 0;
            }
            FreeInchi_Stereo0D( &atom_stereo0D );
        }

        while (bTooLongLine &&
                0 < inchi_ios_getsTab1( szLine, sizeof( szLine ) - 1, inp_file, &bTooLongLine ))
        {
            ;
        }


        /* cleanup */
        if (!*at)
        {
            if (atom)
            {
                inchi_free( atom );
                atom = NULL;
            }
            if (pszCoord)
            {
                inchi_free( pszCoord );
                pszCoord = NULL;
            }
        }

        return num_atoms;
    }

    /***********************************************************/
    /*   extract reversibility info from xml text INChI format */
    /*                                                         */
    /*   OBSOLETE CODE because InChI output in XML             */
    /*      does not exist anymore. Unsupported.               */
    /*                                                         */
    /***********************************************************/

    if (nInputType == INPUT_INCHI_XML)
    {

        /* xml tags */

        static const char sStructHdrXml[] = "<structure";
        static const char sStructHdrXmlEnd[] = "</structure";
        static const char sStructHdrXmlNumber[] = "number=\"";
        static const char sStructHdrXmlIdName[] = "id.name=\"";
        static const char sStructHdrXmlIdValue[] = "id.value=\"";
        static const char sStructMsgXmlErr[] = "<message type=\"error (no InChI)\" value=\"";
        static const char sStructMsgXmlErrFatal[] = "<message type=\"fatal (aborted)\" value=\"";
        static const char sStructRevXmlRevHdr[] = "<reversibility>";
        static const char sStructRevXmlRevAt[] = "<atoms>";
        static const char sStructRevXmlRevAtEnd[] = "</atoms>";
        static const char sStructRevXmlRevBn[] = "<bonds>";
        static const char sStructRevXmlRevBnEnd[] = "</bonds>";
        static const char sStructRevXmlRevXYZ[] = "<xyz>";
        static const char sStructRevXmlRevXYZEnd[] = "</xyz>";
        static const char sStructAuxXml[] = "<identifier.auxiliary-info";
        static const char sStructAuxXmlEnd[] = "</identifier.auxiliary-info";
        int         bInTheAuxInfo = 0;

        while (0 < ( res = inchi_ios_gets( szLine, sizeof( szLine ) - 1, inp_file, &bTooLongLine ) ))
        {
            /********************* find and interpret structure header ************/

            if (!memcmp( szLine, sStructHdrXml, sizeof( sStructHdrXml ) - 1 ))
            {
                num_struct = 1;
                p = szLine + sizeof( sStructHdrXml ) - 1;
                longID = 0;
                num_atoms = 0;
                /* structure number */
                if (q = strstr( p, sStructHdrXmlNumber ))
                {
                    p = q + sizeof( sStructHdrXmlNumber ) - 1;
                    longID = strtol( p, &q, 10 );
                    if (q && *q == '\"')
                    {
                        p = q + 1;
                    }
                }
                if (pSdfLabel)
                {
                    pSdfLabel[0] = '\0';
                }
                if (pSdfValue)
                {
                    pSdfValue[0] = '\0';
                }
                /* pSdfLabel */
                if (q = strstr( p, sStructHdrXmlIdName ))
                {
                    p = q + sizeof( sStructHdrXmlIdName ) - 1;
                    q = strchr( p, '\"' );
                    if (q)
                    {
                        len = inchi_min( q - p + 1, MAX_SDF_HEADER - 1 );
                        if (pSdfLabel)
                        {
                            mystrncpy( pSdfLabel, p, len );
                        }
                        p = q + 1;
                    }
                }
                /* pSdfValue */
                if (q = strstr( p, sStructHdrXmlIdValue ))
                {
                    p = q + sizeof( sStructHdrXmlIdValue ) - 1;
                    q = strchr( p, '\"' );
                    if (q)
                    {
                        len = inchi_min( q - p + 1, MAX_SDF_VALUE - 1 );
                        if (pSdfValue)
                        {
                            mystrncpy( pSdfValue, p, len );
                        }
                        p = q + 1;
                    }
                }
                if (Id)
                {
                    *Id = longID;
                }
                bHeaderRead = 1;
                bErrorMsg = bRestoreInfo = 0;
            }
            else if (bHeaderRead && ( bFatal = 0, len = sizeof( sStructMsgXmlErr ) - 1, !memcmp( szLine, sStructMsgXmlErr, len ) ) ||
                 bHeaderRead && ( len = sizeof( sStructMsgXmlErrFatal ) - 1, !memcmp( szLine, sStructMsgXmlErrFatal, len ) ) && ( bFatal = 1 ))
            {
                p = szLine + len;
                q = strchr( p, '\"' );
                if (q && !bFindNext)
                {
                    int c;
                    bErrorMsg = 1;
                    pStrErr[0] = '\0';
                    c = *q;
                    *q = '\0';
                    TREAT_ERR( *err, 0, p );
                    *q = c;
                }
                *err = bFatal ? INCHI_INP_FATAL_ERR : INCHI_INP_ERROR_ERR;
                num_atoms = bFatal ? INCHI_INP_FATAL_RET : INCHI_INP_ERROR_RET;
                goto bypass_end_of_INChI;
            }
            else if (bHeaderRead && !memcmp( szLine, sStructAuxXml, sizeof( sStructAuxXml ) - 1 ))
            {
                bInTheAuxInfo = 1;
            }
            else if (bHeaderRead && !memcmp( szLine, sStructAuxXmlEnd, sizeof( sStructAuxXmlEnd ) - 1 ))
            {
                *err = INCHI_INP_ERROR_ERR;
                num_atoms = INCHI_INP_ERROR_RET;
                TREAT_ERR( *err, 0, "Missing reversibility info" );
                goto bypass_end_of_INChI; /* reversibility info not found */
            }
            else if (bHeaderRead && bInTheAuxInfo && !memcmp( szLine, sStructRevXmlRevHdr, sizeof( sStructRevXmlRevHdr ) - 1 ))
            {
                /***********************  atoms xml ***************************/
                num_struct = 1;
                res = inchi_ios_gets( szLine, sizeof( szLine ) - 1, inp_file, &bTooLongLine );
                if (res <= 0)
                {
                    num_atoms = INCHI_INP_EOF_RET; /* no data, probably end of file */
                    *err = INCHI_INP_EOF_ERR;
                    goto bypass_end_of_INChI;
                }
                if (memcmp( szLine, sStructRevXmlRevAt, sizeof( sStructRevXmlRevAt ) - 1 ))
                {
                    bHeaderRead = 0; /* invalid reversibility info; look for another header */
                    continue;
                }
                /* read (the head of) the atoms line */
                res = inchi_ios_gets( szLine, sizeof( szLine ) - 1, inp_file, &bTooLongLine );
                if (res <= 0)
                {
                    num_atoms = INCHI_INP_EOF_RET; /* no data */
                    *err = INCHI_INP_EOF_ERR;
                    goto bypass_end_of_INChI;
                }
                p = szLine;
                num_atoms = strtol( p, &q, 10 );
                if (!num_atoms || !q || !*q)
                {
                    num_atoms = INCHI_INP_EOF_RET; /* no atom data */
                    *err = INCHI_INP_EOF_ERR;
                    goto bypass_end_of_INChI;
                }
                p = q;
                /* Molfile chirality flag */
                switch (*p)
                {
                    case 'c':
                        InpAtomFlags |= FLAG_INP_AT_CHIRAL;
                        p++;
                        break;
                    case 'n':
                        InpAtomFlags |= FLAG_INP_AT_NONCHIRAL;
                        p++;
                        break;
                }
                if (at && *at)
                {
                    if (num_atoms > max_num_at)
                    {
                        inchi_free( *at );
                        *at = NULL;
                    }
                    else
                    {
                        memset( *at, 0, max_num_at * sizeof( **at ) );
                        atom = *at;
                    }
                }
                if (!at || !*at)
                {
                    atom = CreateInchiAtom( num_atoms + 1 );
                    if (!atom)
                    {
                        num_atoms = INCHI_INP_FATAL_RET; /* fatal error: cannot allocate */
                        *err = INCHI_INP_FATAL_ERR;
                        TREAT_ERR( *err, 0, "Out of RAM" );
                        goto bypass_end_of_INChI;
                    }
                }
                if (stereo0D && *stereo0D)
                {
                    if (num_atoms > max_len_stereo0D)
                    {
                        FreeInchi_Stereo0D( stereo0D );
                    }
                    else
                    {
                        memset( *stereo0D, 0, max_len_stereo0D * sizeof( **stereo0D ) );
                        atom_stereo0D = *stereo0D;
                    }
                }
                if (!stereo0D || !*stereo0D)
                {
                    max_len_stereo0D = num_atoms + 1;
                    atom_stereo0D = CreateInchi_Stereo0D( max_len_stereo0D );
                    if (!atom_stereo0D)
                    {
                        num_atoms = INCHI_INP_FATAL_RET; /* fatal error: cannot allocate */
                        *err = INCHI_INP_FATAL_ERR;
                        TREAT_ERR( *err, 0, "Out of RAM" );
                        goto bypass_end_of_INChI;
                    }
                }

                i = 0;
                bItemIsOver = 0;
                res2 = bTooLongLine2 = -1;

                /* read all atoms xml */
                while (i < num_atoms)
                {
                    pos = p - szLine;
                    if (!bItemIsOver && ( int )sizeof( szLine ) - res + pos > ( int )sizeof( szNextLine ))
                    {
                        /* load next line if possible */
                        res2 = inchi_ios_gets( szNextLine, sizeof( szNextLine ) - 1, inp_file, &bTooLongLine2 );
                        if (res2 > 0 && memcmp( szNextLine, sStructRevXmlRevAtEnd, sizeof( sStructRevXmlRevAtEnd ) - 1 ))
                        {
                            if (pos)
                            {
                                res -= pos;  /* number of chars left to process in szLine */
                                memmove( szLine, p, res * sizeof( szLine[0] ) ); /* move them to the start of the line */
                            }
                            memcpy( szLine + res, szNextLine, ( res2 + 1 ) * sizeof( szNextLine[0] ) );
                            res += res2;
                            szLine[res] = '\0';
                            bTooLongLine = bTooLongLine2;
                            p = szLine;
                        }
                        else
                        {
                            bItemIsOver = 1;
                        }
                    }

                    /* element, first char */
                    if (!isalpha( UCINT *p ) || !isupper( UCINT *p ) || i >= num_atoms)
                    {
                        bHeaderRead = 0; /* wrong atom data */
                        num_atoms = INCHI_INP_ERROR_RET; /* was 0, error */
                        *err = INCHI_INP_ERROR_ERR;     /* 40 */
                        TREAT_ERR( *err, 0, "Wrong atoms data" );
                        goto bypass_end_of_INChI;
                    }
                    atom[i].elname[0] = *p++;
                    /* element, second char */
                    if (isalpha( UCINT *p ) && islower( UCINT *p ))
                    {
                        atom[i].elname[1] = *p++;
                    }

                    /* bonds' valence */
                    if (isdigit( UCINT *p ))
                    {
                        AT_BONDS_VAL( atom, i ) = (char) strtol( p, &q, 10 );
                        if (!AT_BONDS_VAL( atom, i ))
                            AT_BONDS_VAL( atom, i ) = ISOLATED_ATOM; /* same convention as in MOLfile, found zero bonds valence */
                        p = q;
                    }
                    /* charge */
                    atom[i].charge = ( *p == '+' ) ? 1 : ( *p == '-' ) ? -1 : 0;
                    if (atom[i].charge)
                    {
                        p++;
                        if (isdigit( UCINT *p ))
                        {
                            atom[i].charge *= (S_CHAR) ( strtol( p, &q, 10 ) & CHAR_MASK );
                            p = q;
                        }
                    }

                    /* radical */
                    if (*p == '.')
                    {
                        p++;
                        if (isdigit( UCINT *p ))
                        {
                            atom[i].radical = (S_CHAR) strtol( p, &q, 10 );
                            p = q;
                        }
                    }

                    /* isotopic mass */
                    if (*p == 'i')
                    {
                        p++;
                        if (isdigit( UCINT *p ))
                        {
                            int mw = strtol( p, &q, 10 );
                            p = q;

                            atom[i].isotopic_mass = mw;
                        }
                    }

                    /* parity */
                    switch (*p)
                    {
                        case 'o':
                            parity = INCHI_PARITY_ODD;
                            p++;
                            break;
                        case 'e':
                            parity = INCHI_PARITY_EVEN;
                            p++;
                            break;
                        case 'u':
                            parity = INCHI_PARITY_UNKNOWN;
                            p++;
                            break;
                        case '?':
                            parity = INCHI_PARITY_UNDEFINED;
                            p++;
                            break;
                        default:
                            parity = 0;
                            break;
                    }
                    if (parity)
                    {
                        atom_stereo0D[len_stereo0D].central_atom = i;
                        atom_stereo0D[len_stereo0D].parity = parity;
                        atom_stereo0D[len_stereo0D].type = INCHI_StereoType_Tetrahedral;
                        len_stereo0D++;
                    }

                    /* isotopic h, d, t */
                    for (k = 0; k < NUM_H_ISOTOPES; k++)
                    {
                        if (*p == szIsoH[k])
                        {
                            NUM_ISO_Hk( atom, i, k ) = 1;
                            p++;
                            if (isdigit( UCINT *p ))
                            {
                                NUM_ISO_Hk( atom, i, k ) = (char) strtol( p, &q, 10 );
                                p = q;
                            }
                        }
                    }

                    i++;
                }

                if (!bItemIsOver || p - szLine != res || i != num_atoms)
                {
                    num_atoms = INCHI_INP_ERROR_RET; /* error */
                    *err = INCHI_INP_ERROR_ERR;
                    TREAT_ERR( *err, 0, "Wrong number of atoms" );
                    goto bypass_end_of_INChI;
                }

                /********************** bonds xml ****************************/

                res = inchi_ios_gets( szLine, sizeof( szLine ) - 1, inp_file, &bTooLongLine );
                if (res <= 0)
                {
                    num_atoms = 0; /* no data */
                    goto bypass_end_of_INChI;
                }
                if (memcmp( szLine, sStructRevXmlRevBn, sizeof( sStructRevXmlRevBn ) - 1 ))
                {
                    bHeaderRead = 0; /* invalid reversibility info; look for another header */
                    continue;
                }

                /* read (the head of) the xml bonds line */

                res = inchi_ios_gets( szLine, sizeof( szLine ) - 1, inp_file, &bTooLongLine );

                if (res <= 0)
                {
                    num_atoms = INCHI_INP_ERROR_RET; /* was 0; error: no data -- eof? */
                    *err = INCHI_INP_ERROR_ERR;
                    goto bypass_end_of_INChI;
                }

                i = 1;
                bItemIsOver = 0;
                res2 = bTooLongLine2 = -1;
                p = szLine;

                if (!memcmp( szLine, sStructRevXmlRevBnEnd, sizeof( sStructRevXmlRevBnEnd ) - 1 ))
                {
                    /* empty bonds section */
                    res = 0;
                    bItemIsOver = 1;
                }

                /* read all bonds (xml), starting from atom 1 (not 0) */

                while (i < num_atoms)
                {
                    pos = p - szLine;
                    if (!bItemIsOver &&
                        ( int )sizeof( szLine ) - res + pos > ( int )sizeof( szNextLine ))
                    {
                        /* load next line if possible */

                        res2 = inchi_ios_gets( szNextLine, sizeof( szNextLine ) - 1, inp_file, &bTooLongLine2 );

                        if (res2 > 0 && memcmp( szNextLine, sStructRevXmlRevBnEnd, sizeof( sStructRevXmlRevBnEnd ) - 1 ))
                        {
                            if (pos)
                            {
                                res -= pos;  /* number of chars left to process in szLine */
                                memmove( szLine, p, res * sizeof( szLine[0] ) ); /* move them to the start of the line */
                            }
                            memcpy( szLine + res, szNextLine, ( res2 + 1 ) * sizeof( szNextLine[0] ) );
                            res += res2;
                            szLine[res] = '\0';
                            bTooLongLine = bTooLongLine2;
                            p = szLine;
                        }
                        else
                            bItemIsOver = 1;
                    }

                    if (i >= num_atoms)
                        break;

                    /* bond, first char */

                    if (*p == ';')
                    {
                        p++;
                        i++;
                        continue;
                    }

                    if (!isalpha( UCINT *p ))
                    {
                        num_atoms = INCHI_INP_ERROR_RET; /* error in input data */
                        *err = INCHI_INP_ERROR_ERR;
                        TREAT_ERR( *err, 0, "Wrong bonds data" );
                        goto bypass_end_of_INChI;
                    }

                    bond_char = *p++;

                    /* bond parity */

                    switch (*p)
                    {
                        case '-':
                            bond_parity = INCHI_PARITY_ODD;
                            p++;
                            break;
                        case '+':
                            bond_parity = INCHI_PARITY_EVEN;
                            p++;
                            break;
                        case 'u':
                            bond_parity = INCHI_PARITY_UNKNOWN;
                            p++;
                            break;
                        case '?':
                            bond_parity = INCHI_PARITY_UNDEFINED;
                            p++;
                            break;
                        default:
                            bond_parity = 0;
                            break;
                    }

                    if (bond_parity)
                    {
                        switch (*p)
                        {
                            case '-':
                                bond_parityNM = INCHI_PARITY_ODD;
                                p++;
                                break;
                            case '+':
                                bond_parityNM = INCHI_PARITY_EVEN;
                                p++;
                                break;
                            case 'u':
                                bond_parityNM = INCHI_PARITY_UNKNOWN;
                                p++;
                                break;
                            case '?':
                                bond_parityNM = INCHI_PARITY_UNDEFINED;
                                p++;
                                break;
                            default:
                                bond_parityNM = 0;
                                break;
                        }
                    }
                    else
                    {
                        bond_parityNM = 0;
                    }

                    /* neighbor of the current atom */

                    if (!isdigit( UCINT *p ))
                    {
                        num_atoms = INCHI_INP_ERROR_RET; /* error in input data */
                        *err = INCHI_INP_ERROR_ERR;
                        TREAT_ERR( *err, 0, "Wrong bonds data" );
                        goto bypass_end_of_INChI;
                    }

                    neigh = (int) strtol( p, &q, 10 ) - 1;

                    if (i >= num_atoms || neigh >= num_atoms)
                    {
                        num_atoms = INCHI_INP_ERROR_RET; /* error in input data */
                        *err = INCHI_INP_ERROR_ERR;
                        TREAT_ERR( *err, 0, "Bond to nonexistent atom" );
                        goto bypass_end_of_INChI;
                    }
                    p = q;
                    bond_stereo1 = bond_stereo2 = 0;

                    /* bond type & 2D stereo */
                    switch (bond_char)
                    {
                        case 'v':
                            bond_type = INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 = INCHI_BOND_STEREO_SINGLE_1EITHER;
                            bond_stereo2 = INCHI_BOND_STEREO_SINGLE_2EITHER;
                            break;
                        case 'V':
                            bond_type = INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 = INCHI_BOND_STEREO_SINGLE_2EITHER;
                            bond_stereo2 = INCHI_BOND_STEREO_SINGLE_1EITHER;
                            break;
                        case 'w':
                            bond_type = INCHI_BOND_TYPE_DOUBLE;
                            bond_stereo1 =
                                bond_stereo2 = INCHI_BOND_STEREO_DOUBLE_EITHER;
                            break;
                        case 's':
                            bond_type = INCHI_BOND_TYPE_SINGLE;
                            break;
                        case 'd':
                            bond_type = INCHI_BOND_TYPE_DOUBLE;
                            break;
                        case 't':
                            bond_type = INCHI_BOND_TYPE_TRIPLE;
                            break;
                        case 'a':
                            bond_type = INCHI_BOND_TYPE_ALTERN;
                            break;
                        case 'p':
                            bond_type = INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 = INCHI_BOND_STEREO_SINGLE_1UP;
                            bond_stereo2 = INCHI_BOND_STEREO_SINGLE_2UP;
                            break;
                        case 'P':
                            bond_type = INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 = INCHI_BOND_STEREO_SINGLE_2UP;
                            bond_stereo2 = INCHI_BOND_STEREO_SINGLE_1UP;
                            break;
                        case 'n':
                            bond_type = INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 = INCHI_BOND_STEREO_SINGLE_1DOWN;
                            bond_stereo2 = INCHI_BOND_STEREO_SINGLE_2DOWN;
                            break;
                        case 'N':
                            bond_type = INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 = INCHI_BOND_STEREO_SINGLE_2DOWN;
                            bond_stereo2 = INCHI_BOND_STEREO_SINGLE_1DOWN;
                            break;
                        default:
                            num_atoms = INCHI_INP_ERROR_RET; /* error */
                            *err = INCHI_INP_ERROR_ERR;
                            TREAT_ERR( *err, 0, "Wrong bond type" );
                            goto bypass_end_of_INChI;
                    }

                    k = AT_NUM_BONDS( atom[i] )++;
                    atom[i].bond_type[k] = bond_type;
                    atom[i].bond_stereo[k] = bond_stereo1;
                    atom[i].neighbor[k] = (ATOM_NUMBER) neigh;

                    k2 = AT_NUM_BONDS( atom[neigh] )++;
                    atom[neigh].bond_type[k2] = bond_type;
                    atom[neigh].bond_stereo[k2] = bond_stereo2;
                    atom[neigh].neighbor[k2] = (ATOM_NUMBER) i;

                    bond_parity |= ( bond_parityNM << SB_PARITY_SHFT );

                    if (bond_parity)
                    {
                        if (max_len_stereo0D <= len_stereo0D)
                        {
                            /* realloc atom_Stereo0D */

                            inchi_Stereo0D *new_atom_stereo0D = CreateInchi_Stereo0D( max_len_stereo0D + num_atoms );

                            if (!new_atom_stereo0D)
                            {
                                num_atoms = INCHI_INP_FATAL_RET; /* fatal error: cannot allocate */
                                *err = INCHI_INP_FATAL_ERR;
                                TREAT_ERR( *err, 0, "Out of RAM" );
                                goto bypass_end_of_INChI;
                            }
                            memcpy( new_atom_stereo0D, atom_stereo0D, len_stereo0D * sizeof( *atom_stereo0D ) );
                            FreeInchi_Stereo0D( &atom_stereo0D );
                            atom_stereo0D = new_atom_stereo0D;
                            max_len_stereo0D += num_atoms;
                        }
                        /* (a) i may be allene endpoint and     neigh = allene middle point or
                           (b) i may be allene middle point and neigh = allene endpoint
                           !!!!! CURRENTLY ONLY (b) IS ALLOWED !!!!!
                        */
                        atom_stereo0D[len_stereo0D].neighbor[1] = neigh; /* neigh < i */
                        atom_stereo0D[len_stereo0D].neighbor[2] = i;
                        atom_stereo0D[len_stereo0D].parity = bond_parity;
                        atom_stereo0D[len_stereo0D].type = INCHI_StereoType_DoubleBond; /* incl allenes & cumulenes */
                        len_stereo0D++;
                    }
                }

                if (!bItemIsOver || p - szLine != res || i != num_atoms)
                {
                    num_atoms = INCHI_INP_ERROR_RET; /* error in input data */
                    *err = INCHI_INP_ERROR_ERR;
                    TREAT_ERR( *err, 0, "Wrong number of bonds" );
                    goto bypass_end_of_INChI;
                }

                /********************** coordinates xml ****************************/

                pszCoord = (MOL_COORD*) inchi_malloc( inchi_max( num_atoms, 1 ) * sizeof( MOL_COORD ) );

                if (pszCoord)
                {
                    memset( pszCoord, ' ', inchi_max( num_atoms, 1 ) * sizeof( MOL_COORD ) );
                    res = inchi_ios_gets( szLine, sizeof( szLine ) - 1, inp_file, &bTooLongLine );

                    if (res <= 0 ||
                         /* compare the header */
                         memcmp( szLine, sStructRevXmlRevXYZ, sizeof( sStructRevXmlRevXYZ ) - 1 ) ||
                         /* read (the head of) the coordinates (xml) line */
                         0 >= ( res = inchi_ios_gets( szLine, sizeof( szLine ) - 1, inp_file, &bTooLongLine ) ))
                    {
                        num_atoms = INCHI_INP_ERROR_RET; /* error in input data: atoms, bonds & coord must be present together */
                        *err = INCHI_INP_ERROR_ERR;
                        TREAT_ERR( *err, 0, "Missing atom coordinates data" );
                        goto bypass_end_of_INChI;
                    }

                    i = 0;
                    bItemIsOver = 0;
                    res2 = bTooLongLine2 = -1;
                    p = szLine;
                    if (!memcmp( szLine, sStructRevXmlRevXYZEnd, sizeof( sStructRevXmlRevXYZEnd ) - 1 ))
                    {
                        /* empty bonds section */
                        res = 0;
                        bItemIsOver = 1;
                    }

                    /* read all coordinates (xml), starting from atom 1 (not 0) */

                    while (i < num_atoms)
                    {
                        pos = p - szLine;

                        if (!bItemIsOver &&
                            ( int )sizeof( szLine ) - res + pos > ( int )sizeof( szNextLine ))
                        {

                            /* load next line if possible */

                            res2 = inchi_ios_gets( szNextLine, sizeof( szNextLine ) - 1, inp_file, &bTooLongLine2 );

                            if (res2 > 0 && memcmp( szNextLine, sStructRevXmlRevXYZEnd, sizeof( sStructRevXmlRevXYZEnd ) - 1 ))
                            {
                                if (pos)
                                {
                                    res -= pos;  /* number of chars left to process in szLine */
                                    memmove( szLine, p, res * sizeof( szLine[0] ) ); /* move them to the start of the line */
                                }
                                memcpy( szLine + res, szNextLine, ( res2 + 1 ) * sizeof( szNextLine[0] ) );
                                res += res2;
                                szLine[res] = '\0';
                                bTooLongLine = bTooLongLine2;
                                p = szLine;
                            }
                            else
                            {
                                bItemIsOver = 1;
                            }
                        }

                        /* coord, first char */

                        if (*p == ';')
                        {
                            for (k = 0; k < NUM_COORD; k++)
                            {
                                pszCoord[i][LEN_COORD*k + 4] = '0';
                            }
                            p++;
                            i++;
                            continue;
                        }

                        for (k = 0; k < 3; k++)
                        {
                            double xyz;
                            bNonZeroXYZ = 0;
                            if (*p == ';')
                            {
                                pszCoord[i][LEN_COORD*k + 4] = '0';
                                xyz = 0.0;
                            }
                            else if (*p == ',')
                            {
                                /* empty */
                                pszCoord[i][LEN_COORD*k + 4] = '0';
                                xyz = 0.0;
                                p++;
                            }
                            else
                            {
                                xyz = strtod( p, &q );
                                bNonZeroXYZ = fabs( xyz ) > MIN_BOND_LENGTH;
                                if (q != NULL)
                                {
                                    memcpy( pszCoord[i] + LEN_COORD*k, p, q - p );
                                    if (*q == ',')
                                        q++;
                                    p = q;
                                }
                                else
                                {
                                    pszCoord[i][LEN_COORD*k + 4] = '0';
                                }
                            }

                            switch (k)
                            {
                                case 0:
                                    atom[i].x = xyz;
                                    b2D |= bNonZeroXYZ;
                                    break;
                                case 1:
                                    atom[i].y = xyz;
                                    b2D |= bNonZeroXYZ;
                                    break;
                                case 2:
                                    b3D |= bNonZeroXYZ;
                                    atom[i].z = xyz;
                                    break;
                            }
                        }

                        if (*p == ';')
                        {
                            p++;  /* end of this triple of coordinates */
                            i++;
                        }
                        else
                        {
                            num_atoms = INCHI_INP_ERROR_RET; /* error in input data: atoms, bonds & coord must be present together */
                            *err = INCHI_INP_ERROR_ERR;
                            TREAT_ERR( *err, 0, "Wrong atom coordinates data" );
                            goto bypass_end_of_INChI;
                        }
                    }

                    if (!bItemIsOver || p - szLine != res || i != num_atoms)
                    {
                        num_atoms = INCHI_INP_ERROR_RET; /* error in input data: atoms, bonds & coord must be present together */
                        *err = INCHI_INP_ERROR_ERR;
                        TREAT_ERR( *err, 0, "Wrong number of coordinates" );
                        goto bypass_end_of_INChI;
                    }
                }
                else
                {
                    /* allocation failed */
                    num_atoms = INCHI_INP_FATAL_RET;
                    *err = INCHI_INP_FATAL_ERR;
                    TREAT_ERR( *err, 0, "Out of RAM" );
                    goto bypass_end_of_INChI;
                }

                /* set special valences and implicit H (xml) */

                b23D = b2D | b3D;
                b2D = b3D = 0;

                if (at)
                {
                    if (!*at)
                    {
                        int a1, a2, n1, n2, valence;
                        int chem_bonds_valence;
                        int    nX = 0, nY = 0, nZ = 0, nXYZ;
                        *at = atom;

                        /* special valences */
                        for (bNonMetal = 0; bNonMetal < 1 /*2*/; bNonMetal++)
                        {
                            for (a1 = 0; a1 < num_atoms; a1++)
                            {
                                int num_bond_type[MAX_INPUT_BOND_TYPE - MIN_INPUT_BOND_TYPE + 1];

                                memset( num_bond_type, 0, sizeof( num_bond_type ) );

                                valence = AT_BONDS_VAL( atom, a1 ); /*  save atom valence if available */
                                AT_BONDS_VAL( atom, a1 ) = 0;

                                nX = nY = nZ = 0;

                                for (n1 = 0; n1 < AT_NUM_BONDS( atom[a1] ); n1++)
                                {
                                    bond_type = atom[a1].bond_type[n1] - MIN_INPUT_BOND_TYPE;
                                    if (bond_type < 0 || bond_type > MAX_INPUT_BOND_TYPE - MIN_INPUT_BOND_TYPE)
                                    {
                                        bond_type = 0; /* cannot happen */
                                        TREAT_ERR( *err, 0, "Unknown bond type in InChI aux assigned as a single bond" );
                                    }

                                    num_bond_type[bond_type] ++;
                                    nNumBonds++;

                                    if (b23D)
                                    {
                                        neigh = atom[a1].neighbor[n1];
                                        nX |= ( fabs( atom[a1].x - atom[neigh].x ) > MIN_BOND_LENGTH );
                                        nY |= ( fabs( atom[a1].y - atom[neigh].y ) > MIN_BOND_LENGTH );
                                        nZ |= ( fabs( atom[a1].z - atom[neigh].z ) > MIN_BOND_LENGTH );
                                    }
                                }

                                chem_bonds_valence = 0;
                                for (n1 = 0; MIN_INPUT_BOND_TYPE + n1 <= 3 && MIN_INPUT_BOND_TYPE + n1 <= MAX_INPUT_BOND_TYPE; n1++)
                                {
                                    chem_bonds_valence += ( MIN_INPUT_BOND_TYPE + n1 ) * num_bond_type[n1];
                                }

                                if (MIN_INPUT_BOND_TYPE <= INCHI_BOND_TYPE_ALTERN &&
                                     INCHI_BOND_TYPE_ALTERN <= MAX_INPUT_BOND_TYPE &&
                                     ( n2 = num_bond_type[INCHI_BOND_TYPE_ALTERN - MIN_INPUT_BOND_TYPE] ))
                                {

                                    /* accept input aromatic bonds for now */

                                    switch (n2)
                                    {
                                        case 2:
                                            chem_bonds_valence += 3;  /* =A- */
                                            break;
                                        case 3:
                                            chem_bonds_valence += 4;  /* =A< */
                                            break;
                                        default:
                                            /*  if 1 or >= 4 aromatic bonds then replace such bonds with single bonds */
                                            for (n1 = 0; n1 < AT_NUM_BONDS( atom[a1] ); n1++)
                                            {
                                                if (atom[a1].bond_type[n1] == INCHI_BOND_TYPE_ALTERN)
                                                {
                                                    ATOM_NUMBER *p1;
                                                    a2 = atom[a1].neighbor[n1];
                                                    p1 = IN_NEIGH_LIST( atom[a2].neighbor, (ATOM_NUMBER) a1, AT_NUM_BONDS( atom[a2] ) );
                                                    if (p1)
                                                    {
                                                        atom[a1].bond_type[n1] =
                                                            atom[a2].bond_type[p1 - atom[a2].neighbor] = INCHI_BOND_TYPE_SINGLE;
                                                    }
                                                    else
                                                    {
                                                        *err = -2;  /*  Program error */
                                                        TREAT_ERR( *err, 0, "Program error interpreting InChI aux" );
                                                        num_atoms = 0;
                                                        goto bypass_end_of_INChI; /*  no structure */
                                                    }
                                                }
                                            }
                                            chem_bonds_valence += n2;
                                            *err |= 32; /*  Unrecognized aromatic bond(s) replaced with single */
                                            TREAT_ERR( *err, 0, "Atom has 1 or more than 3 aromatic bonds" );
                                            break;
                                    }
                                }

                                /*************************************************************************************
                                 *
                                 *  Set number of hydrogen atoms
                                 */
                                {
                                    int num_iso_H;
                                    num_iso_H = atom[a1].num_iso_H[1] + atom[a1].num_iso_H[2] + atom[a1].num_iso_H[3];
                                    if (valence == ISOLATED_ATOM)
                                    {
                                        atom[a1].num_iso_H[0] = 0;
                                    }
                                    else
                                    {
                                        if (valence && valence >= chem_bonds_valence)
                                        {
                                            atom[a1].num_iso_H[0] = valence - chem_bonds_valence;
                                        }
                                        else
                                        {
                                            if (valence || bDoNotAddH)
                                            {
                                                atom[a1].num_iso_H[0] = 0;
                                            }
                                            else
                                            {
                                                if (!bDoNotAddH)
                                                {
                                                    atom[a1].num_iso_H[0] = -1; /* auto add H */
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }

                        nNumBonds /= 2;

                        if (b23D && nNumBonds)
                        {
                            nXYZ = nX + nY + nZ;
                            b2D = ( nXYZ > 0 );
                            b3D = ( nXYZ == 3 );
                            *num_dimensions = b3D ? 3 : b2D ? 2 : 0;
                            *num_bonds = nNumBonds;
                        }

                        /*======= 0D parities =================================*/
                        if (len_stereo0D > 0 && atom_stereo0D && stereo0D)
                        {
                            *stereo0D = atom_stereo0D;
                            *num_stereo0D = len_stereo0D;
                        }
                        else
                        {
                            FreeInchi_Stereo0D( &atom_stereo0D );
                            *num_stereo0D = len_stereo0D = 0;
                        }

                        for (i = 0; i < len_stereo0D; i++)
                        {
                            ATOM_NUMBER *p1, *p2;
                            int     sb_ord_from_a1 = -1, sb_ord_from_a2 = -1, bEnd1 = 0, bEnd2 = 0;
                            switch (atom_stereo0D[i].type)
                            {

                                case INCHI_StereoType_Tetrahedral:
                                    a1 = atom_stereo0D[i].central_atom;
                                    if (atom_stereo0D[i].parity &&
                                        ( AT_NUM_BONDS( atom[a1] ) == 3 || AT_NUM_BONDS( atom[a1] ) == 4 ))
                                    {
                                        int ii, kk = 0;
                                        if (AT_NUM_BONDS( atom[a1] ) == 3)
                                        {
                                            atom_stereo0D[i].neighbor[kk++] = a1;
                                        }
                                        for (ii = 0; ii < AT_NUM_BONDS( atom[a1] ); ii++)
                                        {
                                            atom_stereo0D[i].neighbor[kk++] = atom[a1].neighbor[ii];
                                        }
                                    }

                                    break;

                                case INCHI_StereoType_DoubleBond:

#define MAX_CHAIN_LEN 20

                                    a1 = atom_stereo0D[i].neighbor[1];
                                    a2 = atom_stereo0D[i].neighbor[2];
                                    p1 = IN_NEIGH_LIST( atom[a1].neighbor, (ATOM_NUMBER) a2, AT_NUM_BONDS( atom[a1] ) );
                                    p2 = IN_NEIGH_LIST( atom[a2].neighbor, (ATOM_NUMBER) a1, AT_NUM_BONDS( atom[a2] ) );

                                    if (!p1 || !p2)
                                    {
                                        atom_stereo0D[i].type = INCHI_StereoType_None;
                                        atom_stereo0D[i].central_atom = NO_ATOM;
                                        atom_stereo0D[i].neighbor[0] =
                                            atom_stereo0D[i].neighbor[3] = -1;
                                        *err |= 64; /* Error in cumulene stereo */
                                        TREAT_ERR( *err, 0, "0D stereobond not recognized" );
                                        break;
                                    }

                                    /* streobond, allene, or cumulene */

                                    sb_ord_from_a1 = p1 - atom[a1].neighbor;
                                    sb_ord_from_a2 = p2 - atom[a2].neighbor;

                                    if (AT_NUM_BONDS( atom[a1] ) == 2 &&
                                          atom[a1].bond_type[0] + atom[a1].bond_type[1] == 2 * INCHI_BOND_TYPE_DOUBLE &&
                                          0 == inchi_NUMH2( atom, a1 ) &&
                                          ( AT_NUM_BONDS( atom[a2] ) != 2 ||
                                              atom[a2].bond_type[0] + atom[a2].bond_type[1] != 2 * INCHI_BOND_TYPE_DOUBLE ))
                                    {
                                        bEnd2 = 1; /* a2 is the end-atom, a1 is middle atom */
                                    }
                                    if (AT_NUM_BONDS( atom[a2] ) == 2 &&
                                          atom[a2].bond_type[0] + atom[a2].bond_type[1] == 2 * INCHI_BOND_TYPE_DOUBLE &&
                                          0 == inchi_NUMH2( atom, a2 ) &&
                                          ( AT_NUM_BONDS( atom[a1] ) != 2 ||
                                              atom[a1].bond_type[0] + atom[a1].bond_type[1] != 2 * INCHI_BOND_TYPE_DOUBLE ))
                                    {
                                        bEnd1 = 1; /* a1 is the end-atom, a2 is middle atom */
                                    }

                                    if (bEnd2 + bEnd1 == 1)
                                    {
                                        /* allene or cumulene */
                                        ATOM_NUMBER  chain[MAX_CHAIN_LEN + 1], prev, cur, next;
                                        if (bEnd2 && !bEnd1)
                                        {
                                            cur = a1;
                                            a1 = a2;
                                            a2 = cur;
                                            sb_ord_from_a1 = sb_ord_from_a2;
                                        }

                                        sb_ord_from_a2 = -1;
                                        cur = a1;
                                        next = a2;
                                        len = 0;
                                        chain[len++] = cur;
                                        chain[len++] = next;

                                        while (len < MAX_CHAIN_LEN)
                                        {
                                            /* arbitrary very high upper limit to prevent infinite loop */

                                            prev = cur;
                                            cur = next;
                                                /* follow double bond path && avoid going back */
                                            if (AT_NUM_BONDS( atom[cur] ) == 2 &&
                                                 atom[cur].bond_type[0] + atom[cur].bond_type[1] == 2 * INCHI_BOND_TYPE_DOUBLE &&
                                                 0 == inchi_NUMH2( atom, cur ))
                                            {
                                                next = atom[cur].neighbor[atom[cur].neighbor[0] == prev];
                                                chain[len++] = next;
                                            }
                                            else
                                                break;
                                        }

                                        if (len > 2 &&
                                            ( p2 = IN_NEIGH_LIST( atom[cur].neighbor, (ATOM_NUMBER) prev, AT_NUM_BONDS( atom[cur] ) ) ))
                                        {
                                            sb_ord_from_a2 = p2 - atom[cur].neighbor;
                                            a2 = cur;

                                            /* by design we need to pick up the first non-stereo-bond-neighbor as "sn"-atom */

                                            atom_stereo0D[i].neighbor[0] = atom[a1].neighbor[sb_ord_from_a1 == 0];
                                            atom_stereo0D[i].neighbor[1] = a1;
                                            atom_stereo0D[i].neighbor[2] = a2;
                                            atom_stereo0D[i].neighbor[3] = atom[a2].neighbor[sb_ord_from_a2 == 0];

                                            if (len % 2)
                                            {
                                                atom_stereo0D[i].central_atom = chain[len / 2];
                                                atom_stereo0D[i].type = INCHI_StereoType_Allene;
                                            }
                                            else
                                            {
                                                atom_stereo0D[i].central_atom = NO_ATOM;
                                            }
                                        }
                                        else
                                        {
                                            /* error */
                                            atom_stereo0D[i].type = INCHI_StereoType_None;
                                            atom_stereo0D[i].central_atom = NO_ATOM;
                                            atom_stereo0D[i].neighbor[0] =
                                                atom_stereo0D[i].neighbor[3] = -1;
                                            *err |= 64; /* Error in cumulene stereo */
                                            TREAT_ERR( *err, 0, "Cumulene stereo not recognized (0D)" );
                                        }

#undef MAX_CHAIN_LEN
                                    }
                                    else
                                    {

                                        /****** a normal possibly stereogenic bond -- not an allene or cumulene *******/
                                        /* by design we need to pick up the first non-stereo-bond-neighbor as "sn"-atom */

                                        sb_ord_from_a1 = p1 - atom[a1].neighbor;
                                        sb_ord_from_a2 = p2 - atom[a2].neighbor;
                                        atom_stereo0D[i].neighbor[0] = atom[a1].neighbor[p1 == atom[a1].neighbor];
                                        atom_stereo0D[i].neighbor[3] = atom[a2].neighbor[p2 == atom[a2].neighbor];
                                        atom_stereo0D[i].central_atom = NO_ATOM;
                                    }

                                    if (atom_stereo0D[i].type != INCHI_StereoType_None &&
                                         sb_ord_from_a1 >= 0 && sb_ord_from_a2 >= 0 &&
                                         ATOM_PARITY_WELL_DEF( SB_PARITY_2( atom_stereo0D[i].parity ) ))
                                    {

                                        /* Detected well-defined disconnected stereo
                                         * locate first non-metal neighbors */

                                        int    a, n, j, /* k,*/ sb_ord, cur_neigh, min_neigh;
                                        for (k = 0; k < 2; k++)
                                        {
                                            a = k ? atom_stereo0D[i].neighbor[2] : atom_stereo0D[i].neighbor[1];
                                            sb_ord = k ? sb_ord_from_a2 : sb_ord_from_a1;
                                            min_neigh = num_atoms;
                                            for (n = j = 0; j < AT_NUM_BONDS( atom[a] ); j++)
                                            {
                                                cur_neigh = atom[a].neighbor[j];
                                                if (j != sb_ord && !IS_METAL_ATOM( atom, cur_neigh ))
                                                {
                                                    min_neigh = inchi_min( cur_neigh, min_neigh );
                                                }
                                            }
                                            if (min_neigh < num_atoms)
                                            {
                                                atom_stereo0D[i].neighbor[k ? 3 : 0] = min_neigh;
                                            }
                                            else
                                            {
                                                TREAT_ERR( *err, 0, "Cannot find non-metal stereobond neighor (0D)" );
                                            }
                                        }
                                    }

                                    break;
                            }
                        }

                        /* end of 0D parities extraction */
/*exit_cycle:;*/
                    }

                    if (pInpAtomFlags)
                    {
                        /* save chirality flag */
                        *pInpAtomFlags |= InpAtomFlags;
                    }
                }
                else if (atom)
                {
                    inchi_free( atom );
                    atom = NULL;
                }

                if (pszCoord)
                {
                    inchi_free( pszCoord );
                }
                goto bypass_end_of_INChI;
                /*return num_atoms;*/
            }
        }

        if (atom_stereo0D)
            FreeInchi_Stereo0D( &atom_stereo0D );

        /* end of struct. reading cycle, code never used? */

        if (res <= 0)
        {
            if (*err == INCHI_INP_ERROR_ERR)
            {
                return num_atoms;
            }

            *err = INCHI_INP_EOF_ERR;
            return INCHI_INP_EOF_RET; /* no more data */
        }

    bypass_end_of_INChI:

        /* cleanup */
        if (num_atoms == INCHI_INP_ERROR_RET && atom_stereo0D)
        {
            if (stereo0D && *stereo0D == atom_stereo0D)
            {
                *stereo0D = NULL;
                *num_stereo0D = 0;
            }
            FreeInchi_Stereo0D( &atom_stereo0D );
        }

        if (!memcmp( szLine, sStructHdrXmlEnd, sizeof( sStructHdrXmlEnd ) - 1 ))
        {
            num_struct--;
        }

        if (!memcmp( szLine, sStructHdrXml, sizeof( sStructHdrXml ) - 1 ))
        {
            num_struct++;
        }

        while (num_struct > 0 && 0 < inchi_ios_gets( szLine, sizeof( szLine ) - 1, inp_file, &bTooLongLine ))
        {
            if (!memcmp( szLine, sStructHdrXmlEnd, sizeof( sStructHdrXmlEnd ) - 1 ))
            {
                num_struct--;
            }
            else
            {
                if (!memcmp( szLine, sStructHdrXml, sizeof( sStructHdrXml ) - 1 ))
                {
                    num_struct++;
                }
            }
        }
        return num_atoms;
    }

    return num_atoms;
}

