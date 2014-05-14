#ifndef __euclid_coef__
#define __euclid_coef__

#include "bingo_sim_coef.h"
#include <math.h>
#include "base_c/bitarray.h"

namespace bingo
{
   class EuclidCoef : public SimCoef
   {
   public:
      EuclidCoef (int fp_size);

      double calcCoef (const byte *f1, const byte *f2, int f1_bit_count, int f2_bit_count );

      double calcUpperBound (int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count );

      double calcUpperBound (int f1_bit_count, int min_f2_bit_count, int max_f2_bit_count, int m10, int m01 );

   private:
      int _fp_size;
   };
};

#endif /* __euclid_coef__ */
