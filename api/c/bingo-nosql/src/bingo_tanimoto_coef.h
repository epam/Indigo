#ifndef __tanimoto_coef__
#define __tanimoto_coef__

#include "base_c/bitarray.h"
#include "bingo_sim_coef.h"
#include <math.h>

namespace bingo
{
    class TanimotoCoef : public SimCoef
    {
    public:
        TanimotoCoef(int fp_size);

        double calcCoef(const byte* target, const byte* query, int target_bit_count, int query_bit_count);

        double calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count);

        double calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count, int m10, int m01);

    private:
        int _fp_size;
    };
}; // namespace bingo

#endif /* __tanimoto_coef__ */
