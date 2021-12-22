#ifndef __cell_container__
#define __cell_container__

#include "base_cpp/d_bitset.h"

#include "bingo_sim_coef.h"
#include "mmf/mmf_ptr.h"

namespace bingo
{
    class CellContainer
    {
    protected:
        int _min_fp_bit_number;
        int _max_fp_bit_number;
        int _fp_size;

    public:
        CellContainer(int fp_size) : _min_fp_bit_number(-1), _max_fp_bit_number(-1), _fp_size(fp_size)
        {
        }

        virtual void build(MMFPtr<byte> fingerprints, int fp_count, int min_fp_bit_number, int max_fp_bit_number) = 0;
        virtual void findSimilar(const byte* query, SimCoef& sim_coef, double min_coef, indigo::Array<SimResult>& sim_fp_indices) = 0;
    };
};     // namespace bingo
#endif /* _cell_container_ */
