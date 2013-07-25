#ifndef __bingo_ptr__
#define __bingo_ptr__

#include "base_cpp/exception.h"
#include <new>

//typedef unsigned char byte;

using namespace indigo;

namespace bingo
{
   typedef size_t BingoPtr;

   class BingoAllocator
   {
   public:

      static BingoAllocator *create(byte *memory_ptr = 0, size_t size = 0);

      static BingoAllocator *load(byte *memory_ptr = 0, size_t size = 0);

      static BingoAllocator *getInstance();

      byte * get ( BingoPtr bingo_ptr );

      template<typename T> BingoPtr allocate ( int count = 1 )
      {
         int alloc_size = sizeof(T) * count;

         if (alloc_size > _size - _free_off)
            throw Exception("BingoAllocator: not enough memory");

         size_t res_off = _free_off;
         _free_off += alloc_size;

         return res_off;
      }

   private:
      BingoAllocator (byte *begin_ptr, size_t size) : _begin_ptr(begin_ptr), _free_off(0), _size(size)
      {
      }

      static BingoAllocator *_instance;
      byte *_begin_ptr;
      size_t _free_off;
      size_t _size;
   };
};

#endif //__bingo_ptr__
