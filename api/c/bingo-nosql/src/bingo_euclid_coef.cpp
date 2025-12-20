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

#include "bingo_euclid_coef.h"

using namespace bingo;

EuclidCoef::EuclidCoef(int fp_size) : _fp_size(fp_size)
{
}

double EuclidCoef::calcCoef(const byte* target, const byte* query, int target_bit_count, int query_bit_count)
{
    int common_bits = bitCommonOnes(target, query, _fp_size);

    if (target_bit_count == -1)
        target_bit_count = bitGetOnesCount(target, _fp_size);

    return (double)common_bits / target_bit_count;
}

double EuclidCoef::calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count)
{
    int min = (query_bit_count < max_target_bit_count ? query_bit_count : max_target_bit_count);

    return (double)min / query_bit_count;
}

double EuclidCoef::calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count, int m10, int m01)
{
    int max_a = max_target_bit_count - m10;
    int b = query_bit_count - m01;

    int min = (b > max_a ? max_a : b);

    return (double)min / min_target_bit_count;
}