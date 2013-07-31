#ifndef __container_set__
#define __container_set__

#include "base_c/bitarray.h"
#include "base_cpp/tlscont.h"
#include "base_cpp/obj_array.h"
#include "bingo_cell_container.h"

#include "bingo_ptr.h"

#include <vector>

#include "base_cpp/profiling.h"

using namespace indigo;

namespace bingo
{
   class ContainerSet
   {
   public:
      ContainerSet ();

      ContainerSet (int fp_size, int container_size, int min_ones_count, int max_ones_count);

      void setParams (int fp_size, int container_size, int min_ones_count, int max_ones_count);

      int getContCount ();

      int getMinBorder ();

      int getMaxBorder ();

      void add (const byte *fingerprint, int id);

      void findSimilar (const byte *query, SimCoef &sim_coef, double min_coef, Array<SimResult> &sim_indices);

      void optimize ();

      int getSimilar (const byte *query, SimCoef &sim_coef, double min_coef, 
                        Array<SimResult> &sim_fp_indices, int cont_idx);

   private:
      static const int _max_set_size = 128;
      BingoPtr _set[_max_set_size];
      int _set_size;
      int _fp_size;
      int _container_size;
      BingoPtr _increment;
      BingoPtr _indices;

      int _inc_count;
      int _min_ones_count;
      int _max_ones_count;

      int _findSimilarInc (const byte *query, SimCoef &sim_coef, double min_coef, Array<SimResult> &sim_indices);
   };
};

#endif /* __container_set__ */
