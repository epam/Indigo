#include "bingo_tversky_coef.h"

using namespace bingo;

TverskyCoef::TverskyCoef(int fp_size) : _fp_size(fp_size)
{
    _alpha = 1;
    _beta = 0;
}

TverskyCoef::TverskyCoef(int fp_size, double a, double b) : _fp_size(fp_size)
{
    _alpha = a;
    _beta = b;
}

double TverskyCoef::calcCoef(const byte* target, const byte* query, int target_bit_count, int query_bit_count)
{
    int common_bits = bitCommonOnes(target, query, _fp_size);

    if (target_bit_count == -1)
        target_bit_count = bitGetOnesCount(target, _fp_size);
    if (query_bit_count == -1)
        query_bit_count = bitGetOnesCount(query, _fp_size);

    return (double)common_bits / ((target_bit_count - common_bits) * _alpha + (query_bit_count - common_bits) * _beta + common_bits);
}

double TverskyCoef::calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count)
{
    if (fabs(_alpha + _beta - 1) > 1e-7)
        return 1;

    int min = (query_bit_count < max_target_bit_count ? query_bit_count : max_target_bit_count);
    return min / (_alpha * min_target_bit_count + _beta * query_bit_count);
}

double TverskyCoef::calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count, int m10, int m01)
{
    if (fabs(_alpha + _beta - 1) > 1e-7)
        return 1;

    int max_a = max_target_bit_count - m10;
    int b = query_bit_count - m01;

    int min = (b > max_a ? max_a : b);

    return (double)min / (_alpha * min_target_bit_count + _beta * query_bit_count);
}