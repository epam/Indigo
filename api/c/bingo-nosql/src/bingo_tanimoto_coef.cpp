#include "bingo_tanimoto_coef.h"

using namespace bingo;

TanimotoCoef::TanimotoCoef(int fp_size) : _fp_size(fp_size)
{
}

double TanimotoCoef::calcCoef(const byte* target, const byte* query, int target_bit_count, int query_bit_count)
{
    int common_bits = bitCommonOnes(target, query, _fp_size);
    int unique_bits = bitDifferentOnes(target, query, _fp_size);

    return (double)common_bits / (common_bits + unique_bits);
}

double TanimotoCoef::calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count)
{
    int min = (query_bit_count < max_target_bit_count ? query_bit_count : max_target_bit_count);
    int max = (query_bit_count > min_target_bit_count ? query_bit_count : min_target_bit_count);

    return (double)min / max;
}

double TanimotoCoef::calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count, int m10, int m01)
{
    int b = query_bit_count - m01;
    int min_a = min_target_bit_count - m10;
    int max_a = max_target_bit_count - m10;

    int min = (b > max_a ? max_a : b);
    int max = (b < min_a ? min_a : b);

    return (double)min / (m10 + m01 + max);
}