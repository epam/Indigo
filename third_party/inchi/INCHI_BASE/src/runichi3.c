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
    Pre-processing related functions

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
#include "inpdef.h"
#include "ichi_io.h"


/* Local prototypes */
int bCheckUnusualValences( ORIG_ATOM_DATA *orig_at_data, int bAddIsoH, char *pStrErrStruct );
void throw_away_inappropriate_bond( int at1, int at2, int *nbonds, int **bonds);





/*    Check inp_ATOM's for unusual valence */
int bCheckUnusualValences( ORIG_ATOM_DATA *orig_at_data,
                           int bAddIsoH, char *pStrErrStruct )
{
int i, val, num_found = 0;
char msg[32];
int len, num_H;

    int already_here = ( orig_at_data && orig_at_data->num_inp_atoms > 0 );

    inp_ATOM *at = already_here ? orig_at_data->at : NULL;

    if ( at )
    {
        for ( i = 0, num_found = 0; i < orig_at_data->num_inp_atoms; i ++ )
        {
            num_H = bAddIsoH ? NUMH(at,i) : at[i].num_H;

            val = detect_unusual_el_valence( at[i].el_number,
                                             at[i].charge,
                                             at[i].radical,
                                             at[i].chem_bonds_valence,
                                             num_H,
                                             at[i].valence );
            if ( val )
            {
                num_found ++;
                WarningMessage(pStrErrStruct, "Accepted unusual valence(s):");
                len = sprintf( msg, "%s", at[i].elname );
                if ( at[i].charge )
                {
                    len += sprintf( msg+len, "%+d", at[i].charge );
                }
                if ( at[i].radical )
                {
                    len += sprintf( msg + len, ",%s", at[i].radical == RADICAL_SINGLET? "s" :
                                                      at[i].radical == RADICAL_DOUBLET? "d" :
                                                      at[i].radical == RADICAL_TRIPLET? "t" : "?" );
                }
                len += sprintf( msg + len, "(%d)", val );
                WarningMessage(pStrErrStruct, msg);
            }
        }
    }

    return num_found;
}


/*
    Make a copy of ORIG_ATOM_DATA structure
    ( inp_ATOM array, etc., etc. )
*/
int DuplicateOrigAtom( ORIG_ATOM_DATA *new_orig_atom,
                       ORIG_ATOM_DATA *orig_atom )
{
    inp_ATOM  *at                 = NULL;
    AT_NUMB   *nCurAtLen          = NULL;
    AT_NUMB   *nOldCompNumber     = NULL;

    if ( new_orig_atom->at &&
         new_orig_atom->num_inp_atoms >= orig_atom->num_inp_atoms )
    {
        at = new_orig_atom->at;
    }
    else
    {
        at = (inp_ATOM *)inchi_calloc( orig_atom->num_inp_atoms+1,
                                       sizeof(at[0]));
    }

    if ( new_orig_atom->nOldCompNumber &&
         new_orig_atom->num_components >= orig_atom->num_components )
    {
        nCurAtLen = new_orig_atom->nCurAtLen;
    }
    else
    {
        nCurAtLen = (AT_NUMB *)inchi_calloc( orig_atom->num_components+1,
                                             sizeof(nCurAtLen[0]));
    }

    if ( new_orig_atom->nCurAtLen &&
         new_orig_atom->num_components >= orig_atom->num_components )
    {
        nOldCompNumber = new_orig_atom->nOldCompNumber;
    }
    else
    {
        nOldCompNumber = (AT_NUMB *)inchi_calloc( orig_atom->num_components+1,
                                                  sizeof(nOldCompNumber[0]));
    }

    if ( at && nCurAtLen && nOldCompNumber )
    {
        /* Copy */
        if ( orig_atom->at )
            memcpy( at, orig_atom->at,
                    orig_atom->num_inp_atoms * sizeof(new_orig_atom->at[0]) );
        if ( orig_atom->nCurAtLen )
            memcpy( nCurAtLen, orig_atom->nCurAtLen,
                    orig_atom->num_components*sizeof(nCurAtLen[0]) );
        if ( orig_atom->nOldCompNumber )
            memcpy( nOldCompNumber, orig_atom->nOldCompNumber,
                    orig_atom->num_components*sizeof(nOldCompNumber[0]) );

        /* Deallocate */
        if ( new_orig_atom->at && new_orig_atom->at != at )
            inchi_free( new_orig_atom->at );
        if ( new_orig_atom->nCurAtLen && new_orig_atom->nCurAtLen!=nCurAtLen )
            inchi_free( new_orig_atom->nCurAtLen );
        if ( new_orig_atom->nOldCompNumber &&
             new_orig_atom->nOldCompNumber != nOldCompNumber )
            inchi_free( new_orig_atom->nOldCompNumber );

        *new_orig_atom                = *orig_atom;
        new_orig_atom->at             = at;
        new_orig_atom->nCurAtLen      = nCurAtLen;
        new_orig_atom->nOldCompNumber = nOldCompNumber;

        /* Data that are not to be copied */
        new_orig_atom->nNumEquSets    = 0;
        memset(new_orig_atom->bSavedInINCHI_LIB, 0, sizeof(new_orig_atom->bSavedInINCHI_LIB));
        memset(new_orig_atom->bPreprocessed,    0, sizeof(new_orig_atom->bPreprocessed));

        /* Arrays that are not to be copied */
        new_orig_atom->szCoord        = NULL;
        new_orig_atom->nEquLabels     = NULL;
        new_orig_atom->nSortedOrder   = NULL;

        new_orig_atom->polymer          = NULL;
        new_orig_atom->v3000          = NULL;

        return 0;
    }

    /* Deallocate */
    if ( at && new_orig_atom->at != at )
        inchi_free( at );
    if ( nCurAtLen && new_orig_atom->nCurAtLen != nCurAtLen )
        inchi_free( nCurAtLen );
    if ( nOldCompNumber && new_orig_atom->nOldCompNumber != nOldCompNumber )
        inchi_free( nOldCompNumber );

    return -1; /* Failed */
}


/*
    Preprocess the whole structure

    The plan is:

    1.    Copy orig_inp_data --> prep_inp_data (then work with the latter)

    2.    Fix odd things in prep_inp_data

            Find whether the structure can be disconnected or is a salt
                - check if needs salt disconnection
                - check if needs metal disconnection

    3.    If ( orig_inp_data->bDisconnectSalts ) then
            disconnect salts in prep_inp_data

            Mark the (disconnected) components in prep_inp_data

            Detect isotopic H on heteroatoms (necessary condition
            for global isotopic tautomerism)

    4.    Detect unusual valences (should be called before metal disconnection)

    5.    Create metal-disconnected structure if applicable.
            - save reconnected structure in prep_inp_data+1 if requested
            - make Disconnected structure in prep_inp_data

*/
int PreprocessOneStructure( struct tagINCHI_CLOCK *ic,
                            STRUCT_DATA *sd,
                            INPUT_PARMS *ip,
                            ORIG_ATOM_DATA *orig_inp_data,
                            ORIG_ATOM_DATA *prep_inp_data )
{
int i;
INCHI_MODE bTautFlags     = 0;
INCHI_MODE bTautFlagsDone = 0;



    /* 1. Copy orig_inp_data --> prep_inp_data */

    if ( 0 > DuplicateOrigAtom( prep_inp_data, orig_inp_data ) )
    {
        AddErrorMessage(sd->pStrErrStruct, "Out of RAM");
        sd->nStructReadError =  99;
        sd->nErrorType = _IS_FATAL;
        goto exit_function;
    }

#if ( bRELEASE_VERSION == 0 && (EXTR_HAS_METAL_ATOM & (EXTR_MASK | EXTR_FLAG) ) )
        if ( bHasMetalAtom( orig_inp_data ) ) {
            sd->bExtract |= EXTR_HAS_METAL_ATOM;
        }
#endif



    /* 2. Fix odd things in prep_inp_data            */

    if ( 0 < fix_odd_things( prep_inp_data->num_inp_atoms, prep_inp_data->at, /*0*/ip->bTautFlags & TG_FLAG_FIX_SP3_BUG, ip->bFixNonUniformDraw ) )
    {
        /* changed 2010-03-17 DT */
        WarningMessage(sd->pStrErrStruct, "Charges were rearranged");
        if ( sd->nErrorType < _IS_WARNING )
        {
            sd->nErrorType = _IS_WARNING;
        }
        sd->bTautFlagsDone[INCHI_BAS] |= TG_FLAG_FIX_ODD_THINGS_DONE;
    }

#if ( FIX_ADJ_RAD == 1 )
    if ( ip->bTautFlags & TG_FLAG_FIX_ADJ_RADICALS ) {
        if ( 0 < FixAdjacentRadicals( prep_inp_data->num_inp_atoms, prep_inp_data->at ) ) {
            sd->bTautFlagsDone[INCHI_BAS] |= TG_FLAG_FIX_ADJ_RADICALS_DONE;
        }
    }
#endif

#if ( bRELEASE_VERSION == 0 && (EXTR_FLAGS & EXTR_HAS_FEATURE) )
    if ( bFoundFeature( prep_inp_data->at, prep_inp_data->num_inp_atoms ) ) {
        sd->bExtract |= EXTR_HAS_FEATURE;
    }
#endif


    /* Find whether the structure can be disconnected or is a salt */

    /* Needs salt disconnection? */

    if ( ip->bTautFlags & TG_FLAG_DISCONNECT_SALTS )
    {
        prep_inp_data->bDisconnectSalts = (0 < DisconnectSalts( prep_inp_data, 0 ));
    }
    else
    {
        prep_inp_data->bDisconnectSalts = 0;
    }

    /* Needs metal disconnection? */

    if ( ip->bTautFlags & TG_FLAG_DISCONNECT_COORD )
    {
        i = (0 != (ip->bTautFlags & TG_FLAG_CHECK_VALENCE_COORD));
        bMayDisconnectMetals( prep_inp_data, i, &bTautFlagsDone ); /* changes prep_inp_data->bDisconnectCoord */
        sd->bTautFlagsDone[INCHI_BAS] |= bTautFlagsDone; /* whether any disconnection has been rejected because of the metal proper valence */

#if ( bRELEASE_VERSION == 0 )
        if ( i && (bTautFlagsDone & TG_FLAG_CHECK_VALENCE_COORD_DONE) ) {
            sd->bExtract |= EXTR_METAL_WAS_NOT_DISCONNECTED;
        }
#endif
    }
    else
    {
        prep_inp_data->bDisconnectCoord = 0;
    }
    orig_inp_data->bDisconnectSalts = prep_inp_data->bDisconnectSalts;
    orig_inp_data->bDisconnectCoord = prep_inp_data->bDisconnectCoord;



    /* 3. if( orig_inp_data->bDisconnectSalts ) then
          disconnect salts in prep_inp_data    */

    if ( ( ip->bTautFlags & TG_FLAG_DISCONNECT_SALTS ) && prep_inp_data->bDisconnectSalts &&
         0 < (i=DisconnectSalts( prep_inp_data, 1 )) )
    {
        WarningMessage(sd->pStrErrStruct, "Salt was disconnected");
        sd->bTautFlagsDone[INCHI_BAS] |= TG_FLAG_DISCONNECT_SALTS_DONE;
        if ( sd->nErrorType < _IS_WARNING )
        {
            sd->nErrorType = _IS_WARNING;
        }
        if ( i = ReconcileAllCmlBondParities( prep_inp_data->at, prep_inp_data->num_inp_atoms, 0 ) )
        {
            char szErrCode[16];
            sprintf( szErrCode, "%d", i);
            AddErrorMessage( sd->pStrErrStruct, "0D Parities Reconciliation failed:" );
            AddErrorMessage( sd->pStrErrStruct, szErrCode );
        }

#if ( bRELEASE_VERSION == 0 )
        sd->bExtract |= EXTR_SALT_WAS_DISCONNECTED;
#endif
    }
    else
    {
        prep_inp_data->bDisconnectSalts = 0;
    }


    /*  Mark the (disconnected) components in prep_inp_data    */

    prep_inp_data->num_components = MarkDisconnectedComponents( prep_inp_data, 0 );

    if ( prep_inp_data->num_components < 0 )
    {
        AddErrorMessage(sd->pStrErrStruct, "Out of RAM");
        sd->nStructReadError =  99;
        sd->nErrorType = _IS_FATAL;
        goto exit_function;
    }


    /* Detect isotopic H on heteroatoms -- necessary condition
       for global isotopic tautomerism */

    if ( i = bNumHeterAtomHasIsotopicH( prep_inp_data->at, prep_inp_data->num_inp_atoms ) )
    {
        if ( i & 1 )
        {
            sd->bTautFlagsDone[INCHI_BAS] |= TG_FLAG_FOUND_ISOTOPIC_H_DONE;
        }
        if ( i & 2 )
        {
            sd->bTautFlagsDone[INCHI_BAS] |= TG_FLAG_FOUND_ISOTOPIC_ATOM_DONE;
        }
    }


    /* 4a. Detect unusual valences                                              */

    if ( bCheckUnusualValences( prep_inp_data, 1, sd->pStrErrStruct ) )
    {

#if ( bRELEASE_VERSION == 0 )
        sd->bExtract |= EXTR_UNUSUAL_VALENCES;
#else
        ;
#endif
    }


    /*    5. if( orig_inp_data->bDisconnectCoord ) then
              -- copy prep_inp_data --> prep_inp_data+1
              -- disconnect metals in prep_inp_data            */

    if ( prep_inp_data->bDisconnectCoord )
    {

        prep_inp_data->num_components = MarkDisconnectedComponents( prep_inp_data, 0 );
        if ( prep_inp_data->num_components < 0 ) {
            AddErrorMessage(sd->pStrErrStruct, "Out of RAM");
            sd->nStructReadError =  99;
            sd->nErrorType = _IS_FATAL;
            goto exit_function;
        }

        /* Save reconnected structure in prep_inp_data+1 if requested */
        if ( 0 != ( ip->bTautFlags & TG_FLAG_RECONNECT_COORD) )
        {
            if ( 0 > DuplicateOrigAtom( prep_inp_data+1, prep_inp_data ) )
            {
                AddErrorMessage(sd->pStrErrStruct, "Out of RAM");
                sd->nStructReadError =  99;
                sd->nErrorType = _IS_FATAL;
                goto exit_function;
            }
            sd->bTautFlags[INCHI_REC]     = sd->bTautFlags[INCHI_BAS];
            sd->bTautFlagsDone[INCHI_REC] = sd->bTautFlagsDone[INCHI_BAS];
            {
                /* Remove "parity undefined in disconnected structure" flag from reconnected structure */
                int k, m, p;
                inp_ATOM *at     = (prep_inp_data+1)->at;
                int       num_at = (prep_inp_data+1)->num_inp_atoms;
                for ( k = 0; k < num_at; k ++ ) {
                    for ( m = 0; m < MAX_NUM_STEREO_BONDS && (p=at[k].sb_parity[m]); m ++ ) {
                        at[k].sb_parity[m] &= SB_PARITY_MASK;
                    }
                }
            }
        }

        /* Make disconnected structure in prep_inp_data */
        i = (0 != ( ip->bTautFlags & TG_FLAG_CHECK_VALENCE_COORD ));

        /*    prep_inp_data->bDisconnectCoord > 1 means add
                prep_inp_data->bDisconnectCoord-1 explicit H atoms    */
        if ( 0 < (i = DisconnectMetals( prep_inp_data, i, &bTautFlagsDone ) ) )
        {
            WarningMessage(sd->pStrErrStruct, "Metal was disconnected");
            sd->bTautFlagsDone[INCHI_BAS] |= TG_FLAG_DISCONNECT_COORD_DONE;
            if ( sd->nErrorType < _IS_WARNING )
            {
                sd->nErrorType = _IS_WARNING;
            }

#if ( bRELEASE_VERSION == 0 )
            sd->bExtract |= EXTR_METAL_WAS_DISCONNECTED;
#endif

            /* last parm=1 means find link between unchanged by Metal Disconnection components */
            prep_inp_data->num_components = MarkDisconnectedComponents( prep_inp_data, 1 );

            if ( prep_inp_data->num_components < 0 )
            {
                AddErrorMessage(sd->pStrErrStruct, "Out of RAM");
                sd->nStructReadError =  99;
                sd->nErrorType = _IS_FATAL;
                goto exit_function;
            }

            {
                /* Set parities for the disconnected structure */
                int k, m, p;
                inp_ATOM *at     = (prep_inp_data)->at;
                int       num_at = (prep_inp_data)->num_inp_atoms;
                for ( k = 0; k < num_at; k ++ ) {
                    for ( m = 0; m < MAX_NUM_STEREO_BONDS && (p=at[k].sb_parity[m]); m ++ ) {
                        if ( p & SB_PARITY_FLAG ) {
                            at[k].sb_parity[m] = (p >> SB_PARITY_SHFT) & SB_PARITY_MASK;
                        }
                    }
                }
            }

            if ( i = ReconcileAllCmlBondParities( prep_inp_data->at, prep_inp_data->num_inp_atoms, 1 ) )
            {
                char szErrCode[16];
                sprintf( szErrCode, "%d", i);
                AddErrorMessage( sd->pStrErrStruct, "0D Parities Reconciliation failed:" );
                AddErrorMessage( sd->pStrErrStruct, szErrCode );
            }

#if ( REMOVE_ION_PAIRS_DISC_STRU == 1 )
            if ( 0 < remove_ion_pairs( prep_inp_data->num_inp_atoms, prep_inp_data->at ) )
            {
                WarningMessage(sd->pStrErrStruct, "Charges were rearranged");
                if ( sd->nErrorType < _IS_WARNING )
                {
                    sd->nErrorType = _IS_WARNING;
                }
                sd->bTautFlagsDone[INCHI_REC] |= TG_FLAG_FIX_ODD_THINGS_DONE;
                sd->bTautFlagsDone[INCHI_BAS] |= TG_FLAG_FIX_ODD_THINGS_DONE;
            }
#endif

            /*
              if prep_inp_data->nOldCompNumber[i] = iINChI+1 > 0 then
              component #(i+1) in prep_inp_data is identical to component #(iINChI+1) in prep_inp_data+1
            */
        }
        else if ( i < 0 )
        {
            AddErrorMessage(sd->pStrErrStruct, "Cannot disconnect metal error");
            sd->nStructReadError =  i;
            sd->nErrorType = _IS_ERROR;
            goto exit_function;
        }
    }
    else
    {
        /* Remove "disconnected structure parities" from the structure */
        int k, m, p;
        inp_ATOM *at     = (prep_inp_data)->at;
        int       num_at = (prep_inp_data)->num_inp_atoms;
        for ( k = 0; k < num_at; k ++ ) {
            for ( m = 0; m < MAX_NUM_STEREO_BONDS && (p=at[k].sb_parity[m]); m ++ ) {
                at[k].sb_parity[m] &= SB_PARITY_MASK;
            }
        }
    }



exit_function:
    if ( sd->nErrorType < _IS_ERROR && prep_inp_data )
    {
        if ( 0 < post_fix_odd_things( prep_inp_data->num_inp_atoms, prep_inp_data->at ) )
        {
            WarningMessage(sd->pStrErrStruct, "Charges were rearranged");
            if ( sd->nErrorType < _IS_WARNING )
                sd->nErrorType = _IS_WARNING;
            sd->bTautFlagsDone[INCHI_BAS] |= TG_FLAG_FIX_ODD_THINGS_DONE;
        }
        if ( (sd->bTautFlagsDone[INCHI_BAS] & TG_FLAG_DISCONNECT_COORD_DONE) &&
             (prep_inp_data+1)->at && (prep_inp_data+1)->num_inp_atoms > 0 )
        {
            if ( 0 < post_fix_odd_things( (prep_inp_data+1)->num_inp_atoms, (prep_inp_data+1)->at ) )
            {
                WarningMessage(sd->pStrErrStruct, "Charges were rearranged");
                if ( sd->nErrorType < _IS_WARNING )
                    sd->nErrorType = _IS_WARNING;

                sd->bTautFlagsDone[INCHI_REC] |= TG_FLAG_FIX_ODD_THINGS_DONE;
                sd->bTautFlagsDone[INCHI_BAS] |= TG_FLAG_FIX_ODD_THINGS_DONE;
            }
        }
    }

    sd->bTautFlags[INCHI_BAS]     |= bTautFlags;  /* TG_FLAG_CHECK_VALENCE_COORD_DONE, TG_FLAG_MOVE_CHARGE_COORD_DONE */
    sd->bTautFlagsDone[INCHI_BAS] |= bTautFlagsDone;  /* TG_FLAG_CHECK_VALENCE_COORD_DONE, TG_FLAG_MOVE_CHARGE_COORD_DONE */

    return sd->nErrorType;
}




#ifndef TARGET_API_LIB


/*
    Create Composite Norm Atom
*/
int CreateCompositeNormAtom(COMP_ATOM_DATA *composite_norm_data,
                            INP_ATOM_DATA2 *all_inp_norm_data,
                            int num_components)
{
    int i, j, jj, k, n, m, tot_num_at, tot_num_H, cur_num_at, cur_num_H, nNumRemovedProtons;
    int num_comp[TAUT_NUM+1], num_taut[TAUT_NUM+1], num_del[TAUT_NUM+1], num_at[TAUT_NUM+1], num_inp_at[TAUT_NUM+1];
    int ret = 0, indicator = 1;
    inp_ATOM *at, *at_from;
    memset( num_comp, 0, sizeof(num_comp) );
    memset( num_taut, 0, sizeof(num_taut) );
    memset( num_del, 0, sizeof(num_taut) );
    /* count taut and non-taut components */
    for ( j = 0; j < TAUT_NUM; j ++ ) {
        num_comp[j] = num_taut[j] = 0;
        for ( i = 0; i < num_components; i ++ ) {
            if ( all_inp_norm_data[i][j].bExists ) {
                num_del[j]  += (0 != all_inp_norm_data[i][j].bDeleted );
                num_comp[j] ++;
                num_taut[j] += (0 != all_inp_norm_data[i][j].bTautomeric);
            }
        }
    }
    /* count intermediate taut structure components */
    if ( num_comp[TAUT_YES] > num_del[TAUT_YES] && num_taut[TAUT_YES] ) {
        /*
        num_comp[TAUT_INI] = num_comp[TAUT_YES] - num_del[TAUT_YES];
        */

        for ( i = 0, j=TAUT_YES; i < num_components; i ++ ) {
            if ( all_inp_norm_data[i][j].bExists &&
                (all_inp_norm_data[i][j].bDeleted ||
                 all_inp_norm_data[i][j].bTautomeric &&
                 all_inp_norm_data[i][j].at_fixed_bonds &&
                 all_inp_norm_data[i][j].bTautPreprocessed) ) {
                num_comp[TAUT_INI] ++;
            }
        }
    }
    /* count atoms and allocate composite atom data */
    for ( jj = 0; jj <= TAUT_INI; jj ++ ) {
        num_at[jj] = num_inp_at[jj] = 0;
        j = inchi_min (jj, TAUT_YES);
        if ( num_comp[jj] ) {
            for ( i = 0; i < num_components; i ++ ) {
                if ( all_inp_norm_data[i][j].bDeleted )
                    continue;
                /* find k = the normaized structure index */
                if ( jj == TAUT_INI ) {
                    if ( all_inp_norm_data[i][j].bExists &&
                         all_inp_norm_data[i][j].at_fixed_bonds ) {
                        k = j;
                    } else
                    if ( all_inp_norm_data[i][ALT_TAUT(j)].bExists && !all_inp_norm_data[i][ALT_TAUT(j)].bDeleted &&
                         !all_inp_norm_data[i][j].bDeleted  ) {
                        k = ALT_TAUT(j);
                    } else
                    if ( all_inp_norm_data[i][j].bExists ) {
                        k = j;
                    } else {
                        continue;
                    }
                } else {
                    if ( all_inp_norm_data[i][j].bExists ) {
                        k = j;
                    } else
                    if ( all_inp_norm_data[i][ALT_TAUT(j)].bExists && !all_inp_norm_data[i][ALT_TAUT(j)].bDeleted) {
                        k = ALT_TAUT(j);
                    } else {
                        continue;
                    }
                }
                num_inp_at[jj] += all_inp_norm_data[i][k].num_at; /* all atoms including terminal H */
                num_at[jj]     += all_inp_norm_data[i][k].num_at - all_inp_norm_data[i][k].num_removed_H;
            }
            if ( num_inp_at[jj] ) {
                if ( !CreateCompAtomData( composite_norm_data+jj, num_inp_at[jj], num_components, jj == TAUT_INI ) )
                    goto exit_error;
                composite_norm_data[jj].num_removed_H = num_inp_at[jj] - num_at[jj];
            }
        }
    }
    /* fill out composite atom */
    for ( jj = 0; jj <= TAUT_INI; jj ++, indicator <<= 1 ) {
        j = inchi_min (jj, TAUT_YES);
        if ( num_comp[jj] ) {
            tot_num_at = 0;
            tot_num_H = 0;
            for ( i = 0; i < num_components; i ++ ) {
                if ( all_inp_norm_data[i][j].bDeleted ) {
                    composite_norm_data[jj].nNumRemovedProtons += all_inp_norm_data[i][j].nNumRemovedProtons;
                    for ( n = 0; n < NUM_H_ISOTOPES; n ++ ) {
                        composite_norm_data[jj].nNumRemovedProtonsIsotopic[n] += all_inp_norm_data[i][j].nNumRemovedProtonsIsotopic[n];
                    }
                    continue;
                }
                nNumRemovedProtons = 0;
                k = TAUT_NUM;
                /* find k = the normaized structure index */
                if ( jj == TAUT_INI ) {
                    if ( all_inp_norm_data[i][j].bExists && all_inp_norm_data[i][j].at_fixed_bonds ) {
                        k = j;
                    } else
                    if ( all_inp_norm_data[i][ALT_TAUT(j)].bExists ) {
                        k = ALT_TAUT(j);
                    } else
                    if ( all_inp_norm_data[i][j].bExists && !all_inp_norm_data[i][ALT_TAUT(j)].bDeleted ) {
                        k = j;
                    } else {
                        continue;
                    }
                } else {
                    if ( all_inp_norm_data[i][j].bExists ) {
                        k = j;
                    } else
                    if ( all_inp_norm_data[i][ALT_TAUT(j)].bExists && !all_inp_norm_data[i][ALT_TAUT(j)].bDeleted ) {
                        k = ALT_TAUT(j);
                    } else {
                        continue;
                    }
                }
                /* copy main atoms */
                cur_num_H  = all_inp_norm_data[i][k].num_removed_H;       /* number of terminal H atoms */
                cur_num_at = all_inp_norm_data[i][k].num_at - cur_num_H;  /* number of all but explicit terminal H atoms */

                if ( (tot_num_at + cur_num_at) > num_at[jj] ||
                     (num_at[jj] + tot_num_H + cur_num_H) > num_inp_at[jj] ) {
                    goto exit_error; /* miscount */
                }
                at      = composite_norm_data[jj].at+tot_num_at; /* points to the 1st destination atom */
                at_from = (jj == TAUT_INI && k == TAUT_YES && all_inp_norm_data[i][k].at_fixed_bonds)?
                          all_inp_norm_data[i][k].at_fixed_bonds : all_inp_norm_data[i][k].at;
                memcpy( at, at_from, sizeof(composite_norm_data[0].at[0]) * cur_num_at ); /* copy atoms except terminal H */
                /* shift neighbors of main atoms */
                for ( n = 0; n < cur_num_at; n ++, at ++ ) {
                    for ( m = 0; m < at->valence; m ++ ) {
                        at->neighbor[m] += tot_num_at;
                    }
                }
                /* copy explicit H */
                if ( cur_num_H ) {
                    at = composite_norm_data[jj].at+num_at[jj]+tot_num_H; /* points to the 1st destination atom */
                    memcpy( at, at_from+cur_num_at,
                            sizeof(composite_norm_data[0].at[0]) * cur_num_H );
                    /* shift neighbors of explicit H atoms */
                    for ( n = 0; n < cur_num_H; n ++, at ++ ) {
                        for ( m = 0; m < at->valence; m ++ ) {
                            at->neighbor[m] += tot_num_at;
                        }
                    }
                }
                /* composite counts */
                composite_norm_data[jj].bHasIsotopicLayer   |= all_inp_norm_data[i][k].bHasIsotopicLayer;
                composite_norm_data[jj].num_isotopic        += all_inp_norm_data[i][k].num_isotopic;
                composite_norm_data[jj].num_bonds           += all_inp_norm_data[i][k].num_bonds;
                composite_norm_data[jj].bTautomeric         += (j == jj) && all_inp_norm_data[i][k].bTautomeric;
                composite_norm_data[jj].nNumRemovedProtons  += all_inp_norm_data[i][k].nNumRemovedProtons;
                for ( n = 0; n < NUM_H_ISOTOPES; n ++ ) {
                    composite_norm_data[jj].nNumRemovedProtonsIsotopic[n] += all_inp_norm_data[i][k].nNumRemovedProtonsIsotopic[n];
                    composite_norm_data[jj].num_iso_H[n]                  += all_inp_norm_data[i][k].num_iso_H[n];
                }
                /*
                composite_norm_data[j].num_at            += cur_num_at + cur_num_H;
                composite_norm_data[j].num_removed_H     += cur_num_H;
                */
                /* total count */
                tot_num_at += cur_num_at;
                tot_num_H += cur_num_H;
                /* offset for the next component */
                if (  composite_norm_data[jj].nOffsetAtAndH ) {
                    composite_norm_data[jj].nOffsetAtAndH[2*i]   = tot_num_at;
                    composite_norm_data[jj].nOffsetAtAndH[2*i+1] = num_at[jj]+tot_num_H;
                }
            }
            if ( tot_num_at != num_at[jj] ||
                 num_at[jj] + tot_num_H  != num_inp_at[jj] ) {
                goto exit_error; /* miscount */
            }
            composite_norm_data[jj].bExists       = (tot_num_at>0);
            ret |= indicator;
        }
    }
    return ret;

exit_error:
    return ret;
}
#endif


/*
    Make a copy of ORIG_ATOM_DATA structure
    ( inp_ATOM array, etc., etc. )
*/
int OrigAtData_CreateCopy( ORIG_ATOM_DATA *new_orig_atom,
                                  ORIG_ATOM_DATA *orig_atom )
{
    inp_ATOM  *at                 = NULL;
    AT_NUMB   *nCurAtLen          = NULL;
    AT_NUMB   *nOldCompNumber     = NULL;

    at = (inp_ATOM *)inchi_calloc( orig_atom->num_inp_atoms+1,
                                       sizeof(at[0]));

    nCurAtLen = (AT_NUMB *)inchi_calloc( orig_atom->num_components+1,
                                             sizeof(nCurAtLen[0]));

    nOldCompNumber = (AT_NUMB *)inchi_calloc( orig_atom->num_components+1,
                                                  sizeof(nOldCompNumber[0]));

    if ( at && nCurAtLen && nOldCompNumber )
    {
        /* Copy */
        if ( orig_atom->at )
            memcpy( at, orig_atom->at,
                    orig_atom->num_inp_atoms * sizeof(new_orig_atom->at[0]) );
        if ( orig_atom->nCurAtLen )
            memcpy( nCurAtLen, orig_atom->nCurAtLen,
                    orig_atom->num_components*sizeof(nCurAtLen[0]) );
        if ( orig_atom->nOldCompNumber )
            memcpy( nOldCompNumber, orig_atom->nOldCompNumber,
                    orig_atom->num_components*sizeof(nOldCompNumber[0]) );

        /* Deallocate */
        if ( new_orig_atom->at && new_orig_atom->at != at )
            inchi_free( new_orig_atom->at );
        if ( new_orig_atom->nCurAtLen && new_orig_atom->nCurAtLen!=nCurAtLen )
            inchi_free( new_orig_atom->nCurAtLen );
        if ( new_orig_atom->nOldCompNumber &&
             new_orig_atom->nOldCompNumber != nOldCompNumber )
            inchi_free( new_orig_atom->nOldCompNumber );

        *new_orig_atom                = *orig_atom;
        new_orig_atom->at             = at;

        new_orig_atom->nCurAtLen      = nCurAtLen;
        new_orig_atom->nOldCompNumber = nOldCompNumber;

        new_orig_atom->nCurAtLen      = nCurAtLen;
        new_orig_atom->nOldCompNumber = nOldCompNumber;

        /* Data that are not to be copied */
        new_orig_atom->nNumEquSets    = 0;    /*** no matter ***/
        memset(new_orig_atom->bSavedInINCHI_LIB, 0, sizeof(new_orig_atom->bSavedInINCHI_LIB));    /*** no matter ***/
        memset(new_orig_atom->bPreprocessed,    0, sizeof(new_orig_atom->bPreprocessed));    /*** no matter ***/

        /* Arrays that are not to be copied */
        new_orig_atom->szCoord        = NULL;    /*** no matter ***/
        new_orig_atom->nEquLabels     = NULL;    /*** no matter ***/
        new_orig_atom->nSortedOrder   = NULL;    /*** no matter ***/

        new_orig_atom->bDisconnectCoord = orig_atom->bDisconnectCoord;
        new_orig_atom->bDisconnectSalts = orig_atom->bDisconnectSalts;

        new_orig_atom->polymer          = NULL;
        new_orig_atom->v3000          = NULL;

        return 0;
    }

    /* Deallocate */
    if ( at && new_orig_atom->at != at )
        inchi_free( at );
    if ( nCurAtLen && new_orig_atom->nCurAtLen != nCurAtLen )
        inchi_free( nCurAtLen );
    if ( nOldCompNumber && new_orig_atom->nOldCompNumber != nOldCompNumber )
        inchi_free( nOldCompNumber );

    return -1; /* Failed */
}


/*
    OrigAtData debug output
*/
void OrigAtData_DebugTrace( ORIG_ATOM_DATA* d )
{
int i, k;

    ITRACE_( "\n\n*********************************************************************\n* ORIG_ATOM_DATA @ 0x%p", d );
    ITRACE_( "\n*  num_inp_atoms = %-d\n*  num_inp_bonds = %-d\n*  num_dimensions = %-d\n*  num_components = %-d",
            d->num_inp_atoms, d->num_inp_bonds, d->num_dimensions, d->num_components  );
    ITRACE_( "\n*  ATOMS");
    for (i=0; i<d->num_inp_atoms; i++)
    {
        ITRACE_( "\n*    #%-5d %s%-d ( charge %-d, rad %-d nH %-d val %-d) [%-f %-f %-f]",
            i, d->at[i].elname, d->at[i].orig_at_number, d->at[i].charge, d->at[i].radical, d->at[i].num_H, d->at[i].valence,
            d->at[i].x, d->at[i].y, d->at[i].z);
        if ( d->at[i].valence > 0 )
        {
            ITRACE_( "\n            bonds to     " );
            for (k=0; k < d->at[i].valence; k++)
                ITRACE_( "%-3d ", d->at[i].neighbor[k] );
        }
        if ( d->at[i].valence > 0 )
        {
            ITRACE_( "\n            bond types   " );
            for (k=0; k < d->at[i].valence; k++)
                ITRACE_( "%-3d ", d->at[i].bond_type[k] );
        }
    }
    /*OrigAtDataPolymer_DebugTrace( d->polymer );*/
    ITRACE_( "\n* V3000 INFO @ 0x%-p", d->v3000 );
    ITRACE_( "\n*\n" );
    if ( d->v3000 )
    {
        ITRACE_( "\n*  n_star_atoms = %-d\n*  n_haptic_bonds = %-d\n*  n_collections = %-d",
            d->v3000->n_star_atoms , d->v3000->n_haptic_bonds, d->v3000->n_collections  );
    }
    ITRACE_( "\n*\n* End ORIG_ATOM_DATA\n*********************************************************************\n" );

    return;
}




/*

    Polymer related procedures

*/




/*    Create a new OrigAtDataPolymerUnit */
OrigAtDataPolymerUnit * OrigAtDataPolymerUnit_New( int maxatoms, int maxbonds,
                                                   int id, int label, int type,
                                                   int subtype, int conn,
                                                   char *smt,
                                                   int na, INT_ARRAY *alist,
                                                   int nb, INT_ARRAY *blist,
                                                   int npsbonds, int **psbonds )
{
int k, err = 0;
OrigAtDataPolymerUnit *u2 = NULL;

    u2 = (OrigAtDataPolymerUnit*) inchi_calloc( 1, sizeof(OrigAtDataPolymerUnit) );
    if ( NULL==u2 )
    {
        err = 1;
        goto exitf;
    }
    u2->id                = id;
    u2->label            = label;
    u2->type            = type;
    u2->subtype            = subtype;
    u2->conn            = conn;
    u2->na                = na;
    u2->nb                = nb;
    u2->real_kind        = POLYMER_UNIT_KIND_UNKNOWN;
    u2->disjoint        = 0;
    u2->closeable        = CLOSING_SRU_NOT_APPLICABLE;
    u2->already_closed    = 0;
    for (k=0;k<4;k++)
    {
        u2->xbr1[k]        = 0.0;
        u2->xbr2[k]        = 0.0;
    }
    strcpy( u2->smt, smt );
    u2->star1            = 0;
    u2->star_partner1    = 0;
    u2->star2            = 0;
    u2->star_partner2    = 0;
    u2-> maxpsbonds        = maxbonds;
    u2->npsbonds        = npsbonds;

    u2->alist = NULL;
    if ( na > 0 || maxatoms > 0 )
    {
        u2->alist = (int *) inchi_calloc( na > 0 ? na : maxatoms, sizeof(int) );
        if ( !u2->alist )
        {
            err = 2;
            goto exitf;
        }
        for (k=0; k<na; k++)
            u2->alist[k] = alist->item[k];
    }
    u2->blist = NULL;
    if ( nb > 0 || maxbonds > 0 )
    {
        u2->blist = (int *) inchi_calloc( nb > 0 ? 2*nb : 2*maxbonds, sizeof(int) );
        if ( !u2->blist )
        {
            err = 3;
            goto exitf;
        }
        if ( blist )
        {
            for (k=0; k<2*nb; k++)
                u2->blist[k] = blist->item[k];
        }
    }
    u2->psbonds = NULL;

exitf:
    if ( err )
    {
        OrigAtDataPolymerUnit_Free( u2 );
        return NULL;
    }

    return u2;
}


/*    Create a copy of OrigAtDataPolymerUnit */
OrigAtDataPolymerUnit *
OrigAtDataPolymerUnit_CreateCopy(OrigAtDataPolymerUnit *u)
{
int k, err = 0;
OrigAtDataPolymerUnit *u2 = NULL;

    u2 = (OrigAtDataPolymerUnit*) inchi_calloc( 1, sizeof(OrigAtDataPolymerUnit) );
    if ( NULL==u2 )
    {
        err = 1;
        goto exitf;
    }
    u2->id                = u->id;
    u2->type            = u->type;
    u2->subtype            = u->subtype;
    u2->conn            = u->conn;
    u2->label            = u->label;
    u2->na                = u->na;
    u2->nb                = u->nb;
    u2->real_kind        = u->real_kind;
    u2->disjoint        = u->disjoint;
    u2->closeable        = u->closeable;
    u2->already_closed    = u->already_closed;
    for (k=0;k<4;k++)
    {
        u2->xbr1[k]        = u->xbr1[k];
        u2->xbr2[k]        = u->xbr2[k];
    }
    strcpy( u2->smt, u->smt );
    u2->star1            = u->star1;
    u2->star_partner1    = u->star_partner1;
    u2->star2            = u->star2;
    u2->star_partner2    = u->star_partner2;
    u2->npsbonds        = u->npsbonds;
    u2-> maxpsbonds        = inchi_max( u->maxpsbonds, u->npsbonds );

    u2->alist = (int *) inchi_calloc( u2->na, sizeof(int) );
    if ( NULL == u2->alist )
    {
        err = 2;
        goto exitf;
    }
    for (k=0; k<u2->na; k++)
        u2->alist[k] = u->alist[k];

    u2->blist = (int *) inchi_calloc( 2*u2->nb, sizeof(int) );
    if ( NULL == u2->blist )
    {
        err = 2;
        goto exitf;
    }
    for (k=0; k<2*u2->nb; k++)
        u2->blist[k] = u->blist[k];

    err = imat_new( u2->maxpsbonds, 2, &(u2->psbonds) );
    if ( !err )
    {
        for ( k = 0; k<u2->npsbonds; k++ )
        {
            u2->psbonds[k][0] = u->psbonds[k][0];
            u2->psbonds[k][1] = u->psbonds[k][1];
        }
    }

exitf:
    if ( err )
    {
        OrigAtDataPolymerUnit_Free( u2 );
        return NULL;
    }
    return u2;
}


/*    OrigAtDataPolymerUnit_Free    */
void OrigAtDataPolymerUnit_Free( OrigAtDataPolymerUnit *unit )
{
    ITRACE_("\n************** About to free OrigAtDataPolymerUnit @ %-p\n", unit );
    OrigAtDataPolymerUnit_DebugTrace( unit);
    if ( unit )
    {
        if ( unit->alist )        { inchi_free( unit->alist );  unit->alist = NULL; }
        if ( unit->blist )        { inchi_free( unit->blist );  unit->blist = NULL; }
        if ( unit->psbonds )
        {
            imat_free( unit->maxpsbonds, unit->psbonds );
            unit->psbonds = NULL;
        }
    }
    inchi_free( unit );
    return;
}


/*
    Compare two polymer units, modified lexicographic order
    Modification: unit with smaller alist always go first
*/
int  OrigAtDataPolymerUnit_CompareAtomListsMod( OrigAtDataPolymerUnit* u1, OrigAtDataPolymerUnit* u2 )
{
int i;
    int n1    = u1->na;
    int n2    = u2->na;
    int n    = n1;
    if ( n1 < n2 )    return -1;
    if ( n1 > n2 )    return 1;
    /* n1 == n2 == n */
    for (i=0; i<n; i++)
    {
        if ( u1->alist[i] < u2->alist[i] )    return -1;
        if ( u1->alist[i] > u2->alist[i] )    return    1;
    }
    return 0;
}


/*    Compare two polymer units, lexicographic order */
int  OrigAtDataPolymerUnit_CompareAtomLists( OrigAtDataPolymerUnit* u1, OrigAtDataPolymerUnit* u2 )
{
int i;
    int n1 = u1->na;
    int n2 = u2->na;
    int n = inchi_min(n1, n2);
    for (i=0; i<n; i++)
    {
        if ( u1->alist[i] < u2->alist[i] )    return -1;
        if ( u1->alist[i] > u2->alist[i] )    return 1;
    }
    if ( n1 < n2 )    return -1;
    if ( n1 > n2 )    return    1;
    return 0;
}


/*    Sort SRU bond lists atoms and bonds themselves */
int  OrigAtDataPolymerUnit_OrderBondAtomsAndBondsThemselves( OrigAtDataPolymerUnit *u,
                                                             int n_star_atoms,
                                                             int *star_atoms )
{
int k;
    /* Sort bond atoms */
    for (k=0; k<u->nb; k++)
    {
        /* Place not-in-unit bond end to first place */

        int a1 = u->blist[ 2*k ];
        int a2 = u->blist[ 2*k+1 ];
        int a1_is_not_in_alist    = 0,
            a1_is_star_atom        = 0,
            a2_is_not_in_alist    = 0,
            a2_is_star_atom        = 0;

        if ( !is_in_the_ilist( u->alist, a1, u->na ) )
            a1_is_not_in_alist = 1;
        if ( is_in_the_ilist( star_atoms, a1, n_star_atoms ) )
            a1_is_star_atom = 1;

        if ( !is_in_the_ilist( u->alist, a2, u->na ) )
            a2_is_not_in_alist = 1;
        if ( is_in_the_ilist( star_atoms, a2, n_star_atoms ) )
            a2_is_star_atom = 1;

        if ( ( a1_is_not_in_alist || a1_is_star_atom ) &&
             ( a2_is_not_in_alist || a2_is_star_atom )  )
        {
            /* Both the ends are out of unit: the crossing bond is invalid */
            return 1;
        }
        /* If a2 is star atom or non-star external to the current unit, swap(a2,a1) */
        if ( a2_is_star_atom || a2_is_not_in_alist )
        {
            u->blist[ 2*k ]            =    a2;
            u->blist[ 2*k + 1 ]        =    a1;
        }
    }

    /* Sort bond themselves
            for now, consider only the simplest cases of 2 bonds
    */
    if ( u->nb == 2 )            /* two bonds in SBL */
    {
        int b1a1 = u->blist[0];
        int b1a2 = u->blist[1];
        int b2a1 = u->blist[2];
        int b2a2 = u->blist[3];
        if ( b1a1 > b2a1 )
        {
            /* swap */
            u->blist[0] = b2a1; u->blist[1] = b2a2;
            u->blist[2] = b1a1; u->blist[3] = b1a2;
        }
    }

    /* for single or no bonds, do nothing
    else
        ;
    */

    return 0;
}


/*
    Parse polymer data (unit, types, subtypes, connections, etc.)
*/
int OrigAtDataPolymer_ParseAndValidate(    ORIG_ATOM_DATA *orig_at_data,
                                        int allowed, char *pStrErr )
{
    int i, k, kk, type, subtype, representation, err=0;
    int nsgroups = orig_at_data->polymer->n;
    int nat = orig_at_data->num_inp_atoms;
    OrigAtDataPolymer *pd = orig_at_data->polymer;
    OrigAtDataPolymerUnit* u = NULL;


    /* Quick first checks */

    if ( !orig_at_data->polymer )
        goto exitf;

    if ( nsgroups < 1 )
    {
        /* not a polymer */
        goto exitf;
    }

    if ( nsgroups == 1 )
    {
        /* Check if copolymer */
        type = pd->units[0]->type;
        if ( type == POLYMER_STY_COP )
            { TREAT_ERR (err, 9001, "Copolymer contains a single unit"); goto exitf; }
        /* Check if copolymer subtype */
        subtype = pd->units[0]->subtype;
        if ( subtype == POLYMER_SST_RAN || subtype == POLYMER_SST_ALT || subtype == POLYMER_SST_BLK )
            { TREAT_ERR (err, 9002, "Single polymer unit may not be RAN/ALT/BLO" ); goto exitf; }
    }

    for (i=0; i< nsgroups; i++)
    {
        u = pd->units[i];

        if ( u->nb != 0 && u->nb!=2 )
            { TREAT_ERR (err, 9003, "Number of crossing bonds in polymer unit is not 0 or 2"); goto exitf; }
        if ( u->na < 1 )
            { TREAT_ERR (err, 9004, "Empty polymer unit"); goto exitf;  }
        if ( u->na > nat )
            { TREAT_ERR (err, 9005, "Too large polymer unit"); goto exitf;  }
        for (k=0; k<u->na; k++)
        {
            int atom = u->alist[k];
            if ( atom < 1 || atom > nat )
                { TREAT_ERR (err, 9006, "Invalid atom number in polymer unit"); goto exitf; }
            if ( is_in_the_ilist( pd->star_atoms, atom, pd->n_star_atoms ) )
                { TREAT_ERR (err, 9007, "Star atom inside polymer unit"); goto exitf; }
        }

        /* Set possibly missing unit parameters */
        u->npsbonds            =    0;
        u->disjoint            =    0;
        u->closeable        =    CLOSING_SRU_NOT_APPLICABLE;
        u->already_closed    =    0;
        u->star1            =    0;
        u->star2            =    0;
        u->star_partner1    =    0;
        u->star_partner2    =    0;
        u->real_kind        =    POLYMER_UNIT_KIND_UNKNOWN;
    }


    /* Collect star atoms info */

    pd->n_star_atoms = 0;
    for (k=0; k<nat; k++ )
        if ( !strcmp(orig_at_data->at[k].elname,"Zz") )
            pd->n_star_atoms++;
    if ( pd->n_star_atoms > 0 )
    {
        pd->star_atoms =
            (int *) inchi_calloc( pd->n_star_atoms, sizeof( int ) );
        if ( !pd->star_atoms )
            { TREAT_ERR (err, 9010, "Not enough memory"); goto exitf;  }
        kk = 0;
        for (k=0; k<nat; k++ )
            if ( !strcmp(orig_at_data->at[k].elname,"Zz") )
                pd->star_atoms[ kk++ ] = k + 1;
    }


    /* Check copolymers and ensure that COP includes > 1 SRU */

    for (i=0; i<pd->n; i++)
    {
        u = pd->units[i];

        if ( u->type == POLYMER_STY_COP )
        {
            int j, in_units = 0;

            if ( u->nb > 0 )
                { TREAT_ERR (err, 9026, "Polymer COP unit contains bracket-crossing bonds, not supported"); goto exitf; }

            for (j=0; j<pd->n; j++)
            {
                if ( pd->units[j]->type == POLYMER_STY_COP )
                    continue;
                if ( is_ilist_inside( pd->units[j]->alist, pd->units[j]->na, pd->units[i]->alist, pd->units[i]->na ) )
                {
                    in_units++;
                    if ( in_units == 2 )
                        break;
                }
            }
            if ( in_units < 2 )
                { TREAT_ERR (err, 9027, "Polymer COP unit contains a single SRU instead of multiple"); goto exitf; }
        }
    }


    representation = OrigAtDataPolymer_GetRepresentation( pd );


    /* More checks and some corrections*/

    if ( representation == POLYMER_REPRESENTATION_SOURCE_BASED )
    {
        for (i=0; i< nsgroups; i++)
        {
            /* Replace source-based 'SRU' with 'MON' */
            if ( pd->units[i]->type == POLYMER_STY_SRU )
            {
                pd->units[i]->type = POLYMER_STY_MON;
                WarningMessage( pStrErr, "Converted src-based polymer unit type to MON" );
            }
            if ( pd->units[i]->type == POLYMER_STY_COP )
            {
                /* Set missing copolymer subtype to RAN */
                if ( pd->units[i]->subtype == POLYMER_SST_NON )
                {
                    pd->units[i]->subtype = POLYMER_SST_RAN;
                    WarningMessage( pStrErr, "Set missing copolymer subtype to RAN" );
                }
            }
            /* Suppress connectivity (�HH�, �HT�, �EU�) */
            if ( pd->units[i]->conn != POLYMER_CONN_NON )
            {
                pd->units[i]->conn = POLYMER_CONN_NON;
                WarningMessage( pStrErr, "Ignore connection pattern for src-based polymer unit" );
            }
            /* Recognize finally */
            if ( pd->units[i]->type == POLYMER_STY_MON )
            {
                u->real_kind = POLYMER_UNIT_KIND_SOURCE_BASED_COMPONENT;
            }
            else if ( pd->units[i]->type == POLYMER_STY_MOD )
            {
                u->real_kind = POLYMER_UNIT_KIND_SOURCE_BASED_COMPONENT;
            }
            else if ( pd->units[i]->type == POLYMER_STY_MER )
            {
                u->real_kind = POLYMER_UNIT_KIND_SOURCE_BASED_COMPONENT;
            }
            else if ( pd->units[i]->type == POLYMER_STY_CRO )
            {
                u->real_kind = POLYMER_UNIT_KIND_SOURCE_BASED_COMPONENT;
            }
            else if ( pd->units[i]->type == POLYMER_STY_COP )
            {
                u->real_kind = POLYMER_UNIT_KIND_SOURCE_BASED_COPOLYMER;
                if ( u->subtype == POLYMER_SST_ALT )
                    u->real_kind = POLYMER_UNIT_KIND_SOURCE_BASED_ALT_COPOLYMER;
                else if ( u->subtype == POLYMER_SST_BLK )
                    u->real_kind = POLYMER_UNIT_KIND_SOURCE_BASED_BLK_COPOLYMER;
                else if ( u->subtype == POLYMER_SST_RAN || u->subtype == POLYMER_SST_NON )
                    u->real_kind = POLYMER_UNIT_KIND_SOURCE_BASED_RAN_COPOLYMER;
            }
            else if ( pd->units[i]->type == POLYMER_STY_NON )
            {
                u->real_kind = POLYMER_UNIT_KIND_SOURCE_BASED_POLYMER;
            }
            else
            {
                TREAT_ERR (err, 9028, "Unrecognized kind of source-based represented polymer unit");
                goto exitf;
            }
        }
    }

    else if ( representation == POLYMER_REPRESENTATION_STRUCTURE_BASED )
    {
        for (i=0; i< nsgroups; i++)
        {
            int a1, a2, a1_is_not_in_alist, a1_is_star_atom, a2_is_not_in_alist, a2_is_star_atom;

            u = pd->units[i];

            OrigAtDataPolymerUnit_FindStarsAndPartners( u, orig_at_data, &err, pStrErr );

            /*    SRU that is copolymer unit embedding other SRU's */
            if ( u->nb == 0 )
            {
                if ( u->type == POLYMER_STY_COP )
                    ;
                else if ( u->type == POLYMER_STY_SRU )
                {
                    u->type = POLYMER_STY_COP;
                    WarningMessage( pStrErr, "Set copolymer embedding unit mark to COP" );
                }
            }
            if ( u->type == POLYMER_STY_COP )
            {
                u->real_kind = POLYMER_UNIT_KIND_SRU_EMBEDDING_STRUCTURE_BASED_SRUS;
                u->closeable = CLOSING_SRU_NOT_APPLICABLE;
                /* Set possibly missing copolymer subtype to RAN */
                if ( u->subtype == POLYMER_SST_NON )
                {
                    u->subtype = POLYMER_SST_RAN;
                    WarningMessage( pStrErr, "Set missing copolymer subtype to RAN" );
                }
                continue;
            }

            /*    SRU with endgroups or stars.
                Check it. */
            for (k=0; k<u->nb; k++)
            {
                a1 = u->blist[ 2*k ]; a2 = u->blist[ 2*k+1 ];
                if ( !strcmp( orig_at_data->at[a1-1].elname, "H" ) ||
                     !strcmp( orig_at_data->at[a1-1].elname, "D" ) ||
                     !strcmp( orig_at_data->at[a1-1].elname, "T" ) )
                    { TREAT_ERR (err, 9030, "H as polymer end group is not supported"); goto exitf; }
                if ( !strcmp( orig_at_data->at[a2-1].elname, "H" ) ||
                     !strcmp( orig_at_data->at[a2-1].elname, "D" ) ||
                     !strcmp( orig_at_data->at[a2-1].elname, "T" ) )
                    { TREAT_ERR (err, 9031, "H as polymer end group is not supported"); goto exitf;  }
                a1_is_not_in_alist    = a1_is_star_atom = 0;
                a2_is_not_in_alist    = a2_is_star_atom = 0;
                if ( !is_in_the_ilist( u->alist, a1, u->na ) )
                    a1_is_not_in_alist = 1;
                if ( is_in_the_ilist( pd->star_atoms, a1, pd->n_star_atoms ) )
                    a1_is_star_atom = 1;
                if ( !is_in_the_ilist( u->alist, a2, u->na ) )
                    a2_is_not_in_alist = 1;
                if ( is_in_the_ilist( pd->star_atoms, a2, pd->n_star_atoms ) )
                    a2_is_star_atom = 1;
                if ( ( a1_is_not_in_alist || a1_is_star_atom ) &&
                     ( a2_is_not_in_alist || a2_is_star_atom )  )
                { TREAT_ERR (err, 9032, "Ends of crossing bond lie inside polymer unit"); goto exitf; }
            }


            if ( u->type==POLYMER_STY_SRU  || u->type==POLYMER_STY_MOD        ||
                 u->type==POLYMER_STY_CRO  || u->type==POLYMER_STY_MER          )
            {

                /* If SRU connection is missing, set to default ('either') */
                if ( u->conn == POLYMER_CONN_NON )
                {
                    WarningMessage( pStrErr, "Set missing copolymer unit connection to EU" );
                    u->conn = POLYMER_CONN_EU;
                }

                if ( u->star1 && u->star2 )
                {
                    u->real_kind = POLYMER_UNIT_KIND_STRUCTURE_BASED_SRU_WITH_TWO_STARS;

                    /* Set SRU closure type */
                    if ( u->na == 1 )
                    {
#ifdef ALLOW_CLOSING_SRU_VIA_DIRADICAL
                        u->closeable = CLOSING_SRU_DIRADICAL;
#else
                        u->closeable = CLOSING_SRU_NOT_APPLICABLE;
#ifdef  CLOSING_STARRED_SRU_IS_A_MUST
                        TREAT_ERR (err, 9029, "Could not perform SRU closure");
                        goto exitf;
#endif
#endif
                    }
                    else if ( u->na == 2 )
                    {

#ifdef ALLOW_CLOSING_SRU_VIA_HIGHER_ORDER_BOND
                        u->closeable = CLOSING_SRU_HIGHER_ORDER_BOND;
#else
                        u->closeable = CLOSING_SRU_NOT_APPLICABLE;
#ifdef  CLOSING_STARRED_SRU_IS_A_MUST
                        TREAT_ERR (err, 9029, "Could not perform SRU closure");
                        goto exitf;
#endif
#endif
                    }
                    else
                    {
                        u->closeable = CLOSING_SRU_RING;
                    }
                }

                else if ( u->star1 < 1 && u->star2 < 1 )
                    u->real_kind = POLYMER_UNIT_KIND_STRUCTURE_BASED_SRU_WITH_NO_STARS;

                if ( u->closeable )
                {
                    /* Allocate PS (phase-shiftable) bonds */
                    u->maxpsbonds = orig_at_data->num_inp_bonds + 2;
                    err = imat_new( u->maxpsbonds, 2, &(u->psbonds) );
                    if ( err )
                        { TREAT_ERR ( err, 9034, "Not enough memory (polymers)" ); goto exitf; }
                }
            }

            if ( u->real_kind == POLYMER_UNIT_KIND_UNKNOWN )
                { TREAT_ERR (err, 9035, "Could not recognize type of polymer unit"); goto exitf; }
        }
    }
    else
    {
        { TREAT_ERR (err, 9035, "Invalid kind of polymer representation"); goto exitf; }
    }

    pd->valid = 1;

exitf:
    if ( err )
        pd->valid = 0;

    return err;
}


int UnMarkRingSystemsInp( inp_ATOM *at, int num_atoms );
/*

*/
int UnMarkRingSystemsInp( inp_ATOM *at, int num_atoms )
{
    int i;
    for ( i = 0; i < num_atoms; i ++ ) {
        at[i].bCutVertex         = 0;
        at[i].nRingSystem        = 0;
        at[i].nNumAtInRingSystem = 0;
        at[i].nBlockSystem       = 0;
    }
    return 0;
}


/*
    Preprocess OrigAtDataPolymer
    (NB: phase shift is invoked from here)
*/
int OrigAtDataPolymer_CyclizeCloseableUnits( ORIG_ATOM_DATA *orig_at_data, char *pStrErr )
{
int i, ncyclized=0, err=0;

    for (i=0; i<orig_at_data->polymer->n; i++)
    {
        OrigAtDataPolymerUnit *unit = orig_at_data->polymer->units[i];

        if ( unit->real_kind != POLYMER_UNIT_KIND_STRUCTURE_BASED_SRU_WITH_TWO_STARS )
            continue;
        if ( !unit->closeable )
            continue;

        /* Find stars and their partners */
        OrigAtDataPolymerUnit_FindStarsAndPartners( unit, orig_at_data, &err, pStrErr );

        if ( err )                break;
        if ( !unit->closeable )    continue;


        if ( OrigAtDataPolymerUnit_HasMetal( unit, orig_at_data->at ) )
        {
            /*unit->closeable = CLOSING_SRU_NOT_APPLICABLE;*/
            if ( unit->closeable == CLOSING_SRU_RING )
            {
                /*unit->closeable = CLOSING_SRU_HIGHER_ORDER_BOND;*/
                WarningMessage( pStrErr, "Phase shift in metallated polymer unit may be missed");
            }
        }

        /* Now remove bonds to star atoms and cyclize a SRU */
        OrigAtDataPolymerUnit_DetachStarsAndConnectStarPartners( unit, orig_at_data, &err, pStrErr );

        if ( err )                break;
        if ( !unit->closeable )    continue;

        ncyclized++;
    }

    /*
    if ( ncyclized )
        WarningMessage( pStrErr, "Made provision for phase shift in polymer unit(s)" );
    */

    return err;
}


/*

*/
int OrigAtDataPolymerUnit_HasMetal( OrigAtDataPolymerUnit *u, inp_ATOM *at)
{
int i;
    for (i=0; i < u->na; i++)
        if ( is_el_a_metal( at[ u->alist[i]-1].el_number ) )
            return 1;

    return 0;
}



/*
    OrigAtDataPolymer_Free
*/
void OrigAtDataPolymer_Free( OrigAtDataPolymer *pd )
{
    if ( pd )
    {
        if ( pd->star_atoms )
        {
            inchi_free( pd->star_atoms );
            pd->star_atoms = NULL;
            pd->n_star_atoms = 0;
        }
        if ( pd->n && pd->units )
        {
            int k;
            for (k=0; k<pd->n; k++)
            {
                OrigAtDataPolymerUnit_Free( pd->units[k] );
            }
            inchi_free( pd->units );
            pd->units = NULL;
            pd->n = 0;
        }
        inchi_free( pd );
        pd = NULL;
    }

    return;
}


/*
    OrigAtDataPolymerUnit_DetachStarsAndConnectStarPartners
*/
void OrigAtDataPolymerUnit_DetachStarsAndConnectStarPartners( OrigAtDataPolymerUnit *unit,
                                                              ORIG_ATOM_DATA *orig_inp_data,
                                                              int *err, char *pStrErr )
{
int bond_type, bond_stereo;

    *err = 0;
    if ( !unit->closeable )
        return;

    if ( unit->closeable == CLOSING_SRU_RING )
    {
        /* Disconnect both star atoms */
        OrigAtData_RemoveBond( unit->star1 - 1, unit->star_partner1 - 1, orig_inp_data->at,
                               &bond_type, &bond_stereo, &orig_inp_data->num_inp_bonds );

        OrigAtData_RemoveBond( unit->star2 - 1, unit->star_partner2 - 1, orig_inp_data->at,
                               &bond_type, &bond_stereo, &orig_inp_data->num_inp_bonds );

        OrigAtData_AddSingleStereolessBond( unit->star_partner1 - 1, unit->star_partner2 - 1,
                                            orig_inp_data->at, &orig_inp_data->num_inp_bonds );
    }

    else if ( unit->closeable == CLOSING_SRU_HIGHER_ORDER_BOND )
    {
        int elevated;
        elevated = OrigAtData_IncreaseBondOrder( unit->star_partner1 - 1, unit->star_partner2 - 1, orig_inp_data->at );
#if 0
/* the bond may already be broken at metal disconnection, so ignore the result here */
        if ( !elevated)
        {
            /* *err = 1; */
            WarningMessage( pStrErr, "SRU closure via higher order bond failed");
            unit->closeable = CLOSING_SRU_NOT_APPLICABLE;
            return;
        }
#endif
        OrigAtData_RemoveBond( unit->star1 - 1, unit->star_partner1 - 1, orig_inp_data->at,
                               &bond_type, &bond_stereo, &orig_inp_data->num_inp_bonds );
        OrigAtData_RemoveBond( unit->star2 - 1, unit->star_partner2 - 1, orig_inp_data->at,
                               &bond_type, &bond_stereo, &orig_inp_data->num_inp_bonds );
    }

    else if ( unit->closeable == CLOSING_SRU_DIRADICAL )
    {
        orig_inp_data->at[unit->star_partner1 - 1].radical = RADICAL_TRIPLET;
        OrigAtData_RemoveBond( unit->star1 - 1, unit->star_partner1 - 1, orig_inp_data->at,
                               &bond_type, &bond_stereo, &orig_inp_data->num_inp_bonds );
        OrigAtData_RemoveBond( unit->star2 - 1, unit->star_partner2 - 1, orig_inp_data->at,
                                &bond_type, &bond_stereo, &orig_inp_data->num_inp_bonds );
    }

    if ( !*err )
        unit->already_closed = 1;

    return;
}


/*
    OrigAtDataPolymerUnit_FindStarsAndPartners
*/
void OrigAtDataPolymerUnit_FindStarsAndPartners( OrigAtDataPolymerUnit *unit,
                                                 ORIG_ATOM_DATA *orig_at_data,
                                                 int *err, char *pStrErr )
{
int i, j, k;
int num_atoms;
int debug_polymers = 0;
#if ( DEBUG_POLYMERS == 1 )
    debug_polymers = 1;
#elif  ( DEBUG_POLYMERS == 2 )
    debug_polymers = 2;
#endif

    *err = 0;

    if ( !unit->blist || unit->nb < 1 )
        /*    We may get here e.g. for copolymer SRU (no crossing bonds) that embed
            actual structure-based SRU's which do have crossing bonds                */
        return;

    num_atoms = orig_at_data->num_inp_atoms;

    /* left bond 0-1 */
    i = unit->blist[0];
    j = unit->blist[1];
    unit->star_partner1 = i;
    unit->star1 = j;
    if ( strcmp( orig_at_data->at[unit->star1-1].elname, "Zz" ) )
    {
        unit->star_partner1 = j;
        unit->star1 = i;
    }
    if ( strcmp( orig_at_data->at[unit->star1-1].elname, "Zz" ) )
    {
        /* unexpectedly, not a unit->star atom */
        unit->closeable = CLOSING_SRU_NOT_APPLICABLE;
        unit->star1 = 0;
        return;
    }
    if ( unit->star_partner1 <= 0 || unit->star_partner1 > num_atoms || unit->star1 <= 0 || unit->star1 > num_atoms )
    {
        TREAT_ERR (*err, 9090, "Invalid polymeric CRU crossing bond");
        unit->closeable = CLOSING_SRU_NOT_APPLICABLE;
        return;
    }
    /* right bond 2-3 */
    i = unit->blist[2];
    j = unit->blist[3];
    unit->star_partner2 = i;
    unit->star2 = j;
    if ( strcmp( orig_at_data->at[unit->star2-1].elname, "Zz" ) )
    {
        unit->star_partner2 = j;
        unit->star2 = i;
    }
    if ( strcmp( orig_at_data->at[unit->star2-1].elname, "Zz" ) )
    {
        /* unexpectedly, not a unit->star atom */
        unit->closeable = CLOSING_SRU_NOT_APPLICABLE;
        unit->star2 = 0;
        return;
    }
    if ( unit->star_partner2 <= 0 || unit->star_partner2 > num_atoms || unit->star2 <= 0 || unit->star2 > num_atoms )
    {
        TREAT_ERR (*err, 9091, "Invalid polymeric CRU crossing bond");
        unit->closeable = CLOSING_SRU_NOT_APPLICABLE;
        return;
    }

    if ( debug_polymers )
        ITRACE_( "Star atom-partner pairs (nums base is 1) are: %-d-%-d and %-d-%-d\n",
                unit->star1, unit->star_partner1, unit->star2, unit->star_partner2);

    /* Stars are separated by one atom - that's not error but do nothing */
    if ( unit->star_partner1 == unit->star_partner2 )
    {
#ifdef ALLOW_CLOSING_SRU_VIA_DIRADICAL
        unit->closeable = CLOSING_SRU_DIRADICAL;
#else
        unit->closeable = CLOSING_SRU_NOT_APPLICABLE;
#endif
        return;
    }

    /* Stars are separated by two atoms - that's not error but do nothing */
    for (k=0; k<orig_at_data->at[unit->star_partner1-1].valence; k++)
    {
        if ( orig_at_data->at[unit->star_partner1-1].neighbor[k] == unit->star_partner2 - 1 )
        {
#ifdef ALLOW_CLOSING_SRU_VIA_HIGHER_ORDER_BOND
            unit->closeable = CLOSING_SRU_HIGHER_ORDER_BOND;
#else
            unit->closeable = CLOSING_SRU_NOT_APPLICABLE;
#endif
            return;
        }
    }

    unit->closeable = CLOSING_SRU_RING;

    return;
}


/*
    Replace original atom numbers in polymer data with canonical ones ( + 1 ).
    Then prepare:
        units2    a copy of original polymer units (p->units) with atomic numbers
                changed to curr canonical ones; atoms in alists sorted; atoms in blists
                and blists themselves sorted
        unum    numbers of units (0..p->n) as they go
                when sorted by alist's in lexicographic orders
*/
int OrigAtDataPolymer_PrepareWorkingSet( OrigAtDataPolymer *p,
                                   int *cano_nums,
                                   int *compnt_nums,
                                   OrigAtDataPolymerUnit** units2,    /* allocd by caller, to be filled */
                                   int *unum                /* allocd by caller, to be filled */
                                 )
{
    int i, k,  err = 0, cano_num1 = -1, cano_num2 = -1;
    OrigAtDataPolymerUnit *u;

    OrigAtDataPolymer_DebugTrace (p );

    /*    Replace original atom numbers in polymer data with canonical ones.
        Note that here cano nums are 'cano1', started from 1 (InChI internals 'cano' are 0-based).
        Also remove from the list atoms who mapped to cano number 0
        (i.e. -1 + 1offset): they are explicit H which have already been deleted. */


    for (k=0; k < p->n_star_atoms; k++ )
    {
        cano_num1 = cano_nums[ p->star_atoms[k] ] + 1;
        if ( cano_num1 == 0 )
        {
            /* we shouldn't arrive here */
            err = 10;
            goto exitf;
        }
        p->star_atoms[k] = cano_num1;
    }

    for (i=0; i<p->n; i++)
    {
        int na_new = -1;
        u = units2[i];

        for (k=0; k<u->na; k++)
        {
            cano_num1 = cano_nums[ u->alist[k] ] + 1;
            if ( cano_num1 == 0 )
                continue;
            u->alist[++na_new] = cano_num1;
        }
        u->na = na_new + 1;
        for (k=0; k<2*u->nb; k++)
        {
            cano_num1 = cano_nums[ u->blist[k] ] + 1;
            if ( cano_num1 == 0 )
            {
                /* one of PU crossing bond ends leads to explicit H which disappeared already */
                err = 11;
                goto exitf;
            }
            u->blist[k] = cano_num1;
        }

        cano_num1 = cano_nums[ u->star1 ] + 1;
        if ( cano_num1 == 0 )
        {    err = 11; goto exitf;    }
        u->star1 = cano_num1;

        cano_num1 = cano_nums[ u->star2 ] + 1;
        if ( cano_num1 == 0 )
        {    err = 11; goto exitf;    }
        u->star2 = cano_num1;

        cano_num1 = cano_nums[ u->star_partner1 ] + 1;
        if ( cano_num1 == 0 )
        {    err = 11; goto exitf;    }
        u->star_partner1 = cano_num1;

        cano_num1 = cano_nums[ u->star_partner2 ] + 1;
        if ( cano_num1 == 0 )
        {    err = 11; goto exitf;    }
        u->star_partner2 = cano_num1;

        for (k=0; k<u->npsbonds; k++)
        {
            cano_num1 = cano_nums[ u->psbonds[k][0] ] + 1;
            if ( cano_num1 == 0 )
                continue;
            cano_num2 = cano_nums[ u->psbonds[k][1] ] + 1;
            if ( cano_num2 == 0 )
                continue;
            u->psbonds[k][0] = inchi_min(cano_num1, cano_num2);
            u->psbonds[k][1] = inchi_max(cano_num1, cano_num2);
        }
    }

    /* Sort atoms and bonds in all units */
    for (i=0; i<p->n; i++)
    {
        int icompnt;

        u = units2[i];

        /* sort atoms (alist) */
        iisort( u->alist,  u->na );

        ITRACE_("\n*** Polymer unit %-d : ( ", i);
        for (k=0; k<u->na-1; k++)
            ITRACE_("%-d-", u->alist[k] );
        ITRACE_("%-d )\n", u->alist[u->na-1] );

        /* sort bonds (blist) */
        err = OrigAtDataPolymerUnit_OrderBondAtomsAndBondsThemselves( u, p->n_star_atoms, p->star_atoms );
        if ( err )
        {
            /* crossing bonds in blist are invalid */
            err = 12;
            goto exitf;
        }

        /* check each unit for >1 connected components */
        icompnt = compnt_nums[ u->alist[0]-1];
        for (k=1; k<u->na; k++)
        {
            if ( compnt_nums[ u->alist[k]-1 ] != icompnt )
            {
                u->disjoint = 1;
                break;
            }
        }
    }

    /* Sort all units in modified alist's lexicographic order (modificn. is: longer list always go first ) */
    for (i=0; i<p->n; i++)
        unum[i] = i;
    for (i=1; i<p->n; i++)
    {
        int tmp = unum[i];
        int j = i - 1;
        while ( j >= 0 &&    OrigAtDataPolymerUnit_CompareAtomListsMod( units2[ unum[j] ], units2[ tmp ] ) > 0  )
        /*while ( j >= 0 &&    OrigAtDataPolymerUnit_CompareAtomLists( units2[ unum[j] ], units2[ tmp ] ) > 0  )*/
        {
            unum[j+1] = unum[j];
            j--;
        }
        unum[j+1] = tmp;
    }


exitf:
    return err;
}


/*
    Helper for cyclizing CRU
    NB: end1, end1 are 0-based
*/
int  OrigAtData_RemoveHalfBond( int this_atom, int other_atom, inp_ATOM *at, int *bond_type, int *bond_stereo )
{
int k, kk;
inp_ATOM *a;
    a = &( at[this_atom] );
    for (k=0; k<a->valence; k++)
    {
        if ( a->neighbor[k] != other_atom )
            continue;

        *bond_type        =    a->bond_type[k];
        *bond_stereo    =    a->bond_stereo[k];

        a->neighbor[k]    = a->bond_type[k] = a->bond_stereo[k] = 0;

        for (kk=k+1; kk < a->valence; kk++ )
        {
            a->neighbor[kk-1]        =    a->neighbor[kk];
            a->bond_type[kk-1]        =    a->bond_type[kk];
            a->bond_stereo[kk-1]    =    a->bond_stereo[kk];
        }
        for (kk=a->valence-1; kk < MAXVAL; kk++ )
        {
            a->neighbor[kk]            = 0;
            a->bond_type[kk]        = (U_CHAR) 0;
            a->bond_stereo[kk]    = (S_CHAR) 0;
        }
        return 1;
    } /* k */

    return 0;
}


int  OrigAtData_DestroyBond( int this_atom, int other_atom,inp_ATOM *at, int *num_inp_bonds)
{
int del = 0, bond_type, bond_stereo;
    del = OrigAtData_RemoveHalfBond( this_atom, other_atom, at, &bond_type, &bond_stereo );
    del+= OrigAtData_RemoveHalfBond( other_atom, this_atom, at, &bond_type, &bond_stereo );
    if ( del == 2 )
    {
        (*num_inp_bonds)--;
        at[this_atom].valence--;
        at[this_atom].chem_bonds_valence-= bond_type;
        at[other_atom].valence--;
        at[other_atom].chem_bonds_valence-=bond_type;
        return 1;
    }

    return 0;
}


int  OrigAtData_RemoveBond( int this_atom, int other_atom, inp_ATOM *at,
                            int *bond_type, int *bond_stereo, int *num_inp_bonds)
{
int del = 0;
    del = OrigAtData_RemoveHalfBond( this_atom, other_atom, at, bond_type, bond_stereo );
    del+= OrigAtData_RemoveHalfBond( other_atom, this_atom, at, bond_type, bond_stereo );
    if ( del == 2 )
    {
        (*num_inp_bonds)--;
        at[this_atom].valence--;
        at[this_atom].chem_bonds_valence-= *bond_type;
        at[other_atom].valence--;
        at[other_atom].chem_bonds_valence-= *bond_type;
        return 1;
    }

    return 0;
}



int  OrigAtData_AddBond( int this_atom, int other_atom, inp_ATOM *at,
                         int bond_type, int bond_stereo, int *num_bonds )
{
int i, k, already_here;
inp_ATOM *a;

    if ( at[this_atom].valence  >= MAXVAL ||
         at[other_atom].valence >= MAXVAL )
         return 0;

    if ( bond_type!=INCHI_BOND_TYPE_DOUBLE && bond_type!=INCHI_BOND_TYPE_TRIPLE )
        bond_type = INCHI_BOND_TYPE_SINGLE;

    a = &(at[this_atom] );
    k = a->valence;
    already_here = 0;
    for (i=0; i<k; i++)
        if ( a->neighbor[i] == other_atom )
            { already_here = 1; break; }
    if ( !already_here )
    {
        a->neighbor[k]        =             other_atom;
        a->bond_type[k]        =    (U_CHAR) bond_type;
        a->bond_stereo[k]    =    (S_CHAR) bond_stereo;
        a->chem_bonds_valence+=             bond_type;
        a->valence++;
    }

    a = &(at[other_atom] );
    k = a->valence;
    already_here = 0;
    for (i=0; i<k; i++)
        if ( a->neighbor[i] == this_atom )
            { already_here = 1; break; }
    if ( !already_here )
    {
        a->neighbor[k]        =             this_atom;
        a->bond_type[k]        =    (U_CHAR) bond_type;
        a->bond_stereo[k]    =    (S_CHAR) bond_stereo;
        a->chem_bonds_valence+=             bond_type;
        a->valence++;
    }

    (*num_bonds)++;

    return 1;
}

int  OrigAtData_AddSingleStereolessBond( int this_atom,
                                         int other_atom,
                                         inp_ATOM *at,
                                         int *num_bonds)
{
    return OrigAtData_AddBond( this_atom, other_atom, at, INCHI_BOND_TYPE_SINGLE, 0, num_bonds );
}


/*

*/
int  OrigAtData_IncreaseBondOrder( int this_atom, int other_atom, inp_ATOM *at )
{
int i, k, n_up=0;
inp_ATOM *a;

    if ( at[this_atom].valence  >= MAXVAL ||
         at[other_atom].valence >= MAXVAL )
         return 0;

    a = &(at[this_atom] );
    if ( a->chem_bonds_valence > MAXVAL - 1)
        return 0;
    k = a->valence;
    for (i=0; i<k; i++)
    {
        if ( a->neighbor[i] != other_atom )
            continue;
        if ( a->bond_type[i] > 3 )
            return 0;
        a->bond_type[i]++;
        a->chem_bonds_valence++;
        n_up++;
        break;
    }

    a = &(at[other_atom] );
    if ( a->chem_bonds_valence > MAXVAL - 1)
        return 0;
    k = a->valence;
    for (i=0; i<k; i++)
    {
        if ( a->neighbor[i] != this_atom )
            continue;
        if ( a->bond_type[i] > 3 )
            return 0;
        a->bond_type[i]++;
        a->chem_bonds_valence++;
        n_up++;
        break;
    }

    return n_up;
}


/*

*/
int  OrigAtData_DecreaseBondOrder( int this_atom, int other_atom, inp_ATOM *at )
{
int i, k, n_dn=0;
inp_ATOM *a;


    a = &(at[this_atom] );
    if ( a->chem_bonds_valence > MAXVAL - 1)
        return 0;
    k = a->valence;
    for (i=0; i<k; i++)
    {
        if ( a->neighbor[i] != other_atom )
            continue;
        if ( a->bond_type[i] < 2 )
            return 0;
        a->bond_type[i]--;
        a->chem_bonds_valence--;
        n_dn++;
        break;
    }

    a = &(at[other_atom] );
    k = a->valence;
    for (i=0; i<k; i++)
    {
        if ( a->neighbor[i] != this_atom )
            continue;
        if ( a->bond_type[i] < 2 )
            return 0;
        a->bond_type[i]--;
        a->chem_bonds_valence--;
        n_dn++;
        break;
    }

    return n_dn;
}



/*
    OrigAtDataPolymer Collect Phase Shiftable Bonds
*/
void OrigAtDataPolymer_CollectPhaseShiftBonds( ORIG_ATOM_DATA *at_data,
                                               COMP_ATOM_DATA *composite_norm_data,
                                               int *err, char *pStrErr )
{
int i;

    ITRACE_("\n\n*** Now revealing paths between original star partners. ");

    *err = 0;

    for (i=0; i<at_data->polymer->n; i++)
    {

        if ( !at_data->polymer->units[i]->closeable )
            continue;

        ITRACE_("\n\tUnit %-d, paths between orig nums %-d and %-d. ", i+1,
                at_data->polymer->units[i]->star_partner1,
                at_data->polymer->units[i]->star_partner2);
        OrigAtDataPolymerUnit_PreselectPSBonds( at_data->polymer->units[i],
                                                at_data, err, pStrErr );
        if ( *err )
            continue;

        if ( at_data->polymer->units[i]->npsbonds < 1)
            continue;

        if ( at_data->polymer->units[i]->npsbonds == 1)
            /*
                Special case: we got only one bond between star partner 1 and star partner 2
                (result of metal disconnection)
            */
            continue;

        ITRACE_("\n\n*** Now detecting and removing intra-ring edges. " );
        OrigAtDataPolymerUnit_DelistIntraRingPSBonds( at_data->polymer->units[i],
                                                          at_data, err, pStrErr );
        if ( *err )    continue;

        ITRACE_("\n\n*** Now detecting and removing high-order bonds. " );
        OrigAtDataPolymerUnit_DelistMultiplePSBonds( at_data->polymer->units[i], at_data,
                                                         composite_norm_data, err, pStrErr );
        if ( *err )    continue;

        if ( at_data->polymer->units[i]->npsbonds == 0 )
        {
            /*    We already cyclized phase-shiftable unit and preprocessed it (in 'prep_inp_data').
                Despite that, now we discovered that there are no bonds eligible for phase shift
                (as either ring systems or alternate bonds cover all possibly useful in-unit bonds).
                We can not simply restore original connections as the structure may have been already heavily touched.
                The most viable action is to hold a single phase-shift bond (between original partners of star atoms).
                It is for sure will be converted to original bonds to star atoms on possible inchi2struct.
            */
            at_data->polymer->units[i]->closeable    = 1;
            at_data->polymer->units[i]->npsbonds        = 1;
            at_data->polymer->units[i]->psbonds[0][0]    = at_data->polymer->units[i]->star_partner1;
            at_data->polymer->units[i]->psbonds[0][1]    = at_data->polymer->units[i]->star_partner2;
        }
    }
    /*OrigAtDataPolymer_DebugTrace (at_data->polymer );*/

    return;
}


/*
    Find bonds eligible for phase shift
*/
void OrigAtDataPolymerUnit_PreselectPSBonds( OrigAtDataPolymerUnit *unit,
                                             ORIG_ATOM_DATA *at_data,
                                             int *err, char *pStrErr )
{
int start=0, end=0;
subgraf *sg=NULL;
subgraf_pathfinder *spf=NULL;

    unit->npsbonds = 0;

    sg = subgraf_new( at_data, unit->na, unit->alist, &unit->npsbonds, unit->psbonds );
    if ( !sg )
    {
        TREAT_ERR (*err, 9037, "Not enough memory (polymers)");
        unit->closeable = CLOSING_SRU_NOT_APPLICABLE;
        return;
    }

    start = sg->orig2node[ unit->star_partner1 ]; end =    sg->orig2node[ unit->star_partner2 ];
    if ( start > end )    { int tmp = end; end = start; start = tmp; }
    spf = subgraf_pathfinder_new( sg, at_data, start, end );
    if ( !spf )
    {
        TREAT_ERR (*err, 9039, "Not enough memory (polymers)");
        unit->closeable = CLOSING_SRU_NOT_APPLICABLE;
        return;
    }

    spf->seen[0] = spf->start; spf->nseen = 1;    unit->npsbonds = 0;
    subgraf_pathfinder_run( spf, &(unit->npsbonds), unit->psbonds );

    subgraf_free( sg );
    subgraf_pathfinder_free( spf );
    *err = 0;

    return;
}


/*
    Detect and throw away intra-ring bonds
*/
void OrigAtDataPolymerUnit_DelistIntraRingPSBonds( OrigAtDataPolymerUnit *unit,
                                         ORIG_ATOM_DATA *at_data,
                                         int *err, char *pStrErr )
{
    /* Establish ring systems assignments for atoms */
    int nrings = 0;
    int *num_ring_sys = NULL;

    if ( !unit )
        return;
    if ( unit->npsbonds < 1 )
        return;

    *err =  1;
    num_ring_sys = (int *) calloc( at_data->num_inp_atoms + 1, sizeof(int) );
    if ( !num_ring_sys )
        goto exitf;
    *err = 0;

    nrings = OrigAtData_FindRingSystems( at_data->polymer, at_data->at,
                                         at_data->num_inp_atoms, &at_data->num_inp_bonds,
                                         num_ring_sys, NULL,
                                         unit->star_partner1 - 1 /* NB: start dfs within connected compt! */
                                         );

    if ( nrings == 0 )
        goto exitf;
    else
    {
        int at1, at2, j=0;
repeatj:
        at1 = unit->psbonds[j][0];
        at2 = unit->psbonds[j][1];
        /* ITRACE_("\n\tat1=%-d  at2=%-d  num_ring_sys[at1]=%-d  num_ring_sys[at2]=%-d  ", at1, at2, num_ring_sys[at1], num_ring_sys[at2] ); */
        if ( ( num_ring_sys[at1] ==  num_ring_sys[at2] ) &&
             ( num_ring_sys[at1] !=  -1 ) )
        {
            ITRACE_("\n\tThrowing away intra-ring bond (%-d, %-d)  ", at1, at2 );
            throw_away_inappropriate_bond( at1, at2, &unit->npsbonds, unit->psbonds );
        }
        else
            ++j;
        if ( j < unit->npsbonds )
            goto repeatj;
    }


exitf:
    if ( num_ring_sys )
        inchi_free( num_ring_sys );

    return;
}


/*
    Find ring systems accounting for possible cuclizing bonds in polymer SRU's
*/
int  OrigAtData_FindRingSystems( OrigAtDataPolymer *pd, inp_ATOM *at, int nat, int *num_inp_bonds,
                                 int *num_ring_sys, int *size_ring_sys, int start )
{
int i, j, nrings = 0, bond_type, bond_stereo;

    if ( NULL==num_ring_sys )
        return 0;

    /* remove polymer SRU 'cyclizing' bonds */
    for (j=0; j<pd->n; j++)
        if ( pd->units[j]->already_closed    )
            OrigAtData_RemoveBond( pd->units[j]->star_partner1 - 1,
                                   pd->units[j]->star_partner2 - 1,
                                   at,
                                   &bond_type, &bond_stereo, num_inp_bonds );


    MarkRingSystemsInp( at, nat, start); /*0 );*/

    for (i=0; i<=nat; i++)
        num_ring_sys[i] = -1;
    for (i=0; i<nat; i++)
    {
        /*int rsize = at[i].nNumAtInRingSystem > 2;*/
        if ( at[i].nNumAtInRingSystem > 2 )
        {
            int atnum = at[i].orig_at_number;
            num_ring_sys[ atnum ] = at[i].nRingSystem;
            if ( NULL!= size_ring_sys )
                size_ring_sys[atnum] = at[i].nNumAtInRingSystem;
        }
    }

    UnMarkRingSystemsInp( at, nat );

    ITRACE_("\nRing system numbers for in-ring atoms (atom@ringsystem; original atom numbering) : ");
    for (i=0; i<nat; i++)
    {
        if ( num_ring_sys[i] > -1 )
        {
            ITRACE_("%-d@%-d  ", at[i].orig_at_number, num_ring_sys[i] );
            nrings++;
        }
    }
    if ( !nrings )
        ITRACE_("None");
    ITRACE_("\n");

    /* restore polymer SRU 'cyclizing' bonds */
    for (j=0; j<pd->n; j++)
        if ( pd->units[j]->already_closed    )
            OrigAtData_AddSingleStereolessBond( pd->units[j]->star_partner1 - 1,
                                pd->units[j]->star_partner2 - 1,
                                at, num_inp_bonds );

    return nrings;
}


/*
    OrigAtData FillAtProps (for polymer SRU analysis)
*/
void OrigAtData_FillAtProps( OrigAtDataPolymer *pd, inp_ATOM *at,
                             int nat, int *num_inp_bonds,
                             OrigAtDataPolymerAtomProps *aprops )
{

/*
    Max rank for in-ring atom is 216 which is achieved for N (element number 7 in Periodic system & erank_rule2[] ),
    then goes O with rank 215 (element number 8), and so on... lowest rank is 1 for H .

    This follows to IUPAC rule 2 [Pure Appl. Chem., Vol. 74, No. 10, 2002, p. 1926] which states:
    a. a ring or ring system containing nitrogen;
    b. a ring or ring system containing the heteroatom occurring earliest in the order given in Rule 4;
        ( which is     O > S > Se > Te > N > P > As > Sb > Bi > Si > Ge > Sn > Pb > B > Hg )
    ...

*/
int erank_rule2[] = { 0,1,198,197,196,202,2,216,215,191,190,189,188,187,206,210,214,183,182,181,180,179,178,177,176,
                     175,174,173,172,171,170,169,205,209,213,165,164,163,162,161,160,159,158,157,156,155,154,153,152,
                     151,204,208,212,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,
                     127,126,125,124,123,122,121,201,119,203,207,116,115,114,113,112,111,110,109,108,107,106,105,104,
                     103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81};



/*
    Max rank for chain atom is 215 which is achieved for O (element number 8 in Periodic system & erank_rule4[] ),
    then goes N with rank 212 (element number 8), and so on... lowest rank is 1 for H .

    This follows to IUPAC rule 4 [Pure Appl. Chem., Vol. 74, No. 10, 2002, p. 1927] which states:
    O > S > Se > Te > N > P > As > Sb > Bi > Si > Ge > Sn > Pb > B > Hg
    Note: Other heteroatoms may be placed within this order as indicated by their positions in the
    periodic table [5].
*/
int erank_rule4[] = { 0,1,198,197,196,202,2,211,215,191,190,189,188,187,206,210,214,183,182,181,180,179,178,177,176,
                    175,174,173,172,171,170,169,205,209,213,165,164,163,162,161,160,159,158,157,156,155,154,153,152,
                     151,204,208,212,147,146,145,144,143,142,141,140,139,138,137,136,135,134,133,132,131,130,129,128,
                     127,126,125,124,123,122,121,201,119,203,207,116,115,114,113,112,111,110,109,108,107,106,105,104,
                     103,102,101,100,99,98,97,96,95,94,93,92,91,90,89,88,87,86,85,84,83,82,81};


int i, j, k, nrings = 0;
int a1, a2, dummy = 0, bond_type, bond_stereo;
int *num_ring_sys = NULL,
        *size_ring_sys = NULL;

    if ( NULL == aprops )
        return;

    /* Establish element ranks for atoms */
    for (k=0; k<nat; k++)
    {
        aprops[k].erank            =    erank_rule4[ at[k].el_number ];
        aprops[k].ring_erank    =    0;
        aprops[k].ring_size        =    0;
        aprops[k].ring_num        =  -1;
    }

    /* Establish ring systems assignments for atoms */
    num_ring_sys = (int *) calloc( nat + 1, sizeof(int) );
    if ( NULL == num_ring_sys )
        goto exitf;
    size_ring_sys = (int *) calloc( nat + 1, sizeof(int) );
    if ( NULL == size_ring_sys )
        goto exitf;

    /*
        Note that we get here on the way of InChI2Struct conversion.

        Break temporarily any of (actually, the first)
        SRU 'cyclising' bonds
    */
     for (j=0; j<pd->n; j++)
     {
         if ( pd->units[j]->na > 2                        &&
              pd->units[j]->npsbonds > 0                &&
              pd->units[j]->already_closed ==0            &&
              pd->units[j]->closeable == CLOSING_SRU_RING )
         {
             a1 = pd->units[j]->psbonds[0][0] - 1;
             a2 = pd->units[j]->psbonds[0][1] - 1;
             OrigAtData_RemoveBond( a1, a2, at, &bond_type, &bond_stereo, &dummy );
         }
     }


    nrings = OrigAtData_FindRingSystems( pd, at, nat, num_inp_bonds,
                                         num_ring_sys, size_ring_sys, 0 );


    /*    Immediately restore just broken bond(s) */
     for (j=0; j<pd->n; j++)
     {
         if ( pd->units[j]->na > 2                        &&
              pd->units[j]->npsbonds > 0                &&
              pd->units[j]->already_closed ==0            &&
              pd->units[j]->closeable == CLOSING_SRU_RING )
         {
             a1 = pd->units[j]->psbonds[0][0] - 1;
             a2 = pd->units[j]->psbonds[0][1] - 1;
             /*OrigAtData_AddSingleStereolessBond( a1, a2, at, &dummy ); */
             OrigAtData_AddBond( a1, a2, at, bond_type, bond_stereo, &dummy );
         }
     }


    if ( nrings )
    {
        int max_ring_num = 0;
        /* SRU contains ring[s], proceed with them loosely following the IUPAC guidelines */
        for (k=0; k<nat; k++)
        {
            int atnum = at[k].orig_at_number;
            if ( num_ring_sys[atnum] >= 0 )
            {
                aprops[k].ring_num    = num_ring_sys[atnum];    /* temporarily */
                if ( max_ring_num < aprops[k].ring_num )
                    max_ring_num = aprops[k].ring_num;        /*    NB: OrigAtData_FindRingSystems may return num_ring_sys[]
                                                                which is not a list of consecutive numbers                */
                aprops[k].ring_size    = size_ring_sys[atnum]; /*    Size of ring system which includes the atom k .
                                                                    It is used as an additional score for in-ring
                                                                    atoms' prioritizing (instead of criteria in
                                                                    2c-2h of IUPAC rule 2 which deal with ring sizes).
                                                                 */
            }
        }

        /*for (k=0; k<nat; k++)
            ITRACE_("Source:  {%-s%-d ring num %-d, size %-d}\n", at[k].elname, k, aprops[k].ring_num, aprops[k].ring_size );*/

        for (i=0; i<=max_ring_num; i++)
        {
            int erank, max_erank = 0;
            for (k=0; k<nat; k++)
            {
                if ( aprops[k].ring_num == i )
                {
                    erank = erank_rule2[ at[k].el_number ];
                    if ( erank > max_erank )
                        max_erank = erank;
                }
            }
            for (k=0; k<nat; k++)
                if ( aprops[k].ring_num == i )
                    if ( aprops[k].ring_size > 2 )
                        aprops[k].ring_erank = max_erank;
        }
    }

exitf:
    if ( num_ring_sys )
        free( num_ring_sys );
    if ( size_ring_sys )
        free( size_ring_sys );

    return;
}



/*
    Detecting and removing high-order bonds
*/
void OrigAtDataPolymerUnit_DelistMultiplePSBonds( OrigAtDataPolymerUnit *unit,
                                        ORIG_ATOM_DATA *orig_at_data,
                                        COMP_ATOM_DATA *composite_norm_data,
                                        int *err, char *pStrErr )
{
int at1, at2, border, j=0, k, check_taut = 0, remove;

    int *orig_num = NULL, *curr_num = NULL;

    if ( unit->na < 2 )
        return;
    if ( unit->nb < 2 )
        return;
    if ( unit->npsbonds < 1 )
        return;

    /* Take care on the tautomeric bonds */
    if ( composite_norm_data )
    {
        check_taut    = 1;
        orig_num    = (int *) inchi_calloc( orig_at_data->num_inp_atoms + 2, sizeof(int)  );
        curr_num    = (int *) inchi_calloc( orig_at_data->num_inp_atoms + 2, sizeof(int)  );
        if ( orig_num && curr_num )
        {
            check_taut    = 1;
            CompAtomData_GetNumMapping( composite_norm_data, orig_num, curr_num );
        }
    }


repeatj:
    remove = 0;
    at1 = unit->psbonds[j][0];
    at2 = unit->psbonds[j][1];
    border = 0;
    for (k=0; k<orig_at_data->at[at1-1].valence; k++ )
    {
        if ( orig_at_data->at[at1-1].neighbor[k] != at2 - 1 )
            continue;
        border = orig_at_data->at[at1-1].bond_type[k];
    }
    /*if ( border > 1 )        */
    {
        int bond_is_untouchable = 0, btype;
        if ( check_taut && composite_norm_data && composite_norm_data->at )
        {
            for (k=0; k<composite_norm_data->at[ curr_num[at1] ].valence; k++ )
            {
                if ( composite_norm_data->at[ curr_num[at1] ].neighbor[k] != curr_num[at2] )
                    continue;
                btype = composite_norm_data->at[ curr_num[at1] ].bond_type[k];
                bond_is_untouchable = ( btype == BOND_TAUTOM ); /*|| btype == BOND_ALTERN );*/
                break;
            }
        }
        if ( bond_is_untouchable )
            remove = 1;
    }
    if ( remove )
    {
        ITRACE_("\n\tThrowing away bond (%-d, %-d) of order %-d ", at1, at2, border );
        throw_away_inappropriate_bond( at1, at2, &unit->npsbonds, unit->psbonds );
    }
    else
        ++j;
    if ( j < unit->npsbonds )
        goto repeatj;

    if ( orig_num )
        inchi_free( orig_num );
    if ( curr_num )
        inchi_free( curr_num );

    return;
}



/* Remove bond (at1, at2) */
void throw_away_inappropriate_bond( int at1, int at2, int *nbonds, int **bonds)
{
int p, q;
    if ( at1 > at2 )
    {
        int tmp = at1;
        at1 = at2;
        at2 = tmp;
    }
    for (p=0; p<*nbonds; p++)
    {
        if ( bonds[p][0]==at1 && bonds[p][1]==at2 )
        {
            for (q=p+1; q<*nbonds; q++)
            {
                bonds[q-1][0] = bonds[q][0];
                bonds[q-1][1] = bonds[q][1];
            }
            (*nbonds)--;
            break;
        }
    }
    return;
}


void OrigAtDataPolymerUnit_DebugTrace( OrigAtDataPolymerUnit *u )
{
int i, k;

    int na, nb;
    char *conn = "ABSENT", *typ="ABSENT", *styp="ABSENT";

    if ( !u )
        return;
    if        ( u->conn==1 )        conn = "HT";
    else if ( u->conn==2 )        conn = "HH";
    else if ( u->conn==3 )        conn = "EU";

    if        ( u->type==0 )        typ = "NONE";
    else if ( u->type==1 )        typ = "SRU";
    else if ( u->type==2 )        typ = "MON";
    else if ( u->type==3 )        typ = "COP";
    else if ( u->type==4 )        typ = "MOD";
    else if ( u->type==5 )        typ = "MER";

    if        ( u->subtype==1 )    styp = "ALT";
    else if ( u->subtype==2 )    styp = "RAN";
    else if ( u->subtype==3 )    styp = "BLK";

    ITRACE_( "\n\tid=%-d   label=%-d   type=%-s   subtype=%-s   conn=%-s   subscr='%-s'\n",
            u->id, u->label, typ, styp, conn, u->smt );
    ITRACE_( "\tBracket1 coords: %-f, %-f, %-f, %-f\n", u->xbr1[0], u->xbr1[1], u->xbr1[2], u->xbr1[3] );
    ITRACE_( "\tBracket2 coords: %-f, %-f, %-f, %-f\n", u->xbr2[0], u->xbr2[1], u->xbr2[2], u->xbr2[3] );
    na = u->na;
    ITRACE_( "\t%-d atoms { ", na);
    for (k=0;k<na-1; k++)
        ITRACE_( " %-d, ", u->alist[k]);
    ITRACE_( " %-d }\n", u->alist[na-1]);
    nb = u->nb;
    ITRACE_( "\t%-d bonds { ", nb);
    for (k=0; k<nb; k++)
        ITRACE_( " %-d-%-d ", u->blist[2*k], u->blist[2*k+1] );
    ITRACE_( "}\n" );

    ITRACE_("\tPhase-shift bonds candidates (may include cyclizing one) : %-d  ", u->npsbonds );
    if ( u->npsbonds )
    {
        for(i=0; i<u->npsbonds; i++)
        {
            ITRACE_("(%-d, %-d)  ", u->psbonds[i][0], u->psbonds[i][1] );
        }
    }
    return;
}


void OrigAtDataPolymer_DebugTrace( OrigAtDataPolymer *p )
{
int i;
    ITRACE_( "\n\n* POLYMER INFO @ %-p (%-d group(s))", p , p->n);
    ITRACE_( "\n\n* %-d star atoms: ", p->n_star_atoms );
    for (i=0; i<p->n_star_atoms; i++)
        ITRACE_( " %-d", p->star_atoms[i] );

    for (i=0; i<p->n; i++)
    {
        ITRACE_( "\n* Polymer unit %-d", i );
        OrigAtDataPolymerUnit_DebugTrace( p->units[i] );
    }
    ITRACE_( "\n* Really-do-PS = %-d", p->really_do_phase_shift );
    ITRACE_( "\n* End POLYMER INFO\n" );
    return;
}


int  OrigAtDataPolymer_GetRepresentation( OrigAtDataPolymer *p )
{
int i, nsrc=0, nstruct=0;

    if ( !p )
        return NO_POLYMER;

    for (i=0; i<p->n; i++)
    {
        if ( p->units[i]->nb==2 || p->units[i]->npsbonds>0 || ((p->units[i]->star1 > 0)&&(p->units[i]->star2 > 0)) )
        {
            p->units[i]->representation = POLYMER_REPRESENTATION_STRUCTURE_BASED;
            nstruct++;
        }
        else if ( p->units[i]->nb == 0 )
        {
            p->units[i]->representation = POLYMER_REPRESENTATION_SOURCE_BASED;
            nsrc++;
        }
    }
    if ( p->n == nsrc )
        return POLYMER_REPRESENTATION_SOURCE_BASED;
    else if ( p->n == nstruct )
        return POLYMER_REPRESENTATION_STRUCTURE_BASED;
    else if ( p->n == ( nsrc + nstruct ) )
    {
        /*
            Structure based presentation may include no-crossing bond units
            which only serve as embedding for (>1) structure-based SRU's
            Account for this in code below.
        */
        if ( nsrc < nstruct )
        {
            int j, atom, atom_is_shared_with_struct_based_unit=0;
            for (i=0; i<p->n; i++)
            {
                int k;
                if ( p->units[i]->representation != POLYMER_REPRESENTATION_SOURCE_BASED )
                    continue;
                for (k=0; k<p->units[i]->na; k++ )
                {
                    atom = p->units[i]->alist[k];
                    if ( is_in_the_ilist( p->star_atoms, atom, p->n_star_atoms ) )
                        continue;
                    atom_is_shared_with_struct_based_unit = 0;
                    for (j=0; j<p->n; j++)
                    {
                        if ( p->units[j]->representation != POLYMER_REPRESENTATION_STRUCTURE_BASED )
                            continue;
                        if ( is_in_the_ilist( p->units[j]->alist, atom, p->units[j]->na ) )
                        {
                            atom_is_shared_with_struct_based_unit = 1;
                            break;
                        }
                    }
                    if ( !atom_is_shared_with_struct_based_unit )
                        break;
                }
                if ( !atom_is_shared_with_struct_based_unit )
                    break;
            }
            if ( atom_is_shared_with_struct_based_unit )
                return POLYMER_REPRESENTATION_STRUCTURE_BASED;
        }
        return POLYMER_REPRESENTATION_MIXED;
    }

    return POLYMER_REPRESENTATION_UNRECOGNIZED;
}
