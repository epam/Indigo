#include <iostream>
#include "bingo_ptr.h"

using namespace indigo;
using namespace bingo;

BingoAllocator * BingoAllocator::_instance = 0;

BingoAllocator * BingoAllocator::create(byte *memory_ptr, size_t size)
{
   if ((memory_ptr == 0) || (size == 0) || (size < sizeof(BingoAllocator)))
      throw Exception("BingoAllocator: Incorrect instance initialization");

   _instance = new(memory_ptr) BingoAllocator(memory_ptr + sizeof(BingoAllocator), 
                                                size - sizeof(BingoAllocator));

   return _instance;
}

BingoAllocator * BingoAllocator::load(byte *memory_ptr, size_t size)
{
   if ((memory_ptr == 0) || (size == 0) || (size < sizeof(BingoAllocator)))
      throw Exception("BingoAllocator: Incorrect instance initialization");

   _instance = (BingoAllocator *)memory_ptr;
   _instance->_begin_ptr = memory_ptr + sizeof(BingoAllocator);
   return _instance;
}

BingoAllocator * BingoAllocator::getInstance()
{
   return _instance;
}

byte * BingoAllocator::get ( BingoPtr bingo_ptr )
{
   if (bingo_ptr >= _size)
      throw Exception("BingoAllocator: out of memory");

   return _begin_ptr + bingo_ptr;
}