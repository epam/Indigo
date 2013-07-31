#include "bingo_container_set.h"
#include "bingo_multibit_tree.h"

using namespace bingo;

ContainerSet::ContainerSet()
{
   _inc_count = 0;
   _set_size = 0;
}

void ContainerSet::setParams( int fp_size, int container_size, int min_ones_count, int max_ones_count )
{
   profTimerStart(t, "cs_set_params");
   _fp_size = fp_size; 
   _min_ones_count = min_ones_count;
   _max_ones_count = max_ones_count; 
   _container_size = container_size;

   BingoAllocator *bingo_allocator = BingoAllocator::getInstance();
   _increment = bingo_allocator->allocate<byte>(_container_size * _fp_size);
   _indices = bingo_allocator->allocate<int>(_container_size);
   for (int i = 0; i < _max_set_size; i++)
   {
      _set[i] = bingo_allocator->allocate<MultibitTree>(1);
      byte *cont_ptr = bingo_allocator->get(_set[i]);
      new(cont_ptr) MultibitTree(_fp_size);
   }
}

int ContainerSet::getContCount()
{
   return _set_size + 1;
}

int ContainerSet::getMinBorder()
{
   return _min_ones_count;
}

int ContainerSet::getMaxBorder()
{
   return _max_ones_count;
}

void ContainerSet::add( const byte *fingerprint, int id )
{
   BingoAllocator *bingo_allocator = BingoAllocator::getInstance();
   byte *inc = bingo_allocator->get(_increment);
   int *indices = (int *)bingo_allocator->get(_indices);

   memcpy(inc + _inc_count * _fp_size, fingerprint, _fp_size);
   indices[_inc_count] = id;
   _inc_count++;

   if (_inc_count == _container_size)
   {
      profIncCounter("trees_count", 1);
      
      if (_set_size == _max_set_size)
         throw Exception("Container set: Set overflow");

      _set_size++;

      MultibitTree *cont = (MultibitTree *)bingo_allocator->get(_set[_set_size - 1]);
      
      cont->build(_increment, _indices, _container_size, _min_ones_count, _max_ones_count);
      _increment = bingo_allocator->allocate<byte>(_container_size * _fp_size);
      _indices = bingo_allocator->allocate<int>(_container_size);
   
      _inc_count = 0;
   }
}

void ContainerSet::findSimilar( const byte *query, SimCoef &sim_coef, double min_coef, Array<SimResult> &sim_indices )
{
   BingoAllocator *bingo_allocator = BingoAllocator::getInstance();
   sim_indices.clear();

   static int idx = 0;

   int query_bit_number = bitGetOnesCount(query, _fp_size);

   QS_DEF(Array<SimResult>, cell_sim_indices);
   for (int i = 0; i < _set_size; i++)
   {
      MultibitTree *container = (MultibitTree *)bingo_allocator->get(_set[i]);
      cell_sim_indices.clear();
      container->findSimilar(query, sim_coef, min_coef, cell_sim_indices);

      sim_indices.concat(cell_sim_indices);
   }

   cell_sim_indices.clear();
   _findSimilarInc(query, sim_coef, min_coef, cell_sim_indices);
   sim_indices.concat(cell_sim_indices);

   idx++;
}

void ContainerSet::optimize()
{
   BingoAllocator *bingo_allocator = BingoAllocator::getInstance();
   
   if (_inc_count < _container_size / 10)
      return;

   profIncCounter("trees_count", 1);   

   if (_set_size == _max_set_size)
      throw Exception("Container set: Set overflow");

   _set_size++;

   MultibitTree *cont = (MultibitTree *)bingo_allocator->get(_set[_set_size - 1]);
      
   cont->build(_increment, _indices, _inc_count, _min_ones_count, _max_ones_count);
   _increment = bingo_allocator->allocate<byte>(_container_size * _fp_size);
   _indices = bingo_allocator->allocate<int>(_container_size);
   
   _inc_count = 0;
}

int ContainerSet::getSimilar( const byte *query, SimCoef &sim_coef, double min_coef, 
                  Array<SimResult> &sim_fp_indices, int cont_idx )
{
   profTimerStart(cs_s, "getSimilar");
   
   if (cont_idx >= getContCount())
      throw Exception("ContainerSet: Incorrect container index");


   if (cont_idx == _set_size)
   {
      {
         profTimerStart(cs_s, "inc_findSimilar");
         _findSimilarInc(query, sim_coef, min_coef, sim_fp_indices);
      }

      return sim_fp_indices.size();
   }


   BingoAllocator *bingo_allocator = BingoAllocator::getInstance();

   MultibitTree *container = (MultibitTree *)bingo_allocator->get(_set[cont_idx]);

   {
      profTimerStart(cs_s, "set_findSimilar");
      container->findSimilar(query, sim_coef, min_coef, sim_fp_indices);
   }

   return sim_fp_indices.size();
}

int ContainerSet::_findSimilarInc (const byte *query, SimCoef &sim_coef, double min_coef, Array<SimResult> &sim_indices)
{
   BingoAllocator *bingo_allocator = BingoAllocator::getInstance();
   byte *inc = bingo_allocator->get(_increment);
   int *indices = (int *)bingo_allocator->get(_indices);

   sim_indices.clear();

   int query_bit_number = bitGetOnesCount(query, _fp_size);

   for (int i = 0; i < _inc_count; i++)
   {
      byte *fp = inc + i * _fp_size;
      int fp_bit_number = bitGetOnesCount(query, _fp_size);

      double coef = sim_coef.calcCoef(query, fp, query_bit_number, fp_bit_number);
      if (coef < min_coef)
         continue;

      sim_indices.push(SimResult(indices[i], (float)coef));
   }

   return sim_indices.size();
}