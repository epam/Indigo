#ifndef __tversky_coef__
#define __tversky_coef__

#include "base_c/bitarray.h"
#include "bingo_sim_coef.h"
#include <math.h>

namespace bingo
{
    class TverskyCoef : public SimCoef
    {
    public:
        TverskyCoef(int fp_size);

        TverskyCoef(int fp_size, double a, double b);

        double calcCoef(const byte* target, const byte* query, int target_bit_count, int query_bit_count);

        double calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count);

        double calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count, int m10, int m01);

    private:
        int _fp_size;
        double _alpha;
        double _beta;
    };
}; // namespace bingo

#endif /* __tversky_coef__ */
