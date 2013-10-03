#include "bingo_fingerprint_table.h"
#include "bingo_container_set.h"

#include <iostream>

using namespace bingo;

FingerprintTable::FingerprintTable (int fp_size, const Array<int> &borders, int mt_size) : _fp_size(fp_size)
{
   _table_ptr.allocate(borders.size() - 1);
   
   profTimerStart(tfp, "FingerprintTable constructing");

   _cell_count = borders.size() - 1;

   ContainerSet *table = _table_ptr.ptr();

   for (int i = 0; i < _cell_count; i++)
   {
      {
         profTimerStart(tfp, "FingerprintTable element pushing");
         new(table + i) ContainerSet();
         table[i].setParams(_fp_size, mt_size, borders[i],  borders[i + 1] - 1);
      }
   }
}

size_t FingerprintTable::create (BingoPtr<FingerprintTable> &ptr, int fp_size, int mt_size )
{
   int borders_buf[97] = {0 ,18,24,28,31,34,37,39,41,43,45,47,49,51,52,53,
                     54,55,56,57,58,59,60,61,62,63,64,65,66,67,68,69,
                     70,71,72,73,74,75,76,77,78,79,80,81,82,83,84,85,
                     86,87,88,89,90,91,92,93,94,95,96,97,98,99,
                     100,101,102,103,104,105,106,107,108,109,110,
                     111,112,113,114,115,116,117,118,119,120,122,
                     124,126,128,130,132,135,138,141,145,150,158,197,513};

   Array<int> borders;
   borders.copy(borders_buf, 97);

   ptr.allocate();
   new(ptr.ptr()) FingerprintTable(fp_size, borders, mt_size);

   return (size_t)ptr;
}

void FingerprintTable::load (BingoPtr<FingerprintTable> &ptr, size_t offset)
{
   ptr = BingoPtr<FingerprintTable>(offset);
}

void FingerprintTable::add (const byte *fingerprint, int id)
{
   int fp_bit_count = bitGetOnesCount(fingerprint, _fp_size);

   ContainerSet *table = _table_ptr.ptr();

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
   
   int query_bit_number = bitGetOnesCount(query, _fp_size);

   ContainerSet *table = _table_ptr.ptr();

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
   ContainerSet *table = _table_ptr.ptr();

   for (int i = 0; i < _cell_count; i++)
      table[i].optimize();
}

int FingerprintTable::getCellCount () const
{
   return _cell_count;
}

int FingerprintTable::getCellSize (int cell_idx) const
{
   if (cell_idx >= _cell_count)
      throw Exception("FingerprintTable: Incorrect cell index");

   const ContainerSet *table = _table_ptr.ptr();

   return table[cell_idx].getContCount();
}

void FingerprintTable::getCellsInterval (const byte *query, SimCoef &sim_coef, double min_coef, int &min_cell, int &max_cell)
{
   min_cell = -1;
   max_cell = -1;
   int query_bit_count = bitGetOnesCount(query, _fp_size);

   ContainerSet *table = _table_ptr.ptr();

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
   int first_cell = -1;

   const ContainerSet *table = _table_ptr.ptr();

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

   ContainerSet *table = _table_ptr.ptr();

   int query_bit_number = bitGetOnesCount(query, _fp_size);

   if (sim_coef.calcUpperBound(query_bit_number, table[cell_idx].getMinBorder(), table[cell_idx].getMaxBorder()) < min_coef)
      return 0;

   table[cell_idx].getSimilar( query, sim_coef, min_coef, sim_fp_indices, cont_idx);

   return sim_fp_indices.size();
}

FingerprintTable::~FingerprintTable ()
{
}