#ifndef __multibit_tree__
#define __multibit_tree__

#include "bingo_cell_container.h"
#include "math/algebra.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/profiling.h"
#include "base_c/bitarray.h"
#include "bingo_sim_coef.h"
#include "bingo_ptr.h"

using namespace indigo;

namespace bingo 
{
   class MultibitTree
   {
   public:
      MultibitTree (int fp_size);

      void build (BingoPtr fingerprints, BingoPtr indices, int fp_count, int min_fp_bit_number, int max_fp_bit_number);

      int findSimilar (const byte *query, SimCoef &sim_coef, double min_coef, Array<SimResult> &sim_fp_indices);
      
   private:
      struct _MatchBit
      {
         int idx;
         bool val;

         _MatchBit()
         {
            idx = -1;
            bool val = 0;
         }

         _MatchBit (int new_idx, bool new_val) : idx(new_idx), val(new_val)
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
         BingoPtr match_bits_array;
         int match_bits_count;
         BingoPtr fp_indices_array;
         int fp_indices_count;
         BingoPtr left;
         BingoPtr right;

         _MultibitNode()
         {
            match_bits_array = -1;
            match_bits_count = 0;
            fp_indices_array = -1;
            fp_indices_count = 0;
            left = -1;
            right = -1;
         }
      };

      int _min_fp_bit_number;
      int _max_fp_bit_number;
      int _fp_size;

      BingoPtr _fingerprints_ptr;
      BingoPtr _indices_ptr;
      int _fp_count;
      BingoPtr _tree_ptr;
      int _query_bit_number;
      int _max_level;
      
      static int _compareBitWeights (_DistrWeight &bw1, _DistrWeight &bw2, void *context);

      BingoPtr _buildNode (Array<int> &fit_fp_indices, const Array<bool> &is_parrent_mb, int level);

      void _build ();

      void _findLinear (_MultibitNode *node, const byte *query, SimCoef &sim_coef, double min_coef, Array<SimResult> &sim_indices, int fp_bit_number = -1);

      void _findSimilarInNode (BingoPtr node_ptr, const byte *query, SimCoef &sim_coef, double min_coef, 
                                Array<SimResult> &sim_indices, int m01, int m10);
   };
};

#endif /* __multibit_tree__ */
