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
#include <limits.h>
#include <math.h>
#include <float.h>
#include <string.h>
#include <ctype.h>

/* for use in the InChI library */

#include "../../../INCHI_BASE/src/mode.h"

#include "../../../INCHI_BASE/src/incomdef.h"
#include "../../../INCHI_BASE/src/ichidrp.h"
#include "../../../INCHI_BASE/src/inpdef.h"
#include "../../../INCHI_BASE/src/util.h"
#include "../../../INCHI_BASE/src/ichierr.h"
#include "../../../INCHI_BASE/src/ichicomp.h"
#include "../../../INCHI_BASE/src/ichi_io.h"
#include "../../../INCHI_BASE/src/inchi_api.h"

#include "inchi_dll_b.h"



int InchiToInchi_Input( INCHI_IOSTREAM *inp_molfile, inchi_Input *orig_at_data, int bMergeAllInputStructures,
                       int bDoNotAddH, int vABParityUnknown, INPUT_TYPE nInputType,
                       char *pSdfLabel, char *pSdfValue, long *lSdfId, INCHI_MODE *pInpAtomFlags,
                       int *err, char *pStrErr );

/* This contains executable code. Included in lReadAux.c, e_ReadINCH.c, ReadINCH.c,  */
#include "../../../INCHI_BASE/src/readinch.h"



/*****************************************************************************************************/
EXPIMP_TEMPLATE INCHI_API int INCHI_DECL Get_std_inchi_Input_FromAuxInfo
( char *szInchiAuxInfo,
int bDoNotAddH,
InchiInpData *pInchiInp )
{
    int bDiffUnkUndfStereo = 0;
    return Get_inchi_Input_FromAuxInfo( szInchiAuxInfo, bDoNotAddH, bDiffUnkUndfStereo,
                                        pInchiInp );
}

EXPIMP_TEMPLATE INCHI_API int INCHI_DECL Get_inchi_Input_FromAuxInfo( char *szInchiAuxInfo,
                                                  int bDoNotAddH, int bDiffUnkUndfStereo,
                                                  InchiInpData *pInchiInp )
{
    INCHI_IOSTREAM inp;
    int num_at, nRet = inchi_Ret_OKAY, err = 0;
    INCHI_MODE bChiral = 0;
    /* the input string may contain the following line: "Structure NNN. HHH=VVV" */
    long         lNumber;                   /* structure number NNN from the input */
    char         szHeader[MAX_SDF_HEADER];  /* stucture label header HHH from the input */
    char         szLabel[MAX_SDF_VALUE];    /* stucture label VVV from the input */

    /* vABParityUnknown holds actual value of an internal constant signifying       */
    /* unknown parity: either the same as for undefined parity (default==standard)  */
    /*  or a specific one (non-std; requested by SLUUD switch).                     */
    int vABParityUnknown = AB_PARITY_UNDF;
    if (0 != bDiffUnkUndfStereo)
    {
        /* Make labels for unknown and undefined stereo different */
        vABParityUnknown = AB_PARITY_UNKN;
    }



    if (pInchiInp && pInchiInp->pInp)
    {
/* clear output fields */
        inchi_Input *pInp = pInchiInp->pInp;
        char        *szOptions = pInp->szOptions;
        memset( pInchiInp, 0, sizeof( *pInchiInp ) );
        memset( pInp, 0, sizeof( *pInp ) );
        pInp->szOptions = szOptions;
        pInchiInp->pInp = pInp;
    }
    else
    {
        return inchi_Ret_ERROR;
    }
    szHeader[0] = '\0';
    szLabel[0] = '\0';
    lNumber = 0;
    /* prepare input string pointers */
    inchi_ios_init( &inp, INCHI_IOS_TYPE_STRING, NULL ); /* fix bug discovered by Burt Leland 2008-12-23 */
    inp.s.pStr = szInchiAuxInfo;
    inp.s.nUsedLength = (int) strlen( szInchiAuxInfo );
    inp.s.nAllocatedLength = inp.s.nUsedLength + 1;
    inp.s.nPtr = 0;

    num_at = InchiToInchi_Input( &inp, pInchiInp->pInp, 1, bDoNotAddH, vABParityUnknown,
                                 INPUT_INCHI_PLAIN, szHeader, szLabel,
                                 &lNumber, &bChiral, &err, pInchiInp->szErrMsg );
    pInchiInp->bChiral = bChiral;
    if (num_at <= 0)
    {
        if (10 < err && err < 20)
        {
            nRet = inchi_Ret_EOF;
        }
        else
            if (err == 9)
            {
                nRet = inchi_Ret_ERROR; /*  sdfile bypassed to $$$$ */
            }
            else
                if (err && err < 30)
                {
                    nRet = inchi_Ret_FATAL;
                }
                else
                    if (98 == err)
                    {
                        nRet = inchi_Ret_WARNING; /* empty AuxInfo */
                    }
                    else
                        if (err)
                        {
                            nRet = inchi_Ret_ERROR;
                        }
                        else
                            if (pInchiInp->szErrMsg[0])
                            {
                                nRet = inchi_Ret_WARNING;
                            }
    }
    if (nRet != inchi_Ret_OKAY && nRet != inchi_Ret_WARNING)
    {
        Free_inchi_Input( pInchiInp->pInp );
        pInchiInp->bChiral = 0;
    }

    return nRet;
}
/*****************************************************************************************************/
void INCHI_DECL Free_std_inchi_Input( inchi_Input *pInp )
{
    Free_inchi_Input( pInp );
}

void INCHI_DECL Free_inchi_Input( inchi_Input *pInp )
{
    FreeInchi_Atom( &pInp->atom );
    FreeInchi_Stereo0D( &pInp->stereo0D );
    pInp->num_atoms = 0;
    pInp->num_stereo0D = 0;
}


/*#endif*/ /* INCHI_MAIN */

#ifndef TARGET_API_LIB
#error "TARGET_API_LIB MUST be defined here"
#endif


/**********************************************************************************/
int InchiToInchi_Input( INCHI_IOSTREAM *inp_molfile,
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
    inchi_Atom     *at_new = NULL;
    inchi_Atom     *at_old = NULL;
    inchi_Stereo0D *stereo0D_new = NULL;
    inchi_Stereo0D *stereo0D_old = NULL;
    int             nNumAtoms = 0, nNumStereo0D = 0;
    MOL_COORD      *szCoordNew = NULL;
    MOL_COORD      *szCoordOld = NULL;
    int            i, j;

    if (pStrErr)
    {
        pStrErr[0] = '\0';
    }

    /*FreeOrigAtData( orig_at_data );*/
    if (lSdfId)
        *lSdfId = 0;
    do
    {

        at_old = orig_at_data ? orig_at_data->atom : NULL; /*  save pointer to the previous allocation */
        stereo0D_old = orig_at_data ? orig_at_data->stereo0D : NULL;
        szCoordOld = NULL;
        num_inp_atoms_new =
            InchiToInchiAtom( inp_molfile, orig_at_data ? &stereo0D_new : NULL, &num_inp_0D_new,
                          bDoNotAddH, vABParityUnknown, nInputType,
                          orig_at_data ? &at_new : NULL, MAX_ATOMS,
                          &num_dimensions_new, &num_inp_bonds_new,
                          pSdfLabel, pSdfValue, lSdfId, pInpAtomFlags, err, pStrErr );
        if (num_inp_atoms_new <= 0 && !*err)
        {
            TREAT_ERR( *err, 0, "Empty structure" );
            *err = 98;
        }
        else
            if (orig_at_data && !num_inp_atoms_new && 10 < *err && *err < 20 && orig_at_data->num_atoms > 0 && bMergeAllInputStructures)
            {
                *err = 0; /* end of file */
                break;
            }
            else
                if (num_inp_atoms_new > 0 && orig_at_data)
                {
/*  merge pOrigDataTmp + orig_at_data => pOrigDataTmp; */
                    nNumAtoms = num_inp_atoms_new + orig_at_data->num_atoms;
                    nNumStereo0D = num_inp_0D_new + orig_at_data->num_stereo0D;
                    if (nNumAtoms >= MAX_ATOMS)
                    {
                        TREAT_ERR( *err, 0, "Too many atoms [did you forget 'LargeMolecules' switch?]" );
                        *err = 70;
                        orig_at_data->num_atoms = -1;
                    }
                    else
                        if (!at_old)
                        {
            /* the first structure */
                            orig_at_data->atom = at_new;            at_new = NULL;
                            orig_at_data->num_atoms = num_inp_atoms_new; num_inp_atoms_new = 0;
                            orig_at_data->stereo0D = stereo0D_new;      stereo0D_new = NULL;
                            orig_at_data->num_stereo0D = num_inp_0D_new;    num_inp_0D_new = 0;
                        }
                        else
                            if (orig_at_data->atom = CreateInchiAtom( nNumAtoms ))
                            {
/*  switch at_new <--> orig_at_data->at; */
                                if (orig_at_data->num_atoms)
                                {
                                    memcpy( orig_at_data->atom, at_old, orig_at_data->num_atoms * sizeof( orig_at_data->atom[0] ) );
                                    /*  adjust numbering in the newly read structure */
                                    for (i = 0; i < num_inp_atoms_new; i++)
                                    {
                                        for (j = 0; j < at_new[i].num_bonds; j++)
                                        {
                                            at_new[i].neighbor[j] += orig_at_data->num_atoms;
                                        }
                                    }
                                }
                                FreeInchi_Atom( &at_old );
                                /*  copy newly read structure */
                                memcpy( orig_at_data->atom + orig_at_data->num_atoms,
                                        at_new,
                                        num_inp_atoms_new * sizeof( orig_at_data->atom[0] ) );
                                /*  copy newly read 0D stereo */
                                if (num_inp_0D_new > 0 && stereo0D_new)
                                {
                                    if (orig_at_data->stereo0D = CreateInchi_Stereo0D( nNumStereo0D ))
                                    {
                                        memcpy( orig_at_data->stereo0D, stereo0D_old, orig_at_data->num_stereo0D * sizeof( orig_at_data->stereo0D[0] ) );
                                        /*  adjust numbering in the newly read structure */
                                        for (i = 0; i < num_inp_0D_new; i++)
                                        {
                                            if (stereo0D_new[i].central_atom >= 0)
                                            {
                                                stereo0D_new[i].central_atom += orig_at_data->num_atoms;
                                            }
                                            for (j = 0; j < 4; j++)
                                            {
                                                stereo0D_new[i].neighbor[j] += orig_at_data->num_atoms;
                                            }
                                        }
                                        FreeInchi_Stereo0D( &stereo0D_old );
                                        memcpy( orig_at_data->stereo0D + orig_at_data->num_stereo0D,
                                                stereo0D_new,
                                                num_inp_0D_new * sizeof( orig_at_data->stereo0D[0] ) );
                                    }
                                    else
                                    {
                                        num_inp_0D_new = 0;
                                        TREAT_ERR( *err, 0, "Out of RAM" );
                                        *err = -1;
                                    }
                                }
                                else
                                {
                                    num_inp_0D_new = 0;
                                }
                                /* update lengths */
                                orig_at_data->num_atoms += num_inp_atoms_new;
                                orig_at_data->num_stereo0D += num_inp_0D_new;
                            }
                            else
                            {
                                TREAT_ERR( *err, 0, "Out of RAM" );
                                *err = -1;
                            }
                }
                else
                    if (num_inp_atoms_new > 0)
                    {
                        nNumAtoms += num_inp_atoms_new;
                    }
        FreeInchi_Atom( &at_new );
        num_inp_atoms_new = 0;
        FreeInchi_Stereo0D( &stereo0D_new );
        num_inp_0D_new = 0;
    }
    while (!*err && bMergeAllInputStructures);
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
    if (szCoordNew)
    {
        inchi_free( szCoordNew );
    }
    if (at_new)
    {
        inchi_free( at_new );
    }
    /*
    if ( !*err ) {
        if ( ReconcileAllCmlBondParities( orig_at_data->atom, orig_at_data->num_atoms ) ) {
            TREAT_ERR (*err, 0, "Cannot reconcile stereobond parities");
            if (!orig_at_data->num_atoms) {
                *err = 1;
            }
        }
    }
    */
    if (*err)
    {
        FreeInchi_Input( orig_at_data );
    }
    if (*err && !( 10 < *err && *err < 20 ) && pStrErr && !pStrErr[0])
    {
        TREAT_ERR( *err, 0, "Unknown error" );  /*   <BRKPT> */
    }
    return orig_at_data ? orig_at_data->num_atoms : nNumAtoms;
}
