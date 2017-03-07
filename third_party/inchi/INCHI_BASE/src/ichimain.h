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


#ifndef _ICHIMAIN_H_
#define _ICHIMAIN_H_

#include "strutil.h"
#include "ichicomn.h"

#define ESC_KEY       27
#define INCHI_SEGM_BUFLEN  511999

/* for DisplayTheWholeStructure() */
#define COMP_ORIG_0_MAIN  0x0001
#define COMP_ORIG_0_RECN  0x0002
#define COMP_PREP_0_MAIN  0x0004
#define COMP_PREP_0_RECN  0x0008
#define COMP_ORIG_1_MAIN  0x0010
#define COMP_ORIG_1_RECN  0x0020


typedef struct tagStructData
{
    unsigned long ulStructTime;
    int           nErrorCode;
    int           nErrorType;
    int           nStructReadError;
    char          pStrErrStruct[STR_ERR_LEN];
    long          fPtrStart;  /* or number of processed structures */
    long          fPtrEnd;    /* or number of errors */
    int           bUserQuit;
    int           bUserQuitComponent;
    int           bUserQuitComponentDisplay;
    int           bChiralFlag;

    /* information related to normal or disconnected layers */
    int           num_taut[INCHI_NUM];
    int           num_non_taut[INCHI_NUM];
    INCHI_MODE     bTautFlags[INCHI_NUM];        /* reconnected does not have TG_FLAG_DISCONNECT_COORD_DONE flag */
    INCHI_MODE     bTautFlagsDone[INCHI_NUM];    /* reconnected does not have TG_FLAG_DISCONNECT_COORD_DONE flag */
    int           num_components[INCHI_NUM];    /* number of allocated INChI, INChI_Aux data structures */
    /* debugging info */

#if ( bRELEASE_VERSION == 0 )
    int           bExtract;
#endif
} STRUCT_DATA;


#define PRINT_INCHI_MAX_TAG_LEN 64

/* Convenience storage for InChI serialization control data */
typedef struct OutputINCHI_CtlData
{
    int ATOM_MODE;
    int TAUT_MODE;

    int *pSortPrintINChIFlags;

    int bOverflow;
    int bAlways;
    int bOutputType;
    int bOutType;
    int bPlainTextTags;
    int bOmitRepetitions;
    int bUseMulipliers;
    int bNonTautNonIsoIdentifierNotEmpty;
    int bNonTautIsoIdentifierNotEmpty;
    int bSecondNonTautPass;
    int bTautomericOutputAllowed;
    int bTautomeric;
    int bNonTautomeric;
    int bNonTautIsIdenticalToTaut;
    int bFhTag;
    int bRelRac;
    int bAbcNumbers;
    int bIsotopic;

    int iCurTautMode;

    int num_components;
    int  nNumRemovedProtons;
    int nTag;
    int bTag1;
    int bTag2;
    int bTag3;
    int tot_len;
    int tot_len2;

    int nCurINChISegment;
    int nSegmAction;

    int num_comp[TAUT_NUM];
    int num_iso_H[NUM_H_ISOTOPES];
    int bAtomEqu[TAUT_NUM];
    int bTautEqu[TAUT_NUM];
    int    bInvStereo[TAUT_NUM];
    int bInvStereoOrigNumb[TAUT_NUM];
    int bRacemicStereo[TAUT_NUM];
    int bRelativeStereo[TAUT_NUM];
    int bIsotopicOrigNumb[TAUT_NUM];
    int bIsotopicAtomEqu[TAUT_NUM];
    int bIsotopicTautEqu[TAUT_NUM];
    int bInvIsotopicStereo[TAUT_NUM];
    int bInvIsotopicStereoOrigNumb[TAUT_NUM];
    int bIsotopicRacemicStereo[TAUT_NUM];
    int bIsotopicRelativeStereo[TAUT_NUM];
    int bIgn_UU_Sp3[TAUT_NUM];
    int bIgn_UU_Sp2[TAUT_NUM];
    int bIgn_UU_Sp3_Iso[TAUT_NUM];
    int bIgn_UU_Sp2_Iso[TAUT_NUM];
    int bChargesRadVal[TAUT_NUM];
    int bOrigCoord[TAUT_NUM];

    char sDifSegs[DIFL_LENGTH][DIFS_LENGTH];
    char szTag1[PRINT_INCHI_MAX_TAG_LEN];
    char szTag2[PRINT_INCHI_MAX_TAG_LEN];
    char szTag3[PRINT_INCHI_MAX_TAG_LEN];

    INCHI_SORT   **pINChISortTautAndNonTaut;
    INCHI_SORT   *pINChISort;
    INCHI_SORT   *pINChISort2;
}
OutputINCHI_CtlData;


#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif


int ProcessSingleInputFile( int argc, char *argv[ ] );
int ProcessMultipleInputFiles( int argc, char *argv[ ] );
int ReadCommandLineParms( int argc, const char *argv[],
                          INPUT_PARMS *ip,
                          char *szSdfDataValue,
                          unsigned long *ulDisplTime,
                          int bReleaseVersion,
                          INCHI_IOSTREAM *log_file);
void HelpCommandLineParms( INCHI_IOSTREAM *f);
int OpenFiles( FILE **inp_file,
               FILE **out_file,
               FILE **log_file,
               FILE **prb_file,
               INPUT_PARMS *ip );
int PrintInputParms( INCHI_IOSTREAM *log_file,
                     INPUT_PARMS *ip);
int SortAndPrintINChI( struct tagCANON_GLOBALS *pCG,
                       INCHI_IOSTREAM *out_file,
                       INCHI_IOSTREAM_STRING *strbuf,
                       INCHI_IOSTREAM *log_file,
                       INPUT_PARMS *ip,
                       ORIG_ATOM_DATA *orig_inp_data,
                       ORIG_ATOM_DATA *prep_inp_data,
                       COMP_ATOM_DATA composite_norm_data[INCHI_NUM][TAUT_NUM+1],
                       ORIG_STRUCT *pOrigStruct,
                       int num_components[INCHI_NUM],
                       int num_non_taut[INCHI_NUM],
                       int num_taut[INCHI_NUM],
                       INCHI_MODE bTautFlags[INCHI_NUM],
                       INCHI_MODE bTautFlagsDone[INCHI_NUM],
                       NORM_CANON_FLAGS *pncFlags,
                       long num_inp,
                       PINChI2 *pINChI[INCHI_NUM],
                       PINChI_Aux2 *pINChI_Aux[INCHI_NUM],
                       int *pSortPrintINChIFlags,
                       unsigned char save_opt_bits);
void FreeAllINChIArrays( PINChI2 *pINChI[INCHI_NUM],
                         PINChI_Aux2 *pINChI_Aux[INCHI_NUM],
                         int num_components[2]);
void FreeINChIArrays( PINChI2 *pINChI,
                      PINChI_Aux2 *pINChI_Aux,
                      int num_components );
void SplitTime( unsigned long ulTotalTime,
                int *hours,
                int *minutes,
                int *seconds,
                int *mseconds );
int ReadTheStructure( struct tagINCHI_CLOCK *ic,
                      STRUCT_DATA *sd,
                      INPUT_PARMS *ip,
                      INCHI_IOSTREAM *inp_file,
                      ORIG_ATOM_DATA *orig_inp_data,
                      int inp_index,
                      int *out_index );
int TreatErrorsInReadTheStructure( STRUCT_DATA *sd,
                                      INPUT_PARMS *ip,
                                      int nLogMask,
                                      INCHI_IOSTREAM *inp_file,
                                      INCHI_IOSTREAM *log_file,
                                      INCHI_IOSTREAM *out_file,
                                      INCHI_IOSTREAM *prb_file,
                                      ORIG_ATOM_DATA *orig_inp_data,
                                      long *num_inp );
int GetOneComponent( struct tagINCHI_CLOCK *ic,
                     STRUCT_DATA *sd,
                     INPUT_PARMS *ip,
                     INCHI_IOSTREAM *log_file,
                     INCHI_IOSTREAM *out_file,
                     INP_ATOM_DATA *inp_cur_data,
                     ORIG_ATOM_DATA *orig_inp_data,
                     int i, long num_inp );
int CreateOneComponentINChI( struct tagCANON_GLOBALS *pCG,
                             struct tagINCHI_CLOCK *ic,
                             STRUCT_DATA *sd,
                             INPUT_PARMS *ip,
                             INP_ATOM_DATA *inp_cur_data,
                             ORIG_ATOM_DATA *orig_inp_data,
                             PINChI2 *pINChI,
                             PINChI_Aux2 *pINChI_Aux,
                             int iINChI,
                             int i, long num_inp,
                             INP_ATOM_DATA **inp_norm_data,
                             NORM_CANON_FLAGS *pncFlags,
                             INCHI_IOSTREAM *log_file);
int TreatErrorsInCreateOneComponentINChI( STRUCT_DATA *sd,
                                          INPUT_PARMS *ip,
                                          ORIG_ATOM_DATA *orig_inp_data,
                                          int i, long num_inp,
                                          INCHI_IOSTREAM *inp_file,
                                          INCHI_IOSTREAM *log_file,
                                          INCHI_IOSTREAM *out_file,
                                          INCHI_IOSTREAM *prb_file );
int TreatCreateINChIWarning( STRUCT_DATA *sd,
                             INPUT_PARMS *ip,
                             ORIG_ATOM_DATA *orig_inp_data,
                             long num_inp,
                             INCHI_IOSTREAM *inp_file,
                             INCHI_IOSTREAM *log_file,
                             INCHI_IOSTREAM *out_file,
                             INCHI_IOSTREAM *prb_file );
int GetProcessingWarningsOneComponentInChI( INChI *cur_INChI[],
                                            INP_ATOM_DATA **inp_norm_data,
                                            STRUCT_DATA *sd);
#ifndef COMPILE_ANSI_ONLY
void eat_keyboard_input( void );
int user_quit( struct tagINCHI_CLOCK *ic, const char *msg, unsigned long ulMaxTime );
#endif
int GetOneStructure( struct tagINCHI_CLOCK *ic,
                     STRUCT_DATA *sd,
                     INPUT_PARMS *ip,
                     char *szTitle,
                     INCHI_IOSTREAM *inp_file,
                     INCHI_IOSTREAM *log_file,
                     INCHI_IOSTREAM *out_file,
                     INCHI_IOSTREAM *prb_file,
                     ORIG_ATOM_DATA *orig_inp_data,
                     long *num_inp,
                     STRUCT_FPTRS *struct_fptrs );
int ProcessOneStructure( struct tagINCHI_CLOCK *ic,
                         struct tagCANON_GLOBALS *pCG,
                         STRUCT_DATA *sd,
                         INPUT_PARMS *ip,
                         char *szTitle,
                         PINChI2 *pINChI2[INCHI_NUM],
                         PINChI_Aux2 *pINChI_Aux2[INCHI_NUM],
                         INCHI_IOSTREAM *inp_file,
                         INCHI_IOSTREAM *log_file,
                         INCHI_IOSTREAM *out_file,
                         INCHI_IOSTREAM *prb_file,
                         ORIG_ATOM_DATA *orig_inp_data,
                         ORIG_ATOM_DATA *prep_inp_data,
                         long num_inp,
                         INCHI_IOSTREAM_STRING *strbuf,
                         unsigned char save_opt_bits);
int ProcessOneStructureEx(    struct tagINCHI_CLOCK *ic,
                            struct tagCANON_GLOBALS *pCG,
                            STRUCT_DATA *sd,
                            INPUT_PARMS *ip,
                            char *szTitle,
                            PINChI2 *pINChI2[INCHI_NUM],
                            PINChI_Aux2 *pINChI_Aux2[INCHI_NUM],
                            INCHI_IOSTREAM *inp_file,
                            INCHI_IOSTREAM *log_file,
                            INCHI_IOSTREAM *out_file,
                            INCHI_IOSTREAM *prb_file,
                            ORIG_ATOM_DATA *orig_inp_data,
                            ORIG_ATOM_DATA *prep_inp_data,
                            long num_inp,
                            INCHI_IOSTREAM_STRING *strbuf,
                            unsigned char save_opt_bits);

int  OrigAtData_CreateCopy( ORIG_ATOM_DATA *new_orig_atom, ORIG_ATOM_DATA *orig_atom );
void OrigAtData_DebugTrace( ORIG_ATOM_DATA *at_data );
int  OrigAtData_RemoveHalfBond( int this_atom, int other_atom, inp_ATOM *at, int *bond_type, int *bond_stereo );
int  OrigAtData_DestroyBond( int this_atom, int other_atom, inp_ATOM *at, int *num_inp_bonds);
int  OrigAtData_RemoveBond( int this_atom, int other_atom, inp_ATOM *at,
                            int *bond_type, int *bond_stereo, int *num_inp_bonds);
int  OrigAtData_AddBond( int this_atom, int other_atom, inp_ATOM *at,
                         int bond_type, int bond_stereo, int *num_bonds );
int  OrigAtData_AddSingleStereolessBond( int this_atom, int other_atom,
                         inp_ATOM *at, int *num_inp_bonds );
int  OrigAtData_IncreaseBondOrder( int this_atom, int other_atom, inp_ATOM *at );
int  OrigAtData_DecreaseBondOrder( int this_atom, int other_atom, inp_ATOM *at );
void OrigAtData_CheckAndMakePolymerPhaseShifts( OrigAtDataPolymer *p,
                                                inp_ATOM *at,
                                                int nat,
                                                int *num_inp_bonds);
int  OrigAtData_FindRingSystems( OrigAtDataPolymer *pd,
                                 inp_ATOM *at,
                                 int nat,
                                 int *num_inp_bonds,
                                 int *num_ring_sys,
                                 int *size_ring_sys,
                                 int start );
void OrigAtData_FillAtProps( OrigAtDataPolymer *pd,
                             inp_ATOM *at,
                             int nat,
                             int *num_inp_bonds,
                             OrigAtDataPolymerAtomProps *aprops );

int get_canonical_atom_numbers_and_component_numbers( CANON_GLOBALS *pCG,
                                                      INCHI_IOSTREAM_STRING *strbuf,
                                                      OutputINCHI_CtlData *io,
                                                      int nat, int *cano_nums,
                                                      int *compnt_nums );


int CreateOneStructureINChI( struct tagCANON_GLOBALS *pCG,
                             struct tagINCHI_CLOCK *ic,
                             STRUCT_DATA *sd,
                             INPUT_PARMS *ip,
                             char *szTitle,
                             PINChI2 *pINChI2[INCHI_NUM],
                             PINChI_Aux2 *pINChI_Aux2[INCHI_NUM],
                             int iINChI,
                             INCHI_IOSTREAM *inp_file,
                             INCHI_IOSTREAM *log_file,
                             INCHI_IOSTREAM *out_file,
                             INCHI_IOSTREAM *prb_file,
                             ORIG_ATOM_DATA *orig_inp_data,
                             ORIG_ATOM_DATA *prep_inp_data,
                             COMP_ATOM_DATA composite_norm_data2[][TAUT_NUM+1],
                             long num_inp,
                             INCHI_IOSTREAM_STRING *strbuf,
                             NORM_CANON_FLAGS *pncFlags );
int bIsStructChiral( PINChI2 *pINChI2[INCHI_NUM],
                     int num_components[]);
int PreprocessOneStructure( struct tagINCHI_CLOCK *ic,
                            STRUCT_DATA *sd,
                            INPUT_PARMS *ip,
                            ORIG_ATOM_DATA *orig_inp_data,
                            ORIG_ATOM_DATA *prep_inp_data );
int DuplicateOrigAtom( ORIG_ATOM_DATA *new_orig_atom,
                       ORIG_ATOM_DATA *orig_atom );
int OrigStruct_FillOut( struct tagCANON_GLOBALS *pCG,
                       ORIG_ATOM_DATA *orig_inp_data,
                       ORIG_STRUCT *pOrigStruct,
                       STRUCT_DATA *sd);
void FreeOrigStruct( ORIG_STRUCT *pOrigStruct);
int ReadWriteInChI( INCHI_IOSTREAM *pInp,
                    INCHI_IOSTREAM *pOut,
                    INCHI_IOSTREAM *pLog,
                    INPUT_PARMS *ip_inp,
                    STRUCT_DATA *sd_inp,
                    /* the following are InChI library-specific parameters */
                    inp_ATOM **at,
                    int *num_at,
                    int *num_bonds,
                    OrigAtDataPolymer **polymer,
                    OrigAtDataV3000    **v3000,
                    /* end of InChI library-specific parameters */
                    char *szMsg,
                    int nMsgLen,
                    unsigned long WarningFlags[2][2],
                    struct tagINCHI_CLOCK *ic,
                    struct tagCANON_GLOBALS *pCG);
int CompareHillFormulasNoH( const char *f1,
                            const char *f2,
                            int *num_H1,
                            int *num_H2);
int CreateCompositeNormAtom( COMP_ATOM_DATA *composite_norm_data,
                             INP_ATOM_DATA2 *all_inp_norm_data,
                             int num_components );
void set_line_separators( int bINChIOutputOptions, char **pLF, char **pTAB );
void save_command_line( int argc, char *argv[ ], INCHI_IOSTREAM *plog);
void emit_error_inchi_text( INPUT_PARMS *ip,
                            long num_inp,
                            char *pLF,
                            char *pTAB,
                            INCHI_IOSTREAM *pout);
#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif

#endif    /* _ICHIMAIN_H_ */
