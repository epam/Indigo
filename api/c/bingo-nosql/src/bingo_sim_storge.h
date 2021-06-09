#ifndef __sim_storage__
#define __sim_storage__

#include "base_cpp/obj_array.h"
#include "base_cpp/output.h"
#include "base_cpp/scanner.h"
#include "base_cpp/tlscont.h"
#include "bingo_fingerprint_table.h"
#include "math/algebra.h"
#include "new"
#include "time.h"

#include <vector>

using namespace indigo;
namespace bingo
{
    class SimStorage
    {
    public:
        SimStorage(int fp_size, int mt_size, int inc_size);

        static BingoAddr create(BingoPtr<SimStorage>& ptr, int fp_size, int mt_size, int inc_size);

        static void load(BingoPtr<SimStorage>& ptr, BingoAddr offset);

        void add(const byte* fingerprint, int id);

        void optimize();

        int getCellCount() const;

        int getCellSize(int cell_idx) const;

        void getCellsInterval(const byte* query, SimCoef& sim_coef, double min_coef, int& min_cell, int& max_cell);

        int firstFitCell(int query_bit_count, int min_cell, int max_cell) const;

        int nextFitCell(int query_bit_count, int first_fit_cell, int min_cell, int max_cell, int idx) const;

        int getSimilar(const byte* query, SimCoef& sim_coef, double min_coef, ArrayNew<SimResult>& sim_fp_indices, int cell_idx, int cont_idx);

        bool isSmallBase();

        int getIncSimilar(const byte* query, SimCoef& sim_coef, double min_coef, ArrayNew<SimResult>& sim_fp_indices);

        ~SimStorage();

    private:
        BingoPtr<FingerprintTable> _fingerprint_table;
        BingoPtr<byte> _inc_buffer;
        BingoPtr<size_t> _inc_id_buffer;
        int _inc_size;
        int _inc_fp_count;

        int _mt_size;
        int _fp_size;
    };
}; // namespace bingo

#endif /* __fingerprint_table__ */
