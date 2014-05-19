#include "bingo_tversky_coef.h"

using namespace bingo;

TverskyCoef::TverskyCoef (int fp_size) : _fp_size(fp_size)
{
   _alpha = 1;
   _beta = 0;
}

TverskyCoef::TverskyCoef (int fp_size, double a, double b) : _fp_size(fp_size)
{
   _alpha = a;
   _beta = b;
}

double TverskyCoef::calcCoef( const byte *f1, const byte *f2, int f1_bit_count, int f2_bit_count )
{
   int common_bits = bitCommonOnes(f1, f2, _fp_size);

   if (f1_bit_count == -1)
      f1_bit_count = bitGetOnesCount(f1, _fp_size);
   if (f2_bit_count == -1)
      f2_bit_count = bitGetOnesCount(f2, _fp_size);

   return (double)common_bits / ((f1_bit_count - common_bits) * _alpha + 
                                 (f2_bit_count - common_bits) * _beta + common_bits);
}

double TverskyCoef::calcUpperBound( int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count )
{
   if (fabs(_alpha + _beta - 1) > 1e-7)
      return 1;

   int min = (f1_bit_count < max_f2_bit_count ? f1_bit_count : max_f2_bit_count);
   return min / (_alpha * f1_bit_count + _beta * min_f2_bit_count);
}

double TverskyCoef::calcUpperBound( int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count, int m10, int m01 )
{
   if (fabs(_alpha + _beta - 1) > 1e-7)
      return 1;

   int a = f1_bit_count - m10;
   int max_b = max_f2_bit_count - m01;

   int min = (a > max_b ? max_b : a );

   return (double)min / (_alpha * f1_bit_count + _beta * min_f2_bit_count);
}