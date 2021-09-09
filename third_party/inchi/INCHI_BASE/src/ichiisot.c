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


#include "mode.h"
#include "ichicomn.h"

/**********************************************************************************/
AT_ISO_SORT_KEY make_iso_sort_key( int iso_atw_diff, int num_1H, int num_2H, int num_3H )
{
    AT_ISO_SORT_KEY iso_sort_key = 0, mult = 1;

    iso_sort_key += mult * num_1H;
    mult *= AT_ISO_SORT_KEY_MULT;
    iso_sort_key += mult * num_2H;
    mult *= AT_ISO_SORT_KEY_MULT;
    iso_sort_key += mult * num_3H;
    mult *= AT_ISO_SORT_KEY_MULT;
    iso_sort_key += mult * iso_atw_diff;

    return iso_sort_key;
}



/****************************************************************************
 Set sp_ATOM isotopic sort keys
****************************************************************************/
int set_atom_iso_sort_keys( int num_at,
                            sp_ATOM *at,
                            T_GROUP_INFO* t_group_info,
                            int *bHasIsotopicInTautomerGroups )
{
    int             i, num_isotopic = 0, bMergedTgroup;
    AT_ISO_SORT_KEY iso_sort_key;
    T_GROUP        *t_group =
        ( t_group_info &&
         t_group_info->t_group &&
         t_group_info->num_t_groups > 0 ) ? t_group_info->t_group : NULL;

    if (bHasIsotopicInTautomerGroups)
    {
        *bHasIsotopicInTautomerGroups = 0;
    }
    for (i = 0; i < num_at; i++)
    {
        bMergedTgroup = ( t_group_info && t_group_info->nIsotopicEndpointAtomNumber && ( at[i].cFlags & AT_FLAG_ISO_H_POINT ) );
        if (( !at[i].endpoint || !t_group ) && !bMergedTgroup)
        {
            iso_sort_key = make_iso_sort_key( at[i].iso_atw_diff, at[i].num_iso_H[0], at[i].num_iso_H[1], at[i].num_iso_H[2] );
        }
        else
        {
             /*  H isotopes go to the tautomer part of the CT (name) */
             /*  if (at[i].endpoint && t_group) ... */
            iso_sort_key = make_iso_sort_key( at[i].iso_atw_diff, 0, 0, 0 );
            if (bHasIsotopicInTautomerGroups)
            {
                *bHasIsotopicInTautomerGroups += ( at[i].num_iso_H[0] || at[i].num_iso_H[1] || at[i].num_iso_H[2] || bMergedTgroup );
            }
        }
        at[i].iso_sort_key = iso_sort_key;
        num_isotopic += ( iso_sort_key != 0 );
    }

    return num_isotopic;
}


#ifdef NEVER


/****************************************************************************/
int unpack_iso_sort_key( AT_ISO_SORT_KEY iso_sort_key,
                         S_CHAR *num_1H,
                         S_CHAR *num_2H,
                         S_CHAR *num_3H,
                         S_CHAR *iso_atw_diff )
{
    int is_negative;
    AT_ISO_SORT_KEY HOnlyAtwPart;
    static const AT_ISO_SORT_KEY MultAtwDiff = AT_ISO_SORT_KEY_MULT*AT_ISO_SORT_KEY_MULT*AT_ISO_SORT_KEY_MULT;
    if (!iso_sort_key)
    {
        *num_1H = *num_2H = *num_3H = *iso_atw_diff = 0;
        return 0;
    }
    else
    {
        if (iso_sort_key < 0)
        {
            is_negative = 1;
            iso_sort_key = -iso_sort_key;
            HOnlyAtwPart = MultAtwDiff - iso_sort_key % MultAtwDiff;
            iso_sort_key += HOnlyAtwPart;
        }
        else
        {
            is_negative = 0;
            HOnlyAtwPart = iso_sort_key % MultAtwDiff;
            iso_sort_key -= HOnlyAtwPart;
        }
    }

    iso_sort_key /= MultAtwDiff;

    *num_1H = (S_CHAR) ( HOnlyAtwPart % AT_ISO_SORT_KEY_MULT );
    HOnlyAtwPart /= AT_ISO_SORT_KEY_MULT;
    *num_2H = (S_CHAR) ( HOnlyAtwPart % AT_ISO_SORT_KEY_MULT );
    HOnlyAtwPart /= AT_ISO_SORT_KEY_MULT;
    *num_3H = (S_CHAR) ( HOnlyAtwPart % AT_ISO_SORT_KEY_MULT );

    *iso_atw_diff = (S_CHAR) ( is_negative ? -iso_sort_key : iso_sort_key );

    return 1;
}
#endif
