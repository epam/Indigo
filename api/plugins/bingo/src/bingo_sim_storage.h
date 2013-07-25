#ifndef __bingo_sim_storage__
#define __bingo_sim_storage__

#include "bingo_storage.h"
//#include "bingo_mmf.h"

#include "bingo_multibit_tree.h"
#include "bingo_container_set.h"
#include "bingo_fingerprint_table.h"
#include "bingo_tanimoto_coef.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <new>

namespace bingo
{
   class SimStorage
   {
   public:
      void create( int fp_size, void *memory_ptr, size_t size, int mt_size );

      void load( int fp_size, void *memory_ptr, size_t size );

      void add( const byte *fingerprint, int id );

      void findSimilar( const byte *query, SimCoef &sim_coef, double min_coef, Array<int> &sim_fp_indices );

      void optimize();

      int getCellCount() const;

      int getCellSize( int cell_idx ) const;

      int getSimilar( const byte *query, SimCoef &sim_coef, double min_coef, 
                      Array<int> &sim_fp_indices, int cell_idx, int cont_idx );


   private:
      BingoPtr _table_ptr;
   };
};

#endif /* __bingo_sim_storage__ */
