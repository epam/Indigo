#include <stdio.h>
#include <new>
#include <fstream>
#include "base_cpp/exception.h"
#include "bingo_mmf_storage.h"

using namespace bingo;

MMFStorage::MMFStorage()
{
}

void MMFStorage::create (const char *filename, size_t size)
{
   BingoAllocator::_create(filename, size, &_mm_files);
}

void MMFStorage::load (const char *filename)
{
   BingoAllocator::_load(filename, &_mm_files);
}

void MMFStorage::close ()
{
   for (int i = 0; i < _mm_files.size(); i++)
      _mm_files[i].close();
}
