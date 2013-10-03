#include <stdio.h>
#include <new>
#include <fstream>
#include "base_cpp/exception.h"
#include "bingo_mmf_storage.h"

using namespace bingo;

MMFStorage::MMFStorage()
{
}

void MMFStorage::create (const char *filename, size_t size, const char *header)
{
   size_t header_len = strlen(header);

   if (header_len >= max_header_len)
      throw Exception("MMfStorage: create(): Too long header");

   BingoAllocator::_create(filename, size, max_header_len, &_mm_files);

   BingoPtr<char> header_ptr(0);
   strcpy(header_ptr.ptr(), header);
}

void MMFStorage::load (const char *filename, BingoPtr<char> header_ptr)
{
   BingoAllocator::_load(filename, max_header_len, &_mm_files);
   
   header_ptr = BingoPtr<char>(0);
}

void MMFStorage::close ()
{
   for (int i = 0; i < _mm_files.size(); i++)
      _mm_files[i].close();
}
