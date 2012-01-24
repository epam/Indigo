/*
 * International Chemical Identifier (InChI)
 * Version 1
 * Software version 1.04
 * September 9, 2011
 *
 * The InChI library and programs are free software developed under the
 * auspices of the International Union of Pure and Applied Chemistry (IUPAC).
 * Originally developed at NIST. Modifications and additions by IUPAC 
 * and the InChI Trust.
 *
 * IUPAC/InChI-Trust Licence No.1.0 for the 
 * International Chemical Identifier (InChI) Software version 1.04
 * Copyright (C) IUPAC and InChI Trust Limited
 * 
 * This library is free software; you can redistribute it and/or modify it 
 * under the terms of the IUPAC/InChI Trust InChI Licence No.1.0, 
 * or any later version.
 * 
 * Please note that this library is distributed WITHOUT ANY WARRANTIES 
 * whatsoever, whether expressed or implied.  See the IUPAC/InChI Trust 
 * Licence for the International Chemical Identifier (InChI) Software 
 * version 1.04, October 2011 ("IUPAC/InChI-Trust InChI Licence No.1.0") 
 * for more details.
 * 
 * You should have received a copy of the IUPAC/InChI Trust InChI 
 * Licence No. 1.0 with this library; if not, please write to:
 * 
 * The InChI Trust
 * c/o FIZ CHEMIE Berlin
 *
 * Franklinstrasse 11
 * 10587 Berlin
 * GERMANY
 *
 * or email to: ulrich@inchi-trust.org.
 * 
 */


#ifndef __INCHISTER_H__
#define __INCHISTER_H__

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif
int bCanAtomBeAStereoCenter( char *elname, S_CHAR charge, S_CHAR radical );
int bCanInpAtomBeAStereoCenter( inp_ATOM *at, int cur_at, int bPointedEdgeStereo );
int bCanAtomHaveAStereoBond( char *elname, S_CHAR charge, S_CHAR radical );
int bCanAtomBeTerminalAllene( char *elname, S_CHAR charge, S_CHAR radical );
int bCanAtomBeMiddleAllene( char *elname, S_CHAR charge, S_CHAR radical );
int bAtomHasValence3( char *elname, S_CHAR charge, S_CHAR radical );
int set_stereo_parity( inp_ATOM* at, sp_ATOM* at_output, int num_at, int num_removed_H,
                       int *nMaxNumStereoAtoms, int *nMaxNumStereoBonds, INCHI_MODE nMode,
                       int bPointedEdgeStereo, int vABParityUnknown );
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

#endif  /* __INCHISTER_H__ */
