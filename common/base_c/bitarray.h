/****************************************************************************
 * Copyright (C) 2009-2012 GGA Software Services LLC
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

DLLEXPORT int   bitGetBit  (const void *bitarray, int bitno);
DLLEXPORT void  bitSetBit  (void *bitarray, int bitno, int value);
DLLEXPORT void  bitFlipBit (void *bitarray, int bitno);
DLLEXPORT int   bitTestEquality (const void *bits1, const void *bits2, int nbits);
DLLEXPORT int   bitTestEquality_Array (const void *bits, const void *bitarray, 
                             int array_start, int nbits);
DLLEXPORT int   bitTestEqualityByMask (const void *bits1, const void *bits2, 
                             const void *bitMask, int nbits);

// res = a & (b ^ ~c)
DLLEXPORT int   bitGetAandBxorNotC (const void *a, const void *b, const void *c, void *res, int nbits);
// Get number of ones
DLLEXPORT int   bitGetOnesCountByte (byte value);
DLLEXPORT int   bitGetOnesCountQword (qword value);
DLLEXPORT int   bitGetOnesCountDword (dword value);
DLLEXPORT int   bitGetOnesCount (const byte *data, int size);
// Get high-order 1-bit in byte
DLLEXPORT int   bitGetOneHOIndex  (byte value);
// Get low-order 1-bit in byte
DLLEXPORT int   bitGetOneLOIndex  (byte value);

DLLEXPORT int   bitGetSize (int nbits);

DLLEXPORT int bitTestOnes      (const byte *pattern, const byte *candidate, int n_bytes);
DLLEXPORT int bitIdecticalBits (const byte *bit1, const byte *bit2, int n_bytes);
DLLEXPORT int bitCommonOnes    (const byte *bit1, const byte *bit2, int n_bytes);
DLLEXPORT int bitUniqueOnes    (const byte *bit1, const byte *bit2, int n_bytes);

DLLEXPORT int bitDifferentOnes (const byte *bit1, const byte *bit2, int n_bytes);
DLLEXPORT int bitUnionOnes     (const byte *bit1, const byte *bit2, int n_bytes);

DLLEXPORT void bitAnd (byte *a, const byte *b, int n_bytes);
DLLEXPORT void bitOr (byte *a, const byte *b, int nbytes);

// Check whether bit array is zero
DLLEXPORT int bitIsAllZero (const void *bits, int nbytes);

#ifdef __cplusplus
}
#endif

#endif
