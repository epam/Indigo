#include "bingo_sim_storage.h"
#include "bingo_mmf_storage.h"

using namespace bingo;

size_t SimStorage::create (int fp_size, int mt_size )
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

   int table_size = sizeof(FingerprintTable);
   _table_ptr.allocate();
   FingerprintTable *table = new(_table_ptr.ptr()) FingerprintTable(fp_size, borders, mt_size);

   return (size_t)_table_ptr;
}

void SimStorage::load (int fp_size, size_t offset)
{
   _table_ptr = BingoPtr<FingerprintTable>(offset);
}

size_t SimStorage::getOffset ()
{
   return (size_t)_table_ptr;
}

void SimStorage::add (const byte *fingerprint, int id)
{
   _table_ptr.ptr()->add(fingerprint, id);
}

void SimStorage::findSimilar (const byte *query, SimCoef &sim_coef, double min_coef, Array<SimResult> &sim_fp_indices)
{
   _table_ptr.ptr()->findSimilar(query, sim_coef, min_coef, sim_fp_indices);
}

void SimStorage::optimize ()
{
   _table_ptr.ptr()->optimize();
}

void SimStorage::getCellsInterval (const byte *query, SimCoef &sim_coef, double min_coef, int &min_cell, int &max_cell)
{
   _table_ptr.ptr()->getCellsInterval(query, sim_coef, min_coef, min_cell, max_cell);
}

int SimStorage::getCellCount () const
{
   return _table_ptr.ptr()->getCellCount();
}

int SimStorage::firstFitCell (int query_bit_count, int min_cell, int max_cell) const
{
   return _table_ptr.ptr()->firstFitCell(query_bit_count, min_cell, max_cell);
}

int SimStorage::nextFitCell (int query_bit_count, int first_fit_cell, int min_cell, int max_cell, int idx) const
{
   return _table_ptr.ptr()->nextFitCell(query_bit_count, first_fit_cell, min_cell, max_cell, idx);
}

int SimStorage::getCellSize (int cell_idx) const
{
   return _table_ptr.ptr()->getCellSize( cell_idx );
}

int SimStorage::getSimilar (const byte *query, SimCoef &sim_coef, double min_coef, 
                           Array<SimResult> &sim_fp_indices, int cell_idx, int cont_idx)
{
   return _table_ptr.ptr()->getSimilar( query, sim_coef, min_coef, sim_fp_indices, cell_idx, cont_idx);
}