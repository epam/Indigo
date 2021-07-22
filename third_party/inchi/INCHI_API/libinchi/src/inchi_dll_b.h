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


#ifndef __INCHI_DLL_B_H__
#define __INCHI_DLL_B_H__

#ifndef AB_PARITY_UNKN
#define AB_PARITY_UNKN   3  /* 3 => user marked as unknown parity */
#endif
#ifndef AB_PARITY_UNDF
#define AB_PARITY_UNDF   4  /* 4 => parity cannot be defined because of symmetry or not well defined geometry */
#endif


#define MOL2INCHI_NO_RAM    1001
#define MOL2INCHI_BAD_COMMAND_LINE 1002


void FreeInchi_Stereo0D( inchi_Stereo0D **stereo0D );
void FreeInchi_Atom( inchi_Atom **at );
inchi_Atom *CreateInchiAtom( int num_atoms );
inchi_Stereo0D *CreateInchi_Stereo0D( int num_stereo0D );
void FreeInchi_Input( inchi_Input *inp_at_data );
S_SHORT *is_in_the_slist( S_SHORT *pathAtom, S_SHORT nNextAtom, int nPathLen );
int is_element_a_metal( char szEl[] );

int InchiToInchiAtom( INCHI_IOSTREAM *inp_molfile,
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


#endif /* __INCHI_DLL_B_H__ */
