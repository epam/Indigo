#ifndef __multibit_tree__
#define __multibit_tree__

#include "base_c/bitarray.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/profiling.h"
#include "base_cpp/tlscont.h"
#include "bingo_cell_container.h"
#include "bingo_ptr.h"
#include "bingo_sim_coef.h"
#include "math/algebra.h"

using namespace indigo;

namespace bingo
{
    class MultibitTree
    {
    public:
        MultibitTree(int fp_size);

        void build(BingoPtr<byte> fingerprints, BingoPtr<int> indices, int fp_count, int min_fp_bit_number, int max_fp_bit_number);

        int findSimilar(const byte* query, SimCoef& sim_coef, double min_coef, Array<SimResult>& sim_fp_indices);

    private:
        struct _MatchBit
        {
            int idx;
            bool val;

            _MatchBit()
            {
                idx = -1;
                val = 0;
            }

            _MatchBit(int new_idx, bool new_val) : idx(new_idx), val(new_val)
            {
            }
        };

        struct _DistrWeight
        {
            int idx;
            double weight;
        };

        struct _MultibitNode
        {
            BingoPtr<_MatchBit> match_bits_array;
            int match_bits_count;
            BingoPtr<int> fp_indices_array;
            int fp_indices_count;
            BingoPtr<_MultibitNode> left;
            BingoPtr<_MultibitNode> right;

            _MultibitNode()
            {
                match_bits_count = 0;
                fp_indices_count = 0;
            }
        };

        int _min_fp_bit_number;
        int _max_fp_bit_number;
        int _fp_size;

        BingoPtr<byte> _fingerprints_ptr;
        BingoPtr<int> _indices_ptr;

        int _fp_count;
        BingoPtr<_MultibitNode> _tree_ptr;
        int _query_bit_number;
        int _max_level;

        static int _compareBitWeights(_DistrWeight& bw1, _DistrWeight& bw2, void* context);

        BingoPtr<_MultibitNode> _buildNode(Array<int>& fit_fp_indices, const Array<bool>& is_parrent_mb, int level);

        void _build();

        void _findLinear(_MultibitNode* node, const byte* query, int query_bit_number, SimCoef& sim_coef, double min_coef, Array<SimResult>& sim_indices,
                         int fp_bit_number = -1);

        void _findSimilarInNode(BingoPtr<_MultibitNode> node_ptr, const byte* query, int query_bit_number, SimCoef& sim_coef, double min_coef,
                                Array<SimResult>& sim_indices, int m01, int m10);
    };
}; // namespace bingo

#endif /* __multibit_tree__ */
