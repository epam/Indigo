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


#ifndef __INCHI_DLL_A_H__
#define __INCHI_DLL_A_H__

#include "../../../INCHI_BASE/src/ichicant.h"

typedef struct tagCOMPONENT_TREAT_INFO
{
    int n1;
    int n2;
    int num_atoms;
    int num_at_tg;
    int num_deleted_H;
    int num_deleted_H_taut;
    INCHI_MODE nMode;
    T_GROUP_INFO vt_group_info;
    T_GROUP_INFO vt_group_info_orig;
    ATOM_SIZES  s[TAUT_NUM];
    BCN Bcn;
    int bHasIsotopicAtoms;
    int bMayHaveStereo;
    int num_taut_at;

    int bPointedEdgeStereo;
    int vABParityUnknown; /* actual value of constant for unknown parity (2009-12-10 ) */

    INCHI_MODE bTautFlags;
    INCHI_MODE bTautFlagsDone;
    INCHI_MODE nUserMode;

    sp_ATOM  *at[TAUT_NUM];
    inp_ATOM *out_at;
    int fix_isofixedh; /* 04-12-2008 */
    int fix_termhchrg; /* 07-06-2008 */
} COMPONENT_TREAT_INFO;

typedef struct tagINCHIGEN_CONTROL
{

    int             init_passed;
    int             norm_passed;
    int             canon_passed;

    INPUT_PARMS     InpParms;

    unsigned long   ulTotalProcessingTime;
    char            szTitle[MAX_SDF_HEADER+MAX_SDF_VALUE+256];
    /* Expandable string buffer */
    INCHI_IOSTREAM_STRING strbuf_container;
                                    /*char            *pStr;*/
    long            num_err;
    long            num_inp;


    ORIG_STRUCT     OrigStruct;
    ORIG_ATOM_DATA  OrigInpData;

    /* For the whole structure: */
    STRUCT_DATA StructData;

    /* For each member of pair disconnected/reconnected structures: */
    ORIG_ATOM_DATA  PrepInpData[INCHI_NUM]; /* INCHI_NUM=2;  0   disconnected/original
                                                                1   reconnected         */
    INP_ATOM_DATA   *InpCurAtData[INCHI_NUM];

    INP_ATOM_DATA   *InpNormAtData[INCHI_NUM];
    INP_ATOM_DATA   *InpNormTautData[INCHI_NUM];

    COMP_ATOM_DATA  composite_norm_data[INCHI_NUM][TAUT_NUM+1];
                                        /* TAUT_NUM=2;   0   non-tautomeric
                                                            1   tautomeric
                                                            2   intermediate tautomeric */

    NORM_CANON_FLAGS
                    ncFlags;

    /* For each connected component of structures: */
    PINChI2         *pINChI[INCHI_NUM];
    PINChI_Aux2     *pINChI_Aux[INCHI_NUM];

    /* For each connected component of structures: */
    COMPONENT_TREAT_INFO
                    *cti[INCHI_NUM];

    /* Placed at the end intentionally */
    INCHI_IOSTREAM      inchi_file[3];
} INCHIGEN_CONTROL;



/*^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
Exported functions
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^*/


#if (defined( _WIN32 ) && defined( _MSC_VER ) && defined(BUILD_LINK_AS_DLL) )
    /* Win32 & MS VC ++, compile and link as a DLL */
    #ifdef _USRDLL
        /* InChI library dll */
        #define INCHI_API __declspec(dllexport)
        #define EXPIMP_TEMPLATE
        #define INCHI_DECL
     #else
        /* calling the InChI dll program */
        #define INCHI_API __declspec(dllimport)
        #define EXPIMP_TEMPLATE extern
        #define INCHI_DECL
     #endif
#else
    /* create a statically linked InChI library or link to an executable */
    #define INCHI_API
    #define EXPIMP_TEMPLATE
    #define INCHI_DECL
#endif



/* to compile all InChI code as a C++ code #define COMPILE_ALL_CPP */
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif




/* Local functions */
void make_norm_atoms_from_inp_atoms(INCHIGEN_DATA *gendata, INCHIGEN_CONTROL *genctl);

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#define PSTR_BUFFER_SIZE 511999


#endif /* __INCHI_DLL_A_H__ */
