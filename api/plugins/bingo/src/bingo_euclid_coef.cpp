#include "bingo_euclid_coef.h"

using namespace bingo;

EuclidCoef::EuclidCoef( int fp_size ) : _fp_size(fp_size)
{
}

double EuclidCoef::calcCoef( const byte *f1, const byte *f2, int f1_bit_count, int f2_bit_count )
{
   int common_bits = bitCommonOnes(f1, f2, _fp_size);

   if (f1_bit_count == -1)
      f1_bit_count = bitGetOnesCount(f1, _fp_size);
   
   return (double)common_bits / f1_bit_count;
}

double EuclidCoef::calcUpperBound( int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count )
{
   int min = (f1_bit_count < max_f2_bit_count ? f1_bit_count : max_f2_bit_count);
   
   return (double)min / f1_bit_count;
}

double EuclidCoef::calcUpperBound( int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count, int m10, int m01 )
{
   int a = f1_bit_count - m10;
   int min_b = min_f2_bit_count - m01;
   int max_b = max_f2_bit_count - m01;

   int min = (a > max_b ? max_b : a);

   return (double)min / f1_bit_count;
}