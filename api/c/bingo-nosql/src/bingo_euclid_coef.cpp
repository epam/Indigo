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