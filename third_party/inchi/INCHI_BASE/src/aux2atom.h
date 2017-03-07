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


#ifndef _AUX2ATOM_H_
#define _AUX2ATOM_H_


#ifdef TARGET_API_LIB


/*************************************************************************/
S_SHORT *is_in_the_slist( S_SHORT *pathAtom, S_SHORT nNextAtom, int nPathLen )
{
    for ( ; nPathLen && *pathAtom != nNextAtom; nPathLen--,  pathAtom++ )
        ;
    return nPathLen? pathAtom : NULL;
}
/************************************************/
int is_element_a_metal( char szEl[] )
{
    static const char szMetals[] = "K;V;Y;W;U;"
        "Li;Be;Na;Mg;Al;Ca;Sc;Ti;Cr;Mn;Fe;Co;Ni;Cu;Zn;Ga;Rb;Sr;Zr;"
        "Nb;Mo;Tc;Ru;Rh;Pd;Ag;Cd;In;Sn;Sb;Cs;Ba;La;Ce;Pr;Nd;Pm;Sm;"
        "Eu;Gd;Tb;Dy;Ho;Er;Tm;Yb;Lu;Hf;Ta;Re;Os;Ir;Pt;Au;Hg;Tl;Pb;"
        "Bi;Po;Fr;Ra;Ac;Th;Pa;Np;Pu;Am;Cm;Bk;Cf;Es;Fm;Md;No;Lr;Rf;";
    const int len = strlen(szEl);
    const char *p;

    if ( 0 < len && len <= 2 &&
         isalpha( UCINT szEl[0] ) && isupper( szEl[0] ) &&
         (p = strstr(szMetals, szEl) ) && p[len] == ';' )
    {
            return 1; /*return AtType_Metal;*/
    }
    return 0;
}

#endif






#define INPUT_FILE          INCHI_IOSTREAM



#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )

#ifdef TARGET_API_LIB
/* #define InchiToAtom        ll_InchiToInchi_Atom */
#else
/*#define InchiToAtom        ee_InchiToIbChI_Atom*/
#define FindToken           e_FindToken
#define LoadLine            e_LoadLine
#endif

#define AT_NUM_BONDS(AT)    (AT).num_bonds
#define ATOM_NUMBER         AT_NUM
#define IN_NEIGH_LIST       is_in_the_slist
/*#define INPUT_FILE          INCHI_IOSTREAM*/
#define Create_Atom         CreateInchi_Atom
#define AT_BONDS_VAL(AT,I)  AT[I].num_iso_H[0]
#define ISOLATED_ATOM       (-15)
#define NUM_ISO_Hk(AT,I,K)  AT[I].num_iso_H[K+1]
#define IS_METAL_ATOM(AT,I) is_element_a_metal( AT[I].elname )

#else

#define inchi_Atom          inp_ATOM
#define AT_NUM_BONDS(AT)    (AT).valence
#define ATOM_NUMBER         AT_NUMB
#define IN_NEIGH_LIST       is_in_the_list
#define inchi_NUMH2(AT,N)   NUMH(AT,N)
/*#define InchiToAtom        cc_InchiToInpAtom*/
/*#define INPUT_FILE          FILE*/
#define Create_Atom         CreateInpAtom
#define AT_BONDS_VAL(AT,I)  AT[I].chem_bonds_valence
#define ISOLATED_ATOM       15
#define NUM_ISO_Hk(AT,I,K)  AT[I].num_iso_H[K]
#define IS_METAL_ATOM(AT,I) is_el_a_metal( AT[I].el_number )

#endif

/*****************************************************************************/

#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )

/*****************************************************************************/
int InchiToInchi_Atom ( INCHI_IOSTREAM *inp_file,
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
                        char *pStrErr );

int InchiToInchi_Atom ( INCHI_IOSTREAM *inp_file,
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
    return InchiToAtom ( inp_file,
                         NULL, stereo0D, num_stereo0D,
                         bDoNotAddH, vABParityUnknown,
                         nInputType,
                         at, max_num_at,
                         num_dimensions, num_bonds,
                         pSdfLabel, pSdfValue,
                         Id,
                         pInpAtomFlags,
                         err, pStrErr );
}

#endif


#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
#else

/****************************************************************************/
int InchiToAtom( INCHI_IOSTREAM *inp_file,
                 MOL_COORD **szCoord,
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
int      b2D=0, b3D=0, b23D, nNumBonds = 0, bNonZeroXYZ, bNonMetal;
int      len_stereo0D = 0, max_len_stereo0D = 0;
inchi_Stereo0D  *atom_stereo0D = NULL;
inchi_Atom      *atom          = NULL;
MOL_COORD       *pszCoord      = NULL;
INCHI_MODE InpAtomFlags = 0; /* 0 or FLAG_INP_AT_NONCHIRAL or FLAG_INP_AT_CHIRAL */
static const char szIsoH[] = "hdt";
/* plain tags */
static const char sStructHdrPln[]         = "Structure:";
static const char sStructHdrPlnNoLblVal[] = " is missing";
static char sStructHdrPlnAuxStart[64] =""; /*"$1.1Beta/";*/
static int  lenStructHdrPlnAuxStart = 0;
static const char sStructHdrPlnRevAt[]    = "/rA:";
static const char sStructHdrPlnRevBn[]    = "/rB:";
static const char sStructHdrPlnRevXYZ[]   = "/rC:";
const  char *sToken;
int  lToken;

    if ( !lenStructHdrPlnAuxStart )
        lenStructHdrPlnAuxStart = sprintf( sStructHdrPlnAuxStart, "AuxInfo=" );


    if ( at )
    {

        if ( *at && max_num_at )
            memset( *at, 0, max_num_at * sizeof(**at) );

#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
        if ( stereo0D && num_stereo0D )
        {
            if ( *stereo0D && *num_stereo0D )
            {
                max_len_stereo0D = *num_stereo0D;
                memset( *stereo0D, 0, max_len_stereo0D * sizeof( **stereo0D ));
            }
            else
                max_len_stereo0D = 0;
        }
#else
        if ( szCoord && *szCoord )
        {
            inchi_free( *szCoord );
            *szCoord = NULL;
        }
#endif
    }
    else  /* if ( at )  */
        bFindNext = 1;


    bHeaderRead = bErrorMsg = bRestoreInfo = 0;
    *num_dimensions = *num_bonds = 0;


    /*************************************************************/
    /*   extract reversibility info from plain text INChI format */
    /*************************************************************/

    if ( nInputType == INPUT_INCHI_PLAIN )
    {

        bHeaderRead = hk = 0;

        while ( 0 < (res = inchi_ios_getsTab( szLine, sizeof(szLine)-1, inp_file, &bTooLongLine ) ) )
        {

            /********************* find and interpret structure header ************/
            if ( !bTooLongLine &&
                 (hlen=sizeof(sStructHdrPln)-1, !memcmp(szLine, sStructHdrPln, hlen)) )
            {
                p = szLine + hlen;
                longID = 0;
                num_atoms = 0;

                /* structure number */
                longID = strtol( p, &q, 10 );
                if ( q && q[0] == '.' && q[1] == ' ' )
                    p = q+2;
                p = p + strspn( p, " \n\r" );

                if ( pSdfLabel )
                    pSdfLabel[0] = '\0';

                if ( pSdfValue )
                    pSdfValue[0] = '\0';


                if ( *p )
                {
                    /* has label name */

                    /*p ++;*/
                    if ( q = strchr( p, '=' ) )
                    {

                        /* '=' separates label name from the value */
                        len = inchi_min( q-p+1, MAX_SDF_HEADER-1);

                        if ( pSdfLabel )
                        {
                            mystrncpy( pSdfLabel, p, len );
                            lrtrim( pSdfLabel, &len );
                        }

                        p = q+1;
                        q = p + (int)strlen( p );

                        if ( q-p > 0 )
                        {
                            len = inchi_min( q-p+1, MAX_SDF_VALUE-1);
                            if ( pSdfValue )
                            {
                                mystrncpy( pSdfValue, p, len );
                            }
                            p = q;
                        }
                    }
                    else if ( q = strstr( p, sStructHdrPlnNoLblVal ) )
                    {
                        len = inchi_min( q-p+1, MAX_SDF_HEADER-1);
                        if ( pSdfLabel ) {
                            mystrncpy( pSdfLabel, p, len );
                        }
                        p = q+1;
                    }
                }

                if ( Id )
                    *Id = longID;

                bHeaderRead = 1;
                bErrorMsg = bRestoreInfo = 0;
            }
            else if ( !memcmp( szLine, sStructHdrPlnAuxStart, lenStructHdrPlnAuxStart) )
            {
                /* found the header of the AuxInfo, read AuxInfo head of the line */

                if ( !bHeaderRead )
                {
                    longID = 0;
                    if ( Id )
                        *Id = longID;
                    if ( pSdfLabel )
                        pSdfLabel[0] = '\0';
                    if ( pSdfValue )
                        pSdfValue[0] = '\0';
                }

                bHeaderRead = 0;

                /* check for empty "AuxInfo=ver//" */

                p = strchr( szLine + lenStructHdrPlnAuxStart, '/' );

                if ( p && p[1] == '/' && (!p[2] || '\n' == p[2]) )
                {
                    goto bypass_end_of_INChI_plain;
                }

                /***************** search for atoms block (plain) **********************/

                p = szLine;
                sToken = sStructHdrPlnRevAt;
                lToken = sizeof(sStructHdrPlnRevAt)-1;

                /* search for sToken in the line; load next segments of the line if sToken has not found */

                p = FindToken( inp_file, &bTooLongLine, sToken, lToken,
                               szLine, sizeof(szLine), p, &res );

                if ( !p )
                {
                    *err      = INCHI_INP_ERROR_ERR;
                    num_atoms = INCHI_INP_ERROR_RET;
                    TREAT_ERR (*err, 0, "Missing atom data");
                    goto bypass_end_of_INChI_plain;
                }
                else
                {
                    /* atoms block started */

                    i = 0;
                    res2 = bTooLongLine2 = -1;
                    bItemIsOver = (s = strchr( p, '/') ) || !bTooLongLine;

                    while ( 1 )
                    {

                        p = LoadLine( inp_file, &bTooLongLine, &bItemIsOver, &s,
                                      szLine, sizeof(szLine), INCHI_LINE_ADD, p, &res );

                        if ( !i )
                        {
                            /* allocate atom */
                            num_atoms = strtol( p, &q, 10 );

                            if ( !num_atoms || !q || !*q )
                            {
                                num_atoms = 0; /* no atom data */
                                goto bypass_end_of_INChI_plain;
                            }
                            p = q;

                            /* Molfile chirality flag */
                            switch( *p )
                            {
                            case 'c':
                                InpAtomFlags |= FLAG_INP_AT_CHIRAL;
                                p ++;
                                break;
                            case 'n':
                                InpAtomFlags |= FLAG_INP_AT_NONCHIRAL;
                                p ++;
                                break;
                            }

                            if ( at && *at )
                            {
                                if ( num_atoms > max_num_at )
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

                            if ( !at || !*at )
                            {

                                atom = Create_Atom( num_atoms+1 );

                                if ( !atom )
                                {
                                    num_atoms = INCHI_INP_FATAL_RET; /* was -1; error */
                                    *err      = INCHI_INP_FATAL_ERR;
                                    TREAT_ERR (*err, 0, "Out of RAM");
                                    goto bypass_end_of_INChI_plain;
                                }
                            }

                            if ( stereo0D && *stereo0D )
                            {
                                if ( num_atoms > max_len_stereo0D )
                                    FreeInchi_Stereo0D( stereo0D );
                                else
                                {
                                    memset( *stereo0D, 0, max_len_stereo0D * sizeof( **stereo0D ) );
                                    atom_stereo0D = *stereo0D;
                                }
                            }

                            if ( !stereo0D || !*stereo0D )
                            {
                                max_len_stereo0D = num_atoms+1;

                                atom_stereo0D = CreateInchi_Stereo0D( max_len_stereo0D );

                                if ( !atom_stereo0D )
                                {
                                    num_atoms = INCHI_INP_FATAL_RET; /* fatal error: cannot allocate */
                                    *err      = INCHI_INP_FATAL_ERR;
                                    TREAT_ERR (*err, 0, "Out of RAM");
                                    goto bypass_end_of_INChI_plain;
                                }
                            }
                        }

                        /* element, first char */
                        if ( !isalpha( UCINT *p ) || !isupper( UCINT *p ) || i >= num_atoms )
                        {
                            break; /* end of atoms block */
                        }

                        atom[i].elname[0] = *p ++;

                        /* element, second char */
                        if ( isalpha( UCINT *p ) && islower( UCINT *p ) )
                        {
                            atom[i].elname[1] = *p ++;
                        }

    #if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
    #else
                        atom[i].el_number = get_periodic_table_number( atom[i].elname );
    #endif

                        /* bonds' valence + number of non-isotopic H */
                        if ( isdigit( UCINT *p ) )
                        {
                            AT_BONDS_VAL(atom,i) = (char)strtol( p, &q, 10 );
                            if ( !AT_BONDS_VAL(atom,i) )
                                AT_BONDS_VAL(atom,i) = ISOLATED_ATOM; /* same convention as in MOLfile, found zero bonds valence */
                            p = q;
                        }

                        /* charge */
                        atom[i].charge = (*p == '+')? 1 : (*p == '-')    ? -1
                                                                        : 0;
                        if ( atom[i].charge )
                        {
                            p ++;
                            if ( isdigit( UCINT *p ) )
                            {
                                atom[i].charge *= (S_CHAR)(strtol( p, &q, 10 ) & CHAR_MASK);
                                p = q;
                            }
                        }

                        /* radical */
                        if ( *p == '.' )
                        {
                            p ++;
                            if ( isdigit( UCINT *p ) )
                            {
                                atom[i].radical = (S_CHAR)strtol( p, &q, 10 );
                                p = q;
                            }
                        }

                        /* isotopic mass */
                        if ( *p == 'i' )
                        {
                            p ++;
                            if ( isdigit( UCINT *p ) )
                            {
                                int mw = strtol( p, &q, 10 );
                                p = q;

    #if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
                                atom[i].isotopic_mass = mw;
    #else
                                mw -= get_atomic_mass_from_elnum( atom[i].el_number );
                                if ( mw >= 0 )
                                    mw ++;
                                atom[i].iso_atw_diff = mw;
    #endif
                            }
                        }

                        /* parity */
                        switch( *p )
                        {
                        case 'o':
                            parity = INCHI_PARITY_ODD;
                            p ++;
                            break;
                        case 'e':
                            parity = INCHI_PARITY_EVEN;
                            p ++;
                            break;
                        case 'u':
                            parity = INCHI_PARITY_UNKNOWN;
                            p ++;
                            break;
                        case '?':
                            parity = INCHI_PARITY_UNDEFINED;
                            p ++;
                            break;
                        default:
                            parity = 0;
                            break;
                        }

                        if ( parity )
                        {
                            atom_stereo0D[len_stereo0D].central_atom = i;
                            atom_stereo0D[len_stereo0D].parity       = parity;
                            atom_stereo0D[len_stereo0D].type         = INCHI_StereoType_Tetrahedral;
                            len_stereo0D ++;
                        }

                        /* isotopic h, d, t */
                        for ( k = 0; k < NUM_H_ISOTOPES; k ++ )
                        {
                            if ( *p == szIsoH[k] ) {
                                NUM_ISO_Hk(atom,i,k) = 1;
                                p ++;
                                if ( isdigit( UCINT *p ) ) {
                                    NUM_ISO_Hk(atom,i,k) = (char)strtol( p, &q, 10 );
                                    p = q;
                                }
                            }
                        }

                        i ++;
                    }


                    if ( !bItemIsOver || i != num_atoms || s && p != s )
                    {
                        num_atoms = INCHI_INP_ERROR_RET; /* error */
                        *err      = INCHI_INP_ERROR_ERR;
                        TREAT_ERR (*err, 0, "Wrong number of atoms");
                        goto bypass_end_of_INChI_plain;
                    }
                }

                /***************** search for bonds block (plain) and read it *****************/

                /*p = szLine;*/
                sToken = sStructHdrPlnRevBn;
                lToken = sizeof(sStructHdrPlnRevBn)-1;

                /* search for sToken in the line; load next segments of the line if sToken has not found */

                p = FindToken( inp_file, &bTooLongLine, sToken, lToken,
                               szLine, sizeof(szLine), p, &res );

                if ( !p )
                {
                    num_atoms = INCHI_INP_ERROR_RET; /* error */
                    *err      = INCHI_INP_ERROR_ERR;
                    TREAT_ERR (*err, 0, "Missing bonds data");
                    goto bypass_end_of_INChI_plain;
                }
                else
                {
                    /* bonds block started */

                    i = 1;

                    res2 = bTooLongLine2 = -1;

                    bItemIsOver = (s = strchr( p, '/') ) || !bTooLongLine;

                    if ( 1 == num_atoms )
                    {
                        /* needed because the next '/' may be still out of szLine */

                        p = LoadLine( inp_file, &bTooLongLine, &bItemIsOver, &s,
                                      szLine, sizeof(szLine), INCHI_LINE_ADD, p, &res );
                    }

                    while ( i < num_atoms )
                    {

                        p = LoadLine( inp_file, &bTooLongLine, &bItemIsOver, &s,
                                      szLine, sizeof(szLine), INCHI_LINE_ADD, p, &res );

                        if ( i >= num_atoms || s && p >= s )
                        {
                            break; /* end of bonds (plain) */
                        }

                        /* bond, first char */
                        if ( *p == ';' )
                        {
                            p ++;
                            i ++;
                            continue;
                        }

                        if ( !isalpha( UCINT *p ) )
                        {
                            num_atoms = INCHI_INP_ERROR_RET; /* error */
                            *err      = INCHI_INP_ERROR_ERR;
                            TREAT_ERR (*err, 0, "Wrong bonds data");
                            goto bypass_end_of_INChI_plain;
                        }

                        bond_char = *p ++;

                        /* bond parity */
                        switch( *p )
                        {
                        case '-':
                            bond_parity = INCHI_PARITY_ODD;
                            p ++;
                            break;
                        case '+':
                            bond_parity = INCHI_PARITY_EVEN;
                            p ++;
                            break;
                        case 'u':
                            bond_parity = INCHI_PARITY_UNKNOWN;
                            p ++;
                            break;
                        case '?':
                            bond_parity = INCHI_PARITY_UNDEFINED;
                            p ++;
                            break;
                        default:
                            bond_parity = 0;
                            break;
                        }

                        if ( bond_parity )
                        {
                            switch( *p )
                            {
                            case '-':
                                bond_parityNM = INCHI_PARITY_ODD;
                                p ++;
                                break;
                            case '+':
                                bond_parityNM = INCHI_PARITY_EVEN;
                                p ++;
                                break;
                            case 'u':
                                bond_parityNM = INCHI_PARITY_UNKNOWN;
                                p ++;
                                break;
                            case '?':
                                bond_parityNM = INCHI_PARITY_UNDEFINED;
                                p ++;
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
                        if ( !isdigit( UCINT *p ) )
                        {
                            num_atoms = INCHI_INP_ERROR_RET; /* error */
                            *err      = INCHI_INP_ERROR_ERR;
                            TREAT_ERR (*err, 0, "Wrong bonds data");
                            goto bypass_end_of_INChI_plain;
                        }

                        neigh = (int)strtol( p, &q, 10 )-1;

                        if ( i >= num_atoms || neigh >= num_atoms ) {
                            num_atoms = INCHI_INP_ERROR_RET; /* error */
                            *err      = INCHI_INP_ERROR_ERR;
                            TREAT_ERR (*err, 0, "Bond to nonexistent atom");
                            goto bypass_end_of_INChI_plain;
                        }

                        p = q;
                        bond_stereo1 = bond_stereo2 = 0;

                        /* bond type & 2D stereo */
                        switch( bond_char )
                        {
                        case 'v':
                            bond_type    = INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 = INCHI_BOND_STEREO_SINGLE_1EITHER;
                            bond_stereo2 = INCHI_BOND_STEREO_SINGLE_2EITHER;
                            break;
                        case 'V':
                            bond_type    = INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 = INCHI_BOND_STEREO_SINGLE_2EITHER;
                            bond_stereo2 = INCHI_BOND_STEREO_SINGLE_1EITHER;
                            break;
                        case 'w':
                            bond_type    = INCHI_BOND_TYPE_DOUBLE;
                            bond_stereo1 =
                            bond_stereo2 = INCHI_BOND_STEREO_DOUBLE_EITHER;
                            break;
                        case 's':
                            bond_type    = INCHI_BOND_TYPE_SINGLE;
                            break;
                        case 'd':
                            bond_type    = INCHI_BOND_TYPE_DOUBLE;
                            break;
                        case 't':
                            bond_type    = INCHI_BOND_TYPE_TRIPLE;
                            break;
                        case 'a':
                            bond_type    = INCHI_BOND_TYPE_ALTERN;
                            break;
                        case 'p':
                            bond_type    =  INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 =  INCHI_BOND_STEREO_SINGLE_1UP;
                            bond_stereo2 =  INCHI_BOND_STEREO_SINGLE_2UP;
                            break;
                        case 'P':
                            bond_type    =  INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 =  INCHI_BOND_STEREO_SINGLE_2UP;
                            bond_stereo2 =  INCHI_BOND_STEREO_SINGLE_1UP;
                            break;
                        case 'n':
                            bond_type    =  INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 =  INCHI_BOND_STEREO_SINGLE_1DOWN;
                            bond_stereo2 =  INCHI_BOND_STEREO_SINGLE_2DOWN;
                            break;
                        case 'N':
                            bond_type    =  INCHI_BOND_TYPE_SINGLE;
                            bond_stereo1 =  INCHI_BOND_STEREO_SINGLE_2DOWN;
                            bond_stereo2 =  INCHI_BOND_STEREO_SINGLE_1DOWN;
                            break;
                        default:
                            num_atoms = INCHI_INP_ERROR_RET; /* error */
                            *err      = INCHI_INP_ERROR_ERR;
                            TREAT_ERR (*err, 0, "Wrong bond type");
                            goto bypass_end_of_INChI_plain;
                        }

                        k = AT_NUM_BONDS(atom[i]) ++;

                        atom[i].bond_type[k]   = bond_type;
                        atom[i].bond_stereo[k] = bond_stereo1;
                        atom[i].neighbor[k]    = (ATOM_NUMBER)neigh;

                        k2 = AT_NUM_BONDS(atom[neigh]) ++;
                        atom[neigh].bond_type[k2]   = bond_type;
                        atom[neigh].bond_stereo[k2] = bond_stereo2;
                        atom[neigh].neighbor[k2]    = (ATOM_NUMBER)i;

                        bond_parity |= (bond_parityNM << SB_PARITY_SHFT);

                        if ( bond_parity )
                        {
                            if ( max_len_stereo0D <= len_stereo0D )
                            {
                                /* realloc atom_Stereo0D */

                                inchi_Stereo0D *new_atom_stereo0D = CreateInchi_Stereo0D( max_len_stereo0D+num_atoms );

                                if ( !new_atom_stereo0D )
                                {
                                    num_atoms = INCHI_INP_FATAL_RET; /* fatal error: cannot allocate */
                                    *err      = INCHI_INP_FATAL_ERR;
                                    TREAT_ERR (*err, 0, "Out of RAM");
                                    goto bypass_end_of_INChI_plain;
                                }

                                memcpy( new_atom_stereo0D, atom_stereo0D, len_stereo0D * sizeof(*atom_stereo0D) );
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
                            atom_stereo0D[len_stereo0D].parity      = bond_parity;
                            atom_stereo0D[len_stereo0D].type        = INCHI_StereoType_DoubleBond; /* incl allenes & cumulenes */
                            len_stereo0D ++;
                        }
                    }

                    if ( !bItemIsOver || i != num_atoms || s && p != s )
                    {
                        num_atoms = INCHI_INP_ERROR_RET; /* error */
                        *err      = INCHI_INP_ERROR_ERR;
                        TREAT_ERR (*err, 0, "Wrong number of bonds");
                        goto bypass_end_of_INChI_plain;
                    }
                }

                /***************** search for coordinates block (plain) **********************/
                /*p = szLine;*/

                sToken = sStructHdrPlnRevXYZ;
                lToken = sizeof(sStructHdrPlnRevXYZ)-1;

                /* search for sToken in the line; load next segments of the line if sToken has not found */

                p = FindToken( inp_file, &bTooLongLine, sToken, lToken,
                               szLine, sizeof(szLine), p, &res );

                if ( !p )
                {
                    num_atoms = INCHI_INP_ERROR_RET; /* error */
                    *err      = INCHI_INP_ERROR_ERR;
                    TREAT_ERR (*err, 0, "Missing atom coordinates data");
                    goto bypass_end_of_INChI_plain;
                }
                else
                {
                    /* coordinates block started */
                    if ( pszCoord = (MOL_COORD*) inchi_malloc(inchi_max(num_atoms,1) * sizeof(MOL_COORD)) )
                    {
                        memset( pszCoord, ' ', inchi_max(num_atoms,1) * sizeof(MOL_COORD));
                    }
                    else
                    {
                        num_atoms = INCHI_INP_FATAL_RET; /* allocation error */
                        *err      = INCHI_INP_FATAL_ERR;
                        TREAT_ERR (*err, 0, "Out of RAM");
                        goto bypass_end_of_INChI_plain;
                    }

                    i = 0;
                    res2 = bTooLongLine2 = -1;
                    bItemIsOver = (s = strchr( p, '/') ) || !bTooLongLine;

                    while ( i < num_atoms )
                    {

                        p = LoadLine( inp_file, &bTooLongLine, &bItemIsOver, &s,
                                      szLine, sizeof(szLine), INCHI_LINE_ADD, p, &res );

                        if ( i >= num_atoms || s && p >= s ) {
                            break; /* end of bonds (plain) */
                        }

                        /* coord, first char */
                        if ( *p == ';' )
                        {
                            for ( k = 0; k < NUM_COORD; k ++ )
                            {
                                pszCoord[i][LEN_COORD*k + 4] = '0';
                            }
                            p ++;
                            i ++;
                            continue;
                        }

                        for ( k = 0; k < 3; k ++ )
                        {
                            double xyz;
                            bNonZeroXYZ = 0;
                            if ( *p == ';' )
                            {
                                pszCoord[i][LEN_COORD*k + 4] = '0';
                                xyz = 0.0;
                            } else
                            if ( *p == ',' )
                            {
                                /* empty */
                                pszCoord[i][LEN_COORD*k + 4] = '0';
                                xyz = 0.0;
                                p ++;
                            }
                            else
                            {
                                xyz = strtod( p, &q );
                                bNonZeroXYZ = fabs(xyz) > MIN_BOND_LENGTH;
                                if ( q != NULL ) {
                                    memcpy( pszCoord[i]+LEN_COORD*k, p, q-p );
                                    if ( *q == ',' )
                                        q ++;
                                    p = q;
                                }
                                else
                                    pszCoord[i][LEN_COORD*k + 4] = '0';
                            }

                            switch( k )
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

                        if ( *p == ';' )
                        {
                            p ++; /* end of this triple of coordinates */
                            i ++;
                        }
                        else
                        {
                            num_atoms = INCHI_INP_ERROR_RET; /* error in input data: atoms, bonds & coord must be present together */
                            *err      = INCHI_INP_ERROR_ERR;
                            TREAT_ERR (*err, 0, "Wrong atom coordinates data");
                            goto bypass_end_of_INChI_plain;
                        }
                    }

                    if ( !bItemIsOver || s && p != s || i != num_atoms )
                    {
                        num_atoms = INCHI_INP_ERROR_RET; /* error */
                        *err      = INCHI_INP_ERROR_ERR;
                        TREAT_ERR (*err, 0, "Wrong number of coordinates");
                        goto bypass_end_of_INChI_plain;
                    }
                } /* end of coordinates */


                /* set special valences and implicit H (xml) */

                b23D = b2D | b3D;
                b2D = b3D = 0;
                if ( at )
                {
                    if ( !*at )
                    {
                        int a1, a2, n1, n2, valence;
                        int chem_bonds_valence;
                        int    nX=0, nY=0, nZ=0, nXYZ;
                        *at = atom;

                        /* special valences */

                        for ( bNonMetal = 0; bNonMetal < 1; bNonMetal ++ )
                        {

                            for ( a1 = 0; a1 < num_atoms; a1 ++ )
                            {

                                int num_bond_type[MAX_INPUT_BOND_TYPE - MIN_INPUT_BOND_TYPE + 1];

#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
#else
                                int bHasMetalNeighbor=0;
#endif

                                memset( num_bond_type, 0, sizeof(num_bond_type) );

                                valence = AT_BONDS_VAL(atom, a1); /*  save atom valence if available */
                                AT_BONDS_VAL(atom, a1) = 0;

#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
#else
                                atom[a1].orig_at_number = a1+1;
#endif

                                nX = nY = nZ = 0;

                                for ( n1 = 0; n1 < AT_NUM_BONDS(atom[a1]); n1 ++ )
                                {
                                    bond_type = atom[a1].bond_type[n1] - MIN_INPUT_BOND_TYPE;
                                    if ( bond_type < 0 || bond_type > MAX_INPUT_BOND_TYPE - MIN_INPUT_BOND_TYPE )
                                    {
                                        bond_type = 0;
                                        TREAT_ERR (*err, 0, "Unknown bond type in InChI aux assigned as a single bond");
                                    }

                                    num_bond_type[ bond_type ] ++;
                                    nNumBonds ++;
                                    if ( b23D )
                                    {
                                        neigh = atom[a1].neighbor[n1];
                                        nX |= (fabs(atom[a1].x - atom[neigh].x) > MIN_BOND_LENGTH);
                                        nY |= (fabs(atom[a1].y - atom[neigh].y) > MIN_BOND_LENGTH);
                                        nZ |= (fabs(atom[a1].z - atom[neigh].z) > MIN_BOND_LENGTH);
                                    }
                                }

                                chem_bonds_valence = 0;
                                for ( n1 = 0; MIN_INPUT_BOND_TYPE + n1 <= 3 && MIN_INPUT_BOND_TYPE + n1 <= MAX_INPUT_BOND_TYPE; n1 ++ )
                                {
                                    chem_bonds_valence += (MIN_INPUT_BOND_TYPE + n1) * num_bond_type[n1];
                                }

                                if ( MIN_INPUT_BOND_TYPE <= INCHI_BOND_TYPE_ALTERN && INCHI_BOND_TYPE_ALTERN <= MAX_INPUT_BOND_TYPE &&
                                     ( n2 = num_bond_type[INCHI_BOND_TYPE_ALTERN-MIN_INPUT_BOND_TYPE] ) )
                                {

                                    /* accept input aromatic bonds for now */

                                    switch ( n2 )
                                    {
                                    case 2:
                                        chem_bonds_valence += 3;  /* =A- */
                                        break;

                                    case 3:
                                        chem_bonds_valence += 4;  /* =A< */
                                        break;

                                    default:
                                        /*  if 1 or >= 4 aromatic bonds then replace such bonds with single bonds */
                                        for ( n1 = 0; n1 < AT_NUM_BONDS(atom[a1]); n1 ++ )
                                        {
                                            if ( atom[a1].bond_type[n1] == INCHI_BOND_TYPE_ALTERN )
                                            {
                                                ATOM_NUMBER *p1;
                                                a2 = atom[a1].neighbor[n1];
                                                p1 = IN_NEIGH_LIST( atom[a2].neighbor, (ATOM_NUMBER)a1, AT_NUM_BONDS(atom[a2]) );
                                                if ( p1 )
                                                {
                                                    atom[a1].bond_type[n1] =
                                                    atom[a2].bond_type[p1-atom[a2].neighbor] = INCHI_BOND_TYPE_SINGLE;
                                                }
                                                else
                                                {
                                                    *err = -2;  /*  Program error */
                                                    TREAT_ERR (*err, 0, "Program error interpreting InChI aux");
                                                    num_atoms = 0;
                                                    goto bypass_end_of_INChI_plain; /*  no structure */
                                                }
                                            }
                                        }

                                        chem_bonds_valence += n2;
                                        *err |= 32; /*  Unrecognized aromatic bond(s) replaced with single */
                                        TREAT_ERR (*err, 0, "Atom has 1 or more than 3 aromatic bonds");
                                        break;
                                    }
                                }

#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
                                /*************************************************************************************
                                 *
                                 *  Set number of hydrogen atoms
                                 */
                                {
                                    int num_iso_H;
                                    num_iso_H = atom[a1].num_iso_H[1] + atom[a1].num_iso_H[2] + atom[a1].num_iso_H[3];
                                    if ( valence == ISOLATED_ATOM ) {
                                        atom[a1].num_iso_H[0] = 0;
                                    } else
                                    if ( valence && valence >= chem_bonds_valence ) {
                                        atom[a1].num_iso_H[0] = valence - chem_bonds_valence;
                                    } else
                                    if ( valence || bDoNotAddH ) {
                                        atom[a1].num_iso_H[0] = 0;
                                    } else
                                    if ( !bDoNotAddH ) {
                                        atom[a1].num_iso_H[0] = -1; /* auto add H */
                                    }
                                }
#else
                                /* added 2006-07-19 to process aromatic bonds same way as from molfile */
                                if ( n2 && !valence )
                                {
                                    int num_H = NUMH(atom, a1); /* only isotopic */
                                    int chem_valence = chem_bonds_valence;
                                    int bUnusualValenceArom =
                                        detect_unusual_el_valence( (int)atom[a1].el_number, atom[a1].charge,
                                                                    atom[a1].radical, chem_valence,
                                                                    num_H, atom[a1].valence );
                                    int bUnusualValenceNoArom =
                                        detect_unusual_el_valence( (int)atom[a1].el_number, atom[a1].charge,
                                                                    atom[a1].radical, chem_valence-1,
                                                                    num_H, atom[a1].valence );

#if ( CHECK_AROMBOND2ALT == 1 )
                                    if ( bUnusualValenceArom && !bUnusualValenceNoArom && 0 == nBondsValToMetal( atom, a1) )
#else
                                    if ( bUnusualValenceArom && !bUnusualValenceNoArom )
#endif

                                    {
                                        /* typically NH in 5-member aromatic ring */
                                        chem_bonds_valence --;
                                    }
                                }
                                else if ( n2 && valence )
                                {
                                    /* atom has aromatic bonds AND the chemical valence is known */
                                    int num_H = NUMH(atom, a1);
                                    int chem_valence = chem_bonds_valence + num_H;
                                    if ( valence == chem_valence-1 ) {
                                        /* typically NH in 5-member aromatic ring */
                                        chem_bonds_valence --;
                                    }
                                }

                                atom[a1].chem_bonds_valence = chem_bonds_valence;

                                atom[a1].num_H = get_num_H( atom[a1].elname, atom[a1].num_H, atom[a1].num_iso_H, atom[a1].charge, atom[a1].radical,
                                                          atom[a1].chem_bonds_valence,
                                                          valence,
                                                          0, bDoNotAddH, bHasMetalNeighbor );
#endif
                            }
                        }

                        nNumBonds /= 2;

                        if ( b23D && nNumBonds )
                        {
                            nXYZ = nX+nY+nZ;
                            b2D  = (nXYZ > 0);
                            b3D  = (nXYZ == 3);
                            *num_dimensions = b3D? 3 : b2D? 2 : 0;
                            *num_bonds = nNumBonds;
                        }

                        /*======= 0D parities =================================*/

#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
                        if ( len_stereo0D > 0 && atom_stereo0D && stereo0D ) {
                            *stereo0D     = atom_stereo0D;
                            *num_stereo0D = len_stereo0D;
                        } else {
                            FreeInchi_Stereo0D( &atom_stereo0D );
                            *num_stereo0D = len_stereo0D = 0;
                        }
#endif

                        for ( i = 0; i < len_stereo0D; i ++ )
                        {
                            ATOM_NUMBER *p1, *p2;
                            int     sb_ord_from_a1 = -1, sb_ord_from_a2 = -1, bEnd1 = 0, bEnd2 = 0;

                            switch( atom_stereo0D[i].type )
                            {

                            case INCHI_StereoType_Tetrahedral:
                                a1 = atom_stereo0D[i].central_atom;
                                if ( atom_stereo0D[i].parity && (AT_NUM_BONDS(atom[a1]) == 3 || AT_NUM_BONDS(atom[a1]) == 4) )
                                {
                                    int ii, kk = 0;
                                    if ( AT_NUM_BONDS(atom[a1]) == 3 )
                                        atom_stereo0D[i].neighbor[kk++] = a1;
                                    for ( ii = 0; ii < AT_NUM_BONDS(atom[a1]); ii ++ )
                                        atom_stereo0D[i].neighbor[kk++] = atom[a1].neighbor[ii];
                                }

                            break;

                            case INCHI_StereoType_DoubleBond:
#define MAX_CHAIN_LEN 20
                                a1 = atom_stereo0D[i].neighbor[1];
                                a2 = atom_stereo0D[i].neighbor[2];
                                p1 = IN_NEIGH_LIST( atom[a1].neighbor, (ATOM_NUMBER)a2, AT_NUM_BONDS(atom[a1]) );
                                p2 = IN_NEIGH_LIST( atom[a2].neighbor, (ATOM_NUMBER)a1, AT_NUM_BONDS(atom[a2]) );
                                if ( !p1 || !p2 )
                                {
                                    atom_stereo0D[i].type = INCHI_StereoType_None;
                                    atom_stereo0D[i].central_atom = NO_ATOM;
                                    atom_stereo0D[i].neighbor[0] =
                                    atom_stereo0D[i].neighbor[3] = -1;
                                    *err |= 64; /* Error in cumulene stereo */
                                    TREAT_ERR (*err, 0, "0D stereobond not recognized");
                                    break;
                                }

                                /* streobond, allene, or cumulene */

                                sb_ord_from_a1 = p1 - atom[a1].neighbor;
                                sb_ord_from_a2 = p2 - atom[a2].neighbor;

                                if ( AT_NUM_BONDS(atom[a1]) == 2 &&
                                      atom[a1].bond_type[0] + atom[a1].bond_type[1] == 2*INCHI_BOND_TYPE_DOUBLE &&
                                      0 == inchi_NUMH2(atom, a1) &&
                                     (AT_NUM_BONDS(atom[a2]) != 2 ||
                                      atom[a2].bond_type[0] + atom[a2].bond_type[1] != 2*INCHI_BOND_TYPE_DOUBLE ) )
                                {
                                    bEnd2 = 1; /* a2 is the end-atom, a1 is middle atom */
                                }

                                if ( AT_NUM_BONDS(atom[a2]) == 2 &&
                                      atom[a2].bond_type[0] + atom[a2].bond_type[1] == 2*INCHI_BOND_TYPE_DOUBLE &&
                                      0 == inchi_NUMH2(atom, a2) &&
                                     (AT_NUM_BONDS(atom[a1]) != 2 ||
                                      atom[a1].bond_type[0] + atom[a1].bond_type[1] != 2*INCHI_BOND_TYPE_DOUBLE ) )
                                {
                                    bEnd1 = 1; /* a1 is the end-atom, a2 is middle atom */
                                }

                                if ( bEnd2 + bEnd1 == 1 )
                                {
                                    /* allene or cumulene */

                                    ATOM_NUMBER  chain[MAX_CHAIN_LEN+1], prev, cur, next;

                                    if ( bEnd2 && !bEnd1 )
                                    {
                                        cur = a1;
                                        a1 = a2;
                                        a2 = cur;
                                        sb_ord_from_a1 = sb_ord_from_a2;
                                    }

                                    sb_ord_from_a2 = -1;
                                    cur  = a1;
                                    next = a2;
                                    len = 0;
                                    chain[len++] = cur;
                                    chain[len++] = next;

                                    while ( len < MAX_CHAIN_LEN )
                                    {
                                        /* arbitrary very high upper limit to prevent infinite loop */

                                        prev = cur;
                                        cur  = next;
                                            /* follow double bond path && avoid going back */
                                        if ( AT_NUM_BONDS(atom[cur]) == 2 &&
                                             atom[cur].bond_type[0]+atom[cur].bond_type[1] == 2*INCHI_BOND_TYPE_DOUBLE &&
                                             0 == inchi_NUMH2(atom, cur) )
                                        {
                                            next     = atom[cur].neighbor[atom[cur].neighbor[0] == prev];
                                            chain[len++] = next;
                                        }
                                        else
                                        {
                                            break;
                                        }
                                    }
                                    if ( len > 2 &&
                                         (p2 = IN_NEIGH_LIST( atom[cur].neighbor, (ATOM_NUMBER)prev, AT_NUM_BONDS(atom[cur]))) )
                                    {
                                        sb_ord_from_a2 = p2 - atom[cur].neighbor;
                                        a2 = cur;
                                        /* by design we need to pick up the first non-stereo-bond-neighbor as "sn"-atom */
                                        atom_stereo0D[i].neighbor[0] = atom[a1].neighbor[sb_ord_from_a1 == 0];
                                        atom_stereo0D[i].neighbor[1] = a1;
                                        atom_stereo0D[i].neighbor[2] = a2;
                                        atom_stereo0D[i].neighbor[3] = atom[a2].neighbor[sb_ord_from_a2 == 0];

                                        if ( len % 2 )
                                        {
                                            atom_stereo0D[i].central_atom = chain[len/2];
                                            atom_stereo0D[i].type         = INCHI_StereoType_Allene;
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
                                        TREAT_ERR (*err, 0, "Cumulene stereo not recognized (0D)");
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

                                if ( atom_stereo0D[i].type != INCHI_StereoType_None &&
                                     sb_ord_from_a1 >= 0 && sb_ord_from_a2 >= 0 &&
                                     ATOM_PARITY_WELL_DEF( SB_PARITY_2(atom_stereo0D[i].parity) ) )
                                {
                                    /* Detected well-defined disconnected stereo
                                     * locate first non-metal neighbors */

                                    int    a, n, j, /* k,*/ sb_ord, cur_neigh, min_neigh;

                                    for ( k = 0; k < 2; k ++ )
                                    {
                                        a      = k? atom_stereo0D[i].neighbor[2] : atom_stereo0D[i].neighbor[1];
                                        sb_ord = k? sb_ord_from_a2 : sb_ord_from_a1;
                                        min_neigh = num_atoms;
                                        for ( n  = j = 0; j < AT_NUM_BONDS(atom[a]); j ++ )
                                        {
                                            cur_neigh = atom[a].neighbor[j];
                                            if ( j != sb_ord && !IS_METAL_ATOM(atom, cur_neigh) )
                                            {
                                                min_neigh = inchi_min( cur_neigh, min_neigh );
                                            }
                                        }
                                        if ( min_neigh < num_atoms ) {
                                            atom_stereo0D[i].neighbor[k?3:0] = min_neigh;
                                        } else {
                                            TREAT_ERR (*err, 0, "Cannot find non-metal stereobond neighor (0D)");
                                        }
                                    }
                                }

                                break;
                            }
                        }
                        /* end of 0D parities extraction */
/*exit_cycle:;*/
                    }

#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
#else
                    /* transfer atom_stereo0D[] to atom[] */
                    if ( len_stereo0D )
                    {
                        Extract0DParities( atom, num_atoms, atom_stereo0D, len_stereo0D,
                            pStrErr, err, vABParityUnknown );
                    }
#endif

                    if ( pInpAtomFlags )
                    {
                        /* save chirality flag */
                        *pInpAtomFlags |= InpAtomFlags;
                    }
                }
                else if ( atom )
                {
                    inchi_free( atom );
                    atom = NULL;
                }

#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
#else
#if ( FIX_READ_AUX_MEM_LEAK == 1 )

                /* 2005-08-04 avoid memory leak */
                if ( atom_stereo0D && !(stereo0D && *stereo0D == atom_stereo0D) )
                {
                    FreeInchi_Stereo0D( &atom_stereo0D );
                }
#endif

                if ( szCoord )
                {
                    *szCoord = pszCoord;
                    pszCoord = NULL;
                }
                else
#endif
                if ( pszCoord )
                {
                    inchi_free( pszCoord );
                    pszCoord = NULL;
                }

                goto bypass_end_of_INChI_plain;
                /*return num_atoms;*/
            }
        }

        if ( atom_stereo0D )
        {
            FreeInchi_Stereo0D( &atom_stereo0D );
        }


        /* end of structure reading cycle */

        if ( res <= 0 )
        {
            if ( *err == INCHI_INP_ERROR_ERR )
                return num_atoms;

            *err = INCHI_INP_EOF_ERR;
            return INCHI_INP_EOF_RET; /* no more data */
        }


bypass_end_of_INChI_plain:

        /* cleanup */
        if ( num_atoms == INCHI_INP_ERROR_RET && atom_stereo0D )
        {
            if ( stereo0D && *stereo0D == atom_stereo0D )
            {
                *stereo0D     = NULL;
                *num_stereo0D = 0;
            }
            FreeInchi_Stereo0D( &atom_stereo0D );
        }

        while ( bTooLongLine &&
                0 < inchi_ios_getsTab1( szLine, sizeof(szLine)-1, inp_file, &bTooLongLine ) )
        {
            ;
        }


#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
        /* cleanup */
        if ( !*at ) {
            if ( atom ) {
                inchi_free( atom );
                atom = NULL;
            }
            if ( pszCoord ) {
                inchi_free( pszCoord );
                pszCoord = NULL;
            }
        }
#endif


        return num_atoms;
    }

    return num_atoms;

#undef AT_NUM_BONDS
#undef ATOM_NUMBER
#undef IN_NEIGH_LIST
#undef inchi_NUMH2

#if ( defined(TARGET_API_LIB) || defined(TARGET_EXE_USING_API) )
#else
#undef inchi_Atom
#endif

#undef AT_NUM_BONDS
#undef ATOM_NUMBER
#undef IN_NEIGH_LIST
#undef inchi_NUMH2
#undef InchiToAtom
#undef MoreParms
#undef INPUT_FILE
#undef Create_Atom
#undef AT_BONDS_VAL
#undef ISOLATED_ATOM
#undef NUM_ISO_Hk
#undef IS_METAL_ATOM
}
#endif



#endif    /* _AUX2ATOM_H_ */
