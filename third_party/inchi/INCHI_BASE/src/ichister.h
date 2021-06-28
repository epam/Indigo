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


#ifndef _ICHISTER_H_
#define _ICHISTER_H_

#include "ichicomn.h"

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif
    int bCanAtomBeAStereoCenter( char *elname, S_CHAR charge, S_CHAR radical );
    int bCanInpAtomBeAStereoCenter( inp_ATOM *at, int cur_at, int bPointedEdgeStereo, int bStereoAtZz );
    int bCanAtomHaveAStereoBond( char *elname, S_CHAR charge, S_CHAR radical );
    int bCanAtomBeTerminalAllene( char *elname, S_CHAR charge, S_CHAR radical );
    int bCanAtomBeMiddleAllene( char *elname, S_CHAR charge, S_CHAR radical );
    int bAtomHasValence3( char *elname, S_CHAR charge, S_CHAR radical );

    double dot_prod3(const double a[], const double b[]); 
    double* cross_prod3(const double a[], const double b[], double result[]);

    struct tagCANON_GLOBALS;
    int set_stereo_parity( struct tagCANON_GLOBALS *pCG,
                           inp_ATOM* at,
                           sp_ATOM* at_output,
                           int num_at,
                           int num_removed_H,
                           int *nMaxNumStereoAtoms,
                           int *nMaxNumStereoBonds,
                           INCHI_MODE nMode,
                           int bPointedEdgeStereo,
                           int vABParityUnknown,
                           int bLooseTSACheck,
                           int bStereoAtZz );

    int get_opposite_sb_atom( inp_ATOM *at, int cur_atom, int icur2nxt,
                              int *pnxt_atom, int *pinxt2cur, int *pinxt_sb_parity_ord );

#define PES_BIT_POINT_EDGE_STEREO    1
#define PES_BIT_PHOSPHINE_STEREO     2
#define PES_BIT_ARSINE_STEREO        4
#define PES_BIT_FIX_SP3_BUG          8

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif    /* _ICHISTER_H_ */
