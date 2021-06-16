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


#include <string.h>

#include "mode.h"

#include "ichimain.h"
#include "inchi_gui.h"




/****************************************************************************/


#ifndef COMPILE_ANSI_ONLY


#ifndef TARGET_LIB_FOR_WINCHI


 /****************************************************************************/
int DisplayStructure( struct tagCANON_GLOBALS   *pCG,
                      inp_ATOM      *at,
                      int           num_at,
                      OAD_Polymer   *polymer,
                      int           num_removed_H,
                      int           bAdd_DT_to_num_H,
                      int           nNumRemovedProtons,
                      NUM_H         *nNumRemovedProtonsIsotopic,
                      int           bIsotopic,
                      int           j /*bTautomeric*/,
                      INChI         **cur_INChI,
                      INChI_Aux     **cur_INChI_Aux,
                      int           bAbcNumbers,
                      DRAW_PARMS    *dp,
                      INCHI_MODE    nMode,
                      char          *szTitle )
{
    INF_ATOM_DATA inf_data = { NULL, };
    int err = -1;

    if (CreateInfoAtomData( &inf_data, num_at, 1 ))
    {
        err = 0;

        FillOutInfAtom( pCG, at, &inf_data, num_at, num_removed_H, bAdd_DT_to_num_H,
                        nNumRemovedProtons, nNumRemovedProtonsIsotopic, bIsotopic,
                        cur_INChI ? cur_INChI[j] : NULL,
                        cur_INChI_Aux ? cur_INChI_Aux[j] : NULL,
                        bAbcNumbers, nMode );

        FillTableParms( &dp->sdp, cur_INChI, cur_INChI_Aux, nMode, bIsotopic, j );

        err = DisplayInputStructure( szTitle, at, &inf_data, num_at, dp );

        FreeInfoAtomData( &inf_data );
    }

    return err;
}



/****************************************************************************/
int DisplayCompositeStructure( struct tagCANON_GLOBALS *pCG,
                               COMP_ATOM_DATA   *composite_norm_data,
                               OAD_Polymer		*polymer,
                               int              bIsotopic,
                               int              bTautomeric,
                               PINChI2          *pINChI2,
                               PINChI_Aux2      *pINChI_Aux2,
                               int              bAbcNumbers,
                               DRAW_PARMS       *dp,
                               INCHI_MODE       nMode,
                               char             *szTitle )
{
    INF_ATOM_DATA inf_data;
    int err = -1, ret;

    memset( &inf_data, 0, sizeof( inf_data ) );

    if (CreateInfoAtomData( &inf_data, ( composite_norm_data + bTautomeric )->num_at,
        ( composite_norm_data + bTautomeric )->num_components ))
    {

        ret = FillOutCompositeCanonInfAtom( pCG, composite_norm_data,
                                            &inf_data, bIsotopic, bTautomeric,
                                            pINChI2, pINChI_Aux2,
                                            bAbcNumbers, nMode );
        if (!ret)
        {
            goto exit_function;
        }

        if (bTautomeric == TAUT_INI)
        {
            /*
            FillOutInfAtom( (composite_norm_data+bTautomeric)->at, &inf_data, (composite_norm_data+bTautomeric)->num_at,
                            (composite_norm_data+bTautomeric)->num_removed_H, bAdd_DT_to_num_H,
                            (composite_norm_data+bTautomeric)->nNumRemovedProtons,
                            (composite_norm_data+bTautomeric)->nNumRemovedProtonsIsotopic, bIsotopic,
                            NULL, NULL, bAbcNumbers, nMode);
            */
            ;
        }
        else
        {
            /* real check for tautomeric components 02-04-2005 */
            int m, nNumTautComponents = 0;

            if (1 == bTautomeric)
            {
                for (m = 0; m < composite_norm_data[TAUT_YES].num_components; m++)
                {
                    if (!pINChI2[m][TAUT_YES])
                    {
                        continue;
                    }
                    if (pINChI2[m][TAUT_YES]->bDeleted || pINChI2[m][TAUT_YES]->lenTautomer > 0)
                    {
                        nNumTautComponents++;
                    }
                }
            }

            FillCompositeTableParms( &dp->sdp, inf_data.StereoFlags, nMode, bIsotopic, nNumTautComponents );
        }

        err = DisplayInputStructure( szTitle, ( composite_norm_data + bTautomeric )->at, &inf_data, ( composite_norm_data + bTautomeric )->num_at, dp );

        FreeInfoAtomData( &inf_data );
    }

exit_function:

    return err;
}

#endif


/****************************************************************************/
void FillTableParms( SET_DRAW_PARMS *sdp,
                     INChI          **cur_INChI,
                     INChI_Aux      **cur_INChI_Aux,
                     INCHI_MODE     nMode,
                     int            bShowIsotopic,
                     int            indx )
{
    TBL_DRAW_PARMS *tdp = sdp->tdp;
    char( *ReqShownFound )[TDP_NUM_PAR] = tdp->ReqShownFound;
    int  i, j;
    INChI_Stereo *Stereo;
    int          bShowTaut = ( cur_INChI && cur_INChI[indx]->lenTautomer > 0 ) ? 1 : 0;

#if ( REL_RAC_STEREO_IGN_1_SC == 1 )
    int bRelRac = 0 != ( nMode &
        ( REQ_MODE_RELATIVE_STEREO | REQ_MODE_RACEMIC_STEREO ) );
#endif

    if (!cur_INChI || !cur_INChI_Aux)
    {
        sdp->tdp->bDrawTbl = 0;
        sdp->bOrigAtom = 1;
        return;
    }

    /*  Displayed */

    ReqShownFound[ilSHOWN][itBASIC] = bShowTaut ? 'T' : '\0';
    ReqShownFound[ilSHOWN][itISOTOPIC] = bShowIsotopic ? 'I' : '\0';

    /*
    ReqShownFound[ilSHOWN][itBASIC]    =  bShowTaut?     'T':'B';
    ReqShownFound[ilSHOWN][itISOTOPIC] =  bShowIsotopic? 'I':'N';
    */

    i = indx;
    if (cur_INChI[i])
    {
        Stereo = bShowIsotopic ? cur_INChI[i]->StereoIsotopic
            : cur_INChI[i]->Stereo;
    }
    else
    {
        Stereo = NULL;
    }

#if ( REL_RAC_STEREO_IGN_1_SC == 1 )

    if (Stereo && ( 0 < Stereo->nNumberOfStereoBonds ||
        0 < Stereo->nNumberOfStereoCenters - bRelRac ))
    {
        ReqShownFound[ilSHOWN][itSTEREO] = 'S';
        if (Stereo->nNumberOfStereoCenters && Stereo->nCompInv2Abs == -1 &&
            ( nMode & ( REQ_MODE_RELATIVE_STEREO | REQ_MODE_RACEMIC_STEREO ) ))
        {
            if (Stereo->nNumberOfStereoCenters < 2 &&
                !Stereo->nNumberOfStereoBonds)
            {
                ReqShownFound[ilSHOWN][itSTEREO] = '\0';
            }
            else if (Stereo->nNumberOfStereoCenters >= 2)
            {
                /* shown Inverted stereo */
                ReqShownFound[ilSHOWN][itSTEREO] = 's';
            }
        }

#else  /* REL_RAC_STEREO_IGN_1_SC == 0 */

    if (Stereo &&
        ( Stereo->nNumberOfStereoBonds || Stereo->nNumberOfStereoCenters ))
    {

        ReqShownFound[ilSHOWN][itSTEREO] = 'S';

        if (Stereo->nNumberOfStereoCenters && Stereo->nCompInv2Abs == -1 &&
            ( nMode & ( REQ_MODE_RELATIVE_STEREO | REQ_MODE_RACEMIC_STEREO ) ))
        {
            /*
            if ( Stereo->nNumberOfStereoCenters < 2 && !Stereo->nNumberOfStereoBonds )
            {
                ReqShownFound[ilSHOWN][itSTEREO] = '\0';
            } else
            if ( Stereo->nNumberOfStereoCenters >= 2 )
            {
            */

            /* shown Inverted stereo */
            ReqShownFound[ilSHOWN][itSTEREO] = 's';

            /*
            }
            */
        }
#endif /* REL_RAC_STEREO_IGN_1_SC */
    }
    else
    {
        ReqShownFound[ilSHOWN][itSTEREO] = '\0';
    }

    /*
    ReqShownFound[ilSHOWN][itSTEREO]   =
        (bShowIsotopic?  (cur_INChI[i] && cur_INChI[i]->StereoIsotopic &&
                         (cur_INChI[i]->StereoIsotopic->nNumberOfStereoBonds ||
                          cur_INChI[i]->StereoIsotopic->nNumberOfStereoCenters))
                        :
                         (cur_INChI[i] && cur_INChI[i]->Stereo &&
                         (cur_INChI[i]->Stereo->nNumberOfStereoBonds ||
                          cur_INChI[i]->Stereo->nNumberOfStereoCenters) )
        ) ? 'S':'\0';
    */

    /* Remove zeroes between chars */
    for (i = j = 0; i < TDP_NUM_PAR; i++)
    {
        if (ReqShownFound[ilSHOWN][i] >= ' ')
        {
            ReqShownFound[ilSHOWN][j++] = ReqShownFound[ilSHOWN][i];
        }
    }

    i = j;

    for (; i < TDP_NUM_PAR; i++)
    {
        ReqShownFound[ilSHOWN][i] = '\0';
    }

    sdp->tdp->bDrawTbl = j ? 1 : 0;
    sdp->bOrigAtom = 0;

    return;
}


/****************************************************************************/
void FillCompositeTableParms( SET_DRAW_PARMS    *sdp,
                              AT_NUMB           StereoFlags,
                              INCHI_MODE        nMode,
                              int               bShowIsotopic,
                              int               bShowTaut )
{
    TBL_DRAW_PARMS *tdp = sdp->tdp;
    char( *ReqShownFound )[TDP_NUM_PAR] = tdp->ReqShownFound;
    int  i, j;

        /*  Displayed */

    ReqShownFound[ilSHOWN][itBASIC] = bShowTaut ? 'T' : '\0';
    ReqShownFound[ilSHOWN][itISOTOPIC] = bShowIsotopic ? 'I' : '\0';

    /*
    ReqShownFound[ilSHOWN][itBASIC]    =  bShowTaut?     'T':'B';
    ReqShownFound[ilSHOWN][itISOTOPIC] =  bShowIsotopic? 'I':'N';
     */

    if (StereoFlags & INF_STEREO)
    {
        ReqShownFound[ilSHOWN][itSTEREO] = 'S';
        if (( StereoFlags & INF_STEREO_INV ) &&
            ( nMode & ( REQ_MODE_RELATIVE_STEREO | REQ_MODE_RACEMIC_STEREO ) ))
        {
            if (StereoFlags & ( INF_STEREO_REL | INF_STEREO_RAC ))
            {
                ReqShownFound[ilSHOWN][itSTEREO] = 's';
            }
            else
            {
                ReqShownFound[ilSHOWN][itSTEREO] = '\0'; /* shown Inverted stereo */
            }
        }
    }
    else
    {
        ReqShownFound[ilSHOWN][itSTEREO] = '\0';
    }

    /*
    ReqShownFound[ilSHOWN][itSTEREO]   =
        (bShowIsotopic? (cur_INChI[i] && cur_INChI[i]->StereoIsotopic &&
                         (cur_INChI[i]->StereoIsotopic->nNumberOfStereoBonds ||
                          cur_INChI[i]->StereoIsotopic->nNumberOfStereoCenters) )
                        :
                        (cur_INChI[i] && cur_INChI[i]->Stereo &&
                         (cur_INChI[i]->Stereo->nNumberOfStereoBonds ||
                          cur_INChI[i]->Stereo->nNumberOfStereoCenters) )
        ) ? 'S':'\0';
    */

    /* Remove zeroes between chars */

    for (i = j = 0; i < TDP_NUM_PAR; i++)
    {
        if (ReqShownFound[ilSHOWN][i] >= ' ')
        {
            ReqShownFound[ilSHOWN][j++] = ReqShownFound[ilSHOWN][i];
        }
    }

    i = j;

    for (; i < TDP_NUM_PAR; i++)
    {
        ReqShownFound[ilSHOWN][i] = '\0';
    }

    sdp->tdp->bDrawTbl = j ? 1 : 0;
    sdp->bOrigAtom = 0;

    return;
}

#endif
