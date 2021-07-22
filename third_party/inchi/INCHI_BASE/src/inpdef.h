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


#ifndef _INPDEF_H_
#define _INPDEF_H_


/* input/output format */


#include "mode.h"
#include "ichidrp.h"
#include "mol_fmt.h"


#define CLOSING_STARRED_SRU_IS_A_MUST 1
#define ALLOW_CLOSING_SRU_VIA_HIGHER_ORDER_BOND 1
#define ALLOW_CLOSING_SRU_VIA_DIRADICAL 1

#define CLOSING_SRU_NOT_APPLICABLE 0
#define CLOSING_SRU_RING 1
#define CLOSING_SRU_HIGHER_ORDER_BOND 2
#define CLOSING_SRU_DIRADICAL 3
#define CLOSING_SRU_RING_OPENED 4
#define CLOSING_SRU_MOVED_BRACKETS 5

#define bDrawingLabelLeftShift endpoint    /* for drawing only */
typedef S_SHORT ST_CAP_FLOW;

/* inp_ATOM::at_type */
#define ATT_NONE         0x0000
#define ATT_ACIDIC_CO    0x0001
#define ATT_ACIDIC_S     0x0002
#define ATT_OO           0x0004
#define ATT_ZOO          0x0008
#define ATT_NO           0x0010
#define ATT_N_O          0x0020
#define ATT_ATOM_N       0x0040
#define ATT_ATOM_P       0x0080
#define ATT_OTHER_NEG_O  0x0100
#define ATT_OTHER_ZO     0x0200   /* -Z=O or =Z=O */
#define ATT_OH_MINUS     0x0400   /* OH(-), O=O,S,Se,Te */
#define ATT_O_PLUS       0x0800   /* -OH2(+), =OH(+), -OH(+)-, OH3(+), =O(+)-, etc; O=O,S,Se,Te */
#define ATT_PROTON       0x1000
#define ATT_HalAnion     0x2000
#define ATT_HalAcid      0x4000
#if ( FIX_NP_MINUS_BUG == 1 )
#define ATT_NP_MINUS_V23 0x8000   /* =N(-) or =P(-) where = previously was triple */
#endif

#define AT_FLAG_ISO_H_POINT 0x01  /* may have isotopic H */

#define PERIODIC_NUMBER_H  1

#ifndef NUMH
#define NUM_ISO_H(AT,N) (AT[N].num_iso_H[0]+AT[N].num_iso_H[1]+AT[N].num_iso_H[2])
#define NUMH(AT,N)     (AT[N].num_H+NUM_ISO_H(AT,N))
#endif

#define FlagSC_0D  1  /* bUsed0DParity */
#define FlagSB_0D  2  /* bUsed0DParity */

#define SB_PARITY_FLAG  0x38 /* mask for disconnected metal parity if it is different */
#define SB_PARITY_SHFT  3    /* number of right shift bits to get disconnected metal parity */
#define SB_PARITY_MASK  0x07
#define SB_PARITY_1(X) (X & SB_PARITY_MASK)  /* refers to connected structure */
#define SB_PARITY_2(X) (((X) >> SB_PARITY_SHFT) & SB_PARITY_MASK) /* refers to connected structure */



typedef struct tagInputAtom
{
    char          elname[ATOM_EL_LEN];      /* name of chemical element                                 */
    U_CHAR        el_number;                /* number of the element in the Periodic Table              */
    AT_NUMB       neighbor[MAXVAL];         /* positions (from 0) of the neighbors in the inp_ATOM array*/
    AT_NUMB       orig_at_number;           /* original atom number                                     */
    AT_NUMB       orig_compt_at_numb;       /* original atom number within the component                */
                                            /* before terminal H removal                                */
    S_CHAR        bond_stereo[MAXVAL];      /* 1=Up,4=Either,6=Down; this atom is at the pointing wedge,*/
                                            /*   negative => on the opposite side; 3=Either double bond */
    U_CHAR        bond_type[MAXVAL];        /* 1..4; 4="aromatic", should be discouraged on input       */
    S_CHAR        valence;                  /* for most chemists, it is coordination number, CN;        */
                                            /* number of bonds = number of neighbors					*/
    S_CHAR        chem_bonds_valence;       /* for most chemists, it is what usually called valence;    */
                                            /* sum of bond types (type 4 needs special treatment)       */
    S_CHAR        num_H;                    /* number of implicit hydrogens, including D and T          */
    S_CHAR        num_iso_H[NUM_H_ISOTOPES]; /* number of implicit 1H, 2H(D), 3H(T) < 16                */
    S_CHAR        iso_atw_diff;             /* =0 => natural isotopic abundances                        */
                                            /* >0 => (mass) - (mass of the most abundant isotope) + 1   */
                                            /* <0 => (mass) - (mass of the most abundant isotope)       */
    S_CHAR        charge;                   /* charge                                                   */
    S_CHAR        radical;                  /* RADICAL_SINGLET, RADICAL_DOUBLET, or RADICAL_TRIPLET     */
    S_CHAR        bAmbiguousStereo;
    S_CHAR        cFlags;                   /* AT_FLAG_ISO_H_POINT                                      */
    AT_NUMB       at_type;                  /* ATT_NONE, ATT_ACIDIC                                     */
    AT_NUMB       component;                /* number of the structure component > 0                    */
    AT_NUMB       endpoint;                 /* id of a tautomeric group                                 */
    AT_NUMB       c_point;                  /* id of a positive charge group                            */
    double        x;
    double        y;
    double        z;
    /* 0D parities (originally were used with CML) */
    S_CHAR        bUsed0DParity;            /* bit=1 => stereobond; bit=2 => stereocenter               */
    /* 0D tetrahedral parity */
    S_CHAR        p_parity;
    AT_NUMB       p_orig_at_num[MAX_NUM_STEREO_ATOM_NEIGH];
    /* 0D bond parities */
    S_CHAR        sb_ord[MAX_NUM_STEREO_BONDS]; /* stereo bond/neighbor ordering number, starts from 0  */
    /* neighbors on both sides of stereobond have same sign=> trans/T/E, diff. signs => cis/C/Z         */
    S_CHAR        sn_ord[MAX_NUM_STEREO_BONDS]; /* ord. num. of the neighbor adjacent to the SB;        */
                                                /* starts from 0;                                       */
                                                /* -1 means removed explicit H                          */
    /* neighbors on both sides of stereobond have same parity => trans/T/E/2, diff. parities => cis/C/Z/1 */
    S_CHAR        sb_parity[MAX_NUM_STEREO_BONDS];
    AT_NUMB       sn_orig_at_num[MAX_NUM_STEREO_BONDS]; /* orig. at number of sn_ord[] neighbors        */

#if ( FIND_RING_SYSTEMS == 1 )
    S_CHAR  bCutVertex;
    AT_NUMB nRingSystem;
    AT_NUMB nNumAtInRingSystem;
    AT_NUMB nBlockSystem;

#if ( FIND_RINS_SYSTEMS_DISTANCES == 1 )
    AT_NUMB nDistanceFromTerminal;       /* terminal atom or ring system has 1, next has 2, etc. */
#endif

#endif
} inp_ATOM;



/*  v. 1.05 extensions: extended input supporting V3000; polymers   */
/*  OAD stands for OrigAtData                                       */

/* v. 1.05 Polymer stuff                                            */

/* Polymer representation type */
#define NO_POLYMER -1
#define POLYMER_REPRESENTATION_SOURCE_BASED     1
#define POLYMER_REPRESENTATION_STRUCTURE_BASED  2
#define POLYMER_REPRESENTATION_MIXED            3
#define POLYMER_REPRESENTATION_UNRECOGNIZED     4

#define ALLOW_MIXED_SRU_AND_MON                 1 /* allow simultaneous presence of source-based and    */
                                                  /* structure-based units in embedding copolymer unit    */

#define POLYMER_STY_NON     0
#define POLYMER_STY_SRU     1   /* structure-based unit or copolymer subunit                        */
#define POLYMER_STY_MON     2   /* source-based    polymer unit or copolymer subunit                    */
#define POLYMER_STY_COP     3   /* copolymer unit embedding >1 subunits                                */
                                /* (may be SRU, MON, MER, CRO, MOD; others not supported yet)        */
#define POLYMER_STY_MOD     4   /* copolymer subunit only, designates chemical modification of SRU    */
#define POLYMER_STY_CRO     5   /* copolymer subunit only, designates cross-linked version of SRU    */
#define POLYMER_STY_MER     6   /* copolymer subunit only, source-based with no homopolymerize        */

#define POLYMER_SST_NON     0
#define POLYMER_SST_ALT     1
#define POLYMER_SST_RAN     2
#define POLYMER_SST_BLK     3

#define POLYMER_CONN_NON    0
#define POLYMER_CONN_HT     1
#define POLYMER_CONN_HH     2
#define POLYMER_CONN_EU     3


typedef struct OAD_PolymerUnit
{
    int     id;             /* it is what is called 'Sgroup number' in CTFILE           */
    int     type;           /* type as by MDL format (STY)                              */
    int     subtype;        /* subtype as by MDL format (SST)                           */
    int     conn;           /* connection scheme  as by MDL format (SCN)                */
    int     label;          /* it is what is called 'unique Sgroup identifier' CTFILE   */
    int     na;             /* number of atoms in the unit                              */
    int     nb;             /* number of bonds in the unit                              */
    int     cyclizable;     /* =1 if frame shift via CRU 'cyclization' is applicable    */
    int     cyclized;       /* =1 i CRU alredy was frame_shift' treated                 */
    double  xbr1[4];        /* bracket ends coordinates (SDI)                           */
    double  xbr2[4];        /* bracket ends coordinates (SDI)                           */
    char    smt[80];        /* Sgroup Subscript (SMT)                                   */
    int     representation;
    /* CRU structure is:            cap1-[-end_atom1/\/\/\end_atom2-]-cap2              */
    int     cap1;
    int     end_atom1;
    int     end_atom2;
    int     cap2;
    int     cap1_is_undef;
    int     cap2_is_undef;
    int     *alist;         /* list of atoms in the unit (SAL)                          */
    int     *blist;         /* bonds in the unit as list [atom1, atom2; atom1, atom2,..]*/
                            /* for crossing bonds (S)                                   */
    int     maxbkbonds;		/* max (allocd) number of frame_shift involved bonds		*/
    int     nbkbonds;       /* number of bkbonds in CRU main chain						*/
                            /* bkbonds are [breakable at possible frame shift] backbone */
                            /* bonds in between "left" end_atom1 and "right" end_atom2  */
                            /* ends (they may be only single-order non-intraring bonds) */
    int     **bkbonds;		/* list of [breakable] backbone bonds [(a1,a2), (a3,a4),...}*/
}  OAD_PolymerUnit;


typedef struct OAD_Polymer
{
    OAD_PolymerUnit **units;                /* array of pointers to units               */
    int             n;
    int             n_pzz;                  /* number of polymeric Zz atoms             */
                                            /* paired undefined-nature caps/stars around*/
                                            /* CRY's (must be even)                     */
    int             *pzz;                   /* polymeric Zz atoms                       */
    int             really_do_frame_shift;
    int             frame_shift_scheme;     /* frame shift analysis scheme              */
    int             treat;
    int             representation;
    int             is_in_reconn;
    int             edit_repeats;           /*  -1 unknown, to be checked               */
                                            /*   0 no, no edits required                */
                                            /*   1 yes, edits are necessary             */
} OAD_Polymer;


typedef struct OAD_AtProps
{
    int erank;              /* rank of element; 2 - C, >2 - rank of heteroatom in chain,    */
                            /* O > S > Se > Te > N ...., Rule 4                             */
    int ring_erank;         /* 0 - not ring or just carbocycle,
                            /* >2 - rank of senior heteroatom in this cycle                 */
                            /* according to Rule 2 ( N > O >... )                           */
    int ring_num;
    int ring_size;          /* 0 or ring system size                                        */
                            /* that is:                                                     */
                            /* ring_erank != 0    heterocycle of ring_size                  */
                            /* ring_erank==0 && ring_size>0    carbocycle of ring_size      */
} OAD_AtProps;

/* Extended input supports v. 1.05 extensions: V3000; polymers  */
typedef struct OAD_V3000
{
    int n_non_star_atoms;
    int n_star_atoms;
    int *atom_index_orig;   /* index as supplied for atoms                                  */
    int *atom_index_fin;    /* = index or -1 for star atom                                  */
    int n_sgroups;          /* currently, we do not use this.                               */
    int n_3d_constraints;   /* currently, we do not use this.                               */
    int n_collections;
    int n_non_haptic_bonds;
    int n_haptic_bonds;
    int **lists_haptic_bonds;
                            /* haptic_bonds[i] is pointer to int* which contains:           */
                            /* bond_type, non-star atom number, nendpts,                    */
                            /* then endpts themselves                                       */
    /* Enhanced stereo */
    int n_steabs;
    int **lists_steabs;     /* steabs[k][0] - not used                                      */
                            /* steabs[k][1] -  number of members in collection              */
                            /* steabs[k][2..] - member atom numbers                         */
    int n_sterel;
    int **lists_sterel;     /* sterel[k][0] - n from "STERELn" tag                          */
                            /* sterel[k][1] -  number of members in collection              */
                            /* sterel[k][2..] - member atom numbers                         */
    int n_sterac;
    int **lists_sterac;     /* sterac[k][0] - n from "STERACn" tag                          */
                            /* sterac[k][1] -  number of members in collection              */
                            /* sterac[k][0] - number from "STERACn" tag                     */
} OAD_V3000;


typedef struct tagOrigAtom
{
    /* Initially filled out by CreateOrigInpDataFromMolfile()                               */
    /* may be changed by disconnecting salts and disconnecting metals                       */
    inp_ATOM    *at;
    int         num_dimensions;
    int         num_inp_bonds;
    int         num_inp_atoms;
    /* may be changed by disconnecting salts and disconnecting metals                       */
    int         num_components;     /* set by MarkDisconnectedComponents and disconn. metals*/
    int         bDisconnectSalts;   /* whether salt disconnection is possible               */
    int         bDisconnectCoord;   /* 0 if no disconnection needed                         */
                                    /* else (NumImplicitH to disconnect)+1                  */
#if ( bRELEASE_VERSION == 0 )
    int bExtract;
#endif
    AT_NUMB     *nCurAtLen;         /* has max_num_components elements                      */
    AT_NUMB     *nOldCompNumber;    /* 0 or component number in previous numbering          */
    int         nNumEquSets;        /* number of found component equivalence sets           */
    AT_NUMB     *nEquLabels;        /* num_inp_atoms elements, value>0 marks atoms          */
                                    /* in the set #value                                    */
    AT_NUMB     *nSortedOrder;      /* num_components elements, values = 1..num_components; */
                                    /* only if num_components > 1                           */
    int         bSavedInINCHI_LIB[INCHI_NUM];
    int         bPreprocessed[INCHI_NUM];
    MOL_COORD   *szCoord;
    /* v. 1.05 extensions                                                                   */
    OAD_Polymer *polymer;
    OAD_V3000   *v3000;
    int valid_polymer;
    int n_zy;                      /* number of non-polymeric pseudoatoms (Zy)             */

} ORIG_ATOM_DATA;


typedef struct tagOriginalStruct
{
    int         num_atoms;
    char        *szAtoms;
    char        *szBonds;
    char        *szCoord;
    /* v. 1.05 extensions                                                                   */
    OAD_Polymer *polymer;   /* uses pointer copy from orig_inp_data, do not free after use! */
    OAD_V3000   *v3000;     /* uses pointer copy from orig_inp_data, do not free after use! */
    int         n_zy;      /* number of non-polymeric pseudoatoms (Zy) */

} ORIG_STRUCT;



typedef struct tagAtomParmsForDrawing
{
    char      at_string[ATOM_INFO_LEN];
    int       DrawingLabelLeftShift;
    int       DrawingLabelLength;
    AT_NUMB   nCanonNbr;    /* if zero then do not use all data for the atom                */
    AT_NUMB   nCanonEquNbr;
    AT_NUMB   nTautGroupCanonNbr;
    AT_NUMB   nTautGroupEquNbr;
    S_CHAR    cFlags;       /* AT_FLAG_ISO_H_POINT                                          */
#ifdef DISPLAY_DEBUG_DATA
    int       nDebugData;
#endif
    S_CHAR    cHighlightTheAtom;
    S_CHAR    cStereoCenterParity;
    S_CHAR    cStereoBondParity[MAX_STEREO_BONDS];
    S_CHAR    cStereoBondWarning[MAX_STEREO_BONDS];
    S_CHAR    cStereoBondNumber[MAX_STEREO_BONDS];
} inf_ATOM;



#define INF_STEREO_ABS         0x0001
#define INF_STEREO_REL         0x0002
#define INF_STEREO_RAC         0x0004
#define INF_STEREO_NORM        0x0008
#define INF_STEREO_INV         0x0010
#define INF_STEREO             0x0020
#define INF_STEREO_ABS_REL_RAC (INF_STEREO_ABS | INF_STEREO_REL | INF_STEREO_RAC)
#define INF_STEREO_NORM_INV    (INF_STEREO_NORM | INF_STEREO_INV)

#define MAX_LEN_REMOVED_PROTONS 128



typedef struct tagInfoAtomData
{
    inf_ATOM  *at;
    int        num_at;
    AT_NUMB    StereoFlags;
    AT_NUMB    num_components;
    AT_NUMB    *pStereoFlags;

    int        nNumRemovedProtons;
    int        num_removed_iso_H;                   /* number of exchangable isotopic H     */
    NUM_H      num_iso_H[NUM_H_ISOTOPES];           /* number of exchangable isotopic H     */
    char       szRemovedProtons[MAX_LEN_REMOVED_PROTONS];
} INF_ATOM_DATA;



typedef struct tagInputAtomData
{
    inp_ATOM   *at;
    inp_ATOM   *at_fixed_bonds;                     /* tautomeric case, added or removed H  */
    int        num_at;
    int        num_removed_H;
    int        num_bonds;
    int        num_isotopic;
    int        bExists;
    int        bDeleted;                              
    int        bHasIsotopicLayer;
    int        bTautomeric;
    int        bTautPreprocessed;
    int        nNumRemovedProtons;
    NUM_H      nNumRemovedProtonsIsotopic[NUM_H_ISOTOPES];
                    /* isotopic composition of removed protons, not included in num_iso_H[] */
    NUM_H      num_iso_H[NUM_H_ISOTOPES];
                    /* isotopic H on tautomerIC atoms and those in nIsotopicEndpointAtomNumber */
    INCHI_MODE bTautFlags;
    INCHI_MODE bTautFlagsDone;
    INCHI_MODE bNormalizationFlags;
} INP_ATOM_DATA;

typedef INP_ATOM_DATA INP_ATOM_DATA2[TAUT_NUM];



typedef struct tagNormCanonFlags
{
    INCHI_MODE bTautFlags[INCHI_NUM][TAUT_NUM];
    INCHI_MODE bTautFlagsDone[INCHI_NUM][TAUT_NUM];
    INCHI_MODE bNormalizationFlags[INCHI_NUM][TAUT_NUM];
    int        nCanonFlags[INCHI_NUM][TAUT_NUM];
} NORM_CANON_FLAGS;



typedef struct tagCompositeAtomData
{
    inp_ATOM *at;
    int      num_at;
    int      num_removed_H;
    int      num_bonds;
    int      num_isotopic;
    int      bExists;
    int      bDeleted;    /* unused */
    int      bHasIsotopicLayer;
    int      bTautomeric;
    int      nNumRemovedProtons;
    NUM_H    nNumRemovedProtonsIsotopic[NUM_H_ISOTOPES];
                    /* isotopic composition of removed protons, not included in num_iso_H[] */
    NUM_H    num_iso_H[NUM_H_ISOTOPES];
                    /* isotopic H on tautomeric atoms and those                             */
                    /* in nIsotopicEndpointAtomNumber                                       */
    AT_NUMB  *nOffsetAtAndH;
    int      num_components;
} COMP_ATOM_DATA;

/*
typedef COMP_ATOM_DATA COMP_ATOM_DATA3[TAUT_NUM+1];
*/

#define ADD_LEN_STRUCT_FPTRS 100  /* allocation increments                                  */

typedef long INCHI_FPTR;



typedef struct tagStructFptrs
{
    INCHI_FPTR *fptr;       /* input:  fptr[cur_fptr]  =file ptr to the struct to read      */
                            /* output: fptr[cur_fptr+1]=file ptr to the next struct or EOF  */
    int        len_fptr;    /* allocated length of fptr                                     */
    int        cur_fptr;    /* input: k-1 to read the kth struct, k = 1, 2, 3,...;          */
                            /* left unchanged; struct number := cur_fptr+1                  */
    int        max_fptr;    /* length of the filled out portion of fptr                     */
} STRUCT_FPTRS;

/* forward declaration */
struct tagCANON_GLOBALS;

#ifndef __ICHITIME_H__
struct tagInchiTime;
struct tagINCHI_CLOCK;
int bInchiTimeIsOver( struct tagINCHI_CLOCK *ic,
                      struct tagInchiTime   *TickEnd );
#endif


#define FLAG_INP_AT_CHIRAL        1
#define FLAG_INP_AT_NONCHIRAL     2
#define FLAG_SET_INP_AT_CHIRAL    4
#define FLAG_SET_INP_AT_NONCHIRAL 8
#define FLAG_SET_INP_LARGE_MOLS   16

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif


int CreateOrigInpDataFromMolfile( INCHI_IOSTREAM  *inp_file,
                                    ORIG_ATOM_DATA *orig_at_data,
                                    int            bMergeAllInputStructures,
                                    int            bGetOrigCoord,
                                    int            bDoNotAddH,
                                    int			   treat_polymers,
                                    int			   treat_NPZz,
                                    const char     *pSdfLabel,
                                    char           *pSdfValue,
                                    unsigned long   *lSdfId,
                                    long           *lMolfileNumber,
                                    INCHI_MODE     *pInpAtomFlags,
                                    int            *err,
                                    char           *pStrErr,
                                    int            bNoWarnings );
int InchiToOrigAtom( INCHI_IOSTREAM *infile,
                        ORIG_ATOM_DATA *orig_at_data,
                        int            bMergeAllInputStructures,
                        int            bGetOrigCoord,
                        int            bDoNotAddH,
                        int            vABParityUnknown,
                        INPUT_TYPE     nInputType,
                        char           *pSdfLabel,
                        char           *pSdfValue,
                        unsigned long   *lSdfId,
                        INCHI_MODE     *pInpAtomFlags,
                        int            *err,
                        char           *pStrErr );
int MarkDisconnectedComponents( ORIG_ATOM_DATA  *orig_at_data,
                                int             bProcessOldCompNumbers );
int DisconnectSalts( ORIG_ATOM_DATA *orig_inp_data,
                        int            bDisconnect );
int DisconnectMetals( ORIG_ATOM_DATA *orig_inp_data,
                        int            bCheckMetalValence,
                        INCHI_MODE     *bTautFlagsDone );
int bMayDisconnectMetals( ORIG_ATOM_DATA *orig_inp_data,
                            int            bCheckMetalValence,
                            INCHI_MODE     *bTautFlagsDone );
int bHasMetalAtom( ORIG_ATOM_DATA   *orig_inp_data );
int FixAdjacentRadicals( int        num_inp_atoms,
                            inp_ATOM   *at ); /* FIX_ADJ_RAD == 1 */
int fix_odd_things( int         num_atoms,
                    inp_ATOM    *at,
                    int         bFixBug,
                    int         bFixNonUniformDraw );
int post_fix_odd_things( int      num_atoms,
                            inp_ATOM *at );
int remove_ion_pairs( int      num_atoms,
                        inp_ATOM *at );
int bFoundFeature( inp_ATOM *at,
                    int      num_atoms );

int UnMarkRingSystemsInp( inp_ATOM *at, int num_atoms );

int OrigAtData_WriteToSDfile( const ORIG_ATOM_DATA  *inp_at_data, INCHI_IOSTREAM *fcb,
                                const char *name, const char *comment,
                                int bChiralFlag, int bAtomsDT,
                                const char *szLabel, const char *szValue );
void FreeInpAtom( inp_ATOM **at );
void FreeInfAtom( inf_ATOM **at );
void FreeOrigAtData( ORIG_ATOM_DATA *orig_at_data );
void FreeExtOrigAtData( OAD_Polymer *pd, OAD_V3000 *v3k );
void FreeInpAtomData( INP_ATOM_DATA *inp_at_data );
void FreeCompAtomData( COMP_ATOM_DATA *inp_at_data );
void FreeInfoAtomData( INF_ATOM_DATA *inf_at_data );


/*
    ORIG_ATOM_DATA
        functions
*/


/* OAD_Edit */
typedef struct tagOAD_StructureEdits
{
    INT_ARRAY *del_atom;
    INT_ARRAY *del_bond;
    INT_ARRAY *new_bond;
    INT_ARRAY *mod_bond;
    INT_ARRAY *mod_coord;
    int del_side_chains;
} OAD_StructureEdits;
int OAD_StructureEdits_Init(OAD_StructureEdits *ed);
void OAD_StructureEdits_Clear(OAD_StructureEdits *ed);
void OAD_StructureEdits_DebugPrint(OAD_StructureEdits *ed);




#if ( RING2CHAIN == 1 )
int Ring2Chain( struct tagINCHI_CLOCK *ic, struct tagCANON_GLOBALS *pCG, ORIG_ATOM_DATA *orig_inp_data );
#endif

#if ( UNDERIVATIZE == 1 )
int OAD_Edit_Underivatize( struct tagINCHI_CLOCK *ic,
                           struct tagCANON_GLOBALS *pCG,
                           ORIG_ATOM_DATA *orig_inp_data,
                           int bOutputSdf,
                           int bOutputReport,
                           char *pSdfValue );

void OAD_Edit_MergeComponentsAndRecreateOAD( ORIG_ATOM_DATA *orig_OrigAtomData,
                                             INP_ATOM_DATA *curr_InpAtomData,
                                             int num_components,
                                             int *errcode );

#endif


/* OAD misc. */


int  OAD_ValidatePolymerAndPseudoElementData( ORIG_ATOM_DATA *orig_at_data,
                                              int treat_polymers,
                                              int bNPZz,
                                              char *pStrErr,
                                              int bNoWarnings );


/* OAD_Polymer */


void OAD_ValidateAndSortOutPseudoElementAtoms( ORIG_ATOM_DATA *orig_at_data,
                                   int treat_polymers,
                                   int use_zz,
                                   int *err,
                                   char *pStrErr );
int  OAD_Polymer_GetRepresentation( OAD_Polymer *p );
int  OAD_Polymer_CyclizeCloseableUnits( ORIG_ATOM_DATA *orig_at_data,
                                        int use_zz,
                                        char *pStrErr,
                                        int bNoWarnings );
int  OAD_Polymer_FindRingSystems( OAD_Polymer *pd,
                                    inp_ATOM    *at,
                                    int         nat,
                                    int         *num_inp_bonds,
                                    int         *num_ring_sys,
                                    int         *size_ring_sys,
                                    int         start );
void OAD_Polymer_SetAtProps( OAD_Polymer *pd,
                                inp_ATOM    *at,
                                int         nat,
                                int         *num_inp_bonds,
                                OAD_AtProps *aprops,
                                int         *cano_nums );
int OAD_Polymer_CompareBackboneBondsSeniority( int         *b1,
                                               int         *b2,
                                               OAD_AtProps *aprops );
int OAD_Polymer_CompareRanksOfTwoAtoms( int         atom1,
                                        int         atom2,
                                        OAD_AtProps *aprops );
int OAD_Polymer_IsFirstAtomRankLower( int atom1, int atom2, OAD_AtProps *aprops );
int OAD_PolymerUnit_SetReopeningDetails( OAD_PolymerUnit *u, inp_ATOM *at );
void OAD_PolymerUnit_SortBackboneBondsAndSetSeniors( OAD_PolymerUnit *u, inp_ATOM *at,
                                                OAD_AtProps *aprops, int *senior_bond );
void OAD_Polymer_FindBackbones( ORIG_ATOM_DATA *where_to_look,
                                COMP_ATOM_DATA *composite_norm_data,
                                int *err, char *pStrErr );
int  OAD_Polymer_PrepareWorkingSet( OAD_Polymer *p, int *cano_nums, int *compnt_nums,
                                    OAD_PolymerUnit **units2, int *unum );
void OAD_Polymer_Free( OAD_Polymer *p );
void OAD_Polymer_DebugTrace( OAD_Polymer *p );
void OAD_Polymer_SmartReopenCyclizedUnits( OAD_Polymer *p, inp_ATOM *at,
                                            int nat, int *num_inp_bonds );
int  OAD_Polymer_PrepareFoldCRUEdits( ORIG_ATOM_DATA *orig_at_data,
                                      char *sinchi_noedits, 
                                      char *saux_noedits,
                                      char *sinchi,
                                      char *saux,
                                      OAD_StructureEdits *ed);
int  OAD_Polymer_PrepareFrameShiftEdits( ORIG_ATOM_DATA *orig_at_data,
                                         char *sinchi,
                                         char *saux,
                                         OAD_StructureEdits *ed);
    
/* OAD_PolymerUnit */
OAD_PolymerUnit * OAD_PolymerUnit_New( int       maxatoms,
                                        int       maxbonds,
                                        int       id,
                                        int       label,
                                        int       type,
                                        int       subtype,
                                        int       conn,
                                        char      *smt,
                                        int       na,
                                        INT_ARRAY *alist,
                                        int       nb,
                                        INT_ARRAY *blist,
                                        int       nbkbonds,
                                        int       **bkbonds );
OAD_PolymerUnit * OAD_PolymerUnit_CreateCopy( OAD_PolymerUnit *u );
void OAD_PolymerUnit_Free( OAD_PolymerUnit *unit );
void OAD_PolymerUnit_DebugTrace( OAD_PolymerUnit *unit );
void OAD_PolymerUnit_FindEndsAndCaps( OAD_PolymerUnit *unit,
                                      ORIG_ATOM_DATA  *orig_at_data,
                                      int *end1, int *cap1, int *cap1_is_undef,
                                      int *end2, int *cap2, int *cap2_is_undef,
                                      int *err, char *pStrErr);
void OAD_PolymerUnit_SetEndsAndCaps( OAD_PolymerUnit *unit,
                                     ORIG_ATOM_DATA  *orig_at_data,
                                     int             *err,
                                     char            *pStrErr );
void OAD_PolymerUnit_UnlinkCapsAndConnectEndAtoms( OAD_PolymerUnit *unit,
                                                   ORIG_ATOM_DATA  *orig_at_data,
                                                   int             *err,
                                                   char            *pStrErr );
void OAD_PolymerUnit_PrepareToFrameShift( OAD_PolymerUnit *unit,
                                            ORIG_ATOM_DATA  *orig_at_data,
                                            int             *err,
                                            char            *pStrErr );

int OAD_CollectReachableAtoms( ORIG_ATOM_DATA  *orig_at_data,
                               int start_atom,
                               int nforbidden_bonds,
                               int *forbidden_bonds,
                               int *n_reachable,
                               int *reachable,
                               int *err,
                               char *pStrErr);

void OAD_CollectBackboneAtoms(ORIG_ATOM_DATA  *at_data,
                              int na,
                              int *alist,
                              int end_atom1,
                              int end_atom2,
                              int *nbkatoms,
                              int *bkatoms,
                              int *err,
                              char *pStrErr);

void OAD_CollectBackboneBonds( ORIG_ATOM_DATA  *at_data,
                               int na,
                               int *alist,
                               int end_atom1,
                               int end_atom2,
                               int *nbkbonds,
                               int **bkbonds,
                               int *err,
                               char *pStrErr);
void OAD_PolymerUnit_DelistIntraRingBackboneBonds( OAD_PolymerUnit *unit,
                                                   ORIG_ATOM_DATA  *orig_at_data,
                                                   int             *err,
                                                   char            *pStrErr );
void OAD_PolymerUnit_DelistHighOrderBackboneBonds( OAD_PolymerUnit *unit,
                                                   ORIG_ATOM_DATA  *orig_at_data,
                                                   COMP_ATOM_DATA  *composite_norm_data,
                                                   int             *err,
                                                   char            *pStrErr );
void OAD_PolymerUnit_SortBackboneBonds( OAD_PolymerUnit *u,
                                        OAD_AtProps     *aprops,
                                        int             *bnum );

void OAD_PolymerUnit_ReopenCyclized( OAD_PolymerUnit *u,
                                        inp_ATOM        *at,
                                        OAD_AtProps     *aprops,
                                        int             nat,
                                        int             *num_inp_bonds );

int OAD_PolymerUnit_CompareAtomLists( OAD_PolymerUnit* u1,
                                        OAD_PolymerUnit* u2 );
int OAD_PolymerUnit_CompareAtomListsMod( OAD_PolymerUnit* u1,
                                            OAD_PolymerUnit* u2 );
int OAD_PolymerUnit_OrderBondAtomsAndBondsThemselves( OAD_PolymerUnit *u,
                                                        int             n_stars,
                                                        int             *stars );
int OAD_PolymerUnit_HasMetal( OAD_PolymerUnit *u,
                                inp_ATOM        *at );


int FixUnkn0DStereoBonds( inp_ATOM *at,
                            int num_at );
inf_ATOM *CreateInfAtom( int num_atoms );
inp_ATOM *CreateInpAtom( int num_atoms );
int CreateInfoAtomData( INF_ATOM_DATA *inf_at_data,
                        int           num_atoms,
                        int           num_components );
int AllocateInfoAtomData( INF_ATOM_DATA *inf_at_data,
                            int            num_atoms,
                            int            num_components );
int DuplicateInfoAtomData( INF_ATOM_DATA       *inf_at_data_to,
                            const INF_ATOM_DATA *inf_at_data_from );
int CreateInpAtomData( INP_ATOM_DATA *inp_at_data,
                        int           num_atoms,
                        int           create_at_fixed_bonds );
int CreateCompAtomData( COMP_ATOM_DATA *inp_at_data,
                        int            num_atoms,
                        int            num_components,
                        int            bIntermediateTaut );


#ifndef COMPILE_ANSI_ONLY
int DisplayInputStructure( char          *szOutputString,
                            inp_ATOM      *at,
                            INF_ATOM_DATA *inf_at_data,
                            int           num_at,
                            DRAW_PARMS    *dp );
#endif


void PrintFileName( const char *fmt,
                    FILE       *out_file,
                    const char *szFname );

void MySleep( unsigned long ms );

int ReconcileAllCmlBondParities( inp_ATOM *at,
                                 int      num_atoms,
                                 int      bDisconnected );

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif    /* _INPDEF_H_ */
