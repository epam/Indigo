#ifndef __fingerprint_table__
#define __fingerprint_table__

#include <ctime>
#include <new>
#include <vector>

#include "base_cpp/obj_array.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "math/algebra.h"

#include "bingo_container_set.h"
#include "bingo_sim_coef.h"
#include "mmf/mmf_ptr.h"

namespace bingo
{
    class FingerprintTable
    {
    public:
        FingerprintTable(int fp_size, const indigo::Array<int>& borders, int mt_size);

        static MMFAddress create(MMFPtr<FingerprintTable>& ptr, int fp_size, int mt_size);

        static void load(MMFPtr<FingerprintTable>& ptr, MMFAddress offset);

        void add(const byte* fingerprint, int id);

        void findSimilar(const byte* query, SimCoef& sim_coef, double min_coef, indigo::Array<SimResult>& sim_fp_indices);

        void optimize();

        int getCellCount() const;

        int getCellSize(int cell_idx) const;

        void getCellsInterval(const byte* query, SimCoef& sim_coef, double min_coef, int& min_cell, int& max_cell);

        int firstFitCell(int query_bit_count, int min_cell, int max_cell) const;

        int nextFitCell(int query_bit_count, int first_fit_cell, int min_cell, int max_cell, int idx) const;

        int getSimilar(const byte* query, SimCoef& sim_coef, double min_coef, indigo::Array<SimResult>& sim_fp_indices, int cell_idx, int cont_idx);

        ~FingerprintTable();

    private:
        MMFArray<ContainerSet> _table;
        int _max_cell_count;
        int _fp_size;
        int _mt_size;

        MMFPtr<byte> _inc_buffer;
        MMFPtr<size_t> _inc_id_buffer;
        int _inc_size;
        int _inc_fp_count;
    };
}; // namespace bingo

#endif /* __fingerprint_table__ */
