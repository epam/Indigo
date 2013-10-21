#include <stdio.h>
#include <new>
#include <fstream>
#include "base_cpp/exception.h"
#include "bingo_mmf_storage.h"

using namespace bingo;

BINGO_TL int MMFStorage::database_id = -1;

MMFStorage::MMFStorage()
{
}

void MMFStorage::create (const char *filename, size_t size, const char *header, int index_id)
{
   size_t header_len = strlen(header);
   _mm_files.clear();

   if (header_len >= max_header_len)
      throw Exception("MMfStorage: create(): Too long header");

   BingoAllocator::_create(filename, size, max_header_len, &_mm_files, index_id);

   BingoPtr<char> header_ptr(0);
   strcpy(header_ptr.ptr(), header);
}

void MMFStorage::load (const char *filename, BingoPtr<char> header_ptr, int index_id, bool read_only)
{
   _mm_files.clear();

   BingoAllocator::_load(filename, max_header_len, &_mm_files, index_id, read_only);

   header_ptr = BingoPtr<char>(0);
}

void MMFStorage::close ()
{
   for (int i = 0; i < _mm_files.size(); i++)
      _mm_files[i].close();
   _mm_files.clear();
}
