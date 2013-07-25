#include "bingo_sim_storage.h"

using namespace bingo;

void SimStorage::create( int fp_size, void *memory_ptr, size_t size, int mt_size )
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

   BingoAllocator * bingo_allocator = BingoAllocator::create((byte *)memory_ptr, size);

   int table_size = sizeof(FingerprintTable);
   _table_ptr = bingo_allocator->allocate<FingerprintTable>();
   byte *mem_ptr = bingo_allocator->get(_table_ptr);
   FingerprintTable *table = new(mem_ptr) FingerprintTable(fp_size, borders, mt_size);
   return;
}

void SimStorage::load( int fp_size, void *memory_ptr, size_t size )
{
   BingoAllocator * bingo_allocator = BingoAllocator::load((byte *)memory_ptr, size);

   int table_size = sizeof(FingerprintTable);
   _table_ptr = 0;
   FingerprintTable *table = (FingerprintTable *)bingo_allocator->get(_table_ptr);
   return;
}

void SimStorage::add( const byte *fingerprint, int id )
{
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   FingerprintTable *fp_table = (FingerprintTable *)bingo_allocator->get(_table_ptr);

   fp_table->add(fingerprint, id);
}

void SimStorage::findSimilar( const byte *query, SimCoef &sim_coef, double min_coef, Array<int> &sim_fp_indices )
{
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   FingerprintTable *fp_table = (FingerprintTable *)bingo_allocator->get(_table_ptr);

   fp_table->findSimilar(query, sim_coef, min_coef, sim_fp_indices);
}

void SimStorage::optimize()
{
    BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   FingerprintTable *fp_table = (FingerprintTable *)bingo_allocator->get(_table_ptr);

   fp_table->optimize();
}

int SimStorage::getCellCount() const
{
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   FingerprintTable *fp_table = (FingerprintTable *)bingo_allocator->get(_table_ptr);

   return fp_table->getCellCount();
}

int SimStorage::getCellSize( int cell_idx ) const
{
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   FingerprintTable *fp_table = (FingerprintTable *)bingo_allocator->get(_table_ptr);

   return fp_table->getCellSize( cell_idx );
}

int SimStorage::getSimilar( const byte *query, SimCoef &sim_coef, double min_coef, 
                  Array<int> &sim_fp_indices, int cell_idx, int cont_idx )
{
   BingoAllocator * bingo_allocator = BingoAllocator::getInstance();
   FingerprintTable *fp_table = (FingerprintTable *)bingo_allocator->get(_table_ptr);

   return fp_table->getSimilar( query, sim_coef, min_coef, sim_fp_indices, cell_idx, cont_idx);
}