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


#ifndef __INCHIRING_H__
#define __INCHIRING_H__
#define QUEUE_QINT 1
typedef AT_RANK qInt; /* queue optimization: known type */

#if ( QUEUE_QINT == 1 )
#define QINT_TYPE qInt
#else
#define QINT_TYPE void
#endif

typedef struct tagQieue {
    QINT_TYPE *Val;
    int nTotLength;
    int nFirst;  /* element to remove if nLength > 0 */
    int nLength; /* (nFirst + nLength) is next free position */
#if ( QUEUE_QINT != 1 )
    int nSize;
#endif
}QUEUE;

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
extern "C" {
#endif
#endif

QUEUE *QueueCreate( int nTotLength, int nSize );
QUEUE *QueueDelete( QUEUE *q );
int is_bond_in_Nmax_memb_ring( inp_ATOM* atom, int at_no, int neigh_ord, QUEUE *q, AT_RANK *nAtomLevel, S_CHAR *cSource, AT_RANK nMaxRingSize );
int is_atom_in_3memb_ring( inp_ATOM* atom, int at_no );

#ifndef COMPILE_ALL_CPP
#ifdef __cplusplus
}
#endif
#endif


#endif /* __INCHIRING_H__ */
