#ifndef __tanimoto_coef__
#define __tanimoto_coef__

#include "bingo_sim_coef.h"
#include <math.h>
#include "base_c/bitarray.h"

class TanimotoCoef : public SimCoef
{
public:
   TanimotoCoef( int fp_size );

   double calcCoef( const byte *f1, const byte *f2, int f1_bit_count, int f2_bit_count );

   double calcUpperBound( int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count, int min_dist );

   double calcUpperBound( int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count );

   double calcUpperBound1( int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count, int m10, int m01 );

private:
   int _fp_size;
};

#endif /* __tanimoto_coef__ */
