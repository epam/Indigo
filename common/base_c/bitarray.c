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

#include <stdlib.h>
#include <string.h>

#include "base_c/bitarray.h"

int bitGetBit(const void* bitarray, int bitno)
{
    return ((((char*)bitarray)[bitno / 8] & (char)(1 << (bitno % 8))) == 0) ? 0 : 1;
}

void bitSetBit(void* bitarray, int bitno, int value)
{
    if (value)
        ((char*)bitarray)[bitno / 8] |= (1 << (bitno % 8));
    else
        ((char*)bitarray)[bitno / 8] &= ~(1 << (bitno % 8));
}

void bitFlipBit(void* bitarray, int bitno)
{
    ((char*)bitarray)[bitno / 8] ^= (1 << (bitno % 8));
}

int bitTestEquality(const void* bits1, const void* bits2, int nbits)
{
    char* chars1 = (char*)bits1;
    char* chars2 = (char*)bits2;
    char mask = ~(0xFF << (nbits & 7));
    int i;

    for (i = 0; i < nbits / 8; i++)
        if (chars1[i] != chars2[i])
            return 0;

    if ((chars1[nbits / 8] & mask) != (chars2[nbits / 8] & mask))
        return 0;

    return 1;
}

int bitTestEquality_Array(const void* bits, const void* bitarray, int array_start, int nbits)
{
    int i;

    for (i = 0; i < nbits; i++)
        if (bitGetBit(bits, i) != bitGetBit(bitarray, array_start + i))
            return 0;

    return 1;
}

int bitTestEqualityByMask(const void* bits1, const void* bits2, const void* bitMaskVoid, int nbits)
{
    const char* chars1 = (const char*)bits1;
    const char* chars2 = (const char*)bits2;
    const char* bitMask = (const char*)bitMaskVoid;
    char mask = ~(0xFF << (nbits & 7));
    int i;

    for (i = 0; i < nbits / 8; i++)
        if ((chars1[i] & bitMask[i]) != (chars2[i] & bitMask[i]))
            return 0;

    if (((chars1[nbits / 8] & bitMask[nbits / 8]) & mask) != ((chars2[nbits / 8] & bitMask[nbits / 8]) & mask))
        return 0;

    return 1;
}

// res = a & (b ^ ~c)
int bitGetAandBxorNotC(const void* a, const void* b, const void* c, void* res, int nbits)
{
    const char* ac = (const char*)a;
    const char* bc = (const char*)b;
    const char* cc = (const char*)c;
    char* rc = (char*)res;
    int i;

    for (i = 0; i < nbits / 8; i++)
        rc[i] = ac[i] & (bc[i] ^ ~cc[i]);

    if ((nbits & 7) != 0)
        rc[i] = ac[i] & (bc[i] ^ ~cc[i]);

    return 1;
}

int bitGetOnesCountByte(byte value)
{
    static const int onesCount[] = {0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 1, 2, 2, 3, 2,
                                    3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3,
                                    3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5,
                                    6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4,
                                    3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4,
                                    5, 5, 6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6,
                                    6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8};
    return onesCount[value];
}

int bitGetOnesCountDword(dword v)
{
    // http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetKernighan
    v = v - ((v >> 1) & 0x55555555);                         // reuse input as temporary
    v = (v & 0x33333333) + ((v >> 2) & 0x33333333);          // temp
    return (((v + (v >> 4)) & 0xF0F0F0F) * 0x1010101) >> 24; // count
}

int bitGetOnesCountQword(qword value)
{
    return bitGetOnesCountDword((dword)value) + bitGetOnesCountDword((dword)(value >> 32));
}

int bitGetOnesCount(const byte* data, int size)
{
    int count = 0;
    while (size-- > 0)
        count += bitGetOnesCountByte(*data++);
    return count;
}

int bitGetOneHOIndex(byte value)
{
    static const int oneHOIndex[] = {0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6, 6, 6,
                                     6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
                                     7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                     8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                     8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
                                     8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
    return oneHOIndex[value];
}

int bitGetOneLOIndex(byte value)
{
    static const int oneLOIndex[] = {8, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2,
                                     0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0,
                                     1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1,
                                     0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0,
                                     2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3,
                                     0, 1, 0, 2, 0, 1, 0, 6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0,
                                     1, 0, 5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};
    return oneLOIndex[value];
}

int bitGetSize(int nbits)
{
    return (nbits + 7) / 8;
}

int bitTestOnes(const byte* pattern, const byte* candidate, int n_bytes)
{
    int qwords_count = n_bytes / sizeof(qword);
    int bytes_left = n_bytes - qwords_count * sizeof(qword);

    const qword* pattern_ptr = (const qword*)pattern;
    const qword* candidate_ptr = (const qword*)candidate;
    while (qwords_count-- > 0)
    {
        if ((*candidate_ptr & *pattern_ptr) != *pattern_ptr)
            return 0;
        pattern_ptr++;
        candidate_ptr++;
    }

    if (bytes_left != 0)
    {
        qword mask = ~(qword)0 >> (64 - 8 * bytes_left);

        if ((*candidate_ptr & *pattern_ptr & mask) != (*pattern_ptr & mask))
            return 0;
    }
    return 1;
}

int bitIdecticalBits(const byte* bit1, const byte* bit2, int n_bytes)
{
    int qwords_count = n_bytes / sizeof(qword);
    int bytes_left = n_bytes - qwords_count * sizeof(qword);

    int count = 0;
    const qword* bit1_ptr = (const qword*)bit1;
    const qword* bit2_ptr = (const qword*)bit2;
    while (qwords_count-- > 0)
    {
        qword id = ~(*bit1_ptr ^ *bit2_ptr);
        count += bitGetOnesCountQword(id);

        bit1_ptr++;
        bit2_ptr++;
    }

    if (bytes_left != 0)
    {
        qword mask = ~(qword)0 >> (64 - 8 * bytes_left);
        qword id = (~(*bit1_ptr ^ *bit2_ptr)) & mask;
        count += bitGetOnesCountQword(id);
    }
    return count;
}

int bitCommonOnes(const byte* bit1, const byte* bit2, int n_bytes)
{
    int qwords_count = n_bytes / sizeof(qword);
    int bytes_left = n_bytes - qwords_count * sizeof(qword);

    int count = 0;
    const qword* bit1_ptr = (const qword*)bit1;
    const qword* bit2_ptr = (const qword*)bit2;
    while (qwords_count-- > 0)
    {
        qword id = *bit1_ptr & *bit2_ptr;
        count += bitGetOnesCountQword(id);

        bit1_ptr++;
        bit2_ptr++;
    }

    if (bytes_left != 0)
    {
        qword mask = ~(qword)0 >> (64 - 8 * bytes_left);
        qword id = *bit1_ptr & *bit2_ptr & mask;
        count += bitGetOnesCountQword(id);
    }
    return count;
}

int bitUniqueOnes(const byte* bit1, const byte* bit2, int n_bytes)
{
    int qwords_count = n_bytes / sizeof(qword);
    int bytes_left = n_bytes - qwords_count * sizeof(qword);

    int count = 0;
    const qword* bit1_ptr = (const qword*)bit1;
    const qword* bit2_ptr = (const qword*)bit2;
    while (qwords_count-- > 0)
    {
        qword id = *bit1_ptr & ~*bit2_ptr;
        count += bitGetOnesCountQword(id);

        bit1_ptr++;
        bit2_ptr++;
    }

    if (bytes_left != 0)
    {
        qword mask = ~(qword)0 >> (64 - 8 * bytes_left);
        qword id = *bit1_ptr & ~*bit2_ptr & mask;
        count += bitGetOnesCountQword(id);
    }
    return count;
}

int bitDifferentOnes(const byte* bit1, const byte* bit2, int n_bytes)
{
    int qwords_count = n_bytes / sizeof(qword);
    int bytes_left = n_bytes - qwords_count * sizeof(qword);

    int count = 0;
    const qword* bit1_ptr = (const qword*)bit1;
    const qword* bit2_ptr = (const qword*)bit2;
    while (qwords_count-- > 0)
    {
        qword id = *bit1_ptr ^ *bit2_ptr;
        count += bitGetOnesCount((byte*)&id, sizeof(qword));

        bit1_ptr++;
        bit2_ptr++;
    }

    if (bytes_left != 0)
    {
        qword mask = ~(qword)0 >> (64 - 8 * bytes_left);
        qword id = (*bit1_ptr ^ *bit2_ptr) & mask;
        count += bitGetOnesCountQword(id);
    }
    return count;
}

int bitUnionOnes(const byte* bit1, const byte* bit2, int n_bytes)
{
    int qwords_count = n_bytes / sizeof(qword);
    int bytes_left = n_bytes - qwords_count * sizeof(qword);

    int count = 0;
    const qword* bit1_ptr = (const qword*)bit1;
    const qword* bit2_ptr = (const qword*)bit2;
    while (qwords_count-- > 0)
    {
        qword id = *bit1_ptr | *bit2_ptr;
        count += bitGetOnesCountQword(id);

        bit1_ptr++;
        bit2_ptr++;
    }

    if (bytes_left != 0)
    {
        qword mask = ~(qword)0 >> (64 - 8 * bytes_left);
        qword id = (*bit1_ptr | *bit2_ptr) & mask;
        count += bitGetOnesCountQword(id);
    }
    return count;
}

// a &= b
void bitAnd(byte* a, const byte* b, int nbytes)
{
    while (nbytes-- > 0)
    {
        *a = *a & *b;
        a++;
        b++;
    }
}

// a |= b
void bitOr(byte* a, const byte* b, int nbytes)
{
    while (nbytes-- > 0)
    {
        *a = *a | *b;
        a++;
        b++;
    }
}

int bitIsAllZero(const void* bits, int nbytes)
{
    const byte* a = (const byte*)bits;

    while (nbytes-- > 0)
    {
        if (*a != 0)
            return 0;
        a++;
    }
    return 1;
}

int bitLog2Dword(dword input)
{
    int count = 0;
    while (input > 0)
    {
        count++;
        input >>= 1;
    }
    return count;
}
