#include "bingo_fingerprint_table.h"
#include "bingo_container_set.h"

#include <iostream>

using namespace bingo;

FingerprintTable::FingerprintTable (int fp_size, const Array<int> &borders, int mt_size) : _fp_size(fp_size)
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
   }
}

void FingerprintTable::add (const byte *fingerprint, int id)
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

void FingerprintTable::findSimilar (const byte *query, SimCoef &sim_coef, double min_coef, Array<SimResult> &sim_fp_indices)
{
   sim_fp_indices.clear();
   
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   ContainerSet *table = (ContainerSet *)bingo_allocator->get(_table_ptr);

   int query_bit_number = bitGetOnesCount(query, _fp_size);

   QS_DEF(Array<SimResult>, cell_sim_indices);
   for (int i = 0; i < _cell_count; i++)
   {
      if (sim_coef.calcUpperBound(query_bit_number, table[i].getMinBorder(), table[i].getMaxBorder()) < min_coef)
         continue;

      cell_sim_indices.clear();
      table[i].findSimilar(query, sim_coef, min_coef, cell_sim_indices);

      sim_fp_indices.concat(cell_sim_indices);
   }
}

void FingerprintTable::optimize ()
{
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   ContainerSet *table = (ContainerSet *)bingo_allocator->get(_table_ptr);

   for (int i = 0; i < _cell_count; i++)
   {
      table[i].optimize();
   }
}

int FingerprintTable::getCellCount ()
{
   return _cell_count;
}

int FingerprintTable::getCellSize (int cell_idx)
{
   if (cell_idx >= _cell_count)
      throw Exception("FingerprintTable: Incorrect cell index");

   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   ContainerSet *table = (ContainerSet *)bingo_allocator->get(_table_ptr);

   return table[cell_idx].getContCount();
}

void FingerprintTable::getCellsInterval (const byte *query, SimCoef &sim_coef, double min_coef, int &min_cell, int &max_cell)
{
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   ContainerSet *table = (ContainerSet *)bingo_allocator->get(_table_ptr);

   min_cell = -1;
   max_cell = -1;
   int query_bit_count = bitGetOnesCount(query, _fp_size);
   for (int i = 0; i < _cell_count; i++)
   {
      if ((min_cell == -1) && 
          (sim_coef.calcUpperBound(query_bit_count, table[i].getMinBorder(), table[i].getMaxBorder()) > min_coef))
         min_cell = i;

      if ((min_cell != -1)&& 
          (sim_coef.calcUpperBound(query_bit_count, table[i].getMinBorder(), table[i].getMaxBorder()) > min_coef))
         max_cell = i;
   }
}

      
int FingerprintTable::firstFitCell (int query_bit_count, int min_cell, int max_cell) const
{
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   ContainerSet *table = (ContainerSet *)bingo_allocator->get(_table_ptr);


   int first_cell = -1;

   for (int i = min_cell; i <= max_cell; i++)
   {
      if (query_bit_count >= table[i].getMinBorder() && query_bit_count <= table[i].getMaxBorder())
         first_cell = i;
   }

   return first_cell;
}

int FingerprintTable::nextFitCell (int query_bit_count, int first_fit_cell, int min_cell, int max_cell, int idx) const
{
   int next_idx;

   if (first_fit_cell == idx)
      next_idx = first_fit_cell + 1;
   else if (first_fit_cell < idx)
      next_idx = first_fit_cell - (idx - first_fit_cell);
   else
      next_idx = first_fit_cell + (first_fit_cell - idx) + 1;

   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   ContainerSet *table = (ContainerSet *)bingo_allocator->get(_table_ptr);

   if (next_idx < min_cell || next_idx > max_cell)
   {
      if (idx < min_cell || idx > max_cell)
         return -1;
      else
         return nextFitCell(query_bit_count, first_fit_cell, min_cell, max_cell, next_idx );
   }

   return next_idx;
}

int FingerprintTable::getSimilar (const byte *query, SimCoef &sim_coef, double min_coef, 
                  Array<SimResult> &sim_fp_indices, int cell_idx, int cont_idx)
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

FingerprintTable::~FingerprintTable ()
{
}