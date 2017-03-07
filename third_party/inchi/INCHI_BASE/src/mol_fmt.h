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


#ifndef _MOL_FMT_H_
#define _MOL_FMT_H_

#include <stdio.h>

#include "ichisize.h"

/*
    Data structures and constants
*/


/*************** read MOL file V2000.************************/
/* ref: A.Dalby et al, "Description of Several Chemical Structure
 * File Formats Used by Computer Programs Developed at Molecular
 * Design Limited", J. Chem. Inf. Comput. Sci., 1992, 32, 244-255.
 */

/*************** read MOL file V3000.************************/
/* http://download.accelrys.com/freeware/ctfile-formats/CTFile-formats.zip
 * Last accessed 2013-06-11
*/

/*-----------*/
/* CONSTANTS */
/*-----------*/

#define SD_FMT_END_OF_DATA "$$$$"

#define MOL_FMT_INPLINELEN   204  /* add cr, lf, double zero termination */
#ifndef MOL_FMT_MAXLINELEN
#define MOL_FMT_MAXLINELEN   200
#endif

#define MOL_FMT_PRESENT 1
#define MOL_FMT_ABSENT  0

/* configuration */
#define MOL_FMT_QUERY   MOL_FMT_ABSENT
#define MOL_FMT_CPSS    MOL_FMT_ABSENT
#define MOL_FMT_REACT   MOL_FMT_ABSENT

#define MOL_FMT_STRING_DATA      'S'
#define MOL_FMT_CHAR_INT_DATA    'C'
#define MOL_FMT_SHORT_INT_DATA   'N'
#define MOL_FMT_LONG_INT_DATA    'L'
#define MOL_FMT_DOUBLE_DATA      'D'
#define MOL_FMT_FLOAT_DATA       'F'
#define MOL_FMT_JUMP_TO_RIGHT    'J'
#define MOL_FMT_MAX_VALUE_LEN    32    /* max length of string containing a numerical value */
#define MOL_FMT_INT_DATA    'I'

#define MOL_FMT_M_STY_NON    0
#define MOL_FMT_M_STY_SRU   1
#define MOL_FMT_M_STY_MON   2
#define MOL_FMT_M_STY_COP   3
#define MOL_FMT_M_STY_MOD    4
#define MOL_FMT_M_STY_CRO    5
#define MOL_FMT_M_STY_MER    6

#define MOL_FMT_M_SST_NON   0
#define MOL_FMT_M_SST_ALT   1
#define MOL_FMT_M_SST_RAN   2
#define MOL_FMT_M_SST_BLK   3

#define MOL_FMT_M_CONN_NON  0
#define MOL_FMT_M_CONN_HT   1
#define MOL_FMT_M_CONN_HH   2
#define MOL_FMT_M_CONN_EU   3


/* V3000 specific constants */
#define MOL_FMT_V3000_STENON -1
#define MOL_FMT_V3000_STEABS 1
#define MOL_FMT_V3000_STEREL 2
#define MOL_FMT_V3000_STERAC 3

/* provisional limits for V3000 */
/* TODO: remove, replace affected strings with reallocatable buffers */
#define MOL_FMT_V3000_INPLINELEN   32004  /* add cr, lf, double zero termination */
#ifndef MOL_FMT_V3000_MAXLINELEN
#define MOL_FMT_V3000_MAXLINELEN   32000
#endif
#define MOL_FMT_V3000_MAXFIELDLEN   4096

/*#ifdef TARGET_EXE_USING_API*/
#ifndef ISOTOPIC_SHIFT_FLAG
#define ISOTOPIC_SHIFT_FLAG   10000 /* add to isotopic mass if isotopic_mass =      */
#endif
/*#endif*/

/*-------------------*/
/* SIMPLE DATA TYPES */
/*-------------------*/

#ifndef INCHI_US_CHAR_DEF
typedef signed char   S_CHAR;
typedef unsigned char U_CHAR;
#define INCHI_US_CHAR_DEF
#endif

#ifndef LEN_COORD
#define LEN_COORD 10
#endif
#ifndef NUM_COORD
#define NUM_COORD 3
#endif


/*-----------------*/
/* DATA STRUCTURES */
/*-----------------*/


/*    NUM_LISTS - dynamically growing array of numeric lists */
typedef struct A_NUM_LISTS
{
    int        **lists;
    int     allocated;
    int     used;
    int     increment;
} NUM_LISTS;
int  NumLists_Alloc( NUM_LISTS *num_lists, int nlists );
int  NumLists_ReAlloc( NUM_LISTS *num_lists );
int  NumLists_Append( NUM_LISTS *num_lists, int *list );
void NumLists_Free( NUM_LISTS *num_lists);
/*    INT_ARRAY - dynamically growing array of int    */
typedef struct tagINT_ARRAY
{
    int *item;
    int allocated;
    int used;
    int increment;
} INT_ARRAY;
int  IntArray_Alloc( INT_ARRAY *items, int nitems );
int  IntArray_ReAlloc( INT_ARRAY *items );
int  IntArray_Append( INT_ARRAY *items, int new_item );
void IntArray_Reset( INT_ARRAY *items);
void IntArray_Free( INT_ARRAY *items);
void IntArray_DebugPrint( INT_ARRAY *items);
/* MOL_FMT_SGROUP is a container for Sgroup data */
typedef struct A_MOL_FMT_SGROUP
{
    int id;                /* it is what is called 'Sgroup number' in CTFile */
    int type;            /* type (STY) */
    int subtype;        /* (SST) */
    int conn;            /* (SCN) */
    int label;            /* it is what is called 'unique Sgroup identifier' in CTFile */
    double xbr1[4];        /* bracket ends coordinates (SDI) */
    double xbr2[4];        /* bracket ends coordinates (SDI) */
    char smt[80];        /* Sgroup Subscript (SMT) */
    INT_ARRAY alist;
    INT_ARRAY blist;
} MOL_FMT_SGROUP;
int  MolFmtSgroup_Create( MOL_FMT_SGROUP **sgroup, int id, int type );
void MolFmtSgroup_Free( MOL_FMT_SGROUP *sgroup );
/*    MOL_FMT_SGROUPS is a dynamically growing array of pointers to MOL_FMT_SGROUP objects */
typedef struct A_MOL_FMT_SGROUPS
{
    MOL_FMT_SGROUP    **group;    /* growable array of pointers to MOL_FMT_SGROUP objects */
    int        allocated;    /* allocated number of objects */
    int        used;        /* current number of objects */
    int        increment;    /* array expansion icrement */
} MOL_FMT_SGROUPS;
int  MolFmtSgroups_Alloc( MOL_FMT_SGROUPS *items, int nitems );
int  MolFmtSgroups_ReAlloc( MOL_FMT_SGROUPS *items );
int  MolFmtSgroups_Append( MOL_FMT_SGROUPS *items, int id, int type );
void MolFmtSgroups_Free( MOL_FMT_SGROUPS *items);
int  MolFmtSgroups_GetIndexBySgroupId( int id, MOL_FMT_SGROUPS *items);

typedef struct A_MOL_FMT_HEADER_BLOCK
{
    /* Line #1 */
    char    molname[MOL_FMT_MAXLINELEN+1];    /* up to 80 characters            */
    /* Line #2: optional */
    char    line2[MOL_FMT_MAXLINELEN+1];    /* the whole line2 -- up to 80 characters */
    char    user_initls[3];                    /* 2 bytes; char                */
    char    prog_name[9];                    /* 8 bytes; char                */
    char    month;                            /* 2 bytes; integral            */
    char    day;                            /* 2 bytes; integral            */
    char    year;                            /* 2 bytes; integral            */
    char    hour;                            /* 2 bytes; integral            */
    char    minute;                            /* 2 bytes; integral            */
    char    dim_code[3];                    /* 2 bytes: dimensional code; char */
    short   scaling_factor1;                /* 2 bytes;  I2                    */
    double  scaling_factor2;                /* 10 bytes, F10.5                */
    double  energy;                            /* 10 bytes, F10.5                */
    long    internal_regno;                    /* 6 bytes, integral            */
    /* Line #3: comment */
    char    comment[81];
} MOL_FMT_HEADER_BLOCK;


typedef struct A_MOL_FMT_ATOM
{
    double   fx;                            /* F10.5;       Generic            */
    double   fy;                            /* F10.5;       Generic            */
    double   fz;                            /* F10.5;       Generic            */
    char     symbol[6];                        /* aaa;         Generic
                                               changed from 4 to 6 to match STDATA */
    S_CHAR   mass_difference;                /* dd;  (M_ISO)
                                               Generic: -3..+4 otherwise 0 or
                                               127=most abund. isotope        */
    S_CHAR   charge;                        /* ccc; (M CHG),
                                               Generic: 1=+3, 2=+2,3=+1,
                                               4=doublet,5=-1,6=-2,7=-3        */
    char     radical;                        /*      (M RAD)                    */
    char     stereo_parity;                    /* sss;         Generic            */
#if ( MOL_FMT_QUERY == MOL_FMT_PRESENT )
    char     H_count_plus_1;                /* hhh;         Query;
                                               Hn means >= n H;
                                               H0 means no H*/
    char     stereo_care;                    /* bbb;         Query: 0=ignore;
                                               1=must match                    */
#endif
    char     valence;                        /* vvv:
                                               0=no marking; (1..14)=(1..14);
                                               15=zero valence. Number of bonds
                                               includes bonds to impl. H's    */

#if ( MOL_FMT_CPSS == MOL_FMT_PRESENT )
    char     H0_designator;                    /* HHH:         CPSS            */
    char     reaction_component_type;        /* rrr:
                                               CPSS: 1=reactant,
                                                     2=product,
                                                     3=intermediate            */
    char     reaction_component_num;        /* iii:         CPSS: 0 to (n-1)*/
#endif

#if ( MOL_FMT_REACT == MOL_FMT_PRESENT )
    short    atom_atom_mapping_num;            /* mmm:        Reaction: 1..255    */
    char     cInversionRetentionFlag;        /* nnn:
                                               1=inverted, 2=retained config.;
                                               0=property not applied        */
#endif
#if ( MOL_FMT_REACT == MOL_FMT_PRESENT || MOL_FMT_QUERY == MOL_FMT_PRESENT )
    char     exact_change_flag;                /* eee                            */
#endif
    char my_n_impH;                            /* number of implicit H calculated for
                                               adding H to strings in STDATA */
    char display_tom;                        /* Do not hide element's name
                                               (applies to C 7-25-98 DCh    */
    char atom_aliased_flag;                    /* Do not remove
                                               charge/radical/isotope if it
                                               is in the alias. 9-3-99 DCh    */
} MOL_FMT_ATOM;


typedef struct A_MOL_FMT_BOND
{
    short atnum1;                            /* 111:     First atom number:  Generic */
    short atnum2;                            /* 222:     Second atom number: Generic */
    char  bond_type;                        /* ttt:
                                               1,2,3=single, double, triple;
                                               4=aromatic;
                                               5=single or double;
                                               6=single or aromatic;
                                               7=double or aromatic;
                                               8=any.
                                               Values 4-8 are for
                                               SSS queries only                */
    char bond_stereo;                        /* sss:
                                               Single bonds:
                                                 0=not stereo, 1=up, 4=either, 6=down
                                               Double bonds:
                                                 0=use x,y,z to determine cis/trans,
                                                 3=cis or trans (either)    */
                                            /* xxx:     not used */

#if ( MOL_FMT_QUERY == MOL_FMT_PRESENT )
    char bond_topology;                        /* rrr:
                                               0=either, 1=ring, 2=chain:
                                               SSS queries only                */
#endif
#if ( MOL_FMT_REACT == MOL_FMT_PRESENT )
    char react_center_status;                /* ccc:
                                               0 = unmarked,
                                               1 = a center,
                                               -1 = not a center;
                                               Additional:
                                               2 = no charge,
                                               4 = bond made/broken,
                                               8 = bond order changes
                                               12=4+8; 5=4+1, 9=8+1, 13=12+1 are also possible      */
#endif
} MOL_FMT_BOND;


typedef struct A_MOL_FMT_v3000
{
    int n_non_star_atoms;
    int n_star_atoms;
    int * atom_index_orig;    /* index as supplied for atoms */
    int * atom_index_fin;    /* = index or -1 for star atom */
    int n_sgroups;            /* currently, we do not use this. */
    int n_3d_constraints;    /* currently, we do not use this. */
    int n_collections;
    int n_non_haptic_bonds;
    int n_haptic_bonds;
    NUM_LISTS *haptic_bonds;/* haptic_bonds[i] is pointer to int* which contains:             */
                            /* bond_type, non-star atom number, nendpts, then endpts themselves */
    /* Enhanced stereo */
    int n_steabs;
    NUM_LISTS *steabs;        /* steabs[k][0] - not used                            */
                            /* steabs[k][1] -  number of members in collection    */
                            /* steabs[k][2..] - member atom numbers                */
    int n_sterel;
    NUM_LISTS *sterel;        /* sterel[k][0] - n from "STERELn" tag                */
                            /* sterel[k][1] -  number of members in collection    */
                            /* sterel[k][2..] - member atom numbers                */
    int n_sterac;
    NUM_LISTS *sterac;        /* sterac[k][0] - n from "STERACn" tag                */
                            /* sterac[k][1] -  number of members in collection    */
                            /* sterac[k][0] - number from "STERACn" tag            */
} MOL_FMT_v3000;


typedef struct A_MOL_FMT_CTAB
{
    /* Line #1: Counts line */
    int n_atoms;                            /* int accounts for possible V3000. Was: aaa; <= 255; Generic */
    int n_bonds;                            /* int accounts for possible V3000. Was: bbb; <= 255; Generic */
#if ( MOL_FMT_QUERY == MOL_FMT_PRESENT )
    short n_atom_lists;                        /* lll; <=  30; Query   */
#endif

    char chiral_flag;                       /* ccc; 0 or 1; Generic */
    short n_stext_entries;                    /* sss;         CPSS    */
#if ( MOL_FMT_CPSS == MOL_FMT_PRESENT )
    short n_reaction_components_plus_1;        /* xxx;         CPSS    */
    short n_reactants;                        /* rrr;         CPSS    */
    short n_products;                        /* ppp;         CPSS    */
    short n_intermediates;                    /* iii;         CPSS    */
#endif
    short n_property_lines;                    /* mmm;         Generic */
    short follow_inchi_1_treating_iso_mass;
    char  version_string[7];                /* vvvvvv;      Generic; 'V2000' */
    MOL_FMT_ATOM  *atoms;                    /* The Atom Block */
    MOL_FMT_BOND *bonds;
    MOL_COORD *coords;
    MOL_FMT_SGROUPS sgroups;                        /*    growable array of pointers to Sgroup objects */
    MOL_FMT_v3000 *v3000;            /* 2013
                                            intentionally separated from
                                            older version_string[7] */
} MOL_FMT_CTAB;


typedef struct A_MOL_FMT_DATA
{

    MOL_FMT_HEADER_BLOCK hdr;

    MOL_FMT_CTAB         ctab;
} MOL_FMT_DATA;



/*
    Functions
*/


MOL_FMT_DATA* ReadMolfile( INCHI_IOSTREAM *inp_file,
                           MOL_FMT_HEADER_BLOCK *OnlyHeaderBlock,
                           MOL_FMT_CTAB *OnlyCTab,
                           int bGetOrigCoord,
                           int treat_polymers,
                           char *pname,
                           int lname,
                           long *Id,
                           const char *pSdfLabel,
                           char *pSdfValue,
                           int *err,
                           char *pStrErr );
int MolfileStrnread( char* dest,
                     char* source,
                     int len,
                     char **first_space );
int MolfileReadField( void* data,
                      int field_len,
                      int data_type,
                      char** line_ptr );
long MolfileExtractStrucNum( MOL_FMT_HEADER_BLOCK *pHdr );
int MolfileHasNoChemStruc( MOL_FMT_DATA* mfdata);
int MolfileSaveCopy( INCHI_IOSTREAM *inp_file,
                     long fPtrStart,
                     long fPtrEnd,
                     FILE *outfile,
                     long num );
int MolfileGetXYZDimAndNormFactors( MOL_FMT_DATA* mfdata,
                                    int find_norm_factors,
                                    double *x0,
                                    double *y0,
                                    double *z0,
                                    double *xmin,
                                    double *ymin,
                                    double *zmin,
                                    double *scaler,
                                    int *err,
                                    char *pStrErr );
MOL_FMT_DATA* FreeMolfileData( MOL_FMT_DATA* mfdata );

/*
    V3000 Molfile
*/

int MolfileV3000Init( MOL_FMT_CTAB* ctab,
                      char *pStrErr );
int MolfileV3000ReadCTABBeginAndCountsLine( MOL_FMT_CTAB* ctab,
                                            INCHI_IOSTREAM *inp_file,
                                            char *pStrErr );
int MolfileV3000ReadAtomsBlock( MOL_FMT_CTAB* ctab,
                                INCHI_IOSTREAM *inp_file,
                                int err,
                                char *pStrErr );
int MolfileV3000ReadBondsBlock( MOL_FMT_CTAB* ctab,
                                INCHI_IOSTREAM *inp_file,
                                int err,
                                char *pStrErr );
int MolfileV3000ReadTailOfCTAB( MOL_FMT_CTAB* ctab,
                                INCHI_IOSTREAM *inp_file,
                                int err,
                                char *pStrErr );
int MolfileV3000ReadHapticBond( MOL_FMT_CTAB* ctab,
                            char** line_ptr,
                            int **num_list,
                            char *pStrErr );
int MolfileV3000ReadStereoCollection( MOL_FMT_CTAB* ctab,
                                      char** line_ptr,
                                      int **num_list,
                                      char *pStrErr );
int MolfileV3000ReadSGroup( MOL_FMT_CTAB* ctab,
                            INCHI_IOSTREAM *inp_file,
                            int err,
                            char *pStrErr );
int MolfileV3000Read3DBlock( MOL_FMT_CTAB* ctab,
                             INCHI_IOSTREAM *inp_file,
                             int err,
                             char *pStrErr );
int MolfileV3000ReadCollections( MOL_FMT_CTAB* ctab,
                                INCHI_IOSTREAM *inp_file,
                                int err,
                                char *pStrErr );
/*    Clean V3000 stuff */
int DeleteMolfileV3000Info( MOL_FMT_v3000* v3000 ) ;

char* inchi_fgetsLf_V3000( char* line, INCHI_IOSTREAM* inp_stream );
int get_V3000_input_line_to_strbuf( INCHI_IOSTREAM_STRING *buf, INCHI_IOSTREAM* inp_stream );

/*    Extract the 'data' in specified mol file field at given text position 'line_ptr'  */
int MolfileV3000ReadField( void* data, int data_type, char** line_ptr );
/*    Read keyword */
int MolfileV3000ReadKeyword(char* key, char** line_ptr);

/*
    SDF
*/

int SDFileSkipExtraData( INCHI_IOSTREAM *inp_file,
                         long *CAS_num,
                         char* comment,
                         int lcomment,
                         char *name,
                         int lname,
                         int prev_err,
                         const char *pSdfLabel,
                         char *pSdfValue,
                         char *pStrErr );
int SDFileIdentifyLabel( char* inp_line, const char *pSdfLabel );
long SDFileExtractCASNo( char *line );

#endif    /* _MOL_FMT_H_ */
