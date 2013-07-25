#include "bingo_tanimoto_coef.h"

TanimotoCoef::TanimotoCoef( int fp_size ) : _fp_size(fp_size)
{
}

double TanimotoCoef::calcCoef( const byte *f1, const byte *f2, int f1_bit_count, int f2_bit_count )
{
   int common_bits = bitCommonOnes(f1, f2, _fp_size);
   int unique_bits = bitDifferentOnes(f1, f2, _fp_size);
   
   return (double)common_bits / (common_bits + unique_bits);
}

double TanimotoCoef::calcUpperBound( int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count, int min_dist )
{
   int a = f1_bit_count;
   int min_b = min_f2_bit_count;
   int max_b = max_f2_bit_count;

   return (double)(a + max_b - min_dist) / (a + min_b + min_dist);
}

double TanimotoCoef::calcUpperBound( int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count )
{
   int min = (f1_bit_count < max_f2_bit_count ? f1_bit_count : max_f2_bit_count);
   int max = (f1_bit_count > min_f2_bit_count ? f1_bit_count : min_f2_bit_count);
   
   return (double)min / max;
}

double TanimotoCoef::calcUpperBound1( int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count, int m10, int m01 )
{
   int a = f1_bit_count - m10;
   int min_b = min_f2_bit_count - m01;
   int max_b = max_f2_bit_count - m01;

   int min = (a > max_b ? max_b : a );
   int max = (a < min_b ? min_b : a );

   return (double)min / (m10 + m01 + max);
}