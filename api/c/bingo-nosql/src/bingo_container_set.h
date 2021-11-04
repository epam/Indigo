#ifndef __container_set__
#define __container_set__

#include <vector>

#include "base_c/bitarray.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/profiling.h"
#include "base_cpp/tlscont.h"

#include "bingo_cell_container.h"
#include "bingo_multibit_tree.h"
#include "mmf/mmf_array.h"
#include "mmf/mmf_ptr.h"

namespace bingo
{
    class ContainerSet
    {
    public:
        ContainerSet();

        void setParams(int fp_size, int container_size, int min_ones_count, int max_ones_count);

        int getContCount() const;

        int getMinBorder() const;

        int getMaxBorder() const;

        bool add(const byte* fingerprint, int id, int fp_ones_count = -1);

        void buildContainer();

        void splitSet(ContainerSet& new_set);

        void findSimilar(const byte* query, SimCoef& sim_coef, double min_coef, indigo::Array<SimResult>& sim_indices);

        void optimize();

        int getSimilar(const byte* query, SimCoef& sim_coef, double min_coef, indigo::Array<SimResult>& sim_fp_indices, int cont_idx);

    private:
        MMFArray<MultibitTree> _set;
        int _fp_size;
        int _container_size;
        MMFPtr<byte> _increment;
        MMFPtr<int> _indices;

        int _inc_count;
        int _inc_total_ones_count;
        int _min_ones_count;
        int _max_ones_count;

        int _findSimilarInc(const byte* query, SimCoef& sim_coef, double min_coef, indigo::Array<SimResult>& sim_indices);
    };
}; // namespace bingo

#endif /* __container_set__ */
