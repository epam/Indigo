#ifndef __fingerprint_table__
#define __fingerprint_table__

#include "base_cpp/scanner.h"
#include "base_cpp/output.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/obj_array.h"
#include "math/algebra.h"
#include "time.h"
#include "new"
#include "bingo_sim_coef.h"
#include "bingo_ptr.h"

#include <vector>

using namespace indigo;
namespace bingo
{
   class FingerprintTable
   {
   public:
      FingerprintTable( int fp_size, const Array<int> &borders, int mt_size );

      void add( const byte *fingerprint, int id );

      void findSimilar( const byte *query, SimCoef &sim_coef, double min_coef, Array<int> &sim_fp_indices );

      void optimize();

      int getCellCount();

      int getCellSize( int cell_idx );

      int getSimilar( const byte *query, SimCoef &sim_coef, double min_coef, 
                        Array<int> &sim_fp_indices, int cell_idx, int cont_idx );

      ~FingerprintTable();
   
   private:
      BingoPtr _table_ptr;
      int _cell_count;
      int _fp_size;
   };
};

#endif /* __fingerprint_table__ */
