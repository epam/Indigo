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


#ifndef _INCHI_GUI_H_
#define _INCHI_GUI_H_


#include "strutil.h"
#include "ichicomn.h"


#ifndef COMPILE_ANSI_ONLY

struct tagCANON_GLOBALS;


int DisplayStructure( struct tagCANON_GLOBALS *pCG,
                      inp_ATOM *at,
                      int num_at,
                      OAD_Polymer *polymer,
                      int num_removed_H,
                      int bAdd_DT_to_num_H,
                      int nNumRemovedProtons,
                      NUM_H nNumRemovedProtonsIsotopic[],
                      int bIsotopic,
                      int j /*bTautomeric*/,
                      INChI **cur_INChI,
                      INChI_Aux **cur_INChI_Aux,
                      int bAbcNumbers,
                      DRAW_PARMS *dp,
                      INCHI_MODE nMode,
                      char *szTitle );

int DisplayCompositeStructure( struct tagCANON_GLOBALS *pCG,
                               COMP_ATOM_DATA *composite_norm_data,
                               OAD_Polymer *polymer,
                               int bIsotopic,
                               int bTautomeric,
                               PINChI2 *pINChI2,
                               PINChI_Aux2 *pINChI_Aux2,
                               int bAbcNumbers,
                               DRAW_PARMS *dp,
                               INCHI_MODE nMode,
                               char *szTitle );


int DisplayTheWholeStructure( struct tagCANON_GLOBALS *pCG,
                              struct tagINCHI_CLOCK *ic,
                              struct tagStructData *sd,
                              INPUT_PARMS *ip,
                              char *szTitle,
                              INCHI_IOSTREAM *inp_file,
                              INCHI_IOSTREAM *log_file,
                              ORIG_ATOM_DATA *orig_inp_data,
                              long num_inp,
                              int iINChI,
                              int bShowStruct,
                              int bINCHI_LIB_Flag );

int DisplayTheWholeCompositeStructure( struct tagCANON_GLOBALS *pCG,
                                       struct tagINCHI_CLOCK *ic,
                                       INPUT_PARMS *ip,
                                       struct tagStructData *sd,
                                       long num_inp,
                                       int iINChI,
                                       PINChI2 *pINChI2,
                                       PINChI_Aux2 *pINChI_Aux2,
                                       ORIG_ATOM_DATA *orig_inp_data,
                                       ORIG_ATOM_DATA *prep_inp_data,
                                       COMP_ATOM_DATA composite_norm_data[TAUT_NUM + 1] );


void FillTableParms( SET_DRAW_PARMS *sdp,
                     INChI **cur_INChI,
                     INChI_Aux **cur_INChI_Aux,
                     INCHI_MODE nMode,
                     int bShowIsotopic,
                     int bShowTaut );

void FillCompositeTableParms( SET_DRAW_PARMS *sdp,
                              AT_NUMB StereoFlags,
                              INCHI_MODE nMode,
                              int bShowIsotopic,
                              int bShowTaut );

#endif

#endif /* _INCHI_GUI_H_ */
