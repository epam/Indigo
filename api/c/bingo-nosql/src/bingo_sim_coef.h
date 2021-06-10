#ifndef __sim_coef__
#define __sim_coef__

#include "base_c/defs.h"

namespace bingo
{
    struct SimResult
    {
        int id;
        float sim_value;

        SimResult(int new_id, float new_sim_value) : id(new_id), sim_value(new_sim_value)
        {
        }
    };

    class SimCoef
    {
    public:
        SimCoef()
        {
        }

        virtual ~SimCoef(){};

        virtual double calcCoef(const byte* target, const byte* query, int target_bit_count, int query_bit_count) = 0;

        virtual double calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count) = 0;

        virtual double calcUpperBound(int query_bit_count, int min_target_bit_count, int max_target_bit_count, int m10, int m01) = 0;
    };
}; // namespace bingo

#endif /* __sim_coef__ */
