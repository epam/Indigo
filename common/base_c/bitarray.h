/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#ifndef __bitarray_h__
#define __bitarray_h__

#ifdef __cplusplus
extern "C" {
#endif

#include "base_c/defs.h"

int   bitGetBit  (const void *bitarray, int bitno);
void  bitSetBit  (void *bitarray, int bitno, int value);
void  bitFlipBit (void *bitarray, int bitno);
int   bitTestEquality (const void *bits1, const void *bits2, int nbits);
int   bitTestEquality_Array (const void *bits, const void *bitarray, 
                             int array_start, int nbits);
int   bitTestEqualityByMask (const void *bits1, const void *bits2, 
                             const void *bitMask, int nbits);

// res = a & (b ^ ~c)
int   bitGetAandBxorNotC (const void *a, const void *b, const void *c, void *res, int nbits);
// Get number of ones
int   bitGetOnesCountByte (byte value);
int   bitGetOnesCount (const byte *data, int size);
// Get high-order 1-bit in byte
int   bitGetOneHOIndex  (byte value);
// Get low-order 1-bit in byte
int   bitGetOneLOIndex  (byte value);

int   bitGetSize (int nbits);

int bitTestOnes      (const byte *pattern, const byte *candidate, int n_bytes);
int bitIdecticalBits (const byte *bit1, const byte *bit2, int n_bytes);
int bitCommonOnes    (const byte *bit1, const byte *bit2, int n_bytes);
int bitUniqueOnes    (const byte *bit1, const byte *bit2, int n_bytes);

int bitDifferentOnes (const byte *bit1, const byte *bit2, int n_bytes);
int bitUnionOnes     (const byte *bit1, const byte *bit2, int n_bytes);

void bitAnd (byte *a, const byte *b, int n_bytes);
void bitOr (byte *a, const byte *b, int nbytes);

// Check whether bit array is zero
int bitIsAllZero (const void *bits, int nbytes);

#ifdef __cplusplus
}
#endif

#endif
