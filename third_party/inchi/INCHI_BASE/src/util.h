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


#ifndef _UTIL_H_
#define _UTIL_H_

#include "inpdef.h"

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

int get_atomic_mass(const char *elname);
int get_atomic_mass_from_elnum( int nAtNum );
int get_num_H ( const char* elname,
                int inp_num_H,
                S_CHAR num_iso_H[],
                int charge,
                int radical,
                int chem_bonds_valence,
                int atom_input_valence,
                int bAliased,
                int bDoNotAddH,
                int bHasMetalNeighbor );
int extract_charges_and_radicals( char *elname,
                                  int *pnRadical,
                                  int *pnCharge );
int extract_H_atoms( char *elname, S_CHAR num_iso_H[] );

int normalize_string( char* name );
int read_upto_delim( char **pstring, char *field, int maxlen, char* delims );
int is_matching_any_delim( char c, char* delims );
int dotify_non_printable_chars( char *line );
char* lrtrim( char *p, int* nLen );
void remove_trailing_spaces( char* p );
void remove_one_lf( char* p);
int mystrncpy( char *target,
               const char *source,
               unsigned maxlen);
void mystrrev( char *p );


#define ALPHA_BASE  27


int inchi_memicmp( const void * p1, const void * p2, size_t length );
int inchi_stricmp( const char *s1, const char *s2 );
char *inchi__strnset( char *s, int val, size_t length );
char *inchi__strdup( const char *string );


long    inchi_strtol( const char *str, const char **p, int base);
double    inchi_strtod( const char *str, const char **p );
AT_NUMB *is_in_the_list( AT_NUMB *pathAtom, AT_NUMB nNextAtom, int nPathLen );
int        *is_in_the_ilist( int *pathAtom, int nNextAtom, int nPathLen );
int is_ilist_inside( int *ilist, int nlist, int *ilist2, int nlist2 );

int get_periodic_table_number( const char* elname );
int is_el_a_metal( int nPeriodicNum );
int get_el_valence( int nPeriodicNum,
                    int charge,
                    int val_num );
int get_unusual_el_valence( int nPeriodicNum,
                            int charge,
                            int radical,
                            int bonds_valence,
                            int num_H,
                            int num_bonds );
/*  Output valence that does not fit any known valences */
int detect_unusual_el_valence( int nPeriodicNum,
                               int charge,
                               int radical,
                               int bonds_valence,
                               int num_H,
                               int num_bonds );
int needed_unusual_el_valence( int nPeriodicNum,
                               int charge,
                               int radical,
                               int bonds_valence,
                               int actual_bonds_val,
                               int num_H,
                               int num_bonds );
int get_el_type( int nPeriodicNum );
int if_skip_add_H( int nPeriodicNum );
int get_element_chemical_symbol(int nAtNum, char *szElement );
int MakeRemovedProtonsString( int nNumRemovedProtons,
                              NUM_H *nNumExchgIsotopicH,
                              NUM_H *nNumRemovedProtonsIsotopic,
                              int bIsotopic,
                              char *szRemovedProtons,
                              int *num_removed_iso_H );


/*
    Ion pairs and fixing bonds
*/


int num_of_H( inp_ATOM *at, int iat );
int has_other_ion_neigh( inp_ATOM *at,
                         int iat,
                         int iat_ion_neigh,
                         const char *el,
                         int el_len );
int has_other_ion_in_sphere_2( inp_ATOM *at,
                               int iat,
                               int iat_ion_neigh,
                               const char *el,
                               int el_len );
int nNoMetalNumBonds( inp_ATOM *at, int at_no );
int nNoMetalBondsValence( inp_ATOM *at, int at_no );
int nNoMetalNeighIndex( inp_ATOM *at, int at_no );
int nNoMetalOtherNeighIndex( inp_ATOM *at,
                             int at_no,
                             int cur_neigh );
int nNoMetalOtherNeighIndex2( inp_ATOM *at,
                              int at_no,
                              int cur_neigh,
                              int cur_neigh2 );
void extract_inchi_substring(char ** buf,
                             const char *str,
                             size_t slen);
int nBondsValToMetal( inp_ATOM* at, int iat );
int nBondsValenceInpAt( const inp_ATOM *at,
                        int *nNumAltBonds,
                        int *nNumWrongBonds );
int bHeteroAtomMayHaveXchgIsoH( inp_ATOM *atom, int iat );
int get_endpoint_valence( U_CHAR el_number );
#if ( KETO_ENOL_TAUT == 1 )
int get_endpoint_valence_KET( U_CHAR el_number );
#endif

/* Forward declaration */
struct tagCANON_GLOBALS;

int SetBitFree( struct tagCANON_GLOBALS *pCG);
void WriteCoord( char *str, double x );
extern const int ERR_ELEM;
extern const int nElDataLen;

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif    /* _UTIL_H_ */
