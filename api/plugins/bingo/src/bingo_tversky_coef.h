#ifndef __tversky_coef__
#define __tanimoto_coef__

#include "bingo_sim_coef.h"
#include <math.h>
#include "base_c/bitarray.h"

namespace bingo
{
   class TverskyCoef : public SimCoef
   {
   public:
      TverskyCoef (int fp_size);

      TverskyCoef (int fp_size, double a, double b);

      double calcCoef (const byte *f1, const byte *f2, int f1_bit_count, int f2_bit_count );

      double calcUpperBound (int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count );

      double calcUpperBound (int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count, int m10, int m01 );

   private:
      int _fp_size;
      double _alpha;
      double _beta;
   };
};

#endif /* __tversky_coef__ */
