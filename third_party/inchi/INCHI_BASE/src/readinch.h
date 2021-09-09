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


#ifndef _READINCH_H_
#define _READINCH_H_

#define INPUT_FILE          INCHI_IOSTREAM

#define MIN_BOND_LENGTH   (1.0e-6)
#define INCHI_LINE_LEN   262144 /*32767*/ /*512*/ /*1024*/ /*256*/
#define INCHI_LINE_ADD   32767 /*384*/  /*128*/  /*64*/


/* Convenience storage for InChI read control data */
typedef struct ReadINCHI_CtlData
{
    unsigned long ulongID;
    int bTooLongLine;
    int bHeaderRead;
    int bErrorMsg;
    int bRestoreInfo;
}
ReadINCHI_CtlData;


/*
Note:
(INCHI_LINE_LEN - INCHI_LINE_ADD) > (length of the longest item: szCoord) = 33
*/

char *FindToken( INCHI_IOSTREAM *inp_molfile,
                 int *bTooLongLine,
                 const char *sToken,
                 int lToken,
                 char *szLine,
                 int nLenLine,
                 char *p,
                 int *res );
char *LoadLine( INPUT_FILE *inp_molfile,
               int *bTooLongLine,
               int *bItemIsOver,
               char **s,
               char *szLine,
               int nLenLine,
               int nMinLen2Load,
               char *p,
               int *res );
inchi_Stereo0D *CreateInchi_Stereo0D( int num_stereo0D );
void FreeInchi_Stereo0D( inchi_Stereo0D **stereo0D );
int Extract0DParities( inp_ATOM *at,
                       int nNumAtoms,
                       inchi_Stereo0D *stereo0D,
                       int num_stereo0D,
                       char *pStrErr,
                       int *err,
                       int vABParityUnknown );
int InchiToInpAtom( INCHI_IOSTREAM *inp_molfile,
                     MOL_COORD **szCoord,
                     int bDoNotAddH,
                     int vABParityUnknown,
                     INPUT_TYPE nInputType,
                     inp_ATOM **at,
                     int max_num_at,
                     int *num_dimensions,
                     int *num_bonds,
                     char *pSdfLabel,
                     char *pSdfValue,
                     unsigned long *Id,
                     INCHI_MODE *pInpAtomFlags,
                     int *err,
                     char *pStrErr );


#endif    /* _READINCH_H_ */
