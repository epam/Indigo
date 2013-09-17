#ifndef __bingo_sim_storage__
#define __bingo_sim_storage__

#include "bingo_storage.h"
//#include "bingo_mmf.h"

#include "bingo_multibit_tree.h"
#include "bingo_container_set.h"
#include "bingo_fingerprint_table.h"
#include "bingo_tanimoto_coef.h"
#include "bingo_mmf_storage.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <new>

namespace bingo
{
   class SimStorage
   {
   public:
      size_t create (int fp_size, int mt_size);

      void load (int fp_size, size_t offset);

      size_t getOffset ();

      void add (const byte *fingerprint, int id);

      void findSimilar (const byte *query, SimCoef &sim_coef, double min_coef, Array<SimResult> &sim_fp_indices);

      void optimize ();

      int getCellCount () const;

      void getCellsInterval (const byte *query, SimCoef &sim_coef, double min_coef, int &min_cell, int &max_cell);

      int firstFitCell (int query_bit_count, int min_cell, int max_cell) const;

      int nextFitCell (int query_bit_count, int first_fit_cell, int min_cell, int max_cell, int idx) const;

      int getCellSize (int cell_idx) const;
      
      int getSimilar (const byte *query, SimCoef &sim_coef, double min_coef, 
                      Array<SimResult> &sim_fp_indices, int cell_idx, int cont_idx);

   private:
      BingoPtr<FingerprintTable> _table_ptr;
   };
};

#endif /* __bingo_sim_storage__ */
