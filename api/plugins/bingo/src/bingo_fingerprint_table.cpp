#include "bingo_fingerprint_table.h"
#include "bingo_container_set.h"

#include <iostream>

using namespace bingo;

FingerprintTable::FingerprintTable( int fp_size, const Array<int> &borders, int mt_size ) : _fp_size(fp_size)
{
   int s1 = sizeof(*this);
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();

   _table_ptr = bingo_allocator->allocate<ContainerSet>(borders.size() - 1);
   byte *mem_ptr = bingo_allocator->get(_table_ptr);

   profTimerStart(tfp, "FingerprintTable constructing");

   ContainerSet *table = new(mem_ptr) ContainerSet[borders.size() - 1];
   _cell_count = borders.size() - 1;
   for (int i = 0; i < _cell_count; i++)
   {
      {
         profTimerStart(tfp, "FingerprintTable element pushing");
        table[i].setParams(_fp_size, mt_size, borders[i],  borders[i + 1] - 1);
      }
      std::cout << "Table Cell #" << i << " created" << std::endl;
   }
}

void FingerprintTable::add( const byte *fingerprint, int id )
{
   int fp_bit_count = bitGetOnesCount(fingerprint, _fp_size);

   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   ContainerSet *table = (ContainerSet *)bingo_allocator->get(_table_ptr);

   for (int i = 0; i < _cell_count; i++)
   {
      if ((fp_bit_count >= table[i].getMinBorder()) && (fp_bit_count <= table[i].getMaxBorder()))
      {
         table[i].add(fingerprint, id);
      }
   }  
}

void FingerprintTable::findSimilar( const byte *query, SimCoef &sim_coef, double min_coef, Array<int> &sim_fp_indices )
{
   sim_fp_indices.clear();
   
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   ContainerSet *table = (ContainerSet *)bingo_allocator->get(_table_ptr);

   int query_bit_number = bitGetOnesCount(query, _fp_size);

   std::cout << "query bit count - " << query_bit_number << std::endl;

   QS_DEF(Array<int>, cell_sim_indices);
   for (int i = 0; i < _cell_count; i++)
   {
      if (sim_coef.calcUpperBound(query_bit_number, table[i].getMinBorder(), table[i].getMaxBorder()) < min_coef)
         continue;

      cell_sim_indices.clear();
      table[i].findSimilar(query, sim_coef, min_coef, cell_sim_indices);

      sim_fp_indices.concat(cell_sim_indices);
   }
}

void FingerprintTable::optimize()
{
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   ContainerSet *table = (ContainerSet *)bingo_allocator->get(_table_ptr);

   for (int i = 0; i < _cell_count; i++)
   {
      table[i].optimize();
   }
}

int FingerprintTable::getCellCount()
{
   return _cell_count;
}

int FingerprintTable::getCellSize( int cell_idx )
{
   if (cell_idx >= _cell_count)
      throw Exception("FingerprintTable: Incorrect cell index");

   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   ContainerSet *table = (ContainerSet *)bingo_allocator->get(_table_ptr);

   return table[cell_idx].getContCount();
}

int FingerprintTable::getSimilar( const byte *query, SimCoef &sim_coef, double min_coef, 
                  Array<int> &sim_fp_indices, int cell_idx, int cont_idx )
{
   if (cell_idx >= _cell_count)
      throw Exception("FingerprintTable: Incorrect cell index");

   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   ContainerSet *table = (ContainerSet *)bingo_allocator->get(_table_ptr);

   int query_bit_number = bitGetOnesCount(query, _fp_size);

   if (sim_coef.calcUpperBound(query_bit_number, table[cell_idx].getMinBorder(), table[cell_idx].getMaxBorder()) < min_coef)
      return 0;

   table[cell_idx].getSimilar( query, sim_coef, min_coef, sim_fp_indices, cont_idx);

   return sim_fp_indices.size();
}

FingerprintTable::~FingerprintTable()
{
}