/*
 * International Chemical Identifier (InChI)
 * Version 1
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
                                       COMP_ATOM_DATA composite_norm_data[TAUT_NUM+1] );


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
