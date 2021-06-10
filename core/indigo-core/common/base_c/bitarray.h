/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __bitarray_h__
#define __bitarray_h__

#ifdef __cplusplus
extern "C"
{
#endif

#include "base_c/defs.h"

    DLLEXPORT int bitGetBit(const void* bitarray, int bitno);
    DLLEXPORT void bitSetBit(void* bitarray, int bitno, int value);
    DLLEXPORT void bitFlipBit(void* bitarray, int bitno);
    DLLEXPORT int bitTestEquality(const void* bits1, const void* bits2, int nbits);
    DLLEXPORT int bitTestEquality_Array(const void* bits, const void* bitarray, int array_start, int nbits);
    DLLEXPORT int bitTestEqualityByMask(const void* bits1, const void* bits2, const void* bitMask, int nbits);

    // res = a & (b ^ ~c)
    DLLEXPORT int bitGetAandBxorNotC(const void* a, const void* b, const void* c, void* res, int nbits);
    // Get number of ones
    DLLEXPORT int bitGetOnesCountByte(byte value);
    DLLEXPORT int bitGetOnesCountQword(qword value);
    DLLEXPORT int bitGetOnesCountDword(dword value);
    DLLEXPORT int bitGetOnesCount(const byte* data, int size);
    // Get high-order 1-bit in byte
    DLLEXPORT int bitGetOneHOIndex(byte value);
    // Get low-order 1-bit in byte
    DLLEXPORT int bitGetOneLOIndex(byte value);

    DLLEXPORT int bitGetSize(int nbits);

    DLLEXPORT int bitTestOnes(const byte* pattern, const byte* candidate, int n_bytes);
    DLLEXPORT int bitIdecticalBits(const byte* bit1, const byte* bit2, int n_bytes);
    DLLEXPORT int bitCommonOnes(const byte* bit1, const byte* bit2, int n_bytes);
    DLLEXPORT int bitUniqueOnes(const byte* bit1, const byte* bit2, int n_bytes);

    DLLEXPORT int bitDifferentOnes(const byte* bit1, const byte* bit2, int n_bytes);
    DLLEXPORT int bitUnionOnes(const byte* bit1, const byte* bit2, int n_bytes);

    DLLEXPORT void bitAnd(byte* a, const byte* b, int n_bytes);
    DLLEXPORT void bitOr(byte* a, const byte* b, int nbytes);

    // Check whether bit array is zero
    DLLEXPORT int bitIsAllZero(const void* bits, int nbytes);

    DLLEXPORT int bitLog2Dword(dword input);

#ifdef __cplusplus
}
#endif

#endif
